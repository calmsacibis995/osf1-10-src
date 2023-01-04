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

/************************************************************************
 * Modification History
 *
 * 13-Nov-91 -- Lloyd Wheeler
 *		Tweaked includes to work in OSF kernel build environment
 *
 * 29-Aug-90 -- Sam Hsu
 *		Cleanup and merge into 4.titan/law.
 *
 * 27-May-90 -- Win Treese
 *		Created for 2DA from fb_data.c
 *
 ************************************************************************/
#define _PX_DATA_C_

#ifndef _WS_DATA_C_
#include <sys/devio.h>
#include <sys/param.h>
#include <sys/workstation.h>
#include <sys/inputdevice.h>
#include <sys/wsdevice.h>
#include <sys/buf.h>
#include <io/dec/uba/ubavar.h>		/* auto-config headers 		*/
#include <io/dec/ws/bt459.h>		/* specific to BT459 VDAC 	*/
#endif /* _WS_DATA_C_ */

#include <sys/dk.h>			/* CP_IDLE */
#include <sys/vmmac.h>			/* svtophys() */
#include <io/dec/ws/stamp.h>		/* PixelStamp definitions. 	*/
#include <io/dec/ws/px.h>		/* PixelStamp information. 	*/
#include <sys/pxinfo.h>			/* PX closure */
#include <io/common/devdriver.h>

#include "fb.h"
#include "px.h"

/*
 * XXX visual closures must have pointer to device as first element, this
 * is a hack, but I can't think of a better one right now.
 */
typedef struct _px_type {
    ws_screen_descriptor screen;
    ws_depth_descriptor depth[NDEPTHS];
    ws_visual_descriptor visual[NVISUALS];
    u_char cursor_type;
    u_char cmap_type;
    u_char screen_type;			/* ??? */
    u_char max_fbn;			/* 0 to max_fbn frame buffer #'s */
    ws_cursor_functions cf;
    ws_color_map_functions cmf;
    ws_screen_functions sf;
    int 	(*attach)();
    int		(*bootmsg)();
    int 	(*map)();		/* Map screen procedure. */
    void 	(*interrupt)();
    int        *(*getPacket)();		/* Allocate a PixelStamp request */
    void 	(*sendPacket)();	/* Send a PixelStamp packet. */
    void	(*getImageBuffer)();	/* Get pointers to image buffer. */
    int 	(*setup)();		/* Setup procedure. */
    int		(*vmHook)();		/* VM hook. */
    caddr_t	stic;			/* STIC registers (offset). */
    caddr_t 	stamp;			/* Stamp registers (offset). */
    u_short	stamp_width;
    u_short	stamp_height;
    u_int 	text_foreground;	/* Text foreground color. */
    u_int	text_background;	/* Text background color. */
    u_int	ws_display_type;	/* hold value for global ws_display_type */
} px_type;


int	pxprobe(), pxattach();
void	pxintr();

caddr_t px_init_closure();
int	px_init_screen();
int	px_clear_screen();
int	px_scroll_screen();
int	px_blitc();
int	px_probe_fb();
int	px_cpu_idle();

#ifdef BINARY

#ifdef ultrix
extern struct uba_device *pxinfo[];
#else /* ultrix */
extern struct controller *pxinfo[];
#endif /* ultrix */

extern px_info	px_softc[];
extern px_type	px_types[];
extern int 	npx_types;
extern int	npx_softc;
extern int	_px_level;
extern int	_px_debug;

/*
 * Keyboard translation and font tables.
 */
extern  u_short fg_font[];

#else /* BINARY */
#ifndef lint
static char *sccsid = "@(#)px_data.c	5.1      (ULTRIX)  6/19/91";
#endif /* BINARY */

#if NPX > 0

u_short	pxstd[] = { 0 };

#ifdef ultrix
struct uba_device *pxinfo[NPX];
#else /* ultrix */
struct controller *pxinfo[NPX];
#endif /* ultrix */

