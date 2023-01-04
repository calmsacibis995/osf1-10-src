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
#ifndef _PVINFO_H_
#define _PVINFO_H_

#ifndef NDEPTHS
#define NDEPTHS 1			/* all current hardware just has one */
#define NVISUALS 1
#endif /* NDEPTHS */

typedef struct _pv_info {
    ws_screen_descriptor screen;	/* MUST be first!!! */
    ws_depth_descriptor depth[NDEPTHS];
    ws_visual_descriptor visual[NVISUALS];
    ws_cursor_functions cf;
    ws_color_map_functions cmf;
    ws_screen_functions sf;
    int		(*attach)();		/* called @ attach time (if defined) */
    int		(*bootmsg)();		/* boot configuration message */
    int		(*map)();		/* Map the screen. */
    void	(*interrupt)();		/* called at interrupt (if defined) */
    int		(*setup)();		/* Setup procedure. */
    int		(*vm_hook)();		/* VM hook. */
    struct controller *ctlr;		/* Need this for special tc ops */
    caddr_t		p_gcp;
    caddr_t		p_gcp_s;
    caddr_t		p_pva;
    caddr_t		p_pva_s;
    caddr_t		p_pv;
    caddr_t		p_pv_s;
    caddr_t		p_sram;
    caddr_t		p_sram_s;
    u_int 		text_foreground;	/* Text foreground color. */
    u_int		text_background;	/* Text background color. */
    /*
     * stuff above this line gets initialized from px_types[] 
     */
    caddr_t		p_pvo;
    caddr_t		p_pvo_s;
    unsigned int	vram_size;
    unsigned int	sram_size;
    unsigned int	pixelmap_size;
    unsigned int	pixelmap_count;
    unsigned int	update_size;
    unsigned int	vis_pmap_size;
    unsigned int	page_table_0_size;
    unsigned int	page_table_1_size;
    pv_register_t	phys_page_table_0;
    pv_register_t	phys_page_table_1;
    pv_pva_tlb_entry_t	*p_page_table_0;	/* PV shadow page table */
    pv_pva_tlb_entry_t	*p_page_table_1;	/* GCP shadow page table */
    unsigned int	ppn_dead_area;
    caddr_t		phys_dead_area;
    caddr_t		phys_clip_area;
    caddr_t		phys_info_area;
    caddr_t		phys_cmd_area;
    caddr_t		phys_packet_area;
    unsigned int	cmd_area_size;
    caddr_t		p_cmd_area;		/* GCP command ring buffer */
    caddr_t		p_clip_area;		/* Clipping data xfer area */
    caddr_t		p_info_area;		/* info block area */
    caddr_t		p_packet_area;		/* PV packet area */
    caddr_t		p_dead_area;		/* Dead page */
    caddr_t 		dev_closure;		/* Device-specific closure. */
    pv_register_t	last_intr_status;	/* last interrupt status */
    pv_render_config_t	render_config;		/* current PV config reg */
    volatile int	image_stlb_lock;
    volatile int	pmap_stlb_lock;

#define	PV_INFO_SLEEP_PV_DMA		0
#define	PV_INFO_SLEEP_RENDER		1
#define	PV_INFO_SLEEP_IMAGE		2
#define	PV_INFO_SLEEP_GFPA		3
#define	PV_INFO_SLEEP_NUM		4
    unsigned int	sleep_chan[PV_INFO_SLEEP_NUM];
    unsigned long	state;
#define	PV_INFO_STATE_VALID		0x00000001
#define	PV_INFO_STATE_GCP_VALID		0x00000002
#define	PV_INFO_STATE_SAVED_GTLB_VALID	0x00000004
    /*
     * Keep the PV config register around
     */
    pv_render_imp_rev_t	render_imp_rev;
    /*
     * pid and sig_mask are used to direct delivery of signals to
     * the owning process.  They are controlled by device-private ioctl's.
     */
    struct proc		*p_proc[PV_IOC_PID_NUM];
    pid_t		pid[PV_IOC_PID_NUM];
    unsigned long	sig_mask[PV_IOC_PID_NUM];
    /*
     * gcp_reg_type, gcp_reg_offset, and gcp_reg define the various
     * software registers (put and get pointers, etc.) used by the host,
     * driver, and gfpa.  These are also maintained by the driver.
     */
    pv_register_t	gcp_clock_control;
    unsigned char	gcp_reg_type[PV_IOC_CNFG_NUM];
    unsigned int	gcp_reg_offset[PV_IOC_CNFG_NUM];
    vm_offset_t		gcp_reg[PV_IOC_CNFG_NUM];
    volatile int	gcp_count_down_event;
    /*
     * For 'soft' lockdowns during image operations, save the locked
     * down address range here.  Also bits for error status and to
     * enable VA range checking.  For hard lockdowns, we still use the
     * first and last vpn.
     */
    unsigned long	image_check_va_range;
    unsigned long	image_hardlock_in_progress;
    unsigned long	image_failure;
    unsigned int	image_va_first_vpn;
    unsigned int	image_va_last_vpn;
    /*
     * Interrupt parity.  We need to take two interrupts for vsync
     * operations
     */
    unsigned int	intr_parity;
    /*
     * During Put/GetImage, it may be necessary to service a pixelmap
     * fault.  Tuck the required PV TLB entries into this area.
     */
#define	PV_INFO_SAVED_PV_TLB_MAX	512
    pv_pva_tlb_entry_t	saved_tlb[PV_INFO_SAVED_PV_TLB_MAX];
    pv_graphic_tlb_t	saved_graphic_tlb;
    /*
     * DCC information.  We will have a default value which can be
     * overridden with an ioctl.  This value is placed in the high byte
     * to force treatment as a truecolor value
     */
    unsigned char	dcc_default;
    unsigned char	dcc_user;
    /*
     * Video on/off state.  Somewhat parallels that kept in the
     * Brooktree code.
     */
    char		video_on_off_dirty;
    char		video_on_off;
    /*
     * Window tag information
     */
    char		wt_dirty;
    char		wt_left_min_dirty;
    char		wt_left_max_dirty;
    char		wt_right_min_dirty;
    char		wt_right_max_dirty;
    pv_window_tag_cell	wt_left_cell[BT463_WINDOW_TAG_COUNT];
    pv_window_tag_cell	wt_right_cell[BT463_WINDOW_TAG_COUNT];
    /*
     * State for maintaining stereo modes
     */
    unsigned int	stereo_mode;
#define	PV_INFO_STEREO_NONE		0
#define	PV_INFO_STEREO_24		1
#define	PV_INFO_STEREO_1212		2

    int			stereo_wtag_start;
    int			stereo_wtag_count;
    /*
     * Safe area for data used in pageoutin operations
     */
    pv_register_t	poi_out_tlb_entries[PV_IOC_PAGEOUTIN_COUNT_MAX];
    pv_register_t	poi_out_format[PV_IOC_PAGEOUTIN_COUNT_MAX];
    pv_register_t	poi_out_offset[PV_IOC_PAGEOUTIN_COUNT_MAX];
    pv_register_t	poi_out_pixelmapid[PV_IOC_PAGEOUTIN_COUNT_MAX];
    pv_register_t	poi_in_tlb_entries[PV_IOC_PAGEOUTIN_COUNT_MAX];
    pv_register_t	poi_in_format[PV_IOC_PAGEOUTIN_COUNT_MAX];
    pv_register_t	poi_in_offset[PV_IOC_PAGEOUTIN_COUNT_MAX];
    pv_register_t	poi_in_pixelmapid[PV_IOC_PAGEOUTIN_COUNT_MAX];
    pv_subpixel_pair_t	poi_out_xy_min[PV_IOC_PAGEOUTIN_COUNT_MAX];
    pv_subpixel_pair_t	poi_out_xy_max[PV_IOC_PAGEOUTIN_COUNT_MAX];
    pv_subpixel_pair_t	poi_in_xy_min[PV_IOC_PAGEOUTIN_COUNT_MAX];
    pv_subpixel_pair_t	poi_in_xy_max[PV_IOC_PAGEOUTIN_COUNT_MAX];
    int			poi_count;
    int			poi_clear_page_int;
    int			poi_tlb_index[PV_IOC_PAGEOUTIN_COUNT_MAX];
    vm_offset_t		poi_out_page[PV_IOC_PAGEOUTIN_COUNT_MAX];
    vm_offset_t		poi_in_page[PV_IOC_PAGEOUTIN_COUNT_MAX];
    /*
     * GFPA VM DMA miss restart information
     */
    unsigned int	gfpa_pc_valid;
    pv_register_t	gfpa_pc;
    unsigned int	gfpa_is_running;
    /*
     * Monitor information
     */
    unsigned int	monitor_rate;
    /*
     * Driver/Server communication area
     */
    pvInfo		*pv;
} pv_info;


#define PV_BASE(C)		(((pv_info *)(C))->p_pvo)
#define	PV_BASES(C)		(((pv_info *)(C))->p_pvo_s)

#define PV_SILENT	0
#define PV_CONSOLE	1		/* allow output (at all) on SLU3 */
#define PV_PSST		2
#define PV_TERSE	3
#define PV_TALK		4
#define PV_YAK		5
#define PV_GAB		7
#define PV_BLAB		10
#define PV_YOW		13
#define PV_NEVER	99
#define PV_DEBUGGING	PV_TALK		/* default debug level */
#define PV_PANIC	PV_SILENT	/* msgs allowed when panic'ing */

#define PV_NODEBUG			/* define this to compile out */
					/* debugging code */
#ifdef  PV_NODEBUG
#	define PV_DEBUG(L,S)
#else
#	define PV_DEBUG(L,S)	if ((L) <= _pv_debug) { S }
#endif /* PV_NODEBUG */

#endif /* _PVINFO_H_ */
