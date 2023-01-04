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
static char *rcsid = "@(#)$RCSfile: px.c,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/06/24 22:45:12 $";
#endif

#ifndef lint
static char *sccsid = "%W%      (ULTRIX/OSF)  %G%";
#endif lint

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
#define _PX_C_
/*
 * Modification History
 *
 * 00-Dec-91		Lloyd Wheeler
 *		Temporarily removed CPU idle-time sampling at Sam's
 *		recommendation;  PEX should be able to survive without it
 *		until we figure out whether it is *really* needed.
 *		Completed conversion to OSF (initializing px_info structures
 * 		with addresses of memory allocated during mips_init().
 *
 * 00-Aug-91		Sam Hsu
 *		Convert to OSF.
 *
 * 10 Aug 1990		Sam Hsu
 *		Modified for use w/ PX, PXG, PXG-Turbo.  Lots of cleanup.
 *
 * 27 May 1990		Win Treese
 *		Modified for PixelStamp 2-D accelerator hardware, borrowing
 *		heavily from io/tc/gx.c.
 *
 * November, 1989	Jim Gettys
 *
 *		Complete rewrite for multiscreen support.  Boy how kludges
 *		accumulate over time....
 */

#include <machine/hal/kn15aa.h> /* get DENSE macro */
#include <data/ws_data.c>
#include <data/bt_data.c>
#include <data/px_data.c>
#include <io/dec/tc/tc.h>

static caddr_t	console_address = NULL;

int	px_attach();
int	px_which_option();
int	px_cons_init();
void	px_rect();
void	px_init_stic();

extern unsigned short fg_font[];	/* font table for px_blitc() */
extern  int     ws_display_type;
extern  int     ws_display_units;

extern int ws_graphics_config_error;

/*
 * Probe nothing and return a 1.
 */
pxprobe(nxv, ui)
    char *nxv;
    struct controller *ui;
{
    /* 
     * the initialization of the first screen is done through px_cons_init,
     * so if we have gotten this far we are alive so return a 1
     */
    return(1);
}


/*
 * Routine to attach to the graphic device.
 */
pxattach(ui)
    struct controller *ui;
{
    register int i;
    register px_info *pxp = &px_softc[ui->ctlr_num];

    if (i = px_attach(ui->addr, ui->ctlr_num, ui->flags)) {
	if (i > 0 && pxp->attach) {
	    /* i > 0: not console, continue attaching */
	    if ((*pxp->attach)(ui, pxp)) {
		px_init_screen(pxp, &pxp->screen);
		px_clear_screen(pxp, &pxp->screen);
	    }
	    else {
		bzero((caddr_t)pxp, sizeof(px_info));
		return 0;
	    }
	}
	else i = 0;

	if (pxp->bootmsg)
	    (*pxp->bootmsg)(ui, pxp);

	return i;			/* attach successful */
    }
    return 0;
}
#undef Number
#undef Addr
#undef Flags

/*
 * the routine that does the real work.  This is so the console can get
 * initialized before normal attach goes on.
 */
