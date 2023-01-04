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
static char *rcsid = "@(#)$RCSfile: vga.c,v $ $Revision: 1.1.4.8 $ (DEC) $Date: 1993/11/23 21:25:59 $";
#endif

/* #define DEBUG_PRINT_ENTRY */
/* #define DEBUG_PRINT_CHAR */
/* #define DEBUG_PRINT_ADDRESS */
/* #define DEBUG_PRINT_PROBE */
/* #define DEBUG_TEST */
/* #define DEBUG_PRINT_INTERRUPT */

#include <data/ws_data.c>
#include <data/vga_data.c>

/* #define DPRINTF jprintf */
#define DPRINTF printf
/* #define DPRINTF xxputs */

#define VT100_EMUL

typedef int vga_entry_t;

int vgaprobe(), vgaattach(), vgakint(), vgaint();

caddr_t	vgastd[] = { 0 };

struct	driver vgadriver = 
        { vgaprobe, 0, vgaattach, 0, 0, vgastd, 0, 0, "vga", vgainfo };

/* for terminal emulation support */
int vga_charput(), vga_charclear(), vga_charmvup(), vga_charmvdown(),
 vga_charattr();

ws_emulation_functions vga_emul_funcs =
 { vga_charput, vga_charclear, vga_charmvup, vga_charmvdown, vga_charattr };

extern	int	cpu;
extern  u_int	printstate;
extern  int     ws_display_type;
extern  int     ws_display_units;

/* #define SCROLL_LINES(sp) (sp->max_row >> 3) */
#define SCROLL_LINES(sp)	(1)

extern  caddr_t ws_map_region();
extern  int	ws_graphics_config_error;

static  int	ok_to_print;

#define NULL_CONSOLE_ADDRESS ((caddr_t)0xfffffc00deadbeefL) /* FIXME */

/* macros for I/O register access */
#define INB(a,d)	d = read_io_port((a),1,BUS_IO)
#define OUTB(a,v)	{write_io_port((a),1,BUS_IO,(v));mb();}

int vga_delay = 1; 	/* 200000 */
#define VGA_DELAY()				\
  {						\
    volatile int i;				\
    for (i = vga_delay; i; i--);		\
  }

/*
 * Probe to see if the graphic device will interrupt.
 * This routine is called from the bus configuration code,
 *  via the "driver" data struct.
 */
int
vgaprobe(nxv, ctlr)
	char *nxv;
	register struct controller *ctlr;
{
#ifdef DEBUG_PRINT_ENTRY
DPRINTF("vgaprobe(0x%lx, 0x%lx): entry\n",
       (unsigned long)nxv, (unsigned long)ctlr);
#endif /* DEBUG_PRINT_ENTRY */

        /* 
	 * the initialization of the first screen is done through
	 * vga_cons_init, so if we have gotten this far, we are alive,
	 * so return a 1
	 */
#ifdef FIXME
	if ( BADADDR(nxv, 4, ctlr)) {
                printf("BADADDR returned an error.\n");
                return(0);
        }
#endif /* FIXME */
	return(1);
}

int
vgatype(addr)
	register caddr_t addr;
{
	register int i;

	/*
	 * for each vga_type table entry:
	 *    if its probe routine exists,
	 *        then call it
	 *        if the probe succeeds,
	 *            then return the table index as the "type"
	 */
	for (i = 0; i < nvga_types; i++) {
	    if (vga_type[i].af.probe) {
		if ((*vga_type[i].af.probe)(addr)) {
			return(i);
		}
	    }
	}

	return(-1);
}

/*
 * Routine to attach to the graphic device.
 * This routine is called from the bus configuration code,
 *  via the "driver" data struct.
 *
 * NOTE: this is the "cattach" (controller attach) entry point;
 *       its return value is never tested, so whatever the adapter's
 *       attach routine (if there is one) returns, doesn't matter.
 */
int
vgaattach(ctlr)
	register struct controller *ctlr;
{
        register struct vga_info *vp = &vga_softc[ctlr->ctlr_num];
	int ret;

#ifdef DEBUG_PRINT_ENTRY
DPRINTF("vgaattach: entry\n");
#endif /* DEBUG_PRINT_ENTRY */

	ok_to_print = 1; /* OK now, only a prob 1st time in vga_attach */
	                 /* if it gets called from vga_cons_init */

	/* Mask out the sysmap portion of the io handle so we have a bus   */
	/* local physical address. This works as long as we are on a 32bit */
	/* bus.								   */
	ret = vga_attach(((ulong_t)ctlr->physaddr & 0x0ffffffffL),
			 ctlr->ctlr_num, ctlr->flags);
	if (ret && vp->af.attach)
		ret = (*vp->af.attach)(ctlr);

	return ret;
}

/*
 * The routine that does the real work.  This is so the console can get
 *  initialized before normal attach goes on.
 * Called explicitly from vgaattach and vga_cons_init.
 *
 * NOTE: address passed in is the PHYSICAL address of the controller's slot
 */

int
vga_attach(address, unit, flags)
	register caddr_t address;
	int unit;
	int flags;
{
	static caddr_t console_address = NULL_CONSOLE_ADDRESS;
        register struct vga_info *vp = &vga_softc[unit];
	register int dev_type, m_type = flags;
	register int i;
	char val;

#ifdef DEBUG_PRINT_ENTRY
DPRINTF("vga_attach(0x%lx, 0x%x, 0x%x): entry\n", address, unit, flags);
#endif /* DEBUG_PRINT_ENTRY */

	if ((dev_type = vgatype(address)) == -1)
		return 0;

	/*
	 * deal with screen first, but only init screen structure once!
	 */
#ifdef DEBUG_PRINT_ADDRESS
DPRINTF("vga_attach: console_address 0x%lx  address 0x%lx\n",
       (unsigned long)console_address, (unsigned long)address);
#endif /* DEBUG_PRINT_ADDRESS */

	if (console_address != address)
		vp->screen = vga_type[dev_type].screen;
	vp->screen.screen = unit;

	/*
	 * default to monitor type in screen structure unless flags set
	 */
	if (flags != 0) {
		if ((m_type < 0) || (m_type >= nmon_types)) m_type = 0;
		vp->screen.monitor_type = monitor_type[m_type];
	}

	/*
	 * if we are the console, then we've already done general 
	 * initialization, so we shouldn't attempt to define another screen
	 * given that it's already been defined.  Note that we've dealt
	 * with screen type above.
	 */
        if (console_address == address)
		goto aend;

	/*
	 * then with the depths and visual types
	 */
	for (i = 0; i < vp->screen.allowed_depths; i++) {
		vp->depth[i] = vga_type[dev_type].depth[i];
		vp->depth[i].which_depth = i;
	}

	for (i = 0; i < vp->screen.nvisuals; i++) {
		vp->visual[i] = vga_type[dev_type].visual[i];
		vp->visual[i].which_visual = i;
	}

	/*
	 * get the screen, colormap, cursor and adapter functions over
	 */
	vp->sf   		= vga_type[dev_type].sf;
	vp->cmf  		= vga_type[dev_type].cmf;
	vp->cf   		= vga_type[dev_type].cf;
	vp->af			= vga_type[dev_type].af;

	vp->sf.sc = (*(vp->sf.init_closure))
		(vp->sf.sc, address, unit, vga_type[dev_type].screen_type);
	vp->cf.cc = (*(vp->cf.init_closure))
		(vp->cf.cc, address, unit, vga_type[dev_type].cursor_type);
	vp->cmf.cmc = (*(vp->cmf.init_closure))
		(vp->cmf.cmc, address, unit,
			 vga_type[dev_type].color_map_type);

        if (vp->af.bot) (*vp->af.bot)(vp);
	i = ws_define_screen(&vp->screen, vp->visual, vp->depth, 
			     &vp->sf, &vp->cmf, &vp->cf);
	if (console_address == NULL_CONSOLE_ADDRESS)
		console_address = address;
	if (i == -1) {
	  	printf("vga_driver: could not define screen\n");
	  	return(0);
	}

	/* set the global display type for sizer */
	if (ws_display_units < 8) 
	{
		ws_display_type = (ws_display_type << 8) | WS_DTYPE;
		ws_display_units = (ws_display_units << 1) | 1;
	}
  aend: if (ok_to_print)
            printf(" %dX%d\n", vp->screen.width, vp->screen.height);
	return 1;
}

