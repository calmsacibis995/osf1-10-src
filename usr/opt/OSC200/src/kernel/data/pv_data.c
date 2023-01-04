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
#define _PV_DATA_C_

#ifndef _WS_DATA_C_
#include <sys/devio.h>
#include <sys/param.h>
#include <sys/workstation.h>
#include <sys/inputdevice.h>
#include <sys/wsdevice.h>
#include <sys/buf.h>
#include <io/dec/uba/ubavar.h>		/* auto-config headers 		*/
#include <io/dec/ws/bt459.h>		/* specific to BT459 VDAC 	*/
#include <io/dec/ws/bt431.h>
#include <io/dec/ws/bt463.h>
#endif /* _WS_DATA_C_ */

#include <sys/dk.h>			/* CP_IDLE */
#include <sys/vmmac.h>			/* svtophys() */
#include <io/dec/ws/pv.h>		/* PixelVision information. 	*/
#include <sys/pvinfo.h>			/* PV closure */
#include <io/common/devdriver.h>

#include "fb.h"
#include "pv.h"

/*
 * XXX visual closures must have pointer to device as first element, this
 * is a hack, but I can't think of a better one right now.
 */
typedef struct _pv_type {
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
    int 	(*setup)();		/* Setup procedure. */
    int		(*vm_hook)();		/* VM hook. */
    caddr_t	p_gcp;
    caddr_t	p_gcp_s;
    caddr_t	p_pva;
    caddr_t	p_pva_s;
    caddr_t	p_pv;
    caddr_t	p_pv_s;
    caddr_t	p_sram;
    caddr_t	p_sram_s;
    u_int 	text_foreground;	/* Text foreground color. */
    u_int	text_background;	/* Text background color. */
    u_int	ws_display_type;	/* hold value for global ws_display_type */
    u_int	sram_size;
} pv_type;


int	pvprobe(), pvattach();
void	pvintr();
	
caddr_t pv_init_closure();
int	pv_recolor_cursor();
int	pv_video_on();
int	pv_video_off();
int	pv_init_screen();
int	pv_clear_screen();
int	pv_scroll_screen();
int	pv_blitc();
int	pv_cpu_idle();
int	pv_which_option();
int	pv_attach();
int	pv_dd_attach();
int	pv_bootmsg();
void	pv_close();
int	pv_ioctl();
int	pv_map_screen();
void	pv_enable_interrupt();
void	pv_interrupt();
int	pv_setup();
int	pv_invalidate_gcp_tlb();
caddr_t	pv_bt431_init_closure();
caddr_t	pv_bt463_init_closure();
int	pv_bt463_init_color_map();
int	pv_bt463_load_color_map_entry();
int	pv_bt431_set_cursor_position();

#ifdef BINARY

extern struct controller *pvinfo[];

extern pv_info	pv_softc[];
extern pv_type	pv_types[];
extern int 	npv_types;
extern int	npv_softc;
extern int	pv_realtime;
extern int	_pv_level;
extern int	_pv_debug;

/*
 * Keyboard translation and font tables.
 */
extern  u_short fg_font[];

#else /* BINARY */

#if NPV > 0

u_short	pvstd[] = { 0 };

struct controller *pvinfo[NPV];

struct driver pvdriver = 
    { pvprobe, 0, pvattach, 0, 0, (caddr_t *) pvstd, 0, 0, "pv", pvinfo };

pv_info pv_softc[NPV];
int    npv_softc = NPV;

