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
*****************************************************************************
**									    *
**  Copyright ) Digital Equipment Corporation, 1992		    	    *
**  All Rights Reserved.  Unpublished rights reserved under  the  copyright *
**  laws of the United States.						    *
**									    *
**  The software contained on this media is proprietary to and embodies the *
**  confidential  technology of Digital Equipment Corporation.  Possession, *
**  use,  duplication  or  dissemination  of  the  software  and  media  is *
**  authorized  only  pursuant  to  a  valid  written  license from Digital *
**  Equipment Corporation.						    *
**									    *
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure by the  U.S. *
**  Government  is  subject  to  restrictions  as set forth in Subparagraph *
**  (c)(1)(ii) of DFARS 252.227-7013, or in FAR 52.227-19, as applicable.   *
**									    *
*****************************************************************************/

#ifndef	_PV_H_
#define _PV_H_

/*
 * Basic Data Type Definitions
 */
typedef unsigned int pv_register_t;
typedef unsigned long pv_bus_addr_t;
typedef unsigned int pv_halfaddr_t;
typedef struct {
  pv_halfaddr_t		low;
  pv_halfaddr_t		high;
} pv_bus_addr2_t;

typedef short pv_subpixel_t;

#define	PV_SUBPIX_GET_FRAC( _sp )	((unsigned short) ((_sp) & 0x7))
#define	PV_SUBPIX_GET_WHOLE( _sp )	((short) (((short) (_sp)) >> 3 ))
#define	PV_SUBPIX_MAKE( _w, _f )	((unsigned short) ((((unsigned short) (_w))<<3)|(((unsigned short)(_f))&0x7)))


typedef struct {
  pv_subpixel_t		x;
  pv_subpixel_t		y;
} pv_subpixel_pair_t;

typedef struct {
  unsigned int
    pixelmap_id : 15,
    x : 6,
    y : 6,
    dirty : 1,
    : 4;
} pv_graphic_tlb_t;

/*
 * Server/Driver interface numbers
 */
#define	PV_PVQM_SIZE			(1<<17)
#define	PV_INVALID_PIXELMAP_ID		2
#define	PV_DOORBELL_0_PIXELMAP_ID	3
#define	PV_DOORBELL_1_PIXELMAP_ID	4
#define	PV_PIXELMAP_DCC_MASK		0x8000

/*
 * Address access macros
 */

#define PV_BUS_ADDR_LOAD_L( _addr ) ((pv_halfaddr_t) ((pv_bus_addr2_t *) &(_addr))->low )
#define PV_BUS_ADDR_LOAD_H( _addr ) ((pv_halfaddr_t) ((pv_bus_addr2_t *) &(_addr))->high )
#define PV_BUS_ADDR_STORE_L( _addr, _val ) ((pv_halfaddr_t) ((pv_bus_addr2_t *) &(_addr))->low = (pv_halfaddr_t) (_val))
#define PV_BUS_ADDR_STORE_H( _addr, _val ) ((pv_halfaddr_t) ((pv_bus_addr2_t *) &(_addr))->high = (pv_halfaddr_t) (_val))

/*
 * Module Definitions
 */

#define	PV_PROM_OFFSET			0x00000000L
#define	PV_RENDER_OFFSET		0x00000000L
#define	PV_SRAM_OFFSET			0x00180000L
#define	PV_GCP_OFFSET			0x00200000L
#define	PV_PVA_OFFSET			0x00210000L
#define	PV_BT463_OFFSET			0x00218000L
#define	PV_BT431_0_OFFSET		0x00220000L
#define	PV_BT431_1_OFFSET		0x00228000L
#define	PV_MODULE_SIZE			0x00230000L

#define	PV_SPARSE_PROM_OFFSET		(PV_PROM_OFFSET << 1)
#define	PV_SPARSE_RENDER_OFFSET		(PV_RENDER_OFFSET << 1)
#define	PV_SPARSE_SRAM_OFFSET		(PV_SRAM_OFFSET << 1)
#define	PV_SPARSE_GCP_OFFSET		(PV_GCP_OFFSET << 1)
#define	PV_SPARSE_PVA_OFFSET		(PV_PVA_OFFSET << 1)
#define	PV_SPARSE_BT463_OFFSET		(PV_BT463_OFFSET << 1)
#define	PV_SPARSE_BT431_0_OFFSET	(PV_BT431_0_OFFSET << 1)
#define	PV_SPARSE_BT431_1_OFFSET	(PV_BT431_1_OFFSET << 1)
#define	PV_SPARSE_MODULE_SIZE		(PV_MODULE_SIZE << 1)

/*
 * Module and System Addresses as seen from the PVbus
 */
#define	PVB_RENDER_OFFSET_HIGH		0x00000000L
#define	PVB_RENDER_OFFSET_LOW		0x00000000L
#define	PVB_SRAM_OFFSET_HIGH		0x00000000L
#define	PVB_SRAM_OFFSET_LOW		0x00800000L
#define	PVB_GCP_OFFSET_HIGH		0x00000000L
#define	PVB_GCP_OFFSET_LOW		0x00c00000L
#define	PVB_PHOST_OFFSET_HIGH		0x00080000L
#define	PVB_PHOST_OFFSET_LOW		0x00000000L
#define	PVB_VHOST_OFFSET_HIGH		0x00180000L
#define	PVB_VHOST_OFFSET_LOW		0x00000000L

/*
 * Locate major module structures
 */