px_attach(address, unit, flags)
    caddr_t address;
    int unit;
    int flags;
{
    register px_info *pxp = &px_softc[unit];
    register int dev_type, m_type = flags;
    register int i;
    caddr_t dense_addr;
    
#ifndef __alpha
    address = (caddr_t) PHYS_TO_K1(address);
#endif /* __alpha */

    if ((dev_type = px_which_option(address)) < 0)
	goto Out;

    /* set the global display type for sizer */
    if (ws_display_units < 8) 
    {
	ws_display_type = (ws_display_type << 8) |
		px_types[dev_type].ws_display_type;
	ws_display_units = (ws_display_units << 1) | 1;
    }

    /*
     * deal with screen first, but only initialize the screen
     * structure once!
     */
    if (console_address != address)
	pxp->screen = px_types[dev_type].screen;
    pxp->screen.screen = unit;

    /* default to monitor type in screen structure unless flags set */

    if (flags != 0) {
	if ((m_type < 0) || (m_type >= nmon_types)) m_type = 0;
	pxp->screen.monitor_type = monitor_type[m_type];
    }

    /*
     * if we are the console, then we've already done general 
     * initialization, so we shouldn't attempt to define another screen
     * given that it's already been defined.  Note that we've dealt
     * with screen type above.
     */
    if (console_address == address) {
	PX_DEBUG(PX_TALK,
		 printf("px_attach(%x) -> console\n", address);
		 );
	return -1;
    }

    /* then with the depths and visual types */

    for (i = 0; i < pxp->screen.allowed_depths; i++) {
	pxp->depth[i]  = px_types[dev_type].depth[i];
	pxp->depth[i].which_depth = i;
    }

    for (i = 0; i < pxp->screen.nvisuals; i++) {
	pxp->visual[i] = px_types[dev_type].visual[i];
	pxp->visual[i].which_visual = i;
    }

    if (console_address == NULL) console_address = address;

    /*
     * get the screen, colormap, cursor and other functions over.
     */
    pxp->cf   		= px_types[dev_type].cf;
    pxp->cmf  		= px_types[dev_type].cmf;
    pxp->sf		= px_types[dev_type].sf;
    pxp->attach		= px_types[dev_type].attach;
    pxp->bootmsg	= px_types[dev_type].bootmsg;
    pxp->map		= px_types[dev_type].map;
    pxp->interrupt 	= px_types[dev_type].interrupt;
    pxp->getPacket	= px_types[dev_type].getPacket;
    pxp->sendPacket	= px_types[dev_type].sendPacket;
    pxp->getImageBuffer = px_types[dev_type].getImageBuffer;
    pxp->setup		= px_types[dev_type].setup;
    pxp->vmHook		= px_types[dev_type].vmHook;
    pxp->stic		= px_types[dev_type].stic;
    pxp->stamp		= px_types[dev_type].stamp;
    pxp->max_fbn	= px_types[dev_type].max_fbn;
    pxp->stamp_width	= px_types[dev_type].stamp_width;
    pxp->stamp_height	= px_types[dev_type].stamp_height;
    pxp->text_foreground= px_types[dev_type].text_foreground;
    pxp->text_background= px_types[dev_type].text_background;

    /*
     * initialize closures: screen, cursor, colormap ...
     */

    /* Pass dense space address of board to init_closure routines
     * because bt459s are mapped in dense space.
     */

    dense_addr = (caddr_t)
	    PHYS_TO_KSEG(DENSE(KSEG_TO_PHYS((vm_offset_t)address)));

    pxp->sf.sc = (*(pxp->sf.init_closure))(pxp->sf.sc, dense_addr, unit,
					   (int)px_types[dev_type].screen_type);

    pxp->cf.cc = (*(pxp->cf.init_closure))(pxp->cf.cc, dense_addr, unit,
					   (int)px_types[dev_type].cursor_type);

    pxp->cmf.cmc = (*(pxp->cmf.init_closure))(pxp->cmf.cmc, dense_addr, unit,
					     (int)px_types[dev_type].cmap_type);

    /*
     * must do more setup before calling ws_define_screen(), since *it*
     * may call ws_cons_init(), and we *MUST* be ready for *that*!!
     *
     * NOTE: stuff from here to ws_define_screen() came from px_cons_init()
     */
    if (pxp->setup && !((*pxp->setup)(pxp, address, 0, 0))) {
	    goto Out;			/* setup failed */
    }

    if (pxp->depth[0].depth == 0 &&
	/* probe framebuffer for depth */
	px_probe_fb(pxp, /*fb*/0, /*depth*/0) <= 0) {
	    /* fb probe failed */
	    goto Out;
    }

    /* for root visual, this is always true? */
    pxp->visual[0].depth = pxp->depth[0].depth;

    /* define pseudo-device ws */
    if (ws_define_screen(&pxp->screen, pxp->visual, pxp->depth, 
			 &pxp->sf, &pxp->cmf, &pxp->cf) < 0) {
	PX_DEBUG(PX_TALK,printf("px_attach(%x) failed\n", address);
                );
     Out:
	bzero((caddr_t)pxp, sizeof(px_info));
	return 0;
    }

    return 1;
}