pv_type pv_types[] = {
    {					/**** PV - 3da ****/
	{				/* screen descriptor */
	    0,				/* screen number (in) */
	    MONITOR_VRT19,
	    "PMAGC   ",			/* lowend PV option */
	    1280, 1024,			/* width, height */
	    0,				/* depth */
	    NDEPTHS,			/* number of depths present */
	    NVISUALS,			/* number of visual types of screen */
	    0, 0,			/* current pointer position */
	    0, 0,			/* current text position */
	    63, 127,			/* maximum row, col text position */
	    10, 16,			/* console font width and height */
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
	    32,				/* number of bits per pixel */
	    0xff0000,
	    0x00ff00,
	    0x0000ff,			/* mask of subfields */
	    8,				/* bits per RGB */
	    256,			/* color map entries */
	},
	BT431_PV_TYPE,
	BT463_PV_TYPE,
	BT463_PV_TYPE,			/* XXX */
	3,
	{				/* Cursor functions. */
	    pv_bt431_init_closure,
	    bt431_load_cursor,
	    pv_recolor_cursor,
	    pv_bt431_set_cursor_position,
	    bt431_cursor_on_off,
	    (caddr_t) bt431_softc,
	},
	{				/* Color map functions. */
	    pv_bt463_init_closure,
	    pv_bt463_init_color_map,
	    pv_bt463_load_color_map_entry,
	    NULL,
	    pv_video_on,
	    pv_video_off,
	    (caddr_t) bt463_softc,
	},
	{				/* Screen functions */
	    pv_init_closure,
	    pv_init_screen,
	    pv_clear_screen,
	    pv_scroll_screen,
	    pv_blitc,
	    pv_map_screen,
	    pv_ioctl,			/* ioctl optional */
	    pv_close,			/* close optional */
	    (caddr_t) pv_softc,
	},
	pv_dd_attach,
	pv_bootmsg,
	pv_map_screen,
	pv_interrupt,
	pv_setup,
	pv_invalidate_gcp_tlb,
	(caddr_t) PV_GCP_OFFSET,
	(caddr_t) PV_SPARSE_GCP_OFFSET,
	(caddr_t) PV_PVA_OFFSET,
	(caddr_t) PV_SPARSE_PVA_OFFSET,
	(caddr_t) PV_RENDER_OFFSET,
	(caddr_t) PV_SPARSE_RENDER_OFFSET,
	(caddr_t) PV_SRAM_OFFSET,
	(caddr_t) PV_SPARSE_SRAM_OFFSET,
	0x010101, 0x0,			/* text fg, bg */
	PV_DTYPE, 			/* ws_display_type */
	(1<<17),			/* 128KB SRAM */
    },
    {					/**** PV - 3da ****/
	{				/* screen descriptor */
	    0,				/* screen number (in) */
	    MONITOR_VRT19,
	    "PMAGC-AA",			/* lowend PV option */
	    1280, 1024,			/* width, height */
	    0,				/* depth */
	    NDEPTHS,			/* number of depths present */
	    NVISUALS,			/* number of visual types of screen */
	    0, 0,			/* current pointer position */
	    0, 0,			/* current text position */
	    63, 127,			/* maximum row, col text position */
	    10, 16,			/* console font width and height */
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
	    32,				/* number of bits per pixel */
	    0xff0000,
	    0x00ff00,
	    0x0000ff,			/* mask of subfields */
	    8,				/* bits per RGB */
	    256,			/* color map entries */
	},
	BT431_PV_TYPE,
	BT463_PV_TYPE,
	BT463_PV_TYPE,			/* XXX */
	3,
	{				/* Cursor functions. */
	    pv_bt431_init_closure,
	    bt431_load_cursor,
	    pv_recolor_cursor,
	    pv_bt431_set_cursor_position,
	    bt431_cursor_on_off,
	    (caddr_t) bt431_softc,
	},
	{				/* Color map functions. */
	    pv_bt463_init_closure,
	    pv_bt463_init_color_map,
	    pv_bt463_load_color_map_entry,
	    NULL,
	    pv_video_on,
	    pv_video_off,
	    (caddr_t) bt463_softc,
	},
	{				/* Screen functions */
	    pv_init_closure,
	    pv_init_screen,
	    pv_clear_screen,
	    pv_scroll_screen,
	    pv_blitc,
	    pv_map_screen,
	    pv_ioctl,			/* ioctl optional */
	    pv_close,			/* close optional */
	    (caddr_t) pv_softc,
	},
	pv_dd_attach,
	pv_bootmsg,
	pv_map_screen,
	pv_interrupt,
	pv_setup,
	pv_invalidate_gcp_tlb,
	(caddr_t) PV_GCP_OFFSET,
	(caddr_t) PV_SPARSE_GCP_OFFSET,
	(caddr_t) PV_PVA_OFFSET,
	(caddr_t) PV_SPARSE_PVA_OFFSET,
	(caddr_t) PV_RENDER_OFFSET,
	(caddr_t) PV_SPARSE_RENDER_OFFSET,
	(caddr_t) PV_SRAM_OFFSET,
	(caddr_t) PV_SPARSE_SRAM_OFFSET,
	0x010101, 0x0,			/* text fg, bg */
	PV_DTYPE, 			/* ws_display_type */
	(1<<17),			/* 128KB SRAM */
    },
    {					/**** PV - 3da ****/
	{				/* screen descriptor */
	    0,				/* screen number (in) */
	    MONITOR_VRT19,
	    "PMAGC-BA",			/* midrange PV option */
	    1280, 1024,			/* width, height */
	    0,				/* depth */
	    NDEPTHS,			/* number of depths present */
	    NVISUALS,			/* number of visual types of screen */
	    0, 0,			/* current pointer position */
	    0, 0,			/* current text position */
	    63, 127,			/* maximum row, col text position */
	    10, 16,			/* console font width and height */
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
	    32,				/* number of bits per pixel */
	    0xff0000,
	    0x00ff00,
	    0x0000ff,			/* mask of subfields */
	    8,				/* bits per RGB */
	    256,			/* color map entries */
	},
	BT431_PV_TYPE,
	BT463_PV_TYPE,
	BT463_PV_TYPE,			/* XXX */
	3,
	{				/* Cursor functions. */
	    pv_bt431_init_closure,
	    bt431_load_cursor,
	    pv_recolor_cursor,
	    pv_bt431_set_cursor_position,
	    bt431_cursor_on_off,
	    (caddr_t) bt431_softc,
	},
	{				/* Color map functions. */
	    pv_bt463_init_closure,
	    pv_bt463_init_color_map,
	    pv_bt463_load_color_map_entry,
	    NULL,
	    pv_video_on,
	    pv_video_off,
	    (caddr_t) bt463_softc,
	},
	{				/* Screen functions */
	    pv_init_closure,
	    pv_init_screen,
	    pv_clear_screen,
	    pv_scroll_screen,
	    pv_blitc,
	    pv_map_screen,
	    pv_ioctl,			/* ioctl optional */
	    pv_close,			/* close optional */
	    (caddr_t) pv_softc,
	},
	pv_dd_attach,
	pv_bootmsg,
	pv_map_screen,
	pv_interrupt,
	pv_setup,
	pv_invalidate_gcp_tlb,
	(caddr_t) PV_GCP_OFFSET,
	(caddr_t) PV_SPARSE_GCP_OFFSET,
	(caddr_t) PV_PVA_OFFSET,
	(caddr_t) PV_SPARSE_PVA_OFFSET,
	(caddr_t) PV_RENDER_OFFSET,
	(caddr_t) PV_SPARSE_RENDER_OFFSET,
	(caddr_t) PV_SRAM_OFFSET,
	(caddr_t) PV_SPARSE_SRAM_OFFSET,
	0x010101, 0x0,			/* text fg, bg */
	PV_DTYPE, 			/* ws_display_type */
	(1<<17),			/* 128KB Sram */
    },
};

int npv_types = sizeof(pv_types) / sizeof(pv_type);
int _pv_level;
int _pv_debug = PV_CONSOLE;

#else /* NPV */

/* struct pv_info pv_softc[1]; */
/* struct uba_device *pvinfo[1]; */

int npv_softc = 0;
int npv_types = 0;

#endif /* NPV */

#ifdef REALTIME
int pv_realtime = 1;
#else /* REALTIME */
int pv_realtime = 0;
#endif /* REALTIME */

#endif /* BINARY */
