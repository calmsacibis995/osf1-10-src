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
#ifndef lint
static char *rcsid = "@(#)$RCSfile: pv.c,v $ $Revision: 1.1.9.4 $ (DEC) $Date: 1993/11/23 21:29:02 $";
#endif

/************************************************************************
 *									*
 *			Copyright (c) 1990 by				*
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
#define _PV_C_
/************************************************************************
 *
 * 00-Dec-1991	Lloyd Wheeler
 *		Tweaked includes for OSF kernel build environment.
 *		Modified vm-related calls to work in OSF environment and
 *		otherwise 'OSF-ized'.
 *
 * 13-Aug-90	Sam Hsu
 *		Created, based on gq.c and pa.c
 *
 ************************************************************************/

#include <machine/pmap.h>		/* get vm stuff, inc. svatophys() */
#include <machine/hal/kn15aa.h>	        /* get DENSE macro */
#include <mach/kern_return.h>
#include <data/ws_data.c>
#include <data/bt_data.c>
#include <data/pv_data.c>
#include <sys/vmmac.h>		/* btop() */
#include <sys/proc.h>
#include <sys/user.h>
#include <sys/errno.h>
#include <sys/tty.h>
#include <io/dec/tc/tc.h>


/*
 * Defines
 */
#ifndef PDEVCMD_DMA
#define	PDEVCMD_DMA			2
#endif

/*
 * Enable workarounds for various module and chip bugs
 */
#define	USE_VSLOCK_EMULATION
/*
#define	DONT_CLEAR_GFPA_REASON
*/
/*
#define	DONT_DO_PVA_RESET
*/

/*
 * If we do a lockout in vsync, we can't do 1212 mode because we
 * need to read the PV status register to select which tags table
 * to load.
 */
/*
#define	DO_LOCKOUT_IN_VSYNC
*/
/*
#define	DO_LOCKOUT_IN_INVALIDATION
*/

#define	DO_THREE_STATE_VSYNC

/*
 * Note: PID2 console *always* uses 8192 bytes of working storage.
 * It can be in either host or sram.
 */

#define IMAGE_FAILS_ON_SINGLE_INVALIDATE


#define	DO_NEW_STYLE_INVALIDATE

/*
#define	PRELOAD_PVA_AFTER_MISS
*/
/*
#define	NO_CONSOLE_IF_GFPA_RUNS
*/
/*
#define	BUILD_PERFORMANCE_IOCTLS
*/

#define	DO_COPYIN_ON_TOUCHED_PAGE

#define	GFPA_IS_SYNCHRONOUS

#ifdef	GFPA_IS_SYNCHRONOUS
/*
 * GCP Clock Control Register value
 *
 * 0x200		Use external clock
 * 0x80			2x
 * 0xa0			2.5x
 */
#define	MAGIC_GCP_CLOCK_VALUE		0x200
#else	/* GFPA_IS_SYNCHRONOUS */
#define	MAGIC_GCP_CLOCK_VALUE		0x200
/*
#define	MAGIC_GCP_CLOCK_VALUE		0x89
#define	MAGIC_GCP_CLOCK_VALUE		0x80
#define	MAGIC_GCP_CLOCK_VALUE		0xa0
#define	MAGIC_GCP_CLOCK_VALUE		0x9b
*/
#endif /* GFPA_IS_SYNCHRONOUS */
#define	MAGIC_PVA_COUNTDOWN_VALUE	47
#define	MAGIC_GCP_CLOCK_TRY_COUNT	200000


/*
 * Diagnostics
 */
/*
 * SPL
 */
#define	PV_RAISE_SPL()			splhigh();
#define	PV_LOWER_SPL(_v)		splx(_v);

/*
 * Shift values for forming addresses
 */
#define	SHIFT_FOR_VHOST_DMA		2
#define	SHIFT_FOR_PHOST_DMA		2
#define	SHIFT_FOR_PVBUS_DMA		2
#define	SHIFT_FOR_SPT_BASE		2

/*
 * Handsoff value
 */
#define	HANDSOFF_VALUE			0x40000000

/*
 * Default value for DCC bits
 */
#define	TRUECOLOR_DCC_VALUE		3

/*
 * Define the user's mapping
 */
#define	USER_VA_BASE			(0x200000000L)
#define	USER_VA_DENSE_OFFSET		(0x00000000L)
#define	USER_VA_MISC_OFFSET		(0x00800000L)
#define	USER_VA_SPARSE_OFFSET		(0x01000000L)
#define	USER_VA_UNIT_OFFSET		(0x02000000L)

#ifdef	USE_VSLOCK_EMULATION
#define	pv_vslock		lcl_vslock
#define	pv_vsunlock		lcl_vsunlock

static int lcl_vslock();
static int lcl_vsunlock();
static int lcl_kern_return_xlate();
#else
#define	pv_vslock		vslock
#define	pv_vsunlock		vsunlock
#endif

#if 0
#ifndef	PMAP_COPROC_INVALIDATE_STLB
#define PMAP_COPROC_INVALIDATE_STLB  0
#define PMAP_COPROC_EXIT             1
#endif
#endif

/*
 * Define various invalid tlb entries
 */
#define	INVALID_STLB_ENTRY( _pvp, _index, _entry ) {		\
(_entry)->ppn = (_pvp)->ppn_dead_area; (_entry)->pid = PV_PVA_PID_INVALID;\
(_entry)->we = 0; (_entry)->mru = 0; (_entry)->vpn = (_index); }

#define	INVALID_PVA_TLB_ENTRY( _pvp, _index, _entry ) {	\
(_entry)->ppn = (_pvp)->ppn_dead_area; (_entry)->pid = PV_PVA_PID_INVALID;\
(_entry)->we = 0; (_entry)->mru = 0; (_entry)->vpn = (_index); }

/*
 * Arguments for the invalidation routines
 */
#define	INVALIDATE_PAGER		0
#define	INVALIDATE_SERVER		1
#define	INVALIDATE_ALL			2

/*
 * Number of usable window tags in the 463
 */
#define	WINDOW_TAG_COUNT		(BT463_WINDOW_TAG_COUNT - 2)

/*
 * Put a thread to sleep while things happen
 */
#define	SLEEP_ON_PV_DMA(_pvp)	{				\
	(_pvp)->sleep_chan[PV_INFO_SLEEP_PV_DMA] = 1;		\
	while ( (_pvp)->sleep_chan[PV_INFO_SLEEP_PV_DMA] ) {	\
	    sleep( &(_pvp)->sleep_chan[PV_INFO_SLEEP_PV_DMA], TTIPRI ); } }

#define	ISLEEP_ON_PV_DMA(_pvp, _pri, _msg, _spl)	{	\
	int __status;						\
	(_pvp)->sleep_chan[PV_INFO_SLEEP_PV_DMA] = 1;		\
	if ((__status=tsleep( &(_pvp)->sleep_chan[PV_INFO_SLEEP_PV_DMA], \
				_pri, _msg, 0 ) ) ) {		\
	    PV_LOWER_SPL( _spl );				\
	    return (__status); } } 

#define	SLEEP_ON_IMAGE(_pvp)	{				\
	(_pvp)->sleep_chan[PV_INFO_SLEEP_IMAGE] = 1;		\
	while ( (_pvp)->sleep_chan[PV_INFO_SLEEP_IMAGE] ) {	\
	    sleep( &(_pvp)->sleep_chan[PV_INFO_SLEEP_IMAGE], TTIPRI ); } }

#define	ISLEEP_ON_GFPA(_pvp, _pri, _msg, _spl)	{		\
	int __status;						\
	(_pvp)->sleep_chan[PV_INFO_SLEEP_GFPA] = 1;		\
	if ((__status=tsleep( &(_pvp)->sleep_chan[PV_INFO_SLEEP_GFPA], \
				_pri, _msg, 0 ) ) ) {		\
	    PV_LOWER_SPL( _spl );				\
	    return (__status); } }


/*
 * Load and store of software registers
 */
#define	LOAD_SOFTWARE_REG( _pvp, _type, _val ) {			\
    pv_register_t *_p_tmp = (pv_register_t *) (_pvp)->gcp_reg[_type];	\
    _val = *_p_tmp; }

#define	STORE_SOFTWARE_REG( _pvp, _type, _val ) {			\
    pv_register_t *_p_tmp = (pv_register_t *) (_pvp)->gcp_reg[_type];	\
    *_p_tmp = (pv_register_t) (_val); }

/*
 * Interrupt status manipulations
 */
#define	PV_INT_STEREO		0x01
#define	PV_INT_VSYNC		0x02
#define	PV_INT_PAGE		0x04
#define	PV_INT_VM		0x08
#define	PV_INT_RENDER		0x10
#define	PV_INT_PACKET		0x20
#define	PV_INT_STALL		0x20

#define	PV_INT_ENABLE( _pvp, _mask ) {					\
    pv_render_implement_t *_p_rend_imp = PV_REND_IMP_BASE( _pvp );	\
    pv_render_config_t _rc = _p_rend_imp->config;			\
    if ( (_mask) & PV_INT_STEREO ) _rc.stereo = 1;			\
    if ( (_mask) & PV_INT_VSYNC ) _rc.vsync = 1;			\
    if ( (_mask) & PV_INT_PAGE ) _rc.page = 1;				\
    if ( (_mask) & PV_INT_VM ) _rc.vm = 1;				\
    if ( (_mask) & PV_INT_RENDER ) _rc.render = 1;			\
    if ( (_mask) & PV_INT_PACKET ) _rc.packet = 1;			\
    _rc.block_en = 0xff;						\
    (_pvp)->render_config = _rc;					\
    _p_rend_imp->config = _rc; wbflush(); }

#define	PV_INT_DISABLE( _pvp, _mask ) {					\
    pv_render_implement_t *_p_rend_imp = PV_REND_IMP_BASE( _pvp );	\
    pv_render_config_t _rc = _p_rend_imp->config;			\
    if ( (_mask) & PV_INT_STEREO ) _rc.stereo = 0;			\
    if ( (_mask) & PV_INT_VSYNC ) _rc.vsync = 0;			\
    if ( (_mask) & PV_INT_PAGE ) _rc.page = 0;				\
    if ( (_mask) & PV_INT_VM ) _rc.vm = 0;				\
    if ( (_mask) & PV_INT_RENDER ) _rc.render = 0;			\
    if ( (_mask) & PV_INT_PACKET ) _rc.packet = 0;			\
    _rc.block_en = 0xff;						\
    (_pvp)->render_config = _rc;					\
    _p_rend_imp->config = _rc; wbflush(); }

#define	PV_INT_CLEAR( _pvp, _mask ) {					\
    pv_render_implement_t *_p_rend_imp = PV_REND_IMP_BASE( _pvp );	\
    pv_render_status_t _rs = _p_rend_imp->status;			\
    _rs.vsync_int = !( (_mask) & PV_INT_VSYNC );			\
    _rs.page_int = !( (_mask) & PV_INT_PAGE );				\
    _rs.vm_int = !( (_mask) & PV_INT_VM );				\
    _rs.render_int = !( (_mask) & PV_INT_RENDER );			\
    _rs.packet_int = !( (_mask) & PV_INT_PACKET );			\
    if ( (_mask) & PV_INT_STALL ) _rs.stall = 0;			\
    _p_rend_imp->status = _rs; wbflush(); }

/*
 * Special printing macros.  Eventually, use reall buffered and
 * polling versions of these.
 */
#define	PRINTFBUF( _arg )	printf _arg

#define	PRINTFPOLL( _arg )	printf _arg


/*
 * Local typedefs
 */
typedef union {
    pv_register_t	reg;
    pv_subpixel_pair_t	sub;
} reg_pix_t;

/*
 * External declarations
 */
extern vm_offset_t
  pv_buffers,
  pv_page_table_0,
  pv_page_table_1,
  pv_mem_bot,
  pv_mem_top;

extern size_t
  pv_buffer_size,
  pv_page_table_0_size,
  pv_page_table_1_size;

extern int
  ws_display_units,
  ws_display_type;

extern unsigned short
  fg_font[];

extern unsigned char
  q_font[];

#ifdef __alpha
extern pt_entry_t
  *pmap_pte();

extern void
  pmap_set_modify();

extern vm_offset_t
  pmap_extract();
#endif /* __alpha */

/*
 * Global definitions
 */
u_int
  _pv_intr_total;

/*
 * Forward declarations
 */
static int
  invalidate_tlb( pv_info *pvp, int table ),
  invalidate_pva_tlb( pv_info *pvp, int table ),
  init_devices( pv_info *pvp, int fullreset ),
  init_pv( pv_info *pvp, int fullreset, unsigned int stereo ),
  init_info_area( pv_info *pvp ),
  ioctl_pageoutin( pv_info *pvp, pv_ioc *ioc ),
  vm_touched_page( pv_info *pvp, struct proc *proc, vm_offset_t va ),
  vm_lock( pv_info *pvp, vm_offset_t addr, u_long size, int prot, int save ),
  vm_load_stlb( pv_info *pvp, vm_offset_t addr, u_long size, int save, int master, int hardlock, int modify ),
  vm_unlock( pv_info *pvp, vm_offset_t addr, u_long size, int restore ),
  vm_unload_stlb( pv_info *pvp, vm_offset_t addr, u_long size, int restore, int master ),
  spin_on_pv( pv_info *pvp ),
  pv_bt463_video_on(),
  pv_bt463_video_off(),
  pv_bt463_recolor_cursor(),
  pv_bt463_restore_cursor_color();

#ifdef BUILD_PERFORMANCE_IOCTLS

static int
  lcl_blitc();

static int
  perf_signal_on_vm = 0;

#endif

static void
  pv_signal_pagein( pv_info *pvp, vm_offset_t va, int rd ),
  pv_bt463_enable_interrupt(),
  pv_bt431_enable_interrupt(),
  pv_intr_vsync( pv_info *pvp ),
  set_packet_dcc( pv_info *pvp ),
  bt_clean_window_tag( pv_info *pvp ),
  pv_bt463_load_wid(),
  pv_bt463_init(),
  pv_bt463_clean_cursormap(),
  pv_bt463_clean_colormap(),
  halt_gfpa();

/*
 * Static data definitions
 */

static caddr_t
  console_address = (caddr_t) NULL;

static unsigned int
  status_non_zero = 0;

static Bt463_Wid_Cell wids[BT463_WINDOW_TAG_COUNT] = {
	{0x00, 0x01, 0x80, 0x00}, /* ... 0 */
	{0x00, 0x01, 0x80, 0x00}, /* ... 1 */
	{0x00, 0x01, 0x80, 0x00}, /* ... 2 */
	{0x00, 0x01, 0x80, 0x00}, /* ... 3 */
	{0x00, 0x01, 0x80, 0x00}, /* ... 4 */
	{0x00, 0x01, 0x80, 0x00}, /* ... 5 */
	{0x00, 0x01, 0x80, 0x00}, /* ... 6 */
	{0x00, 0x01, 0x80, 0x00}, /* ... 7 */
	{0x00, 0x01, 0x80, 0x00}, /* ... 8 */
	{0x00, 0x01, 0x80, 0x00}, /* ... 9 */
	{0x00, 0x01, 0x80, 0x00}, /* ... A */
	{0x00, 0x01, 0x80, 0x00}, /* ... B */
	{0x00, 0x01, 0x80, 0x00}, /* ... C */
	{0x00, 0x01, 0x80, 0x00}, /* ... D */
	{0x00, 0x00, 0x00, 0x00}, /* ... E */
	{0xff, 0x00, 0x00, 0x00}, /* ... F */
};

/*
 * Function definitions
 */

/*
 * pvprobe
 *
 * Probe nothing and return a 1.
 */
int
pvprobe(nxv, ui)
    char *nxv;
    struct controller *ui;
{
    /* 
     * the initialization of the first screen is done through pv_cons_init,
     * so if we have gotten this far we are alive so return a 1
     */
    return(1);
}


/*
 * pvattach
 *
 * Routine to attach to the graphic device.
 */
int
pvattach(ui)
    struct controller *ui;
{
    register int
	i;

    register pv_info
	*pvp = &pv_softc[ui->ctlr_num];

    pvp->ctlr = ui;
    if ( i = pv_attach( ui->addr, ui->ctlr_num, ui->flags ) ) {
	if ( i > 0 && pvp->attach != (int (*)()) NULL ) {
	    /*
	     * i > 0: not console, continue attaching
	     */
	    if ( (*pvp->attach)( ui, pvp ) ) {
		pv_init_screen( pvp, &pvp->screen );
		pv_clear_screen( pvp, &pvp->screen );
	    }
	    else {
		bzero( (caddr_t) pvp, sizeof(pv_info) );
		return 0;
	    }
	}
	else i = 0;

	if ( pvp->bootmsg != (int (*)()) NULL ) {
	    (void) (*pvp->bootmsg)( ui, pvp );
	}

	return i;			/* attach successful */
    }

    return 0;
}


/*
 * pv_attach
 *
 * PV family-wide device attach
 */
int
pv_attach( address, unit, flags )
     caddr_t address;
     int unit;
     int flags;
{
    register pv_info
	*pvp = &pv_softc[unit];

    register int
	dev_type,
	m_type = flags,
	i,
	screen_num;

    caddr_t
	address_s;

    /*
     * Set DEBUG level
     */
    _pv_debug = PV_CONSOLE;

    /*
     * Initialize the device closure.
     */
#ifdef __alpha
    address_s = address;
    address = (caddr_t) PHYS_TO_KSEG(DENSE(KSEG_TO_PHYS(address_s)));
#else /* __alpha */
    address_s = address = (caddr_t) PHYS_TO_K1(address);
#endif /* __alpha */

    if ( ( dev_type = pv_which_option( address_s ) ) < 0 ) {
	goto failure;
    }

    /*
     * Dump info into global display descriptor
     */
    if ( ws_display_units < 8 ) {
	ws_display_type = ( ws_display_type << 8 )
	  		| pv_types[dev_type].ws_display_type;
	ws_display_units = ( ws_display_units << 1 ) | 1;
    }

    /*
     * Initialize screen (but only once).
     */
    if ( console_address != address ) {
	pvp->screen = pv_types[dev_type].screen;
    }
    pvp->screen.screen = unit;

    /*
     * Establish monitor type
     */
    if ( flags != 0 ) {
	if ( ( m_type < 0 ) || ( m_type >= nmon_types ) ) m_type = 0;
	pvp->screen.monitor_type = monitor_type[m_type];
    }

    /*
     * If we are console, then we've already been through this.
     */
    if ( console_address == address ) {
	PV_DEBUG( PV_TALK,
	    PRINTFPOLL( ("pv_attach(%x) -> console\n", address ) ); );
	return -1;
    }

    for ( i = 0; i < pvp->screen.allowed_depths; i++ ) {
	pvp->depth[i] = pv_types[dev_type].depth[i];
	pvp->depth[i].which_depth = i;
    }

    for ( i = 0; i < pvp->screen.nvisuals; i++ ) {
	pvp->visual[i] = pv_types[dev_type].visual[i];
	pvp->visual[i].which_visual = i;
    }

    if ( console_address == NULL ) console_address = address;

    pvp->cf		= pv_types[dev_type].cf;
    pvp->cmf		= pv_types[dev_type].cmf;
    pvp->sf		= pv_types[dev_type].sf;
    pvp->attach		= pv_types[dev_type].attach;
    pvp->bootmsg	= pv_types[dev_type].bootmsg;
    pvp->map		= pv_types[dev_type].map;
    pvp->interrupt	= pv_types[dev_type].interrupt;
    pvp->setup		= pv_types[dev_type].setup;
    pvp->vm_hook	= pv_types[dev_type].vm_hook;
    pvp->p_gcp		= pv_types[dev_type].p_gcp;
    pvp->p_gcp_s	= pv_types[dev_type].p_gcp_s;
    pvp->p_pva		= pv_types[dev_type].p_pva;
    pvp->p_pva_s	= pv_types[dev_type].p_pva_s;
    pvp->p_pv		= pv_types[dev_type].p_pv;
    pvp->p_pv_s		= pv_types[dev_type].p_pv_s;
    pvp->p_sram		= pv_types[dev_type].p_sram;
    pvp->p_sram_s	= pv_types[dev_type].p_sram_s;
    pvp->text_foreground= pv_types[dev_type].text_foreground;
    pvp->text_background= pv_types[dev_type].text_background;
    pvp->sram_size	= pv_types[dev_type].sram_size;

    /*
     * Init closures
     */
    pvp->sf.sc = (*(pvp->sf.init_closure))(
      				pvp->sf.sc, address, unit,
				(int) pv_types[dev_type].screen_type );

    pvp->cf.cc = (*(pvp->cf.init_closure))(
				pvp->cf.cc, address, unit,
				(int) pv_types[dev_type].cursor_type );

    pvp->cmf.cmc = (*(pvp->cmf.init_closure))(
				pvp->cmf.cmc, address, unit,
				(int) pv_types[dev_type].cmap_type );

    /*
     * More setup
     */
    if ( pvp->setup != (int (*)()) NULL &&
	 !( (*pvp->setup)( pvp, address, address_s, unit, flags ) ) ) {
	goto failure;
    }
    pvp->visual[0].depth = pvp->depth[0].depth;

    /*
     * Define the screen
     */
    if ( ( screen_num = ws_define_screen( &pvp->screen, pvp->visual,
					  pvp->depth, &pvp->sf, &pvp->cmf,
					  &pvp->cf ) ) < 0 ) {
	PV_DEBUG( PV_TALK,
	    PRINTFPOLL( ("pv_attach(%x) failed\n", address ) ); );
	goto failure;
    }

    /*
     * Successful return
     */
    return ( screen_num );

    /*
     * Failure return
     */
  failure:

    bzero( (caddr_t) pvp, sizeof( pv_info ) );
    return -1;
}

/*
 * pv_bootmsg
 *
 * Not very interesting
 */
int
pv_bootmsg(ctlr, pvp)
    struct controller *ctlr;
    pv_info *pvp;
{
    int
	up_width,
	up_height,
	vis_width,
	vis_height;

    switch (pvp->update_size) {
case PV_RENDER_IMP_USZ_4X4: up_width = 4; up_height = 4; break;
case PV_RENDER_IMP_USZ_8X2: up_width = 8; up_height = 2; break;
case PV_RENDER_IMP_USZ_8X4: up_width = 8; up_height = 4; break;
default: up_width = 4; up_height = 2; break;
    }

    switch (pvp->vis_pmap_size) {
case PV_RENDER_IMP_VSZ_1024X1024: vis_width = 1024; vis_height = 1024; break;
case PV_RENDER_IMP_VSZ_1280X1024: vis_width = 1280; vis_height = 1024; break;
case PV_RENDER_IMP_VSZ_2048X1024: vis_width = 2048; vis_height = 1024; break;
case PV_RENDER_IMP_VSZ_2048X1536: vis_width = 2048; vis_height = 1536; break;
case PV_RENDER_IMP_VSZ_2048X2048: vis_width = 2048; vis_height = 2048; break;
default: vis_width = 9999; vis_height = 9999; break;
    }

    PRINTFPOLL(("pv%d: vram=%dMP, ua=%dx%d, vp=%dx%d, pms=%dKP, pmc=%d, id=%s\n",
		ctlr->ctlr_num, (pvp->vram_size>>20), up_width, up_height,
		vis_width, vis_height,
		(pvp->pixelmap_size>>10), pvp->pixelmap_count,
		pvp->screen.moduleID ) );

    return (0);
}

/*
 * pv_bt463_enable_interrupt
 *
 * Enable vsync interrupts.
 *
 * Assume that the PVA has enabled interrupt passthroughs from the PV
 * device.
 */
static void
pv_bt463_enable_interrupt(closure)
    struct bt463info *closure;		/* XXX */
{
    volatile register struct bt463info
	*bti = (struct bt463info *) closure;

    register int
	s,
	unit = bti->unit;

    volatile pv_info
	*pvp = &pv_softc[unit];

    s = PV_RAISE_SPL();
/*    PV_INT_CLEAR( pvp, PV_INT_VSYNC ); */
    PV_INT_ENABLE( pvp, PV_INT_VSYNC );
    PV_LOWER_SPL(s);

    return;
}

/* 
 * pv_bt431_enable_interrupt
 */
static void
pv_bt431_enable_interrupt(closure)
    caddr_t closure;		/* XXX */
{
    volatile register struct bt431info
	*bti = (struct bt431info *) closure;

    register int
	s,
	unit = bti->unit;

    volatile pv_info
	*pvp = &pv_softc[unit];

    s = PV_RAISE_SPL();
/*    PV_INT_CLEAR( pvp, PV_INT_VSYNC ); */
    PV_INT_ENABLE( pvp, PV_INT_VSYNC );
    PV_LOWER_SPL(s);

    return;
}

/*
 * pv_interrupt
 *
 * Interrupt handling for PV device
 *
 * A brief description of interrupts are in order.
 *
 * All module interrupts are filtered through the PVA which has
 * eight distinct interrupts in the current configuration.  They
 * are:
 *
 *	1.  Interrupt from PV Bus device 0 (PV)
 *	2.  Interrupt from PV Bus device 1 (GCP)
 *	3.  TLB miss on TLB 0 (PV TLB)
 *	4.  TLB miss on TLB 1 (GCP TLB)
 *	5-8.Error during some sort of DMA.
 *
 * The PV itself has five interrupting conditions:
 *
 *	1.  Packet can be transferred
 *	2.  Rendering pipeline empty
 *	3.  DMA engine free
 *	4.  Pixelmap fault
 *	5.  Vsync
 *
 * The GCP interrupts under program control.  Reason codes are left 
 * around.  Their definition can be found in pv.h
 *
 * TLB miss by device zero is a programming error.
 *
 * TLB miss by device one requires pagein and shadow TLB load.
 *
 * DMA errors demand system panic.
 */