px_which_option(address)
    caddr_t address;
{
#ifdef __alpha
    register int i;
    char *module = (char *) NULL;
    vm_offset_t phys = KSEG_TO_PHYS( address );

    for (i = 0; i < TC_IOSLOTS; i++) {
	if ( tc_slot[i].physaddr == phys ) {
	    module =  tc_slot[i].modulename;
	    break;
	}
    }
    if (module != (char *) NULL && *module != '\0') {
	for (i = 0; i < npx_types; i++) {
	    if (strncmp(px_types[i].screen.moduleID, module, TC_ROMNAMLEN) == 0)
		return i;
	}
    }
    else {
	printf("px - couldn't identify tc module\n");
    }

#else /* __alpha */
    register int i;
    char *module = NULL;

    for (i = 0; i < TC_OPTION_SLOTS; i++) {
	if((caddr_t)PHYS_TO_K1(tc_slot[i].physaddr) == address) {
	    module =  tc_slot[i].modulename;
	    break;
	}
    }
    if (module && *module != NULL) {
	for (i = 0; i < npx_types; i++) {
	    if (strncmp(px_types[i].screen.moduleID,module,TC_ROMNAMLEN) == 0)
		return i;
	}
    }
    else {
	printf("px - couldn't identify tc module\n");
    }

#endif /* __alpha */

    return -1;
}


/*
 * Initialize closure for screen.
 */
caddr_t
px_init_closure(closure, address, unit, type)
    caddr_t closure;
    caddr_t address;
    int unit;
    int type;
{
    px_info  *pxp = (px_info *) closure;
    register int i;

    pxp += unit;
    for (i = 0; i < pxp->screen.allowed_depths; i++) {
	pxp->depth[i].physaddr = 
	    address + (long)pxp->depth[i].physaddr;
	if (pxp->depth[i].plane_mask_phys)
	    pxp->depth[i].plane_mask_phys = 
		address + (long)pxp->depth[i].plane_mask_phys;
    }
    return (caddr_t)&pxp->screen;
}


/*
 * Graphics device interrupt service routine.
 * Cursor and/or colormap loading at end of frame interrupt gets done
 * by hardware specific interrupt routine.
 */
void
pxintr(unit)
    int unit;
{
    register px_info *pxp = (px_info *) &px_softc[unit];

    if (pxp->interrupt)
	(*pxp->interrupt)((struct controller *)pxinfo[unit], pxp);
}


/*
 * Scroll 1/8 of screen.
 */
px_scroll_screen(closure, screen)
    caddr_t closure;
    ws_screen_descriptor *screen;
{
    register px_info *pxp = (px_info *) closure;
    register int *pp, fY, tY, h;
    int scroll;
    int *stampPacket;

    scroll = screen->max_row >> 3; 
    fY = (scroll * screen->f_height) << 3; /* from */
    tY = 0 << 3;			   /* to */
					   /* # scanlines to move */
    h = (screen->max_row -scroll + 1) * screen->f_height;

    while (h > 0) {
        register int n = MIN(h, STAMP_MAX_CMDS);
        h -= n;
        stampPacket = pp = (*pxp->getPacket)(pxp);
        *pp++ = CMD_COPYSPANS | LW_PERPKT;
        *pp++ = (n<<24) | 0xffffff;
        *pp++ = 0x0;
        *pp++ = UPD_ENABLE | UMET_COPY | SPAN;
        *pp++ = 1;                      /* linewidth */

        for ( ; n > 0; n--, fY += 8, tY += 8) {
            *pp++ = (screen->max_col * screen->f_width) << 3;
            *pp++ = fY;                 /* x := 0 */
            *pp++ = tY;
        }
        wbflush();
        (*pxp->sendPacket)(pxp, stampPacket);

    }

    /* Clear out bottom scroll lines */
    px_rect(pxp, 0, (screen->max_row -scroll +1) * screen->f_height,
	    scroll * screen->f_height, screen->width,
	    0x0, 0);

    return (scroll-1);
}


/*
 * Clear the bitmap.
 */
px_clear_screen(closure, screen)
    caddr_t closure;
    ws_screen_descriptor *screen;
{
    px_rect((px_info *)closure, 0, 0, screen->height, screen->width, 0x0, 0);
    return 0;
}