/*
 * Graphics device end-of-frame interrupt service routine.
 * Cursor and/or colormap loading at end of frame interrupt gets done
 * by hardware specific interrupt routine.
 */
vgaintr(unit)
	int unit;
{
	register struct vga_info *vp = &vga_softc[unit];
	register struct controller *ctlr = vgainfo[unit];

#ifdef DEBUG_PRINT_INTERRUPT
DPRINTF("vgaintr: entry\n");
#endif /* DEBUG_PRINT_INTERRUPT */

	/*
	 * if specific adaptor has an interrupt routine (all *should*)
	 *  then call it...
	 */
	if (vp->af.interrupt)
		(*vp->af.interrupt)(ctlr, vp);

	else
		vga_interrupt(ctlr, vp);

}

int
vga_cons_init(address, slot)
	register caddr_t address;
	register int slot;
{
	register int ret;

#ifdef DEBUG_PRINT_ENTRY
DPRINTF("vga_cons_init(0x%lx, 0x%x): entry\n",
	(unsigned long)address, slot);
#endif /* DEBUG_PRINT_ENTRY */

#ifdef FIXME
	address = (caddr_t)PHYS_TO_KSEG(address);
#else
/**********************************************/
/* 
  	MAJOR HACK ALERT!!!
*/
/* we need to probe for the slot where the VGA */
/* controller lives which is the graphics console, */
/* since this is *BEFORE* bus */
/* probe time, and thus even ECU information is */
/* not readily available... major bummer... */
/**********************************************/

if (!address && !slot)
{
    register int i, slot_num, probe;
    register int start, end;
    register vm_offset_t slot_addr;

#define MAX_SLOTS 6		/* FIXME - what should this be? */

    /*
     * The device probe routines called from here need to see the address
     *  of the device in the controller data struct as 0
     *  so that they'll respond affirmatively *ONLY* for the *PRIMARY*
     *  VGA controller...
     */
    start = 1;
    end = MAX_SLOTS;
    ret = -1;

    for (slot_num = start; slot_num <= end; slot_num++) {

	slot_addr = (vm_offset_t)(slot_num * 0x1000);

	/*
	 * for each vga_type table entry:
	 *    if its probe routine exists,
	 *        then call it
	 *        if the probe succeeds,
	 *            then return the table index as the "type"
	 */
	for (i = 0; i < nvga_types; i++) {
	    if (vga_type[i].af.probe) {
		probe = (*vga_type[i].af.probe)(slot_addr);
		if (probe == 2) {
		    /* 2 == PRIMARY */
		    ret = i;
		    break; 
		}
	    }
	}
	if (ret != -1)
	    break;
    }
    if (ret != -1) {
	address = (caddr_t)slot_addr;	/* PHYSICAL!!! */
	slot = slot_num;
#ifdef DEBUG_PRINT_PROBE
DPRINTF("vga_cons_init: probe success, address 0x%lx  slot 0x%x \n",
	slot_addr, slot_num);
#endif /* DEBUG_PRINT_PROBE */
    }
    else
    {
#ifdef DEBUG_PRINT_PROBE
DPRINTF("vga_cons_init: probe unsuccessful... :-( :-(\n");
#endif /* DEBUG_PRINT_PROBE */
    }
}
else
{
#ifdef DEBUG_PRINT_PROBE
DPRINTF("vga_cons_init: on entry, address 0x%lx  slot 0x%x \n",
	(unsigned long)address, slot);
#endif /* DEBUG_PRINT_PROBE */
}
#endif /* FIXME */

	/*
	 * Since we are here, we know that an "vga" was chosen as the
	 * graphics console; we want this one to be "vga" controller #0
	 * for obvious reasons. Lets look for "vga0" in the controller
	 * data structures, and make sure that it has the same slot or
	 * is wildcarded. If the latter, make it be hardwired to our slot
	 * from now on, so another vga being normally probed at a later time
	 * will *NOT* be able to grab vga0 out from under us (grin).
	 *
	 * HOWEVER, if there is no "vga0" defined, or if it is hardwired
	 * to a different slot, record the fact that there is some kind
	 * of configuration error, so that when "wsopen" is called by a server
	 * to open "/dev/mouse", it will fail with an appropriate message.
	 */
	{
		struct controller *ctlr, *get_ctlr_num();
		
		if ((ctlr = get_ctlr_num("vga", 0)) != NULL)
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
			/* printf("GFX SW config problem: no vga0 found\n"); */
			ws_graphics_config_error = 1;
		}
	}

	/*
	 * Take "normal" device init path through vga_attach.
	 */
        ok_to_print = 0;
        ret = vga_attach(address, 0, 0);
        ok_to_print = 1;

	/*
	 * Register the VGA terminal emulation functions.
	 */
	if (ret) /* attach went OK... */
	{
		ws_register_emulation_functions(&vga_emul_funcs);
	}

/* NOTE: ws_cons_init() now called by ws_define_screen() in vga_attach() */

        return (ret);
}

/************************************************************************/
/*									*/
/*    SCREEN FUNCTIONS							*/
/*									*/
/************************************************************************/

caddr_t
vga_screen_init_closure (closure, address, unit, type)
	caddr_t closure;
	caddr_t address;
	int unit;
        int type;
{
	struct vga_info *vp = (struct vga_info *)closure;

#ifdef DEBUG_PRINT_ENTRY
DPRINTF("vga_screen_init_closure: entry\n");
#endif /* DEBUG_PRINT_ENTRY */

	vp = vp + unit;

	return (caddr_t) vp;
}

