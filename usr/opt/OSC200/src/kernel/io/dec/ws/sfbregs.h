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

/****************************************************************************
**                                                                          *
**                       COPYRIGHT (c) 1990, 1991 BY                        *
**              DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.               *
**			     ALL RIGHTS RESERVED                            *
**                                                                          *
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED  *
**  ONLY  IN  ACCORDANCE  WITH  THE  TERMS  OF  SUCH  LICENSE AND WITH THE  *
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR  ANY  OTHER  *
**  COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY  *
**  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY  *
**  TRANSFERRED.                                                            *
**                                                                          *
**  THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE  WITHOUT  NOTICE  *
**  AND  SHOULD  NOT  BE  CONSTRUED  AS  A COMMITMENT BY DIGITAL EQUIPMENT  *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE OR  RELIABILITY  OF  ITS  *
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.                 *
**                                                                          *
****************************************************************************/

/****************************************************************************
 *                 Smart Frame Buffer Register Definitions                  *
 ***************************************************************************/

#ifndef	_SFBREGS_H_
#define	_SFBREGS_H_

/* Modes that sfb can operate in. */
typedef enum {
    /* 000 */ SIMPLE,
    /* 001 */ OPAQUESTIPPLE,
    /* 010 */ OPAQUELINE,
    /* 011 */ unusedmode0,
    /* 100 */ unusedmode1,
    /* 101 */ TRANSPARENTSTIPPLE,
    /* 110 */ TRANSPARENTLINE,
    /* 111 */ COPY
} SFBMode;

/* Depths (bits/pixel) that sfb can operate upon. */
typedef enum {
    /* 00 */  SFBDEPTH8,
    /* 01 */  SFBDEPTH16,
    /* 10 */  SFBDEPTH32
} SFBDepth;

#define DISABLE_INTERRUPTS 0
#define ENABLE_INTERRUPTS  1

#if SFBBUSBITS == 32
typedef Pixel32     PixelWord;
typedef Bits32      CommandWord;
#elif SFBBUSBITS == 64
typedef Pixel64     PixelWord;
typedef Bits64      CommandWord;
#endif

typedef struct { PixelWord copy_pixel; } PixelCopy;
#define SFB_PAD( _sym )

#define SFBBUFFERWORDS      (SFBCOPYBITS * SFBPIXELBITS / SFBBUSBITS)


/* Command registers. */
typedef volatile struct {
    PixelCopy   buffer[SFBBUFFERWORDS];/* Port to read/write copy buffer    */
    PixelWord   foreground;     /* Foreground register (minimum 32 bits)    */
    SFB_PAD( _pad1 )
    PixelWord   background;     /* Background register (minimum 32 bits)    */
    SFB_PAD( _pad2 )
    PixelWord   planemask;      /* Planemask (minimum 32 bits)		    */
    SFB_PAD( _pad3 )
    CommandWord	pixelmask;      /* Pixel mask register			    */
    SFB_PAD( _pad4 )
    SFBMode     mode;		/* Hardware mode			    */
    SFB_PAD( _pad5 )
    unsigned int   rop;		/* Raster op for combining src, dst	    */
    SFB_PAD( _pad6 )
    int		shift;		/* -SFBALIGNMASK..+SFBALIGNMASK copy shift  */
    SFB_PAD( _pad7 )
    Pixel32     address;	/* Pixel address register		    */
    SFB_PAD( _pad8 )
    Bits32      bres1;		/* a1, e1				    */
    SFB_PAD( _pad9 )
    Bits32	bres2;		/* a2, e2				    */
    SFB_PAD( _pad10 )
    Bits32	bres3;		/* e, count				    */
    SFB_PAD( _pad11 )
    Bits32	brescont;	/* Continuation data for lines		    */
    SFB_PAD( _pad12 )
    SFBDepth	depth;		/* 8, 16, or 32 bits/pixel?		    */
    SFB_PAD( _pad13 )
    CommandWord	start;		/* Start operation if using address reg     */
    SFB_PAD( _pad14 )
    CommandWord clear_interrupt;/* Clear Interrupt register		    */
    SFB_PAD( _pad15 )
    Bits32 	test_register;	/* ??? 					    */
    SFB_PAD( _pad16 )
    Bits32 	refresh_count;  /* interval between refresh reads	    */
    SFB_PAD( _pad17 )
    Bits32 	horizontal_setup;/* horizontal video state machine	    */
    SFB_PAD( _pad18 )
    Bits32	vertical_setup; /* vertical video state machine		    */
    SFB_PAD( _pad19 )
    Bits32	base_address;	/* base row address for starting scan line  */
    SFB_PAD( _pad20 )
    CommandWord video_valid;	/* writes to video registers have completed */
    SFB_PAD( _pad21 )
    CommandWord enable_disable_interrupt; /* low order bit determines       */
    SFB_PAD( _pad22 )
    Bits32	tcclk_counter;	/* oscillator clock counters		    */
    SFB_PAD( _pad23 )
    Bits32	vidclk_counter; /* oscillator clock counters		    */
    SFB_PAD( _pad24 )
} SFBRec, *SFB;