/*
 * Output a character to the screen.
 */
px_blitc(closure, screen, row, col, c)
    caddr_t closure;
    ws_screen_descriptor *screen;
    int row, col;
    u_char c;
{
    register px_info *pxp = (px_info *)closure;
    register int i;
    register unsigned int two_rows;

    /*
     * 0xA1 to 0xFD are the printable characters added with 8-bit
     * support.
     */

    if (( c >= ' ' && c <= '~' ) || ( c >= 0xA1 && c <= 0xFD)) {
	register int f_height = screen->f_height;
	register int f_width = screen->f_width;
	int xpix = col * f_width;
	int ypix = row * f_height;

	i = c - ' ';
	if ( 0 < i && i <= 221 ) {
	    register unsigned short *f_row;
	    register int *pp;
	    int *stampPacket, *xyp, v1, v2, xya;

	    /* These are to skip the (32) 8-bit 
	     * control chars, as well as DEL 
	     * and 0xA0 which aren't printable */

	    i *= f_height;

	    f_row = &fg_font[i];
            stampPacket = pp = (*pxp->getPacket)(pxp);
            *pp++ = CMD_LINES | RGB_FLAT | XY_PERPRIM | LW_PERPRIM;
            *pp++ = 0x4ffffff;
            *pp++ = 0x0;
            *pp++ = UPD_ENABLE | WE_XYMASK | UMET_COPY;
            xyp = pp;

            /* fg = character to draw */
            *pp++ = PX_GET2ROWS(f_row);
            *pp++ = PX_GET2ROWS(f_row);
            *pp++ = PX_GET2ROWS(f_row);
            *pp++ = PX_GET2ROWS(f_row);
            *pp++ = PX_GET2ROWS(f_row);
            *pp++ = PX_GET2ROWS(f_row);
            *pp++ = PX_GET2ROWS(f_row);
            *pp++ = PX_GET2ROWS(f_row);
            i = (16<<2)-1;
            *pp++ = xya = CONSXYADDR(xpix, ypix);
            *pp++ = v1 =(   xpix                <<19) | ((ypix<<3)+i);
            *pp++ = v2 =((((xpix+f_width)<<3)-1)<<16) | (v1&0xffff);
            *pp++ = i;
            *pp++ = pxp->text_foreground;

            /* opaque background */
            *pp++ = *xyp++ ^ 0xffffffff;
            *pp++ = *xyp++ ^ 0xffffffff;
            *pp++ = *xyp++ ^ 0xffffffff;
            *pp++ = *xyp++ ^ 0xffffffff;
            *pp++ = *xyp++ ^ 0xffffffff;
            *pp++ = *xyp++ ^ 0xffffffff;
            *pp++ = *xyp++ ^ 0xffffffff;
            *pp++ = *xyp   ^ 0xffffffff;
            *pp++ = xya;
            *pp++ = v1;
            *pp++ = v2;
            *pp++ = i;
            *pp++ = pxp->text_background;
            xyp = pp;

            /* lower part of fg character */
            *pp++ = PX_GET2ROWS(f_row);
            *pp++ = PX_GET2ROWS(f_row);
            *pp++ = 0x0;
            *pp++ = 0x0;
            *pp++ = 0x0;
            *pp++ = 0x0;
            *pp++ = 0x0;
            *pp++ = 0x0;
            ypix += 16;
            *pp++ = xya = CONSXYADDR(xpix, ypix);
            *pp++ = v1 =(xpix<<19)|((ypix<<3)+( i=((f_height-16)<<2)-1) );
            *pp++ = v2 =((((xpix+f_width)<<3)-1)<<16) | (v1&0xffff);
            *pp++ = i;
            *pp++ = pxp->text_foreground;

            /* opaque background */
            *pp++ = *xyp++ ^ 0xffffffff;
            *pp++ = *xyp   ^ 0xffffffff;
            *pp++ =          0xffffffff;
            *pp++ =          0xffffffff;
            *pp++ =          0xffffffff;
            *pp++ =          0xffffffff;
            *pp++ =          0xffffffff;
            *pp++ =          0xffffffff;
            *pp++ = xya;
            *pp++ = v1;
            *pp++ = v2;
            *pp++ = i;
            *pp++ = pxp->text_background;
            wbflush();
            (*pxp->sendPacket)(pxp, stampPacket);
        }
	else if (i == 0) {			/* <space> */
	    px_rect(pxp, xpix, ypix, f_height, f_width, 0x0, 0);
	}
    }
    return 0;
}