void
pv_interrupt(ctlr, pvp)
    struct controller *ctlr;
    pv_info *pvp;
{
    pv_register_t
        intr_status,
	gcp_status,
	dummy1,
	*p_pc_reg,
	*p_dmal,
	dmal;


    volatile pv_pva_register_t
        *p_pva;

    pv_dma_miss_t
	miss;

    pv_pva_tlb_entry_t
	tlb_entry;

    static int
	pva_tlb_index = 0;

    pv_csr_t
	csr2,
	csr;

    int
	tlb_index,
	miss_read,
	i,
	s,
	miss_serviced = 0,
	gfpa_event = 0;

    unsigned int
	reason;

    vm_offset_t
	miss_vpn,
	miss_va,
	miss_phys;

    pmap_t
	server_map;

    pt_entry_t
	*pte;

    unsigned int
	final_addr;

    volatile pv_render_implement_t
	*p_pv_imp;

    pv_render_status_t
	rend_status;

    pvInfo
	*pvi;

    _pv_intr_total++;

    p_pva = PV_PVA_BASE( pvp );

    intr_status = p_pva->intr_status;

    PV_DEBUG( PV_TALK,
	PRINTFPOLL( ( "pv_interrupt: status 0x%0x\n", intr_status ) ); );
    PV_DEBUG( PV_TALK,
	PRINTFPOLL( ( "pv_interrupt: mask 0x%0x\n", p_pva->intr_mask ) ); );

    /*
     * First, check for system-corrupting errors
     */
    if ( intr_status & PV_PVA_INTR_DMA_RD_ERR0 ) {
        panic( "pv: DMA Read Error by PV device" );
    }
    if ( intr_status & PV_PVA_INTR_DMA_RD_ERR1 ) {
        panic( "pv: DMA Read Error by GCP device" );
    }
    if ( intr_status & PV_PVA_INTR_DMA_WR_ERR0 ) {
        panic( "pv: DMA Write Error by PV device" );
    }
    if ( intr_status & PV_PVA_INTR_DMA_WR_ERR1 ) {
        panic( "pv: DMA Write Error by GCP device" );
    }
    if ( intr_status & PV_PVA_INTR_TLB_MISS0 ) {
	/*
	 * Took a TLB miss on the PV.  Not good.  Try to give some
	 * useful information.
	 */
	PRINTFPOLL( ( "pv: FATAL!  Page miss on PV TLB\n" ) );
	PRINTFPOLL( ( "    DMA Miss Register:     %08x\n",
	        (unsigned long) * (unsigned *) &p_pva->dma_miss ) );
	for ( i = 0; i < PV_PVA_TLB_ENTRIES; i++ ) {
	    PRINTFPOLL( ( "    TLB[%2d]:               %0lx\n",
		    i, * (unsigned long *) &p_pva->tlb[i] ) );
	}
	panic( "pv: can't continue from PV TLB Miss" );
    }

    /*
     * Check device 0 (PV)
     */
    if ( intr_status & PV_PVA_INTR_PB0 ) {
	/*
	 * PV device has interrupted.  Get individual masks to see what
	 * is going on.
	 */
	volatile pv_render_implement_t
	    *p_pv_imp;

	pv_render_status_t
	    rend_status;

	volatile pv_graphic_tlb_t
	    *p_pv_tlb;

	p_pv_imp = PV_REND_IMP_BASE( pvp );

	/*
	 * Note on intr_parity:  for vsync operations, we take three
	 * interrupts.  The first is used to set the handsoff register
	 * to get the GFPA off of the PV and to enable a transition to
	 * the second state.  In the second, we take interrupts until the
	 * PV is no longer busy.  Then, we actually to the operation.
	 */
	rend_status = p_pv_imp->status;

	PV_DEBUG( PV_TALK,
	    PRINTFPOLL( ( "pv_interrupt: PV int mask: 0x%0x\n",
			* (unsigned *) &rend_status ) ); );

	if ( rend_status.vsync_int && pvp->render_config.vsync ) {
#ifdef DO_THREE_STATE_VSYNC
	    if ( pvp->intr_parity == 2 ) {
		/*
		 * vsync interrupt.  Probably a cursor or colormap reload.
		 * Handler will clear and disable interrupt in the PV.
		 */
#if 0
		    PV_INT_CLEAR( pvp, PV_INT_VSYNC );
		    rend_status = p_pv_imp->status;
		    while ( !rend_status.vsync_int ) {
			rend_status = p_pv_imp->status;
		    }
#endif
		pv_intr_vsync( pvp );
		pvp->intr_parity = 0;
		if ( pvp->state & PV_INFO_STATE_GCP_VALID ) {
		    STORE_SOFTWARE_REG( pvp, PV_IOC_CNFG_GFPA_HANDSOFF, 0 );
		    wbflush();
		}
	    }
	    else if ( pvp->intr_parity == 1 ) {
		if ( !rend_status.render_status || rend_status.page_int ) {
		    pvp->intr_parity = 2;
		}
		PV_INT_CLEAR( pvp, PV_INT_VSYNC );
	    }
	    else {
		if ( pvp->state & PV_INFO_STATE_GCP_VALID ) {
		    STORE_SOFTWARE_REG( pvp, PV_IOC_CNFG_GFPA_HANDSOFF,
			HANDSOFF_VALUE );
		    wbflush();
		}
		pvp->intr_parity = 1;
		PV_INT_CLEAR( pvp, PV_INT_VSYNC );
	    }
#else /* DO_THREE_STATE_VSYNC */
	    if ( pvp->intr_parity ) {
		pv_intr_vsync( pvp );
		pvp->intr_parity = 0;
	    }
	    else {
		pvp->intr_parity = 1;
		PV_INT_CLEAR( pvp, PV_INT_VSYNC );
	    }
#endif
	}

	if ( rend_status.page_int && pvp->render_config.page ) {
	    p_pv_tlb = PV_REND_TLB_BASE( pvp );
	    if ( pvp->p_proc[PV_IOC_PID_PV_INDEX] != (struct proc *) NULL &&
		 pvp->pid[PV_IOC_PID_PV_INDEX]
			== pvp->p_proc[PV_IOC_PID_PV_INDEX]->p_pid &&
		 pvp->sig_mask[PV_IOC_PID_PV_INDEX] & PV_IOC_SIG_PMAP_FAULT ) {

		pv_graphic_tlb_t
		    fault_tlb;

		fault_tlb = *(pv_graphic_tlb_t *) &p_pv_imp->tlb_addr;
		if ( fault_tlb.pixelmap_id == PV_DOORBELL_0_PIXELMAP_ID ||
		     fault_tlb.pixelmap_id == PV_DOORBELL_1_PIXELMAP_ID ) {
		    /*
		     * Synchronization operation.  The above two PIDs
		     * are used to flag the completion of read and write
		     * blocks operations.  So, in here, complete any
		     * wait in the main thread.
		     */
		    pvp->saved_graphic_tlb = p_pv_tlb[pvp->pixelmap_count-1];
		    pvp->state |= PV_INFO_STATE_SAVED_GTLB_VALID;
		    p_pv_tlb[pvp->pixelmap_count-1] = fault_tlb;
		    wbflush();
		    PV_INT_CLEAR( pvp, PV_INT_PAGE );
		    pvp->sleep_chan[PV_INFO_SLEEP_IMAGE] = 0;
		    wakeup( &pvp->sleep_chan[PV_INFO_SLEEP_IMAGE] );
		}
		else {
		    /*
		     * Process has promised to take care of this.  We have to
		     * leave the interrupt bit on or the PV will just restart
		     * so disable the interrupt.  Have the pixelmap pageout/in
		     * ioctl turn it back on.
		     */
		    PV_INT_DISABLE( pvp, PV_INT_PAGE );
		    pvp->pv->signal_reasons[PV_INFO_SIG_PMAP_FAULT] = 1;
		    psignal( pvp->p_proc[PV_IOC_PID_PV_INDEX], SIGURG );
		}
	    }
	    else {
		/*
		 * No process to handle this.  To keep things running so that
		 * the PV returns to a working state, use sequential entries
		 * of the graphics TLB to resolve the misses.  Do *not*
		 * overwrite the console entries.
		 */
		static int
		    victim = 99999;

		if ( victim >= pvp->pixelmap_count ) {
		    /*
		     * MAGIC NUMBER!  Cycle through the non-console graphics
		     * tlb entries when we need to resolve an outstanding
		     * pixemap page fault.  These, at a minimum, start at
		     * 80 (40 for 4x4 but this will do).
		     */
		    victim = 80;
		}
		p_pv_tlb[victim++] = *(pv_graphic_tlb_t *) &p_pv_imp->tlb_addr;
		wbflush();
		PV_INT_CLEAR( pvp, PV_INT_PAGE );
		PV_DEBUG( PV_CONSOLE,
		    PRINTFBUF( ( "pv: Unfielded pixelmap fault.\n" ) ); );
	    }
	}

	if ( rend_status.vm_int && pvp->render_config.vm ) {
	    /*
	     * Someone was interested in the PV DMA completion. 
	     * Perform a wakeup and clear the interrupt.
	     */
#ifdef	BUILD_PERFORMANCE_IOCTLS
	  if ( perf_signal_on_vm &&
	       pvp->p_proc[PV_IOC_PID_PV_INDEX] != (struct proc *) NULL &&
	       pvp->pid[PV_IOC_PID_PV_INDEX]
			== pvp->p_proc[PV_IOC_PID_PV_INDEX]->p_pid ) {
	    psignal( pvp->p_proc[PV_IOC_PID_PV_INDEX], SIGURG );
	    perf_signal_on_vm = 0;
	    PV_INT_DISABLE( pvp, PV_INT_VM );
	    PV_INT_CLEAR( pvp, PV_INT_VM );
	  }
	  else {
#endif	/* BUILD_PERFORMANCE_IOCTLS */
	    pvp->sleep_chan[PV_INFO_SLEEP_PV_DMA] = 0;
	    wakeup( &pvp->sleep_chan[PV_INFO_SLEEP_PV_DMA] );
	    PV_INT_DISABLE( pvp, PV_INT_VM );
	    PV_INT_CLEAR( pvp, PV_INT_VM );
#ifdef	BUILD_PERFORMANCE_IOCTLS
	  }
#endif	/* BUILD_PERFORMANCE_IOCTLS */
	}

	if ( rend_status.render_int && pvp->render_config.render ) {
	    /*
	     * Ditto
	     */
	    pvp->sleep_chan[PV_INFO_SLEEP_RENDER] = 0;
	    wakeup( &pvp->sleep_chan[PV_INFO_SLEEP_RENDER] );
	    PV_INT_DISABLE( pvp, PV_INT_RENDER );
	    PV_INT_CLEAR( pvp, PV_INT_RENDER );
	}

#if 0
	if ( rend_status.packet_int && pvp->render_config.packet ) {
	    /*
	     * Ditto
	     */
	    pvp->sleep_chan[PV_INFO_SLEEP_PACKET] = 0;
	    wakeup( &pvp->sleep_chan[PV_INFO_SLEEP_PACKET] );
	    PV_INT_DISABLE( pvp, PV_INT_PACKET );
	    PV_INT_CLEAR( pvp, PV_INT_PACKET );
	}
#endif
    }

    /*
     * Check device 1 (GFPA)
     */
    if ( intr_status & PV_PVA_INTR_PB1 ) {
	pv_register_t
	    dummy;

	if (!( pvp->state & PV_INFO_STATE_GCP_VALID ) ) {
	    /*
	     * If we don't know about the reason register, all we can
	     * do is clear the interrupt
	     */
	    goto clear_gcp_int;
	}

	LOAD_SOFTWARE_REG( pvp, PV_IOC_CNFG_GFPA_REASON, reason );

	PV_DEBUG( PV_TERSE,
	    PRINTFPOLL( ( "pv_interrupt: GFPA interrupts.  Reason = 0x%0x\n",
			  reason ) ); );

	switch ( reason ) {

	case (unsigned int) PV_GCP_INTR_COUNT_DOWN:
	    /*
	     * Countdown interrupt.  Prepare for a sync operations
	     */
	    pvp->gcp_count_down_event = 1;
	    pvp->sleep_chan[PV_INFO_SLEEP_GFPA] = 0;
	    wakeup( &pvp->sleep_chan[PV_INFO_SLEEP_GFPA] );
	    if ( ((pvInfo *) pvp->p_info_area)->gfpa_halts_on_countdown ) {
		pv_register_t
		    pc,
		    *p_pc;

		p_pc = (pv_register_t *) ( (vm_offset_t) PV_GCP_BASE( pvp )
					 + PV_GCP_PC_OFFSET );
		pc = *p_pc;
		wbflush();
		*p_pc = pc;
		wbflush();
	    }
	    break;

	case (unsigned int) PV_GCP_INTR_HOST_CLIP:
	    /*
	     * Clipping needed.  DMA information into host.
	     */
	    {
		static unsigned int
		    dma_ops[] = {
		        0x20004001,		/* global 0-63 */
			0x21004001,		/* global 64-127 */
			0x00004001,		/* fpu0 0-63 */
			0x04004001,		/* fpu1 0-63 */
			0x08004001,		/* fpu2 0-63 */
			0x0c004001		/* fpu3 0-63 */
		    },
		    dma_size = 0x40 << 2;

		pv_register_t
		    *p_dmal,
		    *p_dmah,
		    *p_dmaop;

		unsigned long
		    clip_addr;

		pvp->gfpa_is_running = 0;

		p_dmal = (pv_register_t *) ( PV_GCP_BASE( pvp ) + PV_GCP_DMAL_OFFSET );
		p_dmah = (pv_register_t *) ( PV_GCP_BASE( pvp ) + PV_GCP_DMAH_OFFSET );
		p_dmaop = (pv_register_t *) ( PV_GCP_BASE( pvp ) + PV_GCP_DMAOP_OFFSET );

		clip_addr = ( ( (unsigned long) PVB_PHOST_OFFSET_HIGH << 32 )
			      | (unsigned long) PVB_PHOST_OFFSET_LOW )
		  	  + (unsigned long) K0_TO_PHYS( pvp->p_clip_area );

		*p_dmal = (pv_register_t) ( clip_addr & 0xffffffff );
		*p_dmah = (pv_register_t) ( ( clip_addr >> 32 ) & 0xffffffff );
		wbflush();

		for ( i = 0; i < (sizeof(dma_ops)/sizeof(dma_ops[0])); i++ ) {
		    *p_dmaop = dma_ops[i];
		    wbflush();
		}

		/*
		 * Now attempt signal delivery
		 */
		if ( pvp->p_proc[PV_IOC_PID_GCP_INDEX] != (struct proc *) NULL &&
		     pvp->pid[PV_IOC_PID_GCP_INDEX]
			== pvp->p_proc[PV_IOC_PID_GCP_INDEX]->p_pid &&
		     pvp->sig_mask[PV_IOC_PID_GCP_INDEX] & PV_IOC_SIG_CLIPPING ) {
		    /*
		     * Someone has registered for this signal
		     */
		    pvp->pv->signal_reasons[PV_INFO_SIG_CLIPPING] = 1;
		    psignal( pvp->p_proc[PV_IOC_PID_GCP_INDEX], SIGURG );
		}
		else {
		    /*
		     * Huh?
		     */
		    PRINTFBUF( ( "pv: No process registered for clipping.\n" ) );
		}
	    }
	    break;

	case PV_GCP_INTR_HALT:
	    /*
	     * GFPA saw a VM DMA failure and is complaining.  Try to
	     * fix it up without getting the server involved.  Also get the
	     * GFPA's PC, we'll need to write it back eventually....
	     */
	    pvp->gfpa_is_running = 0;
	    p_pc_reg = (pv_register_t *) ( (vm_offset_t) PV_GCP_BASE(pvp)
					 + PV_GCP_PC_OFFSET );
	    pvp->gfpa_pc = *p_pc_reg;
	    pvp->gfpa_pc_valid = 1;
	    miss = p_pva->dma_miss;
	    miss_vpn = (vm_offset_t) miss.vpn;
	    miss_read = miss.rd;
	    miss_va = (vm_offset_t) PV_VPN_TO_VA( miss_vpn );
	    PV_DEBUG( PV_TERSE,
		PRINTFPOLL( ( "pv_interrupt: GFPA TLBMiss va=0x%0lx, read=%d\n",
			      miss_va, miss_read ) ); );

	    if ( pvp->p_proc[PV_IOC_PID_GCP_INDEX] != (struct proc *) NULL &&
		 pvp->pid[PV_IOC_PID_GCP_INDEX]
			== pvp->p_proc[PV_IOC_PID_GCP_INDEX]->p_pid ) {
		/*
		 * There is a process.  See if we can resolve this.
		 */
		if ( pvp->p_proc[PV_IOC_PID_GCP_INDEX]->p_flag & SLOAD ) {
		    /*
		     * Process appears to be resident
		     */
		    server_map = pvp->p_proc[PV_IOC_PID_GCP_INDEX]->task->map->vm_pmap;
		    s = splvm();
		    pte = pmap_pte( server_map, miss_va );
		    if ( pmap_extract( server_map, miss_va ) ) {
			if ( !miss_read ) {
			    /*
			     * Mark as modified
			     */
			    miss_phys = alpha_ptob( pte->pg_pfn );
			    pmap_set_modify( miss_phys );
			}
			tlb_entry.ppn	= pte->pg_pfn;
			tlb_entry.pid	= PV_PVA_PID_SERVER;
			tlb_entry.we	= !miss_read;
			tlb_entry.vpn	= miss_vpn;
			tlb_index	= miss_vpn &
			  		( pvp->page_table_1_size - 1 );
			pvp->p_page_table_1[tlb_index] = tlb_entry;
#ifdef PRELOAD_PVA_AFTER_MISS
			pva_tlb_index	= ++pva_tlb_index
			  		% PV_PVA_TLB_ENTRIES;
			p_pva->tlb[pva_tlb_index] = tlb_entry;
#endif
			wbflush();

			*(pv_register_t *) &p_pva->dma_miss = (pv_register_t) 0;
			wbflush();

			PV_DEBUG( PV_TERSE,
			    PRINTFBUF( ("pv_interrupt: Miss serviced in ISR\n" )); );

			/*
			 * Miss is serviced.
			 */
			miss_serviced = 1;
		    }
		    else { /* pmap_extract() failed */
		        pv_signal_pagein( pvp, miss_va, miss_read );
		    }
		    splx(s);
		} /* proc & SLOAD .. proc isn't resident */
		else {
		    pv_signal_pagein( pvp, miss_va, miss_read );
		}
	    }
	    else { /* There's no process registered */
		/*
		 * No process.
		 */
		PRINTFBUF( ( "pv:  GFPA page miss and no target process\n" ) );
	    }

	    /*
	     * Okay, we'll also need to clean up the PVA before
	     * we get the GFPA started.  First check for the
	     * special 3LW DMA fixup.  Then add GFPA DMA miss
	     * to the list of serviced interrupts.
	     */
	    p_dmal = (pv_register_t *) ( PV_GCP_BASE( pvp )
				       + PV_GCP_DMAL_OFFSET );
	    dmal = *p_dmal;
	    final_addr = dmal & ( NBPG - 1 );
	    if ( final_addr < 16 && final_addr > 0 ) {
		p_pv_imp = PV_REND_IMP_BASE( pvp );
		rend_status = p_pv_imp->status;
		while ( rend_status.render_status == 1 &&
			rend_status.page_int == 0 ) {
		    rend_status = p_pv_imp->status;
		}
		if ( rend_status.page_int ) {
		    DELAY( 15 );
		}
		csr2 = csr = p_pva->csr;
		csr.pb_reset = 1;
		csr2.pb_reset = 0;
		p_pva->csr = csr;
		wbflush();
		p_pva->csr = csr2;
		wbflush();
		PV_DEBUG( PV_TERSE,
		    PRINTFPOLL( ("pv_interrupt: did 3LW DMA fixup\n") ); );
	    }
	    intr_status |= PV_PVA_INTR_TLB_MISS1;
	    break;

	default:
	    /*
	     * Unknown interrupt reason.  Ignore it.
	     */
	    PRINTFBUF( ( "pv: Unknown GFPA interrupt reason: 0x%x.\n", reason ) );
	    break;

	} /* switch ( reason ) */

	/*
	 * Did processing.  Record this fact for later
	 */
	gfpa_event = 1;

	/*
	 * Branch to this label if we don't know about GFPA registers.
	 */
clear_gcp_int:
	;
    }

    /*
     * Clear handled interrupts.  *Don't* clear the PV's interrupt line
     * which is just a pass-through.
     */
    intr_status &= ~PV_PVA_INTR_PB0;
    if ( intr_status != 0 ) {
	p_pva->intr_status = intr_status;
	wbflush();
    }
    dummy1 = p_pva->intr_status;
    wbflush();
    if ( dummy1 ) {
	status_non_zero++;
    }

    /*
     * If there was a GFPA interrupt, now clear the reason register
     */
    if ( gfpa_event ) {
	/*
	 * Clear the interrupt.
	 */
#ifndef	DONT_CLEAR_GFPA_REASON
	if ( reason != PV_GCP_INTR_CLEAR ) {
	    STORE_SOFTWARE_REG( pvp, PV_IOC_CNFG_GFPA_REASON,
				PV_GCP_INTR_CLEAR );
	    wbflush();
	    STORE_SOFTWARE_REG( pvp, PV_IOC_CNFG_GFPA_WRITE_REASON,
				PV_GCP_INTR_CLEAR );
	    wbflush();
	    dummy1 = p_pva->intr_status;
	    wbflush();
	}
#endif
	if ( miss_serviced ) {
	    /*
	     * We can restart the GFPA now...
	     */
	    pvp->gfpa_is_running = 1;
	    *p_pc_reg = pvp->gfpa_pc;
	    wbflush();
	    pvp->gfpa_pc_valid = 0;
	}
    }

    /*
     * Done
     */
    return;
}

/*
 * pv_signal_pagein
 *
 * Try to get the server process to perform the pagein
 *
 */
static void
pv_signal_pagein( pvp, va, rd )
     pv_info *pvp;
     vm_offset_t va;
     int rd;
{
    if ( pvp->sig_mask[PV_IOC_PID_GCP_INDEX] & PV_IOC_SIG_VA_PAGEIN ) {
	pvp->pv->pagein_vaddr = (caddr_t) va;
	pvp->pv->pagein_dirty = !rd;
	pvp->pv->signal_reasons[PV_INFO_SIG_VA_PAGEIN] = 1;
	PV_DEBUG( PV_TERSE,
	    PRINTFPOLL( ( "pv_signal_pagein: sending SIGURG to pid %d\n",
		pvp->p_proc[PV_IOC_PID_GCP_INDEX]->p_pid ) ); );

	psignal( pvp->p_proc[PV_IOC_PID_GCP_INDEX], SIGURG );
    }
    else {
	PRINTFBUF( ( "pv: dropped DMA Miss on floor: va = 0x%lx, read = %d\n",
		(unsigned long) va, rd ) );
    }

    return;
}

/*
 * pv_recolor_cursor, pv_video_on, and pv_video_off violate the
 * colormap/cursor functionality separation and so some closure
 * contortions are necessary.
 */
int
pv_recolor_cursor(closure, screen, fg, bg)
    caddr_t closure;
    ws_screen_descriptor *screen;
    ws_color_cell *fg, *bg;
{
    volatile struct bt431info *bti = (struct bt431info *) closure;
    register int unit = bti->unit;
    volatile pv_info *pvp = &pv_softc[unit];
    volatile struct bt463info *btii = (struct bt463info *) pvp->cmf.cmc;
    int status, s;

    s = PV_RAISE_SPL();
    status = pv_bt463_recolor_cursor( btii, screen, fg, bg);
    PV_LOWER_SPL(s);

    return (status);
}

int
pv_video_off(closure)
    caddr_t closure;
{
    register struct bt463info *btii = (struct bt463info *)closure;
    register int unit = btii->unit;
    register pv_info *pvp = &pv_softc[unit];
    register struct bt431info *bti = (struct bt431info *) pvp->cf.cc;
    int s;

    s = PV_RAISE_SPL();

    pvp->video_on_off_dirty = 1;
    pvp->video_on_off = 0;

    pv_bt463_enable_interrupt( closure );

    if(bti->on_off) {
	bt431_cursor_on_off(bti, 0);
	bti->cursor_was_on = 1;
    }

    PV_LOWER_SPL(s);
}

int
pv_video_on(closure)
    caddr_t closure;
{
    register struct bt463info *btii = (struct bt463info *)closure;
    register int unit = btii->unit;
    register pv_info *pvp = &pv_softc[unit];
    register struct bt431info *bti = (struct bt431info *) pvp->cf.cc;
    int s;

    s = PV_RAISE_SPL();

    pvp->video_on_off_dirty = 1;
    pvp->video_on_off = 1;

    pvp->wt_dirty = 1;
    pvp->wt_left_min_dirty = 8;
    pvp->wt_left_max_dirty = WINDOW_TAG_COUNT-1;
    btii->dirty_cursormap = 1;

    pv_bt463_enable_interrupt( closure );

    if ( bti->cursor_was_on ) {
	bt431_cursor_on_off(bti, 1);
	bti->cursor_was_on = 0;
    }

    PV_LOWER_SPL(s);
}

/*
 * pv_which_option
 *
 * Identify which PV-module is in the specified slot
 *
 * There's only one type at the moment so return 0.
 */
int
pv_which_option( address )
caddr_t address;
{
    register int
	i;

    char
	*module = (char *) NULL;

    vm_offset_t
	phys = KSEG_TO_PHYS( address );

    for ( i = 0; i < TC_IOSLOTS; i++ ) {
	if ( tc_slot[i].physaddr ==  phys ) {
	    module = tc_slot[i].modulename;
	    break;
	}
    }

    if ( module != (char *) NULL && *module != '\0' ) {
	for ( i = 0; i < npv_types; i++ ) {
	    if ( strncmp( pv_types[i].screen.moduleID, module, TC_ROMNAMLEN )
		 == 0 ) {
		return i;
	    }
	}
    }
    else {
	PRINTFPOLL( ("pv - couldn't identify tc module\n" ) );
    }

    return -1;
}

/*
 * Module-specific initialization for the first PV module.  We don't
 * know what belongs here and what belongs in common setup in pv_setup
 * yet so do everything there for now.
 */
int
pv_dd_attach( ui, pvp )
struct controller *ui;
pv_info *pvp;
{
    return (1);
}

/*
 * Set up the PV module.
 *
 * This is the real meat of the device initialization.
 *
 * Once there are more PV-type options, a good bit of this code
 * should be placed into module-specific attach functions (e.g.
 * pv_dd_attach).
 */
