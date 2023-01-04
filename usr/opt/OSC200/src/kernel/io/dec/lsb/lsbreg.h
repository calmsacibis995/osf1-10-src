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
/*	"@(#)lsbreg.h	9.3	(ULTRIX/OSF)	10/28/91" */

/************************************************************************
 *									*
 *			Copyright (c) 1991 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any	other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or	reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

/*
 * Revision History: ./dec/io/lsb/lsbreg.h
 *	June-1991 jac:	created this file for the Laser system bus (lsb)
 *
 *
 *************************************************************************/

#ifndef _LSBREG_H_
#define _LSBREG_H_

/* These are the 3 important LSB required registers.
 * The size of this structure is arbitary - but Joe A. has cast
 * iopreg on top of this, so any change in size must
 * be cordinated across all users.
*/
struct lsb_reg {
	unsigned int  	lsb_ldev;	/* Laser device register	*/
	char		lsb_pad0[0x3c];	/* 60 byte pad 04-40		*/
	unsigned int  	lsb_lber;	/* Laser bus error register	*/
	char		lsb_pad1[0x3c];	/* 60 byte pad 44-80		*/
	unsigned int	lsb_lcnr;	/* Laser configuration register	*/
	char		lsb_pad2[0x3c];	/* 60 byte pad	    84-C0	*/
	unsigned int	lsb_eepr_cdr;	/* serial eeprom control/data   */
					/* register                     */
	char		lsb_pad3[0x3c];	/* 60 byte pad      C4-100      */

	char		lsb_rsvd[0x600-0x100];/* reserved     100-600     */

	unsigned int	lsb_lbesr0;	/* Error Syndrome register	*/
	char		lsb_pad4[0x3c];	/* 60 byte pad      604-640     */
	unsigned int	lsb_lbesr1;	/* Error Syndrome register	*/
	char		lsb_pad5[0x3c];	/* 60 byte pad      644-680     */
	unsigned int	lsb_lbesr2;	/* Error Syndrome register	*/
	char		lsb_pad6[0x3c];	/* 60 byte pad      684-6C0     */
	unsigned int	lsb_lbesr3;	/* Error Syndrome register	*/
	char		lsb_pad7[0x3c];	/* 60 byte pad      6C4-700     */
	unsigned int	lsb_lbecr0;	/* Error Command register	*/
	char		lsb_pad8[0x3c];	/* 60 byte pad      704-740     */
	unsigned int	lsb_lbecr1;	/* Error Command register 	*/
	char		lsb_pad9[0x3c];	/* 60 byte pad      744-780     */

/*	char		lsb_pad10[0x6000-0x780]; /* fill 780-6000	*/
	char		lsb_pad10[0x4000-0x780]; /* fill 780-4000	*/
};

#define LSB_REG_SIZE sizeof(struct lsb_reg) /* 16Kb per node		*/

struct lsb_lep_reg {
	unsigned int  	lsb_lep_ldev;	/* Laser device register	*/
	char		lsb_lep_pad0[0x3c];	/* 60 byte pad			*/
	unsigned int  	lsb_lep_lber;	/* Laser bus error register	*/
	char		lsb_lep_pad1[0x3c];	/* 60 byte pad			*/
	unsigned int	lsb_lep_lcnr;	/* Laser configuration register	*/
	char		lsb_lep_pad2[0x3c];	/* 60 byte pad	    84-C0	*/
	unsigned int	lsb_lep_eepr_cdr;	/* serial eeprom control/data   */
					/* register                     */
	char		lsb_lep_pad3[0x3c];	/* 60 byte pad      C4-100      */

	char		lsb_lep_rsvd[0x600-0x100];/* reserved      100-600     */

