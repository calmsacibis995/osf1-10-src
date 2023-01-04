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
 * @(#)ka_ruby.h	9.3  (ULTRIX)        10/29/91
 */

/*
 * Modification History: machine/alpha/ka_ruby.h
 *
 *      14-Jan-92       prm
 *         - Add gbus structure definition for Watch's csrb. Modify csra
 *           definitions to avoid conflict with csrb. Fix gbus_watch offsets.
 *
 *      28-oct-91       prm
 *         - Add more gbus structure definitions, including BB_WATCH (toy).
 *         - Change mchk_logout to ruby_mchk_logout, since it will not be the
 *           same across platforms.
 *
 *      13-Sep-91       prm (Peter Mott) 
 *           Add definitions for Gbus uart support.
 *
 *	12-sep-91	jac	Added lots of register definitions...
 *
 * Jun-91	jac: created this file for processor support of Laser/Ruby
 *
 */

#ifndef __KN7AA_H__
#define __KN7AA_H__
#include <sys/types.h>	/* included to define type u_long for */
			/* el_ruby_mcheck_data structure */
#include <hal/dc21064.h>

/*
 * Ruby register definitions
*/

struct ruby_reg {
	unsigned int  	lep_ldev;	/* Laser device register	*/
	char		lep_pad0[0x3c];	/* 60 byte pad			*/
	unsigned int  	lep_lber;	/* Laser bus error register	*/
	char		lep_pad1[0x3c];	/* 60 byte pad			*/
	unsigned int	lep_lcnr;	/* Laser configuration register	*/
	char		lep_pad2[0x17c]; /* 380 byte pad		*/
	unsigned int	lep_lmmr0;	/* Laser memory mapping Reg 0	*/
	char		lep_pad3[0x3c];	/* 60 byte pad			*/
	unsigned int	lep_lmmr1;	/* Laser memory mapping Reg 1	*/
	char		lep_pad4[0x3c];	/* 60 byte pad			*/
	unsigned int	lep_lmmr2;	/* Laser memory mapping Reg 2	*/
	char		lep_pad5[0x3c];	/* 60 byte pad			*/
	unsigned int	lep_lmmr3;	/* Laser memory mapping Reg 3	*/
	char		lep_pad6[0x3c];	/* 60 byte pad			*/
	unsigned int	lep_lmmr4;	/* Laser memory mapping Reg 4	*/
	char		lep_pad7[0x3c];	/* 60 byte pad			*/
	unsigned int	lep_lmmr5;	/* Laser memory mapping Reg 5	*/
	char		lep_pad8[0x3c];	/* 60 byte pad			*/
	unsigned int	lep_lmmr6;	/* Laser memory mapping Reg 6	*/
	char		lep_pad9[0x3c];	/* 60 byte pad			*/
	unsigned int	lep_lmmr7;	/* Laser memory mapping Reg 7	*/
	char		lep_pad10[0x23c]; /* 572 byte pad		*/
	unsigned int	lep_lbesr0;	/* Laser bus error syndrome R0	*/
	char		lep_pad11[0x3c]; /* 60 byte pad			*/
	unsigned int	lep_lbesr1;	/* Laser bus error syndrome R1	*/
	char		lep_pad12[0x3c]; /* 60 byte pad			*/
	unsigned int	lep_lbesr2;	/* Laser bus error syndrome R2	*/
	char		lep_pad13[0x3c]; /* 60 byte pad			*/
	unsigned int	lep_lbesr3;	/* Laser bus error syndrome R3	*/
	char		lep_pad14[0x3c]; /* 60 byte pad			*/
	unsigned int	lep_lbecr0;	/* Laser bus error command R0	*/
	char		lep_pad15[0x3c]; /* 60 byte pad			*/
	unsigned int	lep_lbecr1;	/* Laser bus error command R1	*/
	char		lep_pad16[0x4bc]; /* 1212 byte pad		*/
	unsigned int	lep_ldiag;	/* LEP diagnostic control reg	*/
	char		lep_pad17[0x3c]; /* 60 byte pad			*/
	unsigned int	lep_lmerr;	/* LEP module error reg		*/
	char		lep_pad18[0x3c]; /* 60 byte pad			*/
	unsigned int	lep_ltaga;	/* LEP tag address reg		*/
	char		lep_pad19[0x3c]; /* 60 byte pad			*/
	unsigned int	lep_ltagw;	/* LEP tag write data reg	*/
	char		lep_pad20[0x3c]; /* 60 byte pad			*/
	unsigned int	lep_ltagr;	/* LEP tag read data reg	*/
	char		lep_pad21[0x3c]; /* 60 byte pad			*/
	unsigned int	lep_llock;	/* LEP lock address reg		*/
	char		lep_pad22[0x3c]; /* 60 byte pad			*/
	unsigned int	lep_lwpend;	/* LEP write pending address reg*/
	char		lep_pad23[0x3c]; /* 60 byte pad			*/
	unsigned int	lep_lvict;	/* LEP victum address reg	*/
	char		lep_pad24[0x3c]; /* 60 byte pad			*/
	unsigned int	lep_lcon0;	/* LEP console communication reg*/
	char		lep_pad25[0x3c]; /* 60 byte pad			*/
	unsigned int	lep_lcon1;	/* LEP console communication reg*/
	char		lep_pad26[0x3c]; /* 60 byte pad			*/
	unsigned int	lep_lmode;	/* LEP mode reg			*/
	char		lep_pad27[0x3c]; /* 60 byte pad			*/
	unsigned int	lep_ledto;	/* LEP EDAL timeout reg		*/
	char		lep_pad28[0x3c]; /* 60 byte pad			*/
	unsigned int	lep_liointr;	/* IO interrupt reg		*/
	char		lep_pad29[0x3c]; /* 60 byte pad			*/
	unsigned int	lep_lipintr;	/* IP interrupt reg		*/
};


/* GBUS addresses of UART base registers et. al. */
#define GBUS_LOC_BASE 	0x3F4000000
#define GBUS_PS_BASE 	0x3F4800000
#define GBUS_P_BASE 	0x3F5000000

#define GBUS_DUART0 	0x3F4000000
#define GBUS_DUART1 	0x3F4800000
#define GBUS_DUART2 	0x3F5000000
#define GBUS_WATCH 	0x3F6000000
#define GBUS_MISC 	0x3F7000000


/* GBUS offsets into mapped pages... */
/* UARTxA and UARTxB have swapped address space from LEP spec of 10/90 */
#define GBUS_UART0A_PG_OFFSET 0x80

#define GBUS_UART_TxEM 0x02

/* Define all UART ports */
#define UART0A 0x1
#define UART0B 0x2
#define UART1A 0x3
#define UART1B 0x4
#define UART2A 0x5
#define UART2B 0x6

/* Define UART ports used */
#define UART_LOCAL UART0A
#define UART_PS UART1B
#define UART_P UART2A

/*
 * define gbus uart register secondary access indexes
 */
#define WR0 0

#define WR1 1
#define WR2 2 
#define WR3 3 
#define WR4 4 
#define WR5 5 
#define WR6 6
#define WR7 7

#define WR8 8

#define WR9  9
#define WR10 10
#define WR11 11
#define WR12 12
#define WR13 13
#define WR14 14
#define WR15 15

#define RR0  0
#define RR1  1
#define RR2  2 
#define RR3  3 

#define RR8_OFFSET 0x40

#define RR8  8
#define RR10 10
#define RR13 13
#define RR15 15


/*
 * define gbus structures and constants...
 */

/* Watch chip csra structure def */

typedef union watch_csra {
  unsigned char reg;
  unsigned char rega;
  struct csra_bits {
    unsigned char rs: 4;
    unsigned char dv: 3;
    unsigned char uip: 1;
  } csra_bits;
} watch_csra;

/* Watch chip csra bit/field defs within the byte */

#define WATCH_CSRA_V_RS  0x000	/* Rate Select                 */
#define WATCH_CSRA_S_RS  0x004
#define WATCH_CSRA_M_RS  0x00F
#define WATCH_CSRA_K_RS  0x006  /* HW Group says set to 0b0110 */

#define WATCH_CSRA_V_DV  0x004	/* Divider select                 */
#define WATCH_CSRA_S_DV  0x003
#define WATCH_CSRA_M_DV  0x070
#define WATCH_CSRA_K_DV  0x020	/* expect Console to set to 0b010 */

#define WATCH_CSRA_V_UIP 0x007  /* Update in progress */
#define WATCH_CSRA_S_UIP 0x001	/* Read Only field    */
#define WATCH_CSRA_M_UIP 0x080

#define WATCH_CSRA_SET   0x026	/* This setting makes the clock run at */
				/* realtime. i.e. It agrees with the   */
				/* clock on the wall                   */

/* Watch chip csra defs within structure field definitions */

#define WATCH_CSRA_RS_SET 0b0110/* The meaning of this 4 bit field is */
				/* dependant upon the setting of the  */
				/* DV field. See LEP spec.            */

#define WATCH_CSRA_DV_SET 0b010	/* This sets the clock divider to */
				/* ~32kHz. As specified by the */
				/* hardware group */

/* Watch chip csrb structure def */

typedef union watch_csrb {
  unsigned char reg;
  unsigned char regb;
  struct csrb_bits {
    unsigned char dse: 1;
    unsigned char mlt: 1;  
    unsigned char dm: 1;
    unsigned char sqwe: 1;
    unsigned char uie: 1;
    unsigned char aie: 1;
    unsigned char pie: 1;
    unsigned char vset: 1;
  } csrb_bits;
} watch_csrb;

/* Watch chip csrb bit/field defs within the byte */

#define WATCH_CSRB_V_DSE 0x000  /* Daylight Savings time */
#define WATCH_CSRB_S_DSE 0x001
#define WATCH_CSRB_M_DSE 0x001
#define WATCH_CSRB_K_DSE 0x000  /* We want this off! - 00 */

#define WATCH_CSRB_V_HM 0x001	/* Hour Mode: 0 = am/pm, 1 = 24hr */ 
#define WATCH_CSRB_S_HM 0x001
#define WATCH_CSRB_M_HM 0x002
#define WATCH_CSRB_K_HM 0x002	/* Want 24 hour mode */

#define WATCH_CSRB_V_DM 0x002	/* Data Mode: 0 = BCD, 1 = binary */ 
#define WATCH_CSRB_S_DM 0x001
#define WATCH_CSRB_M_DM 0x004
#define WATCH_CSRB_K_DM 0x004	/* We want this set */

#define WATCH_CSRB_V_SQWE 0x003	/* Square Wave Interrupt enable */ 
#define WATCH_CSRB_S_SQWE 0x001
#define WATCH_CSRB_M_SQWE 0x008
#define WATCH_CSRB_K_SQWE 0x008	/* expect console to set this */

#define WATCH_CSRB_V_UIE 0x004	/* Update ended Interrupt enable   */ 
#define WATCH_CSRB_S_UIE 0x001
#define WATCH_CSRB_M_UIE 0x010
#define WATCH_CSRB_K_UIE 0x000	/* dont want this */

#define WATCH_CSRB_V_AIE 0x005	/* Alarm Interrupt enable   */ 
#define WATCH_CSRB_S_AIE 0x001
#define WATCH_CSRB_M_AIE 0x020
#define WATCH_CSRB_K_AIE 0x000	/* dont want this */

#define WATCH_CSRB_V_PIE 0x006	/* Periodic Interrupt enable   */ 
#define WATCH_CSRB_S_PIE 0x001
#define WATCH_CSRB_M_PIE 0x040
#define WATCH_CSRB_K_PIE 0x040	/* expect console to set this */

#define WATCH_CSRB_V_SET 0x007	/* Set mode */
#define WATCH_CSRB_S_SET 0x001
#define WATCH_CSRB_M_SET 0x080

#define WATCH_CSRB_SET 0x04E	/* what we would like to see in csrb */

/* Watch chip csrc structure def */

typedef union watch_csrc {
  unsigned char reg;
  unsigned char regc;
  struct csrc_bits {
    unsigned char rsvd: 3;  
    unsigned char uf: 1;  
    unsigned char af: 1;  
    unsigned char pf: 1;  
    unsigned char irqf: 1;  
  } csrc_bits;
} watch_csrc;

/* Watch chip csrc bit/field defs within the byte */

#define WATCH_CSRC_SET 0x000  /* expect to be... */

/* Watch chip csrd structure def */

typedef union watch_csrd {
  unsigned char reg;
  unsigned char regd;
  struct csrd_bits {
    unsigned char rsvd: 7;
    unsigned char vrt: 1;  
  } csrd_bits;
} watch_csrd;

#define csrd_vrt csrd.csrd_bits.vrt

/* Watch chip csrd bit/field defs within the byte */

#define WATCH_CSRD_V_VRT 0x008	/* Valid Ram Time Indicates the */
#define WATCH_CSRD_S_VRT 0x001	/* contents of the watch have been */
#define WATCH_CSRD_M_VRT 0x080	/* maintained since set. (Not */
				/* necessarily correct)       */
#define WATCH_CSRD_K_VRT 0x080  /* expect to be... */

#define WATCH_CSRD_SET 0x080  /* expect to be... */

/* Watch chip structure def */

struct gbus_watch {
  unsigned char seconds;
  char fill1[0x7F];
  unsigned char minutes;
  char fill2[0x7F];
  unsigned char hours;
  char fill3[0xBF];
  unsigned char day;
  char fill4[0x3F];
  unsigned char month;
  char fill5[0x3F];
  unsigned char year;
  char fill6[0x3F];
  watch_csra csra;
  char fill7[0x3F];
  watch_csrb csrb;
  char fill8[0x3F];
  watch_csrc csrc;
  char fill9[0x3F];
  watch_csrd csrd;
};

struct gbus_misc {
  unsigned char whami;
  char fill1[0x3F];
  unsigned char leds;
  char fill2[0x3F];
  unsigned char pmask;
  char fill3[0x3F];
  unsigned char intr;
  char fill4[0x3F];
  unsigned char halt;
  char fill5[0x3F];
  unsigned char lsbrst;
};

struct gbus_map {
  struct gbus_uart *uart0; 
  struct gbus_uart *uart1;
  struct gbus_uart *uart2;
  struct gbus_watch *watch;
  struct gbus_misc *misc;
};

struct gbus_uart_foo {
  vm_offset_t reg;
};

struct gbus_uart {
  union {
    unsigned char uart_wr0;
    unsigned char uart_wr1;
    unsigned char uart_wr2;
    unsigned char uart_wr3;
    unsigned char uart_wr4;
    unsigned char uart_wr5;
    unsigned char uart_wr6;
    unsigned char uart_wr7;

    unsigned char uart_rr0;
    unsigned char uart_rr1;
    unsigned char uart_rr2;
    unsigned char uart_rr3;
  } gu1;
  char uart_fill[0x3F]; /* unused IO space between DUART registers */
  union {
    unsigned char uart_wr8;
    unsigned char uart_wr9;
    unsigned char uart_wr10;
    unsigned char uart_wr11;
    unsigned char uart_wr12;
    unsigned char uart_wr13;
    unsigned char uart_wr14;
    unsigned char uart_wr15;

    unsigned char uart_rr8;
    unsigned char uart_rr10;
    unsigned char uart_rr13;
    unsigned char uart_rr15;
  } gu2;
};
    

#define UART_TXINTENA 2
#define UART_RXINTENA 16

/*
 * System variation numbers used to differenciate between laser and blazer
*/

#define DEC7000_SYSVAR	0L
#define DEC10000_SYSVAR	1L



#define PRM_NEW_MCHECK_FRAME

struct ruby_mchk_logout {
#ifndef PRM_NEW_MCHECK_FRAME
#define KN7AA_MCHECK_FRAME_PA 0x6010	/* This is setup because the */
					/* RPD doesn't have it, and we */
					/* need it */
        long    retry;		/* +6010 */
	long	unused1;
	int	proc_off;	/* +6020 */
	int	sys_off;	/* */
	long	unused2;
	long	das_tag;	/* +6030 */
	long	unused3;
	long	pal_temps[32*2];/* +6040 */
	long	exc_addr;	/* +6240 */
	long	unused4;
        long    exc_sum;	/* +6250 */
	long	unused5;
	long	iccsr;		/* +6260 */
	long	unused6;
/*        long    msk; */
        long    pal_base;	/* +6270 */
	long	unused7;
        long    hier;		/* +6280 */
	long	unused8;
        long    hirr;		/* +6290 */
	long	unused9;
        long    mm_csr;		/* +62A0 */
	long	unused10;
        long    dc_stat;	/* +62B0 */
	long	unused11;
        long    dc_addr;	/* +62C0 */
	long	unused12;
	long	abox_ctl;	/* +62D0 */
	long	unused13;
        long    biu_stat;	/* +62E0 */
	long	unused14;
        long    biu_addr;	/* +62F0 */
	long	unused15;
	long	biu_ctl;	/* +6300 */
	long	unused16;
        long    fill_syndrome;	/* +6310 */
	long	unused17;
        long    fill_addr;	/* +6320 */
	long	unused18;
        long    va;		/* +6330 */
	long	unused19;
        long    bc_tag;		/* +6340 */
	long	unused20;
	u_long lep_gbus;	/* +6350 */
	long	unused21;
	u_long lep_lmode;	/* +6360 */
	long	unused22;
	u_long lep_lmerr;	/* +6370 */
	long	unused23;
	u_long lep_llock;	/* +6380 */
	long	unused24;
	u_long lep_lber;	/* +6390 */
	long	unused25;
	u_long lep_lcnr;	/* +63A0 */
	long	unused26;
	u_long lep_ldev;	/* +63B0 */
	long	unused27;
	u_long lep_lbesr0;	/* +63C0 */
	long	unused28;
	u_long lep_lbesr1;	/* +63D0 */
	long	unused29;
	u_long lep_lbesr2;	/* +63E0 */
	long	unused30;
	u_long lep_lbesr3;	/* +63F0 */
	long	unused31;
	u_long lep_lbecr0;	/* +6400 */
	long	unused32;
        u_long lep_lbecr1;	/* +6410 */
	long	unused33;
#else /* not PRM_NEW_MCHECK_FRAME */
#define KN7AA_MCHECK_FRAME_PA 0x6010	/* This is setup because the */
					/* RPD doesn't have it, and we */
					/* need it */

 /* these are a part of the console's structure, but preceed the */
 /* pointer address passed to osf from pal */
 /* removed	long	flags; */
 /* removed	long	dasdebug; */

				/* the following offset are describe */
				/* from the base of the console's data */
				/* structure as seen with a 'mchk' cmd */
				/* from the console prompt. Subtract */
				/* 10 for offsets from the base of the */
				/* frame address passed to osf bt the */
				/* pal code */

/*	long	osfmces;	/* +010 */
/*	long	osfvers;	/* +018 */

        long    retry;		/* +020 ; known as byte count to PAL */
				/* <63> - retry; <15:0> byte count */

	int	proc_off;	/* +028 ; these two int's, together */
				/* are known as 'offsets' to the pal */
	int	sys_off;	/* +02C */

        long    pal_temps[32];		/* PAL temporary locations - 30->128		*/

	long	exc_addr;	/* +130 */
        long    exc_sum;	/* +138 */
	long	exc_mask;	/* +140 */
	long	iccsr;		/* +148 */
	long	pal_base;	/* +150 */
	long	hier;		/* +158 */
	long	hirr;		/* +160 */
	long	mm_csr;		/* +168 */
	long	dc_stat;	/* +170 */
	long	dc_addr;	/* +178	*/
	long	abox_ctl;	/* +180	*/
	long	biu_stat;	/* +188 */
	long	biu_addr;	/* +190 */
	long	biu_ctl;	/* +198 */
	long	fill_syndrome;	/* +1A0 */
	long	fill_addr;	/* +1A8	*/
	long	va;		/* +1B0 */
 	long	bc_tag;		/* +1B8 */

 	long	lep_gbus;	/* +1C0	*/
 	int	lep_ldev;	/* +1C8 */
 	int	lep_lber;	/* +1CC */
 	int	lep_lcnr;	/* +1D0	 */
 	int	lep_lmerr;	/* +1D4	 */
 	int	lep_lbesr0;	/* +1D8	 */
 	int	lep_lbesr1;	/* +1DC */
 	int	lep_lbesr2;	/* +1E0	 */
 	int	lep_lbesr3;	/* +1E4 */
 	int	lep_lbecr0;	/* +1E8 */
 	int	lep_lbecr1;	/* +1EC */
 	int	lep_lmode;	/* +1F0	 */
 	int	lep_llock;	/* +1F4	 */

/*
 * End official frame - add IOP for console display only 
 */
	int	iop_lber;	/* +1F8	*/
	int	iop_lbecr0;	/* +1FC	*/
	int	iop_lbecr1;	/* +200	*/
	int	iop_ipcnse;	/* +204	*/
	int	iop_ipchst;	/* +208*/
	int	mem_lber;	/* +20C	*/
	int	mem_lbecr0;	/* +210	*/
	int	mem_lbecr1;	/* +214	*/
	int	mem_erra;	/* +218	*/
	int	mem_errb;	/* +21C */

#endif /* PRM_NEW_MCHECK_FRAME */
};



/*
 * Processor (630) correctable error frame fields
 */
struct kn7aa_proccorr_logout {
        long    retry;		/* +000 ; known as byte count to PAL */
				/* <63> - retry; <15:0> byte count */

	int	proc_off;	/* +008 ; these two int's, together */
				/* are known as 'offsets' to the pal */
	int	sys_off;	/* +00C */
	long	crd_code;
 	long	crd_biu_stat;	/* +010 */
	long	crd_biu_addr;	/* +018		(unlocks biu_stat) */
	long	crd_biu_ctl;	/* +020		(for interest) */
 	long	crd_fill_syndrome;/*+028 */
	long	crd_fill_addr;	/* +030		(unlocks fill syn) */
	long	crd_bc_tag;	/* +038		(one bit at a time!) */
	long	crd_dc_stat;    	/* +040		 */
	long	crd_dc_addr;	/* +048		(unlocks dc_stat) */
};



/*
 * Ruby Machine check handler control structure.  The in_mcheck field is a
 * flag to catch software double machine checks.  The mcheck_ok and
 * badaddr_count fields are used to suppress logging of machine checks
 * during bad address probes.  The lgt field points to a VA which is mapped
 * to the physical address of the logout area.  The lgt_copy field points to
 * a page which is used to buffer copies of logout frames while they are
 * being logged.  The next_copy and copy_in_use[] fields are used to keep
 * track of logout frame copies in the lgt_copy area.  Note that the
 * lgt_copy stuff can go away when we get rid of dump_mcheck_frame.
 */
#define LGT_COPY_SIZE 8192
#define LGT_COPIES 8
 

/* 
 * this is a per-cpu data structure for tracking machine check 
 * error handing
 */

struct kn7aa_error_counters {	/* this is a copy of */
				/* elr_error_counters, so that */
				/* errlog.h doesn't need to be */
				/* included here. */
	char error_counters[96];
};

struct ruby_mcheck_control {
	/* this is a per-cpu data structure for tracking machine check */
	/* error handing */

	/* The following set of structure elements are used for */
	/* intentional machine checks, such as those cause by BADADDR, */
	/* or probing. */
	long mctl_mcheck_ok;		/* flag: we're probing. */
	long mctl_badaddr_count;	/* Number of mchecks during probe. */
	u_long mctl_probe_va;		/* VA being probed. */

	/* This next set of elements are used to describe the machine */
	/* check frame, built by PAL, and used in error parsing. */
        struct ruby_mchk_logout
		*mctl_lgt_va;	/* VA of PAL's mcheck logout area */
	u_long mctl_lgt_pa;		/* PA ... */
	u_long mctl_lgt_len;		/* size of logout area */
	u_long mctl_vector;		/* SCB Vector from PAL. */

	long *mctl_regs;		/* ptr to registers saved by */
					/* _XentInt before calling */
					/* machine check routine. */

	/* The next set of elements are used to maintain a running */
	/* count of error flags found set in the elr_soft_flags */
	/* structure returned from each error parse. These values */
	/* start at 0 at system boot time. */


	struct kn7aa_error_counters error_670_counters;
	struct kn7aa_error_counters error_660_counters;
	struct kn7aa_error_counters error_630_counters;
	struct kn7aa_error_counters error_620_counters;

#if MCHECK_BUFFER
	/* this set is used when there is machine check frame copied */
	/* from the PAL's. */
	struct ruby_mchk_logout
		*mctl_lgt_copy;		/* Buffer to hold copies of logout */
	long mctl_next_copy;		/* Number of next copy */
#endif /* MCHECK_BUFFER */

	/*
	 * Fields for maintaining error logger scratch buffer.
	 */
	struct ruby_mcheck_control_elbuf {
		vm_offset_t elctl_buf;		/* Error logging scratch buffer. */
		u_long 	    elctl_len;		/* Length of scratch buffer. */
		vm_offset_t elctl_ptr;		/* Pointer to first free in */
						/* scratch buffer. */
		u_long      elctl_rmng;		/* Remaining room in scratch buffer. */
	} mctl_elctl;


	struct ruby_error_ctl_frames {
		struct elr_soft_flags *soft_flags; 
						/* ptr to software */
						/* flags portion of */
						/* error log packet */
		/* pointers to allocated space for... */
		struct elr_lep_common_header *lep_cmn_hdr;
		struct elr_lep_670_frame *lep_670_frame;
		struct elr_pal_rev *pal_rev;
		struct elr_dlist *dlist;
		struct elr_lsb *lsb;
		struct elr_lma *lma;
		struct elr_adp *adp;
	} mctl_efctl;

#if MCHECK_BUFFER
	int mctl_copy_in_use[LGT_COPIES];
#endif /* MCHECK_BUFFER */

};

#if NCPUS > 1
extern struct ruby_mcheck_control Ruby_mcheck_control[NCPUS];
#define KN7AA_MCHECK_CONTROL(cpu_id) &Ruby_mcheck_control[cpu_id]
#else /* NCPUS <= 1 */
extern struct ruby_mcheck_control Ruby_mcheck_control;
#define KN7AA_MCHECK_CONTROL(cpu_id) &Ruby_mcheck_control
#endif /* NCPUS <= 1 */


#define RETRY_BIT	0x1000000000000000	/* Retry bit in the top word */
						/* of Mcheck frame	*/

#endif /* __KA_RUBY_H__ */