int
pv_setup(pvp, address, address_s, unit, flags)
    pv_info *pvp;
    caddr_t address;
    caddr_t address_s;
    int unit;
    int flags;
{
    struct bt431info
	*bti = (struct bt431info *) pvp->cf.cc;

    struct bt463info
	*btii = (struct bt463info *) pvp->cmf.cmc;

    pv_pva_register_t
 	*p_pva;

    pv_render_implement_t
	*p_rend;

    pv_render_imp_rev_t
	rend_imp_rev;

    pv_render_status_t
	rend_status;

    pv_csr_t
	csr;

    pv_pid_t
	pid;

    int
	i,
	old_level,
	dev_type;

    pv_register_t
	*p_gcp,
	*p_gcp_register;

    long
	*p_zero;

    /*
     * Compute feature addresses
     */
    pvp->p_pvo 		= address;			/* Board. */
    pvp->p_pvo_s	= address_s;
    pvp->p_gcp		+= (vm_offset_t) address;
    pvp->p_gcp_s	+= (vm_offset_t) address_s;
    pvp->p_pva		+= (vm_offset_t) address;
    pvp->p_pva_s	+= (vm_offset_t) address_s;
    pvp->p_pv		+= (vm_offset_t) address;
    pvp->p_pv_s		+= (vm_offset_t) address_s;
    pvp->p_sram		+= (vm_offset_t) address;
    pvp->p_sram_s	+= (vm_offset_t) address_s;

    /*
     * Construct KSEG addresses for specially allocated physical memory.
     */
    if ( pvp->p_clip_area == (caddr_t) NULL ) {
        /*
	 * Uninited.  Note the following:
	 *
	 *  .  All info structs *must* fit within one page
	 *
	 *  .  There is a single kernel PV packet page and a single
	 *     dead page for all modules.
	 *
	 *  .  The dead page must physically follow all other buffers.
	 */
	pvp->page_table_0_size = PV_PVA_TABLE0_SIZE;
	pvp->phys_page_table_0 = pv_page_table_0
	  		       + unit
				 * PV_PVA_TABLE0_SIZE
				 * sizeof( pv_pva_tlb_entry_t );
        pvp->p_page_table_0 = (pv_pva_tlb_entry_t *) PHYS_TO_KSEG( pvp->phys_page_table_0 );

	pvp->page_table_1_size = PV_PVA_TABLE1_SIZE;
	pvp->phys_page_table_1 = pv_page_table_1
	  		       + unit
				 * PV_PVA_TABLE1_SIZE
				 * sizeof( pv_pva_tlb_entry_t );
	pvp->p_page_table_1 = (pv_pva_tlb_entry_t *) PHYS_TO_KSEG( pvp->phys_page_table_1 );

	pvp->cmd_area_size = PV_PVQM_SIZE;
	pvp->phys_cmd_area = (caddr_t) (pv_buffers + unit * PV_PVQM_SIZE);
	pvp->p_cmd_area = (caddr_t) PHYS_TO_KSEG( pvp->phys_cmd_area );
	pvp->phys_clip_area = (caddr_t) ( pv_buffers
					+ npv_softc * PV_PVQM_SIZE
					+ unit * NBPG );
	pvp->p_clip_area = (caddr_t) PHYS_TO_KSEG( pvp->phys_clip_area );
	pvp->phys_info_area = (caddr_t) ( pv_buffers
					+ npv_softc * ( PV_PVQM_SIZE + NBPG )
					+ unit * 512 );
	pvp->p_info_area = (caddr_t) PHYS_TO_KSEG( pvp->phys_info_area );
	pvp->phys_packet_area = (caddr_t) ( pv_buffers
					  + npv_softc * ( PV_PVQM_SIZE + NBPG )
					  + NBPG );
	pvp->p_packet_area = (caddr_t) PHYS_TO_KSEG( pvp->phys_packet_area );
	pvp->phys_dead_area = (caddr_t) ( pv_buffers
					+ npv_softc * ( PV_PVQM_SIZE + NBPG )
					+ 2 * NBPG );
	pvp->p_dead_area = (caddr_t) PHYS_TO_KSEG( pvp->phys_dead_area );
	pvp->ppn_dead_area = PV_PA_TO_PPN( pvp->phys_dead_area );

	/*
	 * Zero clip area first time through
	 */
	p_zero = (long *) pvp->p_clip_area;
	for ( i = 0; i < ( NBPG / sizeof( long ) ); i++ ) {
	    *p_zero++ = 0L;
	}
    }

    old_level = _pv_debug;
/*    _pv_debug = PV_TALK; */
    PV_DEBUG( PV_TALK,
	PRINTFBUF( ("pv: page table 0 at phys 0x%x kseg 0x%lx for 0x%x\n",
			pv_page_table_0,
			pvp->p_page_table_0,
			pv_page_table_0_size ) ); );
    PV_DEBUG( PV_TALK,
	PRINTFBUF( ("pv: page table 1 at phys 0x%x kseg 0x%lx for 0x%x\n",
			pv_page_table_1,
			pvp->p_page_table_1,
			pv_page_table_1_size ) ); );
    PV_DEBUG( PV_TALK,
	PRINTFBUF( ("pv: buffer area at phys 0x%x kseg 0x%lx for 0x%x\n",
			pv_buffers,
			pvp->p_cmd_area,
			pv_buffer_size ) ); );
    _pv_debug = old_level;

    /*
     * Misc. structure initialization
     */
    pvp->dev_closure		= (caddr_t) pvp;
    pvp->last_intr_status	= 0;
    pvp->render_config.vsync	= 0;
    pvp->render_config.page	= 0;
    pvp->render_config.vm	= 0;
    pvp->render_config.render	= 0;
    pvp->render_config.packet	= 0;
    pvp->state			= 0;
    for ( i = 0; i < PV_IOC_PID_NUM; i++ ) {
	pvp->p_proc[i]		= (struct proc *) NULL;
	pvp->pid[i]		= (pid_t) 0;
	pvp->sig_mask[i]	= 0;
    }
    pvp->gcp_clock_control	= MAGIC_GCP_CLOCK_VALUE;
    pvp->image_check_va_range	= 0;
    pvp->image_hardlock_in_progress = 0;
    pvp->image_failure		= 0;
    pvp->intr_parity		= 0;
    pvp->dcc_default		= (TRUECOLOR_DCC_VALUE << 1);
    pvp->dcc_user		= (TRUECOLOR_DCC_VALUE << 1);
    pvp->video_on_off_dirty	= 0;
    pvp->video_on_off		= 1;
    pvp->wt_dirty		= 1;
    pvp->wt_left_min_dirty	= 8;
    pvp->wt_left_max_dirty	= WINDOW_TAG_COUNT-1;
    pvp->wt_right_min_dirty	= 8;
    pvp->wt_right_max_dirty	= WINDOW_TAG_COUNT-1;
    for ( i = 0; i < WINDOW_TAG_COUNT; i++ ) {
	pvp->wt_right_cell[i].windex = pvp->wt_left_cell[i].windex = i;
	pvp->wt_right_cell[i].low = pvp->wt_left_cell[i].low = wids[i].low_byte;
	pvp->wt_right_cell[i].mid = pvp->wt_left_cell[i].mid = wids[i].middle_byte;
	pvp->wt_right_cell[i].high = pvp->wt_left_cell[i].high = wids[i].high_byte;
    }

    /*
     * Clean the info area
     */
    init_info_area( pvp );

    /*
     * init_devices() is a temporary measure.  It performs tasks that
     * are normally performed by powerup testing.
     */
    init_devices( pvp, 1 );

    /*
     * Begin normal device initialization
     */
    invalidate_tlb( pvp, INVALIDATE_PAGER );
    invalidate_tlb( pvp, INVALIDATE_SERVER );
    invalidate_pva_tlb( pvp, INVALIDATE_ALL );

    /*
     * Get addresses of interesting things
     */
    p_pva = PV_PVA_BASE( pvp );
    p_rend = PV_REND_IMP_BASE( pvp );
    p_gcp = (pv_register_t *) PV_GCP_BASE( pvp );

    /*
     * Make certain GCP is quiet
     */
    halt_gfpa( pvp );

    p_gcp_register = (pv_register_t *)( (vm_offset_t) p_gcp
				      + PV_GCP_CMP01_OFFSET );
    *p_gcp_register++ = 0x00000000;
    *p_gcp_register = 0x80000000;			wbflush();

    /*
     * Get the PV to shutup
     */
    PV_INT_DISABLE( pvp, (PV_INT_STEREO|PV_INT_VSYNC|PV_INT_PAGE|PV_INT_VM|
			  PV_INT_RENDER|PV_INT_PACKET) );

    PV_INT_CLEAR( pvp, (PV_INT_VSYNC|PV_INT_PAGE|PV_INT_VM|PV_INT_RENDER|
			PV_INT_PACKET|PV_INT_STALL) );

    /*
     * Fixup various interrupt masks
     */
    * (pv_register_t *) &p_pva->dma_miss = (pv_register_t) 0; wbflush();
    p_pva->intr_mask_set = PV_PVA_INTR_ALL;		wbflush();

    /*
     * Now, config the PVA
     */
    p_pva->spt_base0 = ( pvp->phys_page_table_0 >> SHIFT_FOR_SPT_BASE );
    p_pva->spt_base1 = ( pvp->phys_page_table_1 >> SHIFT_FOR_SPT_BASE );
    wbflush();

    switch ( PV_PVA_PAGE_SIZE ) {
    case PV_PVA_PAGE_SIZE_0: csr.vps = PV_PVA_CSR_VPS_512; break;
    case PV_PVA_PAGE_SIZE_1: csr.vps = PV_PVA_CSR_VPS_4K; break;
    case PV_PVA_PAGE_SIZE_2: csr.vps = PV_PVA_CSR_VPS_8K; break;
    case PV_PVA_PAGE_SIZE_3: csr.vps = PV_PVA_CSR_VPS_64K; break;
    }

    switch ( PV_PVA_TABLE0_SIZE ) {
    case PV_PVA_TABLE_SIZE_0: csr.spts0 = PV_PVA_CSR_SPTS_4K; break;
    case PV_PVA_TABLE_SIZE_1: csr.spts0 = PV_PVA_CSR_SPTS_8K; break;
    case PV_PVA_TABLE_SIZE_2: csr.spts0 = PV_PVA_CSR_SPTS_16K; break;
    case PV_PVA_TABLE_SIZE_3: csr.spts0 = PV_PVA_CSR_SPTS_128K; break;
    }

    switch ( PV_PVA_TABLE1_SIZE ) {
    case PV_PVA_TABLE_SIZE_0: csr.spts1 = PV_PVA_CSR_SPTS_4K; break;
    case PV_PVA_TABLE_SIZE_1: csr.spts1 = PV_PVA_CSR_SPTS_8K; break;
    case PV_PVA_TABLE_SIZE_2: csr.spts1 = PV_PVA_CSR_SPTS_16K; break;
    case PV_PVA_TABLE_SIZE_3: csr.spts1 = PV_PVA_CSR_SPTS_128K; break;
    }

    csr.dma_lockout = 0;
    csr.pb_reset = 0 ;
    csr.gpo = 0;
    csr.gpi = 0;
    p_pva->csr = csr;					wbflush();

    pid.pid0 = PV_PVA_PID_PAGER;
    pid.pid1 = PV_PVA_PID_SERVER;
    p_pva->pid = pid;					wbflush();

    /*
     * Get some information from the PV device
     */
    pvp->render_imp_rev = rend_imp_rev = p_rend->imp_rev;
    pvp->vram_size = 1024 * 1024 * rend_imp_rev.mem_size;
    pvp->update_size = rend_imp_rev.update_size;
    pvp->vis_pmap_size = rend_imp_rev.vis_pmap_size;
    if ( rend_imp_rev.mem_size < 4 ) {
	/*
	 * If less than 4 Meg of memory, assume 16K pixel pages
	 */
	pvp->pixelmap_size = 16 * 1024;
	pvp->pixelmap_count = pvp->vram_size / pvp->pixelmap_size;
    }
    else {
	/*
	 * Otherwise, assume 256 pixelmaps
	 */
	pvp->pixelmap_count = 256;
	pvp->pixelmap_size = pvp->vram_size / pvp->pixelmap_count;
    }

    /*
     * Fill in dcc bits
     */
    set_packet_dcc( pvp );

    /*
     * Now let some interrupts through
     */
    p_pva->intr_mask_clear = ( PV_PVA_INTR_ALL & ~PV_PVA_INTR_TLB_MISS1 );
    wbflush();

    /*
     * now install our vblank interrupt enable routine
     */
    bti->enable_interrupt = pv_bt431_enable_interrupt;
    btii->enable_interrupt = pv_bt463_enable_interrupt;

    /*
     */
    PV_INT_CLEAR( pvp, PV_INT_VSYNC );
    rend_status = p_rend->status;
    while ( !rend_status.vsync_int ) {
	rend_status = p_rend->status;
    }
    bt_clean_window_tag( pvp );

    /*
     * Done
     */
    return 1;
}

/*
 * init_info_area
 *
 * Do some initialization of the info area
 */
static int
init_info_area( pvp )
    pv_info *pvp;
{
    int
	i,
	status = 0;

    pvInfo
	*p_info = (pvInfo *) pvp->p_info_area;

    p_info->pagein_vaddr = (caddr_t) NULL;
    p_info->pagein_page_count = 0;
    p_info->pagein_dirty = 0;
    for ( i = 0; i < PV_INFO_SIG_NUM; i++ ) {
	p_info->signal_reasons[i] = 0;
    }
    p_info->gfpa_halts_on_countdown = 0;

    return (status);
}


/*
 * Initialize the pv0 rendering chip.
 *
 * Note that this is also used to switch the device between mono and
 * stereo modes at run-time.  Also note that the absence of a PV_INT_VSYNC
 * disable in the mono case is intentional:  the cursor or colormap devices
 * may want a vsync interrupt.
 */
static int
init_pv( pvp, fullreset, stereo )
    pv_info *pvp;
    int fullreset;
    unsigned int stereo;
{
    int
	x,
	y,
	xlimit,
	ylimit,
	xdelta,
	ydelta,
	i,
	status = 0;

    pv_render_video_t
	*p_vid;

    pv_render_implement_t
	*p_rend;

    pv_graphic_tlb_t
	*p_table,
	tlb_entry;

    pv_pva_register_t
	*p_pva;

    pv_render_status_t
	rend_status;

    p_vid = PV_REND_VIDEO_BASE( pvp );
    p_table = PV_REND_TLB_BASE( pvp );
    p_rend = PV_REND_IMP_BASE( pvp );
    p_pva = PV_PVA_BASE( pvp );

    if ( fullreset ) {
	/*
	 * Determine monitor speed
	 */
	if ( pvp->ctlr == (struct controller *) NULL ) {
	    pvp->monitor_rate = 72;
	}
	else {
	    unsigned int
		tc_cycle,
		tmp,
		rate1,
		rate5,
		base,
		base1,
		base5;

	    extern unsigned int
		get_info();

	    struct item_list
		getinfo_list;

	    getinfo_list.function = GET_TC_SPEED;
	    getinfo_list.next_function = (struct item_list *) NULL;
	    if ( get_info( &getinfo_list ) != TRUE ||
		 getinfo_list.rtn_status != INFO_RETURNED ||
		 getinfo_list.output_data == 0 ) {
		/*
		 * Something's messed up.  Take a default.
		 */
		tc_cycle = 40;
	    }
	    else {
		tc_cycle = 10000 / getinfo_list.output_data;
	    }

	    p_vid->crt_enable = 0x00;			wbflush();

	    p_vid->vblank_s = 68;			wbflush();
	    p_vid->vblank_f = 107;			wbflush();
	    p_vid->vsync_s = 71;			wbflush();
	    p_vid->vsync_f = 74;			wbflush();

	    rend_status = p_rend->status;

	    rend_status.vsync_int = 0;
	    p_rend->status = rend_status;		wbflush();
	    rend_status = p_rend->status;
	    while ( !rend_status.vsync_int ) {
		rend_status = p_rend->status;
	    }
	    p_pva->counter = PV_PVA_COUNTER_MAX;	wbflush();

	    rend_status.vsync_int = 0;
	    p_rend->status = rend_status;		wbflush();
	    rend_status = p_rend->status;
	    while ( !rend_status.vsync_int ) {
		rend_status = p_rend->status;
	    }
	    base1 = p_pva->counter;

	    rend_status.vsync_int = 0;
	    p_rend->status = rend_status;		wbflush();
	    rend_status = p_rend->status;
	    while ( !rend_status.vsync_int ) {
		rend_status = p_rend->status;
	    }
	    p_pva->counter = PV_PVA_COUNTER_MAX;	wbflush();
	    for ( i = 0; i < 5; i++ ) {
		rend_status.vsync_int = 0;
		p_rend->status = rend_status;		wbflush();
		rend_status = p_rend->status;
		while ( !rend_status.vsync_int ) {
		    rend_status = p_rend->status;
		}
	    }
	    base5 = p_pva->counter;

	    rend_status.vsync_int = 0;
	    p_rend->status = rend_status;		wbflush();
	    rend_status = p_rend->status;

	    rate1 = PV_PVA_COUNTER_MAX - base1;
	    rate5 = PV_PVA_COUNTER_MAX - base5;
	    tmp = ( ( rate5 - rate1 ) * tc_cycle ) / 1000;
	    pvp->monitor_rate = ( ( ( 4 * 107 * 1000000 ) / tmp )
				  + 531 )
				/ 1063;
	}
    }

    /*
     * Initialize video registers
     * Load clock = pixel clock / 4
     */
    p_vid->crt_enable = 0x00;			wbflush();
    if ( stereo == PV_IOC_STEREO_24 ) {
	p_vid->vblank_s = 512;			wbflush();
	p_vid->vblank_f = 551;			wbflush();
	p_vid->vsync_s = 515;			wbflush();
	p_vid->vsync_f = 518;			wbflush();
	p_vid->hblank_s = 320;			wbflush();
	p_vid->hblank_f = 424;			wbflush();
	p_vid->hsync_s = 328;			wbflush();
	p_vid->hsync_f = 368;			wbflush();
	p_vid->hsync2 = 308;			wbflush();
	PV_INT_ENABLE( pvp, PV_INT_STEREO );
	pvp->stereo_mode = PV_INFO_STEREO_24;
    }
    else if ( stereo == PV_IOC_STEREO_1212 ) {
	p_vid->vblank_s = 1024;			wbflush();
	p_vid->vblank_f = 1063;			wbflush();
	p_vid->vsync_s = 1027;			wbflush();
	p_vid->vsync_f = 1030;			wbflush();
	p_vid->hblank_s = 320;			wbflush();
	p_vid->hblank_f = 424;			wbflush();
	p_vid->hsync_s = 328;			wbflush();
	p_vid->hsync_f = 368;			wbflush();
	p_vid->hsync2 = 308;			wbflush();
	PV_INT_DISABLE( pvp, PV_INT_STEREO );
	PV_INT_ENABLE( pvp, PV_INT_VSYNC );
	pvp->stereo_mode = PV_INFO_STEREO_1212;
    }
    else {
	if ( pvp->monitor_rate > 64 ) {
	    p_vid->vblank_s = 1024;			wbflush();
	    p_vid->vblank_f = 1063;			wbflush();
	    p_vid->vsync_s = 1027;			wbflush();
	    p_vid->vsync_f = 1030;			wbflush();
	    p_vid->hblank_s = 320;			wbflush();
	    p_vid->hblank_f = 424;			wbflush();
	    p_vid->hsync_s = 328;			wbflush();
	    p_vid->hsync_f = 368;			wbflush();
	    p_vid->hsync2 = 308;			wbflush();
	}
	else {
	    p_vid->vblank_s = 1024;			wbflush();
	    p_vid->vblank_f = 1043;			wbflush();
	    p_vid->vsync_s = 1027;			wbflush();
	    p_vid->vsync_f = 1030;			wbflush();
	    p_vid->hblank_s = 320;			wbflush();
	    p_vid->hblank_f = 425;			wbflush();
	    p_vid->hsync_s = 340;			wbflush();
	    p_vid->hsync_f = 365;			wbflush();
	    p_vid->hsync2 = 328;			wbflush();
	}
	PV_INT_DISABLE( pvp, PV_INT_STEREO );
	pvp->stereo_mode = PV_INFO_STEREO_NONE;
    }
    p_vid->crt_enable = 0x01;			wbflush();

    /*
     * TLB must be inited.  These values are very important for
     * console operation.  See comment in that area for details.
     */
    if ( p_rend->imp_rev.update_size == PV_RENDER_IMP_USZ_4X2 ) {
	xlimit = 512;
	xdelta = 128;
	ylimit = 2560;
	ydelta = 128;
    }
    else {
	xlimit = 1024;
	xdelta = 256;
	ylimit = 1280;
	ydelta = 128;
    }
    i = 0;
    for ( y = 0; y < ylimit; y += ydelta ) {
	for ( x = 0; x < xlimit; x += xdelta ) {
	    tlb_entry.pixelmap_id = PV_INVALID_PIXELMAP_ID;
	    tlb_entry.x = (x >> 7);
	    tlb_entry.y = (y >> 7);
	    tlb_entry.dirty = 0;
	    p_table[i] = tlb_entry;
	    i++;
	}
    }
    x = 0;
    y = ylimit;
    for ( ; i < PV_RENDER_TLB_TOTAL; i++ ) {
	if ( x >= xlimit ) {
	    x = 0;
	    y += ydelta;
	}
	tlb_entry.pixelmap_id = PV_INVALID_PIXELMAP_ID;
	tlb_entry.x = (x >> 7);
	tlb_entry.y = (y >> 7);
	tlb_entry.dirty = 0;
	p_table[i] = tlb_entry;
	x += xdelta;
    }
    wbflush();

    /*
     * Done
     */
    return (status);
}

/*
 * Initialize the module.  This is stuff that should mostly go into
 * the PROM.
 *
 */
static int
init_devices( pvp, fullreset )
     pv_info *pvp;
     int fullreset;
{
    int
	i,
      	status = 0,
	s,
	try;

    pv_pva_register_t
    	*p_pva;

    pv_csr_t
	csr2,
      	csr;

    pv_register_t
	counter,
	*p_gcp_cc,
	intr_status;


    pv_render_implement_t
	*p_rend_imp = PV_REND_IMP_BASE( pvp );

    pv_render_status_t
	rend_status;

    struct bt463
	*btp;

    p_pva = PV_PVA_BASE( pvp );
    p_gcp_cc = (pv_register_t *) ( (unsigned long) PV_GCP_BASE( pvp )
				 + PV_GCP_CC_OFFSET );

    if (fullreset)
    {
	p_pva->intr_mask_set = PV_PVA_INTR_PB1;		wbflush();
      	csr = p_pva->csr;

#ifndef DONT_DO_PVA_RESET
	csr.pb_reset = 1;
	csr.gpo = 1;
	p_pva->csr = csr;
	wbflush();

	DELAY(1000);

	csr.pb_reset = 0;
	csr.gpo = 0;
	p_pva->csr = csr;
	wbflush();
#endif

#ifdef	GFPA_IS_SYNCHRONOUS

	s = PV_RAISE_SPL();
	*p_gcp_cc = pvp->gcp_clock_control;
	wbflush();
	intr_status = p_pva->intr_status;
	if ( intr_status & PV_PVA_INTR_PB1 ) {
	    p_pva->intr_status = PV_PVA_INTR_PB1;	wbflush();
	}
	p_pva->intr_mask_clear = PV_PVA_INTR_PB1;	wbflush();
	intr_status = p_pva->intr_status;		wbflush();
	PV_LOWER_SPL(s);

#else	/* GFPA_IS_SYNCHRONOUS */

	csr2 = csr = p_pva->csr;
	csr.pb_reset = 1;
	csr2.pb_reset = 0;

	try = 0;
	s = PV_RAISE_SPL();
	do {
	    p_pva->csr = csr;
	    wbflush();
	    p_pva->counter = MAGIC_PVA_COUNTDOWN_VALUE;
	    wbflush();
	    p_pva->csr = csr2;
	    wbflush();
	    *p_gcp_cc = pvp->gcp_clock_control;
	    wbflush();
	    counter = p_pva->counter;
	} while ( counter == 0 && try++ < MAGIC_GCP_CLOCK_TRY_COUNT );
	DELAY( 50000 );
	intr_status = p_pva->intr_status;
	if ( intr_status & PV_PVA_INTR_PB1 ) {
	    p_pva->intr_status = PV_PVA_INTR_PB1;	wbflush();
	}
	p_pva->intr_mask_clear = PV_PVA_INTR_PB1;	wbflush();
	intr_status = p_pva->intr_status;		wbflush();
	PV_LOWER_SPL(s);
	if ( counter == 0 ) {
	    /*
	     * Failed to reset clock control.  Try to take the unit offline
	     */
	    PRINTFBUF( ("pvreset: couldn't reset clock control register\n" ) );
	    return (1);
	}
	wbflush();

#endif	/* GFPA_IS_SYNCHRONOUS */

    }

    if ( ( status = init_pv( pvp, fullreset, PV_INFO_STEREO_NONE ) ) != 0 ) {
        return (status);
    }

    return (status);
}


/*
 * invalidate_tlb
 *
 * This function is called to invalidate either of the software
 * shadow tlb's.  The entire tlb will be invalidated.
 *
 * 0 - success
 * 1 - failure
 */
static int
invalidate_tlb( pvp, table )
     pv_info *pvp;
     int table;
{
  pv_pva_tlb_entry_t
    tlb_entry,
    *p_table;

  int
    i,
    table_size,
    status = 0;

  if ( table == INVALIDATE_PAGER ) {
    p_table = (pv_pva_tlb_entry_t *) pvp->p_page_table_0;
    table_size = PV_PVA_TABLE0_SIZE;
    if ( pvp->image_hardlock_in_progress ) {
	/*
	 * If a hardlock is in progress and we're doing a put image,
	 * just keep the entries for the image in the TLB.  It won't
	 * hurt as we expect to restart the image operation later.
	 */
	for ( i = 0; i < table_size; i++ ) {
	    if ( p_table->vpn < pvp->image_va_first_vpn ||
		 p_table->vpn > pvp->image_va_last_vpn ) {
		INVALID_STLB_ENTRY( pvp, i, p_table );
#ifndef DO_LOCKOUT_IN_INVALIDATION
		PV_DEBUG( PV_YAK,
		    PRINTFPOLL( ( "pv_inval_tlb: stlb0[%d] with 0x%lx\n",
				i, * (unsigned long *) p_table ) ); );
#endif
	    }
	    p_table++;
	}
    }
    else {
	for ( i = 0; i < table_size; i++ ) {
	    INVALID_STLB_ENTRY( pvp, i, p_table );
#ifndef DO_LOCKOUT_IN_INVALIDATION
	    PV_DEBUG( PV_YAK,
		    PRINTFPOLL( ( "pv_inval_tlb: stlb0[%d] with 0x%lx\n",
				i, * (unsigned long *) p_table ) ); );
#endif
	    p_table++;
	}
    }
  }
  else {
    p_table = (pv_pva_tlb_entry_t *) pvp->p_page_table_1;
    table_size = PV_PVA_TABLE1_SIZE;
    for ( i = 0; i < table_size; i++ ) {
	INVALID_STLB_ENTRY( pvp, i, p_table );
#ifndef DO_LOCKOUT_IN_INVALIDATION
	PV_DEBUG( PV_YAK,
	    PRINTFPOLL( ( "pv_inval_tlb: stlb1[%d] with 0x%lx\n",
			i, * (unsigned long *) p_table ) ); );
#endif
	p_table++;
    }
  }

  return (status);
}

/*
 * invalidate_pva_tlb
 *
 * This function is called to invalidate tlb entries for either the
 * server, the pager, or anything.  The entire tlb will be invalidated.
 *
 * 0 - success
 * 1 - failure
 */
static int
invalidate_pva_tlb( pvp, table )
     pv_info *pvp;
     int table;
{
  pv_pva_tlb_entry_t
    *p_table,
    entry;

  int
    i,
    pid,
    status = 0;

  p_table = PV_PVA_BASE(pvp)->tlb;

  /*
   * INVALIDATE_PAGER is a special case.  If there is a hardlock on 
   * an image outstanding, we leave any translation for the range in
   * place as we've locked it and it can just wait.
   */
  if ( table == INVALIDATE_PAGER ) {
    if ( pvp->image_hardlock_in_progress ) {
      for ( i = 0; i < PV_PVA_TLB_ENTRIES; i++ ) {
	entry = *p_table;
  	if ( ( entry.vpn < pvp->image_va_first_vpn ||
	       entry.vpn > pvp->image_va_last_vpn ) &&
	     entry.pid == PV_PVA_PID_PAGER ) {
	  INVALID_PVA_TLB_ENTRY( pvp, i, (&entry) );
	  *p_table = entry;
#ifndef DO_LOCKOUT_IN_INVALIDATION
	  PV_DEBUG( PV_YAK,
	    PRINTFPOLL( ( "pv_inval_pva_tlb: pva[%d] with 0x%lx\n",
			i, * (unsigned long *) &entry ) ); );
#endif
	}
	p_table++;
      }
    }
    else {
      for ( i = 0; i < PV_PVA_TLB_ENTRIES; i++ ) {
	entry = *p_table;
  	if ( entry.pid == PV_PVA_PID_PAGER ) {
	  INVALID_PVA_TLB_ENTRY( pvp, i, (&entry) );
	  *p_table = entry;
#ifndef DO_LOCKOUT_IN_INVALIDATION
	  PV_DEBUG( PV_YAK,
	    PRINTFPOLL( ( "pv_inval_pva_tlb: pva[%d] with 0x%lx\n",
			i, * (unsigned long *) &entry ) ); );
#endif
	}
	p_table++;
      }
    }
  }
  else {
    if ( table == INVALIDATE_SERVER ) {
      pid = PV_PVA_PID_SERVER;
    }
    else {
      pid = PV_PVA_PID_INVALID;
    }

    for ( i = 0; i < PV_PVA_TLB_ENTRIES; i++ ) {
      entry = *p_table;
      if ( pid == PV_PVA_PID_INVALID || entry.pid == pid ) {
	INVALID_PVA_TLB_ENTRY( pvp, i, (&entry) );
	*p_table = entry;
#ifndef DO_LOCKOUT_IN_INVALIDATION
	PV_DEBUG( PV_YAK,
	    PRINTFPOLL( ( "pv_inval_pva_tlb: pva[%d] with 0x%lx\n",
			i, * (unsigned long *) &entry ) ); );
#endif
      }
      p_table++;
    }
  }

  wbflush();

  return (status);
}


