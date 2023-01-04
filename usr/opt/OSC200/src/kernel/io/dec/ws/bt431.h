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
 *	@(#)$RCSfile: bt431.h,v $ $Revision: 1.2.4.7 $ (DEC) $Date: 1993/01/06 21:21:09 $
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

#ifndef bt431_DEFINED
#define bt431_DEFINED 1


#define BT431_PMAGAA_TYPE	0
#define BT431_TCO_TYPE		1
#define BT431_PV_TYPE		2


struct bt431 {
#ifdef __alpha
/* adjustments for SPARSE address space accesses */
	volatile unsigned int addr_low;
	volatile unsigned int addr_high;
	volatile unsigned int cursor_ram;
	volatile unsigned int control;
#else /* __alpha */
	volatile unsigned short addr_low;
	u_short pad1;
    	volatile unsigned short addr_high;
	u_short pad2;
	volatile unsigned short cursor_ram;
	u_short pad3;
	volatile unsigned short control;
	u_short pad4;
#endif /* __alpha */
};


struct bt431info {
        volatile struct bt431 	*btaddr;
	volatile struct bt431   *btaddr2;
	char type;
        char screen_on;                 /* whether screen is on */
        char on_off;                    /* whether cursor is on...*/
        char dirty_cursor;              /* has cursor been reloaded?*/
        short fb_xoffset;               /* offset to video */
        short fb_yoffset;
        short x_hot;                    /* hot spot of current cursor*/
        short y_hot;
	int unit;
	char inited;			/* kludge for cursor init */
        void (*enable_interrupt)();     /* enables one interrupt at V.R. */
	char cursor_was_on;		/* cursor state when video-offed */
	caddr_t cmap_closure;		/* hack for hook into colormap code */
        u_short bits[512];        
};

int bt431_load_cursor();
int bt431_recolor_cursor();
int bt431_set_cursor_position();
int bt431_load_formatted_cursor();
int bt431_cursor_on_off();
int bt431_clean_cursormap();

/*
 * 2 bytes of bt431 internal address.
 */

#define bt431_CUR_CMD           0x0000 /* cursor command reg */

#define bt431_CUR_XLO           0x0001 /* cursor x(lo) */
#define bt431_CUR_XHI           0x0002 /*  */
#define bt431_CUR_YLO           0x0003 /* cursor y(lo) */
#define bt431_CUR_YHI           0x0004 /*  */

#define bt431_WIN_XLO           0x0005 /* window x(lo) */
#define bt431_WIN_XHI           0x0006 /*  */
#define bt431_WIN_YLO           0x0007 /* window y(lo) */
#define bt431_WIN_YHI           0x0008 /*  */

#define bt431_WIN_WLO           0x0009 /* window width(lo) */
#define bt431_WIN_WHI           0x000a /*  */
#define bt431_WIN_HLO           0x000b /* window height(lo) */
#define bt431_WIN_HHI           0x000c /*  */

#define DUP431_B0(X) \
(unsigned short) ((((unsigned short)(X) << 8) & 0xff00) \
			 | (((unsigned short)(X)) & 0xff))

extern struct bt431info bt431_softc[];
extern struct bt431info bt431_type[];

#endif /* !bt431_DEFINED */
