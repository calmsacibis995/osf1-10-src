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
 * @(#)$RCSfile: eisa_cnfg.h,v $ $Revision: 1.1.4.4 $ (DEC) $Date: 1993/11/02 15:30:25 $
 */

/***************************************************************************/
/*                                                                         */
/* MODULE NAME: eisa_cnfg.h						   */
/* 									   */ 
/* LOCATION:	.../src/kernel/io/dec/eisa				   */
/* 									   */ 
/* DESCRIPTION:								   */
/*		Contains definitions for dealing with the EISA configurtion*/
/* 		as created by the EISA Configuration Utility.		   */
/* 									   */ 
/***************************************************************************/

#ifndef	__EISA_CNFG_H__
#define	__EISA_CNFG_H__

#define	FLASH_STRIDE	0x200
#define	CONF_HDR_WIDTH	12
#define	TYPE_STRING	0x1
#define	MEM_ENTRY	0x2
#define	IRQ_ENTRY	0x4
#define	DMA_ENTRY	0x8
#define	PORT_ENTRY	0x10
#define	INIT_ENTRY	0x20
#define	FREE_FORM	0x40
#define FUNC_DISABLED	0x80

#define SIZE_OF_EISA_HDR	31
#define SIZE_OF_EISA_CONFIG_BLK	320
#define SIZE_OF_SELECTIONS	26
#define SIZE_OF_FUNC_INFO	1
#define SIZE_OF_TYPE_STRING	80
#define SIZE_OF_MEM_ENTRY	7
#define NUMB_OF_MEM_ENTRIES	9
#define SIZE_OF_IRQ_ENTRY	2
#define NUMB_OF_IRQ_ENTRIES	7
#define SIZE_OF_DMA_ENTRY	2
#define NUMB_OF_DMA_ENTRIES	4
#define SIZE_OF_PORT_ENTRY	3
#define NUMB_OF_PORT_ENTRIES	20
#define SIZE_OF_INIT_ENTRY	3
#define NUMB_OF_INIT_BYTES	60
#define HDR_SLOT_INFO_OFFSET    20

   struct       cnfg_hdr
      {
      char	manuf_id[8];
      int	sbb_offset;
      int	cnfg_blocks;
      struct cnfg_blk   *config_block_p;
      };
   
   struct	cnfg_blk
      {
      struct cnfg_blk  *next_blk;
      uint_t		id;
      uint_t		slot_info;
      uint_t		cfg_ext;
      uchar_t		selections[26];
      uchar_t		func_info;
      uchar_t		type_string[80];
      ulong_t		mem_cnfg[9];
      uint_t		irq_cnfg[7];
      uint_t		dma_cnfg[4];
      uint_t		port_cnfg[20];
      uchar_t		init_data[60];
      };
   
   struct	ssb_hdr
      {
      int	major_id;
      int	minor_id;
      int	directories;
      int	device_id;
      char	device_name[8];
      int	selftest_addr;
      int	config_blocks;
      };
   

   struct	general_slot_info
      {
      uchar_t	slot_info;
      ushort_t	ecu_major_rev;
      ushort_t	ecu_minor_rev;
      ushort_t	chk_sum;
      ushort_t	numb_dev_funcs;
      uchar_t	func_info;
      uint_t	comp_id;
      };



#endif	/* __EISA_CNFG_H__ */