/*
 * pv_ioctl
 *
 * ioctl functions private to this class of devices
 */
int
pv_ioctl(closure, cmd, data, flag)
     caddr_t closure;
     int cmd;
     caddr_t data;
     int flag;
{
    pv_info
	*pvp = (pv_info *) closure;

    static unsigned int
	pid_masks[PV_IOC_PID_NUM] = {
	    PV_IOC_PID_PV,		/* PV_IOC_PID_PV_INDEX */
	    PV_IOC_PID_GCP		/* PV_IOC_PID_GCP_INDEX */
	},
	allowed_sig_masks[PV_IOC_PID_NUM] = {
	    PV_IOC_SIG_PMAP_FAULT,
	    PV_IOC_SIG_VA_PAGEIN
	    | PV_IOC_SIG_MODULE_ERROR
	    | PV_IOC_SIG_CLIPPING
	    | PV_IOC_SIG_PVA_TLB1_FAULT
	    | PV_IOC_SIG_PACKET_DONE
	};

    int
	i,
	status = 0;

    register pv_ioc
	*pvi = (pv_ioc *) data;

    switch( pvi->cmd ) 
    {
     case PV_IOC_MAP_OPTION:
	if ( pvi->info.data != (unsigned long) NULL ) {
	    if ( status = copyout( (caddr_t)pvp->pv,
				   (caddr_t) pvi->info.data,
				   sizeof(pvInfo) ) ) {
	        return (status);
	    }
	}
	if ( ( pvi->info.data = (unsigned long) ws_map_region(
					(caddr_t) pvp->p_pvo,
					NULL,
					PV_MODULE_SIZE, 0600, (int *) NULL ) )
	     == (unsigned long) NULL ) {
	    return (ENOMEM);
	}
	pvi->screen = (short) PV_DTYPE;
	break;

     case PV_IOC_MAP_OPTION_SPARSE:
	if ( pvi->info.data != (unsigned long) NULL ) {
	    if ( status = copyout( (caddr_t) pvp->pv,
				   (caddr_t) pvi->info.data,
				   sizeof(pvInfo) ) ) {
	        return (status);
	    }
	}
	if ( ( pvi->info.data = (unsigned long) ws_map_region(
					(caddr_t) pvp->p_pvo_s,
					NULL,
					PV_SPARSE_MODULE_SIZE,
					0600, (int *) NULL ) )
	     == (unsigned long) NULL ) {
	    return (ENOMEM);
	}
	pvi->screen = (short) PV_DTYPE;
	break;

      /*
       * ioctl to get the current process assignments for signals.
       */
      case PV_IOC_GET_PID:
	{
	    unsigned int
		pid_mask = 0;

	    for ( i = 0; i < PV_IOC_PID_NUM; i++ ) {
		if ( pvp->p_proc[i] == u.u_procp &&
		     pvp->pid[i] == u.u_procp->p_pid ) {
		    pid_mask |= pid_masks[i];
		}
	    }
	    pvi->info.pid_mask = pid_mask;
	}
	break;

      /*
       * ioctl used by a process to indicate that it will accept
       * signals for either or both of the PV and GCP.
       *
       * Once both are accepted, the device is marked valid.
       */
      case PV_IOC_SET_PID:
	{
	    int
	        count = 0;

	    for ( i = 0; i < PV_IOC_PID_NUM; i++ ) {
		if ( pvi->info.pid_mask & pid_masks[i] ) {
		    if ( i == PV_IOC_PID_GCP_INDEX ) {
			/*
			 * First, check for a dangling RM process.
			 * A new server is trying to register so see if
			 * we have old RMs lying around.  We'll assume
			 * the DDX folks do things in the right order...
			 */
			if ( pvp->p_proc[PV_IOC_PID_PV_INDEX]
			     != (struct proc *) NULL &&
			     pvp->pid[PV_IOC_PID_PV_INDEX]
			     != u.u_procp->p_pid &&
			     pvp->pid[PV_IOC_PID_PV_INDEX]
			     == pvp->p_proc[PV_IOC_PID_PV_INDEX]->p_pid ) {
			  /*
			   * Found a dangler.  Take down *everything*
			   */
			  int
			    j;

			  for ( j = 0; j < npv_softc; j++ ) {
			    if ( pv_softc[j].p_proc[PV_IOC_PID_PV_INDEX]
				 != (struct proc *) NULL &&
				 pv_softc[j].pid[PV_IOC_PID_PV_INDEX]
				 != u.u_procp->p_pid &&
				 pv_softc[j].pid[PV_IOC_PID_PV_INDEX]
				 == pv_softc[j].p_proc[PV_IOC_PID_PV_INDEX]->p_pid ) {
			      PRINTFBUF( ("pv: sending SIGKILL to RM id %d\n",
				pv_softc[j].p_proc[PV_IOC_PID_PV_INDEX]->p_pid ) );
			      psignal( pv_softc[j].p_proc[PV_IOC_PID_PV_INDEX],
				       SIGKILL );
			    }
			  }
			  psignal( u.u_procp, SIGKILL );
			  return (-1);
			}
		    }
		    pvp->p_proc[i] = u.u_procp;
		    pvp->pid[i] = u.u_procp->p_pid;
		}
		if ( pvp->p_proc[i] != (struct proc *) NULL &&
		     pvp->pid[i] == pvp->p_proc[i]->p_pid ) {
		    count++;
		}
	    }

	    if ( count == PV_IOC_PID_NUM ) {
	        pvp->state |= PV_INFO_STATE_VALID;
	    }

	    /*
	     * GCP fault handler needs to use the VM pageout hook and do
	     * some more interrupts.
	     */
	    if ( pvi->info.pid_mask & PV_IOC_PID_GCP ) {
		extern int ws_register_vm_callback();

		ws_register_vm_callback( pvp->screen.screen, pvp->vm_hook, pvp );
	    }

	}
	break;

      /*
       * ioctl to get the GCP software register assignments
       */
      case PV_IOC_GET_CONFIG:
	if ( pvp->state & PV_INFO_STATE_GCP_VALID ) {
	    for ( i = 0; i < PV_IOC_CNFG_NUM; i++ ) {
		pvi->info.config.type[i] = pvp->gcp_reg_type[i];
		pvi->info.config.offset[i] = pvp->gcp_reg_offset[i];
	    }
	}
	else {
	    status = ENODEV;
	}
	break;

      /*
       * ioctl to define the GCP register assignments
       */
      case PV_IOC_SET_CONFIG:
	{
	    unsigned int
		offset;

	    vm_offset_t
		reg[PV_IOC_CNFG_NUM];

	    /*
	     * First, validate all arguments while calculating the
	     * actual register addresses.
	     */
	    for ( i = 0; i < PV_IOC_CNFG_NUM; i++ ) {
		offset = pvi->info.config.offset[i];
		if ( (unsigned) offset & 0x3 ) {
		    return (EINVAL);
		}

		switch ( pvi->info.config.type[i] ) {

		case PV_IOC_TYPE_REG:
		    if ( ( offset >= PV_IOC_CNFG_OFF_GLB_MIN &&
			   offset <= PV_IOC_CNFG_OFF_GLB_MAX ) ||
			 ( offset >= PV_IOC_CNFG_OFF_FU0_MIN &&
			   offset <= PV_IOC_CNFG_OFF_FU0_MAX ) ||
			 ( offset >= PV_IOC_CNFG_OFF_FU1_MIN &&
			   offset <= PV_IOC_CNFG_OFF_FU1_MAX ) ||
			 ( offset >= PV_IOC_CNFG_OFF_FU2_MIN &&
			   offset <= PV_IOC_CNFG_OFF_FU2_MAX ) ||
			 ( offset >= PV_IOC_CNFG_OFF_FU3_MIN &&
			   offset <= PV_IOC_CNFG_OFF_FU3_MAX ) ) {
		        reg[i] = (vm_offset_t) PV_GCP_BASE( pvp );
			reg[i] += offset;
		    }
		    else {
			return (EINVAL);
		    }
		    break;

		case PV_IOC_TYPE_SRAM:
		    if ( offset >= (vm_offset_t) 0 &&
			 offset < (vm_offset_t) pvp->sram_size ) {
			reg[i] = (vm_offset_t) PV_SRAM_BASE( pvp );
			reg[i] += offset;
		    }
		    else {
			return (EINVAL);
		    }
		    break;

		default:
		    return (EINVAL);
		}
	    }

	    /*
	     * Now copy everything out
	     */
	    for ( i = 0; i < PV_IOC_CNFG_NUM; i++ ) {
		pvp->gcp_reg_type[i] = pvi->info.config.type[i];
		pvp->gcp_reg_offset[i] = pvi->info.config.offset[i];
		pvp->gcp_reg[i] = reg[i];
	    }

	    /*
	     * Mark device as GCP safe
	     */
	    pvp->state |= PV_INFO_STATE_GCP_VALID;
	}
	break;

      /*
       * Get the signals for which the current process has requested
       * delivery.
       */
      case PV_IOC_GET_SIGNALS:
	{
	    unsigned int
		sig_mask = 0;

	    for ( i = 0; i < PV_IOC_PID_NUM; i++ ) {
		if ( pvp->p_proc[i] == u.u_procp &&
		     pvp->pid[i] == u.u_procp->p_pid ) {
		    sig_mask |= pvp->sig_mask[i];
		}
	    }
	    pvi->info.sig_mask = sig_mask;
	}
	break;

      /*
       * Specify interest in certain signal types
       */
      case PV_IOC_SET_SIGNALS:
	{
	    int
		s;

	    for ( i = 0; i < PV_IOC_PID_NUM; i++ ) {
		if ( pvp->p_proc[i] == u.u_procp &&
		     pvp->pid[i] == u.u_procp->p_pid ) {
		    pvp->sig_mask[i] = pvi->info.sig_mask
		      		     & allowed_sig_masks[i];
		}
	    }
	    /*
	     * Enable interrupts from PAGE_INT assertion
	     */
	    if ( pvp->sig_mask[PV_IOC_PID_PV_INDEX] & PV_IOC_SIG_PMAP_FAULT ) {
		s = PV_RAISE_SPL();
		PV_INT_ENABLE( pvp, PV_INT_PAGE );
		PV_LOWER_SPL(s);
	    }

	}
	break;

      /*
       * Perform a pixelmap pageout/pagein operation
       */
      case PV_IOC_PMAP_PAGEOUTIN:
	if ( pvi->info.pageoutin.count < 0 ||
	     pvi->info.pageoutin.count > PV_IOC_PAGEOUTIN_COUNT_MAX ) {
	    return (EINVAL);
	}
	status = ioctl_pageoutin( pvp, pvi );
	break;

        /*
	 * Assist in getting an image back from the module
	 */
      case PV_IOC_GET_IMAGE:
	{
	    vm_offset_t
		user_addr = pvi->info.image.addr;

	    unsigned long
		user_size = pvi->info.image.size;

	    pv_register_t
		user_put_value = pvi->info.image.put_val;

	    pv_graphic_tlb_t
		*p_pv_tlb;

	    int
		hard_lock = 1,		/* Always! */
		s;

	    /*
	     * Run some tests on the data
	     */
	    if ( user_addr == (vm_offset_t) NULL ) break;

	    /*
	     * Lock it down and force the STLB entries.
	     */
	    pvp->image_failure = 0;
	    pvp->image_va_first_vpn = PV_VA_TO_VPN( user_addr );
	    pvp->image_va_last_vpn = PV_VA_TO_VPN( (user_addr+user_size) );
	    if ( hard_lock ) {
		if ( status = vm_lock( pvp, user_addr, user_size, 
				       (VM_PROT_READ|VM_PROT_WRITE), 0 ) ) {
	PV_DEBUG( PV_TALK,
	    PRINTFPOLL( ( "get_image: lock failed addr=0x%lx, size=0x%x\n",
				user_addr, user_size ) ); );
		    return (status);
		}
		pvp->image_hardlock_in_progress = 1;
	    }
	    else {
		pvp->image_check_va_range = 1;
	    }

	    if ( status = vm_load_stlb( pvp, user_addr, user_size, 0, 0, hard_lock, 1 ) ) {
		if ( hard_lock ) {
		    vm_unlock( pvp, user_addr, user_size, 0 );
		}
		if ( pvp->image_failure ) {
		    status = PV_ERRNO_RETRY_IMAGE;
		    invalidate_tlb( pvp, INVALIDATE_PAGER );
		    invalidate_pva_tlb( pvp, INVALIDATE_PAGER );
		}
		pvp->image_hardlock_in_progress = 0;
		pvp->image_check_va_range = 0;
	        return (status);
	    }

	    /*
	     * Do the put register
	     */
	    PV_DEBUG( PV_TALK,
		PRINTFPOLL( ( "get_image: about to write PUT pointer\n" )); );
	    STORE_SOFTWARE_REG( pvp, PV_IOC_CNFG_GFPA_PUT, user_put_value );
	    PV_DEBUG( PV_TALK,
		PRINTFPOLL( ( "get_image: wrote PUT pointer\n" )); );

	    /*
	     * Wait for completion.  PAGE_INT will come in then we
	     * spin waiting for the PV to clear, then restore the
	     * original graphics TLB.
	     */
	    if (!( pvp->state & PV_INFO_STATE_SAVED_GTLB_VALID ) ) {
		PV_DEBUG( PV_TALK,
		    PRINTFPOLL( ( "get_image: Doorbell NOT rung\n" )); );
		s = PV_RAISE_SPL();
		if (!( pvp->state & PV_INFO_STATE_SAVED_GTLB_VALID ) ) {
		    SLEEP_ON_IMAGE( pvp );
		}
		PV_LOWER_SPL(s);
		PV_DEBUG( PV_TALK,
		    PRINTFPOLL( ( "get_image: Doorbell rang\n" ) ); );
	    }

	    PV_DEBUG( PV_TALK,
		PRINTFPOLL( ( "get_image: Getting base of GTLB\n" )); );
	    p_pv_tlb = PV_REND_TLB_BASE( pvp );
	    PV_DEBUG( PV_TALK,
		PRINTFPOLL( ( "get_image: Got it\n" )); );
	    pvp->state &= ~PV_INFO_STATE_SAVED_GTLB_VALID;
	    PV_DEBUG( PV_TALK,
		PRINTFPOLL( ( "get_image: Cleared state bit\n" ) ); );
	    p_pv_tlb[pvp->pixelmap_count-1] = pvp->saved_graphic_tlb;
	    PV_DEBUG( PV_TALK,
		PRINTFPOLL( ( "get_image: restored GTLB entry\n" )); );

	    /*
	     * Safe, no more VA checks
	     */
	    pvp->image_check_va_range = 0;
	    pvp->image_hardlock_in_progress = 0;

	    /*
	     * Unlock
	     */
	    if ( status = vm_unload_stlb( pvp, user_addr, user_size, 0, 0 ) ) {
		if ( hard_lock ) {
		    vm_unlock( pvp, user_addr, user_size, 0 );
		}
		if ( pvp->image_failure ) {
		    status = PV_ERRNO_RETRY_IMAGE;
		    invalidate_tlb( pvp, INVALIDATE_PAGER );
		    invalidate_pva_tlb( pvp, INVALIDATE_PAGER );
		}
	        return (status);
	    }
	    if ( hard_lock ) {
		if ( status = vm_unlock( pvp, user_addr, user_size, 0 ) ) {
		    return (status);
		}
	    }

	    /*
	     * Done
	     */
	}
	break;

	/*
	 * Assist the server in getting an image into the module
	 */
      case PV_IOC_PUT_IMAGE:
	{
	    vm_offset_t
		user_addr = pvi->info.image.addr;

	    unsigned long
		user_size = pvi->info.image.size;

	    pv_register_t
		user_put_value = pvi->info.image.put_val;

	    pv_graphic_tlb_t
		*p_pv_tlb;

	    int
		hard_lock = pvi->info.image.hard_lockdown,
		s;

	    /*
	     * Run some tests on the data
	     */
	    if ( user_addr == (vm_offset_t) NULL ) break;

	    /*
	     * Lock it down and force the STLB entries.
	     */
	    pvp->image_failure = 0;
	    pvp->image_va_first_vpn = PV_VA_TO_VPN( user_addr );
	    pvp->image_va_last_vpn = PV_VA_TO_VPN( (user_addr+user_size) );
	    if ( hard_lock ) {
		if ( status = vm_lock( pvp, user_addr, user_size,
				       VM_PROT_READ, 0 ) ) {
	PV_DEBUG( PV_TALK,
	    PRINTFPOLL( ( "PUT IMAGE: vm_lock failed.  status = %d\n", status ) ); );
		    return (status);
		}
		pvp->image_hardlock_in_progress = 1;
	    }
	    else {
		pvp->image_check_va_range = 1;
	    }

	    if ( status = vm_load_stlb( pvp, user_addr, user_size, 0, 0, hard_lock, 0 ) ) {
    PV_DEBUG( PV_TALK,
	PRINTFPOLL( ( "PUT IMAGE: vm_load_stlb failed.  status = %d\n", status )); );
		if ( hard_lock ) {
		    vm_unlock( pvp, user_addr, user_size, 0 );
		}
		if ( pvp->image_failure ) {
		    status = PV_ERRNO_RETRY_IMAGE;
		    invalidate_tlb( pvp, INVALIDATE_PAGER );
		    invalidate_pva_tlb( pvp, INVALIDATE_PAGER );
		}
		pvp->image_hardlock_in_progress = 0;
		pvp->image_check_va_range = 0;
	        return (status);
	    }

	    /*
	     * Do the put register
	     */
    PV_DEBUG( PV_TALK,
	PRINTFPOLL( ( "PUT IMAGE: locked 0x%x bytes at 0x%lx\n", user_size,
			user_addr ) ); );
    PV_DEBUG( PV_TALK,
	PRINTFPOLL( ( "PUT IMAGE: about to write 0x%x to 0x%lx\n",
			user_put_value,
			pvp->gcp_reg[PV_IOC_CNFG_GFPA_PUT] ) ); );
	    STORE_SOFTWARE_REG( pvp, PV_IOC_CNFG_GFPA_PUT, user_put_value );

	    /*
	     * Wait for completion.  PAGE_INT will come in then we
	     * spin waiting for the PV to clear, then restore the
	     * original graphics TLB.
	     */
	    if (!( pvp->state & PV_INFO_STATE_SAVED_GTLB_VALID ) ) {
		s = PV_RAISE_SPL();
		if (!( pvp->state & PV_INFO_STATE_SAVED_GTLB_VALID ) ) {
		    SLEEP_ON_IMAGE( pvp );
		}
		PV_LOWER_SPL(s);
	    }

	    p_pv_tlb = PV_REND_TLB_BASE( pvp );
	    pvp->state &= ~PV_INFO_STATE_SAVED_GTLB_VALID;
	    p_pv_tlb[pvp->pixelmap_count-1] = pvp->saved_graphic_tlb;

	    /*
	     * Safe, no more VA checks
	     */
	    pvp->image_hardlock_in_progress = 0;
	    pvp->image_check_va_range = 0;

	    /*
	     * Unlock
	     */
	    if ( status = vm_unload_stlb( pvp, user_addr, user_size, 0, 0 ) ) {
    PV_DEBUG( PV_TALK,
	PRINTFPOLL( ( "PUT IMAGE: vm_unload_stlb failed.  status = %d\n", status ) ); );
		if ( hard_lock ) {
		    vm_unlock( pvp, user_addr, user_size, 0 );
		}
		if ( pvp->image_failure ) {
		    status = PV_ERRNO_RETRY_IMAGE;
		    invalidate_tlb( pvp, INVALIDATE_PAGER );
		    invalidate_pva_tlb( pvp, INVALIDATE_PAGER );
		}
	        return (status);
	    }
	    if ( hard_lock ) {
	        if ( status = vm_unlock( pvp, user_addr, user_size, 0 ) ) {
    PV_DEBUG( PV_TALK,
	PRINTFPOLL( ( "PUT IMAGE: vm_unlock failed.  status = %d\n", status ) ); );
		    return (status);
		}
	    }

	    /*
	     * Done
	     */
	}
	break;

      /*
       * Get the stereo/mono mode of the device
       */
      case PV_IOC_GET_STEREO:
	switch ( pvp->stereo_mode ) {
	  case PV_INFO_STEREO_NONE:
		pvi->info.stereo.mode = PV_IOC_STEREO_NONE;
		break;

	  case PV_INFO_STEREO_24:
		pvi->info.stereo.mode = PV_IOC_STEREO_24;
		break;

	  case PV_INFO_STEREO_1212:
		pvi->info.stereo.mode = PV_IOC_STEREO_1212;
		pvi->info.stereo.win_tag_start = pvp->stereo_wtag_start;
		pvi->info.stereo.win_tag_count = pvp->stereo_wtag_count;
		break;

	  default:
		pvi->info.stereo.mode = PV_IOC_STEREO_NONE;
		break;
	}
	break;

      /*
       * Set stereo mode
       */
      case PV_IOC_SET_STEREO:
	{
	    int
		start,
		count,
		s;

	    s = PV_RAISE_SPL();
	    status = init_pv( pvp, 0, ( pvi->info.stereo.mode ) );
	    if ( pvi->info.stereo.mode == PV_IOC_STEREO_1212 ) {
		start = pvi->info.stereo.win_tag_start;
		count = pvi->info.stereo.win_tag_count;
		if ( start < 0 )
		    start = 0;
		else if ( start > WINDOW_TAG_COUNT )
		    start = WINDOW_TAG_COUNT;
		if ( count > ( WINDOW_TAG_COUNT - start + 1 ) )
		    count = WINDOW_TAG_COUNT - start + 1;
		pvp->stereo_wtag_start = start;
		pvp->stereo_wtag_count = count;
	    }
	    PV_LOWER_SPL(s);
	}
	break;

      /*
       * Server touched a faulted page.  Complete translation and send
       * PVA and GCP on their way.
       */
      case PV_IOC_TOUCHED_PAGE:
	status = vm_touched_page( pvp, u.u_procp, pvi->info.touched_va );
	break;

      /*
       * Get the dcc bits used by the console
       */
      case PV_IOC_GET_CONSOLE_DCC:
	pvi->info.console_dcc = pvp->dcc_user;
	break;

      /*
       * Set (or reset) the dcc bits used by the console
       */
      case PV_IOC_SET_CONSOLE_DCC:
	if ( pvi->info.console_dcc >= 16 ) {
	    pvp->dcc_user = pvp->dcc_default;
	}
	else {
	    pvp->dcc_user = pvi->info.console_dcc;
	}
	set_packet_dcc( pvp );
	break;

      /*
       * Load some window tag values.
       */
      case PV_IOC_WRITE_WINDOW_TAG:
	{
	    pv_ioc_window_tag
		*wtp = &pvi->info.window_tag;

	    pv_window_tag_cell
		entry;

	    int
		windex,
		s,
		error;

	    struct bt463info
		*bti = (struct bt463info *) pvp->cmf.cmc;

	    s = PV_RAISE_SPL();

	    for ( i = wtp->left_start;
		  i < wtp->left_start + wtp->left_ncells;
		  i++ ) {
		copyin( wtp->p_left_cells + i, &entry, sizeof( entry ) );
		windex				= entry.windex;
		if ( windex < 0 || windex >= WINDOW_TAG_COUNT ) continue;
		pvp->wt_left_cell[windex].low	= entry.low;
		pvp->wt_left_cell[windex].mid	= entry.mid;
		pvp->wt_left_cell[windex].high	= entry.high;
		if ( windex < pvp->wt_left_min_dirty )
		    pvp->wt_left_min_dirty = windex;
		if ( windex > pvp->wt_left_max_dirty )
		    pvp->wt_left_max_dirty = windex;
	    }
	    for ( i = wtp->right_start;
		  i < wtp->right_start + wtp->right_ncells;
		  i++ ) {
		copyin( wtp->p_right_cells + i, &entry, sizeof( entry ) );
		windex				= entry.windex;
		if ( windex < 0 || windex >= WINDOW_TAG_COUNT ) continue;
		pvp->wt_right_cell[windex].low	= entry.low;
		pvp->wt_right_cell[windex].mid	= entry.mid;
		pvp->wt_right_cell[windex].high	= entry.high;
		if ( windex < pvp->wt_right_min_dirty )
		    pvp->wt_right_min_dirty = windex;
		if ( windex > pvp->wt_right_max_dirty )
		    pvp->wt_right_max_dirty = windex;
	    }
	    pvp->wt_dirty = 1;

	    if ( bti->enable_interrupt ) 
		(*bti->enable_interrupt)(bti);
	    else 
		bt_clean_window_tag(pvp);

	    PV_LOWER_SPL(s);

	}
	break;

      case PV_IOC_GET_GCP_CLOCK_CONTROL:
	pvi->info.gcp_clock_control = pvp->gcp_clock_control;
	break;

      case PV_IOC_SET_GCP_CLOCK_CONTROL:
	{
	    pv_register_t
		counter,
		intr_status,
		*p_gcp_cc;

	    pv_pva_register_t
		*p_pva;

	    pv_csr_t
		csr2,
		csr;

	    int
		try,
		s;

	    p_pva = PV_PVA_BASE( pvp );
	    p_pva->intr_mask_set = PV_PVA_INTR_PB1;		wbflush();
	    pvp->gcp_clock_control = pvi->info.gcp_clock_control;
	    p_gcp_cc = (pv_register_t *) ( (unsigned long) PV_GCP_BASE( pvp )
					 + PV_GCP_CC_OFFSET );
	    halt_gfpa( pvp );

#ifdef	GFPA_IS_SYNCHRONOUS

	    s = PV_RAISE_SPL();
	    *p_gcp_cc = pvp->gcp_clock_control;
	    wbflush();
	    intr_status = p_pva->intr_status;
	    if ( intr_status & PV_PVA_INTR_PB1 ) {
		p_pva->intr_status = PV_PVA_INTR_PB1;		wbflush();
	    }
	    p_pva->intr_mask_clear = PV_PVA_INTR_PB1;		wbflush();
	    intr_status = p_pva->intr_status;			wbflush();
	    PV_LOWER_SPL(s);

#else	/* GFPA_IS_SYNCHRONOUS */

	    csr2 = csr = p_pva->csr;
	    csr.pb_reset = 1;
	    csr.gpo = 1;
	    csr2.pb_reset = 0;
	    csr2.gpo = 0;

	    s = PV_RAISE_SPL();
	    try = 0;
	    do {
		p_pva->csr = csr;
		wbflush();
		p_pva->counter = MAGIC_PVA_COUNTDOWN_VALUE;
		wbflush();
		p_pva->csr = csr2;
		wbflush();
		*p_gcp_cc = pvp->gcp_clock_control;
		wbflush();
		counter = p_pva->counter;
	    } while ( counter == 0 && try++ < MAGIC_GCP_CLOCK_TRY_COUNT );
	    DELAY( 50000 );
	    intr_status = p_pva->intr_status;
	    if ( intr_status & PV_PVA_INTR_PB1 ) {
		p_pva->intr_status = PV_PVA_INTR_PB1;		wbflush();
	    }
	    p_pva->intr_mask_clear = PV_PVA_INTR_PB1;		wbflush();
	    intr_status = p_pva->intr_status;			wbflush();
	    PV_LOWER_SPL(s);
	    if ( counter == 0 ) {
		/*
		 * Failed to reset clock control register
		 */
		PRINTFBUF( ("pvioctl: couldn't reset clock control register\n" ) );
		status = EIO;
		break;
	    }

#endif	/* GFPA_IS_SYNCHRONOUS */

	}
	break;

	case PV_IOC_SYNC_ON_COUNT_DOWN:
	{
	    int
		s;

	    if (!( pvp->gcp_count_down_event ) ) {
		s = PV_RAISE_SPL();
		if (!( pvp->gcp_count_down_event ) ) {
		    ISLEEP_ON_GFPA( pvp, (PZERO|PCATCH), "pvcntdwn", s );
		}
		PV_LOWER_SPL(s);
	    }
	    pvp->gcp_count_down_event = 0;
	}
	break;

	case PV_IOC_START_GFPA:
	{
	    int
		s;

	    pv_register_t
		*p_pc;

	    pvInfo
		*pinf = (pvInfo *) pvp->p_info_area;

	    s = PV_RAISE_SPL();

	    p_pc = (pv_register_t *) ( (vm_offset_t) PV_GCP_BASE( pvp )
				     + PV_GCP_PC_OFFSET );
	    pvp->gfpa_is_running = 1;
	    pinf->gfpa_halts_on_countdown =
		pvi->info.start_gfpa.gfpa_halts_on_countdown;
	    wbflush();
	    *p_pc = pvi->info.start_gfpa.gfpa_pc;
	    wbflush();

	    PV_LOWER_SPL(s);
	}
	break;

	case PV_IOC_HALT_GFPA:
	{
	    int
		s;

	    s = PV_RAISE_SPL();
	    halt_gfpa( pvp );
	    PV_LOWER_SPL(s);

	}
	break;

#ifdef BUILD_PERFORMANCE_IOCTLS

	case PV_IOC_PERF1:
	{
	    vm_offset_t
		user_addr = pvi->info.perf1.addr;

	    unsigned long
		user_size = pvi->info.perf1.size;

	    pv_graphic_tlb_t
		*p_pv_tlb;

	    int
		s;

	    /*
	     * Run some tests on the data
	     */
	    if ( user_addr == (vm_offset_t) NULL ) break;

	    /*
	     * Lock it down and force the STLB entries.
	     */
	    if ( status = vm_lock( pvp, user_addr, user_size,
				   (VM_PROT_READ|VM_PROT_WRITE), 0 ) ) {
		return (status);
	    }

	    if ( status = vm_unlock( pvp, user_addr, user_size, 0 ) ) {
		return (status);
	    }

	    /*
	     * Done
	     */
	}
	break;

	case PV_IOC_PERF3:
	break;

	case PV_IOC_PERF2:
	{
	    vm_offset_t
		user_addr = pvi->info.perf2.addr;

	    unsigned long
		user_size = pvi->info.perf2.size;

	    pv_graphic_tlb_t
		*p_pv_tlb;

	    int
		s;

	    /*
	     * Run some tests on the data
	     */
	    if ( user_addr == (vm_offset_t) NULL ) break;

	    /*
	     * Lock it down and force the STLB entries.
	     */
	    if ( status = vm_lock( pvp, user_addr, user_size,
				   (VM_PROT_READ|VM_PROT_WRITE), 0 ) ) {
		return (status);
	    }

	    if ( status = vm_load_stlb( pvp, user_addr, user_size, 0, 0, 1, 1 ) ) {
		vm_unlock( pvp, user_addr, user_size, 0 );
	        return (status);
	    }

	    if ( status = vm_unload_stlb( pvp, user_addr, user_size, 0, 0 ) ) {
		vm_unlock( pvp, user_addr, user_size, 0 );
	        return (status);
	    }

	    if ( status = vm_unlock( pvp, user_addr, user_size, 0 ) ) {
		return (status);
	    }

	    /*
	     * Done
	     */
	}
	break;

	case PV_IOC_PERF4:
	{
	    vm_offset_t
		user_addr = pvi->info.perf2.addr;

	    unsigned long
		user_size = pvi->info.perf2.size;

	    pv_graphic_tlb_t
		*p_pv_tlb;

	    int
		s;

	    /*
	     * Run some tests on the data
	     */
	    if ( user_addr == (vm_offset_t) NULL ) break;

	    /*
	     * Lock it down and force the STLB entries.
	     */

	    if ( status = vm_load_stlb( pvp, user_addr, user_size, 0, 0, 0, 1 ) ) {
		vm_unlock( pvp, user_addr, user_size, 0 );
	        return (status);
	    }

	    if ( status = vm_unload_stlb( pvp, user_addr, user_size, 0, 0 ) ) {
	        return (status);
	    }

	    /*
	     * Done
	     */
	}
	break;

	case PV_IOC_PERF5:
	case PV_IOC_PERF6:
	case PV_IOC_PERF7:
	{
	    pv_pva_register_t
		*p_pva;

	    pv_render_implement_t
		*p_imp;

	    int
		do_sleep,
		do_intr,
		s;

	    do_sleep = ( pvi->cmd == PV_IOC_PERF7 );
	    do_intr = ( do_sleep || ( pvi->cmd == PV_IOC_PERF6 ) );
	    s = PV_RAISE_SPL();
	    if ( do_intr ) {
		PV_INT_CLEAR( pvp, PV_INT_VM );
		PV_INT_ENABLE( pvp, PV_INT_VM );
	    }
	    lcl_blitc( (caddr_t) pvp, &pvp->screen, 1, 1, '$' );
	    if ( do_sleep ) {
		ISLEEP_ON_PV_DMA( pvp, (PZERO|PCATCH), "pvperf", s );
	    }
	    PV_LOWER_SPL(s);
	}
	break;

	case PV_IOC_PERF8:
	{
	    pv_pva_register_t
		*p_pva;

	    pv_render_implement_t
		*p_imp;

	    int
		do_sig,
		s;

	    do_sig = 1;
	    s = PV_RAISE_SPL();
	    PV_INT_CLEAR( pvp, PV_INT_VM );
	    PV_INT_ENABLE( pvp, PV_INT_VM );
	    if ( do_sig ) {
		perf_signal_on_vm = 1;
	    }
	    lcl_blitc( (caddr_t) pvp, &pvp->screen, 1, 1, '$' );
	    PV_LOWER_SPL(s);
	}
	break;

#endif	/* BUILD_PERFORMANCE_IOCTLS */

      default:
	status = EINVAL;
	break;

    }

    /*
     * Done with switch
     */
    return (status);
}