/*
 * Generic routine to init a VGA screen.
 * Could be called (directly or indirectly) from:
 *	ws_cons_init - for the console screen.
 *	wsopen       - for each screen, when going to graphics mode.
 *	wsputc       - for console screen, when panic printf needed.
 */
vga_init_screen(closure, screen)
	caddr_t closure;
	ws_screen_descriptor *screen;
{
	register struct vga_info *vp = (struct vga_info *)closure;
	register ws_screen_descriptor *sp = &vp->screen;

#ifdef DEBUG_PRINT_ENTRY
DPRINTF("vga_init_screen: entry\n");
#endif /* DEBUG_PRINT_ENTRY */

	(*vp->af.restore)(vga_new_state);  /* reset original parameters */

#ifdef VT100_EMUL
/* must scroll the 25th line up to the 24th, so its clear from now on */
/* but only need to do this when we are in text mode and on graphics console */
	if (vp->unit == 0 && !ws_is_mouse_on()) 
	{
		/*                      from,to,how_many */
		vga_charmvup(closure, sp, 80, 0, 80*24);
		/*                         from,how_many */
		vga_charclear(closure, sp, 80*24, 80);
	}
#endif /* VT100_EMUL */

	return 0;
}

/*
 * Generic routine to clear a VGA screen.
 *
 * NOTE: currently, this is *only* called from wsclose, when going
 *       from graphics mode to text mode, so it only needs to operate
 *	 in text mode.
 */
vga_clear_screen(closure, screen)
	caddr_t closure;
	ws_screen_descriptor *screen;
{
	struct vga_info *vp = (struct vga_info *)closure;
	register ws_screen_descriptor *sp = &vp->screen;

#ifdef DEBUG_PRINT_ENTRY
DPRINTF("vga_clear_screen: entry\n");
#endif /* DEBUG_PRINT_ENTRY */

#ifdef VT100_EMUL
	/*
	 * FIXME - this is hardwired to PC size screens with 25 rows...
	 *
	 * hardwire this to 80 cols by 25 rows, so that the ENTIRE screen
	 *  is initially cleared, even though we are only using 24 rows
	 */
	vga_charclear(closure, sp, 0, 80*25);
#else
	vga_charclear(closure, sp, TOP_LINE(sp), ONE_PAGE(sp));
#endif /* VT100_EMUL */
}

/*
 * Scroll Screen
 */
vga_scroll_screen(closure, screen)
	caddr_t closure;
	ws_screen_descriptor *screen;
{
	register struct vga_info *vp = (struct vga_info *)closure;
	register ws_screen_descriptor *sp = &vp->screen;
	register int scroll;
	
#ifdef DEBUG_PRINT_ENTRY_DISABLED
DPRINTF("vga_scroll_screen: entry\n");
#endif /* DEBUG_PRINT_ENTRY */

	scroll = 1; /* no. of lines to be scrolled */

	vga_charmvup(closure, sp, ROW_POS(sp, scroll), ROW_POS(sp, 0),
		     ONE_PAGE(sp) - scroll * ONE_LINE(sp));
	
	vga_charclear(closure, sp, ROW_POS(sp, sp->max_row - scroll),
		      scroll * ONE_LINE(sp));

	return (scroll-1);
}

vga_blitc(closure, sp, row, col, ch)
        caddr_t closure;
        register ws_screen_descriptor *sp;
        register int row, col, ch;
{
        vga_charput(closure, sp, CHAR_POS(sp, row, col), ch);
}

/*************************************************************************/
/* start of "kd"-style device-dependent routines			 */
/* 									 */
/* these are the routines which support terminal emulation for VGA	 */
/*************************************************************************/
/*
 * Put a character on the screen.
 */
vga_charput(closure, sp, pos, ch)
	caddr_t closure;
	register ws_screen_descriptor *sp;
	register int pos, ch;
{
	register struct vga_info *vp = (struct vga_info *)closure;
	register int *addr, data;
	register int attr = vp->attribute;

#ifdef DEBUG_PRINT_CHAR_DISABLED
DPRINTF("vga_charput(-, -, 0x%x, 0x%x, 0x%x)\n", pos, ch, attr);
#endif /* DEBUG_PRINT_CHAR */

	data = (attr << 8) | (ch & 0xff); /* endian */
	/* write 2 bytes (character + attribute) to VGA memory */
	write_io_port(vga_tx_addr + pos * 2, 2, BUS_MEMORY, data);
}

/*
 * Clear (set to blanks) a number of characters
 */
vga_charclear(closure, sp, from, count)
	caddr_t closure;
	register ws_screen_descriptor *sp;
	register int from, count;
{
	register struct vga_info *vp = (struct vga_info *)closure;
	register int i, data;
	register vm_offset_t addr, incr;
	register int attr = vp->attribute;

#ifdef DEBUG_PRINT_CHAR
DPRINTF("vga_charclear(-, -, 0x%x, 0x%x, 0x%x)\n", from, count, attr);
#endif /* DEBUG_PRINT_CHAR */

	data = (attr << 8 | ' '); /* endian :-) */

	/*
	 * clear a char+attr at a time...
	 */
	for (i = 0; i < count; i++)
	{
		write_io_port(vga_tx_addr + (from+i)*2, 2, BUS_MEMORY, data);
	}	
}

/*
 * Move screen characters upward.
 */
vga_charmvup(closure, sp, from, to, count)
	caddr_t closure;
	register ws_screen_descriptor *sp;
	register int from, to, count;
{
	register struct vga_info *vp = (struct vga_info *)closure;
	register int i, data, adjust;
	register vm_offset_t src, dst, incr;
	
#ifdef DEBUG_PRINT_CHAR
DPRINTF("vga_charmvup(-, -, 0x%x, 0x%x, 0x%x)\n", from, to, count);
#endif /* DEBUG_PRINT_CHAR */

	/*
	 * do them all...
	 */
	for (i = 0; i < count; i++)
	{
		data = read_io_port(vga_tx_addr + (from+i)*2, 2, BUS_MEMORY);
		write_io_port(vga_tx_addr + (to+i)*2, 2, BUS_MEMORY, data);
	}
}

/*
 * Move screen characters downward.
 */
vga_charmvdown(closure, sp, from, to, count)
	caddr_t closure;
	register int from, to, count;
{
	register struct vga_info *vp = (struct vga_info *)closure;
	register int i, data, adjust;
	register vm_offset_t src, dst, incr;
	
#ifdef DEBUG_PRINT_CHAR
DPRINTF("vga_charmvdown(-, -, 0x%x, 0x%x, 0x%x)\n", from, to, count);
#endif /* DEBUG_PRINT_CHAR */

	/*
	 * do them all...
	 */
	for (i = 0; i < count; i++)
	{
		data = read_io_port(vga_tx_addr + (from-i)*2, 2, BUS_MEMORY);
		write_io_port(vga_tx_addr + (to-i)*2, 2, BUS_MEMORY, data);
	}
}