/* 
 * We want the sfb display to remain sane even if someone steps on it,
 * so the following data structures are used to save and restore
 * critical pieces of state.
 */

struct sfbinfo {
	Bits32 depth;
	Bits32 refresh_count;
	Bits32 horizontal_setup;
	Bits32 vertical_setup;
	Bits32 base_address;
};

typedef struct {
	unsigned	deep : 1;
	unsigned	mbz0 : 1;
	unsigned	mask : 3;
	unsigned	block : 4;
	unsigned	col_size : 1;
	unsigned	sam_size : 1;
	unsigned	parity : 1;
	unsigned	write_en : 1;
	unsigned	ready : 1;
	unsigned	slow_dac : 1;
	unsigned	dma_size : 1;
	unsigned	sync_type : 1;
	unsigned	mbz1 : 15;
} SFBPlusDepth;

#define	SFBP_DEEP_DEEP_8PLANE		0
#define	SFBP_DEEP_DEEP_32PLANE		1

#define	SFBP_DEEP_MASK_4MB		0x00
#define	SFBP_DEEP_MASK_8MB		0x01
#define	SFBP_DEEP_MASK_16MB		0x03
#define	SFBP_DEEP_MASK_32MB		0x07

#define	SFBP_DEEP_PARITY_ODD		0
#define	SFBP_DEEP_PARITY_EVEN		1

#define	SFBP_DEEP_READY_ON_8		0
#define	SFBP_DEEP_READY_ON_2		1

#define	SFBP_DEEP_DMA_64		0
#define	SFBP_DEEP_DMA_128		1

typedef struct {
	unsigned	s_wr_mask : 8;
	unsigned	s_rd_mask : 8;
	unsigned	s_test : 3;
	unsigned	s_fail : 3;
	unsigned	d_fail : 3;
	unsigned	d_pass : 3;
	unsigned	z_test : 3;
	unsigned	z : 1;
} SFBPlusStencilMode;

#define	SFBP_SM_TEST_GEQ		0x00
#define	SFBP_SM_TEST_TRUE		0x01
#define	SFBP_SM_TEST_FALSE		0x02
#define	SFBP_SM_TEST_LS			0x03
#define	SFBP_SM_TEST_EQ			0x04
#define	SFBP_SM_TEST_LEQ		0x05
#define	SFBP_SM_TEST_GT			0x06
#define	SFBP_SM_TEST_NEQ		0x07

#define	SFBP_SM_RESULT_KEEP		0x00
#define	SFBP_SM_RESULT_ZERO		0x01
#define	SFBP_SM_RESULT_REPLACE		0x02
#define SFBP_SM_RESULT_INCR		0x03
#define	SFBP_SM_RESULT_DECR		0x04
#define	SFBP_SM_RESULT_INV		0x05

#define	SFBP_SM_Z_REPLACE		0
#define	SFBP_SM_Z_KEEP			1