/*
 * ioctl_pmap_pageoutin
 *
 * Perform a pixelmap pageout/in operation for an ioctl
 */
static int
ioctl_pageoutin( pvp, pvi )
     pv_info *pvp;
     pv_ioc *pvi;
{
    int
	status = 0,
	count,
	do_once,
	i,
	s;

    long
	size;

    pv_subpixel_pair_t
	min_xy,
	max_xy;

    pv_register_t
	low_addr,
	high_addr;

    pv_graphic_tlb_t
	*p_rend_tlb,
	tlb_entry;

    pv_render_dma_t
	*p_rend_dma;

    unsigned long
	tmp_addr;

    pv_pva_register_t
	*p_pva;

    /*
     * Copy arguments in
     */
    pvp->poi_count = count = pvi->info.pageoutin.count;
    pvp->poi_clear_page_int = pvi->info.pageoutin.clear_page_int;

    if ( pvp->poi_count > 0 ) {
	size = count * sizeof( pvp->poi_tlb_index[0] );
	if ( status = copyin(	(caddr_t) pvi->info.pageoutin.p_tlb_index,
				(caddr_t) pvp->poi_tlb_index,
				size ) ) return (status);

	size = count * sizeof( pvp->poi_out_tlb_entries[0] );
	if ( status = copyin(	(caddr_t) pvi->info.pageoutin.p_out_tlb_entries,
				(caddr_t) pvp->poi_out_tlb_entries,
				size ) ) return (status);

	size = count * sizeof( pvp->poi_out_page[0] );
	if ( status = copyin(	(caddr_t) pvi->info.pageoutin.p_out_page,
				(caddr_t) pvp->poi_out_page,
				size ) ) return (status);

	size = count * sizeof( pvp->poi_out_format[0] );
	if ( status = copyin(	(caddr_t) pvi->info.pageoutin.p_out_format,
				(caddr_t) pvp->poi_out_format,
				size ) ) return (status);

	size = count * sizeof( pvp->poi_out_xy_min[0] );
	if ( status = copyin(	(caddr_t) pvi->info.pageoutin.p_out_xy_min,
				(caddr_t) pvp->poi_out_xy_min,
				size ) ) return (status);

	size = count * sizeof( pvp->poi_out_xy_max[0] );
	if ( status = copyin(	(caddr_t) pvi->info.pageoutin.p_out_xy_max,
				(caddr_t) pvp->poi_out_xy_max,
				size ) ) return (status);

	size = count * sizeof( pvp->poi_out_offset[0] );
	if ( status = copyin(	(caddr_t) pvi->info.pageoutin.p_out_offset,
				(caddr_t) pvp->poi_out_offset,
				size ) ) return (status);

	size = count * sizeof( pvp->poi_out_pixelmapid[0] );
	if ( status = copyin(	(caddr_t) pvi->info.pageoutin.p_out_pixelmapid,
				(caddr_t) pvp->poi_out_pixelmapid,
				size ) ) return (status);

	size = count * sizeof( pvp->poi_in_tlb_entries[0] );
	if ( status = copyin(	(caddr_t) pvi->info.pageoutin.p_in_tlb_entries,
				(caddr_t) pvp->poi_in_tlb_entries,
				size ) ) return (status);

	size = count * sizeof( pvp->poi_in_page[0] );
	if ( status = copyin(	(caddr_t) pvi->info.pageoutin.p_in_page,
				(caddr_t) pvp->poi_in_page,
				size ) ) return (status);

	size = count * sizeof( pvp->poi_in_format[0] );
	if ( status = copyin(	(caddr_t) pvi->info.pageoutin.p_in_format,
				(caddr_t) pvp->poi_in_format,
				size ) ) return (status);

	size = count * sizeof( pvp->poi_in_xy_min[0] );
	if ( status = copyin(	(caddr_t) pvi->info.pageoutin.p_in_xy_min,
				(caddr_t) pvp->poi_in_xy_min,
				size ) ) return (status);

	size = count * sizeof( pvp->poi_in_xy_max[0] );
	if ( status = copyin(	(caddr_t) pvi->info.pageoutin.p_in_xy_max,
				(caddr_t) pvp->poi_in_xy_max,
				size ) ) return (status);

	size = count * sizeof( pvp->poi_in_offset[0] );
	if ( status = copyin(	(caddr_t) pvi->info.pageoutin.p_in_offset,
				(caddr_t) pvp->poi_in_offset,
				size ) ) return (status);

	size = count * sizeof( pvp->poi_in_pixelmapid[0] );
	if ( status = copyin(	(caddr_t) pvi->info.pageoutin.p_in_pixelmapid,
				(caddr_t) pvp->poi_in_pixelmapid,
				size ) ) return (status);
    }

    p_rend_dma = PV_REND_DMA_BASE( pvp );
    p_rend_tlb = PV_REND_TLB_BASE( pvp );
    p_pva = PV_PVA_BASE( pvp );

    /*
     * Invalidate PVA tlb entries to prevent re-use of stale data
     */
    invalidate_pva_tlb( pvp, INVALIDATE_ALL );

    /*
     * Now perform all pageouts
     */
    for ( i = 0; i < pvp->poi_count; i++ ) {

	if ( pvp->poi_out_page[i] == (vm_offset_t) NULL ) continue;

	if ( pvp->poi_tlb_index[i] < 0 ||
	     pvp->poi_tlb_index[i] >= pvp->pixelmap_count ) {
	    return (EINVAL);
	}
	tlb_entry = *(pv_graphic_tlb_t *) &pvp->poi_out_tlb_entries[i];
	if ( tlb_entry.pixelmap_id != 0 ) {
	    p_rend_tlb[pvp->poi_tlb_index[i]] = tlb_entry;
	    wbflush();
	}

	min_xy = pvp->poi_out_xy_min[i];
	max_xy = pvp->poi_out_xy_max[i];

	size = ( PV_SUBPIX_GET_WHOLE( max_xy.x )
	       - PV_SUBPIX_GET_WHOLE( min_xy.x )
	       + pvp->poi_out_offset[i] )
	     * ( PV_SUBPIX_GET_WHOLE( max_xy.y )
	       - PV_SUBPIX_GET_WHOLE( min_xy.y ) );
	size <<= 2;

	/*
	 * Lockdown VM and load STLB.
	 */
	if ( status = vm_lock( pvp, pvp->poi_out_page[i], size,
			       (VM_PROT_READ|VM_PROT_WRITE), 1 ) ) {
	    return (status);
	}
	if ( status = vm_load_stlb( pvp, pvp->poi_out_page[i], size, 1, 1, 1, 1 ) ) {
	    vm_unlock( pvp, pvp->poi_out_page[i], size, 1 );
	    return (status);
	}

	/*
	 * Initiate DMA
	 */
	PV_INT_CLEAR( pvp, PV_INT_VM );
	PV_INT_ENABLE( pvp, PV_INT_VM );
	tmp_addr = ( (unsigned long) pvp->poi_out_page[i]
		   + PVB_VHOST_OFFSET_LOW
		   + ( (unsigned long) PVB_VHOST_OFFSET_HIGH << 32 ) )
		 >> SHIFT_FOR_VHOST_DMA;
	low_addr = (pv_register_t) ((pv_bus_addr2_t *) &tmp_addr)->low;
	high_addr = (pv_register_t) ((pv_bus_addr2_t *) &tmp_addr)->high;
	s = PV_RAISE_SPL();
	p_rend_dma->pixelmap_id	= pvp->poi_out_pixelmapid[i];
	p_rend_dma->xy_min	= min_xy;
	p_rend_dma->xy_max	= max_xy;
	p_rend_dma->addr_low	= low_addr;
	p_rend_dma->addr_high	= high_addr;
	p_rend_dma->offset	= pvp->poi_out_offset[i];
	p_rend_dma->format	= pvp->poi_out_format[i];
	wbflush();

	p_rend_dma->read_write	= PV_RENDER_DMA_RW_WRITE;
	wbflush();

	do {
	    SLEEP_ON_PV_DMA( pvp );
	} while ( p_rend_dma->status );

	PV_LOWER_SPL(s);

	/*
	 * Unlock VM
	 */
	if ( status = vm_unload_stlb( pvp, pvp->poi_out_page[i], size, 1, 1 ) ) {
	    vm_unlock( pvp, pvp->poi_out_page[i], size, 1 );
	    return (status);
	}
	if ( status = vm_unlock( pvp, pvp->poi_out_page[i], size, 1 ) ) {
	    return (status);
	}
    } /* done with pageouts */

    /*
     * Now perform all pageins.  Note that this always puts
     * a tlb_entry in (unless the id is zero).
     */
    for ( i = 0; i < pvp->poi_count; i++ ) {
	if ( pvp->poi_tlb_index[i] < 0 ||
	     pvp->poi_tlb_index[i] >= pvp->pixelmap_count ) {
	    return (EINVAL);
	}
	tlb_entry = *(pv_graphic_tlb_t *) &pvp->poi_in_tlb_entries[i];
	if ( tlb_entry.pixelmap_id != 0 ) {
	    p_rend_tlb[pvp->poi_tlb_index[i]] = tlb_entry;
	    wbflush();
	}

	if ( pvp->poi_in_page[i] == (vm_offset_t) NULL ) continue;

	min_xy = pvp->poi_in_xy_min[i];
	max_xy = pvp->poi_in_xy_max[i];

	size = ( PV_SUBPIX_GET_WHOLE( max_xy.x )
	       - PV_SUBPIX_GET_WHOLE( min_xy.x )
	       + pvp->poi_in_offset[i] )
	     * ( PV_SUBPIX_GET_WHOLE( max_xy.y )
	       - PV_SUBPIX_GET_WHOLE( min_xy.y ) );
	size <<= 2;

	/*
	 * Lockdown of VM and load STLBs.
	 */
	if ( status = vm_lock( pvp, pvp->poi_in_page[i], size,
			       (VM_PROT_READ|VM_PROT_WRITE), 1 ) ) {
	    return (status);
	}
	if ( status = vm_load_stlb( pvp, pvp->poi_in_page[i], size, 1, 1, 1, 0 ) ) {
	    vm_unlock( pvp, pvp->poi_in_page[i], size, 1 );
	    return (status);
	}

	/*
	 * Initiate DMA
	 */
	PV_INT_CLEAR( pvp, PV_INT_VM );
	PV_INT_ENABLE( pvp, PV_INT_VM );
	tmp_addr = ( (unsigned long) pvp->poi_in_page[i]
		   + PVB_VHOST_OFFSET_LOW
		   + ( (unsigned long) PVB_VHOST_OFFSET_HIGH << 32 ) )
		 >> SHIFT_FOR_VHOST_DMA;
	low_addr = (pv_register_t) ((pv_bus_addr2_t *) &tmp_addr)->low;
	high_addr = (pv_register_t) ((pv_bus_addr2_t *) &tmp_addr)->high;

	s = PV_RAISE_SPL();
	p_rend_dma->pixelmap_id	= pvp->poi_in_pixelmapid[i];
	p_rend_dma->xy_min	= min_xy;
	p_rend_dma->xy_max	= max_xy;
	p_rend_dma->addr_low	= low_addr;
	p_rend_dma->addr_high	= high_addr;
	p_rend_dma->offset	= pvp->poi_in_offset[i];
	p_rend_dma->format	= pvp->poi_in_format[i];
	wbflush();

	p_rend_dma->read_write	= PV_RENDER_DMA_RW_READ;
	wbflush();

	do {
	    SLEEP_ON_PV_DMA( pvp );
	} while ( p_rend_dma->status );

	PV_LOWER_SPL(s);

	/*
	 * Unlock VM
	 */
	if ( status = vm_unload_stlb( pvp, pvp->poi_in_page[i], size, 1, 1 ) ) {
	    vm_unlock( pvp, pvp->poi_in_page[i], size, 1 );
	    return (status);
	}
	if ( status = vm_unlock( pvp, pvp->poi_in_page[i], size, 1 ) ) {
	    return (status);
	}

    } /* replaced tlbs and finished pageins */

    /*
     * Invalidate PVA tlb entries to prevent re-use of stale data
     */
    invalidate_pva_tlb( pvp, INVALIDATE_ALL );

    /*
     * Finally, if requested, we clear the page_int bit
     */
    if ( pvp->poi_clear_page_int ) {
	s = PV_RAISE_SPL();
	p_pva->intr_mask_set = PV_PVA_INTR_PB0;
	wbflush();
	PV_INT_ENABLE( pvp, PV_INT_PAGE )
	PV_INT_CLEAR( pvp, PV_INT_PAGE );
	p_pva->intr_mask_clear = PV_PVA_INTR_PB0;
	wbflush();
	PV_LOWER_SPL(s);
    }

    /*
     * Done with PV_IOC_PMAP_PAGEOUTIN
     */
    return (status);
}

/*
 * vm_touched_page
 *
 *
 * Server claims it has touched a faulted page.  Revalidate and attempt to
 * complete the fault service.  Now, it *is* possible for the system to 
 * fault the page out again.  We'll assume that this is an error condition
 * at the moment.
 */
static int
vm_touched_page( pvp, p_proc, va )
     pv_info *pvp;
     struct proc *p_proc;
     vm_offset_t va;
{

    pmap_t
	server_map;

    vm_offset_t
	miss_va,
	miss_phys,
	miss_vpn;

    pv_dma_miss_t
	miss;

    unsigned int
	miss_read;

    int
	tlb_index,
	pva_tlb_index,
	i,
	s,
	perform_check = 0,
	status = 0;

    pv_pva_tlb_entry_t
	tlb_entry;

    pv_pva_register_t
	*p_pva;

    pt_entry_t
	*pte;

    pv_register_t
	*p_pc_reg;


    p_pva = PV_PVA_BASE( pvp );
    if ( pvp->state & PV_INFO_STATE_GCP_VALID ) {
	perform_check = 1;
    }
    miss = p_pva->dma_miss;
    miss_vpn = (vm_offset_t) miss.vpn;
    miss_read = miss.rd;
    miss_va = (vm_offset_t) PV_VPN_TO_VA( miss_vpn );
    s = splvm();

#ifdef	DO_COPYIN_ON_TOUCHED_PAGE
    {
	char
	    data;

	if ( ( status = copyin( miss_va, &data, 1 ) ) ) {
	    PV_DEBUG( PV_TERSE,
		PRINTFPOLL( ( "pv_touch: copyin failed.  status %d\n",
				status ) ); );
	    goto fail;
	}
	if (!( miss_read ) && 
	     ( status = copyout( &data, miss_va, 1 ) ) ) {
		PV_DEBUG( PV_TERSE,
		    PRINTFPOLL( ( "pv_touch: copyout failed.  status %d\n",
				status ) ); );
	    goto fail;
	}
    }
#endif	/* DCOTP */

    if ( miss_va != va ) {
	/*
	 * Somehow, user got confused
	 */
	PV_DEBUG( PV_TERSE,
	    PRINTFPOLL( ( "pv_touch: user confused, was 0x%lx, is 0x%lx\n",
			miss_va, va ) ); );
	status = EINVAL;
	goto fail;
    }

    server_map = p_proc->task->map->vm_pmap;
    pte = pmap_pte( server_map, miss_va );
    if ( pmap_extract( server_map, miss_va ) ) {
#ifndef	DO_COPYIN_ON_TOUCHED_PAGE
	if ( !miss_read ) {
	    /*
	     * Mark as modified
	     */
	    miss_phys = alpha_ptob( pte->pg_pfn );
	    pmap_set_modify( miss_phys );
	}
#endif /* DCOTP */
	tlb_entry.ppn	= pte->pg_pfn;
	tlb_entry.pid	= PV_PVA_PID_SERVER;
	tlb_entry.we	= !miss_read;
	tlb_entry.vpn	= miss_vpn;
	tlb_index	= miss_vpn &
	  		( pvp->page_table_1_size - 1 );
	pvp->p_page_table_1[tlb_index] = tlb_entry;
#ifdef PRELOAD_PVA_ON_MISS
	pva_tlb_index	= ++pva_tlb_index
	  		% PV_PVA_TLB_ENTRIES;
	p_pva->tlb[pva_tlb_index] = tlb_entry;
#endif
	wbflush();

	*(pv_register_t *) &p_pva->dma_miss = (pv_register_t) 0;
	wbflush();
    }
    else {
	/*
	 * Still no translation.  Force one.
	 */
	PV_DEBUG( PV_TERSE,
	    PRINTFPOLL( ( "pv_touch: no translation.\n" ) ); );
	status = EINVAL;
	goto fail;
    }

done:
    /*
     * Now run a preliminary check on ack
     */
    if ( perform_check ) {
	if ( pvp->gfpa_pc_valid ) {
	    p_pc_reg = (pv_register_t *) ( (vm_offset_t) PV_GCP_BASE( pvp )
					 + PV_GCP_PC_OFFSET );
	    pvp->gfpa_is_running = 1;
	    *p_pc_reg = pvp->gfpa_pc;
	    pvp->gfpa_pc_valid = 0;
	    wbflush();
	}
	else {
	    PRINTFBUF( ("pv: Invalid GFPA pc value.  GFPA not restarted\n" ) );
	}
    }

    /*
     * Done
     */
    splx(s);
    return (status);

    /*
     * Failure condition of some sort.  Cons up a tlb entry for the
     * dead page to satisfy the crapped out va translation.  Then exit
     * through the normal exit return.  Note that we are running at
     * splvm.
     */
fail:

    PRINTFBUF( ( "pv: Faulting GCP VDMA not resolved correctly.\n" ) );
    tlb_entry.ppn	= pvp->ppn_dead_area;
    tlb_entry.pid	= PV_PVA_PID_SERVER;
    tlb_entry.we	= !miss_read;
    tlb_entry.vpn	= miss_vpn;
    tlb_index		= miss_vpn &
	  		( pvp->page_table_1_size - 1 );
    pvp->p_page_table_1[tlb_index] = tlb_entry;
#ifdef	PRELOAD_PVA_ON_MISS
    pva_tlb_index	= ++pva_tlb_index
	  		% PV_PVA_TLB_ENTRIES;
    p_pva->tlb[pva_tlb_index] = tlb_entry;
#endif
    wbflush();

    goto done;
}


/*
 * vm_lock
 *
 * Locks a large piece of user VM into memory.
 */
static int
vm_lock( pvp, addr, size, prot, save_old )
     pv_info *pvp;
     vm_offset_t addr;
     unsigned long size;
     int prot;
     int save_old;
{
    int
	status = 0;

    vm_offset_t
	start = trunc_page( addr ),
	end = round_page( addr + size );

    unsigned long
	table_size = pvp->page_table_0_size,
	full_size = end - start;

    if ( ( full_size / NBPG )
	 >= ( save_old ? PV_INFO_SAVED_PV_TLB_MAX : table_size ) ) {
	return (EINVAL);
    }

    if ( status = pv_vslock( start, full_size, prot ) ) {
	return (status);
    }

    /*
     * Done
     */
    return (status);
}

/*
 * vm_load_stlb
 *
 * Loads stlb entries for a piece of assumed-locked down memory.  If asked,
 * it will save the entries it replaces (for pixelmap paging) and will
 * do master/slave locking on stlb access.
 */
