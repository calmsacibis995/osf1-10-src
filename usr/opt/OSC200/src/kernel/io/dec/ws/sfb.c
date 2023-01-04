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
static char	*sccsid = "@(#)$RCSfile: sfb.c,v $ $Revision: 1.1.8.6 $ (DEC) $Date: 1994/01/20 18:33:47 $";
#endif 
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
 *                                                                      *
 *                      Copyright (c) 1990 by                           *
 *              Digital Equipment Corporation, Maynard, MA              *
 *                      All rights reserved.                            *
 *                                                                      *
 *   This software is furnished under a license and may be used and     *
 *   copied  only  in accordance with the terms of such license and     *
 *   with the  inclusion  of  the  above  copyright  notice.   This     *
 *   software  or  any  other copies thereof may not be provided or     *
 *   otherwise made available to any other person.  No title to and     *
 *   ownership of the software is hereby transferred.                   *
 *                                                                      *
 *   The information in this software is subject to change  without     *
 *   notice  and should not be construed as a commitment by Digital     *
 *   Equipment Corporation.                                             *
 *                                                                      *
 *   Digital assumes no responsibility for the use  or  reliability     *
 *   of its software on equipment which is not supplied by Digital.     *
 *                                                                      *
 ************************************************************************/
/*
 * Modification History
 *	31-Aug-90 --  Joel Gringorten  
 *
 *	Created from vfb03.
 */

#include <sys/types.h>
#include <io/common/devio.h>
#include <sys/param.h>
#include <sys/buf.h>
#include <sys/tty.h>
#include <io/common/devdriver.h>
#include <io/dec/tc/tc.h>
#include <sys/workstation.h>
#include <sys/inputdevice.h>
#include <sys/wsdevice.h>
#include <sys/uio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/kernel.h>
#include <hal/cpuconf.h>
#include <sys/exec.h>
#include <machine/cpu.h>
#include <sys/proc.h>
#include <sys/fbinfo.h>
#include <machine/pmap.h>
#include <machine/hal/kn15aa.h>
#include <io/dec/ws/pmagbba.h>
#include <io/dec/ws/sfbparams.h>
#include <io/dec/ws/sfbregs.h>
#include <io/dec/ws/bt459.h>
#include <io/dec/ws/bt463.h>

#ifdef notdef
#include <machine/hal/ka_flamingo.h>
#endif

#ifdef	__alpha
typedef	int	fb_entry_t;
#define BASE_FB_ADDRESS ( 0x410000000L )
#define	BASE_SFBP_ADDRESS	0x280000000L
#else	/* __alpha */
typedef	int	fb_entry_t;
#define BASE_FB_ADDRESS ( 128 * 1024 * 1024 )
#define	BASE_SFBP_ADDRESS	0x80000000
#endif	/* __alpha */

#define	WINDOW_TAG_COUNT		(BT463_WINDOW_TAG_COUNT - 2)

#ifndef PDEVCMD_DMA
#define	PDEVCMD_DMA			2
#endif

#if 0
#ifndef	PMAP_COPROC_INVALIDATE_STLB
#define PMAP_COPROC_INVALIDATE_STLB  0
#define PMAP_COPROC_EXIT             1
#endif
#endif

#define	DO_NEW_STYLE_INVALIDATE

/*
 * Save-state structure
 */
typedef struct {
    PixelWord   planemask;      /* Planemask (minimum 32 bits)		    */
    CommandWord	pixelmask;      /* Pixel mask register			    */
    SFBMode     mode;		/* Hardware mode			    */
    unsigned    rop;		/* Raster op for combining src, dst	    */
    Bits32	bres3;		/* e, count				    */
} SFBRegsRec, *SFBregs;

typedef struct {
    Bits32		pixelmask;
    SFBPlusMode		mode;
    SFBPlusRasterOp	rop;
    Bits32		bres3;
    Pixel32		address;
} SFBPlusRegsRec, *SFBPlusregs;

extern struct fb_info fb_softc[];
extern struct controller *fbinfo[];
extern struct sfbinfo sfb_softc[];
extern struct sfbplus_info sfbplus_softc[];
extern struct sfbplus_info sfbplus_type[];

extern vm_offset_t ws_heap_start;
extern vm_offset_t ws_heap_at;
extern vm_offset_t ws_heap_end;
extern size_t ws_heap_size;

extern caddr_t ws_map_region();

extern int nsfbp_types;
extern int nbt_softc2;

static SFBRegsRec regs;
static SFBPlusRegsRec plus_regs;

static void sfb_save_regs();
static void sfb_restore_regs();
static int sfb_map_framebuffer();
static int sfb_map_registers();
static void clean_window_tag( /* fbp, sfbp */ );
static int load_window_tag( /* fbp, entry */ );

static void sfbp_init();
static int sfbp_set_stereo_mode();
static void sfbp_bt459_interrupt();
static void sfbp_bt463_interrupt();
static void sfbp_bt459_enable_interrupt();
static void sfbp_bt463_enable_interrupt();
static void sfbp_curs_enable_interrupt();
static void sfbp_reformat_cursor( /* bits, cursor */ );
static int sfbp_invalidate_page_ref( /* func, va, data */ );
static int sfbp_recolor_cursor();
static int sfbp_bt459_video_on();
static int sfbp_bt459_video_off();
static int sfbp_curs_video_on();
static int sfbp_curs_video_off();
static caddr_t sfbp_bt459_init_closure();
static caddr_t sfbp_bt463_init_closure();
static caddr_t sfbp_curs_init_closure();
static int sfbp_load_cursor();
static int sfbp_set_cursor_position();
static int sfbp_cursor_on_off();
static int sfbp_bt459_2_set_cursor_position();

static SFBPlusMode sfbp_reset_mode_24 = {
	SFBP_MODE_MODE_SIMPLE,
	SFBP_MODE_VISUAL_24,
	0,
	0,
	0,
	0,
	0
};
static SFBPlusMode sfbp_reset_mode_8 = {
	SFBP_MODE_MODE_SIMPLE,
	SFBP_MODE_VISUAL_8_PACKED,
	0,
	0,
	0,
	0,
	0
};

static SFBPlusRasterOp sfbp_reset_rop_24 = {
	SFBP_ROP_OP_COPY,
	0,
	SFBP_ROP_VISUAL_24,
	0
};
static SFBPlusRasterOp sfbp_reset_rop_8 = {
	SFBP_ROP_OP_COPY,
	0,
	SFBP_ROP_VISUAL_8_PACKED,
	0
};

void sfb_bot(fbp)
       register struct fb_info *fbp;
{
	union {
		struct {
			unsigned pixels:9;
			unsigned front_porch:5;
			unsigned sync:7;
			unsigned back_porch:7;
		} horizontal_setup;
		unsigned h_setup;
	} hu;
	union {
		struct {
			unsigned scan_lines:11;
			unsigned front_porch:5;
			unsigned sync:6;
			unsigned back_porch:6;
		} vertical_setup;
		unsigned v_setup;
	} vu;
	SFB sfb = (SFB)(fbp->depth[0].physaddr - 
	    PMAGBBA_FB_OFFSET + PMAGBBA_SFB_ASIC_OFFSET);
       register struct bt459info *btp = (struct bt459info *)fbp->cmf.cmc;

/* 
   XXX
   should be code to save sfb state at attach, and we should have
   a close proc that resets the state, so if the sfb gets clobbered,
   you don't necessarily have to reboot...
*/
	fbp->depth[0].fb_width = fbp->screen.width = 
	    (sfb->horizontal_setup & 0777) << 2;
	fbp->depth[0].fb_height = fbp->screen.height = 
	    sfb->vertical_setup & 03777;

	fbp->screen.max_row = fbp->screen.height / fbp->screen.f_height - 1;

/*
 * offset the frame buffer start by one page, to make copy
 * code not have to worry about accessing before the beginning of the
 * first line.
 */
	sfb->base_address = 1;				wbflush();

#define HORZMAGIC 24
#define VERTMAGIC 1

/*
 * now compute offsets for BT459, depending on screen resolution.
 */
	hu.h_setup = (unsigned) sfb->horizontal_setup;
	vu.v_setup = (unsigned) sfb->vertical_setup;
	btp->fb_xoffset = 4 * 
	     (hu.horizontal_setup.sync +  hu.horizontal_setup.back_porch)
	       - HORZMAGIC;
	btp->fb_yoffset = vu.vertical_setup.front_porch + 
	      vu.vertical_setup.sync + vu.vertical_setup.back_porch 
		- VERTMAGIC;
}

int
sfb_attach(ctlr)
        register struct controller *ctlr;
{
        register struct fb_info *fbp = &fb_softc[ctlr->ctlr_num];
        register struct bt459info *bti_459 = (struct bt459info *)fbp->cmf.cmc;
	SFB sfb = (SFB) (ctlr->addr + PMAGBBA_SFB_ASIC_OFFSET);

	sfb_bot(fbp);

        /*
	 * We know we have a bt459
	 */
        bti_459->enable_interrupt = sfb_enable_interrupt;
	sfb_softc[ctlr->ctlr_num].depth			= ~sfb->depth;
	sfb_softc[ctlr->ctlr_num].refresh_count		= sfb->refresh_count;
	sfb_softc[ctlr->ctlr_num].horizontal_setup	= sfb->horizontal_setup;
	sfb_softc[ctlr->ctlr_num].vertical_setup	= sfb->vertical_setup;
	sfb_softc[ctlr->ctlr_num].base_address		= sfb->base_address;

	/*
	 * Done
	 */
	return (0);
}

void
sfb_close(closure)
	caddr_t closure;
{
	register struct fb_info *fbp = (struct fb_info *)closure;
	register int unit = fbp - fb_softc;
	SFB sfb = (SFB)(fbp->depth[0].physaddr - 
	    PMAGBBA_FB_OFFSET + PMAGBBA_SFB_ASIC_OFFSET);

	sfb->video_valid	= 0;				    wbflush();
	sfb->depth		= sfb_softc[unit].depth;	    wbflush();
	sfb->refresh_count 	= sfb_softc[unit].refresh_count;    wbflush();
	sfb->horizontal_setup 	= sfb_softc[unit].horizontal_setup; wbflush();
	sfb->vertical_setup 	= sfb_softc[unit].vertical_setup;   wbflush();
	sfb->base_address 	= sfb_softc[unit].base_address;	    wbflush();
	sfb->planemask		= ~0;				    wbflush();
	sfb->pixelmask		= ~0;				    wbflush();
	sfb->mode		= 0;				    wbflush();
	sfb->rop		= 3;				    wbflush();
	sfb->video_valid	= 1;				    wbflush();
}

void
sfb_interrupt(ctlr, closure)
        register struct controller *ctlr;
        caddr_t closure;
{
        struct fb_info *fbp = (struct fb_info *) closure;
        struct bt459info *bti_459 = (struct bt459info *)fbp->cmf.cmc;
	SFB sfb = (SFB) (ctlr->addr + PMAGBBA_SFB_ASIC_OFFSET);

	sfb->clear_interrupt = 0;				wbflush();
        if (bti_459->dirty_cursor) bt_load_formatted_cursor(bti_459);
        if (bti_459->dirty_colormap) bt_clean_colormap(bti_459);
	sfb->enable_disable_interrupt = DISABLE_INTERRUPTS;	wbflush();
}