/*
 * Initialize as console device.
 */
px_cons_init(address, slot)
caddr_t address;
int slot;
{
    register px_info *pxp = &px_softc[0];
    extern vm_offset_t px_map_pkt;                 /* packet/ring buffers */
    extern vm_offset_t px_map_inf;                 /* pxInfo structures */
    extern vm_offset_t px_soft_tlb;		   /* shadow tlb */
    extern vm_offset_t px_mem_bot;
    extern vm_offset_t px_mem_top;

#ifdef mips
    address = (caddr_t) ws_where_option("px");
#endif
#ifdef __alpha
    address = (caddr_t)PHYS_TO_KSEG(address);

	/*
	 * Since we are here, we know that an "px" was chosen as the
	 * graphics console; we want this one to be "px" controller #0
	 * for obvious reasons. Lets look for "px0" in the controller
	 * data structures, and make sure that it has the same slot or
	 * is wildcarded. If the latter, make it be hardwired to our slot
	 * from now on, so another px being normally probed at a later time
	 * will *NOT* be able to grab px0 out from under us (grin).
	 *
	 * HOWEVER, if there is no "px0" defined, or if it is hardwired
	 * to a different slot, record the fact that there is some kind
	 * of configuration error, so that when "wsopen" is called by a server
	 * to open "/dev/mouse", it will fail with an appropriate message.
	 */
	{
		struct controller *ctlr, *get_ctlr_num();
		
		if ((ctlr = get_ctlr_num("px", 0)) != NULL)
		{
			if (ctlr->slot == -1)
				ctlr->slot = slot;
			else if (ctlr->slot != slot)
			{
				/* printf("GFX HW/SW config problem\n"); */
				ws_graphics_config_error = 1;
			}
		}
		else
		{
			/* printf("GFX SW config problem: no px0 found\n"); */
			ws_graphics_config_error = 1;
		}
	}
#endif
    
    PX_DEBUG(PX_TALK,
	     printf("px_cons_init(0) -> %x\n", address);
	     );

    /*
     * Initialize px_info.ringbuffer and .px fields from kernel
     * memory acquired in mips_init.c.  If we do this initialization,
     * it will always be for unit 0 (the first ringbuffer and first info
     * area).  However, we must turn these physical memory addresses into
     * "virtual" (KSEG0) addresses.
     */
    pxp->ringbuffer = (int *) PHYS_TO_K0(px_map_pkt);
    pxp->px = (pxInfo *) PHYS_TO_K0(px_map_inf);

    if (px_attach(address, 0, 0)) {

	/* NOTE: a bunch of stuff moved from here to inside px_attach() */

	/*
	 * NOTE: px_clear_screen() came from pq_setup(), which is now
	 * being called (via pxp->setup) in px_attach(). the call to
	 * pxp->setup in px_attach() was moved from here (just above)...
	 */
	px_clear_screen( pxp, &pxp->screen ); /* clear the screen */

	/* NOTE: ws_cons_init() now done by ws_define_screen() in px_attach() */
	px_init_screen(pxp, &pxp->screen);
	px_scroll_screen(pxp, &pxp->screen);
	return 1;

     Out:
	/*
	 * ONLY if everything's kosher do we leave the struct px_info filled-in.
	 */
	bzero((caddr_t)pxp, sizeof(px_info));
    }
    return 0;				/* attach failed */
}


/*
 * Initialize the screen.
 */
px_init_screen(closure, sp)
    caddr_t closure;
    ws_screen_descriptor *sp;
{
    px_init_stic((px_info *)closure);
    return 0;
}


/*
 * Draw a rectangle:
 * x,y = top-left corner of rectangle
 * h, w = height, width
 */