#define	PV_BT431_0_BASE(_pi)	((struct bt431 *) ((unsigned long) PV_BASE(_pi) + PV_BT431_0_OFFSET ) )
#define	PV_BT431_1_BASE(_pi)	((struct bt431 *) ((unsigned long) PV_BASE(_pi) + PV_BT431_1_OFFSET ) )
#define	PV_BT463_BASE(_pi)	((struct bt463 *) ((unsigned long) PV_BASE(_pi) + PV_BT463_OFFSET ) )

/*
 * PVA registers
 */

/*
 * Descriptions of page and shadow page table.
 */
#define	PV_PVA_PAGE_SHIFT_0	9
#define	PV_PVA_PAGE_SHIFT_1	12
#define	PV_PVA_PAGE_SHIFT_2	13
#define	PV_PVA_PAGE_SHIFT_3	16
#define	PV_PVA_PAGE_SHIFT	PV_PVA_PAGE_SHIFT_2

#define	PV_PVA_PAGE_SIZE_0	(1<<PV_PVA_PAGE_SHIFT_0)
#define	PV_PVA_PAGE_SIZE_1	(1<<PV_PVA_PAGE_SHIFT_1)
#define	PV_PVA_PAGE_SIZE_2	(1<<PV_PVA_PAGE_SHIFT_2)
#define	PV_PVA_PAGE_SIZE_3	(1<<PV_PVA_PAGE_SHIFT_3)
#define	PV_PVA_PAGE_SIZE	PV_PVA_PAGE_SIZE_2

#define	PV_PVA_VPN_MASK_0	0x7fffffff
#define	PV_PVA_VPN_MASK_1      	0x7fffffff
#define	PV_PVA_VPN_MASK_2	0x3fffffff
#define	PV_PVA_VPN_MASK_3	0x07ffffff
#define	PV_PVA_VPN_MASK		PV_PVA_VPN_MASK_2

#define	PV_PVA_PPN_MASK_0	0x003fffff
#define	PV_PVA_PPN_MASK_1	0x003fffff
#define	PV_PVA_PPN_MASK_2	0x001fffff
#define	PV_PVA_PPN_MASK_3	0x0003ffff
#define	PV_PVA_PPN_MASK		PV_PVA_VPN_MASK_2

#define	PV_PVA_TABLE_SIZE_0	(1<<12)
#define	PV_PVA_TABLE_SIZE_1	(1<<13)
#define	PV_PVA_TABLE_SIZE_2	(1<<14)
#define	PV_PVA_TABLE_SIZE_3	(1<<17)
#define	PV_PVA_TABLE0_SIZE	PV_PVA_TABLE_SIZE_0
#define	PV_PVA_TABLE1_SIZE	PV_PVA_TABLE_SIZE_1

#define	PV_PVA_PREFETCH_SIZE	512

/*
 * TLB Description
 *
 * _ENTRIES gives the number of TLB entries implemented and _TOTAL
 * the number provided for in the module addressing.
 */

#define	PV_PVA_TLB_ENTRIES	16
#define	PV_PVA_TLB_TOTAL	64
#define	PV_RENDER_TLB_ENTRIES	256
#define	PV_RENDER_TLB_TOTAL	256

#define	PV_VA_TO_VPN( _va )	((unsigned int) (((unsigned long) (_va))>>PV_PVA_PAGE_SHIFT) & PV_PVA_VPN_MASK)
#define	PV_VPN_TO_VA( _vpn )	((vm_offset_t) (((unsigned long) (_vpn))<<PV_PVA_PAGE_SHIFT))
#define	PV_PA_TO_PPN( _pa )	((unsigned int) (((unsigned long) (_pa))>>PV_PVA_PAGE_SHIFT) & PV_PVA_PPN_MASK)
#define	PV_PPN_TO_PA( _ppn )	((vm_offset_t) (((unsigned long) (_ppn))<<PV_PVA_PAGE_SHIFT))

typedef struct pv_pva_tlb_entry {
  unsigned int
	vpn : 32,
	ppn : 22,
	pid : 8,
	we : 1,
	mru : 1;
} pv_pva_tlb_entry_t;

typedef struct pv_pva_tlb_entry_s {
  unsigned int
	vpn : 32,
	: 32,
	ppn : 22,
	pid : 8,
	we : 1,
	mru : 1,
	: 32;
} pv_pva_tlb_entry_s_t;

/*
 * PVA PID Description
 */
typedef struct pv_pid {
  unsigned int
    pid0 : 8,
    pid1 : 8,
    : 16;
} pv_pid_t;

/*
 * PVA DMA Miss Register
 */
typedef struct pv_dma_miss {
  unsigned int
    vpn : 31,
    rd : 1;
} pv_dma_miss_t;

/*
 * PVA CSR Description
 */
typedef struct pv_csr {
  unsigned int
    vps : 2,
    spts0 : 2,
    spts1 : 2,
    dma_lockout : 1,
    pb_reset : 1,
    gpo : 1,
    gpi : 1,
    : 22;
} pv_csr_t;

#define	PV_PVA_CSR_VPS_512	0
#define	PV_PVA_CSR_VPS_4K	1
#define	PV_PVA_CSR_VPS_8K	2
#define	PV_PVA_CSR_VPS_64K	3
#define	PV_PVA_CSR_VPS		PV_PVA_CSR_VPS_8K

#define	PV_PVA_CSR_SPTS_4K	0
#define	PV_PVA_CSR_SPTS_8K	1
#define	PV_PVA_CSR_SPTS_16K	2
#define	PV_PVA_CSR_SPTS_128K	3
#define	PV_PVA_CSR_SPTS0	PV_PVA_CSR_SPTS_4K
#define	PV_PVA_CSR_SPTS1	PV_PVA_CSR_SPTS_8K

/*
 * PVA ISR Description
 */
