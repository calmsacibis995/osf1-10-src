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
 *	@(#)$RCSfile: fb_data.c,v $ $Revision: 1.2.16.3 $ (DEC) $Date: 1993/11/17 23:12:49 $
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
#define _FB_DATA_C_

/************************************************************************
 * Modification History
 *
 * 11-May-91 -- Paul Grist
 *	
 *		Added defines for 3max+ (DS5000_300).
 *
 * 13-Oct-90 -- Randall Brown
 *	      
 *		Changed include of cpuconf.h to machine/common
 *
 * 09-Sep-90	Joel Gringorten
 * 
 * 		Added MFB (PMAG-AA)
 *
 * 23-Aug-90 -- Randall Brown
 * 
 *		Changed #ifdef DS5000 to also include DS5000_100 (3MIN)
 *
 * 15-Jan-90 -- Joel Gringorten
 *
 * 		Added PMAX CFB and MFB
 *
 * 15-Nov-89 -- Jim Gettys
 * 
 *	       Created.  From scratch.
 *
 ************************************************************************/

#include "fb.h"
#include <io/common/devio.h>
#include <sys/param.h>
#include <sys/conf.h>
/* #include <sys/dir.h> */
#include <sys/user.h>
#include <sys/workstation.h>
#include <sys/inputdevice.h>
#include <sys/wsdevice.h>
#include <sys/tty.h>
#include <sys/map.h>
#include <sys/buf.h>
#include <sys/vm.h>
/*#include <sys/bk.h>*/
#include <sys/clist.h>
#include <sys/file.h>
#include <sys/uio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/kernel.h>
#include <hal/cpuconf.h>
#include <sys/exec.h>
#include <machine/cpu.h>
#include <io/common/devdriver.h>
#include <sys/proc.h>
#include <sys/fbinfo.h>

/*#if NTC > 0 */
#include <io/dec/ws/bt459.h>		/* specific to BT459 VDAC 	*/
#include <io/dec/ws/bt431.h>
#include <io/dec/ws/bt463.h>
#include <io/dec/ws/pmagro.h>
#include <io/dec/ws/pmagbba.h>
#include <io/dec/ws/sfbparams.h>
#include <io/dec/ws/sfbregs.h>
#ifndef __alpha
#include <io/dec/ws/vfb03.h>		/* specific to 3max cfb module 	*/
#include <io/dec/ws/bt455.h>
#include <io/dec/ws/pmagaa.h>
#include <io/dec/ws/pmagdv.h>
#include <io/dec/ws/ims_g332.h>
#endif /* !__alpha */
/*#endif */

#ifdef DS3100
#include <io/dec/ws/pmvdac.h> 		/* PMAX def */
#endif

/*
 * The following defines up to three frame buffers.  PMAX (DECstation 3100
 * and 2100's can have only one screen. If, for example
 * some new frame buffer were built for 3MAX, it can be added without modifying
 * the base frame buffer driver by adding a new entry.  A third party or
 * other Digital frame buffer can be added by appropriate additions.
 * (I hope...)
 */
#if NTC > 0
#else
#ifdef DS3100
#include <io/dec/tc/tc.h>
struct tc_slot tc_slot[1];
#endif
#endif


/*
 * XXX visual closures must have pointer to device as first element, this
 *is a hack, but I can't think of a better one right now.
 */

struct fb_type {
	ws_screen_descriptor screen;
	ws_depth_descriptor depth[NDEPTHS];
	ws_visual_descriptor visual[NVISUALS];
	int screen_type;
	ws_cursor_functions cf;
	int cursor_type;
	ws_color_map_functions cmf;
	int color_map_type;
	ws_screen_functions sf;
	int (*attach)();
	void (*interrupt)();
	void (*bot)();          /* beginning of time (console init)*/
};


caddr_t fb_init_closure();
int fb_init_screen();
int fb_clear_screen();
int fb_scroll_screen();
int fb_blitc();
int fb_map_unmap_screen();
int fb_ioctl();

int sfbp_init_screen();

#ifdef BINARY

extern struct controller *fbinfo[];
extern struct fb_info fb_softc[];
extern struct fb_type fb_type[];

extern int nfb_types;

#else /**binary*/

#if NFB > 0

/* the flags field from the configuration is used to set the monitor type */

struct fb_info fb_softc[NFB];
struct fb_type fb_type[] = {
#ifdef DS3100
{
	/* PMAX COLOR */
        {
	0, 
        MONITOR_VR297,
	"PMAX-CFB",
	1024, 864,		/* width, height */
	0,			/* depth */
	1,			/* number of depths present		*/
	1,			/* number of visual types of screen 	*/
	0, 0,			/* current pointer position 		*/
	0, 0,			/* current text position		*/
	56, 80,			/* maximum row, col text position 	*/
	8, 15,			/* console font width and height	*/
	16, 16,			/* maximal size cursor for screen	*/
	1, 1,			/* min, max of visual types		*/
	},

	{			/* depth descriptor of root window */
	0, 0, 			/* which screen and depth		*/
	1024, 1024,		/* frame buffer size in pixels		*/
	8,			/* returns the depth (out)		*/
	8,			/* stride of pixel (out)		*/
	32,			/* scan line pad			*/
    	(caddr_t) 0,		/* bitmap starts at beginning */
	0,			/* only filled in when mapped		*/
	(caddr_t)(PLANE_REG - FRAME_BUF), /*plane mask offset */
	0,			/* only filled in when mapped		*/
	},

	{			/* visual descriptor */
	0,			/* which screen (in)			*/
    	0,			/* which visual of screen (in) 		*/
	PseudoColor,		/* class of visual 			*/
	8,			/* number of bits per pixel		*/
	0, 0, 0,		/* zero since pseudo; mask of subfields */
	8,			/* bits per RGB 			*/
	256,			/* color map entries */
    	},
        0,                      /* screen type */
	{
	pmax_init_closure,
	pmax_load_cursor,
	pmax_recolor_cursor,
	pmax_set_cursor_position,
	pmax_cursor_on_off,
	(caddr_t)pm_softc,
        },
        0,                      /* cursor type */
	{
	pmax_init_closure,
	pmax_init_color_map,
	pmax_load_color_map_entry,
        NULL,
	pmax_video_on,
	pmax_video_off,
	(caddr_t)pm_softc,
	},
        0,                      /* cmap type */
	{
	fb_init_closure,
	fb_init_screen,
	fb_clear_screen,
	fb_scroll_screen,
	fb_blitc,
	fb_map_unmap_screen,
	NULL,			/* ioctl optional */
	NULL,			/* close optional */
	(caddr_t)fb_softc,
	},
	NULL,			/* not yet interrupt driven */
	NULL,			/* not yet interrupt driven */
        NULL,                   /* beginning of time */
},
{
	/* PMAX */
        {
	0, 
        MONITOR_VR260,
	"PMAX-MFB",
	1024, 864,		/* width, height */
	0,			/* depth */
	1,			/* number of depths present		*/
	1,			/* number of visual types of screen 	*/
	0, 0,			/* current pointer position 		*/
	0, 0,			/* current text position		*/
	56, 80,			/* maximum row, col text position 	*/
	8, 15,			/* console font width and height	*/
	16, 16,			/* maximal size cursor for screen	*/
	1, 1,			/* min, max of visual types		*/
	},

	{			/* depth descriptor of root window */
	0, 0, 			/* which screen and depth		*/
	2048, 1024,		/* frame buffer size in pixels		*/
	1,			/* returns the depth (out)		*/
	1,			/* stride of pixel (out)		*/
	32,			/* scan line pad			*/
    	(caddr_t) 0,		/* bitmap starts at beginning */
	0,			/* only filled in when mapped		*/
	(caddr_t)(PLANE_REG - FRAME_BUF), /*plane mask offset */
	0,			/* only filled in when mapped		*/
	},

	{			/* visual descriptor */
	0,			/* which screen (in)			*/
    	0,			/* which visual of screen (in) 		*/
	StaticGray,		/* class of visual 			*/
	8,			/* number of bits per pixel		*/
	0, 0, 0,		/* zero since pseudo; mask of subfields */
	8,			/* bits per RGB 			*/
	256,			/* is this correct? */
    	},
	0,
	{
	pmax_init_closure,
	pmax_load_cursor,
	pmax_recolor_cursor,
	pmax_set_cursor_position,
	pmax_cursor_on_off,
	(caddr_t)pm_softc,
        },
	0,
	{
	pmax_init_closure,
	pmax_init_color_map,
	pmax_load_color_map_entry,
        NULL,
	pmax_video_on,
	pmax_video_off,
	(caddr_t)pm_softc,
	},
	0,
	{
	fb_init_closure,
	fb_init_screen,
	fb_clear_screen,
	fb_scroll_screen,
	fb_blitc,
	fb_map_unmap_screen,
	NULL,			/* ioctl optional */
	NULL,			/* close optional */
	(caddr_t)fb_softc,
	},
	NULL,			/* not yet interrupt driven */
	NULL,			/* not yet interrupt driven */
        NULL,                   /* beginning of time */
},
#endif
#ifndef __alpha
/*#if NTC > 0 */
{	
        {
	0, 			/* screen number (in) */
        MONITOR_VR297,
	"PMAG-BA ", 		/* exact string in 3MAX rom option ID	*/
	1024, 864,		/* width, height */
	0,			/* depth */
	1,			/* number of depths present		*/
	1,			/* number of visual types of screen 	*/
	0, 0,			/* current pointer position 		*/
	0, 0,			/* current text position		*/
	56, 80,			/* maximum row, col text position 	*/
	8, 15,			/* console font width and height	*/
	64, 64,			/* maximal size cursor for screen	*/
	1, 1,			/* min, max of visual types		*/
	},

	{			/* depth descriptor of root window */
	0, 0, 			/* which screen and depth		*/
	1024, 864,		/* frame buffer size in pixels		*/
	8,			/* returns the depth (out)		*/
	8,			/* stride of pixel (out)		*/
	32,			/* scan line pad			*/
    	VFB03_FB_OFFSET,	/* frame buffer starts at 0 in fb slot  */
	0,			/* only filled in when mapped		*/
	0,			/* no plane mask on 3MAX CFB, or offset */
	0,			/* only filled in when mapped		*/
	},

	{			/* visual descriptor */
	0,			/* which screen (in)			*/
    	0,			/* which visual of screen (in) 		*/
	PseudoColor,		/* class of visual 			*/
	8,			/* number of bits per pixel		*/
	0, 0, 0,		/* zero since pseudo; mask of subfields */
	8,			/* bits per RGB 			*/
	256,			/* color map entries */
    	},
        0,                      /* screen */
	{
	bt_init_closure,
	bt_load_cursor,
	bt_recolor_cursor,
	bt_set_cursor_position,
	bt_cursor_on_off,
	(caddr_t)bt459_softc,
        },
        BT459_CX_TYPE,          /* cursor */
	{
	bt_init_closure,
	bt_init_color_map,
	bt_load_color_map_entry,
        NULL,
	bt_video_on,
	bt_video_off,
	(caddr_t)bt459_softc,
	},
        BT459_CX_TYPE,          /* cmap */
	{
	fb_init_closure,
	fb_init_screen,
	fb_clear_screen,
	fb_scroll_screen,
	fb_blitc,
	fb_map_unmap_screen,
	NULL,			/* rasterops diag only */
	NULL,			/* close optional */
	(caddr_t)fb_softc,
	},	
	vfb03_attach,
	vfb03_interrupt,
        NULL,                   /* beginning of time */
    },
    {
        {
	0, 			/* screen number (in) */
        MONITOR_VR297,
	"PMAG-AA ", 		/* exact string in 3MAX rom option ID	*/
	1280, 1024,		/* width, height */
	0,			/* depth */
	1,			/* number of depths present		*/
	1,			/* number of visual types of screen 	*/
	0, 0,			/* current pointer position 		*/
	0, 0,			/* current text position		*/
	67, 80,			/* maximum row, col text position 	*/
	8, 15,			/* console font width and height	*/
	64, 64,			/* maximal size cursor for screen	*/
	1, 1,			/* min, max of visual types		*/
	},

	{			/* depth descriptor of root window */
	0, 0, 			/* which screen and depth		*/
	2048, 1024,		/* frame buffer size in pixels		*/
	1,			/* returns the depth (out)		*/
	8,			/* stride of pixel (out)		*/
	32,			/* scan line pad			*/
    	(caddr_t)PMAGAA_FB_OFFSET,  /* frame buffer starts at 0 in fb slot  */
	0,			/* only filled in when mapped		*/
	0,			/* no plane mask on 3MAX CFB, or offset */
	0,			/* only filled in when mapped		*/
	},

	{			/* visual descriptor */
	0,			/* which screen (in)			*/
    	0,			/* which visual of screen (in) 		*/
	StaticGray,		/* class of visual 			*/
	8,			/* number of bits per pixel		*/
	0, 0, 0,		/* zero since pseudo; mask of subfields */
	8,			/* bits per RGB 			*/
	2,			/* color map entries */
    	},
	0,
	{
	pmagaa_bt431_init_closure,
	bt431_load_cursor,
	pmagaa_recolor_cursor,
	bt431_set_cursor_position,
	bt431_cursor_on_off,
	(caddr_t)bt431_softc,
        },
	0,
	{
	pmagaa_bt455_init_closure,
	bt455_init_color_map,
	bt455_load_color_map_entry,
        NULL,
	pmagaa_video_on,
	pmagaa_video_off,
	(caddr_t)bt455_softc,
	},
	0,
	{
	fb_init_closure,
	fb_init_screen,
	fb_clear_screen,
	fb_scroll_screen,
	fb_blitc,
	fb_map_unmap_screen,
	NULL,			/* ioctl optional */
	NULL,			/* close optional */
	(caddr_t)fb_softc,
	},	
	pmagaa_attach,		/* attach */
	pmagaa_interrupt,	/* interrupt */
        NULL,                   /* beginning of time */
    },
#endif /* !__alpha */
    {
        {
	0, 			/* screen number (in) */
        MONITOR_VRT19,
	"PMAG-RO ", 		/* exact string in 3MAX rom option ID	*/
	1280, 1024,		/* width, height */
	0,			/* depth */
	1,			/* number of depths present		*/
	1,			/* number of visual types of screen 	*/
	0, 0,			/* current pointer position 		*/
	0, 0,			/* current text position		*/
	67, 80,			/* maximum row, col text position 	*/
	8, 15,			/* console font width and height	*/
	64, 64,			/* maximal size cursor for screen	*/
	1, 1,			/* min, max of visual types		*/
	},

	{			/* depth descriptor of root window */
	0, 0, 			/* which screen and depth		*/
	1280, 1024,		/* frame buffer size in pixels		*/
	8,			/* returns the depth (out)		*/
	8,			/* stride of pixel (out)		*/
	32,			/* scan line pad			*/
    	(caddr_t)TCO_EIGHT_BIT,  /* start of module. map whole shebang.*/
	0,			/* only filled in when mapped		*/
	0,			/* no plane mask  */
	0,			/* only filled in when mapped		*/
	},

	{			/* visual descriptor */
	0,			/* which screen (in)			*/
    	0,			/* which visual of screen (in) 		*/
	PseudoColor,		/* class of visual 			*/
	8,			/* number of bits per pixel		*/
	0, 0, 0,		/* zero since pseudo; mask of subfields */
	8,			/* bits per RGB 			*/
	512,			/* color map entries */
    	},
        0,                      /* screen */
	{
	pmagro_bt431_init_closure,
	pmagro_load_cursor,
	pmagro_recolor_cursor,
	bt431_set_cursor_position,
	bt431_cursor_on_off,
	(caddr_t)bt431_softc,
        },
        1,                      /* cursor */
	{
	pmagro_bt463_init_closure,
	bt463_init_color_map,
	bt463_load_color_map_entry,
        NULL,
	pmagro_video_on,
	pmagro_video_off,
	(caddr_t)bt463_softc,
	},
        0,                      /* cmap */
	{
	fb_init_closure,
	fb_init_screen,
	fb_clear_screen,
	fb_scroll_screen,
	fb_blitc,
	pmagro_map_screen  /* fb_map_unmap_screen */,
	pmagro_ioctl,			/* ioctl optional */
	NULL,			/* close optional */
	(caddr_t)fb_softc,
	},	
	pmagro_attach,		/* attach */
	pmagro_interrupt,	/* interrupt */
        NULL,
    },
    {
        {
	0, 			/* screen number (in) */
        MONITOR_VRT19,
	"PMAG-JA ", 		/* exact string in 3MAX rom option ID	*/
	1280, 1024,		/* width, height */
	0,			/* depth */
	1,			/* number of depths present		*/
	1,			/* number of visual types of screen 	*/
	0, 0,			/* current pointer position 		*/
	0, 0,			/* current text position		*/
	67, 80,			/* maximum row, col text position 	*/
	8, 15,			/* console font width and height	*/
	64, 64,			/* maximal size cursor for screen	*/
	1, 1,			/* min, max of visual types		*/
	},

	{			/* depth descriptor of root window */
	0, 0, 			/* which screen and depth		*/
	1280, 1024,		/* frame buffer size in pixels		*/
	8,			/* returns the depth (out)		*/
	8,			/* stride of pixel (out)		*/
	32,			/* scan line pad			*/
    	(caddr_t)TCO_EIGHT_BIT,  /* start of module. map whole shebang.*/
	0,			/* only filled in when mapped		*/
	0,			/* no plane mask  */
	0,			/* only filled in when mapped		*/
	},

	{			/* visual descriptor */
	0,			/* which screen (in)			*/
    	0,			/* which visual of screen (in) 		*/
	PseudoColor,		/* class of visual 			*/
	8,			/* number of bits per pixel		*/
	0, 0, 0,		/* zero since pseudo; mask of subfields */
	8,			/* bits per RGB 			*/
	512,			/* color map entries */
    	},
        0,                      /* screen */
	{
	pmagro_bt431_init_closure,
	pmagro_load_cursor,
	pmagro_recolor_cursor,
	bt431_set_cursor_position,
	bt431_cursor_on_off,
	(caddr_t)bt431_softc,
        },
        1,                      /* cursor */
	{
	pmagro_bt463_init_closure,
	bt463_init_color_map,
	bt463_load_color_map_entry,
        NULL,
	pmagro_video_on,
	pmagro_video_off,
	(caddr_t)bt463_softc,
	},
        0,                      /* cmap */
	{
	fb_init_closure,
	fb_init_screen,
	fb_clear_screen,
	fb_scroll_screen,
	fb_blitc,
	pmagro_map_screen  /* fb_map_unmap_screen */,
	pmagro_ioctl,			/* ioctl optional */
	NULL,			/* close optional */
	(caddr_t)fb_softc,
	},	
	pmagro_attach,		/* attach */
	pmagro_interrupt,	/* interrupt */
        NULL,                   /* beginning of time */
    },
#ifndef __alpha
    {
        /* maxine */
        {
        0,      /* screen (in) */
        MONITOR_VR297,
        "PMAG-DV ",
        1024, 768,              /* width, height */
        0,                      /* depth */
        1,                      /* number of depths present             */
        1,                      /* number of visual types of screen     */
        0, 0,                   /* current pointer position             */
        0, 0,                   /* current text position                */
        49, 80,                 /* maximum row, col text position       */
        8, 15,                  /* console font width and height        */
        64, 64,                 /* maximal size cursor for screen       */
        1, 1,                   /* min, max of visual types             */
        },

        {                       /* depth descriptor of root window */
        0, 0,                   /* which screen and depth               */
        1024, 1024,             /* frame buffer size in pixels          */
        8,                      /* returns the depth (out)              */
        8,                      /* stride of pixel (out)                */
        32,                     /* scan line pad                        */
        (caddr_t) PMAGDV_FRAME_BUFFER,  /* bitmap starts at beginning */
        0,                      /* only filled in when mapped           */
        (caddr_t) 0,            /*plane mask offset -- don't have */
        0,                      /* only filled in when mapped           */
        },

        {                       /* visual descriptor */
        0,                      /* which screen (in)                    */
        0,                      /* which visual of screen (in)          */
        PseudoColor,            /* class of visual                      */
        8,                      /* number of bits per pixel             */
        0, 0, 0,                /* zero since pseudo; mask of subfields */
        8,                      /* bits per RGB                         */
        256,                    /* color map entries */
        },
        0,                      /* screen */
        {
        pmagdv_init_closure,
        ims_g332_load_cursor,
        ims_g332_recolor_cursor,
        ims_g332_set_cursor_position,
        ims_g332_cursor_on_off,
        (caddr_t)ims_g332_softc,
        },
        0,                      /* cursor */
        {
        pmagdv_init_closure,
        ims_g332_init_color_map,
        ims_g332_load_color_map_entry,
        NULL,
        ims_g332_video_on,
        ims_g332_video_off,
        (caddr_t)ims_g332_softc,
        },
        0,                      /* cmap */
        {
        pmagdv_screen_init_closure,
        fb_init_screen,
        fb_clear_screen,
        fb_scroll_screen,
        fb_blitc,
        fb_map_unmap_screen,
        NULL,                   /* ioctl optional */
        NULL,                   /* close optional */
        (caddr_t)fb_softc,
        },
        NULL,                   /* not yet interrupt driven */
        NULL,                   /* not yet interrupt driven */
        NULL,                   /* beginning of time */
    },
#endif /* !__alpha */
    {
        /* HX (sfb) */
        {
        0,      /* screen (in) */
        MONITOR_VRT19,
        "PMAGB-BA",
        1280, 1024,              /* width, height */
        0,                      /* depth */
        1,                      /* number of depths present             */
        1,                      /* number of visual types of screen     */
        0, 0,                   /* current pointer position             */
        0, 0,                   /* current text position                */
        56, 80,                 /* maximum row, col text position       */
        8, 15,                  /* console font width and height        */
        64, 64,                 /* maximal size cursor for screen       */
        1, 1,                   /* min, max of visual types             */
        },

        {                       /* depth descriptor of root window */
        0, 0,                   /* which screen and depth               */
        1280, 1024,              /* frame buffer size in pixels          */
        8,                      /* returns the depth (out)              */
        8,                      /* stride of pixel (out)                */
        32,                     /* scan line pad                        */
        (caddr_t) PMAGBBA_FB_OFFSET,    /* bitmap starts at 2meg        */
        0,                      /* only filled in when mapped           */
        (caddr_t) 0,        	/* plane mask offset (filled in by map) */
        0,                      /* only filled in when mapped           */
        },

        {                       /* visual descriptor */
        0,                      /* which screen (in)                    */
        0,                      /* which visual of screen (in)          */
        PseudoColor,            /* class of visual                      */
        8,                      /* number of bits per pixel             */
        0, 0, 0,                /* zero since pseudo; mask of subfields */
        8,                      /* bits per RGB                         */
        256,                    /* color map entries */
        },
        0,
        {
        bt_init_closure,
        bt_load_cursor,
        bt_recolor_cursor,
        bt_set_cursor_position,
        bt_cursor_on_off,
        (caddr_t)bt459_softc,
         },
        BT459_HX_TYPE,
        {
        bt_init_closure,
        bt_init_color_map,
        bt_load_color_map_entry,
        NULL,
        bt_video_on,
        bt_video_off,
        (caddr_t)bt459_softc,
        },
        BT459_HX_TYPE,
        {
        fb_init_closure,
        fb_init_screen,
        sfb_clear_screen,
        sfb_scroll_screen,
        sfb_blitc,
        sfb_map_unmap_screen,
        sfb_ioctl,                      /* ioctl optional */
        (void *)sfb_close,                      /* close optional */
        (caddr_t)fb_softc,
        },
        sfb_attach,
        sfb_interrupt,
        sfb_bot,                        /* beginning of time */
    },
    {
        /* SFB+8,+32,+32Z */
        {
        0,      /* screen (in) */
        MONITOR_VRT19,
        "PMAGD   ",
        1280, 1024,              /* width, height */
        0,                      /* depth */
        1,                      /* number of depths present             */
        1,                      /* number of visual types of screen     */
        0, 0,                   /* current pointer position             */
        0, 0,                   /* current text position                */
        56, 80,                 /* maximum row, col text position       */
        8, 15,                  /* console font width and height        */
        64, 64,                 /* maximal size cursor for screen       */
        1, 1,                   /* min, max of visual types             */
        },

        {                       /* depth descriptor of root window */
        0, 0,                   /* which screen and depth               */
        1280, 1024,              /* frame buffer size in pixels          */
        0,                      /* returns the depth (out)              */
        0,                      /* stride of pixel (out)                */
        0,                     /* scan line pad                        */
        (caddr_t) 0,    /* bitmap starts at 2meg        */
        0,                      /* only filled in when mapped           */
        (caddr_t) 0,        	/* plane mask offset (filled in by map) */
        0,                      /* only filled in when mapped           */
        },

        {                       /* visual descriptor */
        0,                      /* which screen (in)                    */
        0,                      /* which visual of screen (in)          */
        StaticGray,            /* class of visual                      */
        0,                      /* number of bits per pixel             */
        0, 0, 0,                /* zero since pseudo; mask of subfields */
        0,                      /* bits per RGB                         */
        0,                    /* color map entries */
        },
        0,
        {
        sfbp_dummy_caddrt,
        sfbp_dummy_int,
        sfbp_dummy_int,
        sfbp_dummy_int,
        sfbp_dummy_int,
        (caddr_t) NULL,
         },
        0,
        {
        sfbp_dummy_caddrt,
        sfbp_dummy_int,
        sfbp_dummy_int,
        NULL,
        sfbp_dummy_int,
        sfbp_dummy_int,
        (caddr_t) NULL,
        },
        0,
        {
        sfbp_init_closure,
        sfbp_init_screen,
        sfbp_clear_screen,
        sfbp_scroll_screen,
        sfbp_blitc,
        sfbp_map_unmap_screen,
        sfbp_ioctl,                      /* ioctl optional */
        (void *) sfbp_close,                      /* close optional */
        (caddr_t) fb_softc,
        },
        sfbp_attach,
        sfbp_interrupt,
        sfbp_bot,                        /* beginning of time */
    },
    {
        /* SFB+8 */
        {
        0,      /* screen (in) */
        MONITOR_VRT19,
        "PMAGD-AA",
        1280, 1024,              /* width, height */
        0,                      /* depth */
        1,                      /* number of depths present             */
        1,                      /* number of visual types of screen     */
        0, 0,                   /* current pointer position             */
        0, 0,                   /* current text position                */
        56, 80,                 /* maximum row, col text position       */
        8, 15,                  /* console font width and height        */
        64, 64,                 /* maximal size cursor for screen       */
        1, 1,                   /* min, max of visual types             */
        },

        {                       /* depth descriptor of root window */
        0, 0,                   /* which screen and depth               */
        1280, 1024,             /* frame buffer size in pixels          */
        0,                      /* returns the depth (out)              */
        0,                      /* stride of pixel (out)                */
        0,                      /* scan line pad                        */
        (caddr_t) 0,    /* bitmap starts at 2meg        */
        0,                      /* only filled in when mapped           */
        (caddr_t) 0,        	/* plane mask offset (filled in by map) */
        0,                      /* only filled in when mapped           */
        },

        {                       /* visual descriptor */
        0,                      /* which screen (in)                    */
        0,                      /* which visual of screen (in)          */
        StaticGray,             /* class of visual                      */
        0,                      /* number of bits per pixel             */
        0, 0, 0,                /* zero since pseudo; mask of subfields */
        0,                      /* bits per RGB                         */
        0,                    /* color map entries */
        },
        0,
        {
        sfbp_dummy_caddrt,
        sfbp_dummy_int,
        sfbp_dummy_int,
        sfbp_dummy_int,
        sfbp_dummy_int,
        (caddr_t) NULL,
         },
        0,
        {
        sfbp_dummy_caddrt,
        sfbp_dummy_int,
        sfbp_dummy_int,
        NULL,
        sfbp_dummy_int,
        sfbp_dummy_int,
        (caddr_t) NULL,
        },
        0,
        {
        sfbp_init_closure,
        sfbp_init_screen,
        sfbp_clear_screen,
        sfbp_scroll_screen,
        sfbp_blitc,
        sfbp_map_unmap_screen,
        sfbp_ioctl,                      /* ioctl optional */
        (void *) sfbp_close,                      /* close optional */
        (caddr_t) fb_softc,
        },
        sfbp_attach,
        sfbp_interrupt,
        sfbp_bot,                        /* beginning of time */
    },
    {
        /* SFB+32 */
        {
        0,      /* screen (in) */
        MONITOR_VRT19,
        "PMAGD-BA",
        1280, 1024,              /* width, height */
        0,                      /* depth */
        1,                      /* number of depths present             */
        1,                      /* number of visual types of screen     */
        0, 0,                   /* current pointer position             */
        0, 0,                   /* current text position                */
        56, 80,                 /* maximum row, col text position       */
        8, 15,                  /* console font width and height        */
        64, 64,                 /* maximal size cursor for screen       */
        1, 1,                   /* min, max of visual types             */
        },

        {                       /* depth descriptor of root window */
        0, 0,                   /* which screen and depth               */
        1280, 1024,             /* frame buffer size in pixels          */
        0,                      /* returns the depth (out)              */
        0,                      /* stride of pixel (out)                */
        0,                      /* scan line pad                        */
        (caddr_t) 0,    /* bitmap starts at 2meg        */
        0,                      /* only filled in when mapped           */
        (caddr_t) 0,        	/* plane mask offset (filled in by map) */
        0,                      /* only filled in when mapped           */
        },

        {                       /* visual descriptor */
        0,                      /* which screen (in)                    */
        0,                      /* which visual of screen (in)          */
        StaticGray,             /* class of visual                      */
        0,                      /* number of bits per pixel             */
        0, 0, 0,                /* zero since pseudo; mask of subfields */
        0,                      /* bits per RGB                         */
        0,                    /* color map entries */
        },
        0,
        {
        sfbp_dummy_caddrt,
        sfbp_dummy_int,
        sfbp_dummy_int,
        sfbp_dummy_int,
        sfbp_dummy_int,
        (caddr_t) NULL,
         },
        0,
        {
        sfbp_dummy_caddrt,
        sfbp_dummy_int,
        sfbp_dummy_int,
        NULL,
        sfbp_dummy_int,
        sfbp_dummy_int,
        (caddr_t) NULL,
        },
        0,
        {
        sfbp_init_closure,
        sfbp_init_screen,
        sfbp_clear_screen,
        sfbp_scroll_screen,
        sfbp_blitc,
        sfbp_map_unmap_screen,
        sfbp_ioctl,                      /* ioctl optional */
        (void *) sfbp_close,                      /* close optional */
        (caddr_t) fb_softc,
        },
        sfbp_attach,
        sfbp_interrupt,
        sfbp_bot,                        /* beginning of time */
    },
/* #endif */
        0	/* This is to keep the compiler happy. */
};


/*
 * The 'type' index is computed by subtracting the deep.deep value
 * from the deep.mask value.
 */
struct sfbplus_info sfbplus_type[] = {
    {
	(vm_offset_t) 0,
	(SFBPlus) PMAGDA_SFBP_ASIC_OFFSET,
	(vm_offset_t) PMAGDA_0_0_FB_OFFSET,
	PMAGDA_0_0_FB_SIZE,
	0,
	8
    },
    {
	(vm_offset_t) 0,
	(SFBPlus) PMAGDA_SFBP_ASIC_OFFSET,
	(vm_offset_t) PMAGDA_0_1_FB_OFFSET,
	PMAGDA_0_1_FB_SIZE,
	0,
	8
    },
    {
	(vm_offset_t) 0,
	(SFBPlus) PMAGDA_SFBP_ASIC_OFFSET,
	(vm_offset_t) PMAGDA_1_3_FB_OFFSET,
	PMAGDA_1_3_FB_SIZE,
	1,
	32
    },
    {
	(vm_offset_t) 0,
	(SFBPlus) PMAGDA_SFBP_ASIC_OFFSET,
	(vm_offset_t) PMAGDA_0_3_FB_OFFSET,
	PMAGDA_0_3_FB_SIZE,
	0,
	8
    },
    {
	(vm_offset_t) 0,
	(SFBPlus) 0,
	(vm_offset_t) PMAGDA_INVALID_FB_OFFSET,
	PMAGDA_INVALID_FB_SIZE,
	-1,
	-1
    },
    {
	(vm_offset_t) 0,
	(SFBPlus) 0,
	(vm_offset_t) PMAGDA_INVALID_FB_OFFSET,
	PMAGDA_INVALID_FB_SIZE,
	-1,
	-1
    },
    {
	(vm_offset_t) 0,
	(SFBPlus) PMAGDA_SFBP_ASIC_OFFSET,
	(vm_offset_t) PMAGDA_1_7_FB_OFFSET,
	PMAGDA_1_7_FB_SIZE,
	1,
	32
    },
};

int nfb_types = sizeof(fb_type) / sizeof (struct fb_type);
int nsfbp_types = sizeof(sfbplus_type) / sizeof(sfbplus_type[0]);

struct	controller *fbinfo[NFB];
/*#if NTC > 0 */
struct  sfbinfo    sfb_softc[NFB];
struct sfbplus_info    sfbplus_softc[NFB];
/*#endif */

#else
struct fb_info 	fb_softc[1];
struct	controller *fbinfo[1];

int nfb_types = 0;
int nsfbp_types = 0;

#endif

#endif BINARY