/*
 * Set the attribute for screen characters.
 */
vga_charattr(closure, new_attr)
	caddr_t closure;
	register int new_attr;
{
	register struct vga_info *vp = (struct vga_info *)closure;

#ifdef DEBUG_PRINT_ENTRY
DPRINTF("vga_charattr(-, 0x%x)\n", new_attr);
#endif /* DEBUG_PRINT_CHAR */

	switch (new_attr) 
	{
	case ATTR_NORMAL:
		vp->attribute = vga_normal_attribute;
		break;
	case ATTR_REVERSE:
		vp->attribute = vga_reverse_attribute;
		break;
	default:
		vp->attribute = vga_normal_attribute;
		break;
	}
}

/*************************************************************************/
/* end of "kd"-style device-dependent routines */
/****************************************************************************/

/*
 * Map/Unmap Screen
 */
vga_map_unmap_screen(closure, depths, screen, mp)
	caddr_t closure;
	ws_depth_descriptor *depths;
	ws_screen_descriptor *screen;
	ws_map_control *mp;
{
	register struct vga_info *vp = (struct vga_info *)closure;

#ifdef DEBUG_PRINT_MAPPING
DPRINTF("vga_map_unmap_screen: entry\n");
#endif /* DEBUG_PRINT_MAPPING */

	/* unmap not yet (if ever) implemented) */
	if (mp->map_unmap == UNMAP_SCREEN)
		return (EINVAL);

	return (0);
}

vga_ioctl()
{
#ifdef DEBUG_PRINT_ENTRY
DPRINTF("vga_ioctl: entry\n");
#endif /* DEBUG_PRINT_ENTRY */
	return(0);
}

void
vga_close(closure)
	caddr_t closure;
{
	register struct vga_info *vp = (struct vga_info *)closure;

#ifdef DEBUG_PRINT_ENTRY
DPRINTF("vga_close: entry\n");
#endif /* DEBUG_PRINT_ENTRY */

	(*vp->af.restore)(vga_new_state);  /* reset original parameters */

/* FIXME - do we want to clear the screen here??? */
	vga_clear_screen(closure, vp->screen); /* clear it, also... */
}

/************************************************************************/
/*									*/
/*    CURSOR FUNCTIONS							*/
/*									*/
/************************************************************************/

caddr_t
vga_cursor_init_closure (closure, address, unit, type)
	caddr_t closure;
	caddr_t address;
	int unit;
        int type;
{
	struct vga_info *vp = (struct vga_info *)closure;
	register int i;

#ifdef DEBUG_PRINT_ENTRY
DPRINTF("vga_cursor_init_closure: entry\n");
#endif /* DEBUG_PRINT_ENTRY */

	vp = vp + unit;

	return (caddr_t)vp ;
}

vga_load_cursor()
{
#ifdef DEBUG_PRINT_ENTRY
DPRINTF("vga_load_cursor: entry\n");
#endif /* DEBUG_PRINT_ENTRY */
	return(0);
}

vga_recolor_cursor()
{
#ifdef DEBUG_PRINT_ENTRY
DPRINTF("vga_recolor_cursor: entry\n");
#endif /* DEBUG_PRINT_ENTRY */
	return(0);
}

/*
 * Routine to set the cursor position while in TEXT mode *only*.
 *  Called by hardware-specific routines when NOT in graphics mode.
 */
vga_set_cursor_position(closure, sp, x, y)
	caddr_t closure;
	ws_screen_descriptor *sp;
	register int x, y;
{
	register int offset;

#ifdef DEBUG_PRINT_ENTRY_DISABLED
DPRINTF("vga_set_cursor_position: entry\n");
#endif /* DEBUG_PRINT_ENTRY */

	/*
	 * since x,y has been scaled up by font width/height already,
	 *  we need to scale down by the same to get the actual col/row
	 *  that is how we set the cursor position with VGA... :-)
	 */
	if (x) x /= sp->f_width;
	if (y) y /= sp->f_height;

	offset = y * sp->max_col + x;

	OUTB(0x3D4, 0x0e); OUTB(0x3D5, (offset >> 8) & 0xff);
	OUTB(0x3D4, 0x0f); OUTB(0x3D5, offset & 0xff);

	return(0);
}

vga_cursor_on_off()
{
#ifdef DEBUG_PRINT_ENTRY
DPRINTF("vga_cursor_on_off: entry\n");
#endif /* DEBUG_PRINT_ENTRY */

	return(0);
}

/************************************************************************/
/*									*/
/*    COLOR MAP FUNCTIONS						*/
/*									*/
/************************************************************************/

caddr_t
vga_color_map_init_closure (closure, address, unit, type)
	caddr_t closure;
	caddr_t address;
	int unit;
        int type;
{
	struct vga_info *vp = (struct vga_info *)closure;
	register int i;

#ifdef DEBUG_PRINT_ENTRY
DPRINTF("vga_color_map_init_closure: entry\n");
#endif /* DEBUG_PRINT_ENTRY */

	vp = vp + unit;

	return (caddr_t)vp ;
}

vga_init_color_map()
{
#ifdef DEBUG_PRINT_ENTRY
DPRINTF("vga_init_color_map: entry\n");
#endif /* DEBUG_PRINT_ENTRY */
	return(0);
}

vga_load_color_map_entry()
{
#ifdef DEBUG_PRINT_ENTRY
DPRINTF("vga_load_color_map_entry: entry\n");
#endif /* DEBUG_PRINT_ENTRY */
	return(0);
}

void
vga_clean_color_map()
{
#ifdef DEBUG_PRINT_ENTRY
DPRINTF("vgaclean_color_map: entry\n");
#endif /* DEBUG_PRINT_ENTRY */
}

vga_video_on(closure)
	caddr_t closure;
{
	unsigned char state;

#ifdef DEBUG_PRINT_ENTRY
DPRINTF("vga_video_on: entry\n");
#endif /* DEBUG_PRINT_ENTRY */

	OUTB(0x3C4, 0x01);
	INB(0x3C5, state);
  
	state &= 0xDF;
  
	/*
	 * turn on video in Clocking Mode register
	 */
	OUTB(0x3C4, 0x01); OUTB(0x3C5, state); /* change mode */

	return(0);
}

vga_video_off(closure)
	caddr_t closure;
{
	unsigned char state;

#ifdef DEBUG_PRINT_ENTRY
DPRINTF("vga_video_off: entry\n");
#endif /* DEBUG_PRINT_ENTRY */

	OUTB(0x3C4, 0x01);
	INB(0x3C5, state);
  
	state |= 0x20;
  
	/*
	 * turn off video in Clocking Mode register
	 */
	OUTB(0x3C4, 0x01); OUTB(0x3C5, state); /* change mode */

	return(0);
}