#define	PV_PVA_INTR_PB0		0x00000001
#define	PV_PVA_INTR_PB1		0x00000002
#define	PV_PVA_INTR_TLB_MISS0	0x00000004
#define	PV_PVA_INTR_TLB_MISS1	0x00000008
#define	PV_PVA_INTR_DMA_RD_ERR0	0x00000010
#define	PV_PVA_INTR_DMA_RD_ERR1	0x00000020
#define	PV_PVA_INTR_DMA_WR_ERR0	0x00000040
#define	PV_PVA_INTR_DMA_WR_ERR1	0x00000080
#define	PV_PVA_INTR_ALL		0x000000ff

/*
 * PVA Process ID assignments
 */
#define	PV_PVA_PID_SERVER	0x01
#define	PV_PVA_PID_PAGER	0x00
#define	PV_PVA_PID_INVALID	0xff

/*
 * PVA TC countdown counter max value
 */
#define	PV_PVA_COUNTER_MAX	0x0003ffff

/*
 * PVA Register Space Description
 */
typedef struct pv_pva_register {
  pv_pva_tlb_entry_t    tlb[PV_PVA_TLB_TOTAL];
  pv_pid_t		pid;
  pv_dma_miss_t		dma_miss;
  pv_register_t		spt_base0;
  pv_register_t		spt_base1;
  pv_csr_t		csr;
  pv_register_t		intr_status;
  pv_register_t		intr_mask;
  pv_register_t		intr_mask_set;
  pv_register_t		intr_mask_clear;
  pv_register_t		counter;
  pv_register_t		revision;
  pv_register_t		_pad1[4];
  pv_register_t		tlb_burstwr;
} pv_pva_register_t;

typedef struct pv_pva_register_s {
  pv_pva_tlb_entry_s_t	tlb[PV_PVA_TLB_TOTAL];
  pv_pid_t		pid;
  pv_register_t		_pad1;
  pv_dma_miss_t		dma_miss;
  pv_register_t		_pad2;
  pv_register_t		spt_base0;
  pv_register_t		_pad3;
  pv_register_t		spt_base1;
  pv_register_t		_pad4;
  pv_csr_t		csr;
  pv_register_t		_pad5;
  pv_register_t		intr_status;
  pv_register_t		_pad6;
  pv_register_t		intr_mask;
  pv_register_t		_pad7;
  pv_register_t		intr_mask_set;
  pv_register_t		_pad8;
  pv_register_t		intr_mask_clear;
  pv_register_t		_pad9;
  pv_register_t		counter;
  pv_register_t		_pad10;
  pv_register_t		revision;
  pv_register_t		_pad11[9];
  pv_register_t 	tlb_burstwr;
  pv_register_t		_pad12;
} pv_pva_register_s_t;

/*
 * Access to the register structures
 */
#define	PV_PVA_BASE(_pi)	((pv_pva_register_t *) ((pv_info *)(_pi))->p_pva)
#define	PV_SPARSE_PVA_BASE(_pi)	((pv_pva_register_s_t *) ((pv_info *)(_pi))->p_pva_s)

/*
 * GCP Registers
 */
#define PV_GCP_FREG_OFFSET( _fpu, _reg )  ((vm_offset_t) (((_fpu)<<10) | ((_reg)<<2)))
#define	PV_GCP_GREG_OFFSET( _reg ) ((vm_offset_t) (0x00002000 | ((_reg)<<2)))
#define	PV_GCP_IMP_OFFSET	((vm_offset_t) 0x00004000)
#define	PV_GCP_CMP01_OFFSET	((vm_offset_t) 0x00004008)
#define	PV_GCP_CMP23_OFFSET	((vm_offset_t) 0x0000400c)
#define	PV_GCP_INST_OFFSET(_i)	((vm_offset_t) (0x00008000 | ((_i)<<2)))
#define	PV_GCP_PC_OFFSET	((vm_offset_t) 0x00004010)
#define	PV_GCP_SR_OFFSET	((vm_offset_t) 0x00004014)
#define	PV_GCP_HALT_OFFSET	((vm_offset_t) 0x00004018)
#define	PV_GCP_CC_OFFSET	((vm_offset_t) 0x00004024)
#define	PV_GCP_DMAL_OFFSET	((vm_offset_t) 0x0000c000)
#define	PV_GCP_DMAH_OFFSET	((vm_offset_t) 0x0000c004)
#define	PV_GCP_DMAOP_OFFSET	((vm_offset_t) 0x0000c008)

#define	PV_GCP_SPARSE_FREG_OFFSET( _fpu, _reg )	((vm_offset_t) ((unsigned long) PV_GCP_FREG_OFFSET( _fpu, _reg )) << 1)
#define	PV_GCP_SPARSE_GREG_OFFSET( _reg )	((vm_offset_t) ((unsigned long) PV_GCP_GREG_OFFSET( _reg )) << 1)
#define	PV_GCP_SPARSE_IMP_OFFSET		((vm_offset_t) ((unsigned long) PV_GCP_IMP_OFFSE) << 1)
#define	PV_GCP_SPARSE_CMP01_OFFSET		((vm_offset_t) ((unsigned long) PV_GCP_CMP01_OFFSET) << 1)
#define	PV_GCP_SPARSE_CMP23_OFFSET		((vm_offset_t) ((unsigned long) PV_GCP_CMP23_OFFSET) << 1)
#define	PV_GCP_SPARSE_INST_OFFSET(_i)		((vm_offset_t) ((unsigned long) PV_GCP_INST_OFFSET(_i)) << 1)
#define	PV_GCP_SPARSE_PC_OFFSET			((vm_offset_t) ((unsigned long) PV_GCP_PC_OFFSET) << 1)
#define	PV_GCP_SPARSE_SR_OFFSET			((vm_offset_t) ((unsigned long) PV_GCP_SR_OFFSET) << 1)
#define	PV_GCP_SPARSE_HALT_OFFSET		((vm_offset_t) ((unsigned long) PV_GCP_HALT_OFFSET) << 1)
#define	PV_GCP_SPARSE_CC_OFFSET			((vm_offset_t) ((unsigned long) PV_GCP_CC_OFFSET) << 1)
#define	PV_GCP_SPARSE_DMAL_OFFSET		((vm_offset_t) ((unsigned long) PV_GCP_DMAL_OFFSET) << 1)
#define	PV_GCP_SPARSE_DMAH_OFFSET		((vm_offset_t) ((unsigned long) PV_GCP_DMAH_OFFSET) << 1)
#define	PV_GCP_SPARSE_DMAOP_OFFSET		((vm_offset_t) ((unsigned long) PV_GCP_DMAOP_OFFSET) << 1)