void
sfb_enable_interrupt(closure)
        caddr_t closure;
{
        register struct bt459info *bti = (struct bt459info *) closure;
        register int unit = bti->unit;
        register struct controller *ctlr = fbinfo[unit];
	SFB sfb = (SFB) (ctlr->addr + PMAGBBA_SFB_ASIC_OFFSET);
	int s;

	s = splhigh();
	tc_enable_option(fbinfo[unit]);
	sfb->clear_interrupt = 0;				wbflush();
	sfb->enable_disable_interrupt = ENABLE_INTERRUPTS;	wbflush();
	splx(s);
}

static void
sfb_save_regs(sfb)
	SFB sfb;
{
	unsigned data;

	/* saving the regs is a bit funky, to make asic simpler */
	regs.planemask	= sfb->planemask;
	regs.pixelmask 	= ~sfb->pixelmask;
	regs.mode       = sfb->mode;
	regs.rop 	= ~(sfb->rop) & 0xf;
	data		= sfb->bres3;
	regs.bres3     	= data & 0xf;
	regs.bres3	|= ((data & 0x1ffff0) << 11);
	/* set to dumb frame buffer state */
	sfb->planemask	= ~0;					wbflush();
	sfb->pixelmask	= ~0;					wbflush();
	sfb->mode	= 0;					wbflush();
	sfb->rop	= 3;					wbflush();
}

static void
sfb_restore_regs(sfb)
	SFB sfb;
{
	/* now restore state to starting point */
	sfb->planemask	= regs.planemask;			wbflush();
	sfb->pixelmask	= regs.pixelmask;			wbflush();
	sfb->mode	= regs.mode;				wbflush();
	sfb->rop	= regs.rop;				wbflush();
	sfb->bres3 	= regs.bres3;				wbflush();
}

int
sfb_clear_screen(closure, screen)
	caddr_t closure;
	ws_screen_descriptor *screen;
{
	register struct fb_info *fbp = (struct fb_info *)closure;
	SFB sfb = (SFB)(fbp->depth[0].physaddr - 
	    PMAGBBA_FB_OFFSET + PMAGBBA_SFB_ASIC_OFFSET);

	sfb_save_regs(sfb);
	fbp->depth[0].physaddr += NBPG / 2;
	fb_clear_screen(closure, screen);
	fbp->depth[0].physaddr -= NBPG / 2;
	sfb_restore_regs(sfb);

}

int
sfb_scroll_screen(closure, screen)
	caddr_t closure;
	ws_screen_descriptor *screen;
{
	register struct fb_info *fbp = (struct fb_info *)closure;
	SFB sfb = (SFB)(fbp->depth[0].physaddr - 
	    PMAGBBA_FB_OFFSET + PMAGBBA_SFB_ASIC_OFFSET);

	sfb_save_regs(sfb);
	fbp->depth[0].physaddr += NBPG / 2;
	fb_scroll_screen(closure, screen);
	fbp->depth[0].physaddr -= NBPG / 2;
	sfb_restore_regs(sfb);
}

int
sfb_blitc(closure, screen, row, col, c)
	caddr_t closure;
	ws_screen_descriptor *screen;
	int row, col;
	u_char c;
{
	register struct fb_info *fbp = (struct fb_info *)closure;
	SFB sfb = (SFB)(fbp->depth[0].physaddr - 
	    PMAGBBA_FB_OFFSET + PMAGBBA_SFB_ASIC_OFFSET);

	sfb_save_regs(sfb);
	fbp->depth[0].physaddr += NBPG / 2;
	fb_blitc(closure, screen, row, col, c);
	fbp->depth[0].physaddr -= NBPG / 2;
	sfb_restore_regs(sfb);
}

/*
 * map a region of memory.  If the function succeeds, it returns the address
 * where it is visible to the user (the pages are double mapped, so that
 * physical memory can be accessed, for example framebuffers or I/O registers.)
 */

#define DWN(x) ((long)(x) & ~(CLBYTES-1))
static caddr_t sfb_map_region (addr, virtaddr, nbytes, how)
	caddr_t addr;
	caddr_t virtaddr;
	register int nbytes;
	int how;
{
/* This routine was used under Ultrix. Not used any more. Make it a dummy
   routine for compiling purpose. It will be removed when sfb_ioctl will be
   ported to OSF/1 

	register caddr_t result;
	register long tmp1;
	register int smid;
	register long start = DWN(addr);
	register long end = (long)addr + nbytes;
	register long size;
	char *smat();
	size = DWN(end + CLBYTES) - start;

	if ((smid = vm_system_smget(start, size, how)) < 0)
		goto bad;
	if ((result = smat(smid, virtaddr, 0)) == (caddr_t) -1)
		goto bad;
	if (smctl(smid, IPC_RMID, 0))
		goto bad;

	/* the following returns the low order bits to the mapped result. * /
	tmp1 = (long)result;
	tmp1 |= (long) addr & (CLBYTES-1);
	return ((caddr_t) tmp1);
bad:	
	cprintf ("sfbmap: cannot map shared data structures\n");
	cprintf ("u_error = %d\n", u.u_error);
*/
	return (NULL);
}

static sfb_unmap_region(vaddr)
caddr_t vaddr;
{

/* This routine was used under Ultrix. Not used any more. Make it a dummy
   routine for compiling purpose. It will be removed when sfb_ioctl will be
   ported to OSF/1 
	if((long)vaddr & NBPG - 1 || (long)vaddr <= 0 ||
		smdt(vaddr, SHMT_SHM) < 0)
		return(SFB_ERROR);
	return(SFB_SUCCESS);
*/
	return(SFB_ERROR);
}

static ws_depth_descriptor *dp;

/*
 * the following is a gross kludge that should be removed for an
 * OSF/1 based kernel; there should be routines to return the TURBOchannel
 * slot size available there.
 */
extern int cpu;

int
sfb_map_unmap_screen(closure, depths, screen, mp)
	caddr_t closure;
	ws_depth_descriptor *depths;
	ws_screen_descriptor *screen;
	ws_map_control *mp;
{
	register struct fb_info *fbp = (struct fb_info *) closure;
	int nbytes;
	register int unit = fbp - fb_softc;
	register struct controller *ctlr = fbinfo[unit];
  	SFB sfb = (SFB)(ctlr->addr + PMAGBBA_SFB_ASIC_OFFSET);
	caddr_t virtaddr;
	vm_offset_t temp;
	register unsigned long i;

	dp = depths + mp->which_depth;

	/* unmap not yet (if ever) implemented) */
	if (mp->map_unmap == UNMAP_SCREEN) return (EINVAL);

	/*
	 * We need to map the ASIC...  64K bytes long; return SFB
	 * address in plane mask field of structure.  Cheat and return
	 * the asic address in the plane_mask_phys field.
	 * XXX should define new field for returning reg pointer or something
	 */
	dp->plane_mask_phys = (caddr_t) sfb;
	if ((dp->plane_mask_phys =
	     ws_map_region(dp->plane_mask_phys, NULL, 0x10000, 
			   0600, (int *) NULL)) == NULL)
		return ENOMEM;

#ifndef __alpha
	/* return the correct plane mask address */
	dp->plane_mask = (caddr_t) &((SFB)dp->plane_mask_phys)->planemask;
#endif /* !__alpha */

/*
 * XXX add code here to map the amount of memory you really need; right
 * now it just maps the screen itself.  Note the one page offset.
 * For OSF/1, clean up the CPU dependency to key off of slot
 * size, rather than using CPU type, which is a crock.
 *
 */
	nbytes = ( 2 * 1024 * 1024 * sizeof( fb_entry_t ) >> 2 ) 
	       + NBPG;

#ifndef	__alpha
	/* should be "if (slotsize(unit) >= 8 meg) {" */
	if (cpu != DS_5000) {
	    nbytes += 2 * 1024 * 1024;
	}
#endif

	/* Make the alignment to 16M boundary */
        virtaddr = (caddr_t)( BASE_FB_ADDRESS
			    - (0x1000000 * (screen->screen + 1)));

	/* SPARSE space mapping, currently unused by X server */
	/* Map the frame buffer */
	if ((dp->pixmap = ws_map_region(dp->physaddr, virtaddr, nbytes,
					0600, (int *) NULL)) == NULL)
			return(ENOMEM);

	dp->pixmap += NBPG;

#if defined(__alpha)
	/* Some notes. See the notes for the hx code in the X server
 	 * area of the X pool (server/ddx/dec/sfb) for more info.
	 * We want to alias the frame buffer so we don't have to 
	 * do as many mb's to flush the write buffer.
	 * At this point, this code only works for a first pass
	 * hx, not the hx+(sfb+). Currently, we pass back one
	 * pointer and the server and driver just "agree" on
	 * what the aliases will be. We need some mechanism to ensure
	 * the mapping matches, since the hx+ will need different
	 * mappings depending on the configuration of the board.
	 * For now, map 8 aliases, 4M apart. Map 4M for each so we
	 * only use one 4M pte per alias, instead of 4 512K pte's
	 * for each (total of 8 instead of 32 so we have room for
	 * other pte's). Note that we just allocate 4M. In the future,
	 * if there is a need to allocate more than 4M, then the
	 * adjustment of nbytes below must be changed appropriately.
	 * (see notes on alpha granularity hints to determine which
	 * pte sizes are available. pmap.c and ws_device.c will
	 * return the biggest pte for the requested size, so try to
	 * request at least 4M or multiples of 4M when possible)
	 * Note that the first fb is at physical address 2M offset
	 * on the board, so we need to map 2M before the fb with the
	 * 2M of the fb to get a 4M aligned space to map with a 4M pte.
	 * Note that the built in hx of the flamingo is only mapped
	 * in 32M so you can only alias 8 times at 4M instead of
	 * what would be a more generic (to provide for hx+) 8 times
	 * at 32M offsets.
	 * The address alignment of the first frame buffer must be
	 * at a 64M boundary (2 times the area we cover, 8 * 4M) so
	 * that we can use a very simple equation to calculate the
	 * address cycling -- we need the extra bit cleared.
	 */
#define FB_INTERVAL 	0x400000L	/* current interval between mappings */
#define FB_NUM_MAPPINGS	8		/* number of aliases  */
#define FB_ALIGNMENT   	0x4000000L	/* alignment of first buffer */
		/* Must be twice the amount of space for the macros to work */
#define FB_ISNOTALIGNED(p) ((unsigned long)(p) & 0x3FFFFFFL)
#define HX_FB_OFFSET 2 * 1024 * 1024	/* offset of fb from start of
					 * physical memory on board
					 */


        virtaddr = (caddr_t)( BASE_FB_ADDRESS
			    + (FB_ALIGNMENT * screen->screen));

	/* Map the frame buffer */
	temp = (vm_offset_t)dp->physaddr - PMAGBBA_FB_OFFSET;
				/* sparse base KVA */

	temp += PMAGBBA_DENSE_FB_OFFSET;
			/* DENSE fb KVA */
	
	/* Request 4M space mapping, but only use upper 2M */
	nbytes = 4 * 1024 * 1024;

	if ((dp->plane_mask = ws_map_region(
				temp-HX_FB_OFFSET, virtaddr, 
				nbytes, 0600, (int *) NULL)) == NULL)
			return(ENOMEM);
	if ( FB_ISNOTALIGNED(virtaddr) ) {
	    printf(
		"Base address for hx screen %d is not aligned properly\n",
		screen->screen);
	    printf("X server will most likely crash as a result.\n");
	    return(ENOMEM);
	}
        dp->plane_mask += HX_FB_OFFSET;


        for ( 	i = FB_INTERVAL; 
		i < FB_INTERVAL*FB_NUM_MAPPINGS; 
		i += FB_INTERVAL ) {
	    	caddr_t tmp;
	    	if ((tmp = ws_map_region(
				temp+i-HX_FB_OFFSET, virtaddr+i, 
				nbytes, 0600, (int *) NULL)) == NULL)
			return(ENOMEM);
		if ( tmp != (virtaddr+i) ) {
		    printf("Aliased frame buffer address for screen %d\n",
			screen->screen);
		    printf("is not properly aligned.\n");
		    printf("X server will most likely crash as a result\n");
	    	    return(ENOMEM);
		}
	}

	dp->plane_mask += (NBPG >> 1);
#endif /* __alpha */

	return (0);
}

