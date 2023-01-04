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
 *	@(#)$RCSfile: pmagbba.h,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/11/17 23:13:45 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

/************************************************************************
 *									*
 *			Copyright (c) 1992 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any  other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

#ifndef	PMAGBBA_DEFINED
#define	PMAGBBA_DEFINED

#ifdef __alpha
#define	PMAGBBA_DENSE_ROM_OFFSET	0x000000
#define	PMAGBBA_DENSE_SFB_ASIC_OFFSET	0x100000
#define	PMAGBBA_DENSE_BT459_OFFSET	0x1c0000
#define	PMAGBBA_DENSE_FB_OFFSET		0x200000

#define PMAGBBA_ROM_OFFSET		( PMAGBBA_DENSE_ROM_OFFSET )
#define PMAGBBA_SFB_ASIC_OFFSET		( PMAGBBA_DENSE_SFB_ASIC_OFFSET )
#define PMAGBBA_BT459_OFFSET	       	( PMAGBBA_DENSE_BT459_OFFSET )
#define PMAGBBA_FB_OFFSET		( PMAGBBA_DENSE_FB_OFFSET )

#define	PMAGDA_ROM_OFFSET		0x000000
#define	PMAGDA_SFBP_ASIC_OFFSET		0x100000
#define	PMAGDA_RAMDAC_SETUP_OFFSET	0x1000c0
#define	PMAGDA_RAMDAC_DATA_OFFSET	0x1001f0
#define	PMAGDA_XY_REG_OFFSET		0x100074
#define	PMAGDA_VALID_REG_OFFSET		0x100070

#define	PMAGDA_0_0_FB_OFFSET		0x00200000
#define	PMAGDA_0_0_FB_SIZE		0x00200000
#define	PMAGDA_0_1_FB_OFFSET		0x00400000
#define	PMAGDA_0_1_FB_SIZE		0x00400000
#define	PMAGDA_0_3_FB_OFFSET		0x00800000
#define	PMAGDA_0_3_FB_SIZE		0x00800000
#define	PMAGDA_1_3_FB_OFFSET		0x00800000
#define	PMAGDA_1_3_FB_SIZE		0x00800000
#define	PMAGDA_1_7_FB_OFFSET		0x01000000
#define	PMAGDA_1_7_FB_SIZE		0x01000000
#define	PMAGDA_INVALID_FB_OFFSET	0
#define	PMAGDA_INVALID_FB_SIZE		0

#else	/* __alpha */

#define PMAGBBA_FB_OFFSET		0x201000
#define PMAGBBA_BT459_OFFSET		0x1C0000
#define PMAGBBA_SFB_ASIC_OFFSET		0x100000
#define PMAGBBA_SFB_ASIC_GP0		0x140000
#define PMAGBBA_SFB_ASIC_GP1		0x180000
#define PMAGBBA_MAX_PROM_RGN            0x100000

#define	PMAGDA_ROM_OFFSET		0x000000
#define	PMAGDA_SFBP_ASIC_OFFSET		0x100000
#define	PMAGDA_RAMDAC_SETUP_OFFSET	0x1000c0
#define	PMAGDA_RAMDAC_DATA_OFFSET	0x1001f0
#define	PMAGDA_XY_REG_OFFSET		0x100074
#define	PMAGDA_VALID_REG_OFFSET		0x100070

#define	PMAGDA_0_0_FB_OFFSET		0x00200000
#define	PMAGDA_0_0_FB_SIZE		0x00200000
#define	PMAGDA_0_1_FB_OFFSET		0x00400000
#define	PMAGDA_0_1_FB_SIZE		0x00400000
#define	PMAGDA_0_3_FB_OFFSET		0x00800000
#define	PMAGDA_0_3_FB_SIZE		0x00800000
#define	PMAGDA_1_3_FB_OFFSET		0x00800000
#define	PMAGDA_1_3_FB_SIZE		0x00800000
#define	PMAGDA_1_7_FB_OFFSET		0x01000000
#define	PMAGDA_1_7_FB_SIZE		0x01000000
#define	PMAGDA_INVALID_FB_OFFSET	0
#define	PMAGDA_INVALID_FB_SIZE		0

#endif	/* __alpha */


extern void sfb_bot();
extern int sfb_attach();
extern void sfb_close();
extern void sfb_interrupt();
extern void sfb_enable_interrupt();
extern caddr_t sfb_459_init_closure();
extern int sfb_clear_screen();
extern int sfb_scroll_screen();
extern int sfb_blitc();
extern int sfb_ioctl();
extern int sfb_map_unmap_screen();


extern void sfbp_bot();
extern int sfbp_attach();
extern void sfbp_close();
extern void sfbp_interrupt();
extern caddr_t sfbp_dummy_caddrt();
extern int sfbp_dummy_int();
extern caddr_t sfbp_init_closure();
extern int sfbp_clear_screen();
extern int sfbp_scroll_screen();
extern int sfbp_blitc();
extern int sfbp_ioctl();
extern int sfbp_map_unmap_screen();

#define Pixel8      unsigned char
#define Pixel32     unsigned int

#define Bits32      unsigned int
#define Bits8       unsigned char

#define Bool8       unsigned char
#define Bool        int

/*
 *  ws_prom_map is used for mapping and unmapping the address space
 *  of a flash eeprom. In the case of mapping the size and offset
 *  arguments are used to specify the desired prom address space.
 *  A user process virtual address is returned in pm_vaddr and the
 *  corresponding kernel virtual address in pm_svaddr for verification
 *  of mapped space. In unmapping, pm_vaddr specifies the virtual
 *  address to deallocate.  Upon completion of unmapping pm_vaddr
 *  and pm_svaddr are set NULL. 
#ifdef __alpha
 * "screen" and "cmd" should probably not be 16-bit shorts.
#endif
 */ 
   
typedef struct {
    short screen;		/* standard interface		  */
    short cmd; 			/* ioctl differentiator		  */
    caddr_t pm_vaddr;		/* user proc. virtual addr	  */
    caddr_t pm_svaddr;		/* system virtual addr		  */
    size_t pm_offset;		/* position relative to base addr */
    size_t pm_size;		/* size of mapped area 		  */
} ws_prom_map;

#define SFB_SUCCESS		0
#define SFB_ERROR	       -1	

#define PROM_MAP_OPS  		_IOWR('w', (0|IOC_S), ws_prom_map)
#define CXT_MAP_PROM		1
#define CXT_UNMAP_PROM  	2

#define	SFBP_IOCTL_PRIVATE		_IOWR('w', (0|IOC_S), sfbp_ioc)
#define	SFBP_IOC_LOAD_WINDOW_TAGS	0
#define	SFBP_IOC_ENABLE_DMA_OPS		1
#define	SFBP_IOC_SET_STEREO_MODE	2
#define	SFBP_IOC_GET_STEREO_MODE	3

typedef struct {
    char		windex;
    unsigned char	low;
    unsigned char	mid;
    unsigned char	high;
} sfbp_window_tag_cell;

typedef struct {
    short		ncells;
    short		start;
    sfbp_window_tag_cell *p_cells;
} sfbp_ioc_window_tag;

typedef struct {
    short		screen;
    short		cmd;
    union {
	sfbp_ioc_window_tag	window_tag;
	unsigned int		stereo_mode;
#define	SFBP_IOC_STEREO_NONE		0
#define	SFBP_IOC_STEREO_24		1

    } data;
} sfbp_ioc;

#endif	/* PMAGBBA_DEFINED */