/*
 * GCP Interrupt Host Reason Codes
 */
#define	PV_GCP_INTR_CLEAR	0x00000000
#define	PV_GCP_INTR_COUNT_DOWN	0x3f800000
#define	PV_GCP_INTR_HALT	0x40000000
#define	PV_GCP_INTR_HOST_CLIP	0xbf800000

/*
 * Access to the GCP
 */
#define	PV_GCP_BASE(_pi)	((caddr_t) ((pv_info *)(_pi))->p_gcp)
#define	PV_SPARSE_GCP_BASE(_pi)	((caddr_t) ((pv_info *)(_pi))->p_gcp_s)

/*
 * Access to SRAM
 */
#define	PV_SRAM_BASE(_pi)	((caddr_t) ((pv_info *)(_pi))->p_sram)
#define	PV_SPARSE_SRAM_BASE(_pi) ((caddr_t) ((pv_info *)(_pi))->p_sram_s)

/*
 * PV Renderer Registers
 */
#define	PV_REND_IMP_OFFSET	((vm_offset_t) 0x00080000)
#define	PV_REND_POLL_OFFSET	((vm_offset_t) 0x00100000)
#define	PV_REND_DMA_OFFSET	((vm_offset_t) 0x00040000)
#define	PV_REND_TLB_OFFSET	((vm_offset_t) 0x000c0000)
#define	PV_REND_VIDEO_OFFSET	((vm_offset_t) 0x00140000)

#define	PV_REND_SPARSE_IMP_OFFSET	((vm_offset_t) ((unsigned long) PV_REND_IMP_OFFSET) << 1)
#define	PV_REND_SPARSE_POLL_OFFSET	((vm_offset_t) ((unsigned long) PV_REND_POLL_OFFSET) << 1)
#define	PV_REND_SPARSE_DMA_OFFSET	((vm_offset_t) ((unsigned long) PV_REND_DMA_OFFSET) << 1 )
#define	PV_REND_SPARSE_TLB_OFFSET	((vm_offset_t) ((unsigned long) PV_REND_TLB_OFFSET) << 1 )
#define	PV_REND_SPARSE_VIDEO_OFFSET	((vm_offset_t) ((unsigned long) PV_REND_VIDEO_OFFSET) << 1 )

/*
 * PV Register Definitions
 */
#define	PV_RENDER_IMP_USZ_4X2		0
#define	PV_RENDER_IMP_USZ_4X4		1
#define	PV_RENDER_IMP_USZ_8X2		2
#define	PV_RENDER_IMP_USZ_8X4		4

#define	PV_RENDER_IMP_VSZ_1024X1024	0
#define	PV_RENDER_IMP_VSZ_1280X1024	1
#define	PV_RENDER_IMP_VSZ_2048X1024	2
#define	PV_RENDER_IMP_VSZ_2048X1536	3
#define	PV_RENDER_IMP_VSZ_2048X2048	4
#define	PV_RENDER_IMP_VSZ_UNSUPPORTED	5

typedef struct pv_render_imp_rev {
  unsigned int
    min : 4,
    major : 4,
    implement : 8,
    update_size : 4,
    vis_pmap_size : 4,
    mem_size : 5,
    reserved : 3;
} pv_render_imp_rev_t;

typedef struct pv_render_config {
  unsigned int
    max_pkts : 8,
    : 1,
    stereo : 1,
    vsync : 1,
    page : 1,
    vm : 1,
    render : 1,
    packet : 1,
    block_en : 4,
    : 13;
} pv_render_config_t;

typedef struct pv_render_status {
  unsigned int
    field : 1,
    vsync_int : 1,
    page_int : 1,
    vm_status : 1,
    vm_int : 1,
    render_status : 1,
    render_int : 1,
    packet_status : 1,
    packet_int : 1,
    stall : 1,
    : 22;
} pv_render_status_t;

typedef struct pv_render_implement {
  pv_register_t		tlb_addr;
  pv_render_imp_rev_t	imp_rev;
  pv_render_config_t	config;
  pv_render_status_t	status;
} pv_render_implement_t;

typedef struct pv_render_implement_s {
  pv_register_t		tlb_addr;
  pv_register_t		pad0;
  pv_render_imp_rev_t	imp_rev;
  pv_register_t		pad1;
  pv_render_config_t	config;
  pv_register_t		pad2;
  pv_render_status_t	status;
  pv_register_t		pad3;
} pv_render_implement_s_t;

#define	PV_RENDER_DMA_FORMAT_RGB		0
#define	PV_RENDER_DMA_FORMAT_DCC		1

#define	PV_RENDER_DMA_RW_WRITE			0
#define	PV_RENDER_DMA_RW_READ			1