	unsigned int	lsb_lep_lbesr0;	/* Error Syndrome register	*/
	char		lsb_lep_pad4[0x3c];	/* 60 byte pad      604-640     */
	unsigned int	lsb_lep_lbesr1;	/* Error Syndrome register	*/
	char		lsb_lep_pad5[0x3c];	/* 60 byte pad      644-680     */
	unsigned int	lsb_lep_lbesr2;	/* Error Syndrome register	*/
	char		lsb_lep_pad6[0x3c];	/* 60 byte pad      684-6C0     */
	unsigned int	lsb_lep_lbesr3;	/* Error Syndrome register	*/
	char		lsb_lep_pad7[0x3c];	/* 60 byte pad      6C4-700     */
	unsigned int	lsb_lep_lbecr0;	/* Error Command register	*/
	char		lsb_lep_pad8[0x3c];	/* 60 byte pad      704-740     */
	unsigned int	lsb_lep_lbecr1;	/* Error Command register 	*/
	char		lsb_lep_pad9[0x3c];	/* 60 byte pad      744-780     */

	char		lsb_lep_pad10[0xc00-0x780]; /* fill 780-c00	*/

	unsigned int	lsb_lep_lmode;
	char		lsb_lep_pad11[0x3c];	/* 60 byte pad      C04-C40     */
	unsigned int	lsb_lep_lmerr;

/*	char		lsb_lep_pad12[0x6000-0xC44]; /* fill C44-6000	*/
	char		lsb_lep_pad12[0x4000-0xC44]; /* fill C44-4000	*/
};

#define LSB_LEP_REG_SIZE sizeof(struct lsb_lep_reg) /* 16Kb per node		*/

struct lsb_lma_reg {
	unsigned int  	lma_ldev;	/* Laser device register	*/
	char		lma_pad0[0x3c];	/* 60 byte pad	    04-40	*/
	unsigned int  	lma_lber;	/* Laser bus error register	*/
	char		lma_pad1[0x3c];	/* 60 byte pad	    44-80	*/
	unsigned int	lma_lcnr;	/* Laser configuration register	*/
	char		lma_pad2[0x3c];	/* 60 byte pad	    84-C0	*/
	unsigned int	lma_eepr_cdr;	/* serial eeprom control/data   */
					/* register                     */
	char		lma_pad3[0x3c];	/* 60 byte pad      C4-100      */

	char		lma_rsvd[0x600-0x100];/* reserved   100-600     */

	unsigned int	lma_lbesr0;	/* Error Syndrome register	*/
	char		lma_pad4[0x3c];	/* 60 byte pad      604-640     */
	unsigned int	lma_lbesr1;	/* Error Syndrome register	*/
	char		lma_pad5[0x3c];	/* 60 byte pad      644-680     */
	unsigned int	lma_lbesr2;	/* Error Syndrome register	*/
	char		lma_pad6[0x3c];	/* 60 byte pad      684-6C0     */
	unsigned int	lma_lbesr3;	/* Error Syndrome register	*/
	char		lma_pad7[0x3c];	/* 60 byte pad      6C4-700     */
	unsigned int	lma_lbecr0;	/* Error Command register	*/
	char		lma_pad8[0x3c];	/* 60 byte pad      704-740     */
	unsigned int	lma_lbecr1;	/* Error Command register 	*/
	char		lma_pad9[0x3c];	/* 60 byte pad      744-780     */

	char		lma_fill1[0x2000-0x780]; /* fill    780-2000    */

	unsigned int	lma_mcr;	/* Memory Config. register      */
	char		lma_pad10[0x3c];/* 60 byte pad      2004-2040   */
	unsigned int	lma_amr;	/* Address Mapping Register	*/
	char		lma_pad11[0x3c];/* 60 byte pad      2044-2080   */
      	unsigned int	lma_mstr0;	/* Memory Selftest Register 0	*/
	char		lma_pad12[0x3c];/* 60 byte pad      2084-20C0   */
	unsigned int	lma_mstr1;	/* Memory Selftest Register 1	*/
	char		lma_pad13[0x3c];/* 60 byte pad      20C4-2100   */
	unsigned int	lma_fadr;	/* Failing Address Register	*/
	char		lma_pad14[0x3c];/* 60 byte pad      2104-2140   */
	unsigned int	lma_mera;	/* Memory Error Reg. A		*/
	char		lma_pad15[0x3c];/* 60 byte pad      2144-2180   */
	unsigned int	lma_msynda;	/* Memory Syndrome Reg. A	*/
	char		lma_pad16[0x3c];/* 60 byte pad      2184-21C0   */
	unsigned int	lma_mdra;	/* Memory Diagnostic Reg. A	*/
	char		lma_pad17[0x3c];/* 60 byte pad      21C4-2200   */
	unsigned int	lma_mcbsa;	/* Mem. Chk. Bit Subst. A	*/
	char		lma_pad18[0x3c];/* 60 byte pad      2204-2240   */

/*	char 		lma_fill2[0x4140-0x2240]; /* fill   2240-4140	*/
	char 		lma_fill2[0x4000-0x2240]; /* fill   2240-4000	*/