static int
vm_load_stlb( pvp, addr, size, save_old, master, hardlock, modify )
     pv_info *pvp;
     vm_offset_t addr;
     unsigned long size;
     int save_old;
     int master;
     int hardlock;
     int modify;
{
    int
	i,
	s,
	tlb_index,
	status = 0,
	save_index = 0;

    pt_entry_t
	*pte;

    vm_offset_t
	start = trunc_page( addr ),
	end = round_page( addr + size ),
	current,
	phys_addr;

    unsigned long
	table_size = pvp->page_table_0_size,
	full_size = end - start;

    pv_pva_tlb_entry_t
	*p_tlb,
	tlb_entry;

    char
	data;

    int
	did_retry,
	dummy;

    pmap_t
	map = current_task()->map->vm_pmap;

    unsigned int
	*p_kmem;

    if ( ( full_size / NBPG )
	 >= ( save_old ? PV_INFO_SAVED_PV_TLB_MAX : table_size - 1 ) ) {
	return (EINVAL);
    }

    p_tlb = pvp->p_page_table_0;

    /*
     * Make certain that nobody tries to do an image operation while
     * we're in the middle of a pixelmap paging operation.  The opposite
     * must be permitted.  This is also a monoprocessor assumption.
     */
    if ( master ) {
	/*
	 * Pixelmap paging is considered the master
	 */
	pvp->pmap_stlb_lock = 1;
    }
    else {
	/*
	 * Image operations are subservient
	 */
	pvp->image_stlb_lock = 1;
	while ( pvp->pmap_stlb_lock ) {
	    sleep( &pvp->pmap_stlb_lock, TTIPRI );
	}
    }

    s = splvm();
    for ( current = start; current < end; current += NBPG ) {
	did_retry = 0;
retry:
	pte = pmap_pte( map, current );
	if ( pte == (pt_entry_t *) NULL ||
	     pmap_extract( map, current ) == (vm_offset_t) NULL ) {
	    if ( !hardlock && !did_retry ) {
		copyin( (caddr_t) current, &dummy, 1 );
		did_retry = 1;
		goto retry;
	    }
	    else {
		splx(s);
		return (EINVAL);
	    }
	}

	/*
	 * Mark as modified if this will be a write DMA.
	 */
	if ( modify ) {
	    phys_addr		= (vm_offset_t) alpha_ptob( pte->pg_pfn );
	    pmap_set_modify( phys_addr );
	}

    PV_DEBUG( PV_TALK,
	PRINTFPOLL( ( "load_tlb: VA: 0x%lx, pfn: 0x%x, modify: %d\n",
			current, pte->pg_pfn, modify )); );
	/*
	 * Load that TLB entry
	 */
	tlb_entry.ppn		= pte->pg_pfn;
	tlb_entry.pid		= PV_PVA_PID_PAGER;
	tlb_entry.we		= modify;
	tlb_entry.mru		= 0;
	tlb_entry.vpn		= PV_VA_TO_VPN( current );
	tlb_index		= tlb_entry.vpn & ( table_size - 1 );
	if ( save_old ) {
	    pvp->saved_tlb[save_index++] = p_tlb[tlb_index];
	}
	p_tlb[tlb_index]	= tlb_entry;

    }

    /*
     * Load one more STLB that translates to the dead page.  This is
     * both defensive programming and handles the prefetch mechanism.
     */
    tlb_entry.ppn		= pvp->ppn_dead_area;
    tlb_entry.pid		= PV_PVA_PID_PAGER;
    tlb_entry.we		= 1;
    tlb_entry.mru		= 0;
    tlb_entry.vpn		= PV_VA_TO_VPN( current );
    tlb_index			= tlb_entry.vpn & ( table_size - 1 );
    if ( save_old ) {
	pvp->saved_tlb[save_index++] = p_tlb[tlb_index];
    }
    p_tlb[tlb_index]		= tlb_entry;

    splx(s);

    /*
     * Done
     */
    return (status);
}

/*
 * vm_unlock
 *
 * unlock the memory.
 */
static int
vm_unlock( pvp, addr, size, restore_old )
     pv_info *pvp;
     vm_offset_t addr;
     unsigned long size;
     int restore_old;
{
    int
	i,
	status = 0;

    vm_offset_t
	start = trunc_page( addr ),
	end = round_page( addr + size );

    unsigned long
	table_size = pvp->page_table_0_size,
	full_size = end - start;

    if ( ( full_size / NBPG )
	 >= ( restore_old ? PV_INFO_SAVED_PV_TLB_MAX : table_size ) ) {
	return (EINVAL);
    }

    /*
     * Unlock the memory
     */
    if ( status = pv_vsunlock( start, full_size, 1 ) ) {
	return (status);
    }

    /*
     * Done
     */
    return (status);
}

/*
 * vm_unload_stlb
 *
 * Remove or restore entries to an stlb and complete the lockout procedure.
 */
static int
vm_unload_stlb( pvp, addr, size, restore_old, master )
     pv_info *pvp;
     vm_offset_t addr;
     unsigned long size;
     int restore_old;
     int master;
{
    int
	i,
	tlb_index,
	status = 0,
	restore_index = 0;

    vm_offset_t
	start = trunc_page( addr ),
	end = round_page( addr + size + NBPG ),
	current;

    unsigned long
	table_size = pvp->page_table_0_size,
	full_size = end - start;

    unsigned int
	first_vpn,
	last_vpn;

    pv_pva_tlb_entry_t
	*p_tlb,
	tlb_entry;

    pv_pva_register_t
	*p_pva;

    if ( ( full_size / NBPG )
	 >= ( restore_old ? PV_INFO_SAVED_PV_TLB_MAX : table_size - 1 ) ) {
	return (EINVAL);
    }

    p_tlb = pvp->p_page_table_0;
    p_pva = PV_PVA_BASE( pvp );

    for ( current = start; current < end; current += NBPG ) {
	tlb_index = ( PV_VA_TO_VPN( current ) ) & ( table_size - 1 );
	if ( restore_old ) {
	    p_tlb[tlb_index] = pvp->saved_tlb[restore_index++];
	}
	else {
	    INVALID_STLB_ENTRY( pvp, tlb_index, (&tlb_entry) );
	    p_tlb[tlb_index] = tlb_entry;
	}
    }

    /*
     * Invalidate relevant entries
     */
    first_vpn = PV_VA_TO_VPN( start );
    last_vpn = PV_VA_TO_VPN( end );
    for ( tlb_index = 0; tlb_index < PV_PVA_TLB_ENTRIES; tlb_index++ ) {
	tlb_entry = p_pva->tlb[tlb_index];
	if ( tlb_entry.vpn >= first_vpn &&
	     tlb_entry.vpn < last_vpn &&
	     tlb_entry.pid == PV_PVA_PID_PAGER ) {
	    INVALID_PVA_TLB_ENTRY( pvp, tlb_index, (&tlb_entry) );
	    p_pva->tlb[tlb_index] = tlb_entry;
	}
    }

    /*
     * Complete the lock processing started in vm_load_stlb.
     */
    if ( master ) {
	/*
	 * Pixelmap paging is considered the master
	 */
	pvp->pmap_stlb_lock = 0;
	if ( pvp->image_stlb_lock ) {
	    wakeup( &pvp->pmap_stlb_lock );
	}
    }
    else {
	/*
	 * Image operations are subservient
	 */
	pvp->image_stlb_lock = 0;
    }

    /*
     * Done
     */
    return (status);
}

/*
 * Map interesting structures into user space.
 *
 * NOTE:
 *
 * Because this is going to be mapped by one process and we have only
 * one global area for each unit with which to communicate this information
 * to processes, *all* mappings must occur at the same addresses across
 * processes.
 *
 */
int
pv_map_screen(pvp, dp, sp, mp)
    pv_info *pvp;			/* closure */
    ws_depth_descriptor *dp;		/* depths */
    ws_screen_descriptor *sp;		/* screens */
    ws_map_control *mp;			/* control */
{
    caddr_t
	p_pvo,
	p_pvo_s,
	info_area,
	cmd_area,
	dead_area,
	clip_area;

    unsigned long
	tmp1, tmp2, tmp3, tmp4;

    int
	num;

    pvInfo
	*p_info;

    /*
     * depth and screen already checked in ws_device.c
     */
    if ( mp->map_unmap != MAP_SCREEN )
	return EINVAL;

    /*
     * Which entry in pv_softc[] is this?
     */
    num = (pvp - pv_softc);	/* return 0, 1, 2, ... */

    /*
     * First, map the dense mapping of the module.
     */
    tmp1 = USER_VA_BASE + USER_VA_DENSE_OFFSET + ( num * USER_VA_UNIT_OFFSET );
    if ( ( p_pvo = (caddr_t) ws_map_region(
				(caddr_t) pvp->p_pvo,
				tmp1,
				PV_MODULE_SIZE, 0600, (int *) NULL ) )
	== (caddr_t) NULL )
	return (ENOMEM);

    /*
     * Now sparse space
     */
    tmp1 = USER_VA_BASE + USER_VA_SPARSE_OFFSET + ( num * USER_VA_UNIT_OFFSET );
    if ( ( p_pvo_s = (caddr_t) ws_map_region(
				(caddr_t) pvp->p_pvo_s,
				tmp1,
				PV_SPARSE_MODULE_SIZE, 0600, (int *) NULL ) )
	 == (caddr_t) NULL )
	return (ENOMEM);

    /*
     * Now all the little dribbly bits
     */
    tmp1 = USER_VA_BASE + USER_VA_MISC_OFFSET + ( num * USER_VA_UNIT_OFFSET );

    tmp2 = ( pvp->cmd_area_size + ( NBPG - 1 ) ) & ~( NBPG - 1 );
    if ( ( cmd_area = (caddr_t) ws_map_region(
				(caddr_t) pvp->p_cmd_area,
				tmp1,
				tmp2, 0600, (int *) NULL ) )
	 == (caddr_t) NULL ) {
	return (ENOMEM);
    }

    tmp1 = (unsigned long) cmd_area + tmp2 + NBPG + NBPG;
    tmp2 = NBPG;
    if ( ( clip_area = (caddr_t) ws_map_region(
				(caddr_t) pvp->p_clip_area,
				tmp1,
				tmp2, 0600, (int *) NULL ) )
	 == (caddr_t) NULL ) {
	return (ENOMEM);
    }

    tmp1 = (unsigned long) clip_area + tmp2 + NBPG + NBPG;
    tmp2 = NBPG;
    if ( ( dead_area = (caddr_t) ws_map_region(
				(caddr_t) pvp->p_dead_area,
				tmp1,
				tmp2, 0600, (int *) NULL ) )
	 == (caddr_t) NULL ) {
	return (ENOMEM);
    }

    tmp1 = (unsigned long) dead_area + tmp2 + NBPG + NBPG;
    tmp2 = NBPG;
    p_info = pvp->pv = (pvInfo *) pvp->p_info_area;
    tmp3 = (unsigned long) pvp->pv;
    tmp4 = tmp3 & (NBPG - 1);
    tmp3 -= tmp4;
    if ( ( info_area = (caddr_t) ws_map_region(
				tmp3,
				tmp1,
				tmp2, 0600, (int *) NULL ) )
	 == (caddr_t) NULL ) {
	return (ENOMEM);
    }
    info_area += tmp4;
    dp->pixmap = (caddr_t) info_area;

    /*
     * Now, init the pvInfo struct so the user can find out
     * about all the interesting things...
     */
    p_info->p_pvo		= p_pvo;
    p_info->p_gcp		= p_pvo + PV_GCP_OFFSET;
    p_info->p_pva		= p_pvo + PV_PVA_OFFSET;
    p_info->p_pv		= p_pvo + PV_RENDER_OFFSET;
    p_info->p_sram		= p_pvo + PV_SRAM_OFFSET;
    p_info->p_pvo_s		= p_pvo_s;
    p_info->p_gcp_s		= p_pvo_s + PV_SPARSE_GCP_OFFSET;
    p_info->p_pva_s		= p_pvo_s + PV_SPARSE_PVA_OFFSET;
    p_info->p_pv_s		= p_pvo_s + PV_SPARSE_RENDER_OFFSET;
    p_info->p_sram_s		= p_pvo_s + PV_SPARSE_SRAM_OFFSET;
    p_info->vram_size		= pvp->vram_size;
    p_info->sram_size		= pvp->sram_size;
    p_info->pixelmap_size	= pvp->pixelmap_size;
    p_info->pixelmap_count	= pvp->pixelmap_count;
    p_info->update_size		= pvp->update_size;
    p_info->vis_pmap_size	= pvp->vis_pmap_size;
    p_info->monitor_rate	= pvp->monitor_rate;
    p_info->page_table_0_size	= pvp->page_table_0_size;
    p_info->page_table_1_size	= pvp->page_table_1_size;
    p_info->phys_page_table_0	= pvp->phys_page_table_0;
    p_info->phys_page_table_1	= pvp->phys_page_table_1;
    p_info->ppn_dead_area	= pvp->ppn_dead_area;
    p_info->phys_cmd_area	= pvp->phys_cmd_area;
    p_info->phys_dead_area	= pvp->phys_dead_area;
    p_info->phys_clip_area	= pvp->phys_clip_area;
    p_info->phys_info_area	= pvp->phys_info_area;
    p_info->cmd_area_size	= pvp->cmd_area_size;
    p_info->p_cmd_area		= cmd_area;
    p_info->p_clip_area		= clip_area;
    p_info->p_dead_area		= dead_area;

    /*
     * Done
     */

    return 0;
}


/*
 * Graphics device interrupt service routine.
 * Cursor and/or colormap loading at end of frame interrupt gets done
 * by hardware specific interrupt routine.
 */
void
pvintr(unit)
    int unit;
{
    register pv_info *pvp = (pv_info *) &pv_softc[unit];

    if (pvp->interrupt)
	(*pvp->interrupt)((struct controller *)pvinfo[unit], pvp);
}

/*
 * CONSOLE ROUTINES
 *
 * Very complicated.  Due to a bug, we must use PID 2 to do console
 * operations.  See pages 28-29 of the architecture manual and think
 * in terms of finding a function that maps from a linear update array
 * index to X and Y coords in a pageable pixelmap.
 *
 * Okay, I have a moment, so I'll describe what is actually happening.
 * Think of VRAM as a linear array of memory with the update array as
 * the atomic entity.  The visible pixelmap accesses VRAM in simple
 * row-major manner with the address given by:
 *
 * ADDvram = Y / 4 * 320 + X / 4
 *
 * for 4x4 udpate arrays and 1280x1024 visible pixelmap and
 *
 * ADDvram = Y / 2 * 320 + X / 4
 *
 * for 4x2 update arrays and 1280x1024 visible pixelmap where X and Y
 * are pixel coordinates.  The pageable pixelmaps do a dance around
 * memory.  For a 4x2, 4Mpixel system, 
 *
 * ADDvram = PFN<7:2>.Y<6:5>.X<6:5>.PFN<1:0>.Y<4:1>.X<4:2>
 *
 * and on a 4x4, 8Mpixel system,
 *
 * ADDvram = PFN<7:2>.Y<6:5>.X<7:6>.PFN<1:0>.Y<4:2>.X<5:2>
 *
 * Where X and Y are pixel coordinates and PFN is the index of the 
 * graphics TLB entry which resolves the pixelmap access.  From this,
 * we can create a table of address indices within a pixelmap.
 * For the 4x4 case:
 *
 *                                     X
 *
 *   Y    0......63    64......127    128......191    192......255
 *
 *   0    0......15   512......527   1024.....1039   1536.....1551
 *
 *   4   16......31   528......543   1040.....1055   1552.....1567
 *
 *   8   32......47   544......559   1056.....1071   1568.....1583
 *
 *  16   64......79   576......591   1088.....1103   1600.....1615
 *
 *  31  112.....127   624......639   1136.....1151   1648.....1663
 *
 *  32 2048....2061  2560.....2575   3072.....3087   3584.....3599
 *
 *  64 4096....4011  4608.....4623   5120.....5135   5632.....5647
 *
 * 127 6256....6271  6768.....6783   7280.....7295   7792.....7807
 *
 * 4x2:
 *
 *                                     X
 *
 *   Y    0......31    32.......63     64.......95    96......127
 *
 *   0    0.......7   512......519   1024.....1031   1536.....1543
 *
 *   2    8......15   520......527   1032.....1039   1544.....1551
 *
 *   4   16......23   528......535   1040.....1047   1552.....1559
 *
 *   8   32......39   544......551   1056.....1063   1568.....1575
 *
 *  16   64......71   576......583   1088.....1095   1600.....1607
 *
 *  31  120.....127   632......639   1144.....1151   1656.....1663
 *
 *  32 2048....2055  2560.....2567   3072.....3079   3584.....3591
 *
 *  64 4096....4003  4608.....4615   5120.....5127   5632.....5632
 *
 * 127 6264....6271  6776.....6783   7288.....7295   7800.....7807
 *
 *
 * By using four contiguous, four-aligned graphics TLB entries, we can
 * arrange for those pixelmaps to access a single, contiguous range
 * of VRAM.  However, to access sequentially within that range, we
 * must do a great deal of bit twiddling to produce the correct
 * X and Y coordinates.  Notice in the above that every naturally 
 * aligned group of 128 vram addresses forms a contigous group in
 * the pixelmap (64x32 in 4x4, 32x32 in 4x2).  So, define:
 *
 * 	ADDclus = ADDvram / 128
 *
 * where ADDvram is generated from the visible pixelmap address
 * calculation.  Then we can create functions that produce the
 * appropriate X and Y coordinates within a pixelmap to access an
 * arbitrary update array via the pageable pixelmap.  For 4x4:
 *
 *	X = 256 * ( ( ADDclus & 0x180 ) >> 7 )
 *	  + 64 * ( ( ADDclus & 0x600 ) >> 9 )
 *	  + ( Xvis & 0x3f )
 *
 *	Y = 128 * ( ADDclus / 8192 )
 *	  + 32 * ( ( ADDclus & 0x1800 ) >> 11 )
 *	  + 4 * ( ( ADDclus & 0x70 ) >> 4 )
 *
 * Where Xvis is the X coordinate from the ADDvram calculation.  The
 * 4x2 functions are:
 *
 *	X = 128 * ( ( ADDclus & 0x180 ) >> 7 )
 * 	  + 32 * ( ( ADDclus & 0x600 ) >> 9 )
 *	  + ( Xvis & 0x1f )
 *
 *	Y = 128 * ( ADDclus / 8192 )
 *	  + 32 * ( ( ADDclus & 0x1800 ) >> 11 )
 *	  + 2 * ( ( ADDclus & 0x78 ) >> 3 )
 *
 * In the console code, we use a 10x16 pixel glyph.  Observations:
 *
 *  .	By doing jump scrolling of one eighth of a screen at a time, I can
 *	ask for a DMA of 2K pixels at a time.  
 *
 *  .	The glyph width is such that it can cross a cluster boundary.
 *	Detect this when it occurs and do the left and right parts
 *	piecemeal.
 *
 *
 */

/*
 * pv_scroll_screen
 * 
 * Scroll 8 lines of screen.
 */
int
pv_scroll_screen(closure, screen)
    caddr_t closure;
    ws_screen_descriptor *screen;
{
    pv_info
	*pvp = (pv_info *) closure;

    int
	status = 7,
	x,
	y,
	x2,
	xlimit,
	xdelta,
	ylimit,
	ylimit2,
	ydelta,
	ydelta2,
	width,
	height,
	dw,
	stop_gfpa = 0;

    pv_register_t
	low_addr,
	high_addr;

    pv_render_dma_t
	*p_rend_dma;

    pv_subpixel_pair_t
	xy0,
	xy1,
	xy2,
	xy3;

    unsigned long
	tmp_addr;

    pv_pva_register_t
    	*p_pva;

    p_pva = PV_PVA_BASE( pvp );
    p_rend_dma = PV_REND_DMA_BASE( pvp );

#ifdef	NO_CONSOLE_IF_GFPA_RUNS
    {
	pv_register_t
	    *p_sr;

	p_sr = (pv_register_t *) ( (vm_offset_t) PV_GCP_BASE( pvp )
				 + PV_GCP_SR_OFFSET );

	if ( *p_sr != 0 ) {
	    return (1);
	}
    }
#endif	/* NCIGR */

    if ( pvp->state & PV_INFO_STATE_GCP_VALID ) {
	STORE_SOFTWARE_REG( pvp, PV_IOC_CNFG_GFPA_HANDSOFF, HANDSOFF_VALUE );
	wbflush();
	DELAY(20);
	stop_gfpa = 1;
	if ( spin_on_pv( pvp ) ) {
	    goto exit;
	}
    }

    tmp_addr = ( (unsigned long) pvp->phys_packet_area
	       + PVB_PHOST_OFFSET_LOW
	       + ( (unsigned long) PVB_PHOST_OFFSET_HIGH << 32 ) )
	     >> SHIFT_FOR_PHOST_DMA;
    low_addr = ((pv_bus_addr2_t *) &tmp_addr)->low;
    high_addr = ((pv_bus_addr2_t *) &tmp_addr)->high;

    width = pvp->screen.width;
    height = pvp->screen.height;

    if ( pvp->render_imp_rev.update_size == PV_RENDER_IMP_USZ_4X2 ) {
	xlimit = 512;
	xdelta = 64;
	ylimit = 2560;
	ylimit2 = 2240;
	ydelta = 32;
	ydelta2 = 320;
    }
    else {
	xlimit = 1024;
	xdelta = 64;
	ylimit = 1280;
	ylimit2 = 1120;
	ydelta = 32;
	ydelta2 = 160;
    }

    for ( y = 0; y < ylimit; y += ydelta ) {
	if ( y < ylimit2 ) {
	    /*
	     * Moving existing characters
	     */
	    for ( x = 0; x < xlimit; x += xdelta ) {
		xy0.x = PV_SUBPIX_MAKE( x, 0 );
		xy0.y = PV_SUBPIX_MAKE( y+ydelta2, 0 );
		xy1.x = PV_SUBPIX_MAKE( x+xdelta, 0 );
		xy1.y = PV_SUBPIX_MAKE( y+ydelta2+ydelta, 0 );

		xy2.x = PV_SUBPIX_MAKE( x, 0 );
		xy2.y = PV_SUBPIX_MAKE( y, 0 );
		xy3.x = PV_SUBPIX_MAKE( x+xdelta, 0 );
		xy3.y = PV_SUBPIX_MAKE( y+ydelta, 0 );
	    
		/*
		 * First, get the dcc bits in
		 */
		if ( spin_on_pv( pvp ) ) {
		    goto exit;
		}

		p_rend_dma->pixelmap_id	= ( 2 | PV_PIXELMAP_DCC_MASK );
		p_rend_dma->xy_min	= xy0;
		p_rend_dma->xy_max	= xy1;
		p_rend_dma->addr_low	= low_addr;
		p_rend_dma->addr_high	= high_addr;
		p_rend_dma->offset	= 0;
		p_rend_dma->format	= PV_RENDER_DMA_FORMAT_DCC;
		wbflush();

		p_rend_dma->read_write	= PV_RENDER_DMA_RW_WRITE;
		wbflush();

		/*
		 * Now move them out
		 */
		if ( spin_on_pv( pvp ) ) {
		    goto exit;
		}

		p_rend_dma->pixelmap_id	= ( 2 | PV_PIXELMAP_DCC_MASK );
		p_rend_dma->xy_min	= xy2;
		p_rend_dma->xy_max	= xy3;
		p_rend_dma->addr_low	= low_addr;
		p_rend_dma->addr_high	= high_addr;
		p_rend_dma->offset	= 0;
		p_rend_dma->format	= PV_RENDER_DMA_FORMAT_DCC;
		wbflush();

		p_rend_dma->read_write	= PV_RENDER_DMA_RW_READ;
		wbflush();

		/*
		 * Now the image data
		 */
		if ( spin_on_pv( pvp ) ) {
		    goto exit;
		}

		p_rend_dma->pixelmap_id	= 2;
		p_rend_dma->xy_min	= xy0;
		p_rend_dma->xy_max	= xy1;
		p_rend_dma->addr_low	= low_addr;
		p_rend_dma->addr_high	= high_addr;
		p_rend_dma->offset	= 0;
		p_rend_dma->format	= PV_RENDER_DMA_FORMAT_RGB;
		wbflush();

		p_rend_dma->read_write	= PV_RENDER_DMA_RW_WRITE;
		wbflush();

		/*
		 * Now move them out
		 */
		if ( spin_on_pv( pvp ) ) {
		    goto exit;
		}

		p_rend_dma->pixelmap_id	= 2;
		p_rend_dma->xy_min	= xy2;
		p_rend_dma->xy_max	= xy3;
		p_rend_dma->addr_low	= low_addr;
		p_rend_dma->addr_high	= high_addr;
		p_rend_dma->offset	= 0;
		p_rend_dma->format	= PV_RENDER_DMA_FORMAT_RGB;
		wbflush();

		p_rend_dma->read_write	= PV_RENDER_DMA_RW_READ;
		wbflush();
	    } /* end for */
	}
	else {
	    /*
	     * Clear out some text
	     */
	    if ( y == ylimit2 ) {
		set_packet_dcc( pvp );
	    }
	    for ( x = 0; x < xlimit; x += xdelta) {
		xy2.x = PV_SUBPIX_MAKE( x, 0 );
		xy2.y = PV_SUBPIX_MAKE( y, 0 );
		xy3.x = PV_SUBPIX_MAKE( x+xdelta, 0 );
		xy3.y = PV_SUBPIX_MAKE( y+ydelta, 0 );

		/*
		 * reset dcc bits
		 */
		if ( spin_on_pv( pvp ) ) {
		    goto exit;
		}

		p_rend_dma->pixelmap_id	= ( 2 | PV_PIXELMAP_DCC_MASK );
		p_rend_dma->xy_min	= xy2;
		p_rend_dma->xy_max	= xy3;
		p_rend_dma->addr_low	= low_addr;
		p_rend_dma->addr_high	= high_addr;
		p_rend_dma->offset	= 0;
		p_rend_dma->format	= PV_RENDER_DMA_FORMAT_DCC;
		wbflush();

		p_rend_dma->read_write	= PV_RENDER_DMA_RW_READ;
		wbflush();

		/*
		 * Clear data bits
		 */
		if ( spin_on_pv( pvp ) ) {
		    goto exit;
		}

		p_rend_dma->pixelmap_id	= 2;
		p_rend_dma->xy_min	= xy2;
		p_rend_dma->xy_max	= xy3;
		p_rend_dma->addr_low	= low_addr;
		p_rend_dma->addr_high	= high_addr;
		p_rend_dma->offset	= 0;
		p_rend_dma->format	= PV_RENDER_DMA_FORMAT_RGB;
		wbflush();

		p_rend_dma->read_write	= PV_RENDER_DMA_RW_READ;
		wbflush();
	    } /* end for x */
	} /* end if clear */
    } /* end for y */

exit:

    if ( stop_gfpa ) {
	STORE_SOFTWARE_REG( pvp, PV_IOC_CNFG_GFPA_HANDSOFF, 0 );
	wbflush();
    }

    /*
     * Done
     */
    return (status);

}


/*
 * pv_clear_screen
 *
 * Clear the bitmap.
 */
