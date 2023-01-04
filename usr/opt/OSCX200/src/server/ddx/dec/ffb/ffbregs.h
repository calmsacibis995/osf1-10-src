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
 * @(#)$RCSfile: ffbregs.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:17:15 $
 */
/*
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
 *                 Smart Frame Buffer+ Register Definitions                 *
 ***************************************************************************/

#define FIRSTREG_ADDRESS    0x100000

/* Modes that ffb can operate in. */
typedef enum {
    /* 000000 */ SIMPLE						= 0x00,
    /* 000001 */ OPAQUESTIPPLE					= 0x01,
    /* 000010 */ OPAQUELINE					= 0x02,
    /* 000101 */ TRANSPARENTSTIPPLE				= 0x05,
    /* 000110 */ TRANSPARENTLINE				= 0x06,
    /* 000111 */ COPY						= 0x07,
    /* 001101 */ TRANSPARENTBLOCKSTIPPLE			= 0x0d,
    /* 001110 */ COLORINTERPTRANSPARENTNONDITHERLINE		= 0x0e,
    /* 001111 */ WICKEDFASTCOPY					= 0x0f,
    /* 010000 */ ZBUFFERSIMPLE					= 0x10,
    /* 010010 */ ZBUFFEROPAQUELINE				= 0x12,
    /* 010110 */ ZBUFFERTRANSPARENTLINE				= 0x16,
    /* 010111 */ DMAREADCOPY					= 0x17,
    /* 011010 */ ZBUFFEROPAQUECOLORINTERPNONDITHERLINE		= 0x1a,
    /* 011110 */ ZBUFFERTRANSPARENTCOLORINTERPNONDITHERLINE	= 0x1e,
    /* 011111 */ DMAWRITECOPY					= 0x1f,
    /* 100001 */ OPAQUEFILL					= 0x21,
    /* 100101 */ TRANSPARENTFILL				= 0x25,
    /* 101101 */ BLOCKFILL					= 0x2d,
    /* 101110 */ COLORINTERPTRANSPARENTDITHERLINE		= 0x2e,
    /* 110111 */ DMAREADCOPYDITHER 				= 0x37,
    /* 111010 */ ZBUFFEROPAQUECOLORINTERPDITHERLINE		= 0x3a,
    /* 111110 */ ZBUFFERTRANSPARENTCOLORINTERPDITHERLINE	= 0x3e,
    /*1001110 */ SEQUENTIALINTERPTRANSPARENTLINE		= 0x4e,
    /*1011010 */ ZBUFFERSEQUENTIALINTERPOPAQUELINE		= 0x5a,
    /*1011110 */ ZBUFFERSEQUENTIALINTERPTRANSPARENTLINE		= 0x5e
} FFBMode;

#define DISABLE_INTERRUPTS 0
#define ENABLE_INTERRUPTS  1

#if FFBBUSBITS == 32
typedef Pixel32     PixelWord;
typedef Bits32	    CommandWord;
#elif FFBBUSBITS == 64
typedef Pixel64     PixelWord; 
typedef Bits64      CommandWord;
#endif


/* Command registers. */
typedef volatile struct {
    /* 0x000 */
    PixelWord   buffer[FFBBUFFERWORDS];/* Port to read/write copy buffer    */

    /* 0x020 */
    PixelWord   foreground;     /* Foreground register (minimum 32 bits)    */
    PixelWord   background;     /* Background register (minimum 32 bits)    */
    PixelWord   planemask;      /* Planemask (minimum 32 bits)		    */
    CommandWord	pixelmask;      /* Pixel mask register			    */
    FFBMode     mode;		/* Hardware mode			    */
    unsigned    rop;		/* Raster op, dst depth and rotation	    */
    int		shift;		/* -8..+7 copy shift			    */
    Pixel32     address;	/* Pixel address register		    */

    /* 0x040 */
    Bits32      bres1;		/* a1, e1				    */
    Bits32	bres2;		/* a2, e2				    */
    Bits32	bres3;		/* e, count				    */
    Bits32	brescont;	/* Continuation data for lines		    */
    Bits32      deep;		/* Bits/pixel and other juicy stuff         */
    CommandWord	start;		/* Start operation if using address reg     */
    Bits32	stencil;        /* Stencil Mode register 		    */
    CommandWord persistent_pixelmask; /* Persistent pixelmask register 	    */

    /* 0x060 */
    Pixel32     cursor_base;    /* cursor base address 			    */
    Bits32      horiz_ctl;      /* horizontal control 			    */
    Bits32      vert_ctl;       /* vertical control		  	    */
    Bits32      video_base;	/* video base address		            */
    Bits32      video_valid;	/* video valid  		            */
    Bits32 	cursor;		/* cursor xy register 			    */
    Bits32      video_shift;    /* video shift address			    */
    CommandWord int_status;     /* Interrupt Status register		    */
 
    /* 0x080 */
    CommandWord ffbdata;	/* data					    */
    Bits32      red_incr;	/* red increment			    */
    Bits32      green_incr;     /* greeen increment			    */
    Bits32      blue_incr;	/* blue increment			    */
    Bits32      z_fr_incr;	/* Z fractional increment      		    */
    Bits32      z_wh_incr;      /* Z while increment			    */
    Bits32      dma_addr;	/* dma base address			    */
    Bits32      breswidth;	/* Bresenham width			    */

    /* 0x0a0 */
    Bits32      z_fr_value;     /* z fractional value			    */
    Bits32      z_wh_value;	/* z whole value			    */
    Bits32      z_base;		/* z base address			    */
    Pixel32     address_alias;  /* address				    */
    Bits32      red;		/* red value				    */
    Bits32      green;		/* green value				    */
    Bits32      blue; 		/* blue value				    */
    Bits32	span_width;	/* alias for slope_dx_gt_dy		    */

    /* 0x0c0 */
    Bits32      ramdac_setup;   /* Ramdac setup reg			    */
    Bits32	unused0[7];   

    /* 0x0e0 */
#ifdef SOFTWARE_MODEL
    Bits32      unused1[7];
    Bits32      bogus_dma_high; /* Bogus high 32 bits of virtual DMA addr   */
#else
    Bits32      unused1[8];
#endif
    
    /* 0x100 */
    CommandWord sng_ndx_lt_ndy; /* slope no go (|-dx| < |-dy|)              */
    CommandWord sng_ndx_lt_dy;  /* slope no go (|-dx| < |+dy|)              */
    CommandWord sng_dx_lt_ndy;  /* slope no go (|+dx| < |-dy|)              */
    CommandWord sng_dx_lt_dy;   /* slope no go (|+dx| < |+dy|)              */
    CommandWord sng_ndx_gt_ndy; /* slope no go (|-dx| > |-dy|)              */
    CommandWord sng_ndx_gt_dy;  /* slope no go (|-dx| > |+dy|)              */
    CommandWord sng_dx_gt_ndy;  /* slope no go (|+dx| > |-dy|)              */
    CommandWord sng_dx_gt_dy;   /* slope no go (|+dx| > |+dy|)              */

    /* 0x120 */
    CommandWord slope_ndx_lt_ndy; /* slope (|-dx| < |-dy|)                  */
    CommandWord slope_ndx_lt_dy;  /* slope (|-dx| < |+dy|)                  */
    CommandWord slope_dx_lt_ndy;  /* slope (|+dx| < |-dy|)                  */
    CommandWord slope_dx_lt_dy;   /* slope (|+dx| < |+dy|)                  */
    CommandWord slope_ndx_gt_ndy; /* slope (|-dx| > |-dy|)                  */
    CommandWord slope_ndx_gt_dy;  /* slope (|-dx| > |+dy|)                  */
    CommandWord slope_dx_gt_ndy;  /* slope (|+dx| > |-dy|)                  */
    CommandWord slope_dx_gt_dy;   /* slope (|+dx| > |+dy|)                  */

    /* 0x140 */
    PixelWord   color0;         /* block mode color 0 			    */
    PixelWord   color1;         /* block mode color 1			    */
    PixelWord   color2;         /* block mode color 2			    */
    PixelWord   color3;         /* block mode color 3			    */
    PixelWord   color4;         /* block mode color 4			    */
    PixelWord   color5;         /* block mode color 5			    */
    PixelWord   color6;         /* block mode color 6			    */
    PixelWord   color7;         /* block mode color 7			    */

    /* 0x160 */
    Pixel32     copy64src0;	/* copy 64 src register			    */
    Pixel32     copy64dst0;     /* copy 64 dst register 		    */
    Pixel32     copy64src1;     /* copy 64 src register alias	            */
    Pixel32     copy64dst1;     /* copy 64 dst register alias	            */
    Pixel32     copy64src2;     /* copy 64 src register alias	            */
    Pixel32     copy64dst2;     /* copy 64 dst register alias	            */
    Pixel32     copy64src3;     /* copy 64 src register alias	            */
    Pixel32     copy64dst3;     /* copy 64 dst register alias	            */

    /* 0x180 */
    Bits32	unused2[24];	

    /* 0x1e0 */
    Bits32	eprom_write;    /* EPROM Write				    */
    Bits32	reserved1;	/* reserved 				    */
    Bits32	clock;		/* Clock				    */
    Bits32	reserved2;	/* reserved				    */
    Bits32 	ramdac_int;	/* Ramdac Interface			    */
    Bits32      reserved3;	/* reserved				    */
    Bits32      command_status; /* command status			    */
    Bits32      reserved4;      /* reserved				    */
} FFBRec, *FFB;

#ifndef _MACH_ALPHA_VM_TYPES_H_
#define _MACH_ALPHA_VM_TYPES_H_
typedef Pixel8		*vm_offset_t;
#endif

typedef struct {
    vm_offset_t		fb_alias_increment;
    vm_offset_t		option_base;
    Bits32		planemask;
    vm_offset_t		virtual_dma_buffer;
    vm_offset_t		physical_dma_buffer;
} FFBInfoRec, *FFBInfo;


/*
 * HISTORY
 */