/************************************************************************/
/*									*/
/*    MISCELLANEOUS FUNCTIONS						*/
/*									*/
/************************************************************************/

/*
 * Generic VGA enable interrupt processing.
 */
void
vga_enable_interrupt(vp)
	struct vga_info *vp;
{
	int data;

#ifdef DEBUG_PRINT_ENTRY
DPRINTF("vga_enable_interrupt: entry\n");
#endif /* DEBUG_PRINT_ENTRY */

	OUTB(0x3d4, 0x11); INB(0x3d5, data);

	/* clear disable bit, and set Normal */
	OUTB(0x3d4, 0x11); OUTB(0x3d5, (data | 0x10) & ~0x20);
}

/*
 * Generic VGA disable interrupt processing.
 */
void
vga_disable_interrupt(vp)
	struct vga_info *vp;
{
	int data;

#ifdef DEBUG_PRINT_ENTRY
DPRINTF("vga_disable_interrupt: entry\n");
#endif /* DEBUG_PRINT_ENTRY */

	OUTB(0x3d4, 0x11); INB(0x3d5, data);

	/* set disable/normal bits */
	OUTB(0x3d4, 0x11); OUTB(0x3d5, data | 0x30);
}

/*
 * Generic VGA clear interrupt processing.
 */
void
vga_clear_interrupt(vp)
	struct vga_info *vp;
{
	int data;

#ifdef DEBUG_PRINT_ENTRY
DPRINTF("vga_clear_interrupt: entry\n");
#endif /* DEBUG_PRINT_ENTRY */

	OUTB(0x3d4, 0x11); INB(0x3d5, data);
	OUTB(0x3d4, 0x11); OUTB(0x3d5, data & ~0x10); /* clear */
	OUTB(0x3d4, 0x11); OUTB(0x3d5, data | 0x10);  /* normal */
}

/*
 * Generic VGA interrupt processing.
 */
void
vga_interrupt(ctlr, vp)
	struct controller *ctlr;
	struct vga_info *vp;
{
#ifdef DEBUG_PRINT_INTERRUPT
DPRINTF("vga_interrupt: entry\n");
#endif /* DEBUG_PRINT_INTERRUPT */

	vga_clear_interrupt(vp);
	vga_disable_interrupt(vp);
}

/*
 * Generic VGA beginning-of-time processing.
 * Could be dispatched from "vga_attach" above.
 */
void
vga_bot(ctlr, vp)
	struct vga_info *vp;
{
#ifdef DEBUG_PRINT_ENTRY
DPRINTF("vga_bot: entry\n");
#endif /* DEBUG_PRINT_ENTRY */
/* nothing for now... */
}

/************************************************************************/
/*									*/
/*    Code from OSF snapshot - vga.c					*/
/*									*/
/************************************************************************/

/*
 *  VGA Device Driver
 *  
 *  Created 4/91 by Dean Anderson
 *
 */
/*
 * Copyright 1990,91 by Thomas Roell, Dinkelscherben, Germany.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Thomas Roell not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Thomas Roell makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 */

/*
 *-----------------------------------------------------------------------
 * vgaHWInit --
 *      Handle the initialization, etc. of a screen.
 *
 * Results:
 *      None.
 *
 * Side Effects: 
 *
 *-----------------------------------------------------------------------
 */

void
vgaHWInit(new)
  vgaHWRec *new;
{
  int i;

#ifdef DEBUG_PRINT_ENTRY
DPRINTF("vgaHWInit: entry\n");
#endif /* DEBUG_PRINT_ENTRY */
  
  /*
   * initialize default colormap for monochrome
   * FIXME FIXME - better defaults ????
   */
  new->DAC[0] = 0x00; new->DAC[1] = 0x00; new->DAC[2] = 0x1f; /* black */
  new->DAC[3] = 0xff; new->DAC[4] = 0xff; new->DAC[5] = 0xff; /* white */
  for (i=6; i<768; i++) new->DAC[i] = i/3; /* gray-scale ramp */

/* FIXME FIXME - init like JDU console */


/* FIXME FIXME - replaced algorithmically-determined register contents */
/* FIXME FIXME - code from OSF with hardwired values based on BOOK and */
/* FIXME FIXME - JDU console driver, for 80x25 text mode */
/* FIXME FIXME - This means that the state should be ready for chars */
/* FIXME FIXME - to be written into the "screen" memory at b8000 */

  new->MiscOutReg = 0x63;

  /*
   * Time Sequencer
   */
  new->Sequencer[0] = 0x00;
  new->Sequencer[1] = 0x01;
  new->Sequencer[2] = 0x03;
  new->Sequencer[3] = 0x00;				/* Font select */
  new->Sequencer[4] = 0x03; /* ?? alpha mode */		/* Misc */

  /*
   * CRTC Controller
   */
  new->CRTC[0]  = 0x5f;
  new->CRTC[1]  = 0x4f;
  new->CRTC[2]  = 0x50;
  new->CRTC[3]  = 0x82;
  new->CRTC[4]  = 0x55;
  new->CRTC[5]  = 0x81;
  new->CRTC[6]  = 0xbf;
  new->CRTC[7]  = 0x1f;
  new->CRTC[8]  = 0x00;
  new->CRTC[9]  = 0x4f; /* DOC 0xc7 */
  new->CRTC[10] = 0x01; /* DOC 0x06 */ /* cursor start */
  new->CRTC[11] = 0x0f; /* DOC 0x07 */ /* cursor end */
  new->CRTC[12] = 0x00; /* start addr high */
  new->CRTC[13] = 0x00; /* start addr low */
  new->CRTC[14] = 0x00;
  new->CRTC[15] = 0x00; /* DOC 0x59 */
  new->CRTC[16] = 0x9c;
  new->CRTC[17] = 0x3e; /* unlock CRTC regs 0-7, Vert int disable/normal */
  new->CRTC[18] = 0x8f;
  new->CRTC[19] = 0x28;
  new->CRTC[20] = 0x1f;
  new->CRTC[21] = 0x96;
  new->CRTC[22] = 0xb9;
  new->CRTC[23] = 0xa3;
  new->CRTC[24] = 0xff;

  /*
   * Graphics Display Controller
   */
  new->Graphics[0] = 0x00;
  new->Graphics[1] = 0x00;
  new->Graphics[2] = 0x00;
  new->Graphics[3] = 0x00;
  new->Graphics[4] = 0x00;
  new->Graphics[5] = 0x10;
  new->Graphics[6] = 0x0e;
  new->Graphics[7] = 0x00;
  new->Graphics[8] = 0xff;
  
  new->Attribute[0]  = 0x00; /* standard colormap translation */
  new->Attribute[1]  = 0x01;
  new->Attribute[2]  = 0x02;
  new->Attribute[3]  = 0x03;
  new->Attribute[4]  = 0x04;
  new->Attribute[5]  = 0x05;
  new->Attribute[6]  = 0x06;
  new->Attribute[7]  = 0x07;
  new->Attribute[8]  = 0x08;
  new->Attribute[9]  = 0x09;
  new->Attribute[10] = 0x0a;
  new->Attribute[11] = 0x0b;
  new->Attribute[12] = 0x0c;
  new->Attribute[13] = 0x0d;
  new->Attribute[14] = 0x0e;
  new->Attribute[15] = 0x0f;
  new->Attribute[16] = 0x00;
  new->Attribute[17] = 0x00;	/* background border */
  new->Attribute[18] = 0x0f;
  new->Attribute[19] = 0x00;
  new->Attribute[20] = 0x00;

  /*
   * now, copy the font information into its area
   */
  vga_copy_default_font(new);
  
}