int
pv_clear_screen(closure, screen)
    caddr_t closure;
    ws_screen_descriptor *screen;
{
    pv_info
	*pvp = (pv_info *) closure;

    int
	x,
	y,
	x2,
	xlimit,
	xdelta,
	ylimit,
	ydelta,
	width,
	height,
	dw,
	stop_gfpa = 0;

    pv_register_t
	low_addr,
	high_addr;

    pv_render_dma_t
	*p_rend_dma;

    pv_subpixel_pair_t
	xy0,
	xy1;

    unsigned long
	tmp_addr;

    pv_pva_register_t
	*p_pva;

    p_pva = PV_PVA_BASE( pvp );
    p_rend_dma = PV_REND_DMA_BASE( pvp );

#ifdef	NO_CONSOLE_IF_GFPA_RUNS
    {
	pv_register_t
	    *p_sr;

	p_sr = (pv_register_t *) ( (vm_offset_t) PV_GCP_BASE( pvp )
				 + PV_GCP_SR_OFFSET );

	if ( *p_sr != 0 ) {
	    return (0);
	}
    }
#endif	/* NCIGR */

    if ( pvp->state & PV_INFO_STATE_GCP_VALID ) {
	STORE_SOFTWARE_REG( pvp, PV_IOC_CNFG_GFPA_HANDSOFF, HANDSOFF_VALUE );
	wbflush();
	DELAY(20);
	stop_gfpa = 1;
	if ( spin_on_pv( pvp ) ) {
	    goto exit;
	}
    }

    set_packet_dcc( pvp );

    tmp_addr = ( (unsigned long) pvp->phys_packet_area
	       + PVB_PHOST_OFFSET_LOW
	       + ( (unsigned long) PVB_PHOST_OFFSET_HIGH << 32 ) )
	     >> SHIFT_FOR_PHOST_DMA;
    low_addr = ((pv_bus_addr2_t *) &tmp_addr)->low;
    high_addr = ((pv_bus_addr2_t *) &tmp_addr)->high;

    if ( pvp->render_imp_rev.update_size == PV_RENDER_IMP_USZ_4X2 ) {
	xlimit = 512;
	xdelta = 64;
	ylimit = 2560;
	ydelta = 32;
    }
    else {
	xlimit = 1024;
	xdelta = 64;
	ylimit = 1280;
	ydelta = 32;
    }

    for ( y = 0; y < ylimit; y += ydelta ) {
	for ( x = 0; x < xlimit; x += xdelta) {
	    xy0.x = PV_SUBPIX_MAKE( x, 0 );
	    xy0.y = PV_SUBPIX_MAKE( y, 0 );
	    xy1.x = PV_SUBPIX_MAKE( x+xdelta, 0 );
	    xy1.y = PV_SUBPIX_MAKE( y+ydelta, 0 );

	    /*
	     * reset dcc bits
	     */
	    if ( spin_on_pv( pvp ) ) {
		goto exit;
	    }

	    p_rend_dma->pixelmap_id	= ( 2 | PV_PIXELMAP_DCC_MASK );
	    p_rend_dma->xy_min		= xy0;
	    p_rend_dma->xy_max		= xy1;
	    p_rend_dma->addr_low	= low_addr;
	    p_rend_dma->addr_high	= high_addr;
	    p_rend_dma->offset		= 0;
	    p_rend_dma->format		= PV_RENDER_DMA_FORMAT_DCC;
	    wbflush();

	    p_rend_dma->read_write	= PV_RENDER_DMA_RW_READ;
	    wbflush();

	    /*
	     * Clear data bits
	     */
	    if ( spin_on_pv( pvp ) ) {
		goto exit;
	    }

	    p_rend_dma->pixelmap_id	= 2;
	    p_rend_dma->xy_min		= xy0;
	    p_rend_dma->xy_max		= xy1;
	    p_rend_dma->addr_low	= low_addr;
	    p_rend_dma->addr_high	= high_addr;
	    p_rend_dma->offset		= 0;
	    p_rend_dma->format		= PV_RENDER_DMA_FORMAT_RGB;
	    wbflush();

	    p_rend_dma->read_write	= PV_RENDER_DMA_RW_READ;
	    wbflush();
	} /* end for x */
    } /* end for y */

exit:

    if ( stop_gfpa ) {
	STORE_SOFTWARE_REG( pvp, PV_IOC_CNFG_GFPA_HANDSOFF, 0 );
	wbflush();
    }

    /*
     * Done
     */
    return 0;

}


/*
 * Output a character to the screen.
 */
int
pv_blitc(closure, screen, row, col, c)
    caddr_t closure;
    ws_screen_descriptor *screen;
    int row, col;
    u_char c;
{
    pv_info
	*pvp = (pv_info *) closure;

    int
	x,
	y,
	i,
	x2,
	width,
	height,
	char_index,
	stop_gfpa = 0,
	do_frag = 0,
	xdelta,
	xdelta2,
	x0, x1,
	y0, y1,
	frag_size,
	idelta,
	offset_shift,
	is_4x2,
	update_addr;

    pv_register_t
	low_addr,
	high_addr;

    pv_render_dma_t
	*p_rend_dma;

    pv_subpixel_pair_t
	xy0,
	xy1,
	xy2,
	xy3;

    unsigned long
	tmp_addr,
	tmp_addr2,
	tmp_addr3;

    unsigned int
	*p_char,
	*p_junk,
	lrow,
	lcol,
	pixel_on,
	pixel_off,
	glyph,
	offset,
	offset0,
	offset1;

    unsigned short
	*p_glyph,
	mask;

    unsigned char
	*p_cglyph;

    pv_pva_register_t
	*p_pva;

    p_pva = PV_PVA_BASE( pvp );
    p_rend_dma = PV_REND_DMA_BASE( pvp );

#ifdef	NO_CONSOLE_IF_GFPA_RUNS
    {
	pv_register_t
	    *p_sr;

	p_sr = (pv_register_t *) ( (vm_offset_t) PV_GCP_BASE( pvp )
				 + PV_GCP_SR_OFFSET );

	if ( *p_sr != 0 ) {
	    return (0);
	}
    }
#endif	/* NCIGR */

    if ( ( c < ' ' || c > '~' ) && ( c < 0xA1 || c > 0xfd ) ) {
	return 0;
    }

    if ( pvp->state & PV_INFO_STATE_GCP_VALID ) {
	STORE_SOFTWARE_REG( pvp, PV_IOC_CNFG_GFPA_HANDSOFF, HANDSOFF_VALUE );
	wbflush();
	DELAY(20);
	stop_gfpa = 1;
	if ( spin_on_pv( pvp ) ) {
	    goto exit;
	}
    }

    tmp_addr = ( (unsigned long) pvp->phys_packet_area
	     + PVB_PHOST_OFFSET_LOW
	     + ( (unsigned long) PVB_PHOST_OFFSET_HIGH << 32 ) );
    p_char = (unsigned int *) pvp->p_packet_area;
    width = pvp->screen.width;
    height = pvp->screen.height;
    if ( ( lrow = row ) > pvp->screen.max_row ) lrow = pvp->screen.max_row;
    if ( ( lcol = col ) > pvp->screen.max_col ) lcol = pvp->screen.max_col;
    char_index = c - ' ';
    pixel_on = ( pvp->dcc_user << 24 ) | 0xffffff;
    pixel_off = pvp->dcc_user << 24;

    p_junk = p_char;

    p_cglyph = &q_font[char_index * 15];
    for ( y = 0; y < pvp->screen.f_height-1; y++ ) {
	glyph = *p_cglyph++;
	for ( x = 0; x < pvp->screen.f_width; x++ ) {
	    *p_char++ = ( glyph & 0x1 ) != 0 ? pixel_on : pixel_off;
	    glyph >>= 1;
	}
    }
    for ( x = 0; x < pvp->screen.f_width; x++ ) {
	*p_char++ = pixel_off;
    }

    x = lcol * pvp->screen.f_width;
    y = lrow * pvp->screen.f_height;

    tmp_addr2 = tmp_addr;
    offset = pvp->screen.f_width;

    xdelta = pvp->screen.f_width;
    xdelta2 = xdelta - 1;
    is_4x2 = ( pvp->render_imp_rev.update_size == PV_RENDER_IMP_USZ_4X2 );

    if ( is_4x2 ) {
	idelta = 2;
	offset_shift = 3;
    }
    else {
	idelta = 4;
	offset_shift = 4;
    }

    for ( i = 0; i < pvp->screen.f_height; i += idelta ) {
	if ( is_4x2 ) {
	    update_addr = ( ( y + i ) >> 1 ) * 320 + ( x >> 2 );
	    x0 = ( update_addr & 0x180 )
	       + ( ( update_addr & 0x600 ) >> 4 )
	       + ( x & 0x1f );
	    y0 = ( ( update_addr >> 13 ) << 7 )
	       + ( ( update_addr & 0x1800 ) >> 6 )
	       + ( ( update_addr & 0x78 ) >> 2 );
	    x1 = x0 + xdelta;
	    y1 = y0 + 2;
	    xy0.x = PV_SUBPIX_MAKE( x0, 0 );
	    xy0.y = PV_SUBPIX_MAKE( y0, 0 );
	    xy1.y = PV_SUBPIX_MAKE( y1, 0 );
	    frag_size = 0x20 - ( x & 0x1f );
	}
	else {
	    update_addr = ( ( y + i ) >> 2 ) * 320 + ( x >> 2 );
	    x0 = ( ( update_addr & 0x180 ) << 1 )
	       + ( ( update_addr & 0x600 ) >> 3 )
	       + ( x & 0x3f );
	    y0 = ( ( update_addr >> 13 ) << 7 )
	       + ( ( update_addr & 0x1800 ) >> 6 )
	       + ( ( update_addr & 0x70 ) >> 2 );
	    x1 = x0 + xdelta;
	    y1 = y0 + 4;
	    xy0.x = PV_SUBPIX_MAKE( x0, 0 );
	    xy0.y = PV_SUBPIX_MAKE( y0, 0 );
	    xy1.y = PV_SUBPIX_MAKE( y1, 0 );
	    frag_size = 0x40 - ( x & 0x3f );
	}
	if ( frag_size < xdelta ) {
	    /*
	     * rows of updates are discontiguous
	     */
	    do_frag = 1;
	    offset1 = frag_size;
	    offset0 = offset - offset1;	
	    xy1.x = PV_SUBPIX_MAKE( x0 + frag_size, 0 );

	    /*
	     * Re-transform for the second fragment
	     */
	    if ( is_4x2 ) {
		update_addr = ( ( y + i ) >> 1 ) * 320
			    + ( ( x + frag_size ) >> 2 );
		x0 = ( update_addr & 0x180 )
		   + ( ( update_addr & 0x600 ) >> 4 )
		   + ( ( x + frag_size ) & 0x1f );
		y0 = ( ( update_addr >> 13 ) << 7 )
		   + ( ( update_addr & 0x1800 ) >> 6 )
		   + ( ( update_addr & 0x78 ) >> 2 );
		x1 = x0 + xdelta - frag_size;
		y1 = y0 + 2;
		xy2.x = PV_SUBPIX_MAKE( x0, 0 );
		xy2.y = PV_SUBPIX_MAKE( y0, 0 );
		xy3.x = PV_SUBPIX_MAKE( x1, 0 );
		xy3.y = PV_SUBPIX_MAKE( y1, 0 );
	    }
	    else {
		update_addr = ( ( y + i ) >> 2 ) * 320
			    + ( ( x + frag_size ) >> 2 );
		x0 = ( ( update_addr & 0x180 ) << 1 )
		   + ( ( update_addr & 0x600 ) >> 3 )
		   + ( ( x + frag_size ) & 0x3f );
		y0 = ( ( update_addr >> 13 ) << 7 )
		   + ( ( update_addr & 0x1800 ) >> 6 )
		   + ( ( update_addr & 0x70 ) >> 2 );
		x1 = x0 + xdelta - frag_size;
		y1 = y0 + 4;
		xy2.x = PV_SUBPIX_MAKE( x0, 0 );
		xy2.y = PV_SUBPIX_MAKE( y0, 0 );
		xy3.x = PV_SUBPIX_MAKE( x1, 0 );
		xy3.y = PV_SUBPIX_MAKE( y1, 0 );
	    }
	}
	else {
	    /*
	     * contiguous run
	     */
	    do_frag = 0;
	    offset0 = 0;
	    offset1 = 0;
	    xy1.x = PV_SUBPIX_MAKE( x1, 0 );
	}

	tmp_addr3 = ( tmp_addr2 >> SHIFT_FOR_PHOST_DMA );
	low_addr = ((pv_bus_addr2_t *) &tmp_addr3)->low;
	high_addr = ((pv_bus_addr2_t *) &tmp_addr3)->high;

	/*
	 * First, get the dcc bits in
	 */
	if ( spin_on_pv( pvp ) ) {
	    goto exit;
	}
	p_rend_dma->pixelmap_id		= ( 2 | PV_PIXELMAP_DCC_MASK );
	p_rend_dma->xy_min		= xy0;
	p_rend_dma->xy_max		= xy1;
	p_rend_dma->addr_low		= low_addr;
	p_rend_dma->addr_high		= high_addr;
	p_rend_dma->offset		= offset0;
	p_rend_dma->format		= PV_RENDER_DMA_FORMAT_DCC;
	wbflush();

	p_rend_dma->read_write		= PV_RENDER_DMA_RW_READ;
	wbflush();

	/*
	 * Then the glyph
	 */
	if ( spin_on_pv( pvp ) ) {
	    goto exit;
	}
	p_rend_dma->pixelmap_id		= 2;
	p_rend_dma->xy_min		= xy0;
	p_rend_dma->xy_max		= xy1;
	p_rend_dma->addr_low		= low_addr;
	p_rend_dma->addr_high		= high_addr;
	p_rend_dma->offset		= offset0;
	p_rend_dma->format		= PV_RENDER_DMA_FORMAT_RGB;
	wbflush();

	p_rend_dma->read_write		= PV_RENDER_DMA_RW_READ;
	wbflush();

	if ( do_frag ) {

	    tmp_addr3 = ( ( tmp_addr2 + ( offset1 << 2 ) )
		      >> SHIFT_FOR_PHOST_DMA );
	    low_addr = ((pv_bus_addr2_t *) &tmp_addr3)->low;
	    high_addr = ((pv_bus_addr2_t *) &tmp_addr3)->high;

	    /*
	     * First, get the dcc bits in
	     */
	    if ( spin_on_pv( pvp ) ) {
		goto exit;
	    }
	    p_rend_dma->pixelmap_id	= ( 2 | PV_PIXELMAP_DCC_MASK );
	    p_rend_dma->xy_min		= xy2;
	    p_rend_dma->xy_max		= xy3;
	    p_rend_dma->addr_low	= low_addr;
	    p_rend_dma->addr_high	= high_addr;
	    p_rend_dma->offset		= offset1;
	    p_rend_dma->format		= PV_RENDER_DMA_FORMAT_DCC;
	    wbflush();

	    p_rend_dma->read_write	= PV_RENDER_DMA_RW_READ;
	    wbflush();

	    /*
	     * Then the glyph
	     */
	    if ( spin_on_pv( pvp ) ) {
		goto exit;
	    }
	    p_rend_dma->pixelmap_id	= 2;
	    p_rend_dma->xy_min		= xy2;
	    p_rend_dma->xy_max		= xy3;
	    p_rend_dma->addr_low	= low_addr;
	    p_rend_dma->addr_high	= high_addr;
	    p_rend_dma->offset		= offset1;
	    p_rend_dma->format		= PV_RENDER_DMA_FORMAT_RGB;
	    wbflush();

	    p_rend_dma->read_write	= PV_RENDER_DMA_RW_READ;
	    wbflush();
	}

	/*
	 * Move to start of next update array run
	 */
	tmp_addr2 += ( offset << offset_shift );

    } /* end for */

    for ( y = 0; y < pvp->screen.f_height; y++ ) {
	for ( x = 0; x < pvp->screen.f_width; x++ ) {
	    *p_junk++ = pixel_off;
	}
    }

exit:

    if ( stop_gfpa ) {
	STORE_SOFTWARE_REG( pvp, PV_IOC_CNFG_GFPA_HANDSOFF, 0 );
	wbflush();
    }

    /*
     * Done
     */
    return 0;
}


/*
 * Initialize as console device.
 */
int
pv_cons_init(address, slot)
     caddr_t address;
     int slot;
{
    register pv_info
	*pvp = &pv_softc[0];

    int
	status = 0;

#ifdef mips
    address = (caddr_t) ws_where_option("pv");
#endif
#ifdef __alpha
    address = (caddr_t) PHYS_TO_KSEG( address );

    pvp->ctlr = (struct controller *) NULL;
    {
	struct controller
	    *ctlr;

	extern struct controller
	    *get_ctlr_num();

	if ( ( ctlr = get_ctlr_num("pv", 0) ) != (struct controller *) NULL &&
	     ctlr->slot == -1 ) {
	    ctlr->slot = slot;
	    pvp->ctlr = ctlr;
	}
    }

#endif /* __alpha */

    PV_DEBUG(PV_TALK,
	     PRINTFPOLL( ("pv_cons_init(0) -> %x\n", address) );
	     );

    if ( ( status = pv_attach( address, 0, 0 ) ) != -1 ) {
	pv_init_screen( pvp, &pvp->screen );
	pv_scroll_screen( pvp, &pvp->screen );
	status = 1;
    }
    else {
	status = 0;
    }

    return (status);
}


/*
 * Initialize the screen.
 */
pv_init_screen(closure, sp)
    caddr_t closure;
    ws_screen_descriptor *sp;
{
    return 0;
}

/*
 * pv_intr_vsync
 *
 * Vertical sync processing
 *
 * Conditionally clears and disables the vsync interrupt after processing
 *
 * There is some inportant code ordering in this routine.  First, all
 * window tag table operations are performed as these *must* occur during
 * vsync or hsync.  Next, colormap loading is performed.  If the video
 * is turned off while in 12/12 stereo mode, we allow the vsync intr
 * enable bit to be cleared.  Later, when the video is turned on, a single
 * vsync is enabled which will get the 12/12 toggle going again.
 * 
 * As an optimization, we use the lockout bit to get DMAs off of the bus.
 * A side effect of this is that we can't do 1212 stereo as we can't get
 * to the PV status register to check which field we're looking at.
 *
 */
static void
pv_intr_vsync( pvp )
    pv_info *pvp;
{
    volatile struct bt463info
	*bti = (struct bt463info *) pvp->cmf.cmc;

    volatile struct bt431info
	*btii = (struct bt431info *) pvp->cf.cc;

    pv_pva_register_t
	*p_pva;

    pv_csr_t
	csr;

    int
	disable_int = 1;

    p_pva = PV_PVA_BASE( pvp );

/*    s = splvm(); */

#ifdef	DO_LOCKOUT_IN_VSYNC
    csr = p_pva->csr;
    csr.dma_lockout = 1;
    p_pva->csr = csr;
    wbflush();
#endif

    if ( pvp->video_on_off_dirty ) {
	if ( pvp->video_on_off ) {
	    pv_bt463_video_on( bti );
	}
	else {
	    pv_bt463_load_wid( bti->btaddr, 0, WINDOW_TAG_COUNT, wids );
	    pv_bt463_video_off( bti );
	}
	pvp->video_on_off_dirty = 0;
    }

    if ( pvp->stereo_mode == PV_INFO_STEREO_1212 && pvp->video_on_off ) {
	/*
	 * If we're in stereo mode, we leave vsync on all the time
	 * so that we can toggle some windowing registers.  We effect
	 * the update by making the toggled window tag entries appear
	 * as if they had been dirtied and then calling the 'standard'
	 * reload routine.
	 */
	int
	    start,
	    end;

	start = pvp->stereo_wtag_start;
	end = start + pvp->stereo_wtag_count - 1;
	if ( pvp->wt_left_min_dirty > start ) pvp->wt_left_min_dirty = start;
	if ( pvp->wt_right_min_dirty > start ) pvp->wt_right_min_dirty = start;
	if ( pvp->wt_left_max_dirty < end ) pvp->wt_left_max_dirty = end;
	if ( pvp->wt_right_max_dirty < end ) pvp->wt_right_max_dirty = end;
	pvp->wt_dirty = 1;
	bt_clean_window_tag( pvp );
	disable_int = 0;
    }
    else if ( pvp->wt_dirty && pvp->video_on_off ) {
	/*
	 * If we're in 1212 mode, we don't have to do this again.
	 */
	bt_clean_window_tag( pvp );
    }

    if ( bti->dirty_cursormap ) {
	pv_bt463_clean_cursormap( bti );
    }

    if ( bti->dirty_colormap ) {
	pv_bt463_clean_colormap( bti );
    }

#ifdef	DO_LOCKOUT_IN_VSYNC
    csr = p_pva->csr;
    csr.dma_lockout = 0;
    p_pva->csr = csr;
    wbflush();
#endif

    if ( btii->dirty_cursor ) {
	bt431_load_formatted_cursor( btii );
    }

/*    splx(s); */

    if ( disable_int ) {
	PV_INT_DISABLE( pvp, PV_INT_VSYNC );
    }
    PV_INT_CLEAR( pvp, PV_INT_VSYNC );

    return;
}

/*
 * pv_invalidate_gcp_tlb
 *
 * Device-dependent pageout handler.  This should be called at splvm().
 */
int
pv_invalidate_gcp_tlb( func, va, data )
     int func;
     vm_offset_t va;
     caddr_t data;
{
    pv_info
	*pvp = (pv_info *) data;

    unsigned int
	vpn;

    pv_pva_tlb_entry_t
	tlb_entry;

    pv_pva_register_t
	*p_pva;

    pv_csr_t
	csr;

    int
	status = 0,
	s,
	tlb_index;

    pv_register_t
	intr_status;

    pvInfo
	*pvip = (pvInfo *) pvp->p_info_area;

    PV_DEBUG( PV_TALK, PRINTFPOLL( ( "gcp_invalidate: %d  0x%lx\n",
				func, va ) ); );

    /*
     * We need to quiet the module while we're modifying PVA TLB entries.
     * There isn't a very clean way of doing that so we'll use the lockout
     * bit and a delay to achieve an approximation of this effect.
     */
    p_pva = PV_PVA_BASE( pvp );
#ifdef DO_LOCKOUT_IN_INVALIDATION
    csr = p_pva->csr;
    csr.dma_lockout = 1;
    p_pva->csr = csr; wbflush();
    DELAY(2);
#endif

    switch (func) {

#ifdef	DO_NEW_STYLE_INVALIDATE

    case PMAP_COPROC_INVALIDATE_STLB:
        /*
	 * A page in the server process is going to be paged out.
	 * Remove all traces of it from the shadow tlb and the
	 * PVA tlb.
	 */
	vpn = PV_VA_TO_VPN( va );
	if ( pvp->image_check_va_range &&
	     vpn >= pvp->image_va_first_vpn &&
	     vpn <= pvp->image_va_last_vpn ) {
	    /*
	     * Don't let VM scavenge this page.  We tell it to lay off
	     * and we should be OK....
	     */
	    status = 1;
	    break;
	}
	tlb_index = vpn & ( pvp->page_table_1_size - 1 );
	tlb_entry = pvp->p_page_table_1[tlb_index];
	if ( tlb_entry.vpn == vpn && tlb_entry.pid == PV_PVA_PID_SERVER ) {
	    INVALID_STLB_ENTRY( pvp, tlb_index, (&tlb_entry) );
	    pvp->p_page_table_1[tlb_index] = tlb_entry;
#ifndef	DO_LOCKOUT_IN_INVALIDATION
	    PV_DEBUG( PV_TALK,
		PRINTFPOLL( ( "gcp_inval: inval stlb1[%d] with 0x%lx\n",
				tlb_index, * (unsigned long *) &tlb_entry ) ); );
#endif
	}
	for ( tlb_index = 0; tlb_index < PV_PVA_TLB_ENTRIES; tlb_index++ ) {
	    tlb_entry = p_pva->tlb[tlb_index];
	    if ( tlb_entry.vpn == vpn && tlb_entry.pid == PV_PVA_PID_SERVER ) {
		if ( pvip->gfpa_halts_on_countdown &&
		     pvp->gfpa_is_running &&
		     pvp->state & PV_INFO_STATE_GCP_VALID ) {
		    STORE_SOFTWARE_REG( pvp, PV_IOC_CNFG_GFPA_COUNTDOWN,
					0x3f800000 );
		    intr_status = p_pva->intr_status;
		    while (!( intr_status & PV_PVA_INTR_PB1 ) ) {
			intr_status = p_pva->intr_status;
		    }
		}
		INVALID_PVA_TLB_ENTRY( pvp, tlb_index, (&tlb_entry) );
		p_pva->tlb[tlb_index] = tlb_entry;
#ifndef	DO_LOCKOUT_IN_INVALIDATION
	    PV_DEBUG( PV_TALK,
		PRINTFPOLL( ( "gcp_inval: inval pvatlb[%d] with 0x%lx\n",
				tlb_index, * (unsigned long *) &tlb_entry ) ); );
#endif
		break;
	    }
	}
	break;

     case PMAP_COPROC_EXIT:
	/*
	 * We're in the process of exiting so make the hardware safe.
	 */
	halt_gfpa( pvp );
	pvp->image_failure = 1;
	invalidate_tlb( pvp, INVALIDATE_SERVER );
	invalidate_pva_tlb( pvp, INVALIDATE_SERVER );
	break;

#else /* DO_NEW_STYLE_INVALIDATE */

    case PDEVCMD_ONE:
        /*
	 * A page in the server process is going to be paged out.
	 * Remove all traces of it from the shadow tlb and the
	 * PVA tlb.
	 */
	vpn = PV_VA_TO_VPN( va );
	if ( pvp->image_check_va_range &&
	     vpn >= pvp->image_va_first_vpn &&
	     vpn <= pvp->image_va_last_vpn ) {

#ifdef	IMAGE_FAILS_ON_SINGLE_INVALIDATE
	    /*
	     * Problem.  Can't reject this operation.  Mark as potential
	     * failure.  Leave entries in place to allow PUT_IMAGE to
	     * continue.
	     */
	    pvp->image_failure = 1;
#ifndef DO_LOCKOUT_IN_INVALIDATION
	    PV_DEBUG( PV_TALK,
		PRINTFPOLL( ( "gcp_invalidate: softlocked page, fail image\n" ) ); );
#endif

#else /* IMAGE_FAILS_ON_SINGLE_INVALIDATE */

	    /*
	     * Don't let VM scavenge this page.  We tell it to lay off
	     * and we should be OK....
	     */
	    status = 1;
#ifndef DO_LOCKOUT_IN_INVALIDATION
	    PV_DEBUG( PV_TALK,
		PRINTFPOLL( ( "gcp_invalidate: softlocked page, inval rejected\n" ) ); );
#endif
	    break;

#endif /* IMAGE_FAILS_ON_SINGLE_INVALIDATE */

	}
	tlb_index = vpn & ( pvp->page_table_1_size - 1 );
	tlb_entry = pvp->p_page_table_1[tlb_index];
	if ( tlb_entry.vpn == vpn && tlb_entry.pid == PV_PVA_PID_SERVER ) {
	    INVALID_STLB_ENTRY( pvp, tlb_index, (&tlb_entry) );
	    pvp->p_page_table_1[tlb_index] = tlb_entry;
#ifndef	DO_LOCKOUT_IN_INVALIDATION
	    PV_DEBUG( PV_TALK,
		PRINTFPOLL( ( "gcp_inval: inval stlb1[%d] with 0x%lx\n",
				tlb_index, * (unsigned long *) &tlb_entry ) ); );
#endif
	}
	for ( tlb_index = 0; tlb_index < PV_PVA_TLB_ENTRIES; tlb_index++ ) {
	    tlb_entry = p_pva->tlb[tlb_index];
	    if ( tlb_entry.vpn == vpn && tlb_entry.pid == PV_PVA_PID_SERVER ) {
		if ( pvip->gfpa_halts_on_countdown &&
		     pvp->gfpa_is_running &&
		     pvp->state & PV_INFO_STATE_GCP_VALID ) {
		    STORE_SOFTWARE_REG( pvp, PV_IOC_CNFG_GFPA_COUNTDOWN,
					0x3f800000 );
		    intr_status = p_pva->intr_status;
		    while (!( intr_status & PV_PVA_INTR_PB1 ) ) {
			intr_status = p_pva->intr_status;
		    }
		}
		INVALID_PVA_TLB_ENTRY( pvp, tlb_index, (&tlb_entry) );
		p_pva->tlb[tlb_index] = tlb_entry;
#ifndef	DO_LOCKOUT_IN_INVALIDATION
	    PV_DEBUG( PV_TALK,
		PRINTFPOLL( ( "gcp_inval: inval pvatlb[%d] with 0x%lx\n",
				tlb_index, * (unsigned long *) &tlb_entry ) ); );
#endif
		break;
	    }
	}
	break;

     case PDEVCMD_ALL:
     case PDEVCMD_DMA:
	/*
	 * VM system wants to invalidate all translations for the server.
	 * Wipe out everything you know.
	 */
	if ( pvp->image_check_va_range ) {
	    /*
	     * Problem.  Can't reject this operation.  Mark as potential
	     * failure.  Leave entries in place to allow PUT_IMAGE to
	     * continue.
	     */
	    pvp->image_failure = 1;
	}
	if ( func == PDEVCMD_DMA ) {
	    /*
	     * Process is exiting so shutdown the GFPA.
	     */
	    halt_gfpa( pvp );
	}
	if ( pvip->gfpa_halts_on_countdown &&
	     pvp->gfpa_is_running &&
	     pvp->state & PV_INFO_STATE_GCP_VALID ) {
	    STORE_SOFTWARE_REG( pvp, PV_IOC_CNFG_GFPA_COUNTDOWN,
				0x3f800000 );
	    intr_status = p_pva->intr_status;
	    while (!( intr_status & PV_PVA_INTR_PB1 ) ) {
		intr_status = p_pva->intr_status;
	    }
	}
	invalidate_tlb( pvp, INVALIDATE_SERVER );
	invalidate_pva_tlb( pvp, INVALIDATE_SERVER );
	break;

#endif /* DO_NEW_STYLE_INVALIDATE */

     default:
#ifdef DO_LOCKOUT_IN_INVALIDATION
	csr = p_pva->csr;
	csr.dma_lockout = 0;
	p_pva->csr = csr; wbflush();
#endif
	PRINTFPOLL( ("pv_invalidate_gcp_tlb: bad func 0x%x\n", func) );
	panic("pv_invalidate_gcp_tlb");
    }

    /*
     * Re-enable DMAs
     */
#ifdef DO_LOCKOUT_IN_INVALIDATION
    csr = p_pva->csr;
    csr.dma_lockout = 0;
    p_pva->csr = csr; wbflush();
#endif

    /*
     * Done
     */
    return (status);
}

