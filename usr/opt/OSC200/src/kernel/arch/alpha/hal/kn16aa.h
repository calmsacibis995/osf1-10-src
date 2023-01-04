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
 * @(#)$RCSfile: kn16aa.h,v $ $Revision: 1.1.7.6 $ (DEC) $Date: 1993/11/22 23:17:09 $
 */

#ifndef _KN16AA_H_
#define _KN16AA_H_
/*
 * Some useful SCB offset values (architected in SRM).
 * These should probably be in some alpha include file.
 */
#define SYS_CORR_ERR	0x620	/* System correctable error */
#define PROC_CORR_ERR	0x630	/* Processor correctable error */
#define SYS_MCHECK	0x660	/* System machine check */
#define PROC_MCHECK	0x670	/* Processor machine check */
#define IO_INTERRUPT	0x800	/* I/O Device interrupt */

/*
 * Values for the "ident" field in the machine check logout frame
 */
#define ICACHE_DATA_PE	0x01
#define ICACHE_TAG_PE	0x02
#define DCACHE_DATA_PE	0x03
#define DCACHE_TAG_PE	0x04
/* 5 would be external HALT, but is not */
#define BCACHE_TPE_RD	0x06
#define BCACHE_TPE_WR	0x07
#define TC_DMA_PE	0x08
#define SG_DMA_PE_RD	0x09
#define SG_DMA_PE_WR	0x0a
#define SG_INVALID_RD	0x0b
#define SG_INVALID_WR	0x0c
#define TC_IOREAD_PE	0x0d
#define TC_INVALID_RD	0x0e
#define TC_INVALID_WR	0x0f
#define DMA_BUSRT_2BIG	0x10
#define DMA_X2KB	0x11
#define DMA_RD_ECC	0x12
#define IO_WR_ECC	0x13
#define TC_TIMEOUT_RD	0x14
#define TC_TIMEOUT_WR	0x15
#define DMA_OVERFLOW_RD	0x16
#define CORR_DMA_ECC_RD	0x17
#define DMA_OVERFLOW_WR	0x18
#define CPU_ECC_RD	0x19
#define BCACHE_TAG_PE_CPU	0x1a
#define BCACHE_CTL_PE	0x1b
#define BCACHE_TAG_PE_EXT	0x1c

/*
 * TURBOchannel information. Much of this is modelled after mips/kn02.c
 */

#define	KN16AA_TC_CLK_SPEED	125	/* 12.5 MHz */

/* NOTE: These are 34-bit "sparse" space physical addresses */
/* It is assumed that registers are lw/byte, on lw addresses */
#define KN16AA_SLOT_0_ADDR 0x110000000
#define KN16AA_SLOT_1_ADDR 0x130000000
#define SLOT_2_ADDR 0x150000000
#define SLOT_3_ADDR 0x170000000
#define SLOT_4_ADDR 0x190000000
#define SLOT_5_ADDR 0x1b0000000
#define SLOT_6_ADDR 0x1d0000000
#define SLOT_7_ADDR 0x1f0000000
#define IOASIC_ADDR SLOT_5_ADDR	/* slot 7 is really the core I/O ASIC */

/* Subtract offset to convert between "dense" and "sparse" PADDRs */
#define DENSE(a) ((vm_offset_t)(a) - 0x10000000)

/* Offset between slot PADDRs */
#define OFFSET	0x20000000

/*
 * Real-time clock stuff
 */
struct kn16aa_rtc {
	char	seconds;
	char	seconds_pad[7];
	char	seconds_alarm;
	char	seconds_alarm_pad[7];
	char	minutes;
	char	minutes_pad[7];
	char	minutes_alarm;
	char	minutes_alarm_pad[7];
	char	hours;
	char	hours_pad[7];
	char	hours_alarm;
	char	hours_alarm_pad[7];
	char	dow;
	char	dow_pad[7];
	char	dom;
	char	dom_pad[7];
	char	month;
	char	month_pad[7];
	char	year;
	char	year_pad[7];
	char	rega;
	char	rega_pad[7];
	char	regb;
	char	regb_pad[7];
	char	regc;
	char	regc_pad[7];
	char	regd;
	char	regd_pad[7];
	char	bbu_ram_base[50];	/* ??? is this right ??? */
};

/* TOY chip register bit definitions */
#define TOY_REGA_UIP	(u_char)0x80
#define TOY_REGB_DM	(u_char)0x04
#define TOY_REGB_24	(u_char)0x02


/* SCC 0 is after ROM, registers, E-net ROM, and Lance interface */
/* SCC 1 is after SCC 0 and a reserved area; both are 8 KB long */
#define KN16AA_SCC_ADDR	(IOASIC_ADDR + 0x200000)

#define KN16AA_SCSI_ADDR	(SLOT_4_ADDR)

#define KN16AA_CFB_ADDR	0x1c0000000

#define KN16AA_ISDN_ADDR	(IOASIC_ADDR + 0x480000)

#define KN16AA_TOY_ADDR	(IOASIC_ADDR + 0x400000)

#define KN16AA_LANCE_ADDR (IOASIC_ADDR)	/* Offset specified in if_ln_data.c */

/* Some important system registers; physical dense space addresses! */
#define KN16AA_IR	0x1e0000000	/* No clear of bits 31:21 */
#define KN16AA_SSR	0x1a0040100
#define KN16AA_SIR	0x1a0040110
#define KN16AA_SIMR	0x1a0040120

#define KN16AA_SCSI_INDEX	6
#define KN16AA_LN_INDEX		7
#define KN16AA_SCC_INDEX	8
#define KN16AA_ISDN_INDEX	9
#define KN16AA_CFB_INDEX	10

#define KN16AA_TC_0_INTR	0x4
#define KN16AA_TC_1_INTR	0x8

#define TCIR_MASK	0xf800001f	/* Mask to keep only useful IR bits */
#define TCIR_TO 	0x08000000	/* TIMEOUT bit on TURBOchannel IR */

extern unsigned int kn16aa_read_ir();
extern unsigned int kn16aa_read_ssr();
extern void kn16aa_write_ssr();
extern unsigned int kn16aa_read_sir();
extern void kn16aa_write_sir();
extern unsigned int kn16aa_read_simr();
extern void kn16aa_write_simr();
extern int kn16aa_enable_option();
extern int kn16aa_disable_option();
extern int kn16aa_clear_errors();
extern void kn16aa_dispatch_iointr();
extern int kn16aa_option_control();
/*
 * kn16aa read/write_io_port support.
 */
#define KN16_DENSE_ADDRESS_MASK 0x3ffffff
#define KN16_DENSE_SPACE 0x10000000
#define KN16_IOHANDLE_MASK 0xfffffffff8000000
#define MASK_LONG_ADDRESS  0xfffffffffffffffc
#define KN16_MASK_REGISTER_ADDRESS 0xfffffc01a0040100
#define BYTE_ACCESS 1
#define WORD_ACCESS 2
#define LONG_ACCESS 4
#define KN15_CLEAR_MASK   0
#define BUILD_RETURN_STRUCTURE(x,y,z)     \
          ((x >> (y <<3)) & ((1 << (z << 3)) -1))
#define IOA_OKAY        0
#define IOA_ERROR       -1
#define KN16_WRITE_LW_MASK  0x0000000f00000000
#define KN16_WRITE_W_MASK   0x0000000300000000
#define KN16_WRITE_B_MASK   0x0000000100000000
#define KN16_READ_W_MASK   0x00000003
#define KN16_READ_B_MASK   0x00000001
#define KN16_CLEAR_MASK    0xffffffffffffffe0
#define KN16_READ_W_MASK0   0x13  
#define KN16_READ_W_MASK1   0x1C  
#define KN16_READ_B_MASK0  0x11
#define KN16_READ_B_MASK1  0x12
#define KN16_READ_B_MASK2  0x14
#define KN16_READ_B_MASK3  0x18



/*
 * kn16aa io_copyin() io_copyout() and io_copyio() support
 */

#define ALIGNMENT_MASK   0x3
#define LONGWORD_ALIGNMENT_MASK   0xfffffffffffffffC
#define ALIGNED          0x0
#define BYTE_OFFSET      0x1
#define WORD_OFFSET      0x2
#define TRIBYTE_OFFSET   0x3
#define BYTE_REMAINDER   0x01
#define WORD_REMAINDER   0x02
#define TRIBYTE_REMAINDER 0x03
#define TC_BYTE_REMAINDER_MASK_CHAR1    0x0000000100000000
#define TC_BYTE_REMAINDER_MASK_CHAR2    0x0000000200000000
#define TC_BYTE_REMAINDER_MASK_CHAR3    0x0000000400000000
#define TC_BYTE_REMAINDER_MASK_CHAR4    0x0000000800000000
#define TC_WORD_REMAINDER_MASK_WORD1    0x0000000300000000
#define TC_WORD_REMAINDER_MASK_WORD2    0x0000000C00000000
#define TC_TRIBYTE_REMAINDER_MASK_HIGH  0x0000000E00000000
#define TC_TRIBYTE_REMAINDER_MASK_LOW   0x0000000700000000
#define TC_LONGWORD 0x4


struct ONE_INT   { 
           unsigned int IW;
           unsigned int PAD; };

struct ALL_BYTES {
              unsigned char CHAR1;
              unsigned char CHAR2;
              unsigned char CHAR3;
              unsigned char CHAR4;
              unsigned int  PAD; };

struct ALL_WORDS {
             unsigned short WORD1;
             unsigned short WORD2;
             unsigned int PAD;  };

struct WORD_and_BYTES {
                  unsigned short WORD1;
                  unsigned char CHAR1;
                  unsigned char CHAR2;
                  unsigned int PAD;  };

union buffer {
        unsigned long LW;
        struct ONE_INT INT;
        struct ALL_BYTES BYTES;
        struct ALL_WORDS WORDS;
        struct WORD_and_BYTES WORD_BYTES;
        };



#endif
