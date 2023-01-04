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
static char	*sccsid =  "@(#)$RCSfile: fb.c,v $ $Revision: 1.2.10.3 $ (DEC) $Date: 1993/10/19 21:57:00 $";
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

/*
 * Modification History
 * 
 * April 1991		Joel Gringorten
 *
 *		Changes for OSF.
 *
 * November, 1989	Jim Gettys
 *
 *		Complete rewrite for multiscreen support.  Boy how kludges
 *		accumulate over time....
 *
 */

#include <io/dec/tc/tc.h>
#include <data/ws_data.c>
#include <data/fb_data.c>
#include <io/dec/ws/bt459.h>
#ifndef __alpha
#include <io/dec/ws/pmvdac.h>
#endif /* !__alpha */

#ifdef	__alpha
typedef	int	fb_entry_t;
#else	/* __alpha */
typedef	int	fb_entry_t;
#endif	/* __alpha */

#define FB_STRIDE	(sizeof(fb_entry_t)/sizeof(int))

int	fbprobe(), fbattach(), fbkint(), fbint();

static int map_framebuffer(), map_planemask();

caddr_t	fbstd[] = { 0 };

struct	driver fbdriver = 
        { fbprobe, 0, fbattach, 0, 0, fbstd, 0, 0, "fb", fbinfo };

extern int cpu;			/* have to do a minor amount of stuff */
extern  int     ws_display_type;
extern  int     ws_display_units;

extern caddr_t ws_map_region();

static int ok_to_print;

extern int ws_graphics_config_error;

/*
 * Probe to see if the graphic device will interrupt.
 */

fbprobe(nxv, ctlr)
char *nxv;
register struct controller *ctlr;
{
        /* 
	 * the initialization of the first screen is done through fb_cons_init,
	 * so if we have gotten this far we are alive so return a 1
	 */
	if ( BADADDR(nxv ,4, ctlr)) {
                printf ( "BADADDR returned an error.\n");
                return(0);
        }
	return(1);
}


fbtype(address)
	caddr_t address;
{
#ifdef	__alpha
	register int i;
	char *module = (char *) NULL;
	vm_offset_t phys;

	if ( svatophys( (vm_offset_t) address, &phys )
	     == KERN_INVALID_ADDRESS ) {
	    printf( "fb:  bad sva mapping of device.\n" );
	    return (-1);
	}

	for (i = 0; i < TC_IOSLOTS; i++) {
	    if (tc_slot[i].physaddr == phys ) {
		module = tc_slot[i].modulename;
		break;
	    }
	}

	if ( module == (char *) NULL ) {
	    printf("FB:  couldn't identify tc module\n");
	    return (-1);
	}

	for (i = 0; i < nfb_types; i++) {
	    if ( strncmp( fb_type[i].screen.moduleID, module, TC_ROMNAMLEN )
		 == 0 ) {
		    return i;
	    }
	}
	return -1;

#else	/* __alpha */

#define IS_MONO   (*((short *) PM_CSR_ADDR) & PM_CSR_MONO)
	register int i;
	char *module = "";
	if(cpu == DS_3100)
	    if(IS_MONO)
		module = "PMAX-MFB";  
	    else
	        module = "PMAX-CFB";
	else {
	    for (i = 0; i < TC_OPTION_SLOTS; i++) {
		if((caddr_t)PHYS_TO_K1(tc_slot[i].physaddr) == address) {
		    module =  tc_slot[i].modulename;
		    break;
		}
	    }
	}
	if(*module == NULL)
	    printf("fb - couldn't identify tc module\n");
	for (i = 0; i < nfb_types; i++)
	    if (strncmp(fb_type[i].screen.moduleID, module, TC_ROMNAMLEN) == 0){
                    if(ok_to_print) printf("fb: Module type %8s", module);
		    return i;
        }
	return -1;
#undef IS_MONO

#endif	/* __alpha */
}

/*
 * Routine to attach to the graphic device.
 */
fbattach(ctlr)
	register struct controller *ctlr;
{
        register struct fb_info *fbp = &fb_softc[ctlr->ctlr_num];
	int ret;

	ok_to_print = 1; /* OK now, only a prob 1st time in fb_attach */
	                 /* if it gets called from fb_cons_init */

	ret = fb_attach(ctlr->addr, ctlr->ctlr_num, ctlr->flags);
	if (fbp->attach)
		ret = (*fbp->attach)(ctlr);

	return ret;
}

/*
 * the routine that does the real work.  This is so the console can get
 * initialized before normal attach goes on.
 */