	char		lma_fil2a[0x4140-0x4000];  /* fill 4000-4140 */
	unsigned int	lma_merb;	/* Memory Error Reg. B		*/
	char		lma_pad19[0x3c];/* 60 byte pad     4144-4180   */
	unsigned int	lma_msyndb;	/* Memory Syndrome Reg. B	*/
	char		lma_pad20[0x3c];/* 60 byte pad     4184-41C0   */
	unsigned int	lma_mdrb;	/* Memory Diagnostic Reg. B	*/
	char		lma_pad21[0x3c];/* 60 byte pad     41C4-4200   */
	unsigned int	lma_mcbsb;	/* Mem. Chk. Bit Subst. B	*/
	char		lma_pad22[0x3c];/* 60 byte pad     4204-4240   */

	char		lma_fill3[0x6000-0x4240]; /* fill page 4240-6000    */
};

#define LSB_LMA_REG_SIZE sizeof(struct lsb_lma_reg) /* 16Kb per node	*/

#ifdef	KERNEL
extern struct lsb_reg lsbnode[];
#endif /*  KERNEL */

/* IOP Node address is absolute for RUBY */
#define LSB_IOP_NODE 8

/* LSB flags (taken from xmireg...)	*/
#define LSBF_SST 0x1		/* do node reset before call init	*/
#define LSBF_DEVICE 0x4		/* is a device in the config file	*/
#define LSBF_CONTROLLER 0x8	/* is a controller in config file	*/
#define LSBF_ADAPTER  0x10	/* adapters...uba's etc			*/
#define LSBF_NOCONF 0x1000	/* Isn't config'd			*/

/* LSB device register */

#define	LSB_IOP_LDEV	0x00002000	/* LSB IOP device */
#define	LSBLDEV_TYPE	0x0000ffff	/* LSB device type field */
#define	LSBLDEV_REV	0xffff0000	/* LSB device revision field */

/* LSB device types defined */

#define	LSB_LEP		0x8001		/* Ruby processor		*/
#define	LSB_IOP		0x2000		/* Laser I/O module		*/
#define	LSB_MEM		0x4000		/* Laser memory module		*/
#define	LSB_BBMEM	0x4002		/* Laser Battery backed up memory*/

/*
 * Laser System Bus error definitions (LBER)
 * (these lousy abbrevations are what is used in the LSB spec...
*/
#define LSB_E		0x00000001	/* Error line is asserted	*/
#define LSB_UCE		0x00000002	/* Uncorrectable data error	*/
#define LSB_UCE2	0x00000004	/* 2nd uncorrectable data error	*/
#define LSB_CE		0x00000008	/* Correctable data error	*/
#define LSB_CE2		0x00000010	/* 2nd correctable data error	*/
#define LSB_CPE		0x00000020	/* Command parity error		*/
#define LSB_CPE2	0x00000040	/* 2nd command parity error	*/
#define LSB_CDPE	0x00000080	/* CSR data parity error	*/
#define LSB_CDPE2	0x00000100	/* 2nd CSR data parity error	*/
#define LSB_TDE		0x00000200	/* transmitter during error (?)	*/
#define LSB_STE		0x00000400	/* STALL error			*/
#define LSB_CNFE	0x00000800	/* CNF error			*/
#define LSB_NXAE	0x00001000	/* Non-existent address error	*/
#define LSB_CAE		0x00002000	/* CA error			*/
#define LSB_SHE		0x00004000	/* SHARED error			*/
#define LSB_DIE		0x00008000	/* DIRTY error			*/
#define LSB_DTCE	0x00010000	/* Data transmit check error	*/
#define LSB_CTCE	0x00020000	/* Control transmit check error	*/
#define LSB_NSES	0x00040000	/* Node-specific error summary	*/