/* 
 * Performs mapping/unmapping of system I/O space, representing 
 * CXTurbo option, to user process address space.  
 */

/* ARGSUSED */
sfb_ioctl(closure, cmd, data, flag)
caddr_t closure;
unsigned int cmd;
caddr_t data;
int flag;
{
	register struct fb_info *fbp = (struct fb_info *)closure;
        register int unit = fbp - fb_softc;
        register struct controller *ctlr = fbinfo[unit];
	ws_prom_map *wsp = (ws_prom_map *)data;
	caddr_t kseg1_addr = ctlr->addr;
	extern caddr_t sfb_map_region();
	extern sfb_unmap_region();

#if     SEC_BASE
            if (!privileged(SEC_SYSATTR, 0))
#else
            if (suser(u.u_cred, &u.u_acflag))
#endif
		return(EACCES);

	switch(wsp->cmd) {

#ifndef	__alpha
        case CXT_MAP_PROM: 

		if((int)wsp->pm_size <= 0 || (int)wsp->pm_offset < 0)
			return(EINVAL);
		if(wsp->pm_size + wsp->pm_offset > PMAGBBA_MAX_PROM_RGN)
			return(ENOMEM);
		kseg1_addr += wsp->pm_offset;

		/* args aligned in sfb_map_region */



		if((wsp->pm_vaddr = sfb_map_region (kseg1_addr, (char *)0,
			(int)wsp->pm_size, 0600)) == (caddr_t)NULL) 
			return(ENOMEM);
		wsp->pm_svaddr = kseg1_addr;
		return(SFB_SUCCESS);

        case CXT_UNMAP_PROM: 

		if(sfb_unmap_region(wsp->pm_vaddr) < 0) 
			return(EINVAL);
		wsp->pm_svaddr = wsp->pm_vaddr = (caddr_t)NULL;
		return(SFB_SUCCESS);
#endif

	default:
		printf("Ioctl command unrecognized.\n");
		return(EINVAL);
	}
}


/*
 * SFB+
 *
 * SFB+ specific code follows
 */

/*
 * sfbp_dummy_caddrt/sfbp_dummy_int
 *
 * Two dummy routines used to initialize some pointers in the fb_type
 * array.  They are copied then overwritten later once we know what type
 * of PMAGD device we have.
 */
caddr_t sfbp_dummy_caddrt()
{
  printf( "sfb+: dummy caddrt routine called\n" );

  return ((caddr_t) NULL);
}

int sfbp_dummy_int()
{
  printf( "sfb+: dummy int routine called\n" );

  return (0);
}

/*
 * sfbp_bot
 * 
 * beginning-of-time function.  Just fix up the interrupt enable for now.
 */
void sfbp_bot(fbp)
       register struct fb_info *fbp;
{
    register int unit = fbp - fb_softc;
    register struct sfbplus_info *sfbp = &sfbplus_softc[unit];
    register SFBPlus sp;
    register struct bt459info2 *btp = (struct bt459info2 *) fbp->cf.cc;
    register struct sfbp_curs_info *curp = (struct sfbp_curs_info *) fbp->cf.cc;
    union {
	struct {
	    unsigned pixels:9;
	    unsigned front_porch:5;
	    unsigned sync:7;
	    unsigned back_porch:7;
	    unsigned ignore:3;
	    unsigned odd:1;
	} horizontal_setup;
	unsigned int h_setup;
    } hu;
    union {
	struct {
	    unsigned scan_lines:11;
	    unsigned front_porch:5;
	    unsigned sync:6;
	    unsigned back_porch:6;
	} vertical_setup;
	unsigned int v_setup;
    } vu;

    sp = sfbp->asic;
    hu.h_setup = (unsigned int) sp->horizontal_setup;
    vu.v_setup = (unsigned int) sp->vertical_setup;

    fbp->depth[0].fb_width = fbp->screen.width = 
	( hu.horizontal_setup.pixels << 2 );
    /* Disable display of last 4 pixels in case scanline width mod 8 = 4,
      and make sure that displayed portion of scanline mod 8 = 0, regardless
      of what ROM passed in. */
    hu.horizontal_setup.odd = 1;
    sp->horizontal_setup = hu.h_setup;
    fbp->screen.width -= fbp->screen.width % 8;

    fbp->depth[0].fb_height = fbp->screen.height = 
	vu.vertical_setup.scan_lines;

    fbp->screen.max_row = fbp->screen.height / fbp->screen.f_height - 1;

#define SFBP_HORZMAGIC 24
#define SFBP_VERTMAGIC 1

    /*
     * now compute offsets for BT459, depending on screen resolution.
     */
    if ( !sfbp->bt463_present ) {
	/*
	 * Contains a 459
	 */
	btp->fb_xoffset = 4 * 
	     (hu.horizontal_setup.sync +  hu.horizontal_setup.back_porch)
	       - SFBP_HORZMAGIC;
	btp->fb_yoffset = vu.vertical_setup.front_porch + 
	      vu.vertical_setup.sync + vu.vertical_setup.back_porch 
		- SFBP_VERTMAGIC;
    }
    else {
      	/*
	 * 463
	 */
	curp->fb_xoffset = 4 * ( hu.horizontal_setup.sync
			       + hu.horizontal_setup.back_porch );
	curp->fb_yoffset = vu.vertical_setup.front_porch
	  		 + vu.vertical_setup.sync
			 + vu.vertical_setup.back_porch
			 - 3;
    }

    return;
}

/*
 * sfbp_init
 *
 * Minor device initialization.
 */
static void
sfbp_init( sfbp )
     struct sfbplus_info *sfbp;
{
    int
	unit = sfbp - sfbplus_softc,
	i;

    SFBPlus
	sp = sfbp->asic;

    static  Bt463_Wid_Cell  wids[BT463_WINDOW_TAG_COUNT] = {
      /* Window id values to load: */
      /* Low   Mid   High  xxxx */
      /* 0-7   8-15  16-23	*/
	    {0x10, 0xe3, 0x01, 0x00}, /* 0: 8-plane cmap 0 buffer 0 overlay */
	    {0x08, 0xe3, 0x01, 0x00}, /* 1: 8-plane cmap 0 buffer 1 overlay */
	    {0x00, 0xe3, 0x01, 0x00}, /* 2: 8-plane cmap 0 buffer 2 overlay */
	    {0x00, 0xe1, 0x01, 0x00}, /* 3: 24-plane cmap 0 overlay */
	    {0x10, 0xe3, 0x21, 0x00}, /* 4: 8-plane cmap 1 buffer 0 overlay */
	    {0x08, 0xe3, 0x21, 0x00}, /* 5: 8-plane cmap 1 buffer 1 overlay */
	    {0x00, 0xe3, 0x21, 0x00}, /* 6: 8-plane cmap 1 buffer 2 overlay */
	    {0x80, 0xe0, 0x21, 0x00}, /* 7: 24-plane cmap 1 overlay */
	    {0x84, 0xe0, 0x01, 0x00}, /* 8: 12-plane cmap 0 buffer 0 overlay */
	    {0x80, 0xe0, 0x01, 0x00}, /* 9: 12-plane cmap 0 buffer 1 overlay */
	    {0x84, 0xe0, 0x21, 0x00}, /* a: 12-plane cmap 1 buffer 0 overlay */
	    {0x80, 0xe0, 0x21, 0x00}, /* b: 12-plane cmap 1 buffer 1 overlay */
	    {0x00, 0xe1, 0x81, 0x00}, /* c: 24-plane truecolor overlay */
#define     FFB_TRUECOLOR_WID 0xC0000000
	    {0x00, 0x01, 0x80, 0x00}, /* ... D */
	    {0x00, 0x00, 0x00, 0x00}, /* e: cursor 0 color */
	    {0xff, 0xff, 0xff, 0x00}, /* f: cursor 1 color */
    };

    /*
     * offset the frame buffer start by 4K pixels, to make copy
     * code not have to worry about accessing before the beginning of the
     * first line.
     */
    sp->base_address = sfbp->base_address;			wbflush();

    sp->intr_status = SFBP_INTR_ALL
		    | ( ( /*|||SFBP_INTR_PARITY_ERROR |*/ SFBP_INTR_DMA_ERROR )
		      << SFBP_INTR_ENABLE_SHIFT );
    wbflush();

    sfbp_set_stereo_mode( sfbp, SFBP_IOC_STEREO_NONE );

    if ( sfbp->bt463_present ) {
	for (i = 0; i < BT463_WINDOW_TAG_COUNT; i++ ) {
	    sfbp->wt_cell[i].windex = i;
	    sfbp->wt_cell[i].low = wids[i].low_byte;
	    sfbp->wt_cell[i].mid = wids[i].middle_byte;
	    sfbp->wt_cell[i].high = wids[i].high_byte;
	}
	sfbp->wt_min_dirty = 0;
	sfbp->wt_max_dirty = BT463_WINDOW_TAG_COUNT-3;
	sfbp->wt_dirty = 1;

	sfbp_bt463_enable_interrupt( fb_softc[unit].cmf.cmc );
    }

    return;
}

/*
 * sfbp_set_stereo_mode
 *
 */
static int
sfbp_set_stereo_mode( sfbp, mode )
     struct sfbplus_info *sfbp;
     int mode;
{
  int
    status = 0;

  unsigned int
    scanlines;

  Bits32
    vert_setup;

  if ( mode == sfbp->stereo_mode ) {
    return (status);
  }
  switch ( mode ) {

  case SFBP_IOC_STEREO_NONE:
    vert_setup = sfbp->asic->vertical_setup;
    scanlines = ( ( vert_setup & 0x07ff ) << 1 );
    vert_setup &= 0xfffff800;
    vert_setup &= ~SFBP_VERT_STEREO_EN;
    vert_setup |= scanlines;
    sfbp->asic->vertical_setup = vert_setup;
    wbflush();
    sfbp->stereo_mode = SFBP_IOC_STEREO_NONE;
    break;

  case SFBP_IOC_STEREO_24:
    vert_setup = sfbp->asic->vertical_setup;
    scanlines = ( ( vert_setup & 0x07ff ) >> 1 );
    vert_setup &= 0xfffff800;
    vert_setup |= ( SFBP_VERT_STEREO_EN | scanlines );
    sfbp->asic->vertical_setup = vert_setup;
    wbflush();
    sfbp->stereo_mode = SFBP_IOC_STEREO_24;
    break;

  default:
    status = EINVAL;
  }

  return (status);
}