fb_attach(address, unit, flags)
	register caddr_t address;
	int unit;
	int flags;
{
	static caddr_t console_address = NULL;
        register struct fb_info *fbp = &fb_softc[unit];
	register int dev_type, m_type = flags;
	register int i;
	char val;

	if ((dev_type = fbtype(address)) == -1)
		return 0;

	/*
	 * deal with screen first, but only init screen structure once!
	 */
	if (console_address != address)
		fbp->screen = fb_type[dev_type].screen;
	fbp->screen.screen = unit;

	/*
	 * default to monitor type in screen structure unless flags set
	 */
	if (flags != 0) {
		if ((m_type < 0) || (m_type >= nmon_types)) m_type = 0;
		fbp->screen.monitor_type = monitor_type[m_type];
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
	for (i = 0; i < fbp->screen.allowed_depths; i++) {
		fbp->depth[i]  = fb_type[dev_type].depth[i];
		fbp->depth[i].which_depth = i;
	}

	for (i = 0; i < fbp->screen.nvisuals; i++) {
		fbp->visual[i] = fb_type[dev_type].visual[i];
		fbp->visual[i].which_visual = i;
	}
	/*
	 * get the screen, colormap, cursor and interrupt functions over
	 */
	fbp->sf   		= fb_type[dev_type].sf;
	fbp->cmf  		= fb_type[dev_type].cmf;
	fbp->cf   		= fb_type[dev_type].cf;
	fbp->attach		= fb_type[dev_type].attach;
	fbp->interrupt 		= fb_type[dev_type].interrupt;
        fbp->bot                = fb_type[dev_type].bot;

	fbp->sf.sc = (*(fbp->sf.init_closure))
		(fbp->sf.sc, address, unit, fb_type[dev_type].screen_type);
	fbp->cf.cc = (*(fbp->cf.init_closure))
		(fbp->cf.cc, address, unit, fb_type[dev_type].cursor_type);
	fbp->cmf.cmc = (*(fbp->cmf.init_closure))
		(fbp->cmf.cmc, address, unit,
			 fb_type[dev_type].color_map_type);

        if (fbp->bot) (*fbp->bot)(fbp);
	i = ws_define_screen(&fbp->screen, fbp->visual, fbp->depth, 
			 &fbp->sf, &fbp->cmf, &fbp->cf);
	if (console_address == NULL) console_address = address;
	if (i == -1) {
	  	printf("fb_driver: could not define screen\n");
	  	return(0);
	}

	/* set the global display type for sizer */
	if (ws_display_units < 8) 
	{
		ws_display_type = (ws_display_type << 8) | WS_DTYPE;
		ws_display_units = (ws_display_units << 1) | 1;
	}
  aend: if (ok_to_print)
            printf(" %dX%d\n", fbp->screen.width, fbp->screen.height);
	return 1;
}

caddr_t fb_init_closure (closure, address, unit, type)
	caddr_t closure;
	caddr_t address;
	int unit;
        int type;
{
	struct fb_info  *fbp = (struct fb_info *) closure;
	register int i;

	fbp = fbp + unit;
	for (i = 0; i < fbp->screen.allowed_depths; i++) {
		fbp->depth[i].physaddr = 
			address + (vm_offset_t)fbp->depth[i].physaddr;
		if (fbp->depth[i].plane_mask_phys)
			fbp->depth[i].plane_mask_phys = 
			   address + (vm_offset_t)fbp->depth[i].plane_mask_phys;
	}

	return (caddr_t) &fbp->screen;
}


/*
 * Graphics device end-of-frame interrupt service routine.
 * Cursor and/or colormap loading at end of frame interrupt gets done
 * by hardware specific interrupt routine.  For example, the 3MAX
 * color frame buffer provides a routine that implements this.
 */
fbint(unit)
	int unit;
{
	register struct fb_info *fp = &fb_softc[unit];
	register struct controller *ctlr = fbinfo[unit];

	if (fp->interrupt) {
	    (*fp->interrupt)(ctlr, fp);
	    return;
	}
}


/*
 * scroll screen one line; should work on both mono and color screens
 */

fb_scroll_screen(closure, screen)
	caddr_t closure;
	ws_screen_descriptor *screen;
{
	register struct fb_info *fbp = (struct fb_info *)closure;
	register ws_screen_descriptor *sp = &fbp->screen;
 	register ws_depth_descriptor *dp = &fbp->depth[sp->root_depth];
	register unsigned int *dest, *src, *spp, *dpp;
	register unsigned int temp0,temp1,temp2,temp3;
	register int i, j, wpl, wpfbl;
	int n, scroll;

	scroll = sp->max_row >> 3;
	/*
	 * compute # bits,  unroll to 4 words at a time
	 */
	wpl = (sp->f_width * sp->max_col * dp->bits_per_pixel) >> 7;
	wpfbl = (dp->fb_width * dp->bits_per_pixel) >> 5;
	n = (sp->max_row - scroll+1) * sp->f_height;
	spp = src = (unsigned int *)(dp->physaddr + 
			    (dp->fb_width * sp->f_height * scroll *
			     dp->bits_per_pixel >> 3));
	dpp = dest = (unsigned int *)(dp->physaddr);
	for (j = 0; j < n; j++) {
		src = spp;
		dest = dpp;
		i = 0;
		do {
			temp0 = src[0]; temp1 = src[1]; 
			temp2 = src[2]; temp3 = src[3];
			dest[0] = temp0; dest[1] = temp1;
			dest[2] = temp2; dest[3] = temp3;
			dest += 4; src += 4;
			i += 1;
		} while(i < wpl);
		spp += wpfbl;
		dpp += wpfbl;
	};
	/* Now zero out the bottom lines */
	n = sp->f_width * sp->max_col * dp->bits_per_pixel >> 3;
	spp = (unsigned int *)(dp->physaddr +
		      (dp->fb_width * sp->f_height *
		       (sp->max_row -scroll +1) *
		       dp->bits_per_pixel >> 3));
	for (j = sp->f_height * scroll; j > 0 ; j--) {
		bzero (spp, n);
		spp += wpfbl;
	}
	return (scroll-1);
}

/*
 * Clear the bitmap.  Should work for most screens.
 */
fb_clear_screen(closure, screen)
	caddr_t closure;
	ws_screen_descriptor *screen;
{
	register struct fb_info *fbp = (struct fb_info *)closure;
	register ws_screen_descriptor *sp = &fbp->screen;
 	register ws_depth_descriptor *dp = &fbp->depth[sp->root_depth];
	register int i, j, k, stride, nlongs;
	register unsigned int *dst;

	dst = (unsigned int *) (dp->physaddr);
	stride = ( dp->fb_width * dp->bits_per_pixel / 32 ) * FB_STRIDE;
	nlongs = sp->width * dp->bits_per_pixel / 32;

	for (i = 0; i < sp->height; i++, dst += stride) {
	  	for ( j = 0, k = 0; j < nlongs; j++, k += FB_STRIDE ) {
		   	dst[k] = 0;
		}
	}
}
static unsigned int fontmask_bits[16] = {
	0x00000000,
	0x00000001,
	0x00000100,
	0x00000101,
	0x00010000,
	0x00010001,
	0x00010100,
	0x00010101,
	0x01000000,
	0x01000001,
	0x01000100,
	0x01000101,
	0x01010000,
	0x01010001,
	0x01010100,
	0x01010101
};
/*
 * Font tables
 */
extern  char q_font[];

/*
 * put a character on the screen.  This routine is both depth and 
 * font dependent.
 */
fb_blitc(closure, screen, row, col, c) 
	caddr_t closure;
	ws_screen_descriptor *screen;
	int row, col;
	u_char c;
{
	register struct fb_info *fbp = (struct fb_info *)closure;
	register ws_screen_descriptor *sp = &fbp->screen;
 	register ws_depth_descriptor *dp = &fbp->depth[sp->root_depth];
	register unsigned int *pInt;
	register int i, j;
	register char *b_row, *f_row;
	register int ote;

	/*
	 * xA1 to XFD are the printable characters added with 8-bit
	 * support.
	 */
	/*if(( c >= ' ' && c <= '~' ) || ( c >= 0xA1 && c <= 0xFD))*/   {
		b_row = dp->physaddr + (row * dp->fb_width * sp->f_height)
			    + (col * sp->f_width);
		i = c - ' ';
		if( i < 0 || i > 221 ) i = 0;
		else 	{
			/* These are to skip the (32) 8-bit  control chars, 
			 * as well as DEL and 0xA0 which aren't printable 
			 */
			if (c > '~') i -= 34; 
		    	i *= 15;
		}
		f_row = &q_font[i];
		if (dp->bits_per_pixel == 8) {
			ote = dp->fb_width * FB_STRIDE / sizeof(int);
			pInt = (unsigned int *) (dp->physaddr);
			pInt += (row * dp->fb_width * sp->f_height * FB_STRIDE)
			      / sizeof(int);
			pInt += (col * sp->f_width * FB_STRIDE) / sizeof(int);
			/*
			 * fontmask_bits converts a nibble (4 bytes) to a long
			 * word containing 4 pixels corresponding to each bit
			 * in the nibble.
			 * Thus we write two longwords for each byte in font.
			 * 
			 * Remember the font is 8 bits wide and 15 bits high.
			 *
			 * We add 256 to the pointer to point to the pixel on
			 * the next scan line directly below the current pixel.
			 */
			for( j = 0; j < sp->f_height; j++) {
				pInt[0] = fontmask_bits[(*f_row)&0xf];
				pInt[FB_STRIDE] = fontmask_bits[((*f_row)>>4)&0xf];
				f_row++; 
				pInt += ote;
			}
		}
		else if (dp->bits_per_pixel == 1) {
			ote = dp->fb_width * dp->bits_per_pixel
			    * sizeof( fb_entry_t ) / 32;
			b_row = dp->physaddr
			      + ote * (row * sp->f_height & 0x3ff)
			      + sizeof( fb_entry_t ) * ( col / 4 )
			      + col % 4;
			*b_row = *f_row++; b_row += ote;
			*b_row = *f_row++; b_row += ote;
			*b_row = *f_row++; b_row += ote;
			*b_row = *f_row++; b_row += ote;
			*b_row = *f_row++; b_row += ote;
			*b_row = *f_row++; b_row += ote;
			*b_row = *f_row++; b_row += ote;
			*b_row = *f_row++; b_row += ote;
			*b_row = *f_row++; b_row += ote;
			*b_row = *f_row++; b_row += ote;
			*b_row = *f_row++; b_row += ote;
			*b_row = *f_row++; b_row += ote;
			*b_row = *f_row++; b_row += ote;
			*b_row = *f_row++; b_row += ote;
			*b_row = *f_row++; b_row += ote;
		}
	}
	return(0);
}

fb_map_unmap_screen(closure, depths, screen, mp)
	caddr_t closure;
	ws_depth_descriptor *depths;
	ws_screen_descriptor *screen;
	ws_map_control *mp;
{
	register struct fb_info *fbp = (struct fb_info *)closure;
	register ws_depth_descriptor *dp;

	int nbytes;

	/* unmap not yet (if ever) implemented */
	/* when this gets done, must find a way to save the depth */
	/* at which the screen was mapped, so it can be unmapped properly */
	if (mp->map_unmap == UNMAP_SCREEN)
		return (EINVAL);

	dp = depths + mp->which_depth;
	nbytes = ( dp->fb_width * dp->fb_height * dp->bits_per_pixel
		 * sizeof( fb_entry_t ) ) >> 5;
        dp->pixmap = ws_map_region(dp->physaddr, NULL, nbytes, 0600, (int *) NULL);
	if ( dp->pixmap == (caddr_t) NULL )
		return(ENOMEM);

#ifdef NOTDEF
	if ( dp->plane_mask_phys != (caddr_t) 0 ) {
            dp->plane_mask = ws_map_region(dp->plane_mask_phys, NULL, 4, 0600, (int*) NULL);
	    if ( dp->plane_mask == (caddr_t) NULL )
		  	return(ENOMEM);
	}
#endif NOTDEF
	return (0);
}

fb_cons_init(address, slot)
register caddr_t address;
int slot;
{
	int ret;
	extern caddr_t ws_where_option();

#ifdef mips
	if(cpu == DS_3100)
		address = (caddr_t)BITMAP_ADDR ;
	else {
		address = ws_where_option("fb");
		/* address == 0 if there is no cfb board in system */
		if (address == (caddr_t)0)
			return 0;
        }
        address = (caddr_t)PHYS_TO_K1(address);
#endif	/* mips */
#ifdef __alpha
	address = (caddr_t)PHYS_TO_KSEG(address);

	/*
	 * Since we are here, we know that an "fb" was chosen as the
	 * graphics console; we want this one to be "fb" controller #0
	 * for obvious reasons. Lets look for "fb0" in the controller
	 * data structures, and make sure that it has the same slot or
	 * is wildcarded. If the latter, make it be hardwired to our slot
	 * from now on, so another fb being normally probed at a later time
	 * will *NOT* be able to grab fb0 out from under us (grin).
	 *
	 * HOWEVER, if there is no "fb0" defined, or if it is hardwired
	 * to a different slot, record the fact that there is some kind
	 * of configuration error, so that when "wsopen" is called by a server
	 * to open "/dev/mouse", it will fail with an appropriate message.
	 */
	{
		struct controller *ctlr, *get_ctlr_num();
		
		if ((ctlr = get_ctlr_num("fb", 0)) != NULL)
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
			/* printf("GFX SW config problem: no fb0 found\n"); */
			ws_graphics_config_error = 1;
		}
		

	}
	
#endif

        ok_to_print = 0;
        ret = fb_attach(address, 0, 0);
        ok_to_print = 1;
/* NOTE: ws_cons_init() now done by ws_define_screen() in fb_attach() */

#ifdef mips
/* rpbfix: scroll so that console rom output is not munged */
        dprintf("\n");
#endif	/* mips */

        return (ret);
}

fb_init_screen() { return 0;}