/*
 * Laser System Bus configuration register definitions
*/

#define LSB_STF		0x80000000	/* Self test failed		*/
#define LSB_NRST	0x40000000	/* Node reset			*/
#define LSB_NHALT	0x20000000	/* Node halt			*/
#define LSB_RSTSTAT	0x10000000	/* Reset status			*/

/*
 * Laser Information Base Repair register (IBR)
*/
#define LSB_SCLK	0x000000004	/* Serial clock bit		*/
#define LSB_XMT_SDAT	0x000000002	/* Serial data transmit bit	*/
#define LSB_RCV_SDAT	0x000000001	/* Serial data receive bit	*/
#define LSB_EEPROM_SREAD 0x0000000A0	/* EEPROM slave read mask	*/
#define LSB_EEPROM_SWRITE 0x0000000A1	/* EEPROM slave write mask	*/

#define LSB_EEPROM_WRITE 0
#define LSB_EEPROM_READ 1

/*
 * Presto defines for lsb
*/
#define LSB_EEPROM_PRESTO	0x7EC	/* adr of status location in eeprom */
#define RUBY_CACHE_SIZE	0x400000L	/* Used to force bcach evictions */
#define MS700_NVRAM_SIZE 0x01000000	/* The MS7bb module is 16 MB	*/

#define LSB_PRESTO_BAT_OK	0x02
#define LSB_PRESTO_CHARGING	0x04
#define LSB_PRESTO_VBB_RESET	0x08
#define LSB_PRESTO_INIT		0x10
#define LSB_PRESTO_BAT_CONN	0x40	/* The manually operated battery
					 * switch is turned on		*/
/*
 * Number of uS to wait between wiggleing the EEPROM bits...
*/
#define LSB_EEPROM_BITWAIT 5	/* long wait time (stolen from console)	*/
#define LSB_EEPROM_SBITWAIT 1	/* short wait time (dido)		*/

struct lsbsw {

	int	lsb_ldev;		/* LSB device type */
	char	*lsb_name;		/* name of the device*/
	int	(**probes)();		/* funtions to probe at boot time */
	int	(*lsb_reset)();		/* reset routine for device */
	short	lsb_flags;		
};

/*
 * lsbdata is a structure modeled after the bsd xmidata.
*/

#define MAX_LSB_NODE	9

struct lsbdata {
	struct lsbdata *next;		/* points to the next lsbdata. For */
					/* the forseeable future there is  */
					/* only one			*/
	int lsbnum;			/* logical # of lsb, starts at 0*/
	struct lsb_reg *lsbvirt[MAX_LSB_NODE]; /* pointers to lsb slots */
	struct lsb_reg *lsbphys;	/* not used in alpha		*/
	struct lsb_reg *cpu_lsb_addr; 	/* the virtual base addr for the slot*/
					/* the cpu is in		*/
	int (**lsbvec_page)();
	int lsbnodes_alive;
	int lsbintr_dst;		/* 'master' CPU that gets interrupts */
					/* This is a bit mask corresponding  */
					/* to slot number.		*/
	int lsb_err_cnt;		
	unsigned int lsbilast_err_time;
	struct {
		struct lsbsw *plsbsw;
		int lsberr;
		int lsberr1;
	} lsberr[9];
};

extern struct lsbdata *head_lsbdata;
extern struct lsbdata *get_lsb();

#ifdef	__alpha
/*
 * The following node size was 17K in the XMI (because of CIXCD).
 * The LSB spec says LSB CSR space is 64K...
*/
#define LSBNODE_SIZE	65536