#ifdef ultrix
struct	uba_driver pxdriver = {
    pxprobe, 0, pxattach, 0, pxstd, "px", pxinfo,
};
#else /* ultrix */
struct driver pxdriver = 
    { pxprobe, 0, pxattach, 0, 0, (caddr_t *) pxstd, 0, 0, "px", pxinfo };
#endif /* ultrix */

#if 0
int	gcprobe(), gcattach(), gcintr();
struct	uba_driver pxdriver = 
        { gcprobe, 0, gcattach, 0, pxstd, "px", pxinfo };
#endif

px_info px_softc[NPX];
int    npx_softc = NPX;

px_type px_types[] = {
    {					/**** PX - 2da ****/
	{				/* screen descriptor */
	    0,				/* screen number (in) */
	    MONITOR_VRT19,
	    "PMAG-CA ",			/* eqstr in 3MAX rom option ID */
	    1280, 1024,			/* width, height */
	    0,				/* depth */
	    NDEPTHS,			/* number of depths present */
	    NVISUALS,			/* number of visual types of screen */
	    0, 0,			/* current pointer position */
	    0, 0,			/* current text position */
	    50, 115,			/* maximum row, col text position */
	    11, 20,			/* console font width and height */
	    64, 64,			/* maximal size cursor for screen */
	    1, 1,			/* min, max of visual types */
	},
	{				/* depth descriptor of root window */
	    0, 0,			/* which screen and depth (in) */
	    1280, 1024,			/* frame buffer size in pixels */
	    8,				/* returns the depth (out) */
	    8,				/* stride of pixel (out) */
	    32,				/* scan line pad */
	    0,				/* */
	    0,				/* only filled in when mapped */
	    0,				/* plane mask */
	    0,				/* only filled in when mapped */
	},
	{				/* visual descriptor */
	    0,				/* which screen (in) */
	    0,				/* which visual of screen (in) */
	    PseudoColor,		/* class of visual */
	    8,				/* number of bits per pixel */
	    0, 0, 0,			/* zero since pseudo; mask of subfields */
	    8,				/* bits per RGB */
	    256,			/* color map entries */
	},
	BT459_PX_TYPE,			/* cursor, cmap */
	BT459_PX_TYPE,
	BT459_PX_TYPE,			/* XXX */
	1,
	{				/* Cursor functions. */
	    bt_init_closure,
	    bt_load_cursor,
	    bt_recolor_cursor,
	    bt_set_cursor_position,
	    bt_cursor_on_off,
	    (caddr_t) bt459_softc,
	},
	{				/* Color map functions. */
	    bt_init_closure,
	    bt_init_color_map,
	    bt_load_color_map_entry,
	    NULL,
	    bt_video_on,
	    bt_video_off,
	    (caddr_t) bt459_softc,
	},
	{				/* Screen functions */
	    px_init_closure,
	    px_init_screen,
	    px_clear_screen,
	    px_scroll_screen,
	    px_blitc,
	    pa_map_screen,
	    pa_ioctl,			/* ioctl optional */
	    NULL,			/* close optional */
	    (caddr_t) px_softc,
	},
	pa_attach,
	pa_bootmsg,
	pa_map_screen,
	pa_interrupt,
	pa_getPacket,			/* Allocate a PixelStamp packet. */
	pa_sendPacket,			/* Send a PixelStamp packet. */
	NULL,
	pa_setup,
	NULL,
	(caddr_t)PA_STIC_OFFSET,	/* stic */
	(caddr_t)PA_STAMP_OFFSET,	/* stamp */
	5, 1,				/* stamp update array */
	0x010101, 0x0,			/* text fg, bg */
	PX_DTYPE,			/* ws_display_type */
    },
    {					/**** PXG - 3da ****/
	{				/* screen descriptor */
	    0,				/* screen number (in) */
	    MONITOR_VRT19,
	    "PMAG-DA ",			/* eqstr in 3MAX rom option ID */
	    1280, 1024,			/* width, height */
	    0,				/* depth */
	    NDEPTHS,			/* number of depths present */
	    NVISUALS,			/* number of visual types of screen */
	    0, 0,			/* current pointer position */
	    0, 0,			/* current text position */
	    50, 115,			/* maximum row, col text position */
	    11, 20,			/* console font width and height */
	    64, 64,			/* maximal size cursor for screen */
	    1, 1,			/* min, max of visual types */
	},
	{				/* depth descriptor of root window */
	    0, 0,			/* which screen and depth (in) */
	    1280, 1024,			/* frame buffer size in pixels */
	    0,				/* XXX depth  = 8 or 24 (out) */
	    0,				/* XXX stride = 8 or 32 (out) */
	    32,				/* scan line pad */
	    0,				/* */
	    0,				/* only filled in when mapped */
	    0,				/* plane mask */
	    0,				/* only filled in when mapped */
	},
	{				/* visual descriptor */
	    0,				/* which screen (in) */
	    0,				/* which visual of screen (in) */
	    PseudoColor,		/* class of visual */
					/* TrueColor for 24-plane */
	    0,				/* bits per pixel = 8 or 24 */
	    0, 0, 0,			/* zero since pseudo; mask of subfields */
	    8,				/* bits per RGB */
	    256,			/* color map entries */
	},
	BT459_PXG_TYPE,
	BT459_PXG_TYPE,
	BT459_PXG_TYPE,			/* XXX */
	2,
	{				/* Cursor functions. */
	    bt_init_closure,
	    bt_load_cursor,
	    bt_recolor_cursor,
	    bt_set_cursor_position,
	    bt_cursor_on_off,
	    (caddr_t) bt459_softc,
	},
	{				/* Color map functions. */
	    bt_init_closure,
	    bt_init_color_map,
	    bt_load_color_map_entry,
	    NULL,
	    bt_video_on,
	    bt_video_off,
	    (caddr_t) bt459_softc,
	},
	{				/* Screen functions */
	    px_init_closure,
	    px_init_screen,
	    px_clear_screen,
	    px_scroll_screen,
	    px_blitc,
	    pq_map_screen,
	    pq_ioctl,			/* ioctl optional */
	    pq_close,			/* close optional */
	    (caddr_t) px_softc,
	},
	pq_attach,
	pq_bootmsg,
	pq_map_screen,
	pq_interrupt,
	pq_getPacket,			/* Grab a PixelStamp packet. */
	pq_sendPacket,			/* Send a PixelStamp packet. */
	pq_getImageBuffer,		/* Pointers to SRAM image area. */
	pq_setup,
	pq_invalidate_gcp_tlb,
	(caddr_t)PQ_STIC_OFFSET,	/* stic */
	(caddr_t)PQ_STAMP_OFFSET,	/* stamp */
	5, 1,				/* stamp update array */
	0x010101, 0x0,			/* text fg, bg */
	PXG_DTYPE, 			/* ws_display_type */
    },
   {					/**** PXG-T - 3da ****/
	{				/* screen descriptor */
	    0,				/* screen number (in) */
	    MONITOR_VRT19,
	    "PMAG-FA ",			/* eqstr in 3MAX rom option ID */
	    1280, 1024,			/* width, height */
	    0,				/* depth */
	    NDEPTHS,			/* number of depths present */
	    NVISUALS,			/* number of visual types of screen */
	    0, 0,			/* current pointer position */
	    0, 0,			/* current text position */
	    50, 115,			/* maximum row, col text position */
	    11, 20,			/* console font width and height */
	    64, 64,			/* maximal size cursor for screen */
	    1, 1,			/* min, max of visual types */
	},
	{				/* depth descriptor of root window */
	    0, 0,			/* which screen and depth (in) */
	    1280, 1024,			/* frame buffer size in pixels */
	    24,				/* depth (out) */
	    32,				/* stride of pixel (out) */
	    32,				/* scan line pad */
	    0,				/* */
	    0,				/* only filled in when mapped */
	    0,				/* plane mask */
	    0,				/* only filled in when mapped */
	},
	{				/* visual descriptor */
	    0,				/* which screen (in) */
	    0,				/* which visual of screen (in) */
	    TrueColor,			/* class of visual */
	    24,				/* number of bits per pixel */
	    0xff0000,
	    0x00ff00,
	    0x0000ff,			/* mask of subfields */
	    8,				/* bits per RGB */
	    256,			/* color map entries */
	},
	BT459_PXG_TYPE,
	BT459_PXG_TYPE,
	BT459_PXG_TYPE,			/* XXX */
	3,
	{				/* Cursor functions. */
	    bt_init_closure,
	    bt_load_cursor,
	    bt_recolor_cursor,
	    bt_set_cursor_position,
	    bt_cursor_on_off,
	    (caddr_t) bt459_softc,
	},
	{				/* Color map functions. */
	    bt_init_closure,
	    bt_init_color_map,
	    bt_load_color_map_entry,
	    NULL,
	    bt_video_on,
	    bt_video_off,
	    (caddr_t) bt459_softc,
	},
	{				/* Screen functions */
	    px_init_closure,
	    px_init_screen,
	    px_clear_screen,
	    px_scroll_screen,
	    px_blitc,
	    pq_map_screen,
	    pq_ioctl,			/* ioctl optional */
	    pq_close,			/* close optional */
	    (caddr_t) px_softc,
	},
	pq_attach,
	pq_bootmsg,
	pq_map_screen,
	pq_interrupt,
	pq_getPacket,			/* Grab a PixelStamp packet. */
	pq_sendPacket,			/* Send a PixelStamp packet. */
	pq_getImageBuffer,		/* Pointers to SRAM image area. */
	pq_setup,
	pq_invalidate_gcp_tlb,
	(caddr_t)PQ_STIC_OFFSET,	/* stic */
	(caddr_t)PQ_STAMP_OFFSET,	/* stamp */
	5, 2,				/* stamp update array */
	0x010101, 0x0,			/* text fg, bg */
	PXG_DTYPE, 			/* ws_display_type */
    },
    {					/**** PXG-T+ 3da *****/
	{				/* screen descriptor */
	    0,				/* screen number (in) */
	    MONITOR_VRT19,
	    "PMAG-FB ",			/* eqstr in 3MAX rom option ID */
	    1280, 1024,			/* width, height */
	    0,				/* depth */
	    NDEPTHS,			/* number of depths present */
	    NVISUALS,			/* number of visual types of screen */
	    0, 0,			/* current pointer position */
	    0, 0,			/* current text position */
	    50, 100,			/* maximum row, col text position */
	    11, 20,			/* console font width and height */
	    64, 64,			/* maximal size cursor for screen */
	    1, 1,			/* min, max of visual types */
	},
	{				/* depth descriptor of root window */
	    0, 0,			/* which screen and depth (in) */
	    1280, 1024,			/* frame buffer size in pixels */
	    24,				/* depth (out) */
	    32,				/* stride of pixel (out) */
	    32,				/* scan line pad */
	    0,				/* */
	    0,				/* only filled in when mapped */
	    0,				/* plane mask */
	    0,				/* only filled in when mapped */
	},
	{				/* visual descriptor */
	    0,				/* which screen (in) */
	    0,				/* which visual of screen (in) */
	    TrueColor,			/* class of visual */
	    24,				/* number of bits per pixel */
	    0xff0000,
	    0x00ff00,
	    0x0000ff,			/* mask of subfields */
	    8,				/* bits per RGB */
	    256,			/* color map entries */
	},
	BT459_PXG_TYPE,
	BT459_PXG_TYPE,
	BT459_PXG_TYPE,			/* XXX */
	3,
	{				/* Cursor functions. */
	    bt_init_closure,
	    bt_load_cursor,
	    bt_recolor_cursor,
	    bt_set_cursor_position,
	    bt_cursor_on_off,
	    (caddr_t) bt459_softc,
	},
	{				/* Color map functions. */
	    bt_init_closure,
	    bt_init_color_map,
	    bt_load_color_map_entry,
	    NULL,
	    bt_video_on,
	    bt_video_off,
	    (caddr_t) bt459_softc,
	},
	{				/* Screen functions */
	    px_init_closure,
	    px_init_screen,
	    px_clear_screen,
	    px_scroll_screen,
	    px_blitc,
	    pq_map_screen,
	    pq_ioctl,			/* ioctl optional */
	    pq_close,			/* close optional */
	    (caddr_t) px_softc,
	},
	pq_attach,
	pq_bootmsg,
	pq_map_screen,
	pq_interrupt,
	pq_getPacket,			/* Grab a PixelStamp packet. */
	pq_sendPacket,			/* Send a PixelStamp packet. */
	pq_getImageBuffer,		/* Pointers to SRAM image area. */
	pq_setup,
	pq_invalidate_gcp_tlb,
	(caddr_t)PQ_STIC_OFFSET,	/* stic */
	(caddr_t)PQ_STAMP_OFFSET,	/* stamp */
	5, 2,				/* stamp update array */
	0x010101, 0x0,			/* text fg, bg */
	PXG_DTYPE, 			/* ws_display_type */
    },

   {					/**** PXG-T+ - 3da ****/
	{				/* screen descriptor */
	    0,				/* screen number (in) */
	    MONITOR_VRT19,
	    "PMAGB-FA",		/* eqstr in 3MAX rom option ID */
	    1280, 1024,			/* width, height */
	    0,				/* depth */
	    NDEPTHS,			/* number of depths present */
	    NVISUALS,			/* number of visual types of screen */
	    0, 0,			/* current pointer position */
	    0, 0,			/* current text position */
	    50, 100,			/* maximum row, col text position */
	    11, 20,			/* console font width and height */
	    64, 64,			/* maximal size cursor for screen */
	    1, 1,			/* min, max of visual types */
	},
	{				/* depth descriptor of root window */
	    0, 0,			/* which screen and depth (in) */
	    1280, 1024,			/* frame buffer size in pixels */
	    24,				/* depth (out) */
	    32,				/* stride of pixel (out) */
	    32,				/* scan line pad */
	    0,				/* */
	    0,				/* only filled in when mapped */
	    0,				/* plane mask */
	    0,				/* only filled in when mapped */
	},
	{				/* visual descriptor */
	    0,				/* which screen (in) */
	    0,				/* which visual of screen (in) */
	    TrueColor,			/* class of visual */
	    24,				/* number of bits per pixel */
	    0xff0000,
	    0x00ff00,
	    0x0000ff,			/* mask of subfields */
	    8,				/* bits per RGB */
	    256,			/* color map entries */
	},
	BT459_PXG_TYPE,
	BT459_PXG_TYPE,
	BT459_PXG_TYPE,			/* XXX */
	3,
	{				/* Cursor functions. */
	    bt_init_closure,
	    bt_load_cursor,
	    bt_recolor_cursor,
	    bt_set_cursor_position,
	    bt_cursor_on_off,
	    (caddr_t) bt459_softc,
	},
	{				/* Color map functions. */
	    bt_init_closure,
	    bt_init_color_map,
	    bt_load_color_map_entry,
	    NULL,
	    bt_video_on,
	    bt_video_off,
	    (caddr_t) bt459_softc,
	},
	{				/* Screen functions */
	    px_init_closure,
	    px_init_screen,
	    px_clear_screen,
	    px_scroll_screen,
	    px_blitc,
	    pq_map_screen,
	    pq_ioctl,			/* ioctl optional */
	    pq_close,			/* close optional */
	    (caddr_t) px_softc,
	},
	pq_attach,
	pq_bootmsg,
	pq_map_screen,
	pq_interrupt,
	pq_getPacket,			/* Grab a PixelStamp packet. */
	pq_sendPacket,			/* Send a PixelStamp packet. */
	pq_getImageBuffer,		/* Pointers to SRAM image area. */
	pq_setup,
	pq_invalidate_gcp_tlb,
	(caddr_t)PQ_STIC_OFFSET,	/* stic */
	(caddr_t)PQ_STAMP_OFFSET,	/* stamp */
	5, 2,				/* stamp update array */
	0x010101, 0x0,			/* text fg, bg */
	PXG_DTYPE, 			/* ws_display_type */
    },
	
   {					/**** PXG-T+ - 3da ****/
	{				/* screen descriptor */
	    0,				/* screen number (in) */
	    MONITOR_VRT19,
	    "PMAGB-FB",			/* eqstr in 3MAX rom option ID */
	    1280, 1024,			/* width, height */
	    0,				/* depth */
	    NDEPTHS,			/* number of depths present */
	    NVISUALS,			/* number of visual types of screen */
	    0, 0,			/* current pointer position */
	    0, 0,			/* current text position */
	    50, 100,			/* maximum row, col text position */
	    11, 20,			/* console font width and height */
	    64, 64,			/* maximal size cursor for screen */
	    1, 1,			/* min, max of visual types */
	},
	{				/* depth descriptor of root window */
	    0, 0,			/* which screen and depth (in) */
	    1280, 1024,			/* frame buffer size in pixels */
	    24,				/* depth (out) */
	    32,				/* stride of pixel (out) */
	    32,				/* scan line pad */
	    0,				/* */
	    0,				/* only filled in when mapped */
	    0,				/* plane mask */
	    0,				/* only filled in when mapped */
	},
	{				/* visual descriptor */
	    0,				/* which screen (in) */
	    0,				/* which visual of screen (in) */
	    TrueColor,			/* class of visual */
	    24,				/* number of bits per pixel */
	    0xff0000,
	    0x00ff00,
	    0x0000ff,			/* mask of subfields */
	    8,				/* bits per RGB */
	    256,			/* color map entries */
	},
	BT459_PXG_TYPE,
	BT459_PXG_TYPE,
	BT459_PXG_TYPE,			/* XXX */
	3,
	{				/* Cursor functions. */
	    bt_init_closure,
	    bt_load_cursor,
	    bt_recolor_cursor,
	    bt_set_cursor_position,
	    bt_cursor_on_off,
	    (caddr_t) bt459_softc,
	},
	{				/* Color map functions. */
	    bt_init_closure,
	    bt_init_color_map,
	    bt_load_color_map_entry,
	    NULL,
	    bt_video_on,
	    bt_video_off,
	    (caddr_t) bt459_softc,
	},
	{				/* Screen functions */
	    px_init_closure,
	    px_init_screen,
	    px_clear_screen,
	    px_scroll_screen,
	    px_blitc,
	    pq_map_screen,
	    pq_ioctl,			/* ioctl optional */
	    pq_close,			/* close optional */
	    (caddr_t) px_softc,
	},
	pq_attach,
	pq_bootmsg,
	pq_map_screen,
	pq_interrupt,
	pq_getPacket,			/* Grab a PixelStamp packet. */
	pq_sendPacket,			/* Send a PixelStamp packet. */
	pq_getImageBuffer,		/* Pointers to SRAM image area. */
	pq_setup,
	pq_invalidate_gcp_tlb,
	(caddr_t)PQ_STIC_OFFSET,	/* stic */
	(caddr_t)PQ_STAMP_OFFSET,	/* stamp */
	5, 2,				/* stamp update array */
	0x010101, 0x0,			/* text fg, bg */
	PXG_DTYPE, 			/* ws_display_type */
    },
};

int npx_types = sizeof(px_types) / sizeof(px_type);
int _px_level;
int _px_debug = PX_DEBUGGING;

#else /* NPX */

/* struct px_info px_softc[1]; */
/* struct uba_device *pxinfo[1]; */

int npx_softc = 0;
int npx_types = 0;

#endif /* NPX */

#endif /* BINARY */