/*
 * sfbp_attach
 */
int
sfbp_attach(ctlr)
        register struct controller *ctlr;
{
    register struct fb_info *fbp = &fb_softc[ctlr->ctlr_num];
    register struct sfbplus_info *sfbp = &sfbplus_softc[ctlr->ctlr_num];
    register SFBPlus sp = sfbp->asic;
    
    /*
     * Do bot
     */
    sfbp_bot(fbp);

    /*
     * Save initial configuration for later restoration
     */
    sfbp->depth			= sp->depth;
    sfbp->horizontal_setup	= sp->horizontal_setup;
    sfbp->vertical_setup	= sp->vertical_setup;

    /*
     * See if parity is supported
     */
    if ( sp->start > 1 ) {
	/*
	 * Reading the start register returns the chip revision.
	 * Pass 2 and later will do parity correctly so it is safe
	 * to call tc_option_control.  Pass 1 doesn't so disable
	 * parity checks.
	 */
	sfbp->do_parity_check = ( tc_option_control( ctlr, SLOT_PARITY )
				== SLOT_PARITY );
    }
    else {
	sfbp->do_parity_check = 0;
    }

    /*
     * Interrupt-level init.
     */
    sfbp_init( sfbp );

    /*
     * done
     */
    return (0);
}

/*
 * sfbp_close
 *
 * Last close
 */
void
sfbp_close(closure)
	caddr_t closure;
{
    register struct fb_info *fbp = (struct fb_info *)closure;
    register int unit = fbp - fb_softc;
    register struct sfbplus_info *sfbp = &sfbplus_softc[unit];
    register SFBPlus sp = sfbp->asic;
    Bits32 status, oldmode;
    int s;

    wbflush();
    status = sp->command_status;
    while ( status & 0x01 ) {
	status = ( sp->command_status );
    }

    s = spltty();

    /*
     * Interrupt-level init.
     */
    sfbp_init( sfbp );

    sp->video_valid	= 3;				wbflush();
    sp->depth		= sfbp->depth;	    		wbflush();
    sp->horizontal_setup = sfbp->horizontal_setup;	wbflush();
    sp->vertical_setup 	= sfbp->vertical_setup;		wbflush();
    if (!( sfbp->bits_per_pixel == 32 ) ) {
	sp->mode	= sfbp_reset_mode_8;		wbflush();
	sp->rop		= sfbp_reset_rop_8;		wbflush();
	oldmode		= * (Bits32 *) &sfbp_reset_mode_8;
    }
    else {
	sp->mode	= sfbp_reset_mode_24;		wbflush();
	sp->rop		= sfbp_reset_rop_24;		wbflush();
	oldmode		= * (Bits32 *) &sfbp_reset_mode_24;
    }
    sp->planemask	= ~0;				wbflush();
    if ( oldmode & SFBP_MODE_PM_IS_PERS ) {
      sp->pers_pixelmask = ~0;				wbflush();
    }
    else {
      sp->pixelmask	= ~0;				wbflush();
    }
    sp->base_address = sfbp->base_address;		wbflush();
    sp->video_valid	= 1;				wbflush();

    splx(s);

    /*
     * done
     */
    return;
}

/*
 * sfbp_interrupt
 */
void
sfbp_interrupt( ctlr, closure )
     struct controller *ctlr;
     caddr_t closure;
{
  struct fb_info *fbp = (struct fb_info *) closure;
  int unit = fbp - fb_softc;

  if ( sfbplus_softc[unit].bt463_present ) {
    sfbp_bt463_interrupt( ctlr, closure );
  }
  else {
    sfbp_bt459_interrupt( ctlr, closure );
  }
}


/*
 * sfbp_bt459_interrupt
 *
 * ISR for bt459-bearing options
 */
unsigned long bt459_intr_cursor, bt459_intr_vsync;

static void
sfbp_bt459_interrupt(ctlr, closure)
        register struct controller *ctlr;
        caddr_t closure;
{
    struct fb_info *fbp = (struct fb_info *) closure;
    register struct bt459info2 *bti = (struct bt459info2 *) fbp->cmf.cmc;
    register int unit = fbp - fb_softc;
    register struct sfbplus_info *sfbp = &sfbplus_softc[unit];
    register SFBPlus sp = sfbp->asic;
    Bits32 intr_status;

    intr_status = sp->intr_status;

    if ( intr_status & SFBP_INTR_DMA_ERROR ) {
	/*
	 * DMA error on module...
	 * Print message, clear the status, but leave it enabled.
	 */
	printf( "sfb+: DMA error on fb%d.  Continuing.\n", unit );
	intr_status &= ~SFBP_INTR_DMA_ERROR;
    }
    if ( sfbp->do_parity_check && ( intr_status & SFBP_INTR_PARITY_ERROR ) ) {
	/*
	 * Parity error on module...
	 * Print message, clear the status, but leave it enabled.
	 */
	printf( "sfb+: Parity error on fb%d.  Continuing.\n", unit );
	intr_status &= ~SFBP_INTR_PARITY_ERROR;
    }
    if ( intr_status & SFBP_INTR_VSYNC ) {
	/*
	 * Vsync
	 * Do processing, clear the status, disable interrupt.
	 */
        if (bti->dirty_cursor) {
	    bt459_2_load_formatted_cursor(bti);
	    bt459_intr_cursor++;
	}
        if (bti->dirty_colormap) {
	    bt459_2_clean_colormap(bti);
	    bt459_intr_vsync++;
	}
	intr_status &= ~( ( SFBP_INTR_VSYNC << SFBP_INTR_ENABLE_SHIFT )
			| SFBP_INTR_VSYNC );
    }

    sp->intr_status = intr_status;				wbflush();

    return;
}

/*
 * sfbp_bt463_interrupt
 *
 * ISR for bt463-bearing options
 */
static void
sfbp_bt463_interrupt(ctlr, closure)
        register struct controller *ctlr;
        caddr_t closure;
{
    struct fb_info *fbp = (struct fb_info *) closure;
    register struct bt463info2 *bti = (struct bt463info2 *) fbp->cmf.cmc;
    register struct sfbp_curs_info *btii = (struct sfbp_curs_info *) fbp->cf.cc;
    register int unit = fbp - fb_softc;
    register struct sfbplus_info *sfbp = &sfbplus_softc[unit];
    register SFBPlus sp = sfbp->asic;
    Bits32 intr_status, video_valid;

    intr_status = sp->intr_status;

    if ( intr_status & SFBP_INTR_DMA_ERROR ) {
	/*
	 * DMA error on module...
	 * Print message, clear the status, but leave it enabled.
	 */
	printf( "sfb+: DMA error on fb%d.  Continuing.\n", unit );
	intr_status &= ~SFBP_INTR_DMA_ERROR;
    }
    if ( sfbp->do_parity_check && ( intr_status & SFBP_INTR_PARITY_ERROR ) ) {
	/*
	 * Parity error on module...
	 * Print message, clear the status, but leave it enabled.
	 */
	printf( "sfb+: Parity error on fb%d.  Continuing.\n", unit );
	intr_status &= ~SFBP_INTR_PARITY_ERROR;
    }
    if ( intr_status & SFBP_INTR_VSYNC ) {
	/*
	 * Vsync
	 * Do processing, clear the status, disable interrupt.
	 */
	if ( sfbp->wt_dirty ) {
	    video_valid = sp->video_valid;
	    sp->video_valid = ( video_valid | 0x02 );
	    wbflush();
	    clean_window_tag( fbp, sfbp );
	    sp->video_valid = video_valid;
	    wbflush();
	}
        if ( bti->dirty_colormap ) {
	    bt463_2_clean_colormap( bti );
	}
	if ( bti->dirty_cursormap ) {
	    bt463_2_clean_cursormap( bti );
	}
	if ( btii->dirty_cursor ) {
	  sfbp_load_formatted_cursor( btii );
	}
	intr_status &= ~( ( SFBP_INTR_VSYNC << SFBP_INTR_ENABLE_SHIFT )
			| SFBP_INTR_VSYNC );
    }

    sp->intr_status = intr_status;				wbflush();

    return;
}

/*
 * sfbp_save_regs
 *
 * Save SFB+ registers while doing console operations and force it to
 * dumb framebuffer mode to use standard frame buffer console routines.
 */
static void
sfbp_save_regs( sfbp, sp )
	struct sfbplus_info *sfbp;
	SFBPlus sp;
{
    unsigned int data;
    int i;

    /*
     * Wait for device to go quiet
     */
    if ( sp->command_status & 0x01 ) {
	for ( i = 0; i < 10000; i++ ) {
	    if (!( sp->command_status & 0x01 ) ) {
		break;
	    }
	    DELAY( 10 );
	}
    }

    /*
     * saving the regs is a bit funky, to make asic simpler
     */
    plus_regs.pixelmask 	= sp->pixelmask;
    plus_regs.mode		= sp->mode;
    plus_regs.rop		= sp->rop;
    plus_regs.address		= sp->address;

    /*
     * set to dumb frame buffer state
     */
    if (!( sfbp->bits_per_pixel == 32 ) ) {
	sp->mode	= sfbp_reset_mode_8;		wbflush();
	sp->rop		= sfbp_reset_rop_8;		wbflush();
    }
    else {
	sp->mode	= sfbp_reset_mode_24;		wbflush();
	sp->rop		= sfbp_reset_rop_24;		wbflush();
    }
    sp->planemask	= ~0;				wbflush();
    sp->pixelmask	= ~0;				wbflush();

    return;
}

/*
 * sfbp_restore_regs
 *
 * Make some determinations based on saved mode state bits, others on
 * current state bits.  Don't really know what to do yet....
 */
static void
sfbp_restore_regs( sfbp, sp )
	struct sfbplus_info *sfbp;
	SFBPlus sp;
{
    sfbpInfo *sfbpi = (sfbpInfo *) sfbp->info_area;
    Bits32 oldmode;

    /*
     * Get current mode to get state bits
     */
    oldmode = * (Bits32 *) &plus_regs.mode;

    /*
     * now restore state to starting point
     */
    sp->mode		= plus_regs.mode;		wbflush();
    sp->rop		= plus_regs.rop;		wbflush();
    sp->planemask	= sfbpi->planemask;		wbflush();
    if ( oldmode & SFBP_MODE_PM_IS_PERS ) {
      sp->pers_pixelmask = ~0;				wbflush();
    }
    else {
      sp->pixelmask	= plus_regs.pixelmask;		wbflush();
    }
    if ( oldmode & SFBP_MODE_ADDR_IS_NEW ) {
      sp->address	= plus_regs.address;		wbflush();
    }

    return;
}


