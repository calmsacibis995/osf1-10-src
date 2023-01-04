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
 * @(#)$RCSfile: eisa.h,v $ $Revision: 1.1.6.7 $ (DEC) $Date: 1993/12/17 20:56:05 $
 */

/***************************************************************************/
/*                                                                         */
/* MODULE NAME: eisa.h							   */
/* 									   */ 
/* LOCATION:	.../src/kernel/io/dec/eisa				   */
/* 									   */ 
/* DESCRIPTION:								   */
/* 		Contains definitions for EISA bus support.		   */
/* 									   */ 
/* STRUCTURES:								   */
/* 									   */
/*	eisa_option	Defines eisa option table entry.		   */
/*	eisa_slot	Defines eisa slot table entry.			   */
/*	eisa_sw		Eisa switch table for platform dependent info.	   */
/* 									   */ 
/***************************************************************************/

#include	<sys/types.h>
/*====================================================================*/
/*	This header file contains definitions for the EISA bus.	      */
/*====================================================================*/

#ifndef	__EISA_H__
#define	__EISA_H__

#define EISA_SLOT_ADDR_SHIFT	12	/* Shifting slot number by this    */
					/* yields base of slot specific io */
					/* address range for this slot.    */

#define MAX_EISA_SLOTS	16	/* Maximum number of slots EISA supports */

#define EISA_ID_OFFSET	0x0c80	/* Offset in slot specific space of ID. */
#define EISA_ID_PRECHARGE 0xff	/* Have to write this to 1st id byte prior */
				/* to reading id.			   */
#define EISA_BCR_OFFSET	0X0C84	/* Offset in slot specific space of CSR */
#define	EISA_BOARD_RESET 0x04	/* Writing this to EISA_BCR_OFFSET resets */
                                /* and disables option.			  */
#define	EISA_BOARD_ENABLE 0x01	/* Writing this to EISA_BCR_OFFSET enables */
                                /* the board.				   */
#define EISA_RESET_REG	0x461	/* Setting bit 0 of this register to a one */
				/* asserts RESDRV onto the EISA bus thereby */
				/* forcing all options (ISA included) to    */
				/* perform a reset.			    */


#define EISA_IDNAMELEN	7	/* Length of EISA Product ID. */
#define	EISA_NAMELEN	8	/* Length of driver, ctrl etc names. */
#define EISA_FUNCLEN	79	/* Length of function string. */

#define QUIET	0
#define VERBOSE	1

/*---------------------------------------------------------*/
/* The following define the class attribute of the module. */
/*---------------------------------------------------------*/
#define	EISA_CTLR	1
#define	EISA_DEV	2
#define	EISA_ADPT	3
#define	EISA_UNKNOWN	4

/*----------------------------*/
/* EISA data sizes and masks. */
/*----------------------------*/
#define	EISA_B1		1
#define	EISA_B2		2
#define	EISA_B4		4
#define	EISA_B5		5
#define	EISA_B6		6
#define	EISA_B8		8
#define	EISA_BYTE	0x0	/*  8 bits */
#define EISA_WORD	0x1	/* 16 bits */
#define EISA_DWORD	0x2	/* 32 bits */

#define	EISA_B1_MASK	0x1
#define	EISA_B2_MASK	0x3
#define	EISA_B4_MASK	0xF
#define	EISA_B5_MASK	0x1F
#define	EISA_B6_MASK	0x3F
#define	EISA_BYTE_MASK	0xff
#define EISA_WORD_MASK	0xffff
#define EISA_DWORD_MASK	0xffffffff
#define	EISA_B16	16

/*--------------------*/
/* Misc. EISA defines */
/*--------------------*/
#define	EISA_NO_CONFIG	0xff	/* Data loaded into config structure when  */
				/* there is no data in config, e.g. no IRQ */

/*-------------------------------------*/
/* Defines used with eisa_ get_config. */
/*-------------------------------------*/
#define	EISA_MEM	0
#define	EISA_IRQ	1
#define	EISA_DMA	2
#define	EISA_PORT	3

/*-----------------------------------*/
/* EISA DMA size and timing defines. */
/*-----------------------------------*/
#define BITS_8		0x0	/*  8 bit, count by byte */
#define BITS_16W	0x1	/* 16 bit, count by word */
#define BITS_32		0x2	/* 32 bit, count by byte */
#define BITS_16		0x3	/* 16 bit, count by byte */

#define COMPAT_TIMING	0x0	/* ISA Compatible timing */
#define TYPE_A_TIMING	0x1	/* Type A timing */
#define TYPE_B_TIMING	0x2	/* Type B timing */
#define TYPE_C_TIMING	0x3	/* Type C (BURST) timing */

/*------------------------*/
/* Other EISA DMA defines */
/*------------------------*/
#define	EISA_DMA_IRQ	13

/*------------------------------------------------*/
/* EISA DMA Registers. CT1 refers to channels 0-3 */
/* and CT2 refers to channels 4-7.		  */
/*------------------------------------------------*/
#define	EISA_DMA_CT1_CMD	0x08
#define	EISA_DMA_CT2_CMD	0xD0
#define	EISA_DMA_CT1_MODE	0x0B
#define	EISA_DMA_CT2_MODE	0xD6
#define	EISA_DMA_CT1_EMODE	0x40B
#define	EISA_DMA_CT2_EMODE	0x4D6
#define	EISA_DMA_CT1_REQ	0x09
#define	EISA_DMA_CT2_REQ	0x0D2
#define	EISA_DMA_CT1_SMASK	0x0A
#define	EISA_DMA_CT2_SMASK	0xD4
#define	EISA_DMA_CT1_AMASK	0x0F
#define	EISA_DMA_CT2_AMASK	0xDE
#define	EISA_DMA_CT1_STATUS	0x08
#define	EISA_DMA_CT2_STATUS	0xD0
#define	EISA_DMA_CT1_CHAIN	0x40A
#define	EISA_DMA_CT2_CHAIN	0x4D4
#define	EISA_DMA_CHAIN_STAT	0x4D4
#define	EISA_DMA_INTR_STATUS	0x40A
#define	EISA_DMA_CHAIN_EXP	0x40C
#define	EISA_DMA_CT1_CLR_BYTE_P	0x0C
#define	EISA_DMA_CT2_CLR_BYTE_P	0x0D8
#define	EISA_DMA_CT1_MCLR	0x0D
#define	EISA_DMA_CT2_MCLR	0x0DA
#define	EISA_DMA_CT1_CLR_MASK	0x0E
#define	EISA_DMA_CT2_CLR_MASK	0x0DC
#define EISA_DMA_CH0_BASE_CNT	0x01
#define EISA_DMA_CH1_BASE_CNT	0x03
#define EISA_DMA_CH2_BASE_CNT	0x05
#define EISA_DMA_CH3_BASE_CNT	0x07
#define EISA_DMA_CH5_BASE_CNT	0x0C6
#define EISA_DMA_CH6_BASE_CNT	0x0CA
#define EISA_DMA_CH7_BASE_CNT	0x0CE
#define EISA_DMA_CH0_HIGH_CNT	0x0401
#define EISA_DMA_CH1_HIGH_CNT	0x0403
#define EISA_DMA_CH2_HIGH_CNT	0x0405
#define EISA_DMA_CH3_HIGH_CNT	0x0407
#define EISA_DMA_CH5_HIGH_CNT	0x04C6
#define EISA_DMA_CH6_HIGH_CNT	0x04CA
#define EISA_DMA_CH7_HIGH_CNT	0x04CE
#define EISA_DMA_CH0_BASE_ADDR	0x000
#define EISA_DMA_CH1_BASE_ADDR	0x002
#define EISA_DMA_CH2_BASE_ADDR	0x004
#define EISA_DMA_CH3_BASE_ADDR	0x006
#define EISA_DMA_CH5_BASE_ADDR	0x0C4
#define EISA_DMA_CH6_BASE_ADDR	0x0C8
#define EISA_DMA_CH7_BASE_ADDR	0x0CC
#define EISA_DMA_CH0_LOW_ADDR	0x087
#define EISA_DMA_CH1_LOW_ADDR	0x083
#define EISA_DMA_CH2_LOW_ADDR	0x081
#define EISA_DMA_CH3_LOW_ADDR	0x082
#define EISA_DMA_CH5_LOW_ADDR	0x08B
#define EISA_DMA_CH6_LOW_ADDR	0x089
#define EISA_DMA_CH7_LOW_ADDR	0x08A
#define EISA_DMA_CH0_HIGH_ADDR	0x487
#define EISA_DMA_CH1_HIGH_ADDR	0x483
#define EISA_DMA_CH2_HIGH_ADDR	0x481
#define EISA_DMA_CH3_HIGH_ADDR	0x482
#define EISA_DMA_CH5_HIGH_ADDR	0x48B
#define EISA_DMA_CH6_HIGH_ADDR	0x489
#define EISA_DMA_CH7_HIGH_ADDR	0x48A


/*====================================================================*/
/* EISA option data structure. Contains information about options     */
/* that may be installed on a system.				      */
/*====================================================================*/

struct	eisa_option
   {
   char	board_id[EISA_IDNAMELEN + 1];	/* Option's Product ID */
   char function[EISA_FUNCLEN + 1];	/* Function string. */
   char	driver_name[EISA_NAMELEN +1];	/* Driver name */
   int	intr_b4_probe;			/* Enable interrupts before probing */
   int	intr_aft_attach;		/* Enable interrupts after probing */
   char	type;				/* D = dev, C = ctrlr, A = adapter */
   int	(*adpt_config)();		/* Adapter config routine to call */
   };


/*====================================================================*/
/* EISA slot table entry structure. Contains configuration information*/
/* for a given slot. The slot table contains one entry per slot and   */
/* is indexed by the slot number.				      */
/*====================================================================*/

union	u_intr
   {
   uint_t		intr_cnfg;	/* Convienient way to access all */
					/* bits.                         */
   struct	
      {
      uint_t	intr_num :  4;		/* Interrupt channel number (0-F). */
      uint_t	reserv1 :   1;
      uint_t	trigger :   1;		/* 0=edge, 1=level triggered. */
      uint_t	is_shared : 1;		/* 0=not shared, 1=shared. */
      uint_t	reserved :  25;
      }		intr;
   };

union	u_dma
   {
   uint_t	dma_cnfg;	/* Convienient way to access all bits. */
   struct
      {
      uint_t	channel   : 3;	/* DMA channel number (0-7). */
      uint_t	reserv1   : 3;	
      uint_t	is_shared : 1;	/* 0=not shared, 1=shared. */
      uint_t	reserv2   : 3;
      uint_t	xfer_size : 2;	/* Transfer size: */
                                /*	00 = 8-bit (byte) transfer */
                                /*	01 = 16-bit (word) transfer */
                                /*	10 = 32-bit (dword) transfer */
                                /*	11 = Reserved */
      uint_t	timing    : 2;  /* DMA Timing */
                                /*	00 = Default (ISA compatible) */
                                /*	01 = Type A */
                                /*	10 = Type B */
                                /*	11 = Type C (BURST) */
      uint_t	reserv3   : 18;
      }		dma;
   };

struct	bus_mem
   {
   int		isram;		/* 1 = read/write, 0 = read only. */
   int		decode;		/* Address decode size */
                                /* 	0 = 20 addr lines */
                                /* 	1 = 24 addr lines */
                                /* 	2 = 32 addr lines */
   int		unit_size;	/* Access size */
                                /* 	0 = Byte (8 bits) */
                                /* 	1 = Word (16 bits) */
                                /* 	2 = Dword (32 bits) */
   long		size;		/* Size of memory block (64M max). */
   vm_offset_t	start_addr;	/* Physical starting address where memory */
				/* block has been mapped to. */
   };


struct	eisa_info
   {
   struct bus_mem phys_mem;		/* Device physical memory space */
					/* description. */ 
   union u_intr	irq;			/* Interrupt channel assigned to */
					/* device. */
   union u_dma	dma_chan;		/* DMA channel assigned to device.  */
   uchar_t	init_data[60];		/* Initialization data for device.  */
   };					/* This array contains elements     */
					/* that specify the register to     */
					/* initialize and the value to      */
					/* initialize it with. Each element */
					/* can be from 3 to 10 bytes long.  */
					/* For more details see sec 4.8.1.3 */
					/* of the EISA Specification.	    */

struct	eisa_config_info
   {
   struct eisa_config_info *next_p;	/* Pointer to next entry. */
   uchar_t	type[80];		/* Type string for this function    */
   struct bus_mem phys_mem[9];		/* Device physical memory space */
					/* description. */ 
   union u_intr	irq[7];			/* Interrupt channel assigned to */
					/* device. */
   union u_dma	dma_chan[4];		/* DMA channel assigned to device.  */
   uint_t	ports[20];		/* IO port assingments for this */
					/* function. */
   uchar_t	init_data[60];		/* Initialization data for device.  */
   };					/* This array contains elements     */
					/* that specify the register to     */
					/* initialize and the value to      */
					/* initialize it with. Each element */
					/* can be from 3 to 10 bytes long.  */
					/* For more details see sec 4.8.1.3 */
					/* of the EISA Specification.	    */

struct	eisa_slot
   {
   int		next_func_index;	/* Index of entry for next function */
					/* of multi function board.	    */
   char		board_id[EISA_IDNAMELEN + 1];	/* Option's Product ID */
   char		function[EISA_FUNCLEN + 1];
   char		driver_name[EISA_NAMELEN +1];	/* Driver name */
   int		slot;			/* EISA IO slot number */
   int		(*intr)();		/* Interrupt routine for device */
   int		unit;			/* Device unit number */
   vm_offset_t	phys_io_addr;		/* Physical address of device IO */
					/* space. */
   int		class;			/* Indicates if device is a ctrlr */
					/* or a device. Used to call */
					/* correct config function.  */
   int		intr_b4_probe;		/* Enable interrupts before probing */
   int		intr_aft_attach;	/* Enable interrupts after probing */
   int		(*adpt_config)();	/* Adapter config routine to call */
   caddr_t	dev_str;
   caddr_t	intr_param;
   struct controller	*ctlr_p;	/* Pointer to controller structure   */
					/* if option in this slot is a ctlr. */
   struct bus	*bus_p;			/* Pointer to bus structure if      */
					/* option in slot is a bus adapter. */
   struct eisa_config_info *slot_info_p;/* Pointer to slots eisa info list. */
   struct bus_mem phys_mem;		/* Device physical memory space */
					/* description. */ 
   union u_intr	irq;			/* Interrupt channel assigned to */
					/* device. */
   union u_dma	dma_chan;		/* DMA channel assigned to device. */
   uchar_t	init_data[60];		/* Initialization data fo device. */
   };


/*========================================================*/
/* Structures used by eisa_get_config to return requested */
/* data. Note that memory information is returned using   */
/* the bus_mem structure defined above.			  */
/*========================================================*/

struct	irq
   {
   uint_t	channel;	/* Interrupt channel number (0-F). */
   uint_t	trigger;	/* 0=edge, 1=level triggered.      */
   uint_t	is_shared;	/* 0=not shared, 1=shared.         */
   };

struct	dma
   {
   uint_t	channel;	/* DMA channel number (0-7). */
   uint_t	is_shared;	/* 0=not shared, 1=shared.   */
   uint_t	xfer_size;	/* Transfer size: */
                                /*	00 = 8-bit (byte) transfer */
                                /*	01 = 16-bit (word) transfer */
                                /*	10 = 32-bit (dword) transfer */
                                /*	11 = Reserved */
   uint_t	timing;		/* DMA Timing */
                                /*	00 = Default (ISA compatible) */
                                /*	01 = Type A */
                                /*	10 = Type B */
                                /*	11 = Type C (BURST) */
   uint_t	mode;		/* Transfer mode. */
   };

struct	e_port
   {
   vm_offset_t	base_address;	/* Base address of io ports. */
   uint_t	numb_of_ports;	/* Number of ports in this range. */
   uint_t	is_shared;	/* Ports can be shared by another device. */
   };



/*---------------------------------------------------------*/
/* Following is where eisa_info struct is plugged into the */
/* controller structure.				   */
/*---------------------------------------------------------*/
#define	eisainfo	conn_priv[0]

struct	eisa_sw
   {
   vm_offset_t	config_base_addr; /* Base address of eisa configuration */
				  /* data. 				*/
   uint_t	config_stride;	/* Stride between bytes of config data. */
   int	(*enable_option)();	/* Function for enabling option interrupts */
   int	(*disable_option)();	/* Function for disabling option interrupts */
   int	(*set_irq_edge)();	/* Set irq's trigger to edge sensitive. */
   int	(*set_irq_level)();	/* Set irq's trigger to level sensitive. */
   int	(*dma_init)();		/* Initialize EISA DMA engine. */
   int	(*dma_config)();	/* Set DMA channel's timing. */
   int	(*irq_to_scb_vector)();	/* Returns scb vector for a given IRQ level. */
   int	(*intr_dispatch)();	/* Platform specific interrupt dispatcher. */
   vm_offset_t  sparse_io_base; /* io_handle for sparse io base address */ 
   vm_offset_t  sparse_mem_base;/* io_handle for sparse memory base address */
   vm_offset_t  dense_mem_base; /* io_handle for dense memory base address */
   vm_offset_t  (*busphys_to_iohandle)();
   
   };


/*=====================*/
/* Function prototypes */
/*=====================*/

int	eisa_get_config (struct controller *ctlr_p, uint_t config_item,
			 char *func_type,	    void *data_p,
			 int handle);


#endif	/* __EISA_H__ */
