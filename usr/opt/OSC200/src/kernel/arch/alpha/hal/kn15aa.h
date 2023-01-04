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
 * Modification History: machine/alpha/kn15aa.h
 *
 * 12-Sep-91 -- ald
 *	Created this file for Alpha FLAMINGO support.
 */

#ifndef _KN15AA_H_
#define _KN15AA_H_


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
 * Defines that refer to specific areas of the logout area.
 * For all of the logout formats the retry bit is bit number 63.
 * The low 4 bytes of the first quadword specify the number of bytes in the
 * loguot area.
 */
#define RETRY_BIT	0x1000000000000000
#define BCOUNT_MASK	0xFFFF


/*
 * TURBOchannel information. Much of this is modelled after mips/kn02.c
 */

#define KN15AA_SP_TC_CLK_SPEED	222	/* 22.2 MHz */
#define KN15AA_FL_TC_CLK_SPEED	250	/* 25 MHz */

/* NOTE: These are 34-bit "sparse" space physical addresses */
/* It is assumed that registers are lw/byte, on lw addresses */
#define SLOT_0_ADDR 0x110000000
#define SLOT_1_ADDR 0x130000000
#define SLOT_2_ADDR 0x150000000
#define SLOT_3_ADDR 0x170000000
#define SLOT_4_ADDR 0x190000000
#define SLOT_5_ADDR 0x1b0000000
#define SLOT_6_ADDR 0x1d0000000
#define SLOT_7_ADDR 0x1f0000000
#define IOASIC_ADDR SLOT_7_ADDR	/* slot 7 is really the core I/O ASIC */

/* Subtract offset to convert between "dense" and "sparse" PADDRs */
#define DENSE(a) ((vm_offset_t)(a) - 0x10000000)

/* Offset between slot PADDRs */
#define OFFSET	0x20000000

/*
 * Real-time clock stuff
 */