typedef struct {
	unsigned	mode : 8;
	unsigned	visual : 3;
	unsigned	rotate : 2;
	unsigned	line : 1;
	unsigned	z16 : 1;
	unsigned	cap_ends : 1;
	unsigned	mbz : 16;
} SFBPlusMode;

#define	SFBP_MODE_MODE_SIMPLE			0x00
#define	SFBP_MODE_MODE_Z_SIMPLE			0x10
#define	SFBP_MODE_MODE_OPA_STIP			0x01
#define	SFBP_MODE_MODE_OPA_FILL			0x21
#define	SFBP_MODE_MODE_TRA_STIP			0x05
#define	SFBP_MODE_MODE_TRA_FILL			0x25
#define	SFBP_MODE_MODE_TRA_BLK_STIP		0x0d
#define	SFBP_MODE_MODE_TRA_BLK_FILL		0x2d
#define	SFBP_MODE_MODE_OPA_LINE			0x02
#define	SFBP_MODE_MODE_TRA_LINE			0x06
#define	SFBP_MODE_MODE_CINT_TRA_LINE		0x0e
#define	SFBP_MODE_MODE_CINT_TRA_DITH_LINE	0x2e
#define	SFBP_MODE_MODE_Z_OPA_LINE		0x12
#define	SFBP_MODE_MODE_Z_TRA_LINE		0x16
#define	SFBP_MODE_MODE_Z_CINT_OPA_LINE		0x1a
#define	SFBP_MODE_MODE_Z_SINT_OPA		0x5a
#define	SFBP_MODE_MODE_Z_CINT_OPA_DITH_LINE	0x3a
#define	SFBP_MODE_MODE_Z_CINT_TRA_LINE		0x1e
#define	SFBP_MODE_MODE_Z_SINT_TRA		0x5e
#define	SFBP_MODE_MODE_Z_CINT_TRA_DITH_LINE	0x3e
#define	SFBP_MODE_MODE_COPY			0x07
#define	SFBP_MODE_MODE_DMA_READ			0x17
#define	SFBP_MODE_MODE_DMA_READ_DITH		0x37
#define	SFBP_MODE_MODE_DMA_WRITE		0x1f

#define	SFBP_MODE_VISUAL_8_PACKED		0x00
#define	SFBP_MODE_VISUAL_8_UNPACKED		0x01
#define	SFBP_MODE_VISUAL_12_LOW			0x02
#define	SFBP_MODE_VISUAL_12_HIGH		0x06
#define	SFBP_MODE_VISUAL_24			0x03

#define	SFBP_MODE_PM_IS_PERS			0x00800000
#define	SFBP_MODE_ADDR_IS_NEW			0x00400000
#define	SFBP_MODE_BRES3_IS_NEW			0x00200000
#define	SFBP_MODE_COPY_WILL_DRAIN		0x00100000

typedef struct {
	unsigned	opcode : 4;
	unsigned	mbz : 4;
	unsigned 	visual : 2;
	unsigned	rotate : 2;
} SFBPlusRasterOp;

#define	SFBP_ROP_OP_CLEAR			0
#define	SFBP_ROP_OP_AND				1
#define	SFBP_ROP_OP_AND_REVERSE			2
#define	SFBP_ROP_OP_COPY			3
#define	SFBP_ROP_OP_AND_INVERTED		4
#define	SFBP_ROP_OP_NOOP			5
#define	SFBP_ROP_OP_XOR				6
#define	SFBP_ROP_OP_OR				7
#define	SFBP_ROP_OP_NOR				8
#define	SFBP_ROP_OP_EQUIV			9
#define	SFBP_ROP_OP_INVERT			10
#define	SFBP_ROP_OP_OR_REVERSE			11
#define	SFBP_ROP_OP_COPY_INVERTED		12
#define	SFBP_ROP_OP_OR_INVERTED			13
#define	SFBP_ROP_OP_NAND			14
#define	SFBP_ROP_OP_SET				15