void px_rect(pxp, x, y, h, w, rgb, buf)
    px_info *pxp;
    int x, y, h, w, rgb, buf;
{
    register int *ip, *stampPacket, lw;

    stampPacket = ip = (*pxp->getPacket)(pxp);
    *ip++ = CMD_LINES| RGB_CONST| LW_PERPKT;
    *ip++ = 0x1ffffff;
    *ip++ = 0x0;
    *ip++ = UPD_ENABLE| UMET_COPY| ((buf&0x3)<<3);

    lw = (h << 2) - 1;
    y = (y << 3) + lw;

    *ip++ = lw;
    *ip++ = rgb;                        /* rgb */
    *ip++ = ( x            <<19) | y;
    *ip++ = ((((x+w)<<3)-1)<<16) | y;
    wbflush();

    (*pxp->sendPacket)(pxp, stampPacket);
}


/*
 * Initialize the STIC.
 * This is non-destructive, so it may be called at any time.
 */
void px_init_stic(pxp)
    px_info *pxp;
{
    int module_type, xconfig, yconfig, config;
    stic_regs *stic = (stic_regs *) pxp->stic;

    /*
     *  initialize STIC registers
     */
    stic->sticsr = 0x00000030;			/* STIC CSR. */
    wbflush();
    DELAY(4000);				/* Wait for 4 milliseconds */
    stic->sticsr = 0x00000000;			/* Hit the STIC CSR again. */
    wbflush();
    stic->buscsr = 0xffffffff;			/* And the bus CSR. */
    wbflush();
    DELAY(20000);				/* Wait a long time... */

    /*
     *  Initialize Stamp configuration register
     */
    module_type = stic->modcl;
    xconfig = (module_type & 0x800) >> 11;
    yconfig = (module_type & 0x600) >> 9;
    config = (yconfig << 1) | xconfig;

    /*
     * Stamp0 config
     */
#ifdef __alpha
    *(int *)(pxp->stamp + 0x00160) = config;
    wbflush();
    *(int *)(pxp->stamp + 0x00168) = 0x0;
    wbflush();

    if (yconfig > 0) {
	/*
	 * Stamp1 config
	 */
	*(int *)(pxp->stamp + 0x20160) = 0x8 | config;
	wbflush();
	*(int *)(pxp->stamp + 0x20168) = 0x0;
	wbflush();
	if (yconfig > 1) {
	    /* stamp 2 & 3 config */
	}
    }
#else /* __alpha */
    *(int *)(pxp->stamp + 0x000b0) = config;
    *(int *)(pxp->stamp + 0x000b4) = 0x0;

    if (yconfig > 0) {
	/*
	 * Stamp1 config
	 */
	*(int *)(pxp->stamp + 0x100b0) = 0x8 | config;
	*(int *)(pxp->stamp + 0x100b4) = 0x0;
	if (yconfig > 1) {
	    /* stamp 2 & 3 config */
	}
    }
#endif /* __alpha */

    /*
     * Initialize STIC video registers
     */
    stic->vblank = (1024 << 16) | 1063;		/* vblank */
    stic->vsync = (1027 << 16) | 1030;		/* vsync */
    stic->hblank = (255 << 16) | 340;		/* hblank */
    stic->hsync2 = 245;				/* hsync2 */
    stic->hsync = (261 << 16) | 293;		/* hsync */
    stic->ipdvint = STIC_INT_CLR;		/* ipdvint */
    stic->sticsr = 0x00000008;			/* sticcsr */
    wbflush();

}


static
_allzero(ip, msk, cnt)
    u_int *ip;
    u_int msk, cnt;
{
    register int is0 = 1;

    for ( ; cnt--; ip++) {
	if (*ip & msk) {
	    is0 = 0;
	    PX_DEBUG(PX_YAK,
	    	     printf("_allzero: @ 0x%x = 0x%x\n", ip, *ip);
		     );
	    break;
	}
    }
    PX_DEBUG(PX_TALK,
    	     printf("_allzero: 0x%x msk 0x%x cnt %d -> %d\n",
		     ip, msk, cnt, is0);
	     );
    return is0;
}