struct kn15aa_rtc {
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


/* The following defines give the special regions to map in sparse */
/* I/O space at system startup. They are: system ROM, TC register set, */
/* 2 SCC chips, Color Frame Buffer, and SCSI control chips. */
/* NOTE: Look into optimizing these sizes; most are only a few words! */

/* System ROM is at the beginning of the IO ASIC, is 256 KB long */
#define TC_ROM_ADDR	(IOASIC_ADDR)
#define TC_ROM_SIZE	(256*1024)

/* IOCTL registers are after System ROM, are 256 KB long */
#define	TC_REG_ADDR	(IOASIC_ADDR + 0x80000)
#define TC_REG_SIZE	(256*1024)

/* SCC 0 is after ROM, registers, E-net ROM, and Lance interface */
#define TC_SCC0_ADDR	(IOASIC_ADDR + 0x200000)
#define TC_SCC0_SIZE	(8*1024)

/* SCC 1 is after SCC 0 and a reserved area; both are 8 KB long */
#define TC_SCC1_ADDR	(IOASIC_ADDR + 0x300000)
#define TC_SCC1_SIZE	(8*1024)

/* SCSI chips are in slot 6 (skip FEPROM, map only internal registers) */
#define TC_SCSI_ADDR	(SLOT_6_ADDR)
#define TC_SCSI_SIZE	(1032*1024)

/* CFB is after the Core I/O ASIC in slot 7; 64 MB wide */
/* For now send the SFB driver a dense space address */
#define TC_CFB_ADDR	0x1e2000000
#define TC_CFB_SIZE	(8*1024*1024)	/* Should be the same as option slot */

#define TC_ISDN_ADDR	(IOASIC_ADDR + 0x480000)
#define TC_ISDN_SIZE	(16*1024)

#define TC_TOY_ADDR	(IOASIC_ADDR + 0x400000)
#define TC_TOY_SIZE	(8*1024)

#define TC_LANCE_ADDR	(IOASIC_ADDR)	/* Offset specified in if_ln_data.c */
#define TC_LANCE_SIZE	(32*1024)

/* The first 6 slots are user option slots, how much to map ??? */
/* Must map at least 8MB with sparse space; ROM is @ 3c0000 * 2 */
#define TC_OPTION_SIZE	(8*1024*1024)

/* Some important system registers; physical dense space addresses! */
#define KN15AA_IOSLOT	0x1c2000000
#define KN15AA_IR	0x1d4800000	/* SPARSE! No clear of bits 31:21 */
#define KN15AA_IMR_WR	0x1c281fffc	/* Write IMR: dense addr of last S/G map entry */
#define KN15AA_IMR_RD	0x1c2400000	/* Read IMR: dense addr of IR */
#define KN15AA_SSR	0x1e0040100
#define KN15AA_SIR	0x1e0040110
#define KN15AA_SIMR	0x1e0040120

/* KN15AA_IR register values */
#define TCIR_MASK	0xffe001ff	/* Mask to keep only useful IR bits */
#define TCIR_TO 	0x08000000	/* TIMEOUT bit on TURBOchannel IR */

/* KN15AA_IMR bits */
#define KN15AA_IMR_MASK 0x3f		/* Mask to keep only useful IMR bits */

/* KN15AA_IOSLOT register values */
#define TCIO_SGMAP	0x1		/* Bit in 3-bit slot mask for scatter/gather map */
#define TCIO_BLOCK	0x2		/* Bit in 3-bit slot mask for block mode */
#define TCIO_PARITY	0x4		/* Bit in 3-bit slot mask for parity */
#define TCIO_VALID	0x80000000	/* TCIO_BYTEMASK is valid */
#define TCIO_BYTEMASK	0x78000000	/* 4-bit byte mask for I/O reads */


extern unsigned int kn15aa_read_ir();
extern unsigned int kn15aa_read_imr();
extern void kn15aa_write_imr();
extern unsigned int kn15aa_read_ssr();
extern void kn15aa_write_ssr();
extern unsigned int kn15aa_read_sir();
extern void kn15aa_write_sir();
extern unsigned int kn15aa_read_simr();
extern void kn15aa_write_simr();
extern void kn15aa_write_leds();
extern int kn15aa_enable_option();
extern int kn15aa_disable_option();
extern int kn15aa_clear_errors();
extern void kn15aa_set_ioslot();
extern void kn15aa_write_ioslot();
extern unsigned int kn15aa_read_ioslot();
extern void kn15aa_dispatch_iointr();
extern int kn15aa_option_control();





/*
 * kn15aa read/write_io_port support.
 */
#define KN15_DENSE_ADDRESS_MASK 0x3ffffff
#define KN15_DENSE_SPACE 0x10000000
#define KN15_IOHANDLE_MASK 0xfffffffff8000000
#define MASK_LONG_ADDRESS  0xfffffffffffffffc
#define KN15_MASK_REGISTER_ADDRESS 0xfffffc01c2000020
#define BYTE_ACCESS 1
#define WORD_ACCESS 2
#define LONG_ACCESS 4
#define KN15_READ_W_MASK0   0xe0000000
#define KN15_READ_W_MASK1   0x98000000
#define KN15_READ_B_MASK0  0xf0000000
#define KN15_READ_B_MASK1  0xe8000000
#define KN15_READ_B_MASK2  0xd8000000
#define KN15_READ_B_MASK3  0xb8000000
#define KN15_CLEAR_MASK   0
#define BUILD_RETURN_STRUCTURE(x,y,z)     \
          ((x >> (y <<3)) & ((1 << (z << 3)) -1))
#define IOA_OKAY        0
#define IOA_ERROR       -1
#define KN15_WRITE_LW_MASK  0x0000000f00000000
#define KN15_WRITE_W_MASK   0x0000000300000000
#define KN15_WRITE_B_MASK   0x0000000100000000


/* 
 * kn15aa io_copyin() io_copyout() and io_copyio() support
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


struct ONE_INT   { unsigned int IW;
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