#define	SFBP_ROP_VISUAL_8_PACKED		0x00
#define	SFBP_ROP_VISUAL_8_UNPACKED		0x01
#define	SFBP_ROP_VISUAL_12			0x02
#define	SFBP_ROP_VISUAL_24			0x03

#define	SFBP_INTR_VSYNC				0x00000001
#define	SFBP_INTR_SHIFT_ADDR			0x00000002
#define	SFBP_INTR_DMA_ERROR			0x00000004
#define	SFBP_INTR_PARITY_ERROR			0x00000008
#define	SFBP_INTR_TIMER				0x00000010
#define	SFBP_INTR_ALL				0x0000001f
#define	SFBP_INTR_ENABLE_SHIFT			16

#define	SFBP_RAMDAC_SETUP_HEAD			0x00000001
#define	SFBP_RAMDAC_SETUP_RW			0x00000002
#define	SFBP_RAMDAC_SETUP_C0			0x00000004
#define	SFBP_RAMDAC_SETUP_C1			0x00000008
#define	SFBP_RAMDAC_SETUP_C2			0x00000010
#define	SFBP_RAMDAC_463_ADDR_LOW		0
#define	SFBP_RAMDAC_463_ADDR_HIGH		(SFBP_RAMDAC_SETUP_C0)
#define	SFBP_RAMDAC_463_CMD_REG			(SFBP_RAMDAC_SETUP_C1)
#define	SFBP_RAMDAC_463_CMAP			(SFBP_RAMDAC_SETUP_C0|SFBP_RAMDAC_SETUP_C1)
#define	SFBP_RAMDAC_431_ADDR_LOW		0
#define	SFBP_RAMDAC_431_ADDR_HIGH		(SFBP_RAMDAC_SETUP_C0)
#define	SFBP_RAMDAC_431_CURS_RAM		(SFBP_RAMDAC_SETUP_C1)
#define	SFBP_RAMDAC_431_CMD_REG			(SFBP_RAMDAC_SETUP_C0|SFBP_RAMDAC_SETUP_C1)
#define	SFBP_RAMDAC_459_ADDR_LOW		0
#define	SFBP_RAMDAC_459_ADDR_HIGH		(SFBP_RAMDAC_SETUP_C0)
#define	SFBP_RAMDAC_459_CMD_CURS		(SFBP_RAMDAC_SETUP_C1)
#define	SFBP_RAMDAC_459_CMAP			(SFBP_RAMDAC_SETUP_C0|SFBP_RAMDAC_SETUP_C1)

#define	SFBP_RAMDAC_INTERF_WRITE_SHIFT		0
#define	SFBP_RAMDAC_INTERF_READ0_SHIFT		8
#define	SFBP_RAMDAC_INTERF_READ1_SHIFT		16

#define	SFBP_CUR_BASE_BASE_SHIFT		4
#define	SFBP_CUR_BASE_BASE_MASK			0x03f0
#define	SFBP_CUR_BASE_ROW_SHIFT			10
#define	SFBP_CUR_BASE_ROW_MASK			0xfc00

#define	SFBP_CUR_POS_X_SHIFT			0
#define	SFBP_CUR_POS_X_MASK			0x00000fff
#define	SFBP_CUR_POS_Y_SHIFT			12
#define	SFBP_CUR_POS_Y_MASK			0x00fff000