px_probe_fb(pxp, buf, depth)
    px_info *pxp;
    int buf, depth;
{
    register int *pp, i;
    volatile int *ibuf;
    int *stampPacket, dstptr;
    int planes = 0;


    if (buf < 0)
	panic("px_probe_fb: buf < 0");
    if (buf > pxp->max_fbn) {
   	return 0;
    }
    if (depth >= NDEPTHS)
	panic("px_probe_fb: bad depth");
    if (pxp->getImageBuffer == NULL)
	panic("px_probe_fb: no getImageBuffer");

    /*
     * Find out whether there are 0, 8, 16, or 24 planes in buffer buf.
     *
     * Poke the frame buffer and check for corruption...
     */
    px_rect(pxp, 0,0, 1,0xf, 0x0, buf);

    /* wait for things to settle down... */
    DELAY(100);

    (*(pxp->getImageBuffer))(pxp, &ibuf, &dstptr);

    pp = (int *) ibuf;
    for (i = 0xf ; i != 0 ; i--)
        *pp++ = 0x0;                    /* clear out image buffer */
    wbflush();                          /* and read width word */

    stampPacket = pp = (*(pxp->getPacket))(pxp);
    *pp++ = CMD_READSPANS;
    *pp++ = 0x1ffffff;
    *pp++ = 0x0;
    *pp++ = UPD_ENABLE| UMET_NOOP| SPAN| ((buf&0x3)<<3)| ((buf&0x3)<<1);
    *pp++ = dstptr;
    *pp++ = 0xf<<3;                     /* width */
    *pp++ = 0x0;                        /* v1 */
    *pp++ = 0x0;                        /* pad */
    wbflush();
    (*(pxp->sendPacket))(pxp, stampPacket);

    /* wait for frame buffer read to complete... */
    DELAY(100);

    for ( i = 0; *(ibuf+0xf) == 0 && i < 250000; i++ ) {
	DELAY(3);
    }

    if ( i < 250000 )
    {
	PX_DEBUG(PX_YAK,
		 printf("px_probe_fb: ib 0x%x %d dst 0x%x\n",
			 ibuf, *(ibuf+0xf), dstptr);
		 );
	/*
	 * check the 3 groups of 8 planes.  each group that's not 0x00
	 * means reduce the number of planes by 8.  this is independent
	 * of where they decide to put the valid planes...
	 */
	if (_allzero(ibuf, 0x0000ff, 0xe))
	    planes += 8;
	if (_allzero(ibuf, 0x00ff00, 0xe))
	    planes += 8;
	if (_allzero(ibuf, 0xff0000, 0xe))
	    planes += 8;
    }
    else {
	PX_DEBUG(PX_TERSE,
		 printf("px_probe_fb: readspans timeout\n");
		 );
	return -1;
    }

    if (depth >= 0) {
	/* fill in depth[] */
	ws_depth_descriptor *dp = &pxp->depth[depth];
	dp->depth = planes;
	dp->bits_per_pixel = (planes > 8) ? 32 : 8; /* XXX */
    }
    PX_DEBUG(PX_TERSE,
	     printf("px_probe_fb: %d[%d]\n", planes, buf);
	     );
    return planes;
}


unsigned int vint;
px_cpu_idle(arg)
    px_info *arg;
{
#ifdef ultrix
    struct cpudata *pcpu=CURRENT_CPUDATA;
    int idleSample = pcpu->cpu_cptime[CP_IDLE];
    int i, liveN10s = 0;
    stic_regs *stic = (stic_regs *) px_softc[0].stic;

    for( i = 0 ; i < npx_softc ; i++ ) {
	register px_info *pxp = &px_softc[i];
	if (pxp->px__n10_present > 0) {
	    pxp->px__idle_count += 1;
	    pxp->px__idle_sample = idleSample;
	    liveN10s = 1;
	}
    }
    vint = stic->ipdvint;
    if (liveN10s)
	timeout(px_cpu_idle, (char *)0, PX_CPU_IDLE);
#else /* ultrix */
    /*
     * In the short-term, don't bother updating the idle time count or
     * samples when executing on OSF;  PEX should be able to survive the
     * missing "performance enhancement".
     */
#endif /* ultrix */
}