/*
 * pv_close
 *
 * Last close.  Do some cleanup
 */
void
pv_close(pvp)
    pv_info *pvp;
{
    pv_pva_register_t
 	*p_pva;

    pv_render_implement_t
	*p_rend;

    pv_register_t
	*p_gcp,
	*p_gcp_register;

    int
	i,
	s;

    long
	*p_zero;

    /*
     * unexpress interest in server's vm activity...
     */
    if ( pvp->p_proc[PV_IOC_PID_GCP_INDEX] != (struct proc *) NULL ) {
	extern int ws_unregister_vm_callback();

	PV_DEBUG(PV_GAB,
		 PRINTFPOLL( ("pv_close: pid=%d\n",
			pvp->p_proc[PV_IOC_PID_GCP_INDEX]->p_pid) ); );
	ws_unregister_vm_callback( pvp->screen.screen );
    }
    s = PV_RAISE_SPL();
    for ( i = 0; i < PV_IOC_PID_NUM; i++ ) {
	pvp->p_proc[i] = (struct proc *) NULL;
	pvp->sig_mask[i] = 0;
	pvp->pid[i] = 0;
    }
    pvp->state = 0;

    /*
     * Clean the info area
     */
    init_info_area( pvp );

    /*
     * Re-init the hardware
     */
    init_devices(pvp, 1);

    /*
     * NOTE: Shutdown and clean up module
     */
    invalidate_tlb( pvp, INVALIDATE_SERVER );
    invalidate_tlb( pvp, INVALIDATE_PAGER );
    invalidate_pva_tlb( pvp, INVALIDATE_ALL );

    /*
     * Get addresses of interesting things
     */
    p_pva = PV_PVA_BASE( pvp );
    p_rend = PV_REND_IMP_BASE( pvp );
    p_gcp = (pv_register_t *) PV_GCP_BASE( pvp );

    /*
     * Make certain GCP is quiet
     */
    halt_gfpa( pvp );

    p_gcp_register = (pv_register_t *)( (vm_offset_t) p_gcp
				      + PV_GCP_CMP01_OFFSET );
    *p_gcp_register++ = 0x00000000;
    *p_gcp_register = 0x80000000;			wbflush();

    /*
     * Get the PV to shutup
     */
    PV_INT_DISABLE( pvp, (PV_INT_STEREO|PV_INT_VSYNC|PV_INT_PAGE|PV_INT_VM|
			  PV_INT_RENDER|PV_INT_PACKET) );

    PV_INT_CLEAR( pvp, (PV_INT_VSYNC|PV_INT_PAGE|PV_INT_VM|PV_INT_RENDER|
			PV_INT_PACKET|PV_INT_STALL) );

    /*
     * Fixup various interrupt masks
     */
    * (pv_register_t *) &p_pva->dma_miss = (pv_register_t) 0; wbflush();

    /*
     * Reset some working values
     */
    pvp->dcc_user			= pvp->dcc_default;
    pvp->image_check_va_range		= 0;
    pvp->image_hardlock_in_progress	= 0;
    pvp->wt_dirty			= 1;
    pvp->wt_left_min_dirty		= 8;
    pvp->wt_left_max_dirty		= WINDOW_TAG_COUNT-1;
    pvp->wt_right_min_dirty		= 8;
    pvp->wt_right_max_dirty		= WINDOW_TAG_COUNT-1;
    for ( i = 0; i < WINDOW_TAG_COUNT; i++ ) {
	pvp->wt_right_cell[i].windex = pvp->wt_left_cell[i].windex = i;
	pvp->wt_right_cell[i].low = pvp->wt_left_cell[i].low = wids[i].low_byte;
	pvp->wt_right_cell[i].mid = pvp->wt_left_cell[i].mid = wids[i].middle_byte;
	pvp->wt_right_cell[i].high = pvp->wt_left_cell[i].high = wids[i].high_byte;
    }

    /*
     * Zero clip area for next server
     */
    p_zero = (long *) pvp->p_clip_area;
    for ( i = 0; i < ( NBPG / sizeof( long ) ); i++ ) {
	*p_zero++ = 0L;
    }

    /*
     * Restore
     */
    PV_LOWER_SPL(s);

    /*
     * Done
     */
    return;
}

/*
 * pv_init_closure
 */
caddr_t
pv_init_closure(closure, address, unit, type)
        caddr_t closure;
        caddr_t address;
        int unit;
        int type;
{
    pv_info
	*pvp = (pv_info *) closure;

    register int
	i;

    pvp += unit;
    for ( i = 0; i < pvp->screen.allowed_depths; i++ ) {
	pvp->depth[i].physaddr = address
	  		       + (unsigned long) pvp->depth[i].physaddr;
    }
    return (caddr_t) &pvp->screen;
}

/*
 * pv_bt431_init_closure
 *
 * Init closure struct for bt431
 */
caddr_t
pv_bt431_init_closure(closure, address, unit, type)
    caddr_t closure;
    caddr_t address;
    int unit;
    int type;
{
    register struct bt431info *bti = (struct bt431info *) closure;
    register volatile struct bt431 *btp;
    register volatile struct bt431 *btp2;
    register caddr_t addr, addr2;
    register u_int i;
    register u_int nextunit = 0;

    /*
     * see if we've already init'd the closure for this vdac already
     */
    addr = address + (vm_offset_t) bt431_type[type].btaddr;
    for (i = 0; i < nbt_softc; i++) {
        if ( addr == (caddr_t) bti[i].btaddr ) {
            bti[i].unit = unit;
            return (caddr_t) (&bti[i]);
        }
        else if ( bti[i].btaddr == NULL ) {
            nextunit = i;
            break;
        }
    }

    /*
     * setup another struct, if possible
     */
    if ( i >= nbt_softc ) 
       return (caddr_t) NULL;

    bti += nextunit;
    nextunit += 1;

    /*
     * set to initial values
     */
    *bti = bt431_type[type];
    bti->unit = unit;

    /*
     * update relative offsets to physical addresses
     */
    bti->btaddr = btp = (struct bt431 *) addr;
    if ( bti->btaddr2 != NULL ) {
	addr2 = address + (vm_offset_t) bt431_type[type].btaddr2;
	bti->btaddr2 = btp2 = (struct bt431 *) addr2;
    }

    /*
     * The following line is *NOT* correct but we don't use the information.
     */
    bti->cmap_closure = (caddr_t) &bt463_softc[unit];

    /*
     * init the puppy
     */
    bt431_init( bti );

    /*
     * Done
     */
    return (caddr_t) bti;
}

/*
 * pv_bt463_init_closure
 *
 * Init closure struct for bt463
 */
caddr_t
pv_bt463_init_closure(closure, address, unit, type)
    caddr_t closure;
    caddr_t address;
    int unit;
    int type;
{
    register struct bt463info *bti = (struct bt463info *) closure;
    register volatile struct bt463 *btp;
    register caddr_t addr;
    register u_int i;
    register u_int nextunit = 0;

    /*
     * see if we've already init'd the closure for this vdac already
     */
    addr = address + (vm_offset_t) bt463_type[type].btaddr;
    for (i = 0; i < nbt_softc; i++) {
        if ( addr == (caddr_t) bti[i].btaddr ) {
            bti[i].unit = unit;
            return (caddr_t) (&bti[i]);
        }
        else if ( bti[i].btaddr == NULL ) {
            nextunit = i;
            break;
        }
    }

    /*
     * setup another struct, if possible
     */
    if ( i >= nbt_softc ) 
       return (caddr_t) NULL;

    bti += nextunit;
    nextunit += 1;

    /*
     * set to initial values
     */
    *bti = bt463_type[type];
    bti->unit = unit;

    /*
     * update relative offsets to physical addresses
     */
    bti->btaddr = btp = (struct bt463 *) addr;

    /*
     * The following line is *NOT* correct but we don't use the information.
     */
    bti->cursor_closure = (caddr_t) &bt431_softc[unit];

    /*
     * init
     */
    pv_bt463_init( bti );

    /*
     * Done
     */
    return (caddr_t) bti;
}

/*
 * set_packet_dcc
 *
 * Load the upper byte of the data area with the currect user dcc bits
 */
static void
set_packet_dcc( pvp )
     pv_info *pvp;
{
    int
	i,
	j;

    unsigned int
	value = (((unsigned) pvp->dcc_user) << 24),
	*p_crud;

    p_crud = (pv_register_t *) pvp->p_packet_area;
    j = NBPG / sizeof( pv_register_t );
    for ( i = 0; i < j; i++ ) {
	*p_crud++ = value;
    }

    /*
     * Done
     */
    return;
}

/*
 * spin_on_pv
 *
 * Spin waiting for vmstatus to clear.
 */
static int
spin_on_pv( pvp )
    pv_info *pvp;
{
    int
	i;

    pv_render_implement_t
	*p_imp;

    pv_render_status_t
	rend_status;

    p_imp = PV_REND_IMP_BASE( pvp );

    for ( i = 0; i < 100000; i++ ) {
	rend_status = p_imp->status;
	if ( rend_status.vm_status == 0 && 
	     ( rend_status.render_status == 0 ||
	       rend_status.page_int == 1 ) ) {
	    return (0);
	}
	DELAY(20);
    }

    return (1);
}


/*
 * bt_clean_window_tag
 *
 *  Load some window tags
 */
static void
bt_clean_window_tag( pvp )
	pv_info *pvp;
{
	volatile struct bt463info
	    *bti = (struct bt463info *) pvp->cmf.cmc;

	volatile struct bt463
	    *btp = bti->btaddr;

	register int
	    win_addr,
	    start,
	    end,
	    i;

	volatile pv_window_tag_cell
	    *p_table;

	volatile pv_render_implement_t
	    *p_imp;

	pv_render_status_t
	    rend_status;

        register int
	    s = PV_RAISE_SPL();

	if ( !bti->screen_on ) {
	    PV_LOWER_SPL(s);
	    return;
	}

	/*
	 * If we could support 1212 stereo mode here, we would read
	 * the field bit in the PV status register and load either
	 * the left or right set of window tags.
	 */
	start = pvp->wt_left_min_dirty;
	end = pvp->wt_left_max_dirty;
	p_table = pvp->wt_left_cell;

	for ( i = start; i <= end; i++) {
	    win_addr = WINDOW_TYPE_TABLE + i;
	    btp->addr_low = win_addr & ADDR_LOW_MASK;
	    wbflush();
            btp->addr_high = ( win_addr & ADDR_HIGH_MASK ) >> 8;
	    wbflush();
	    btp->bt_reg = p_table[i].low;		wbflush();
	    btp->bt_reg = p_table[i].mid;		wbflush();
	    btp->bt_reg = p_table[i].high;		wbflush();
	}

	/*
	 * Clean up.  Regardless of which field we are actually in,
	 * we can mark both tables as clean.
	 */
	pvp->wt_left_min_dirty = WINDOW_TAG_COUNT-1;
	pvp->wt_right_min_dirty = WINDOW_TAG_COUNT-1;
	pvp->wt_left_max_dirty = 0;
	pvp->wt_right_max_dirty = 0;
	pvp->wt_dirty = 0;
        PV_LOWER_SPL(s);

	return;
}

#ifdef	USE_VSLOCK_EMULATION

static int
lcl_vslock( addr, len, prot )
    caddr_t	addr;
    int		len;
    int		prot;
{
    kern_return_t
	status;

    extern kern_return_t
	vm_map_pageable();

    status = vm_map_pageable( current_task()->map, trunc_page(addr),
			round_page(addr+len), prot );

    return (lcl_kern_return_xlate(status));
}

static int
lcl_vsunlock( addr, len, dirtied )
    caddr_t	addr;
    int		len;
    int		dirtied;
{
    kern_return_t
	status;

    extern kern_return_t
	vm_map_pageable();

    status = vm_map_pageable( current_task()->map, trunc_page(addr),
			round_page(addr+len), VM_PROT_NONE );

    return (lcl_kern_return_xlate(status));
}

static int
lcl_kern_return_xlate( status )
    int		status;
{
    int
	stat;

    switch (status) {

case KERN_SUCCESS:		stat = 0; break;
case KERN_INVALID_ADDRESS:	stat = EFAULT; break;
case KERN_PROTECTION_FAILURE:	stat = EACCES; break;
case KERN_NO_SPACE:		stat = ENOMEM; break;
case KERN_INVALID_ARGUMENT:	stat = EINVAL; break;
case KERN_NO_ACCESS:		stat = EACCES; break;
case KERN_MEMORY_FAILURE:	stat = ENOMEM; break;
case KERN_MEMORY_ERROR:		stat = ENOMEM; break;
default:			stat = EIO; break;
    }

    return (stat);
}

#endif	/* USE_VSLOCK_EMULATION */


#ifdef	BUILD_PERFORMANCE_IOCTLS

/*
 * Output a character to the screen.
 */
static int
lcl_blitc(closure, screen, row, col, c)
    caddr_t closure;
    ws_screen_descriptor *screen;
    int row, col;
    u_char c;
{
    return 0;
}

#endif	/* BUILD_PERFORMANCE_IOCTLS */


/* pv_bt463_clean_colormap
 *
 *	Perform the actual loading of the lookup table to update
 *	it to pristine condition.
 */
static void
pv_bt463_clean_colormap(closure)
	caddr_t closure;
{
	register struct bt463info *bti 		= (struct bt463info *)closure;
	register volatile struct bt463 *btp 	= bti->btaddr;
	register int i;
	register struct bt463_color_cell *entry;
	register unsigned int color;
        register int s = PV_RAISE_SPL();

	for (i = bti->min_dirty; i <= bti->max_dirty ; i++) {
		entry = &bti->cells[i];
		if (entry->dirty_cell) {
			entry->dirty_cell = 0;
                        btp->addr_low = i & ADDR_LOW_MASK;    wbflush();
                        btp->addr_high = (i & ADDR_HIGH_MASK) >> 8; wbflush();
			color = ( entry->red & 0xff )
			      | ( ( entry->green & 0xff ) << 8 )
			      | ( ( entry->blue & 0xff ) << 16 );
			btp->color_map = color;		wbflush();
		}
	}
	bti->max_dirty = 0;
	bti->min_dirty = BT463_CMAP_ENTRY_COUNT;
	bti->dirty_colormap = 0;
        PV_LOWER_SPL(s);
}

/*
 * pv_bt463_load_wid
 */
static void
pv_bt463_load_wid( btp, index, count, data )
	struct bt463 *btp;
	int index;		/* =  first location in wid to load. */
	int count;		/* =  number of entries in wid to load. */
	Bt463_Wid_Cell  *data;	/* -> data to load into wid. */
{
	int data_i;	/* Index: entry in data now accessing. */

	btp->addr_low = index & ADDR_LOW_MASK;	wbflush();
	btp->addr_high = 3;			wbflush();
	for( data_i = 0; data_i < count; data_i++ )
	{
		btp->addr_low = (data_i+index) & ADDR_LOW_MASK;wbflush();
		btp->addr_high = 3;			wbflush();
		btp->bt_reg = data[data_i].low_byte;	wbflush();
		btp->bt_reg = data[data_i].middle_byte;	wbflush();
		btp->bt_reg = data[data_i].high_byte;	wbflush();
	}

	return;
}


/*
 * pv_bt463_init
 */
static void
pv_bt463_init(closure)
	caddr_t closure;
{
    register struct bt463info *bti = (struct bt463info *)closure;
    register volatile struct bt463 *dreg = bti->btaddr;    
    u_int	color_i;	      /* Index: next lut entry to set. */
    u_int	color_v;	      /* Value to load into lut. #### */
    int		i;
    static unsigned char data0[13] = {
	    0x48,
	    0x48,
	    0xc0,
	    0x00,
	    0xff,
	    0xff,
	    0xff,
	    0xff,
	    0x00,
	    0x00,
	    0x00,
	    0x00,
	    0x00
    };

    /*	    
     *	    Configure the RAMDAC.
     *	    Initialize pixel read mask, blink mask and test register.
     */
    dreg->addr_low = 1;		wbflush();
    dreg->addr_high = 2;	wbflush();
    for ( i = 0; i < 13; i++ ) {
	dreg->bt_reg = data0[i];
	wbflush();
    }

    /*
     * Done
     */
    return;
}

/* pv_bt463_init_color_map
 *
 *	Initialize color map in Brooktree 463. Note the entire
 *	color map is initialized, both the 8-bit and the 24-bit
 *	portions.
 */

pv_bt463_init_color_map(closure)
	caddr_t closure;    
{
	register struct bt463info *bti = (struct bt463info *)closure; 
	register volatile struct bt463 *btp = bti->btaddr;
	register int i;	

	pv_bt463_init(closure);

	btp->addr_low = LUT_BASE_8 & ADDR_LOW_MASK; 	wbflush();
	btp->addr_high = (LUT_BASE_8 & ADDR_HIGH_MASK) >> 8; 	wbflush();
	btp->color_map = 0; 				wbflush();

        for(i = 1; i <256; i++) 
	{
	    btp->color_map = 0xffffff;			wbflush();
	}

	btp->addr_low = LUT_BASE_24 & ADDR_LOW_MASK; 	wbflush();
	btp->addr_high = (LUT_BASE_24 & ADDR_HIGH_MASK) >> 8; 	wbflush();

	for(i = 0; i <256; i++) 
	{
	    btp->color_map = (i) | (i<<8) | (i<<16); wbflush();
	}

	bti->cursor_fg.red = bti->cursor_fg.green = bti->cursor_fg.blue 
		= 0xffff;
	bti->cursor_bg.red = bti->cursor_bg.green = bti->cursor_bg.blue 
		= 0x0000;

	pv_bt463_restore_cursor_color(closure, 0);

}

/* pv_bt463_load_color_map_entry
 *
 *	Load one or more entries in the bt463's color lookup table.
 *		= 0 if success
 *		= -1 if error occurred, (index too big)
 *      Two ways to look at colormap.  If map == 1, direct colormap
 *      access.  If map == 0, pseudocolor index 256 == direct color index 0.
 */
/* ARGSUSED */
int
pv_bt463_load_color_map_entry(closure, map, entry)
	caddr_t closure;
	int map;
	register ws_color_cell *entry;
{
	register struct bt463info *bti 		= (struct bt463info *)closure;
	register volatile struct bt463 *btp 	= bti->btaddr;
	register int index = entry->index;
	int s;

        if ( map == 1 )
                index += 256;
	if ( map == 2 )
		index += 512;
	if ( index >= BT463_CMAP_ENTRY_COUNT ) 
		return -1;

	s = PV_RAISE_SPL();
	bti->cells[index].red   = entry->red   >> 8;
	bti->cells[index].green = entry->green >> 8;
	bti->cells[index].blue  = entry->blue  >> 8;
	bti->cells[index].dirty_cell = 1;
	if (index < bti->min_dirty) bti->min_dirty = index;
	if (index > bti->max_dirty) bti->max_dirty = index;
	bti->dirty_colormap = 1;
	if (bti->enable_interrupt) 
		(*bti->enable_interrupt)(closure);
	else 
		pv_bt463_clean_colormap(bti);
	PV_LOWER_SPL(s);
	return 0;
}

/* pv_bt463_video_on
 *
 *	Turn on video display.
 */
static int
pv_bt463_video_on(closure)
	caddr_t closure;
{
	register struct bt463info *bti		= (struct bt463info *)closure;
	register volatile struct bt463 *btp 	= bti->btaddr;    
        register int s;

	if ( bti->screen_on ) 
	    return (0);

	s = PV_RAISE_SPL();
	btp->addr_low = ( P0_P7_READ_MASK & ADDR_LOW_MASK );
	wbflush();
	btp->addr_high = ( P0_P7_READ_MASK & ADDR_HIGH_MASK ) >> 8;
	wbflush();
	btp->bt_reg = 0xff;		wbflush();
	btp->bt_reg = 0xff;		wbflush();
	btp->bt_reg = 0xff;		wbflush();
	btp->bt_reg = 0xff;		wbflush();
	bti->screen_on = 1;
        PV_LOWER_SPL(s);
	return(0);
}

/* pv_bt463_video_off
 *
 *	Turn off video display.
 */
static int
pv_bt463_video_off(closure)
	caddr_t closure;
{
	register struct bt463info *bti 		= (struct bt463info *)closure;
	register volatile struct bt463 *btp 	= bti->btaddr;    
	register volatile Fb_Device_Regs *dreg = (Fb_Device_Regs *)  btp;
        register int s;

	if ( !bti->screen_on )
	    return (0);

	s = PV_RAISE_SPL();
	btp->addr_low = ( P0_P7_READ_MASK & ADDR_LOW_MASK );
	wbflush();
	btp->addr_high = ( P0_P7_READ_MASK & ADDR_HIGH_MASK ) >> 8;
	wbflush();
	btp->bt_reg = 0;			wbflush();
	btp->bt_reg = 0;			wbflush();
	btp->bt_reg = 0;			wbflush();
	btp->bt_reg = 0;			wbflush();
	bti->screen_on = 0;
        PV_LOWER_SPL(s);
	return(0);
}

/*
 * pv_bt463_recolor_cursor
 */
static int
pv_bt463_recolor_cursor( closure, screen, fg, bg)
        caddr_t closure;
        ws_screen_descriptor *screen;
        ws_color_cell *fg, *bg;
{
        register struct bt463info *bti  = (struct bt463info *)closure;
        bti->cursor_fg = *fg;
        bti->cursor_bg = *bg;
        pv_bt463_restore_cursor_color(closure, 1);
        return 0;
}

/*
 * pv_bt463_restore_cursor_color
 */
static int
pv_bt463_restore_cursor_color(closure, sync)
        caddr_t closure;
	int sync;
{
        register struct bt463info *bti  = (struct bt463info *)closure;
        register volatile struct bt463 *btp     = bti->btaddr;

        bti->dirty_cursormap = 1;       /* cursor needs recoloring */
	if (sync) {
            if (bti->enable_interrupt) {
                (*bti->enable_interrupt)(closure);
		return (0);
	    }
	}
	pv_bt463_clean_cursormap( bti );
        return(0);
}

/*
 * pv_bt463_clean_cursormap
 */
static void
pv_bt463_clean_cursormap(closure)
        caddr_t closure;
{
        register struct bt463info *bti  = (struct bt463info *)closure;
        register volatile struct bt463 *btp     = bti->btaddr;
        register int s = PV_RAISE_SPL();

	btp->addr_low = (CURSOR_COLOR0 & ADDR_LOW_MASK);wbflush();
	btp->addr_high = (CURSOR_COLOR0 & ADDR_HIGH_MASK) >> 8;	wbflush();

	btp->bt_reg = bti->cursor_bg.red >> 8; 	wbflush(); /* Cursor location 0. */	
	btp->bt_reg = bti->cursor_bg.green >> 8; wbflush();
	btp->bt_reg = bti->cursor_bg.blue >> 8;	wbflush();

	btp->bt_reg = bti->cursor_fg.red >> 8; 	wbflush(); /*Cursor location 1. */	
	btp->bt_reg = bti->cursor_fg.green >> 8; wbflush();
	btp->bt_reg = bti->cursor_fg.blue >> 8; wbflush();

	btp->bt_reg = bti->cursor_bg.red >> 8;	wbflush(); /* Cursor location 2. */
	btp->bt_reg = bti->cursor_bg.green >> 8; wbflush();
	btp->bt_reg = bti->cursor_bg.blue >> 8; wbflush();

	btp->bt_reg = bti->cursor_bg.red >> 8; wbflush(); /* Cursor location 3. */	
	btp->bt_reg = bti->cursor_bg.green >> 8; wbflush();
	btp->bt_reg = bti->cursor_bg.blue >> 8; wbflush();
        PV_LOWER_SPL(s);
        bti->dirty_cursormap = 0;
}


int
pv_bt431_set_cursor_position(closure, sp, x, y)
	caddr_t closure;
	ws_screen_descriptor *sp;
	register int x, y;
{
	register struct bt431info *bti = (struct bt431info *)closure;
   	register volatile struct bt431 *btp = bti->btaddr;
	register volatile struct bt431 *btp2 = bti->btaddr2;
	register int xt, yt;
	pv_info *pvp = &pv_softc[bti->unit];
	register int s;

	s = PV_RAISE_SPL();
	if ( pvp->stereo_mode == PV_INFO_STEREO_24 ) {
	    y >>= 1;
	}
	yt = y + bti->fb_yoffset - bti->y_hot;
	xt = x + bti->fb_xoffset - bti->x_hot;
	btp->addr_low	= bt431_CUR_XLO; 	wbflush();
	btp->addr_high	= 0;			wbflush();
	btp->control	= (xt & 0xff);		wbflush();
	btp->control	= ((xt & 0xff00) >> 8);	wbflush();
	btp->control	= (yt & 0xff);		wbflush();
	btp->control	= ((yt & 0xff00) >> 8);	wbflush();
	btp2->addr_low	= (bt431_CUR_XLO); 	wbflush();
	btp2->addr_high	= 0;			wbflush();
	btp2->control	= (xt & 0xff);		wbflush();
	btp2->control	= ((xt & 0xff00) >> 8);	wbflush();
	btp2->control	= (yt & 0xff);		wbflush();
	btp2->control	= ((yt & 0xff00) >> 8);	wbflush();
	PV_LOWER_SPL(s);
	return(0);
}

/*
 * Halt GFPA making certain that we don't try to spin on a GFPA interrupt
 * in an invalidation thread.
 */
static void
halt_gfpa( pvp )
pv_info *pvp;
{
    pv_register_t
	*p_gcp,
	*p_gcp_register,
	*p_sr;

    pvInfo
	*p_info = (pvInfo *) pvp->p_info_area;

    p_info->gfpa_halts_on_countdown = 0;
    pvp->gfpa_is_running = 0;
    p_gcp = (pv_register_t *) PV_GCP_BASE( pvp );
    p_gcp_register = (pv_register_t *)( (vm_offset_t) p_gcp
				      + PV_GCP_HALT_OFFSET );
    p_sr = (pv_register_t *)( (vm_offset_t) p_gcp
			    + PV_GCP_SR_OFFSET );
    do {
	*p_gcp_register = 2;
	wbflush();
    } while ( *p_sr );

    return;
}