typedef struct pv_render_dma {
  pv_register_t		pixelmap_id;
  pv_register_t		pad0;
  pv_subpixel_pair_t	xy_min;
  pv_subpixel_pair_t	xy_max;
  pv_register_t		addr_low;
  pv_register_t		addr_high;
  pv_register_t		pad1;
  pv_register_t		offset;
  pv_register_t		format;
  pv_register_t		read_write;
  pv_register_t		status;
} pv_render_dma_t;

typedef struct pv_render_dma_s {
  pv_register_t		pixelmap_id;
  pv_register_t		pad0[3];
  pv_subpixel_pair_t	xy_min;
  pv_register_t		pad1;
  pv_subpixel_pair_t	xy_max;
  pv_register_t		pad2;
  pv_register_t		addr_low;
  pv_register_t		pad3;
  pv_register_t		addr_high;
  pv_register_t		pad4[3];
  pv_register_t		offset;
  pv_register_t		pad5;
  pv_register_t		format;
  pv_register_t		pad6;
  pv_register_t		read_write;
  pv_register_t		pad7;
  pv_register_t		status;
  pv_register_t		pad8;
} pv_render_dma_s_t;

typedef struct pv_render_video {
  pv_register_t		vblank_s;
  pv_register_t		vblank_f;
  pv_register_t		vsync_s;
  pv_register_t		vsync_f;
  pv_register_t		hblank_s;
  pv_register_t		hblank_f;
  pv_register_t		hsync2;
  pv_register_t		pad0;
  pv_register_t		hsync_s;
  pv_register_t		hsync_f;
  pv_register_t		crt_enable;
} pv_render_video_t;

typedef struct pv_render_video_s {
  pv_register_t		vblank_s;
  pv_register_t		pad0;
  pv_register_t		vblank_f;
  pv_register_t		pad1;
  pv_register_t		vsync_s;
  pv_register_t		pad2;
  pv_register_t		vsync_f;
  pv_register_t		pad3;
  pv_register_t		hblank_s;
  pv_register_t		pad4;
  pv_register_t		hblank_f;
  pv_register_t		pad5;
  pv_register_t		hsync2;
  pv_register_t		pad6[3];
  pv_register_t		hsync_s;
  pv_register_t		pad7;
  pv_register_t		hsync_f;
  pv_register_t		pad8;
  pv_register_t		crt_enable;
  pv_register_t		pad9;
} pv_render_video_s_t;

/*
 * Access to PV
 */
#define	PV_REND_BASE(_pi)	((caddr_t) ((pv_info *)(_pi))->p_pv)
#define	PV_REND_IMP_BASE(_pi)	((pv_render_implement_t *) ((unsigned long) PV_REND_BASE(_pi) + PV_REND_IMP_OFFSET ) )
#define	PV_REND_DMA_BASE(_pi)	((pv_render_dma_t *) ((unsigned long) PV_REND_BASE(_pi) + PV_REND_DMA_OFFSET ) )
#define	PV_REND_TLB_BASE(_pi)	((pv_graphic_tlb_t *) ((unsigned long) PV_REND_BASE(_pi) + PV_REND_TLB_OFFSET ) )
#define	PV_REND_VIDEO_BASE(_pi)	((pv_render_video_t *) ((unsigned long) PV_REND_BASE(_pi) + PV_REND_VIDEO_OFFSET ) )


#define	PV_SPARSE_REND_BASE(_pi) ((caddr_t) ((pv_info *)(_pi))->p_pv_s)
#define	PV_SPARSE_REND_IMP_BASE(_pi)	((pv_render_implement_s_t *) ((unsigned long) PV_SPARSE_REND_BASE(_pi) + PV_REND_SPARSE_IMP_OFFSET ) )
#define	PV_SPARSE_REND_DMA_BASE(_pi)	((pv_render_dma_s_t *) ((unsigned long) PV_SPARSE_REND_BASE(_pi) + PV_REND_SPARSE_DMA_OFFSET ) )
#define	PV_SPARSE_REND_TLB_BASE(_pi)	((pv_graphic_tlb_t *) ((unsigned long) PV_SPARSE_REND_BASE(_pi) + PV_REND_SPARSE_TLB_OFFSET ) )
#define	PV_SPARSE_REND_VIDEO_BASE(_pi)	((pv_render_video_s_t *) ((unsigned long) PV_SPARSE_REND_BASE(_pi) + PV_REND_SPARSE_VIDEO_OFFSET ) )

/*
 * PV Renderer Polling Register
 */
#define	PV_REND_POLL_STATUS_FREE	0
#define	PV_REND_POLL_STATUS_BUSY	1

#define	PV_REND_HOST_TO_POLL( _pa )	((vm_offset_t) (PV_RENDER_POLL_OFFSET | 0x00020000 | (((unsigned int) (_pa) & 0x03fff000)>>9)))
#define	PV_REND_SRAM_TO_POLL( _sram )	((vm_offset_t) (PV_RENDER_POLL_OFFSET | (((unsigned int) (_sram) & 0x03fff000)>>9)))

#define	PV_REND_SPARSE_HOST_TO_POLL( _pa )	((vm_offset_t) (PV_REND_HOST_TO_POLL( _pa )>>1))
#define	PV_REND_SPARSE_SRAM_TO_POLL( _sram )	((vm_offset_t) (PV_REND_SRAM_TO_POLL( _sram )>>1))

/*
 * Rendering Definitions
 */
#define PV_2D_SCRATCHBLOCK_ADDR       0L                /* passed to PV      */
#define PV_2D_SCRATCHBLOCK_SIZE       8*1024           /* this is in pixels */