/*
 *-----------------------------------------------------------------------
 * vgaHWRestore --
 *      restore a video mode
 *
 * Results:
 *      nope.
 *
 * Side Effects: 
 *      the display enters a new graphics mode. 
 *
 *-----------------------------------------------------------------------
 */
void
vgaHWRestore(restore)
     vgaHWPtr restore;
{
  register int i;
  register unsigned int *fp;
  vm_offset_t srcp, dstp;
  int data;

#ifdef DEBUG_PRINT_ENTRY
DPRINTF("vgaHWRestore: entry\n");
#endif /* DEBUG_PRINT_ENTRY */
  
  OUTB(0x3C2, restore->MiscOutReg);

  OUTB(0x3D4, 0x11); OUTB(0x3D5, 0x00);   /* unlock CRTC registers 0-7 */

/* FIXME FIXME ?? - kinda mucked this about a bit for the Sequencer regs... */
  OUTB(0x3C4, 0x00); OUTB(0x3C5, 0x01); /* FIXME - what JDU does... ?? */
  for (i=1; i<5; i++) { OUTB(0x3C4, i); OUTB(0x3C5, restore->Sequencer[i]); }
  OUTB(0x3C4, 0x00); OUTB(0x3C5, 0x03); /* now reenable the timing sequencer */

  INB(0x3DA, data); /* reset flip-flop */
  for (i=0; i<16; i++) { OUTB(0x3C0, i); OUTB(0x3C0, restore->Attribute[i]); }
  for (i=16; i<21;i++) { OUTB(0x3C0, i); OUTB(0x3C0, restore->Attribute[i]); }
  OUTB(0x3c0, 0x20); /* FIXME - 'cuz JDU does it this way */

  for (i=0; i<25; i++) { OUTB(0x3D4, i); OUTB(0x3D5, restore->CRTC[i]); }

  for (i=0; i<9;  i++) { OUTB(0x3CE, i); OUTB(0x3CF, restore->Graphics[i]); }
  
  OUTB(0x3C6,0xff); /* FIXME - added 'cuz in JDU code ??? */

  OUTB(0x3C8,0x00);
  for (i=0; i<768; i++) OUTB(0x3C9, restore->DAC[i]);

/* FIXME FIXME - moved this to *AFTER* the regs are restored ??? */
  /*
   * We MUST explicitly restore the font, since we (may have) entered 
   * graphics mode, which could have destroyed it.
   */
  if (((restore->Attribute[0x10] & 0x01) == 0)) {
    /*
     * here we switch temporarily to 16 color-plane-mode, to simply
     * copy the font-info
     *
     * BUGALLERT: The vga's segment-select register MUST be set appropriate !
     */
#ifdef FIXME
    INB(0x3DA, data); /* reset flip-flop */
    OUTB(0x3C0,0x10); OUTB(0x3C0, 0x01); /* graphics mode */
#endif /* FIXME */
    /* Sequencer registers */
    OUTB(0x3C4,0x02); OUTB(0x3C5, 0x04); /* write to plane 2 */
    OUTB(0x3C4,0x04); OUTB(0x3C5, 0x06); /* enable plane graphics */
    /* Graphics registers */
#ifdef FIXME
    OUTB(0x3CE,0x04); OUTB(0x3CF, 0x02); /* read plane 2 */
#endif /* FIXME */
    OUTB(0x3CE,0x05); OUTB(0x3CF, 0x00); /* write mode 0, read mode 0 */
    OUTB(0x3CE,0x06); OUTB(0x3CF, 0x05); /* set graphics */

    VGA_DELAY();

    fp = restore->FontInfo;
    dstp = VGA_FB_ADDR;
    for (i = 0; i < 2048; i++, dstp += sizeof(int)) {
	    write_io_port(dstp, 4, BUS_MEMORY, *fp++);
    }

#ifdef FIXME
    INB(0x3DA, data); /* reset flip-flop */
    OUTB(0x3C0,0x10); OUTB(0x3C0, restore->Attribute[0x10]);
#endif /* FIXME */
    /* Sequencer registers */
    OUTB(0x3C4,0x02); OUTB(0x3C5, restore->Sequencer[2]);
    OUTB(0x3C4,0x04); OUTB(0x3C5, restore->Sequencer[4]);
    /* Graphics registers */
#ifdef FIXME
    OUTB(0x3CE,0x04); OUTB(0x3CF, restore->Graphics[4]);
#endif /* FIXME */
    OUTB(0x3CE,0x05); OUTB(0x3CF, restore->Graphics[5]);
    OUTB(0x3CE,0x06); OUTB(0x3CF, restore->Graphics[6]);
  }

  VGA_DELAY();

  /*
   * Once the registers have been set,
   *
   * DO NOT DEPEND ON THE FIRMWARE FOR KERNEL PRINTFs!!!!
   */
  if (printstate & PROMPRINT) {
        printstate &= ~PROMPRINT;
        printstate |= CONSPRINT;
  }

#ifdef DEBUG_TEST
{
  int j;
  char c[2];
	
  DPRINTF("Dump out a screen's worth of characters:\n");

  j = 0x20;
  c[1] = DEFAULT_ATTRIBUTE; /* blue bg, white fg */

  for (i=0;i<2000;i++)
  {
      if (j>0x7e) j=0x20;
      c[0] = j++;
      vga_copy_to_phys(c, 0x000b8000 + (i<<1), 2);
  }

  DPRINTF("Completed: Dump out a screen's worth of characters\n");
}
#endif /*DEBUG_TEST*/

}

/*
 *-----------------------------------------------------------------------
 * vgaHWSave --
 *      save the current video mode
 *
 * Results:
 *      pointer to the current mode record.
 *
 * Side Effects: 
 *      None.
 *-----------------------------------------------------------------------
 */

