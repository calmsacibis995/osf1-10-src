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
 * @(#)pxinfo.h	5.1	(ULTRIX)	6/19/91
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
#ifndef _PXINFO_H_
#define _PXINFO_H_

#define NDEPTHS 1			/* all current hardware just has one */
#define NVISUALS 1

/*
 * This is the number of bytes to alloc.  128Kb + (1 page) extra
 * are allocated, within which will be located a 32Kb-aligned
 * STIC packet DMA area (2da constraints).
 */
#define _4K	(1<<12)
#define _8K 	(1<<13)
#define _32K 	(1<<15)
#define _64K 	(1<<16)
#define _96K 	(_32K +_64K)
#define _128K 	(1<<17)

#define PX_RB_SIZE	(_128K)	/* Ringbuffer allocation size (bytes). */

typedef struct _px_info {
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
    int	       *(*getPacket)();		/* Allocate a PixelStamp request. */
    void 	(*sendPacket)();	/* Send a PixelStamp packet. */
    void	(*getImageBuffer)();	/* Get pointers to image buffer. */
    int		(*setup)();		/* Setup procedure. */
    int		(*vmHook)();		/* VM hook. */
    caddr_t	stic;			/* STIC registers. */
    caddr_t 	stamp;			/* Stamp registers. */
    u_char	max_fbn;
    u_char	stamp_width;
    u_char	stamp_height;
    u_int 	text_foreground;	/* Text foreground color. */
    u_int	text_background;	/* Text background color. */
    /*** stuff above this line gets initialized from px_types[] ***/
    caddr_t 	pxo;			/* Board phys addr. */
#ifdef __alpha
    caddr_t     pxod;                   /* Dense space addr. */
#endif /* __alpha */
    int	       *ringbuffer;		/* Packet area pointer (aligned). */
    caddr_t 	dev_closure;		/* Device-specific closure. */
    /****************************************************************
     * rest of this stuff mapped to user eventually...
     ****************************************************************/
/*
 * You'll find definitions of px__* 'components' as macro definitions in
 * io/dec/ws/px.h, handled in the manner of the OSF/1 u.area components.
 */
#ifdef ultrix
    pxInfo	px;
    u_char	memory[PX_RB_SIZE];
#else
    pxInfo	*px;
    /* pxInfo & packet areas allocated from mips_init.c */
#endif
} px_info;


#define PX_BASE(C)		(((px_info *)(C))->pxo)
#ifdef __alpha
#define PX_BASED(C)             (((px_info *)(C))->pxod)
#endif /* __alpha */

#define PX_SILENT	0
#define PX_CONSOLE	1		/* allow output (at all) on SLU3 */
#define PX_PSST		2
#define PX_TERSE	3
#define PX_TALK		4
#define PX_YAK		5
#define PX_GAB		7
#define PX_BLAB		10
#define PX_YOW		13
#define PX_NEVER	99
#define PX_DEBUGGING	PX_TALK		/* default debug level */
#define PX_PANIC	PX_SILENT	/* msgs allowed when panic'ing */

#define PX_NODEBUG			/* define this to compile out */
					/* debugging code */
#ifdef  PX_NODEBUG
#	define PX_DEBUG(L,S)
#else
#	define PX_DEBUG(L,S)	if ((_px_level=(L)) <= _px_debug) { S }
#endif

#endif /* _PXINFO_H_ */