/* the following is calculated as follows:                                  */
/*      Per packet data maximum         38 longwords                        */
/*      Per Primitive data              16 longwords * 255                  */
/*      Per Vertex data (2D only)        4 longwords * 255                  */
/*                                      ------------                        */
/*      Total                           5138                                */
/*                                                                          */

#define PV_MAX_BUFFER_SIZE              (5138 * 4)
#define PV_MAX_PRIMITIVES               255
                                                                                

#define PV_MAX_X_COORD    (0x1000)	/* not in 13.3; i.e., (4096), (1<<12) */
#define PV_MAX_PLUS_COORD (0x7fff)	/* in 13.3; i.e., ((1<<15)-1)	      */
#define PV_MAX_NEG_COORD  (0x8000)	/* in 13.3; i.e., ((-1)<<15)	      */

#define PV_VISIBLE_PID 0

/*
 * PV_INSTR_ instruction-field macros may be used instead of 
 * the pvInstruction structure, but the latter is usually preferable.
 */

#define PV_INSTR_OPCODE 	0
#define PV_INSTR_DST 		8
#define PV_INSTR_SRC1 		12
#define PV_INSTR_SRC0 		16
#define PV_INSTR_CCOP 		20
#define PV_INSTR_XYEN 		22
#define PV_INSTR_XYSENSE 	24
#define PV_INSTR_ZEN 		25
#define PV_INSTR_CLEN 		26
#define PV_INSTR_CCEN 		27

#define PV_CAP_BUTT 		0
#define PV_CAP_NOT_LAST		1

#define	PV_XYMASK_DATA_LENGTH	8

#define	PV_POINTS		0
#define	PV_LINES		1
#define	PV_WIDE_LINES		2
#define	PV_QUADS		3
#define	PV_READ_BLOCKS		4
#define	PV_WRITE_BLOCKS		5

#define	PV_NONE			0
#define	PV_PER_PACKET		1
#define	PV_PER_PRIMITIVE	2
#define	PV_PER_VERTEX		3

#define	PV_BAD_GEOM_FMT		0
#define	PV_BAD_Z0_FMT		1
#define	PV_BAD_Z1_FMT		2
#define	PV_BAD_XYMASK_FMT	3

#define	PV_BAD_XYMASK_PTR	4

#define	PV_QUEUED		1
#define	PV_SENT_TO_HW		2

/*
 * opcodes 
 */

#define PV_AND			0
#define PV_OR			1
#define PV_XOR			2
#define PV_NOT			3
#define PV_COPY			4
#define PV_ADD			5
#define PV_SUB			6
#define PV_CMPLT		7
#define PV_CMPLE		8
#define PV_CMPEQ		9
#define PV_CMPGE		10
#define PV_CMPGT		11
#define PV_CMPOVF		12
#define PV_MULHIGH		13
#define PV_MULLOW		14
#define PV_ADD24		15
#define PV_SUB24		16
#define PV_CMPLT24		17
#define PV_CMPLE24		18
#define PV_CMPEQ24		19
#define PV_CMPGE24		20
#define PV_CMPGT24             	21

/*
 * CC OPERATION 
 */

#define PV_CC_REPLACE		0
#define PV_CC_AND		1
#define PV_CC_OR		2

/*
 * xymask enable flags 
 */

#define PV_XYMASK_DISABLE	0
#define PV_XYMASK_16X16 	1
#define PV_XYMASK_256X1 	2
#define PV_XYMASK_1X256 	3
	
/*
 * xymask sense 
 */

#define PV_XYMASK_SET_ENABLED	0
#define PV_XYMASK_SET_DISABLED	1

/*
 * Source and Destination operands
 */

#define PV_OPERAND_PR0		0
#define PV_OPERAND_PR1		1
#define PV_OPERAND_PID0		4
#define PV_OPERAND_PID1		5
#define PV_OPERAND_PID2		6
#define PV_OPERAND_PID3		7
#define PV_OPERAND_PID4		8
#define PV_OPERAND_PID5		9
#define PV_OPERAND_0		10
#define PV_OPERAND_1		11
#define PV_OPERAND_RGB0		12
#define PV_OPERAND_RGB1		13
#define PV_OPERAND_Z0		14
#define PV_OPERAND_Z1		15

/*
 * PixelVision values for the 0.0 and 1.0 contstants.
 */

#define PV_0POINT0		0
#define PV_1POINT0		0xff


/*
 *	Structure definitions
 */ 

typedef struct pv_packet_header
{
  unsigned geomFmt 	: 3 ; /* what primitive? */
  unsigned capStyle 	: 1 ; /* Cap Butt or Cap not last */
  unsigned clipFmt	: 2 ; /* None, per-packet, or per-primitive */
  unsigned xyMaskFmt 	: 2 ; /* None, per-packet, or per-primitive */
  unsigned zMaskFmt 	: 2 ; /* None, per-packet, or per-primitive */
  unsigned rgb0Fmt 	: 2 ; /* none,per-packet,per-primitive,or per-vertex */
  unsigned rgb1Fmt 	: 2 ; /* none,per-packet,per-primitive,or per-vertex */
  unsigned z0Fmt 	: 2 ; /* none,per-packet,per-primitive,or per-vertex */
  unsigned z1Fmt 	: 2 ; /* none,per-packet,per-primitive,or per-vertex */
  unsigned instCount 	: 4 ; /* instructions in the packet */
  unsigned pidCount 	: 2 ; /* number of pixelmap IDs in packet */
  unsigned primCount 	: 8 ; /* number of primitives in packet */
} pv_packet_header_t, *pv_packet_header_ptr_t;


typedef struct pv_point {
  	short x;
  	short y;
} pv_point_t, *pvf_point_ptr_t;