vgaHWSave(save)
     vgaHWPtr save;
{
  register int i;
  int data;

#ifdef DEBUG_PRINT_ENTRY
DPRINTF("vgaHWSave: entry\n");
#endif /* DEBUG_PRINT_ENTRY */
  
  /*
   * Here we are, when we first save the videostate. This means we came here
   * to save the original Text (??) mode. Because some drivers may depend
   * on NoClock we set it here to a reasonable value.
   */
  INB(0x3CC, data);
  save->NoClock = (data >> 2) & 3;

  /*
   * now get the registers
   */
  INB(0x3CC, save->MiscOutReg);
  INB(0x3CA, save->General[1]);
  INB(0x3C2, save->General[2]);
  INB(0x3DA, save->General[3]);

  for (i=0; i<25; i++) { OUTB(0x3D4,i); INB(0x3D5, save->CRTC[i]); }

  for (i=0; i<5; i++) { OUTB(0x3C4,i); INB(0x3C5, save->Sequencer[i]); }
  for (i=0; i<9; i++) { OUTB(0x3CE,i); INB(0x3CF, save->Graphics[i]); }

  INB(0x3DA, data); /* reset flip-flop */
  for (i=0; i<16; i++) { OUTB(0x3C0,i); INB(0x3C1, save->Attribute[i]);
			 OUTB(0x3C0, save->Attribute[i]); }
  for (i=16; i<21; i++) { OUTB(0x3C0,i | 0x20); 
			  INB(0x3C1, save->Attribute[i]);
			  OUTB(0x3C0, save->Attribute[i]); }
  save->CRTC[17] &= 0x7F;  /* clear protect bit !!! */
  
  /*
   * assume we're still in "text" mode, so go out and read the
   *  first attribute from the text area, and squirrel it away...
   *
   * as a side effect, set vga_tx_addr to point to where the base address
   *  of the current text window will be...
   */
  vga_tx_addr = VGA_TX_ADDR;
  data = read_io_port(vga_tx_addr, 2, BUS_MEMORY);
  save->TextAttribute = (data >> 8) & 0xff;

  /*			 
   * save the colorlookuptable 
   */
  OUTB(0x3C7,0x00);
  for (i=0; i<768; i++) INB(0x3C9, save->DAC[i]);

#ifdef FONT_SAVE_ENABLED
  /*
   * get the character set of the first non-graphics application
   */
  if (((save->Attribute[0x10] & 0x01) == 0)) {
    /*
     * Here we switch temporarily to 16 color-plane-mode, to simply
     * copy the font-info
     *
     * BUGALLERT: The vga's segment-select register MUST be set appropriate !
     */
    INB(0x3DA, data); /* reset Attribute flip-flop */
    OUTB(0x3C0,0x10); OUTB(0x3C0, 0x01); /* graphics mode */
    /* Sequencer registers */
    OUTB(0x3C4,0x02); OUTB(0x3C5, 0x04); /* write to plane 2 */
    OUTB(0x3C4,0x04); OUTB(0x3C5, 0x06); /* enable plane graphics */
    /* Graphics registers */
    OUTB(0x3CE,0x04); OUTB(0x3CF, 0x02); /* read plane 2 */
    OUTB(0x3CE,0x05); OUTB(0x3CF, 0x00); /* write mode 0, read mode 0 */
    OUTB(0x3CE,0x06); OUTB(0x3CF, 0x05); /* set graphics */

    VGA_DELAY();

    vga_copy_from_phys(VGA_FB_ADDR, save->FontInfo, 8192);
    /*
     * Now set things back, so that save doesn't destroy as well
     */
    INB(0x3DA, data);
    OUTB(0x3C0,0x10); OUTB(0x3C0, save->Attribute[0x10]);
    /* Sequencer registers */
    OUTB(0x3C4,0x02); OUTB(0x3C5, save->Sequencer[0x2]);
    OUTB(0x3C4,0x04); OUTB(0x3C5, save->Sequencer[0x4]); 
    /* Graphics registers */
    OUTB(0x3CE,0x04); OUTB(0x3CF, save->Graphics[0x4]);
    OUTB(0x3CE,0x05); OUTB(0x3CF, save->Graphics[0x5]); 
    OUTB(0x3CE,0x06); OUTB(0x3CF, save->Graphics[0x6]);

    VGA_DELAY();
  }
#endif /* FONT_SAVE_ENABLED */

  return;
}

vgaHWPrint(tmp)
  vgaHWPtr tmp;
{
  int i;

/* FIXME FIXME - return DPRINTF to printf */

  DPRINTF("\nClock number (0x%lx) 0x%x\n", &tmp->NoClock, tmp->NoClock);
  DPRINTF("\nGeneral Registers: (0x%lx)\n  ", tmp->General);
  for (i=0; i<4; i++) DPRINTF("0x%x ", tmp->General[i]);
  
  DPRINTF("\nSequencer Registers: (0x%lx)\n  ", tmp->Sequencer);
  for (i=0; i<5; i++) DPRINTF("0x%x  ", tmp->Sequencer[i]);
  
  DPRINTF("\nCRTC Registers: (0x%lx)\n  ",tmp->CRTC);
  for (i=0; i<8; i++) DPRINTF("0x%x  ", tmp->CRTC[i]);
  DPRINTF("\n  ");
  for (i=8; i<16; i++) DPRINTF("0x%x  ", tmp->CRTC[i]);
  DPRINTF("\n  ");
  for (i=16; i<25; i++) DPRINTF("0x%x  ", tmp->CRTC[i]);

  
  DPRINTF("\nGraphics Registers: (0x%lx)\n  ",tmp->Graphics);
  for (i=0; i<9; i++) DPRINTF("0x%x  ", tmp->Graphics[i]);

  DPRINTF("\nAttribute Registers: (0x%lx)\n  ",tmp->Attribute);
  for (i=0; i<8; i++) DPRINTF("0x%x  ", tmp->Attribute[i]);
  DPRINTF("\n  ");
  for (i=8; i<16; i++) DPRINTF("0x%x  ", tmp->Attribute[i]);
  DPRINTF("\n  ");
  for (i=16; i<21; i++) DPRINTF("0x%x  ", tmp->Attribute[i]);
  DPRINTF("\n");

  DPRINTF("\n Text Attribute 0x%x\n", tmp->TextAttribute);

  return 0;
}

#ifdef FIXME

vga_setvideo(on)
{
  unsigned char state;

  OUTB(0x3C4, 0x01);
  INB(0x3C5, state);
  
  if (on) state &= 0xDF;
  else    state |= 0x20;
  
  /*
   * turn off screen if necessary
   */
  OUTB(0x3C4, 0x00); OUTB(0x3C5, 0x01);   /* syncronous reset */
  OUTB(0x3C4, 0x01); OUTB(0x3C5, state); /* change mode */
  OUTB(0x3C4, 0x00); OUTB(0x3C5, 0x03);   /* syncronous reset */
  
}

/*
 * Store a color in the color table
 * XXX There needs to be a way to determine this per card and
 *     relay that to the user level code.
 */
int vga_setcolor(data)
     struct vga_color *data;
{
  if (data->index < 0 || data->index > 255)
    return EINVAL;
  OUTB(0x3C8, data->index);
  OUTB(0x3C9, (unsigned char)data->r);
  OUTB(0x3C9, (unsigned char)data->g);
  OUTB(0x3C9, (unsigned char)data->b);
  return 0;
}