/* The total LSB bus size is just LSBNODE_SIZE * number of slots (9)	*/
#define LSBBUS_SIZE	(LSBNODE_SIZE *  MAX_LSB_NODE)

/*
 * The LSB physical starting address is processor specific...
*/
#ifdef DEC7000
#define LSB_START_PHYS 0x00000003f8000000
#endif /* DEC7000 */

#endif /* __alpha */

#ifdef __mips
#define LSBNODE_SIZE	4096	/* Not used - and not mapped on mips */
#endif /* __mips */


#define SCB_LSB_LWOFFSET(lsb_nodenum,level) \
	((lsb_nodenum << 2) | level)

#define SCB_LSB_VEC_ADDR(lsbdata,lsbnumber,lsb_nodenum,level) \
	((lsbdata->lsbvec_page)+(((lsb_nodenum << 2) | level)/4))

#define SCB_LSB_ADDR(lsbdata) \
	((lsbdata->lsbvec_page))


/*
 * Define LSB register bits to be used in error parsing machine
 * checks 
 */
#define LBER_E		( 1 << 0x00 )
#define LBER_UCE	( 1 << 0x01 )
#define LBER_UCE2	( 1 << 0x02 )
#define LBER_CE		( 1 << 0x03 )
#define LBER_CE2	( 1 << 0x04 )
#define LBER_CPE	( 1 << 0x05 )
#define LBER_CPE2	( 1 << 0x06 )
#define LBER_CDPE	( 1 << 0x07 )
#define LBER_CDPE2	( 1 << 0x08 )
#define LBER_TDE	( 1 << 0x09 )
#define LBER_STE	( 1 << 0x0A )
#define LBER_CNFE	( 1 << 0x0B )
#define LBER_NXAE	( 1 << 0x0C )
#define LBER_CAE	( 1 << 0x0D )
#define LBER_SHE	( 1 << 0x0E )
#define LBER_DIE	( 1 << 0x0F )
#define LBER_DTCE	( 1 << 0x10 )
#define LBER_CTCE	( 1 << 0x11 )
#define LBER_NSES	( 1 << 0x12 )


#define LBECR0_CA 	0xFFFFFFFF
#define LBECR1_CA 	0x0000007F
#define LBECR1_CA_CMD 	0x00000038
/* moved - #define LBECR1_CID	0x00000780 */
/* defunct - #define LBECR1_RID	0x00007800 */
#define LBECR1_CID	0x00007800
#define LBECR1_CNF	0x00008000
#define LBECR1_SHARED	0x00010000
#define LBECR1_DIRTY	0x00020000
#define LBECR1_DCYCLE	0x000C0000

/* moved - #define LBECR1_CID_SHIFT	0x07 */
#define LBECR1_CID_SHIFT	0x0B

#define LBECR1_CA_CMD_SHIFT	0x03

#define LBECR1_CA_READ		(0x00 << LBECR1_CA_CMD_SHIFT)
#define LBECR1_CA_WRITE		(0x01 << LBECR1_CA_CMD_SHIFT)
#define LBECR1_CA_RSVD1		(0x02 << LBECR1_CA_CMD_SHIFT)
#define LBECR1_CA_WRITE_VICTIM	(0x03 << LBECR1_CA_CMD_SHIFT)
#define LBECR1_CA_READ_CSR	(0x04 << LBECR1_CA_CMD_SHIFT)
#define LBECR1_CA_WRITE_CSR	(0x05 << LBECR1_CA_CMD_SHIFT)
#define LBECR1_CA_RSVD2		(0x06 << LBECR1_CA_CMD_SHIFT)
#define LBECR1_CA_PRIVATE	(0x07 << LBECR1_CA_CMD_SHIFT)



#define LMERR_PMAPPE	0x000F
#define LMERR_BTAGPE	0x0010
#define LMERR_BSTATPE	0x0020
#define LMERR_BMAPPE	0x0040
#define LMERR_BDATASBE	0x0080
#define LMERR_BDATADBE	0x0100
#define LMERR_ARBCOL	0x0200
#define LMERR_ARBDROP	0x0400
#define LMERR_EDALTO	0x0800


#endif