typedef	struct pv_color
{
	unsigned	blue	 : 8;
  	unsigned	green	 : 8;
  	unsigned	red   	 : 8;
  	unsigned	reserved : 8;
} pv_color_t, *pv_color_ptr_t;


typedef unsigned int pv_z_mask_t,  *pv_z_mask_ptr_t;

typedef unsigned int pv_z_t, *pv_z_ptr_t;

typedef struct pv_clip_rect
{
  	pv_point_t clipRectMin;
  	pv_point_t clipRectMax;
} pv_clip_rect_t, *pv_clip_rect_ptr_t;

typedef	struct pv_instruction
{
	unsigned	opCode 		:8;
	unsigned	dst		:4;
	unsigned	src1		:4;
	unsigned	src0		:4;
	unsigned	ccop		:2;
	unsigned	xyMask		:2;
	unsigned	xyMaskSense	:1;
	unsigned	zMaskEn		:1;
	unsigned	clipEn		:1;
	unsigned	ccEn		:1;
	unsigned	reserved	:4;
} pv_instruction_t, *pv_instruction_ptr_t;

typedef struct pv_xy_mask
{
  	unsigned int data[PV_XYMASK_DATA_LENGTH];
} pv_xy_mask_t, *pv_xy_mask_ptr_t;


typedef struct
{
    pv_point_t    v1;
    pv_point_t    v2;
} pv_line_t, *pv_line_ptr_t;


typedef struct
{
    pv_point_t    v1;
    pv_point_t    v2;
    unsigned int width;
} pv_wide_line_t, *pv_wide_line_ptr_t;

typedef struct
{
    pv_point_t    v1;
    pv_point_t    v2;
    pv_point_t    v3;
    pv_point_t    v4;
} pv_quad_t,  *pv_quad_ptr_t;


/*
 * Driver/Server information
 */

/*
 * If you add/change a component in struct _pvinfo, be sure to add/change
 * a macro defintion for the same component name in the list of pv__* macros
 * which follow.
 */
#define	PV_INFO_SIG_NUM			6
#define	PV_INFO_SIG_PMAP_FAULT		0
#define	PV_INFO_SIG_VA_PAGEIN		1
#define	PV_INFO_SIG_MODULE_ERROR	2
#define	PV_INFO_SIG_CLIPPING		3
#define	PV_INFO_SIG_PVA_TLB1_FAULT	4
#define	PV_INFO_SIG_PACKET_DONE		5

typedef struct _pvInfo {
  caddr_t		p_pvo;
  caddr_t		p_pvo_s;
  caddr_t		p_gcp;
  caddr_t		p_gcp_s;
  caddr_t		p_pva;
  caddr_t		p_pva_s;
  caddr_t		p_pv;
  caddr_t		p_pv_s;
  caddr_t		p_sram;
  caddr_t		p_sram_s;
  unsigned int		vram_size;		/* in pixels */
  unsigned int		sram_size;
  unsigned int		pixelmap_size;		/* in pixels */
  unsigned int		pixelmap_count;
  unsigned int		update_size;
  unsigned int		vis_pmap_size;
  unsigned int		monitor_rate;
  unsigned int		page_table_0_size;
  unsigned int		page_table_1_size;
  pv_register_t		phys_page_table_0;
  pv_register_t		phys_page_table_1;
  pv_pva_tlb_entry_t	*p_page_table_0;	/* PV shadow page table */
  pv_pva_tlb_entry_t	*p_page_table_1;	/* GCP shadow page table */
  unsigned int		ppn_dead_area;
  caddr_t		phys_dead_area;
  caddr_t		phys_clip_area;
  caddr_t		phys_info_area;
  caddr_t		phys_cmd_area;
  unsigned int		cmd_area_size;
  caddr_t		p_cmd_area;		/* GCP command ring buffer */
  caddr_t		p_clip_area;		/* Clipping data xfer area */
  caddr_t		p_dead_area;		/* Dead page */
  caddr_t		pagein_vaddr;
  unsigned short	pagein_page_count;
  unsigned short	pagein_dirty;
  unsigned int		signal_reasons[PV_INFO_SIG_NUM];
  unsigned int		module_error_reason;
#define	PV_MOD_ERR_SUCCESS			0
#define	PV_MOD_ERR_NO_HALT_ON_TLB_MISS		1
  unsigned int		gfpa_halts_on_countdown;
} pvInfo;

/*
 * Ioctl header.
 */

#define	PV_IOC_PAGEOUTIN_COUNT_MAX	16

typedef struct {
  short			count;
  short			clear_page_int;
  int			*p_tlb_index;
  pv_register_t		*p_out_tlb_entries;
  vm_offset_t		*p_out_page;
  pv_register_t		*p_out_format;
  pv_subpixel_pair_t	*p_out_xy_min;
  pv_subpixel_pair_t	*p_out_xy_max;
  pv_register_t		*p_out_offset;
  pv_register_t		*p_out_pixelmapid;
  pv_register_t		*p_in_tlb_entries;
  vm_offset_t		*p_in_page;
  pv_register_t		*p_in_format;
  pv_subpixel_pair_t	*p_in_xy_min;
  pv_subpixel_pair_t	*p_in_xy_max;
  pv_register_t		*p_in_offset;
  pv_register_t		*p_in_pixelmapid;
} pv_ioc_pageoutin;

#define	PV_ERRNO_RETRY_IMAGE		0x666

typedef struct {
  vm_offset_t	addr;
  u_long	size;
  int		hard_lockdown;
  pv_register_t	put_val;
} pv_ioc_image;