int
sfbp_init_screen(closure, screen)
    caddr_t closure;
    ws_screen_descriptor *screen;
{
    register struct fb_info *fbp = (struct fb_info *)closure;
    register int unit = fbp - fb_softc;
    register struct sfbplus_info *sfbp = &sfbplus_softc[unit];

    sfbp->asic->base_address = sfbp->base_address;
    
    fb_init_screen(closure, screen);
}


/*
 * sfbp_clear_screen
 */
int
sfbp_clear_screen(closure, screen)
	caddr_t closure;
	ws_screen_descriptor *screen;
{
    register struct fb_info *fbp = (struct fb_info *)closure;
    register int unit = fbp - fb_softc;
    register struct sfbplus_info *sfbp = &sfbplus_softc[unit];
    register SFBPlus s = sfbp->asic;
    register int status, i, j, stride;
    unsigned int *dst, value;
    register ws_screen_descriptor *sp = &fbp->screen;
    register ws_depth_descriptor *dp = &fbp->depth[sp->root_depth];
    
    sfbp_save_regs( sfbp, s );
    if ( sfbp->bits_per_pixel == 32 ) {
      value = FFB_TRUECOLOR_WID;		/* TrueColor black */
      stride = dp->fb_width;
      dst = (unsigned int *) ( (vm_offset_t) sfbp->fb
			     + 16384 );	/* 4K pixel offset */
      for ( i = 0; i < sp->height; i++, dst += stride ) {
	for ( j = 0; j < sp->width; j++ ) {
	  dst[j] = value;
	}
      }
      status = 0;
    }
    else {
      fbp->depth[0].physaddr += ( NBPG >> 1 );
      status = fb_clear_screen( closure, screen );
      fbp->depth[0].physaddr -= ( NBPG >> 1 );
    }
    sfbp_restore_regs( sfbp, s );

    return (status);
}

/*
 * sfbp_scroll_screen
 */
int
sfbp_scroll_screen(closure, screen)
	caddr_t closure;
	ws_screen_descriptor *screen;
{
    register struct fb_info *fbp = (struct fb_info *)closure;
    register int unit = fbp - fb_softc;
    register struct sfbplus_info *sfbp = &sfbplus_softc[unit];
    register SFBPlus s = sfbp->asic;
    register int status;
    register ws_screen_descriptor *sp = &fbp->screen;
    register ws_depth_descriptor *dp = &fbp->depth[sp->root_depth];
    int n, scroll, i, j, wpl, wpfbl;
    unsigned int *dst, *src, *spp, *dpp, temp0, temp1, temp2, temp3;

    sfbp_save_regs( sfbp, s );
    if ( sfbp->bits_per_pixel == 32 ) {
      wpl = ( sp->f_width * sp->max_col * dp->bits_per_pixel ) >> 7;
      wpfbl = dp->fb_width;
      scroll = sp->max_row >> 3;
      n = ( sp->max_row - scroll + 1 ) * sp->f_height;
      spp = (unsigned int *) ( (vm_offset_t) sfbp->fb
			     + 16384
			     + ( dp->fb_width * sp->f_height
			       * scroll * 4 ) );
      dpp = (unsigned int *) ( (vm_offset_t) sfbp->fb
			     + 16384 );
      for ( j = 0; j < n; j++ ) {
	src = spp;
	dst = dpp;
	i = 0;
	do {
	  temp0 = src[0]; temp1 = src[1]; temp2 = src[2]; temp3 = src[3];
	  dst[0] = temp0; dst[1] = temp1; dst[2] = temp2; dst[3] = temp3;
	  dst += 4; src += 4;
	  i++;
	} while ( i < wpl );
	spp += wpfbl;
	dpp += wpfbl;
      }
      for ( ; j < dp->fb_height; j++ ) {
	dst = dpp;
	i = 0;
	temp0 = FFB_TRUECOLOR_WID;
	do {
	  dst[0] = temp0; dst[1] = temp0; dst[2] = temp0; dst[3] = temp0;
	  dst += 4;
	  i++;
	} while ( i < wpl );
	dpp += wpfbl;
      }
      status = scroll - 1;
    }
    else {
      fbp->depth[0].physaddr += ( NBPG >> 1 );
      status = fb_scroll_screen( closure, screen );
      fbp->depth[0].physaddr -= ( NBPG >> 1 );

    }
    sfbp_restore_regs( sfbp, s );

    return (status);
}

/*
 * sfbp_blitc
 */
extern  char *q_special[],q_font[];
extern  u_short q_key[],q_shift_key[];

int
sfbp_blitc(closure, screen, row, col, c)
	caddr_t closure;
	ws_screen_descriptor *screen;
	int row, col;
	u_char c;
{
    register struct fb_info *fbp = (struct fb_info *)closure;
    register int unit = fbp - fb_softc;
    register struct sfbplus_info *sfbp = &sfbplus_softc[unit];
    register SFBPlus s = sfbp->asic;
    register int status;
    register ws_screen_descriptor *sp = &fbp->screen;
    register ws_depth_descriptor *dp = &fbp->depth[sp->root_depth];
    int i, j;
    unsigned int *dst, value0, value1;
    char *f_row;
    unsigned char glyph_row;

    sfbp_save_regs( sfbp, s );
    if ( sfbp->bits_per_pixel == 32 ) {
      dst = (unsigned int *) ( (vm_offset_t) sfbp->fb
			     + 16384
			     + ( row * dp->fb_width * sp->f_height * 4 )
			     + ( col * sp->f_width * 4 ) );
      i = c - ' ';
      if ( i < 0 || i > 221 ) {
	i = 0;
      }
      else {
	if ( c > '~' ) i -= 34;
	i *= 15;
      }
      f_row = &q_font[i];
      value0 = FFB_TRUECOLOR_WID;
      value1 = FFB_TRUECOLOR_WID | 0xffffff;
      for ( j = 0; j < sp->f_height; j++ ) {
	glyph_row = (unsigned char) *f_row++;
	for ( i = 0; i < sp->f_width; i++ ) {
	  dst[i] = ( glyph_row & 0x01 ? value1 : value0 );
	  glyph_row >>= 1;
	}
	dst += dp->fb_width;
      }
      status = 0;
    }
    else {
      fbp->depth[0].physaddr += ( NBPG >> 1 );
      status = fb_blitc( closure, screen, row, col, c );
      fbp->depth[0].physaddr -= ( NBPG >> 1 );
    }
    sfbp_restore_regs( sfbp, s );

    return (status);
}


#define SFBP_INTERVAL 		0x2000000L	/* interval between mappings */
#define SFBP_ALIGNMENT   	(0x8000000L<<1)	/* alignment of first buffer */

long sfbpPhysInterval = 0x4000000L;

/*
 * sfbp_map_unmap_screen
 *
 * Provide 8 user-space mappings of the asic (in one page) and
 * 4 user-space mappings of a variable-length frame buffer.
 */
int
sfbp_map_unmap_screen(closure, depths, screen, mp)
	caddr_t closure;
	ws_depth_descriptor *depths;
	ws_screen_descriptor *screen;
	ws_map_control *mp;
{
	register struct fb_info *fbp = (struct fb_info *) closure;
	int nbytes;
	register int unit = fbp - fb_softc;
	register struct controller *ctlr = fbinfo[unit];
	register struct sfbplus_info *sfbp = &sfbplus_softc[unit];
	sfbpInfo *sfbpi = (sfbpInfo *) sfbp->info_area;
	caddr_t virtaddr;
	caddr_t base;
	register unsigned long i;
	extern caddr_t kalloc();

	dp = depths + mp->which_depth;

	/*
	 * unmap not yet (if ever) implemented)
	 */
	if ( mp->map_unmap == UNMAP_SCREEN ) {
	    return (EINVAL);
	}

	/*
	 * First, see about grabbing a one-page fragment.
	 */
	if ( sfbp->virtual_dma_buffer == (vm_offset_t) 0 ) {
	    vm_offset_t
		frag,
		frag2;

	    if ( ( frag = (vm_offset_t) kalloc( 2 * NBPG ) )
		 == (vm_offset_t) NULL ) {
		return (ENOMEM);
	    }
	    frag = ( ( frag + NBPG - 1 ) & ~( NBPG - 1 ) );
	    sfbp->virtual_dma_buffer = frag;
	    svatophys( frag, &frag2 );
	    sfbp->physical_dma_buffer = frag2;
	}

	/*
	 * Map the beginning of the module from the base up to and
	 * including the ASIC.
	 */
	base = ws_map_region( (caddr_t) sfbp->base, NULL,
			      ( (vm_offset_t) sfbp->asic - sfbp->base + NBPG ), 
			      0600, (int *) NULL);
	if ( base == (caddr_t) NULL ) {
		return ENOMEM;
	}
	sfbpi->option_base = (vm_offset_t) base;

	/*
	 * Now do four mappings of the frame buffer area itself.
	 *
	 * See notes in the SFB/HX mapping above for justification
	 * and motivation for the following actions.
	 */
		/* Must be twice the amount of space for the macros to work */
#define SFBP_ISNOTALIGNED(p) ((unsigned long)(p) & (SFBP_ALIGNMENT-1))

        virtaddr = (caddr_t)( BASE_SFBP_ADDRESS
			    + ( SFBP_ALIGNMENT * screen->screen ) );

	/*
	 * Map the frame buffer
	 */
	nbytes = sfbp->fb_size;
	if ( nbytes < 0x400000 ) nbytes = 0x400000;

	dp->plane_mask = (caddr_t) ws_map_region( sfbp->fb, virtaddr,
				      nbytes, 0600, (int *) NULL );
	if ( dp->plane_mask == (caddr_t) NULL ) {
		return(ENOMEM);
	}

	if ( SFBP_ISNOTALIGNED( dp->plane_mask ) ) {
	    printf(
		"Base address for sfb+ screen %d is not aligned properly\n",
		screen->screen);
	    printf("X server will most likely crash as a result.\n");
	    return(ENOMEM);
	}
	dp->plane_mask += ( sfbp->bits_per_pixel << 9 );
	sfbpi->fb_alias_increment = SFBP_INTERVAL;

	/*
	 * We have the first mapping, get the remainder, plus one for those
	 * nasty off-the-top-of-the-screen cases...
	 */
	for ( i = 1; i < SFBP_USER_MAPPING_COUNT+1; i++ ) {
	    caddr_t tmp;

	    virtaddr += SFBP_INTERVAL;
	    /*
	     * AXP write-buffer physically-based, so have to step the kseg addr
	     * also.
	     */
	    tmp = ws_map_region( sfbp->fb
				 + ( ( i % SFBP_USER_MAPPING_COUNT )
				   * sfbpPhysInterval ),
				 virtaddr, nbytes,
				 0600, (int *) NULL );
	    if ( tmp == (caddr_t) NULL ) {
		return(ENOMEM);
	    }
	    if ( tmp != virtaddr ) {
		printf("Aliased frame buffer address for screen %d\n",
			screen->screen);
		printf("is not properly aligned.\n");
		printf("X server will most likely crash as a result\n");
		return(ENOMEM);
	    }
	}

	/*
	 * Now map the fb heap so that they can see the info area and
	 * add in the offset to our bit of it.
	 */
	virtaddr = ws_map_region( PHYS_TO_KSEG( ws_heap_start ),
				  NULL,
				  ws_heap_size,
				  0600, (int *) NULL);
	if ( virtaddr == (caddr_t) NULL ) {
		return ENOMEM;
	}
	virtaddr += ( (vm_offset_t) sfbpi
		    - PHYS_TO_KSEG( ws_heap_start ) );

	/*
	 * Now, put the info area in plane_mask_phys.
	 */
	dp->plane_mask_phys = virtaddr;

	/*
	 * Finally get a user mapping of the 8KB cesspool.
	 */
	virtaddr = ws_map_region( sfbp->virtual_dma_buffer,
				  NULL, NBPG, 0600, (int *) NULL );
	if ( virtaddr == (caddr_t) NULL ) {
	    return ENOMEM;
	}
	sfbpi->virtual_dma_buffer = (vm_offset_t) virtaddr;
	sfbpi->physical_dma_buffer = sfbp->physical_dma_buffer;

	/*
	 * Done
	 */
	return (0);
}


