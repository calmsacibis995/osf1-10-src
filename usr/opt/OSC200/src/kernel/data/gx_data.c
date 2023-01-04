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
 *	@(#)$RCSfile: gx_data.c,v $ $Revision: 1.2.6.2 $ (DEC) $Date: 1993/09/21 21:50:23 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/* 
 * derived from gx_data.c	4.3	(ULTRIX)	12/6/90
 */

/************************************************************************
 * Modification History
 *
 * 30-Nov-89 -- Sam Hsu
 *	       Created.  Based on the qv/qd/sm/cfb drivers.
 *
 ************************************************************************/

#include <io/common/devio.h>
#include <io/common/devdriver.h>

#include <sys/param.h>
#include <sys/conf.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <io/dec/tc/gx.h>		/* DS5000 STIC/Stamp graphics */
#include <sys/tty.h>
#include <sys/map.h>
#include <sys/buf.h>
#include <sys/vm.h>
#include <sys/bk.h>
#include <sys/clist.h>
#include <sys/file.h>
#include <sys/uio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/kernel.h>
#include <hal/cpuconf.h>
#include <sys/exec.h>
#include <machine/cpu.h>
#include <io/dec/uba/ubavar.h>		/* auto-config headers */
#include <io/dec/tc/tc.h>
#include <sys/proc.h>
#include <sys/kmalloc.h>
#include <io/dec/tc/slu.h>
#include <io/dec/tc/lk201.h>
#include <io/dec/ws/vsxxx.h>
#include <io/dec/tc/xcons.h>


extern u_int  printstate;
extern struct mouse_report current_rep;	/* now in dc7085.c */

/*
 * Keyboard translation and font tables
 */
extern  char   *q_special[], q_font[];
extern  u_short q_key[], q_shift_key[], fg_font[];

extern  void 	wbflush();

extern  int	dcgetc();
extern	int	dcparam();

extern int	consDev;
extern u_short	pointer_id;	     /* id of pointer dev (mouse,tablet) */
				     /* see dc7085.c */
extern int	ws_display_type;     /* for sizer -wt */
		ws_display_units;    /* for sizer -wu */

/*
 * v_consputc is the switch that is used to redirect the console dcputc to the
 * virtual console vputc.
 * v_consgetc is the switch that is used to redirect the console getchar to the
 * virtual console vgetc.
 *
 * Routine to initialize virtual console. This routine sets up the 
 * graphic device so that it can be used as the system console. It
 * is invoked before autoconfig and has to do everything necessary
 * to allow the device to serve as the system console.           
 *
 */
extern 	int (*v_consgetc)();
extern 	int (*v_consputc)();
extern	int (*vs_gdopen)();
extern	int (*vs_gdclose)();
extern	int (*vs_gdselect)();
extern	int (*vs_gdkint)();
extern	int (*vs_gdioctl)();
extern	int (*vs_gdstop)();

int gxopen(), gxclose(), gxselect(), gxkint(), gxstart(), gxstop(),
    gxputc(), gxgetc(), gxioctl();

int gx_config(), gx_init(), gx_setup(), gx_kbdreset(),
    gx_init_colormap(), gx_init_mouse(), gx_init_stic(), gx_init_vdac(),
    gx_video_on(), gx_video_off(),
    gx_decode_option(), gx_howManyPlanes();

char *gx_devtype();

#ifdef HWDEBUG
int gx_info_get(), gx_info_gc(), gx_info_gone();
#endif

int gx_pos_cur(), gx_mouse_getc(), gx_mouse_putc(),
    gx_load_cursor(), gx_load_defcursor();

int gx_blitc(), gx_scroll(), gx_clearbitmap(), gx_rect(),
    gx_key_out(), gx_load_ccolor(), gx_load_colormap(), gx_panic(), gx_noop();

int *gx_panic2();

#ifndef GX_NODEBUG
int gx_putchar(), gx_puts(), gx_putb(), gx_putn(), gx_putd();
#endif

#ifdef BINARY			      /* {gx,ga,gq}.o */

extern int	gx_debug;
extern int	gx_mode_change;
extern int	gx_level;
extern int	gx_dropped_packet;
extern int	gx_unwedge_stic;
extern int	gx_pint_timeout;

extern int	gx_console;
extern int	gx_planemask;
extern int	gx_textfg;
extern int	gx_textbg;

extern gxInfo	*gxp;

extern int     (*_gx_config)();
extern int     (*_gx_ioctl)();
extern int *   (*_gx_getPacket)();
extern int     (*_gx_sendPacket)();
extern int     (*_gx_getPixBuff)();

extern int     (*_gx_init)();
extern int     (*_gx_open)();
extern int     (*_gx_close)();

extern char *	gx_err_msg[];
extern char	gx_planes[];

extern bt459Regs *_gx_vdac;
extern int	 *_gx_vdacReset;
extern sticRegs  *_gx_stic;
extern int	  _gx_modtype;
extern int	  _gx_stamp;

extern u_short	gxstd[];

extern struct uba_device *gxinfo[1];

extern gxKeyboard	   gx_keyboard;
extern struct mouse_report gx_last_rep;

extern int		gx_inkbdreset;
extern int		gx_mouseon;	/* Mouse is enabled when 1 */
extern u_int		gx_dev_inuse;	/* which minor dev's are in use */
extern int		gx_openflag;	/* graphics dev is open when non-0 */
extern struct proc *	gx_rsel;        /* process waiting for select */
extern struct proc *	gx_serverp;	/* process 2 open dev w/ O_NDELAY */

extern gxPriv *		gx_priv;	/* pointer to KMALLOC'd data */

extern unsigned		gx_slots;

#else					/* gx_data.o */

int	gx_debug		= GX_DEBUGGING;
int	gx_mode_change		= 0;
int	gx_level		= 0;
int	gx_dropped_packet	= 0;
int	gx_unwedge_stic		= 0;
int	gx_pint_timeout		= 0;

int	gx_console		= 0;
int	gx_planemask		= 0xff;
int	gx_textfg		= 0x010101;
int	gx_textbg		= 0x000000;

gxInfo	*gxp;				/* server's gxInfo struct */

/*
 * required linkage between gx and ga/gq:
 */
int	 (*_gx_config)()	= gx_panic;
int	 (*_gx_ioctl)()		= gx_panic;
int	*(*_gx_getPacket)()	= gx_panic2;
int	 (*_gx_sendPacket)()	= gx_panic;

int	 (*_gx_getPixBuff)()	= gx_panic; /* gq only */

bt459Regs *_gx_vdac;
int	  *_gx_vdacReset;
sticRegs  *_gx_stic;
int	   _gx_modtype;
int	   _gx_stamp;

/*
 * optional linkage between gx and ga/gq:
 */
int      (*_gx_init)()		= 0;
int	 (*_gx_open)()		= 0;
int	 (*_gx_close)()		= 0;

char * gx_err_msg[] = {
    "no error",
    "gx_priv",
    "3da board",
    "2da board",
    "stic poll/dma",
    "stic",
    "graphics ram",
    };

char	gx_planes[4] = { 3, 3, 3, 3 };	/* some invalid #'s */

u_short	gxstd[] = { 0 };

struct uba_device *gxinfo[1];

gxKeyboard	    gx_keyboard;
struct mouse_report gx_last_rep;     /* why isn't this in dc7085.c??? */

int		gx_inkbdreset = 0;
int		gx_mouseon = 0;	     /* Mouse is enabled when 1 */
u_int		gx_dev_inuse = 0;    /* which minor dev's are in use */
int		gx_openflag = 0;     /* graphics device is open when non-0 */
struct proc *	gx_rsel = 0;	     /* process waiting for select */
				     /* graphics device is open when >=0 */
struct proc	*gx_serverp = 0;     /* process to open dev w/ flag=O_NDELAY */

gxPriv	        *gx_priv;	     /* KM_ALLOC'd struct */

unsigned	 gx_slots = 0;

#endif binary