#define	PV_IOC_CNFG_GFPA_PUT		0
#define	PV_IOC_CNFG_GFPA_REASON		1
#define	PV_IOC_CNFG_GFPA_HANDSOFF	2
#define	PV_IOC_CNFG_GFPA_WRITE_REASON	3
#define	PV_IOC_CNFG_GFPA_COUNTDOWN	4
#define	PV_IOC_CNFG_NUM			5

#define	PV_IOC_TYPE_REG			0
#define	PV_IOC_TYPE_SRAM		1

#define	PV_IOC_CNFG_OFF_GLB_MIN		0x2000
#define	PV_IOC_CNFG_OFF_GLB_MAX		0x21fc
#define	PV_IOC_CNFG_OFF_FU0_MIN		0x0000
#define	PV_IOC_CNFG_OFF_FU0_MAX		0x00fc
#define	PV_IOC_CNFG_OFF_FU1_MIN		0x0400
#define	PV_IOC_CNFG_OFF_FU1_MAX		0x04fc
#define	PV_IOC_CNFG_OFF_FU2_MIN		0x0800
#define	PV_IOC_CNFG_OFF_FU2_MAX		0x08fc
#define	PV_IOC_CNFG_OFF_FU3_MIN		0x0c00
#define	PV_IOC_CNFG_OFF_FU3_MAX		0x0cfc

typedef struct {
  unsigned char	type[PV_IOC_CNFG_NUM];
  unsigned int	offset[PV_IOC_CNFG_NUM];
} pv_ioc_config;


typedef struct {
    char		windex;
    unsigned char	low;
    unsigned char	mid;
    unsigned char	high;
} pv_window_tag_cell;

typedef struct {
    short		left_start;
    short		left_ncells;
    short		right_start;
    short		right_ncells;
    pv_window_tag_cell	*p_left_cells;
    pv_window_tag_cell	*p_right_cells;
} pv_ioc_window_tag;

typedef struct {
    int			mode;
#define PV_IOC_STEREO_NONE		0
#define	PV_IOC_STEREO_24		1
#define	PV_IOC_STEREO_1212		2

    int			win_tag_start;
    int			win_tag_count;
} pv_ioc_stereo;

typedef struct {
    short         screen;
    short         cmd;
    union {
      unsigned long data;
      pv_ioc_pageoutin	pageoutin;
      pv_ioc_image	image;
      pv_ioc_config	config;
      pv_ioc_stereo	stereo;
      unsigned int	sig_mask;
#define	PV_IOC_SIG_PMAP_FAULT		(1<<PV_INFO_SIG_PMAP_FAULT)
#define	PV_IOC_SIG_VA_PAGEIN		(1<<PV_INFO_SIG_VA_PAGEIN)
#define	PV_IOC_SIG_MODULE_ERROR		(1<<PV_INFO_SIG_MODULE_ERROR)
#define	PV_IOC_SIG_CLIPPING		(1<<PV_INFO_SIG_CLIPPING)
#define	PV_IOC_SIG_PVA_TLB1_FAULT	(1<<PV_INFO_SIG_PVA_TLB1_FAULT)
#define	PV_IOC_SIG_PACKET_DONE		(1<<PV_INFO_SIG_PACKET_DONE)

      unsigned int	pid_mask;
#define	PV_IOC_PID_PV			0x00000001
#define	PV_IOC_PID_GCP			0x00000002
#define	PV_IOC_PID_PV_INDEX		0
#define	PV_IOC_PID_GCP_INDEX		1
#define	PV_IOC_PID_NUM			2

      vm_offset_t	touched_va;
      unsigned char	console_dcc;
      pv_ioc_window_tag	window_tag;
      pv_register_t	gcp_clock_control;
      pv_ioc_image	perf1;
      pv_ioc_image	perf2;
      struct {
	unsigned int		gfpa_pc;
	unsigned int		gfpa_halts_on_countdown;
      } start_gfpa;
    } info;
} pv_ioc;

#define PV_IOC_PRIVATE	_IOWR('w', (0|IOC_S), pv_ioc)

#define PV_IOC_MAP_OPTION		1
#define	PV_IOC_MAP_OPTION_SPARSE	2
#define	PV_IOC_GET_PID			3
#define	PV_IOC_SET_PID			4
#define	PV_IOC_GET_CONFIG		5
#define	PV_IOC_SET_CONFIG		6
#define	PV_IOC_GET_SIGNALS		7
#define	PV_IOC_SET_SIGNALS		8
#define	PV_IOC_PMAP_PAGEOUTIN		9
#define	PV_IOC_GET_IMAGE		10
#define	PV_IOC_PUT_IMAGE		11
#define	PV_IOC_GET_STEREO		12
#define	PV_IOC_SET_STEREO		13
#define	PV_IOC_TOUCHED_PAGE		14
#define	PV_IOC_GET_CONSOLE_DCC		15
#define	PV_IOC_SET_CONSOLE_DCC		16
#define	PV_IOC_WRITE_WINDOW_TAG		17
#define	PV_IOC_GET_GCP_CLOCK_CONTROL	18
#define	PV_IOC_SET_GCP_CLOCK_CONTROL	19
#define	PV_IOC_SYNC_ON_COUNT_DOWN	20
#define	PV_IOC_START_GFPA		21
#define	PV_IOC_HALT_GFPA		22
#define	PV_IOC_PERF3			23
#define	PV_IOC_PERF4			24
#define	PV_IOC_PERF5			25
#define	PV_IOC_PERF6			26
#define	PV_IOC_PERF7			27
#define	PV_IOC_PERF8			28
#define	PV_IOC_PERF9			29
#define	PV_IOC_PERF10			30
#define	PV_IOC_PERF1			31
#define	PV_IOC_PERF2			32

#endif /* _PV_H_ */