/*
 * sfbp_ioctl
 *
 * device-private ioctls for the SFB+
 */
/* ARGSUSED */
int
sfbp_ioctl(closure, cmd, data, flag)
caddr_t closure;
unsigned int cmd;
caddr_t data;
int flag;
{
  register struct fb_info *fbp = (struct fb_info *)closure;
  register int unit = fbp - fb_softc;
  register struct controller *ctlr = fbinfo[unit];
  register struct sfbplus_info *sfbp = &sfbplus_softc[unit];
  sfbp_ioc *ioc = (sfbp_ioc *) data;
  int status = 0;

#if     SEC_BASE
  if (!privileged(SEC_SYSATTR, 0))
#else
  if (suser(u.u_cred, &u.u_acflag))
#endif
    return(EACCES);

  switch( ioc->cmd ) {

  case SFBP_IOC_LOAD_WINDOW_TAGS:
    {
      sfbp_ioc_window_tag
	*wtp = &ioc->data.window_tag;

      sfbp_window_tag_cell
	entry;

      int
	i,
	error;

      if ( !sfbp->bt463_present ) {
	return (ENXIO);
      }
      for ( i = wtp->start;
	    i < wtp->start + wtp->ncells;
	    i++ ) {
	copyin( wtp->p_cells + i, &entry, sizeof( entry ) );
	error = load_window_tag( fbp, &entry );
	if ( error < 0 ) return error;
      }
      sfbp_bt463_enable_interrupt( fbp->cmf.cmc );

    }
    break;

  case SFBP_IOC_ENABLE_DMA_OPS:
    {
      extern int ws_register_vm_callback();

      ws_register_vm_callback( fbp->screen.screen,
			       sfbp_invalidate_page_ref,
			       sfbp );
    }
    break;

  case SFBP_IOC_SET_STEREO_MODE:
    {
      status = sfbp_set_stereo_mode( sfbp, ioc->data.stereo_mode );
    }
    break;

  case SFBP_IOC_GET_STEREO_MODE:
    {
      ioc->data.stereo_mode = sfbp->stereo_mode;
    }
    break;

  default:
    return(EINVAL);
  }

  return (status);
}


/*
 * clean_window_tag
 *
 *  Load some window tags
 */
static void
clean_window_tag( fbp, sfbp )
	struct fb_info *fbp;
	struct sfbplus_info *sfbp;
{
	register struct bt463info2
	    *bti = (struct bt463info2 *) fbp->cmf.cmc;

	register SFBPlus
	    sp = sfbp->asic;

	register int
	    win_addr,
	    start,
	    end,
	    nextcell = -1,
	    i;

	register sfbp_window_tag_cell
	    *p_table,
	    *entry;

	if ( !bti->screen_on ) {
	    return;
	}

	start = sfbp->wt_min_dirty;
	end = sfbp->wt_max_dirty;
	p_table = sfbp->wt_cell;
	for ( i = start; i <= end; i++) {
	    win_addr = WINDOW_TYPE_TABLE + i;
	    entry = p_table + i;
	    if ( i != nextcell ) {
		sp->ramdac_setup = sfbp->head_mask
				 | SFBP_RAMDAC_463_ADDR_LOW;	wbflush();
		sp->ramdac = win_addr & ADDR_LOW_MASK;		wbflush();
		sp->ramdac_setup = sfbp->head_mask
				 | SFBP_RAMDAC_463_ADDR_HIGH;	wbflush();
		sp->ramdac = ( win_addr >> 8 );			wbflush();
		sp->ramdac_setup = sfbp->head_mask
				 | SFBP_RAMDAC_463_CMD_REG;	wbflush();
	    }
	    sp->ramdac = entry->low;				wbflush();
	    sp->ramdac = entry->mid;				wbflush();
	    sp->ramdac = entry->high;				wbflush();
	    nextcell = i+1;
	}

	/*
	 * Clean up.  Regardless of which field we are actually in,
	 * we can mark both tables as clean.
	 */
	sfbp->wt_min_dirty = WINDOW_TAG_COUNT;
	sfbp->wt_max_dirty = 0;
	sfbp->wt_dirty = 0;

	return;
}

/*
 * load_window_tag
 *
 */
/* ARGSUSED */
static int
load_window_tag( fbp, entry )
	struct fb_info *fbp;
	sfbp_window_tag_cell *entry;
{
	register struct bt463info2
	    *bti = (struct bt463info2 *) fbp->cmf.cmc;

	register int
	    s,
	    unit = fbp - fb_softc,
	    windex = entry->windex;

	register struct sfbplus_info
	    *sfbp = &sfbplus_softc[unit];

	sfbp_window_tag_cell
	    *p_cells;

	if ( windex >= WINDOW_TAG_COUNT ) 
	    return -1;

	p_cells = sfbp->wt_cell;

	s = splhigh();
	p_cells[windex].low	= entry->low;
	p_cells[windex].mid	= entry->mid;
	p_cells[windex].high	= entry->high;
	if ( windex < sfbp->wt_min_dirty )
	    sfbp->wt_min_dirty = windex;
	if ( windex > sfbp->wt_max_dirty )
	    sfbp->wt_max_dirty = windex;
	sfbp->wt_dirty = 1;
	splx(s);

	return 0;
}

/*
 * sfbp_bt463_enable_interrupt
 *
 * Enable vsync interrupts.
 */
static void
sfbp_bt463_enable_interrupt(closure)
    caddr_t closure;		/* XXX */
{
    register struct bt463info2
	*bti = (struct bt463info2 *) closure;

    register int
	s,
	unit = bti->unit;

    struct sfbplus_info
	*sp = &sfbplus_softc[unit];

    Bits32
	intr;

    s = spltty();
    intr = sp->asic->intr_status;
    intr |= ( SFBP_INTR_VSYNC << SFBP_INTR_ENABLE_SHIFT );
    intr &= ~SFBP_INTR_VSYNC;
    sp->asic->intr_status = intr;
    wbflush();
    splx(s);

    return;
}

/*
 * sfbp_bt459_enable_interrupt
 *
 * Enable vsync interrupts.
 */
static void
sfbp_bt459_enable_interrupt(closure)
    caddr_t closure;		/* XXX */
{
    register struct bt459info2
	*bti = (struct bt459info2 *) closure;

    register int
	s,
	unit = bti->unit;

    struct sfbplus_info
	*sp = &sfbplus_softc[unit];

    Bits32
	intr;

    s = spltty();
    intr = sp->asic->intr_status;
    intr |= ( SFBP_INTR_VSYNC << SFBP_INTR_ENABLE_SHIFT );
    intr &= ~SFBP_INTR_VSYNC;
    sp->asic->intr_status = intr;
    wbflush();
    splx(s);

    return;
}

/*
 * sfbp_curs_enable_interrupt
 *
 * Enable vsync interrupts.
 */
static void
sfbp_curs_enable_interrupt(closure)
    caddr_t closure;		/* XXX */
{
    register struct sfbp_curs_info
	*bti = (struct sfbp_curs_info *) closure;

    register int
	s,
	unit = bti->unit;

    struct sfbplus_info
	*sp = &sfbplus_softc[unit];

    Bits32
	intr;

    s = spltty();
    intr = sp->asic->intr_status;
    intr |= ( SFBP_INTR_VSYNC << SFBP_INTR_ENABLE_SHIFT );
    intr &= ~SFBP_INTR_VSYNC;
    sp->asic->intr_status = intr;
    wbflush();
    splx(s);

    return;
}

/*
 * sfbp_recolor_cursor violates the colormap/cursor functionality
 * separation and so some closure contortions are necessary.  This is
 * used only for the 463/special cursor case.
 */
static int
sfbp_recolor_cursor(closure, screen, fg, bg)
    caddr_t closure;
    ws_screen_descriptor *screen;
    ws_color_cell *fg, *bg;
{
    register struct sfbp_curs_info *bti = (struct sfbp_curs_info *) closure;
    register int unit = bti->unit;
    register struct fb_info *fbp = &fb_softc[unit];

    return ( bt463_2_recolor_cursor( (struct bt463info2 *) fbp->cmf.cmc,
	     screen, fg, bg) );
}

/*
 * Similarly for video on/off but we need a full set of functions
 */
static int
sfbp_bt459_video_off(closure)
    caddr_t closure;
{
    register struct bt459info2 *btii = (struct bt459info2 *) closure;
    register int unit = btii->unit;
    register struct sfbplus_info *sp = &sfbplus_softc[unit];
    Bits32 valid;
    int s;

    if (btii->screen_on ) {
	s = spltty();
	btii->screen_on = 0;
	valid = sp->asic->video_valid;
	valid |= 2;
	sp->asic->video_valid = valid;
	wbflush();
	splx(s);
    }

    return 0;
}

static int
sfbp_bt459_video_on(closure)
    caddr_t closure;
{
    register struct bt459info2 *btii = (struct bt459info2 *) closure;
    register int unit = btii->unit;
    register struct sfbplus_info *sp = &sfbplus_softc[unit];
    Bits32 valid;
    int s;

    if (! btii->screen_on ) {
	s = spltty();
	btii->screen_on = 1;
	valid = sp->asic->video_valid;
	valid &= ~2;
	sp->asic->video_valid = valid;
	wbflush();
	splx(s);
    }

    return 0;
}

static int
sfbp_curs_video_off(closure)
    caddr_t closure;
{
    register struct bt463info2 *btii = (struct bt463info2 *) closure;
    register int unit = btii->unit;
    register struct sfbplus_info *sp = &sfbplus_softc[unit];
    Bits32 valid;
    int s;

    if ( btii->screen_on ) {
	s = spltty();
	btii->screen_on = 0;
	valid = sp->asic->video_valid;
	valid |= 2;
	sp->asic->video_valid = valid;
	wbflush();
	splx(s);
    }

    return 0;
}

static int
sfbp_curs_video_on(closure)
    caddr_t closure;
{
    register struct bt463info2 *btii = (struct bt463info2 *) closure;
    register int unit = btii->unit;
    register struct sfbplus_info *sp = &sfbplus_softc[unit];
    Bits32 valid;
    int s;

    if ( !btii->screen_on ) {
	s = spltty();
	btii->screen_on = 1;
	valid = sp->asic->video_valid;
	valid &= ~2;
	sp->asic->video_valid = valid;
	wbflush();
	splx(s);
    }

    return 0;
}