static strncmp(s1, s2, n)
     char *s1, *s2;
     int n;
{
  for( ; s1 == s2 && n; s1++, s2++, n--);
  if (n) return (int)(*s1 - *s2);
  else return 0;
}

/* 
 * nondestructively determine memory size in bytes 
 *   End of memory reached when VGASTRT found, or
 *   VGATEST written, but not read.
 */



int vga_get_memsize()
{
  int i;
  char t1[7];
  char t2[7];
  char tmp[7];
  int mode;
  int gdcmode;
  int misc;
  
  /* turn off video */
  OUTB(0x3c4, 0x01);  INB(0x3c5, mode);
  OUTB(0x3c4, 0x01);  OUTB(0x3c5, mode|20);
  /* turn on 256 color graphics mode, set segment size */
  OUTB(0x3ce, 0x05);  INB(0x3cf, gdcmode);
  OUTB(0x3ce, 0x05);  OUTB(0x3cf, gdcmode|40);
  /* turn on graphics mode, set memory map */
  OUTB(0x3c3, 0x06);  INB(0x3cf, misc);
  OUTB(0x3c3, 0x06);  OUTB(0x3cf, 0x01);
  
  vgaSetReadWrite(0);
  vga_copy_from_phys(VGA_FB_ADDR, t1, 7);
  vga_copy_to_phys("VGASTRT", VGA_FB_ADDR, 7);

  for (i=1; i < 64; i++) { /* XXX this should be set per device */
    vgaSetReadWrite(i);
    vga_copy_from_phys(VGA_FB_ADDR, t2, 7);
    if (!strncmp("VGASTRT", tmp, 7)) break;
    vga_copy_to_phys("VGATEST", VGA_FB_ADDR, 7);
    vga_copy_from_phys(VGA_FB_ADDR, tmp, 7);
    vga_copy_to_phys(t2, VGA_FB_ADDR, 7);
    if (strncmp("VGATEST", tmp, 7)) break;    
  }
  
  vgaSetReadWrite(0);
  vga_copy_to_phys(t1, VGA_FB_ADDR, 7);

  /* reset 256 color mode, segment size */
  OUTB(0x3ce, 0x05);  OUTB(0x3cf, gdcmode);
  /* reset graphics mode, memory map */
  OUTB(0x3ce, 0x06);  OUTB(0x3cf, misc);
  /* reset video */
  OUTB(0x3c4, 0x01);  OUTB(0x3c5, mode);

  printf("mem size %d %d\n", i, i * 0x20000);
  return ((i) * 0x20000);
}

#endif /* FIXME */

/************************************************************************/


vga_copy_to_phys(src, dst, siz)
    register unsigned char *src, *dst;
    register int siz;
{
	caddr_t psrc, pdst;

#ifdef DEBUG_PRINT_ENTRY_DISABLED
DPRINTF("vga_copy_to_phys(0x%lx, 0x%lx, 0x%x): entry\n", src, dst, siz);
#endif /* DEBUG_PRINT_ENTRY */

	/*
	 * do the transfer a byte (8-bits) at a time
	 */
	(void) svatophys(src, &psrc); /* MUST use "svatophys" */
	pdst = (caddr_t)get_io_handle(dst, 1, BUS_MEMORY);

	io_bcopy(psrc, pdst, siz);
}

vga_copy_from_phys(src, dst, siz)
    unsigned char *src, *dst;
    int siz;
{
	caddr_t psrc, pdst;

#ifdef DEBUG_PRINT_ENTRY_DISABLED
DPRINTF("vga_copy_from_phys(0x%lx, 0x%lx, 0x%x): entry\n", src, dst, siz);
#endif /* DEBUG_PRINT_ENTRY */

	/*
	 * do the transfer a byte (8-bits) at a time
	 */
	
	psrc = (caddr_t)get_io_handle(src, 1, BUS_MEMORY);
	(void) svatophys(dst, &pdst); /* MUST use "svatophys" */

	io_bcopy(psrc, pdst, siz);
}

/************************************************************************/

/*
 * routine to place the default font information into the state record
 */
vga_copy_default_font(new)
  vgaHWRec *new;
{
	register int i, j;
	register unsigned char *src, *dst;

#ifdef DEBUG_PRINT_ENTRY
DPRINTF("vga_copy_default_font: entry\n");
#endif /* DEBUG_PRINT_ENTRY */
	
	dst = (unsigned char *)&new->FontInfo[0];
	/*
	 * Load in the character fonts for the full range of characters
	 * which is 0x0-0xFF (0-255).
	 */
	for (i=0; i<256; ++i) {
		/*
		 * Initially specify the font for a "bogus" character.  This is
		 * used for non-printable characters.  It is an all F's glyph
		 * which should make it obvious on the display if you are
		 * attempting to display an invalid character.
		 */
		src = &vga8x16xx[0];

		/*
		 * This locates the character glyph for the conventional lower
		 * range 7-bit printable ascii characters.  This is for the
		 * space character up to the DEL (0177) character.
		 */
		if (i >= 0x20 && i <= 0x7F) {
			/*
			 * Figure out the offset into the set of character
			 * glyphs. This is done by subtracting the char code
			 * (i) from the first printable char code (0x20)
			 * to get the offset into the array. Then multiply
			 * by 16 because each character consists
			 * of 16 bytes; this computes the address of the glyph
			 * for (i).
			 */
#ifdef JDU_CONS_FONT
			src = &vga8x16gl[16 * (i-0x20)];
#else
			src = &q_font[15 * (i-0x20)]; /* q_font is 8x15 */
#endif /* JDU_CONS_FONT */
		}
#if LOADGR
		/*
		 * This sets a pointer to the character glyph for characters in
		 * upper range of printable characters.  These consist mostly of
		 * accent typed characters used in non-English.
		 */
		else if (i >= 0xA0 && i <= 0xFF) {
			src = &vga8x16gr[16 * (i-0xA0)];
		}
#endif
		/* At this point "src" points to the beginning byte offset
		 * for the glyph representing the character "i".
		 * Copy the 16 bytes used to represent that character
		 * out to the destination area.
		 */
#ifdef JDU_CONS_FONT
		bcopy(src, dst, 16);
		dst += 16;
#else
/* FIXME FIXME - font from qfont.c is bit-reversed here... :-( :-( */
		for (j = 0; j < 15; j++)
		{
			*dst++ = reverseByte(*src++);
		}
		*dst++ = 0;
#endif /* JDU_CONS_FONT */
		
		/* 16 bytes currently unused per 32-byte char entry */
		bcopy(vga8x16unused, dst, 16);
		dst += 16;
	}
}

int
reverseByte(b)
	int b;
{
        int j, k, t = 0;

        for (j = 0x01, k = 0x80; j < k; j += j, k >>= 1)
        {
		if (b & j) t |= k;
                if (b & k) t |= j;
        }
          
        return (t);

}