/* Command registers. */
typedef volatile struct {
    PixelCopy   buffer[SFBBUFFERWORDS];/* Port to read/write copy buffer    */

    PixelWord   foreground;     /* Foreground register (minimum 32 bits)    */
    PixelWord   background;     /* Background register (minimum 32 bits)    */
    PixelWord   planemask;      /* Planemask (minimum 32 bits)		    */
    Bits32	pixelmask;      /* Pixel mask register			    */
    SFBPlusMode	mode;		/* Hardware mode			    */
    SFBPlusRasterOp rop;	/* Raster op for combining src, dst	    */
    int		shift;		/* -SFBALIGNMASK..+SFBALIGNMASK copy shift  */
    Pixel32     address;	/* Pixel address register		    */

    Bits32      bres1;		/* a1, e1				    */
    Bits32	bres2;		/* a2, e2				    */
    Bits32	bres3;		/* e, count				    */
    Bits32	brescont;	/* Continuation data for lines		    */
    SFBPlusDepth depth;		/* 8, 16, or 32 bits/pixel?		    */
    Bits32	start;		/* Start operation if using address reg     */
    SFBPlusStencilMode stencil_mode; /* Stencil mode			    */
    Bits32 	pers_pixelmask;	/* Persistent pixelmask			    */

    Bits32 	cursor_base_address;  /* Cursor address in vram		    */
    Bits32 	horizontal_setup;/* horizontal video state machine	    */
    Bits32	vertical_setup; /* vertical video state machine		    */
#define	SFBP_VERT_STEREO_EN		0x80000000

    Bits32	base_address;	/* base row address for starting scan line  */
    Bits32	video_valid;	/* writes to video registers have completed */
    Bits32	cursor_xy;	/* Cursor position			    */
    Bits32	video_shift_addr;
    Bits32	intr_status;	/* Interrupt status 			    */

    Bits32	pixel_data;
    Bits32	red_incr;
    Bits32	green_incr;
    Bits32	blue_incr;
    Bits32	z_incr_low;
    Bits32	z_incr_high;
    Bits32	dma_address;
    Bits32	bres_width;

    Bits32	z_value_low;
    Bits32	z_value_high;
    Bits32	z_base_address;
    Bits32	address2;
    Bits32	red_value;
    Bits32	green_value;
    Bits32	blue_value;
    Bits32	_jnk12;

    Bits32	ramdac_setup;
    struct {
	Bits32		junk;
    } _junk[8*2-1];

    struct {
	Bits32		data;
    } slope_no_go[8];

    struct {
	Bits32		data;
    } slope[8];

    Bits32	bm_color_0;
    Bits32	bm_color_1;
    Bits32	bm_color_2;
    Bits32	bm_color_3;
    Bits32	bm_color_4;
    Bits32	bm_color_5;
    Bits32	bm_color_6;
    Bits32	bm_color_7;

    Bits32	c64_src;
    Bits32	c64_dst;
    Bits32	c64_src2;
    Bits32	c64_dst2;
    Bits32	_jnk45;
    Bits32	_jnk46;
    Bits32	_jnk47;
    Bits32	_jnk48;

    struct {
	Bits32		junk;
    } _junk2[8*3];

    Bits32	eprom_write;
    Bits32	_res0;
    Bits32	clock;
    Bits32	_res1;
    Bits32	ramdac;
    Bits32	_res2;
    Bits32	command_status;
    Bits32	command_status2;

} SFBPlusRec, *SFBPlus;


struct sfbplus_info {
	vm_offset_t	base;
	SFBPlus		asic;
	vm_offset_t	fb;
	size_t		fb_size;
	int		bt463_present;
	int		bits_per_pixel;
	SFBPlusDepth	depth;
	Bits32		head_mask;
	Bits32		refresh_count;
	Bits32		horizontal_setup;
	Bits32		vertical_setup;
	Bits32		base_address;
	caddr_t		info_area;
	vm_offset_t	virtual_dma_buffer;
	vm_offset_t	physical_dma_buffer;
	int		wt_min_dirty;
	int		wt_max_dirty;
	int		wt_dirty;
	sfbp_window_tag_cell
			wt_cell[16];	/* magic number */
	unsigned int	stereo_mode;
	unsigned int	do_parity_check;
};

#define	SFBP_USER_MAPPING_COUNT		4

typedef struct {
    vm_offset_t		fb_alias_increment;
    vm_offset_t		option_base;
    unsigned int	planemask;
    vm_offset_t		virtual_dma_buffer;
    vm_offset_t		physical_dma_buffer;
} sfbpInfo;

#endif	/* _SFBREGS_H_ */