/*
 * sfbp_init_closure
 */
caddr_t
sfbp_init_closure(closure, address, unit, type)
        caddr_t closure;
        caddr_t address;
        int unit;
        int type;
{
    struct sfbplus_info
	*sfbp;

    register int
	sfbp_type;

    SFBPlus
	sp;

    SFBPlusDepth
	depth;

    register sfbpInfo
	*sfbpi;

    vm_offset_t
	kseg_at,
	at;

    struct fb_info
	*fbp = (struct fb_info *) closure;

    register int
	i;

    fbp += unit;
    address = (caddr_t) PHYS_TO_KSEG( DENSE( KSEG_TO_PHYS( address ) ) );
    sfbp = sfbplus_softc + unit;
    sp = (SFBPlus) ( address + (vm_offset_t) PMAGDA_SFBP_ASIC_OFFSET );
    depth = sp->depth;

    /*
     * We get the SFB+ 'type' by subtracting deep from mask.  This
     * gives us the index into the types table where the offsets
     * and sizes for various structures are kept.  Weird, but there
     * you go.
     */
    sfbp_type = depth.mask - depth.deep;
    if ( sfbp_type < 0 ||
	 sfbp_type >= nsfbp_types ||
	 sfbplus_type[sfbp_type].fb == PMAGDA_INVALID_FB_OFFSET ) {
      return (caddr_t) NULL;
    }
    *sfbp = sfbplus_type[sfbp_type];
    sfbp->base = (vm_offset_t) address;
    sfbp->asic = (SFBPlus) ( address + (vm_offset_t) sfbp->asic );
    sfbp->fb = (vm_offset_t) ( address + (vm_offset_t) sfbp->fb );
    sfbp->depth = depth;
    for (i = 0; i < fbp->screen.allowed_depths; i++) {
      fbp->depth[i].physaddr = (caddr_t) sfbp->fb;
      if (fbp->depth[i].plane_mask_phys)
	fbp->depth[i].plane_mask_phys = 
	  address + (vm_offset_t)fbp->depth[i].plane_mask_phys;
      if ( sfbp->bt463_present ) {
	fbp->depth[i].depth = 32;
	fbp->depth[i].bits_per_pixel = 32;
	fbp->depth[i].scanline_pad = 32;
      }
      else {
	fbp->depth[i].depth = 8;
	fbp->depth[i].bits_per_pixel = 8;
	fbp->depth[i].scanline_pad = 32;
      }
    }
    for (i = 0; i < fbp->screen.nvisuals; i++ ) {
      if ( sfbp->bt463_present ) {
	fbp->visual[i].screen_class = TrueColor;
	fbp->visual[i].depth = 32;
	fbp->visual[i].red_mask = 0xff0000;
	fbp->visual[i].green_mask = 0x00ff00;
	fbp->visual[i].blue_mask = 0x0000ff;
	fbp->visual[i].bits_per_rgb = 8;
	fbp->visual[i].color_map_entries = 528;
      }
      else {
	fbp->visual[i].screen_class = PseudoColor;
	fbp->visual[i].depth = 8;
	fbp->visual[i].red_mask = 0;
	fbp->visual[i].green_mask = 0;
	fbp->visual[i].blue_mask = 0;
	fbp->visual[i].bits_per_rgb = 8;
	fbp->visual[i].color_map_entries = 256;
      }
    }
    if ( sfbp->bt463_present ) {
      sfbp->base_address = 1;
      fbp->cmf.init_closure = sfbp_bt463_init_closure;
      fbp->cmf.init_color_map = bt463_2_init_color_map;
      fbp->cmf.load_color_map_entry = bt463_2_load_color_map_entry;
      fbp->cmf.clean_color_map = (void (*)()) NULL;
      fbp->cmf.video_on = sfbp_curs_video_on;
      fbp->cmf.video_off = sfbp_curs_video_off;
      fbp->cmf.cmc = (caddr_t ) bt463_softc2;
      fbp->cf.init_closure = sfbp_curs_init_closure;
      fbp->cf.load_cursor = sfbp_load_cursor;
      fbp->cf.recolor_cursor = sfbp_recolor_cursor;
      fbp->cf.set_cursor_position = sfbp_set_cursor_position;
      fbp->cf.cursor_on_off = sfbp_cursor_on_off;
      fbp->cf.cc = (caddr_t) sfbp_curs_softc;
    }
    else {
      sfbp->base_address = 1;
      fbp->cmf.init_closure = sfbp_bt459_init_closure;
      fbp->cmf.init_color_map = bt459_2_init_color_map;
      fbp->cmf.load_color_map_entry = bt459_2_load_color_map_entry;
      fbp->cmf.clean_color_map = (void (*)()) NULL;
      fbp->cmf.video_on = sfbp_bt459_video_on;
      fbp->cmf.video_off = sfbp_bt459_video_off;
      fbp->cmf.cmc = (caddr_t ) bt459_softc2;
      fbp->cf.init_closure = sfbp_bt459_init_closure;
      fbp->cf.load_cursor = bt459_2_load_cursor;
      fbp->cf.recolor_cursor = bt459_2_recolor_cursor;
      fbp->cf.set_cursor_position = sfbp_bt459_2_set_cursor_position;
      fbp->cf.cursor_on_off = bt459_2_cursor_on_off;
      fbp->cf.cc = (caddr_t) bt459_softc2;
    }
    sfbp->asic->base_address = sfbp->base_address;
    wbflush();

    /*
     * Allocate user info area
     */
    at = ws_heap_at + sizeof( vm_offset_t ) - 1;
    at -= at % sizeof( vm_offset_t );
    if ( at + sizeof( sfbpInfo ) >= ws_heap_end ) {
      return (caddr_t) NULL;
    }
    kseg_at = PHYS_TO_KSEG( at );
    sfbpi = (sfbpInfo *) kseg_at;
    ws_heap_at = at + sizeof( sfbpInfo );
    sfbp->info_area = (caddr_t) sfbpi;

    /*
     * Init the info area
     */
    sfbpi->fb_alias_increment = (vm_offset_t) 0;
    sfbpi->option_base = (vm_offset_t) NULL;
    sfbpi->planemask = ~0L;
    sfbpi->virtual_dma_buffer = (vm_offset_t) NULL;
    sfbpi->physical_dma_buffer = (vm_offset_t) NULL;
    
    return ((caddr_t) &fbp->screen);
}

/*
 * sfbp_bt463_init_closure
 *
 * Init closure struct for bt463
 */
static caddr_t
sfbp_bt463_init_closure(closure, address, unit, type)
    caddr_t closure;
    caddr_t address;
    int unit;
    int type;
{
    register struct bt463info2 *bti = (struct bt463info2 *) closure;
    register caddr_t addr;
    register u_int i;
    register u_int nextunit = 0;

    address = (caddr_t) PHYS_TO_KSEG( DENSE( KSEG_TO_PHYS( address ) ) );
    /*
     * see if we've already init'd the closure for this vdac already
     */
    addr = address + (vm_offset_t) bt463_type2[type].setup;
    for (i = 0; i < nbt_softc2; i++) {
        if ( addr == (caddr_t) bti[i].setup ) {
            bti[i].unit = unit;
            return (caddr_t) (&bti[i]);
        }
        else if ( bti[i].setup == (unsigned int *) NULL ) {
            nextunit = i;
            break;
        }
    }

    /*
     * setup another struct, if possible
     */
    if ( i >= nbt_softc2 ) 
       return (caddr_t) NULL;

    bti += nextunit;
    nextunit += 1;

    /*
     * set to initial values
     */
    *bti = bt463_type2[type];
    bti->unit = unit;

    /*
     * update relative offsets to physical addresses
     */
    bti->setup = (unsigned int *) addr;
    bti->data = (unsigned int *) ( address
				 + (vm_offset_t) bt463_type2[type].data );
    bti->enable_interrupt = sfbp_bt463_enable_interrupt;

    /*
     * Init.
     */
    bt463_2_init( bti );

    /*
     * Done
     */
    return (caddr_t) bti;
}


/*
 * sfbp_bt459_init_closure
 *
 * Init closure struct for bt459.  Slightly different.  The basic init
 * closure is in the 459 code.  We add a bit of flavoring here...
 */
static caddr_t
sfbp_bt459_init_closure(closure, address, unit, type)
    caddr_t closure;
    caddr_t address;
    int unit;
    int type;
{
    caddr_t data;

    address = (caddr_t) PHYS_TO_KSEG( DENSE( KSEG_TO_PHYS( address ) ) );

    data = bt459_2_init_closure( closure, address, unit, type );
    if ( data != (caddr_t) NULL ) {
	struct bt459info2 *bti = (struct bt459info2 *) data;

	bti->enable_interrupt = sfbp_bt459_enable_interrupt;
    }

    return (data);
}


/*
 * sfbp_curs_init_closure
 *
 * Init closure struct for cons-up cursor chip
 */
static caddr_t
sfbp_curs_init_closure(closure, address, unit, type)
    caddr_t closure;
    caddr_t address;
    int unit;
    int type;
{
    register struct sfbp_curs_info *bti = (struct sfbp_curs_info *) closure;
    register caddr_t addr;
    register u_int i;
    register u_int nextunit = 0;

    /*
     * see if we've already init'd the closure for this vdac already
     */
    addr = address + (vm_offset_t) sfbp_curs_type[type].xy_reg;
    for (i = 0; i < nbt_softc2; i++) {
        if ( addr == (caddr_t) bti[i].xy_reg ) {
            bti[i].unit = unit;
            return (caddr_t) (&bti[i]);
        }
        else if ( bti[i].xy_reg == (unsigned int *) NULL ) {
            nextunit = i;
            break;
        }
    }

    /*
     * setup another struct, if possible
     */
    if ( i >= nbt_softc2 ) 
       return (caddr_t) NULL;

    bti += nextunit;
    nextunit += 1;

    /*
     * set to initial values
     */
    *bti = sfbp_curs_type[type];
    bti->unit = unit;

    /*
     * update relative offsets to physical addresses
     */
    bti->last_row = -65;
    bti->xy_reg = (unsigned int *) addr;
    bti->valid = (Bits32 *) ( address
			    + (vm_offset_t) sfbp_curs_type[type].valid );
    bti->enable_interrupt = sfbp_curs_enable_interrupt;

    /*
     * Done
     */
    return (caddr_t) bti;
}

/*
 * sfbp_set_cursor_position
 *
 * Position the cursor to a particular location.  Note that most of the
 * work is done by the main driver, which does bounds checking on hot spots,
 * and keeps track of the cursor location.
 */

/* ARGSUSED */
static int
sfbp_set_cursor_position(closure, wsp, x, y)
    caddr_t closure;
    ws_screen_descriptor *wsp;
    register int x, y;
{
    register struct sfbp_curs_info *bti = (struct sfbp_curs_info *) closure;
    int unit = bti->unit;
    struct sfbplus_info *sfbp = &sfbplus_softc[unit];
    SFBPlus sp = sfbp->asic;
    register int xt, yt;
    unsigned int base = 0, row;

    xt = x + bti->fb_xoffset - bti->x_hot;
    if ( sfbp->stereo_mode == SFBP_IOC_STEREO_24 ) {
      y >>= 1;
    }
    yt = y + bti->fb_yoffset - bti->y_hot;
    if ( yt < 0 ) {
	row = (unsigned int) (-yt);
	base = row;
	yt = 0;
    }
    else if ( ( row = wsp->height + 3 - y + bti->y_hot ) >= 64 ) {
      row = 64;
    }
    if ( row != bti->last_row ) {
	sp->cursor_base_address = ( ( row - 1 ) << 10 ) | ( base << 4 );
	wbflush();
	bti->last_row = row;
    }
    sp->cursor_xy = xt | ( yt << 12 );
    wbflush();

    return 0;
}


/*
 * sfbp_load_cursor
 */
static int
sfbp_load_cursor(closure, screen, cursor, sync)
    caddr_t closure;
    ws_screen_descriptor *screen;
    ws_cursor_data *cursor;
    int sync;
{
    register struct sfbp_curs_info *bti = (struct sfbp_curs_info *) closure;
    int s = spltty();
    int status;

    bti->x_hot = cursor->x_hot;
    bti->y_hot = cursor->y_hot;
    sfbp_set_cursor_position(closure, screen, screen->x, screen->y);

    sfbp_reformat_cursor( (unsigned char *) bti->bits, cursor);

    bti->dirty_cursor = 1;

    if (sync) {
        /*
	 * if vblank synchronization important...
	 */
        sfbp_curs_enable_interrupt( bti );
	status = 0;
    }
    else {
	/*
	 * can't enable load at vblank or don't care, then just do it
	 */
	status = sfbp_load_formatted_cursor(bti);
    }
    splx(s);
    return (status);
}

static unsigned char sfbp_lookup_table[256] = {
0x00, 0x02, 0x08, 0x0a, 0x20, 0x22, 0x28, 0x2a,
0x80, 0x82, 0x88, 0x8a, 0xa0, 0xa2, 0xa8, 0xaa,
0x01, 0x03, 0x09, 0x0b, 0x21, 0x23, 0x29, 0x2b,
0x81, 0x83, 0x89, 0x8b, 0xa1, 0xa3, 0xa9, 0xab,
0x04, 0x06, 0x0c, 0x0e, 0x24, 0x26, 0x2c, 0x2e,
0x84, 0x86, 0x8c, 0x8e, 0xa4, 0xa6, 0xac, 0xae,
0x05, 0x07, 0x0d, 0x0f, 0x25, 0x27, 0x2d, 0x2f,
0x85, 0x87, 0x8d, 0x8f, 0xa5, 0xa7, 0xad, 0xaf,
0x10, 0x12, 0x18, 0x1a, 0x30, 0x32, 0x38, 0x3a,
0x90, 0x92, 0x98, 0x9a, 0xb0, 0xb2, 0xb8, 0xba,
0x11, 0x13, 0x19, 0x1b, 0x31, 0x33, 0x39, 0x3b,
0x91, 0x93, 0x99, 0x9b, 0xb1, 0xb3, 0xb9, 0xbb,
0x14, 0x16, 0x1c, 0x1e, 0x34, 0x36, 0x3c, 0x3e,
0x94, 0x96, 0x9c, 0x9e, 0xb4, 0xb6, 0xbc, 0xbe,
0x15, 0x17, 0x1d, 0x1f, 0x35, 0x37, 0x3d, 0x3f,
0x95, 0x97, 0x9d, 0x9f, 0xb5, 0xb7, 0xbd, 0xbf,

0x40, 0x42, 0x48, 0x4a, 0x60, 0x62, 0x68, 0x6a,
0xc0, 0xc2, 0xc8, 0xca, 0xe0, 0xe2, 0xe8, 0xea,
0x41, 0x43, 0x49, 0x4b, 0x61, 0x63, 0x69, 0x6b,
0xc1, 0xc3, 0xc9, 0xcb, 0xe1, 0xe3, 0xe9, 0xeb,
0x44, 0x46, 0x4c, 0x4e, 0x64, 0x66, 0x6c, 0x6e,
0xc4, 0xc6, 0xcc, 0xce, 0xe4, 0xe6, 0xec, 0xee,
0x45, 0x47, 0x4d, 0x4f, 0x65, 0x67, 0x6d, 0x6f,
0xc5, 0xc7, 0xcd, 0xcf, 0xe5, 0xe7, 0xed, 0xef,
0x50, 0x52, 0x58, 0x5a, 0x70, 0x72, 0x78, 0x7a,
0xd0, 0xd2, 0xd8, 0xda, 0xf0, 0xf2, 0xf8, 0xfa,
0x51, 0x53, 0x59, 0x5b, 0x71, 0x73, 0x79, 0x7b,
0xd1, 0xd3, 0xd9, 0xdb, 0xf1, 0xf3, 0xf9, 0xfb,
0x54, 0x56, 0x5c, 0x5e, 0x74, 0x76, 0x7c, 0x7e,
0xd4, 0xd6, 0xdc, 0xde, 0xf4, 0xf6, 0xfc, 0xfe,
0x55, 0x57, 0x5d, 0x5f, 0x75, 0x77, 0x7d, 0x7f,
0xd5, 0xd7, 0xdd, 0xdf, 0xf5, 0xf7, 0xfd, 0xff

};

/*
 * sfbp_reformat_cursor
 */
static void
sfbp_reformat_cursor(bits, cursor)
	register ws_cursor_data *cursor;
	register unsigned char *bits;
{
	register unsigned int cw, mw;
	register int i, j;
	register int nwords, shifts;
	register unsigned int mask, emask, omask;
	unsigned char *cbp = bits;
	unsigned long *cwp;

	nwords = cursor->height;
	mask = 0xffffffff;
	if(cursor->width > 32) {
		nwords *= 2;
		shifts = 32 - (cursor->width - 32);
		emask = 0xffffffff;
		omask = (emask << shifts) >> shifts;
	}
	else {
		shifts = 32 - cursor->width;
		emask = omask = (mask << shifts) >> shifts;
	}
	
	for (i = 0; i < nwords; i++) {
		mask = emask;
		if (i & 1) mask = omask;
		cw = cursor->cursor[i] & mask;
		mw = cursor->mask[i] & mask;
		for (j = 0; j < 8; j++)	 {
		    *cbp++ = sfbp_lookup_table[((cw << 4) | (mw & 0xf)) & 0xff];
		    cw >>= 4;
		    mw >>= 4;
		}
		if (cursor->width <= 32) {
		    *cbp++ = 0; *cbp++ = 0;
		    *cbp++ = 0; *cbp++ = 0;
		    *cbp++ = 0; *cbp++ = 0;
		    *cbp++ = 0; *cbp++ = 0;
		}
	}
	cwp = (unsigned long *) cbp;
	for ( ; i < 128; i++ ) {
	    *cwp++ = 0L;
	}
}

/*
 * sfbp_load_formatted_cursor
 *
 * given precomputed cursor, load it.
 */
sfbp_load_formatted_cursor(bti)
    register struct sfbp_curs_info *bti;
{
    int unit = bti->unit;
    struct sfbplus_info *sfbp = &sfbplus_softc[unit];
    SFBPlus s = sfbp->asic;
    register int i, mask;
    unsigned long *src, *dst;

    sfbp_save_regs( sfbp, s );
    src = (unsigned long *) bti->bits;
    dst = (unsigned long *) sfbp->fb;
    for ( i = 0; i < 1024 / sizeof( unsigned long ); i++ ) {
	*dst++ = *src++;
    }
    wbflush();
    sfbp_restore_regs( sfbp, s );

    bti->dirty_cursor = 0;              /* no longer dirty */

    return 0;
}

/*
 * sfbp_init_cursor
 */
sfbp_init_cursor(closure)
	caddr_t closure;
{
    register struct sfbp_curs_info *bti = (struct sfbp_curs_info *) closure;
    int unit = bti->unit;
    struct sfbplus_info *sfbp = &sfbplus_softc[unit];
    register int i;
    unsigned long *dst;

    dst = (unsigned long *) sfbp->fb;
    for ( i = 0; i < 1024 / sizeof( unsigned long ); i++ ) {
	*dst++ = 0L;
    }

    return 0;
}

/*
 * sfbp_cursor_on_off
 */
static int
sfbp_cursor_on_off(closure, on_off)
	caddr_t closure;
	int on_off;
{
    register struct sfbp_curs_info *bti = (struct sfbp_curs_info *) closure;
    int unit = bti->unit;
    struct sfbplus_info *sfbp = &sfbplus_softc[unit];
    SFBPlus sp = sfbp->asic;
    unsigned int valid;
    register int s = spltty();

    valid = sp->video_valid;
    if (on_off) {
	sp->video_valid = valid | 0x04;
    }
    else {
	sp->video_valid = ( valid & ~0x04 );
    }
    wbflush();
    bti->on_off = on_off;
    splx(s);
    return(0);
}


/*
 * sfbp_invalidate_page_ref
 *
 * Device-dependent pageout handler.  This should be called at splvm().
 */
static int
sfbp_invalidate_page_ref( func, va, data )
     int func;
     vm_offset_t va;
     caddr_t data;
{
#ifdef	DO_NEW_STYLE_INVALIDATE
  if ( func == PMAP_COPROC_EXIT ) {
#else
  if ( func == PDEVCMD_DMA ) {
#endif
    /*
     * Process dying, wait a bit before letting address space evaporate.
     */
    struct sfbplus_info
	*sfbp = (struct sfbplus_info *) data;

    while ( ( sfbp->asic->command_status & 0x01 ) ) {
	DELAY( 10 );
    }
  }
  return (0);
}

/*
 * sfbp_bt459_2_set_cursor_position
 */
/* ARGSUSED */
static int
sfbp_bt459_2_set_cursor_position(closure, sp, x, y)
    caddr_t closure;
    ws_screen_descriptor *sp;
    register int x, y;
{
    register struct bt459info2 *bti = (struct bt459info2 *)closure;
    register int xt, yt, s;
    struct sfbplus_info *sfbp = &sfbplus_softc[bti->unit];

    xt = x + bti->fb_xoffset - bti->x_hot;
    if ( sfbp->stereo_mode == SFBP_IOC_STEREO_24 ) {
      y >>= 1;
    }
    yt = y + bti->fb_yoffset - bti->y_hot;

    IPLTTY(s);

    *bti->setup = bti->head_mask
		| BT459_TYPE2_ADDR_LOW
		| BT459_TYPE2_SETUP_WRITE;	wbflush();
    *bti->data = CURSOR_X_LOW;			wbflush();

    *bti->setup = bti->head_mask
		| BT459_TYPE2_ADDR_HIGH
		| BT459_TYPE2_SETUP_WRITE;	wbflush();
    *bti->data = (CURSOR_X_LOW >> 8);		wbflush();

    *bti->setup = bti->head_mask
		| BT459_TYPE2_CMD_CURS
		| BT459_TYPE2_SETUP_WRITE;	wbflush();
    *bti->data = xt;				wbflush();
    *bti->data = (xt>>8);			wbflush();
    *bti->data = yt;				wbflush();
    *bti->data = (yt>>8);			wbflush();

    SPLX(s);

    return 0;
}

