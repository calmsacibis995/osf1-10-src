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
static char	*sccsid =  "@(#)$RCSfile: ws_device.c,v $ $Revision: 1.2.23.10 $ (DEC) $Date: 1993/11/17 17:28:18 $";
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
 *			Copyright (c) 1989 by				*
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
 * April 18, 91  Joel Gringorten  
 * 	Convert to OSF
 *
 ************************************************************************/

/* #define DEBUG_PRINT_ENTRY */
/* #define DEBUG_PRINT_INFO */
/* #define DEBUG_PRINT_ESC */
/* #define DEBUG_PRINT_CHAR */

/* #define DEBUG_PRINT_ERRORS */
/* #define DEBUG_PRINT_MOUSE */
/* #define DEBUG_PRINT_SELECT */
/* #define DEBUG_PRINT_IOCTL */
/* #define DEBUG_PRINT_KB_EVENTS */
/* #define DEBUG_PRINT_MS_EVENTS */
/* #define DEBUG_PRINT_MAPPING */

/* #define DPRINTF jprintf */
#define DPRINTF printf	  /* MUST be this for FLAMINGO/GENERIC!!!*/
/* #define DPRINTF xxputs */

#include <data/ws_data.c>
#include <hal/cpuconf.h>
#include  <sys/errno.h>
#include <sys/secdefines.h>
#include  <sys/tty.h>
#include  <sys/time.h>
#include  <sys/proc.h>
#include  <sys/user.h>
#include  <sys/exec.h>

#include  <sys/file.h>
#include  <sys/ipc.h>
#include  <sys/shm.h>
#include  <sys/conf.h>
#include  <io/common/devio.h>
#include  <sys/time.h>
#include  <sys/kernel.h>

#include  <io/dec/tc/xcons.h> 

#include <machine/pmap.h>               /* get vm stuff, incl. svatophys() */
#include <sys/vnode.h>                  /* Used to get device number */
#include <sys/specdev.h>                /* Def. 'v_rdev' vnode pseudo-cmpnent */
#include <sys/mman.h>                   /* Symbols for call to smmap() */
#include <sys/vmmac.h>                  /* btop() */

#include <sys/poll.h>

#if SEC_BASE
#include <sys/security.h>
#endif

#ifdef __alpha
#include <sys/vnode.h>
#include <sys/mman.h>
#include <vm/vm_mmap.h>
#endif

#ifdef __alpha
#ifdef btop
#undef btop
#endif
#define	btop(_a) alpha_btop(_a)
#ifdef PHYS_TO_K0
#undef PHYS_TO_K0
#endif /* PHYS_TO_K0 */
#define PHYS_TO_K0(_a) PHYS_TO_KSEG(_a)
#endif

extern int generic_console_active;

extern caddr_t ws_map_region();
extern int wsmmap();
extern int ws_num_controllers;

#define TTLOWAT(TP)     ((TP)->t_lowat)

/* NOTYET - #define X11_R6 - NOTYET */
#ifdef X11_R6
#define TIMEOUT_FREQ	(hz/100)
#else
#define TIMEOUT_FREQ	(hz)
#endif /* X11_R6 */



/*
 * Things left to do in this driver.
 * LINT it!!!
 *
 * reorganize into seperate files for input devices.
 * finish to deal with multiple instances (maybe not really necessary)
 * bt459 bug workaround.
 * faster interface for loading colormap.
 * escape box implementation.
 * get rid of last vestages of serial line dependency
 * make able to use driver when console is not on screen.
 * input extension work (not me).
 * debug event queue problems.  should allow server to pass in address.
 * devget defines for installation.
 * should we worry about SMP of the driver?
 * see if console rom routine can be used rather than crock lk201 routine.
 */

/*
 * ULTRIX settings for first open.		  
 */
#define IFLAGS	(EVENP|ECHO|XTABS|CRMOD)

/*
 * Termio flags will be set to these default values in non-termio mode to
 * provide a backward compatible ULTRIX environment. 
 */
#define IFLAG (BRKINT|IGNPAR|IXON|IXANY|ICRNL)
#define OFLAG (OPOST|TAB3|ONLCR)
#define LFLAG (ISIG|ICANON|ECHO)
#define CFLAG (CREAD|CS8|CLOCAL)

#define EVENTQUEUESIZE 100

/* Array for processing escape sequences. */
#define K_MAXESC	24
#define K_ESC		0x1b
#define K_SPACE		0x20
#define DEFAULT		(-1)

u_char	esc_seq[K_MAXESC];
u_char	*esc_spt	= esc_seq;

#define VT100_EMUL
/* #define DEBUG_VT100_IMPDEL */

#ifdef VT100_EMUL
int saved_row = -1, saved_col = -1;
int top_marg, bot_marg;
#endif /* VT100_EMUL */

/*
**  under conditions of heavy load, the mouse interrupt handlers
**  may switch screens well ahead of the
**  server's processing of the event queue. In those cases, some
**  cursor handling (particularly load_cursor()) to the 'old' screen
**  cause the cursor to hang. A flag (mouse_screen) is maintained
**  by the mouse event routines to point to the current screen, to help
**  keep the action on the straight and narrow path... -bg
*/
/*
 * Definitions supporting the mapping of device/kernel address space to
 * the user process.
 */
/***************************** OSF HACKERY!!! ************************/
/*
 * Define the process address at which frame buffer mapping will begin.
 */
#ifndef  __alpha
#define USER_QUEUE_ADDR ( 0x100000000 + ( 100 * 1024 * 1024 ) )
static char alpha_event_buffer[16384];          /* two pages */
vm_offset_t ws_eventQueue;
#else
#define USER_QUEUE_ADDR ( 0x480000000 )
extern vm_offset_t ws_eventQueue;
#endif

vm_offset_t mapped_area = (vm_offset_t) NULL;
/*
 * This kernel memory is allocated early on in {mips,alpha}_init.
 * ws_eventQueue is a physical page of memory reserved early on.
 */
vm_offset_t ws_user_addr;
/********************************************************************/



static dev_t the_ws_device_number = -1; /* Used by ws_map_region() */
static dev_t cons_dev_number = -1;      /* Used by ws_map_region() */

/*
 * The array 'ws_region_db' is shared between ws_map_region() and
 * wsmmap().  It is the mechanism used to convey the set of kernel
 * addresses which correspond to a given offset value, so that wssmap()
 * can do the right thing.
 *
 * The array is stored in increasing 'offset' order.
 *
 * Access to the database is synchronized by spltty().
 */
#define WS_REGION_MAX 100
static struct ws_region_db_s {
        off_t offset;                   /* Device offset value */
        caddr_t start;                  /* ClustAligned kmem address */
        size_t size;                    /* size in bytes (ClustSized) */
        int prot;                       /* Allowable access */
} ws_region_db[WS_REGION_MAX];

static int ws_region_db_max = -1;

ws_info ws_softc[1] = { 0, 0, 0, 0, 0, 0, 1,		/* ws */
			0, 0,				/* rsel */
			0,				/* queue */
			0,				/* events */
			0,				/* user_queue_address */
			0,				/* mb */
			0,				/* motion */
			0,				/* open_flag */
			0,				/* dev_in_use */
			0,				/* mouse_on */
			0,				/* keybd_reset */
			sizeof(ws_event),               /* max_event_size */
			0,				/* max_axis_count */
			{ 0, 0, 0},			/* last_rep */
			0,				/* new_switch */
			0,				/* old_switch */
			(struct proc *) NULL,		/* server process */
			0,				/* refcount */
			0,				/* screens */
			(ws_emulation_functions *)NULL,	/* emul_funcs */
			-1,				/* mouse_screen */
		      };

#define IS_MOUSE_ON 	(ws_softc[0].mouse_on != 0)
#define NO_EMUL_FUNCS 	(ws_softc[0].emul_funcs==(ws_emulation_functions *)NULL)
#define CHARPUT		(*(ws_softc[0].emul_funcs->charput))
#define CHARCLEAR	(*(ws_softc[0].emul_funcs->charclear))
#define CHARMVUP	(*(ws_softc[0].emul_funcs->charmvup))
#define CHARMVDOWN	(*(ws_softc[0].emul_funcs->charmvdown))
#define CHARATTR	(*(ws_softc[0].emul_funcs->charattr))

extern u_int printstate;

int wsstart(), map_queue();

extern int cpu;

extern int ws_graphics_config_error;

/*
 * Open the graphic device.
 *
 * Note: This routine allows a second open through /dev/mouse. This is used
 *       currently by the PV product which has two (2) processes that run
 *       in user space that must access the device, X server and Resource
 *       Manager. The code assumes that if more than one X server is started
 *       they will collide on socket creation and only one will successfully
 *       run.  
 */
/*ARGSUSED*/
wsopen(dev, flag)
	dev_t dev;
{
	register int i;
	register int unit = minor(dev);
	register struct tty *tp = &slu.slu_tty[unit];
	register ws_info *wi = &ws_softc[0];
	register ws_screens *wsp = &screens[wi->ws.console_screen];
	register ws_devices *wsd = &devices[wi->ws.console_keyboard];
	register ws_devices *wsm = &devices[wi->ws.console_pointer];

#ifdef DEBUG_PRINT_INFO
DPRINTF("wsopen: entry: unit = %d\n", unit);
#endif /* DEBUG_PRINT_INFO */

	if (wi->ws.num_screens_exist < 1)
		return(ENODEV);

#ifndef __alpha
	if (consDev != GRAPHIC_DEV) {
	  	printf( "ws:  Console not graphic device.\n" );
		return(ENODEV);
	}
#endif /* !__alpha */

	/*
 	 * The graphics device can be open only by one person 
 	 */
	if (unit == 1) {
	    /* only check config problem when "/dev/mouse" is opened */
	    if (ws_graphics_config_error)
		return(ENXIO);

	    if (wi->open_flag == 0) {

		wi->open_flag = 1;
	        wi->mouse_on = 1;	/* do this now so we know it below */
                the_ws_device_number = dev;
	        ws_user_addr = USER_QUEUE_ADDR;
                wi->dev_in_use |= GRAPHIC_DEV;  /* graphics dev is open */

		/*
		 **  opening the device should put the mouse_screen
		 **  active screen identifier into its 'scramble'
		 **  state if there is more than one screen available.
		 **  Only one screen justifies a zero. -bg
		 */
		if (wi->ws.num_screens_exist > 1)
			wi->mouse_screen = -1;
		else
			wi->mouse_screen = 0;

	        for (i = 0; i < wi->ws.num_screens_exist; i++) {
		     register ws_screens *wsp = &screens[i];

		     (*(wsp->f->init_screen))(wsp->f->sc, wsp->sp);
		     (*(wsp->cmf->init_color_map))(wsp->cmf->cmc);
		     (*(wsp->cf->load_cursor))
			     (wsp->cf->cc, wsp->sp, &default_cursor, NOSYNC);
		     ws_set_cursor_position (wsp->cf->cc, wsp, 0, 0);
	        }

	        (*(wsd->p.kp->init_keyboard))(wsd->p.kp->kc);

	        (*(wsd->p.kp->enable_keyboard))(wsd->p.kp->kc);
	        (*(wsm->p.pp->enable_pointer))(wsm->p.pp->pc);

	        ttychars(tp);
                tp->t_iflag = TTYDEF_IFLAG|ICRNL;
	        tp->t_oflag = TTYDEF_OFLAG|OPOST|ONLCR;
	        tp->t_lflag = TTYDEF_LFLAG;
	        tp->t_cflag = CS8|CREAD;
	        tp->t_ispeed = tp->t_ospeed = TTYDEF_SPEED;
	        tp->t_state = TS_ISOPEN|TS_CARR_ON;
	        ttsetwater(tp);
	        tp->t_flags = RAW;

#ifdef DS5000_100
	        if (cpu != DS_MAXINE)
#endif
	    	    cnparam(tp, &tp->t_termios);

		/*
		 * set up event queue for later
		 */
	        wi->ws.cpu = cpu;
	     } 
	     return(0);
	}

	wi->dev_in_use |= CONS_DEV;  /* mark console as open */
        cons_dev_number = dev;

	/*
	 * Whoops! gotta enable keyboard when /dev/console too... :-)
	 *  assume that the "reset_keyboard" is already done
	 */
	(*(wsd->p.kp->enable_keyboard))(wsd->p.kp->kc);

#if	SEC_BASE
	if (tp->t_state&TS_XCLUDE && !privileged(SEC_ALLOWDACACCESS, 0))
#else
	if (tp->t_state&TS_XCLUDE && u.u_uid != 0)
#endif
	    return (EBUSY);

	tp->t_addr = (caddr_t)tp;
	tp->t_oproc = wsstart;

	/*
	 * Look at the compatibility mode to specify correct 
	 * default parameters and to insure only standard specified 
	 * functionality.
	 */
#ifdef NOTDEF
	if ((u.u_procp->p_progenv == A_SYSV) || 
		(u.u_procp->p_progenv == A_POSIX)) {
		flag |= O_TERMIO;
		tp->t_line = TERMIODISC;
	}
#endif NOTDEF
	/*
	 * Set state bit to tell tty.c not to assign this line as the 
	 * controlling terminal for the process which opens this line.
	 */
#ifdef NOTDEF
	if ((flag & O_NOCTTY) /* && (u.u_procp->p_progenv == A_POSIX) jmg */)
		tp->t_state |= TS_ONOCTTY;
#endif NOTDEF
/*#endif O_NOCTTY*/
/*#endif NOTDEF*/
#ifdef	MACH
	tp->t_state |= TS_WOPEN;
#endif
	if ((tp->t_state&TS_ISOPEN) == 0) {
	    ttychars(tp);
            tp->t_iflag = TTYDEF_IFLAG|ICRNL;
	    tp->t_oflag = TTYDEF_OFLAG|OPOST|ONLCR;
	    tp->t_lflag = TTYDEF_LFLAG;
	    tp->t_cflag = CS8|CREAD;
	    tp->t_ispeed = tp->t_ospeed = TTYDEF_SPEED;
	    tp->t_state = TS_ISOPEN|TS_CARR_ON;
	    ttsetwater(tp);
	    tp->t_flags = IFLAGS;

	    /* set winsize struct for TIOCGWINSZ support */
	    tp->t_winsize.ws_row = wsp->sp->max_row;
	    tp->t_winsize.ws_col = wsp->sp->max_col;
	    tp->t_winsize.ws_xpixel = wsp->sp->width;
	    tp->t_winsize.ws_ypixel = wsp->sp->height;
	    
#ifdef DS5000_100
	    if (cpu != DS_MAXINE);
#endif
	    	cnparam(tp, &tp->t_termios);
	}
        /*
 	 * Process line discipline specific open.
 	 */
        return ((*linesw[tp->t_line].l_open)(dev, tp, flag));
}

/*
 * Close the graphic device.
 */

/*ARGSUSED*/
wsclose(dev, flag)
	dev_t dev;
	int flag;
{
	register struct tty *tp;
	register int unit = minor(dev);
	register ws_info *wi = &ws_softc[0];
	register ws_screens *wsp = &screens[wi->ws.console_screen];
	register ws_devices *wsd = &devices[wi->ws.console_keyboard];
	register ws_devices *wsm = &devices[wi->ws.console_pointer];
	register int i;
	extern ws_update_event_queue_time();

#ifdef DEBUG_PRINT_INFO
DPRINTF("wsclose: entry: unit = %d\n", unit);
#endif /* DEBUG_PRINT_INFO */
	/*
 	 * If unit is not the mouse call the line disc. 
	 *  otherwise clear the state flag, and put the keyboard
 	 *  into its normal "tty mode" state.
 	 */
	if (unit == 0) {
	    tp = &slu.slu_tty[unit];
	    (*linesw[tp->t_line].l_close)(tp);
	    ttyclose(tp);
	    wi->dev_in_use &= ~CONS_DEV;
	    tp->t_state = 0;
            ws_region_db_max = -1;      /* console can also map the option */
	} else {
	    untimeout(ws_update_event_queue_time, wi);
	    wi->mouse_on = 0;
	    if (wi->open_flag != 1)
		return(EBUSY);
	    else
		wi->open_flag = 0; /* mark the graphics device available */
	    wi->dev_in_use &= ~GRAPHIC_DEV;
	    (*(wsd->p.kp->reset_keyboard))(wsd->p.kp->kc);
	    if (wsm->p.pp->disable_pointer)
		    (*(wsm->p.pp->disable_pointer))(wsm->p.pp->pc);

	    for (i = 0; i < wi->ws.num_screens_exist; i++) {
		register ws_screens *wsp = &screens[i];
		(*(wsp->f->clear_screen))(wsp->f->sc, wsp->sp);
		(*(wsp->cmf->init_color_map))(wsp->cmf->cmc);
		(*(wsp->cf->load_cursor))
			(wsp->cf->cc, wsp->sp, &default_cursor, NOSYNC);
		/* if screen is console, leave cursor on */
		(*(wsp->cf->cursor_on_off))(wsp->cf->cc,
					    (i == 0) ? CURSOR_ON : CURSOR_OFF);
		ws_set_cursor_position(wsp->cf->cc, wsp, 
				       wsp->sp->col * wsp->sp->f_width, 
				       wsp->sp->row * wsp->sp->f_height);
                (*(wsp->cmf->video_on))(wsp->cmf->cmc);
		if (wsp->f->close) (*(wsp->f->close))(wsp->f->sc);
	    }
	    wi->user_queue_address = wi->queue = NULL;

	    /*
	     **  reset the mouse_screen field to impossible value -bg
	     */
	    wi->mouse_screen = -1;

	    /*
	     * Unregister any VM hook we may still have
	     */
	    if ( wi->server_proc_ref_count ) {
	    	int s = splvm();
		if ( wi->p_server_proc->task != NULL &&
		     wi->p_server_proc->task->map != NULL ) {
		    vm_map_t map = wi->p_server_proc->task->map;

		    pmap_clear_coproc_tbi( map->vm_pmap );
		}
		wi->server_proc_ref_count = 0;
		wi->server_proc_screens = 0;
	    	splx(s);
	    }
	}
}

/*
 * workstation device select routine.  If graphics device is open, then
 * detect input queue.
 */

wsselect(dev, events, revents, scanning)
dev_t dev;
short *events, *revents;
int scanning;
{
	register ws_info *wi = &ws_softc[0];
	register ws_event_queue *wsep = wi->queue;
	register int unit = minor(dev);
	register int s=spltty();

#ifdef DEBUG_PRINT_SELECT
	DPRINTF("wsselect(0x%x, 0x%lx, 0x%lx, 0x%x), unit %d\n",
		dev, events, revents, scanning, unit);
#endif /* DEBUG_PRINT_SELECT */

	if (unit == 1) {
		if (scanning) {
			if (*events & POLLNORM) {	/* if events okay */
				if (wsep->head != wsep->tail)
					*revents |= POLLNORM;
				else
					select_enqueue(&wi->rsel);
			} else {
				if (*events & POLLOUT) { /* can never write */
					splx(s);
					return(EACCES);
				}
			}
		} else {
#ifdef DEBUG_PRINT_SELECT
			DPRINTF("wsselect: calling select_dequeue: 0x%lx\n",
				*(unsigned long *)&wi->rsel);
#endif /* DEBUG_PRINT_SELECT */
			select_dequeue(&wi->rsel);
		}
	} else {
		splx(s);
#ifdef DEBUG_PRINT_SELECT
		DPRINTF("wsselect: calling ttselect\n");
#endif /* DEBUG_PRINT_SELECT */
		return(ttselect(dev, events, revents, scanning));
	}
	splx(s);
	return (0);
}


/*
 * Graphic device ioctl routine.
 */
/*ARGSUSED*/
wsioctl(dev, cmd, data, flag)
	dev_t dev;
	u_int cmd;
	register caddr_t data;
{
	register ws_info *wi = &ws_softc[0];
	register ws_screens *wsp;
	register ws_devices *wdp;
	register struct tty *tp;
	register int unit = minor(dev);
	register struct devget *devget;
	register ws_descriptor *wp = (ws_descriptor *)data;
	int error, i;
	ws_cursor_position *wcp = (ws_cursor_position *)data;
	/*
	 **  This flag gets set to TRUE if the mouse interrupt handler
	 **  has gotten ahead of the server's event queue processing,
	 **  and switched to a different screen... -bg
	 */
	int     screenLag = 0;

#ifdef DEBUG_PRINT_IOCTL
	DPRINTF("wsioctl(0x%x, 0x%x, 0x%lx, 0x%x), unit %d\n",
		dev, cmd, data, flag, unit);
#endif /* DEBUG_PRINT_IOCTL */

	if (((cmd >> 8) & 0377) == 'w') {
	/* save a bit of code, by doing the check once */
	    if (cmd & IOC_S) {
		register int screen = ((ws_screen_ioctl *)data)->screen;
                if (screen >= wi->ws.num_screens_exist)
                    return (ENXIO);
                if (screen < 0)         /* "current screen" */
                    screen =
			devices[wi->ws.console_pointer].p.pp->position.screen;
		wsp = &screens[screen]; 

		/*
		 **  If the mouse interrupt handler already has
		 **  switched screens, but the server hasn't caught up
		 **  yet, notify interested parties to ignore the hardware
		 **  during the catch-up. -bg
		 */
		if (wi->mouse_screen != -1 && screen != wi->mouse_screen)
			screenLag = 1;
	    }
	    if (cmd & IOC_D) {
		register int device;
		if (cmd & IOC_S) 
		  device = ((ws_screen_and_device_ioctl *)data)->device_number;
		else
		  device  = ((ws_device_ioctl *)data)->device_number;
                if (device >= wi->ws.num_devices_exist)
                        return (ENXIO);
                if (device < 0)
                    device = wi->ws.console_pointer;
		wdp = &devices[device]; 
	    }

	    switch(cmd)  {
	    case (int) GET_WORKSTATION_INFO:
		*wp = wi->ws;
		break;
	    case (int) WRITE_COLOR_MAP:
	    {
		ws_color_map_data *cp = (ws_color_map_data *)data;
		ws_color_cell entry;
		/* XXX this could be faster than two procedure calls/entry */
		for (i = cp->start;  i < cp->start + cp->ncells; i++) {
			copyin(cp->cells + i, &entry, sizeof(entry));
			error = (*(wsp->cmf->load_color_map_entry))
				(wsp->cmf->cmc, cp->map, &entry);
			if (error < 0) return error;
		}
                if (wsp->cmf->clean_color_map)
                    (*(wsp->cmf->clean_color_map))(wsp->cmf->cmc);
	        break;
	     }
	    case (int) SET_CURSOR_POSITION:
		/*
		 **  if we're lagging behind the ISR, this screen's
		 **  cursor is OFF. Don't bother to locate it. -bg
		 */
		if (screenLag)
			return 0;
		else
			return(ws_set_cursor_position
			       (wsp->cf->cc, wsp, wcp->x, wcp->y));
	    case (int) SET_POINTER_POSITION:
	    {
		ws_pointer_position *wpp = (ws_pointer_position *)data;
		ws_pointer *p;
		if (wpp->device_number != wi->ws.console_pointer) 
			return EINVAL;
		/*
		 **  if event queue processing is lagging behind the
		 **  interrupt handler and the cursor is on another screen,
		 **  don't write the data to the cursor hardware. -bg
		 */
		if (screenLag)
			return 0;
		p = (ws_pointer *)devices[wpp->device_number].p.pp;
		p->position.screen = wpp->screen;
		p->position.x = wpp->x;
		p->position.y = wpp->y;
		return(ws_set_cursor_position
			(wsp->cf->cc, wsp, wpp->x, wpp->y));
	    }
	    case (int) LOAD_CURSOR:
	    {	/* do some work so that cursor load routines don't have to */
                ws_pointer *p;
		register ws_cursor_data *cdp = (ws_cursor_data *)data;
		register nbytes = (((cdp->width + 31) >> 5)<< 2) * cdp->height;

		/*
		 **  if event queue processing is lagging behind the
		 **  interrupt handler and the cursor is on another screen,
		 **  don't write the data to the cursor hardware. -bg
		 */
		if (screenLag)
			return 0;
		if (nbytes > sizeof(wi->cbuf)) nbytes = sizeof(wi->cbuf);
		copyin(cdp->cursor, wi->cbuf, nbytes);
		copyin(cdp->mask,   wi->mbuf, nbytes);
                p = (ws_pointer *)devices[wi->ws.console_pointer].p.pp;
		cdp->cursor = wi->cbuf;
		cdp->mask   = wi->mbuf;
                /* stash away ws_cursor_data for cursor tracking */
                p->cursor = *cdp;
                return (*(wsp->cf->load_cursor))
                        (wsp->cf->cc, wsp->sp, cdp, VSYNC);
	    }
	    case (int) RECOLOR_CURSOR:
	    {
	        ws_cursor_color *ccp = (ws_cursor_color *) data;
		/*
		 **  if event queue processing is lagging behind the
		 **  interrupt handler and the cursor is on another screen,
		 **  don't bother to recolor this cursor, since it's OFF,
		 **  anyway. -bg
		 */
		if (screenLag)
			return 0;
		else
			return (*(wsp->cf->recolor_cursor))
				(wsp->cf->cc, wsp->sp, 
				 &ccp->foreground, &ccp->background);
	     }
	    case (int) GET_SCREEN_INFO:
		*((ws_screen_descriptor *)data) = *(wsp->sp);
		break;
	    case (int) GET_DEPTH_INFO: 
	    {	register ws_depth_descriptor *dp = 
			(ws_depth_descriptor *) data;
		if ((dp->which_depth) < 0 ||
			(dp->which_depth >= wsp->sp->allowed_depths))
			return EINVAL;
		*((ws_depth_descriptor *)data) = wsp->dp[dp->which_depth];
		break;
	    }
	    case (int) MAP_SCREEN_AT_DEPTH:
	    {
		register ws_map_control *mp = (ws_map_control *) data;
		if ((mp->which_depth) < 0 ||
			(mp->which_depth >= wsp->sp->allowed_depths))
			return EINVAL;
		return ((*(wsp->f->map_unmap_screen))
			(wsp->f->sc, wsp->dp, wsp->sp, mp));
	    }
	    case (int) GET_VISUAL_INFO:
	    {	register ws_visual_descriptor *vp = 
			(ws_visual_descriptor *) data;
		if ((vp->which_visual) < 0 ||
		    (vp->which_visual >= wsp->sp->nvisuals))
			return EINVAL;
		*((ws_visual_descriptor *)data) = wsp->vp[vp->which_visual];
		break;
	    }
	    case (int) VIDEO_ON_OFF:
	    {
		register ws_video_control *vcp;
		vcp = (ws_video_control *) data;
		if (vcp->control == SCREEN_OFF) 
		     (*(wsp->cmf->video_off))(wsp->cmf->cmc);
		else (*(wsp->cmf->video_on))(wsp->cmf->cmc);
		break;
	    }
	    case (int) SET_POINTER_BOX:
	    {  	register ws_pointer_box *pbp = (ws_pointer_box *) data;
		register int newx, newy;
		ws_pointer *p;    

		/*
		 **  if event queue processing is lagging behind the
		 **  interrupt handler and the cursor is on another screen,
		 **  don't write the data to the cursor hardware. -bg
		 */
		if (screenLag)
			return 0;
		if (pbp->device_number != wi->ws.console_pointer) 
			return EINVAL;
		p = (ws_pointer *)devices[pbp->device_number].p.pp;
		p->constrain = *pbp;
		/* check validity against selected screen size */
	 	if (p->constrain.box.left < 0)
			p->constrain.box.left = 0;
		if (p->constrain.box.right > wsp->sp->width)
			p->constrain.box.right = wsp->sp->width - 1;
		if (p->constrain.box.top < 0)
			p->constrain.box.top = 0;
		if (p->constrain.box.bottom > wsp->sp->height)
			p->constrain.box.bottom = wsp->sp->height - 1;
		/* if the cursor is outside, move it to nearest edge... */
		if (!p->constrain.enable)
			break;
		newx = p->position.x;
		newy = p->position.y;
	 	if (newx < pbp->box.left)   newx = pbp->box.left;
		if (newx > pbp->box.right)  newx = pbp->box.right;
		if (newy < pbp->box.top)    newy = pbp->box.top;
		if (newy > pbp->box.bottom) newy = pbp->box.bottom;
		p->position.screen = pbp->screen;
		ws_set_cursor_position(wsp->cf->cc, wsp, newx, newy);
		break;
	    }
	    case (int) SET_ESCAPE_BOX:
	    {   register ws_pointer_box *pbp = (ws_pointer_box *) data;
		ws_pointer *p;
		/*
		 **  if event queue processing is lagging behind the
		 **  interrupt handler and the cursor is on another screen,
		 **  don't write the data to the cursor hardware. -bg
		 */
		if (screenLag)
			return 0;

/* XXX following line should get replaced when more general input stuff done */
		if (pbp->device_number != wi->ws.console_pointer) 
			return EINVAL;
		p = (ws_pointer *)devices[pbp->device_number].p.pp;
		p->suppress = *pbp;
		break;
	    }
	    case (int) GET_AND_MAP_EVENT_QUEUE:
	    {	
	      /* XXX this needs some help, should take address of where the
	         queue should be put....*/
		ws_event_queue **weqp;
		extern ws_update_event_queue_time();
		int nbytes;
		short motion_size;
		weqp = (ws_event_queue **) data;
	        motion_size = sizeof(ws_motion_history) /*  +
		    ((wi->max_axis_count == 0 ) ? 0 :
		    (sizeof(short) * (wi->max_axis_count - 1))) */ ;
		nbytes = (int)wi->max_event_size * EVENTQUEUESIZE
  		  + sizeof(ws_event_queue) 
		  + sizeof(ws_motion_buffer)
		  + motion_size * MOTION_BUFFER_SIZE;

                if (mapped_area == (vm_offset_t) NULL) {
                    mapped_area =  (vm_offset_t) PHYS_TO_K0(ws_eventQueue);
                }

		wi->user_queue_address = (ws_event_queue *)
                    ws_map_region(mapped_area, NULL, nbytes, 0600, (int *)NULL);

		wi->queue = (ws_event_queue *) mapped_area;
		wi->queue->events = (ws_event *)(wi->user_queue_address + 1);
		wi->events = (ws_event *) ( (caddr_t) mapped_area + 
					sizeof(ws_event_queue));
		wi->queue->time = TOY;
		wi->queue->size = EVENTQUEUESIZE;
		wi->queue->event_size = wi->max_event_size;
		wi->queue->head = wi->queue->tail = 0;
		wi->mb = (ws_motion_buffer *)( (caddr_t) wi->events +
			 (wi->max_event_size * EVENTQUEUESIZE));

		wi->motion = (ws_motion_history *) 
			( (caddr_t) wi->mb + sizeof(ws_motion_buffer));
		wi->queue->mb = 
			(ws_motion_buffer *) ( (caddr_t) wi->queue->events 
			+ (wi->max_event_size * EVENTQUEUESIZE));
		wi->mb->motion =  (ws_motion_history *) 
		    ( (caddr_t) wi->queue->mb + sizeof(ws_motion_buffer));
		wi->mb->entry_size = motion_size;
		wi->mb->axis_count = wi->max_axis_count;
		wi->mb->size = MOTION_BUFFER_SIZE;
		wi->mb->next = 0;
		*weqp = wi->user_queue_address;
		timeout (ws_update_event_queue_time, wi, TIMEOUT_FREQ);
		break;
	    }
	    case (int) SET_EDGE_CONNECTION:
	    {
		register ws_edge_connection *nep = (ws_edge_connection *)data;
		register ns = wi->ws.num_screens_exist;
		if ( nep->adj_screens.top   < -1  || 
		    nep->adj_screens.bottom < -1  || 
		    nep->adj_screens.right  < -1  || 
		    nep->adj_screens.left   < -1  || 
		    nep->adj_screens.top    >= ns ||
		    nep->adj_screens.bottom >= ns ||
		    nep->adj_screens.right  >= ns ||
		    nep->adj_screens.left   >= ns) return EINVAL;
		wsp->adj_screens = nep->adj_screens;
		break;
	    }		
	    case (int) GET_EDGE_CONNECTION:
	    {
		register ws_edge_connection *nep = (ws_edge_connection *)data;
		nep->adj_screens = wsp->adj_screens;
		break;
	    }
	    case (int) CURSOR_ON_OFF:
	    {
		register ws_cursor_control *ccp = (ws_cursor_control *)data;
		/*
		 **  if event queue processing is lagging behind the
		 **  interrupt handler and the cursor is on another screen,
		 **  the cursor is already in the appropriate state. -bg
		 */
		if (screenLag)
			return 0;
		else
			return (*(wsp->cf->cursor_on_off))
				(wsp->cf->cc, ccp->control);
	     }
	    case (int) SET_MONITOR_TYPE:
	    {
		register ws_monitor_type *wmp = (ws_monitor_type *) data;
		wsp->sp->monitor_type = wmp->monitor_type;
		break;
	    }
	    case (int) SET_POINTER_CONTROL:
	    {
		ws_pointer_control *wpcp = (ws_pointer_control *) data;
		int type = devices[wpcp->device_number].device_type;

		if ((type != MOUSE_DEVICE) && (type != TABLET_DEVICE))
			return EINVAL;
		devices[wpcp->device_number].p.pp->pr = *wpcp;
		break;
	    }
	    case (int) GET_POINTER_CONTROL:
	    {
		ws_pointer_control *wpcp = (ws_pointer_control *) data;
		int type = devices[wpcp->device_number].device_type;
		if ((type != MOUSE_DEVICE) && (type != TABLET_DEVICE))
			return EINVAL;
		*wpcp = devices[wpcp->device_number].p.pp->pr;
		break;
	    }
	    case (int) GET_DEVICE_TYPE:
	    {
                ws_hardware_type *whtp = (ws_hardware_type *) data;
                ws_device *dp = devices[whtp->device_number].p.dp;

		/*
		 * for console pointer, we return an entire data structure,
		 *  while for any other, we just return the hardware type.
		 */
                if (whtp->device_number == wi->ws.console_pointer) {
                    *whtp = *(ws_hardware_type *)(dp->dc);
                }
                else {
                    whtp->hardware_type = dp->hardware_type;
                    whtp->buttons = 0;  /* XXX */
                }
                break;
	    }
	    case (int) SET_KEYBOARD_CONTROL:
	    {
		ws_keyboard_control *wskp = (ws_keyboard_control *) data;

		int type = devices[wskp->device_number].device_type;
		if (type != KEYBOARD_DEVICE) return EINVAL;
		if (wdp->p.kp->set_keyboard_control) 
			return ((*(wdp->p.kp->set_keyboard_control))
				(wdp->p.kp->kc, wdp->p.kp, wskp));

		break;
	    }
	    case (int) GET_KEYBOARD_CONTROL:
	    {
		ws_keyboard_control *wskp = (ws_keyboard_control *) data;
		int type = devices[wskp->device_number].device_type;
		if (type != KEYBOARD_DEVICE) return EINVAL;
		*wskp = wdp->p.kp->control;
		break;
	    }
	    case (int) RING_KEYBOARD_BELL:
	    {
	        short *device_number = (short *) data;
		int type = devices[*device_number].device_type;
		if (type != KEYBOARD_DEVICE) return EINVAL;
		(*(wdp->p.kp->ring_bell)) (wdp->p.kp->kc, wdp->p.kp);
		break;
	    }
	    case (int) GET_KEYBOARD_DEFINITION:
	    {
	        ws_keyboard_definition *wkdp = (ws_keyboard_definition *) data;
		register int device_number = wkdp->device_number;
		int type = devices[device_number].device_type;
		if (type != KEYBOARD_DEVICE) return EINVAL;
		*wkdp = *devices[device_number].p.kp->definition;
		wkdp->device_number = device_number;
		break;
	    }
	    case (int) GET_KEYSYMS_AND_MODIFIERS:
	    {
	        ws_keysyms_and_modifiers *wkmp = 
	    		(ws_keysyms_and_modifiers *) data;
		register int device_number = wkmp->device_number;
		register ws_keyboard *keyboard;
		int type = devices[device_number].device_type;
		if (type != KEYBOARD_DEVICE) return EINVAL;
		keyboard = devices[device_number].p.kp;
		copyout (keyboard->modifiers, wkmp->modifiers, 
			keyboard->definition->modifier_keycode_count 
			* sizeof(ws_keycode_modifiers));
		copyout (keyboard->keysyms, wkmp->keysyms,
			keyboard->definition->keysyms_present
			* sizeof (unsigned int));
		copyout (keyboard->keycodes, wkmp->keycodes,
			keyboard->definition->keysyms_present
			* sizeof (unsigned char));
		break;
	    }
            case (int) SET_TABLET_OVERHANG:
            {
                ws_tablet_control *ioc = (ws_tablet_control *) data;
                /*printf("tabovh %d\n", ioc->data);*/
                if (ioc->data < 0 || ioc->data > 10)
                    return EINVAL;
                (*wdp->p.pp->set_tablet_overhang)(wdp->p.pp, wsp->sp,
						  ioc->data);
                break;
            }
            case (int) PUT_EVENT_ON_QUEUE:
            {
                ws_event *wep = (ws_event *) data;
		/* FIXME - size? */
		return(ws_enter_event(wep, sizeof(ws_event)));
            }
	    default:
		if (cmd & IOC_S) {
                    /* if a screen has an ioctl handler, then call it */
                    if (wsp->f->ioctl == NULL) return EINVAL;
                    error = (*(wsp->f->ioctl))(wsp->f->sc, cmd, data, flag);
                    if (error > 0) return error;
		}
		if (cmd & IOC_D) {
                    /* if a device has an ioctl handler, then call it */
                    if (wdp->p.dp->ioctl == NULL) return EINVAL;
                    error = ((*(wdp->p.dp->ioctl))(wdp->p.dp->dc, cmd,
						   data, flag));
                    if (error > 0) return error;
		}
                if (error == 0) return 0;
                /*printf("wsioctl: unk\n");*/
		return EINVAL;
	    }
	return (0);
	}
	switch (cmd) {
	    case (int) DEVIOCGET: 			/* device status */
		devget = (struct devget *)data;
		bzero(devget, sizeof(struct devget));
		devget->category = DEV_TERMINAL;	/* terminal cat.*/
		devget->bus = DEV_NB;			/* NO bus	*/
		/* "interface" field */
		bcopy(DEV_VS_SLU, devget->interface, strlen(DEV_VS_SLU));
		/* "device" field */
		if (unit == 0) { /* "/dev/console" */
		        bcopy("COLOR", devget->device, 6);   /* Ultrix "fb"*/
		}
		else if (unit == 1) { /* "/dev/mouse" */
		    ws_pointer *p =
			    (ws_pointer *)devices[wi->ws.console_pointer].p.pp;
		    if (pointer_id == MOUSE_ID || pointer_id == TABLET_ID) {
			bcopy(p->name, devget->device, strlen(p->name));
		    }
		    else {
			bcopy(DEV_UNKNOWN, devget->device, strlen(DEV_UNKNOWN));
		    }
/* FIXME FIXME - do we want to set "dev_name" with keyboard name here? */
		}
		/* miscellaneous fields */
/* FIXME FIXME - do we really need/want to do this for *every* device? */
		bcopy("COLOR", devget->dev_name, 6);	/* Ultrix "fb"	*/

		devget->adpt_num = 0;			/* NO adapter	*/
		devget->nexus_num = 0;			/* fake nexus 0	*/
		devget->bus_num = 0;			/* NO bus	*/
		devget->ctlr_num = 0;			/* cntlr number */
		devget->slave_num = unit;		/* line number 	*/
		devget->unit_num = unit;		/* dc line?	*/
		/* XXX TODO: should say not supported instead of zero!	*/
		devget->soft_count = 0;			/* soft err cnt */
		devget->hard_count = 0;			/* hard err cnt */
		devget->stat = 0;			/* status	*/
		devget->category_stat = 0;		/* cat. stat.	*/
/* XXX what to do about devget defines... */
		break;

	    default:					/* not ours ??  */
		tp = &slu.slu_tty[unit];
		error = (*linesw[tp->t_line].l_ioctl)(tp, cmd, data, flag);
		if (error >= 0)
			return (error);
		error = ttioctl(tp, cmd, data, flag);
		if (error >= 0) 
			return (error);
	        /* if error = -1 then ioctl does not exist */
		return (EINVAL);
		break;
	}
	return (0);
}


/*
 * map a region of memory.  If the function succeeds, it returns the address
 * where it is visible to the user (the pages are double mapped, so that
 * physical memory can be accessed, for example framebuffers or I/O registers.)
 *
 * Since osf doesn't support Ultrix sm_get/att, A static pointer into the
 * servers address space is maintained.  All allocations get rounded up to
 * the next highest MEG. That way all user shared buffers are meg aligned.
 */

/*
 * This routine returns the device number associated with the given fd.
 * If the fd does not represent an open file or if any other errors are
 * encountered, -1 is returned instead.
 */
static  dev_t device_from_fd_hack(fd)
        int fd;
{
        struct file *fp;
        struct vnode *vp;
        dev_t dev;

        dev = -1;               /* illegitimate device number */
        if (getvnode(fd, &fp))
            return (-1);

        if (fp) {
            vp = (struct vnode *)fp->f_data;
            if (vp) {
                if (vp->v_type == VCHR)
                    dev = vp->v_rdev;
            }
            FP_UNREF(fp);
        }
        return (dev);
}

static int saved_fd = 0;        /* previous valid fd, look here first */

/*
 * This routine finds the fd (in the current process) which points to the
 * device specified.  If no such fd exists, -1 is returned instead.
 *
 * In order to avoid poking through most of the fd table each time, we
 * remember the last fd we returned and check that one first;  because the
 * device is not shared, this optimization will be used all but the first
 * time (but we always check validity).
 */
static  int find_fd_hack(dev)
        dev_t dev;
{
        volatile struct ufile_state *ufp = &u.u_file_state;
        int fd;

        fd = -1;
        if (dev == device_from_fd_hack(saved_fd)) {
            fd = saved_fd;
        } else {
            int i;

            for (i=0; i <= ufp->uf_lastfile; i++) {

                if (dev == device_from_fd_hack(i)) {
                    fd = i;
                    break;
                }
            }
        }
        saved_fd = fd;
        return (fd);
}

/*
 * This routine acts as a (somewhat tailored) jacket around smmap(),
 * giving default values to some arguments and putting the whole thing into
 * the format demanded by the syscall interface.  What would normally be
 * returned in errno is placed in the location designated by errnop, and the
 * return value from this routine is the virtual address (in the context of
 * the current process) into which the region was mapped.  If the mapping
 * failed, then the return value is 0.
 */
static  caddr_t ws_smmap(uaddr, len, prot, fd, pos, errnop)
        caddr_t uaddr;
        size_t len;
        int prot;
        int fd;
        off_t pos;
        int *errnop;
{
        struct proc *p;
        long retval;
        struct args {
                caddr_t addr;
#ifdef __alpha
		u_long	len;
		long	prot;
		long	flags;
		long	fd;
		u_long	pos;
#else /* __alpha */
                size_t  len;
                int     prot;
                int     flags;
                int     fd;
                off_t   pos;
#endif /* __alpha */
        } uargs;

        uargs.addr = uaddr;
        uargs.len = len;
        uargs.prot = prot;
        uargs.flags = MAP_FILE | MAP_VARIABLE | MAP_SHARED;
        uargs.fd = fd;
        uargs.pos = pos;

        *errnop = smmap(u.u_procp, &uargs, &retval);
        return ( (caddr_t) retval );
}

#define DWN(x) ((long unsigned int)(x) & ~(CLBYTES-1))

caddr_t
ws_map_region (kaddr, uaddr, nbytes, how, erroraddr)
        caddr_t kaddr;          /* kernel address of memory to map */
        caddr_t uaddr;          /* try to put it in curproc at this address */
        register int nbytes;    /* it is this many bytes big */
        int how;                /* ULTRIX-style protection argument */
        int *erroraddr;         /* errno address (NULL to discard value) */
{
        caddr_t result;
        long unsigned int tmp1;
        long unsigned int start = DWN(kaddr);
        long unsigned int end = (long unsigned int)kaddr + nbytes - 1;
        long int size;
        dev_t  dev_num;
	register struct ws_region_db_s *rdbp;
	vm_offset_t physaddr;

        int error;
        int prot;
        int fd;
        int s;
        int elem;
        int i;

        size = DWN(end + CLBYTES) - start;      /* Round up to next page */
        prot = 0;
        if (how & 0400)
            prot |= PROT_READ;
        if (how & 0200)
            prot |= PROT_WRITE;


#ifdef DEBUG_PRINT_MAPPING
	DPRINTF("ws_map_region(0x%lx, 0x%lx, 0x%x, 0x%x, 0x%lx)\n",
		kaddr, uaddr, nbytes, how, erroraddr);
	DPRINTF("ws_map_region: size 0x%x  start 0x%lx  end 0x%lx\n",
		size, start, end);
#endif /* DEBUG_PRINT_MAPPING */

        /*
         * Get an fd for the current process which points to the ws device
         * (/dev/mouse or /dev/console).
         */
        if ((fd = find_fd_hack(the_ws_device_number)) < 0) {
           if ((fd = find_fd_hack(cons_dev_number)) < 0) {
              error = ENODEV;
              goto bad;
           } else {
              dev_num = cons_dev_number;
           }
        } else {
           dev_num = the_ws_device_number;
        }

        /*
         * Determine if we already have a region which encompasses (or is the
         * same as) the requested one.  If so, use the previous definition.
         */
        s = spltty();
        elem = -1;
        for (i=0; i<=ws_region_db_max; i++) {
            caddr_t db_start = ws_region_db[i].start;
            caddr_t db_end = db_start + ws_region_db[i].size;
            caddr_t end = (caddr_t) start + size;

            if ((db_start <= (caddr_t) start) && (end <= db_end)) {
                elem = i;
                ws_region_db[elem].prot |= prot; /* allow the greater access */
#ifdef DEBUG_PRINT_MAPPING
		DPRINTF("ws_map_region: region encompassed or same as %d\n",
			elem);
#endif /* DEBUG_PRINT_MAPPING */
                break;
            }
        }

        /*
         * We couldn't find (or couldn't use) a previous definition, so add
         * one.  For the 'offset' value, choose zero (if this is the first
         * region) or use the next available offset (the last region's offset
         * plus the size of the last region).
         */
        if (elem < 0) {
            if ((ws_region_db_max + 1) < WS_REGION_MAX) {

                elem = (++ws_region_db_max);
                rdbp = &ws_region_db[elem];
                rdbp->offset =          /* offset always paligned (size is) */
                    ( elem==0 ? 0 : (rdbp-1)->offset + (rdbp-1)->size );
                rdbp->start = (caddr_t) start;
                rdbp->size = size;      /* size always page aligned */
                rdbp->prot = prot;
            }
        } else
		rdbp = &ws_region_db[elem];
        splx(s);

        /*
         * If we *still* don't have an element, then flag an error and
         * quit.  Otherwise, get the region mapped for us (which results in
         * a set of calls back to wsmmap() ).
         */
        if (elem < 0) {
            error = ENOSPC;
            goto bad;
        }

#ifdef	__alpha
#define GH_THRESHHOLD(SIZE, ADDR) ((SIZE) >= (1<<16) /* enough for gh > 0 */ \
				   && (ADDR) >= (1L << 32)) /* FLAMINGO TC */

	physaddr = KSEG_TO_PHYS(rdbp->start);
	if (GH_THRESHHOLD(rdbp->size, physaddr)) {
		if (uaddr == (caddr_t)0) {
			/*
			 * Select a virtual address which is equivalent to
			 * the physical address modulo the granularity hint
			 * size.
			 */
			register vm_offset_t bigpage; /* GH size */
			
			bigpage = 1 << (PGSHIFT + 3);
			if (rdbp->size >= (1 <<  (PGSHIFT + 6)))
			    bigpage = 1 <<  (PGSHIFT + 6);
			if (rdbp->size >= (1 <<  (PGSHIFT + 9)))
			    bigpage = 1 <<  (PGSHIFT + 9);
			ws_user_addr
			       = (ws_user_addr + (bigpage-1)) & ~(bigpage-1);
			uaddr = (caddr_t)(ws_user_addr
					  | (physaddr & (bigpage-1)));
			ws_user_addr = (vm_offset_t)uaddr + rdbp->size;
		}
#ifdef DEBUG_PRINT_MAPPING
		DPRINTF("ws_map_region: pmap: uaddr 0x%lx  physaddr 0x%lx\n",
			uaddr, physaddr);
#endif /* DEBUG_PRINT_MAPPING */
		pmap_enter_range(vm_map_pmap(current_task()->map), uaddr,
				 physaddr, rdbp->size, prot);
		result = uaddr;
	} else {
		vm_map_t	user_map = current_task()->map;
		vm_offset_t	ret;
		int error;
		struct vp_mmap_args args;
		register struct vp_mmap_args *ap = &args;
		struct vnode *vp;
		vm_offset_t user_addr = (vm_offset_t) uaddr;

		if ( user_addr == (vm_offset_t) 0 ) user_addr = ws_user_addr;
		if (vfinddev(dev_num, VCHR, &vp)) goto bad; 
		VREF(vp);
		ap->a_vaddr = (vm_offset_t *) &user_addr;
		ap->a_dev = dev_num;
		ap->a_mapfunc = wsmmap;
		ap->a_size = size;
		ap->a_offset = (vm_offset_t) rdbp->offset;
		ap->a_prot = VM_PROT_READ|VM_PROT_WRITE;
		ap->a_maxprot = VM_PROT_ALL;
		ap->a_flags = MAP_SHARED;
#ifdef DEBUG_PRINT_MAPPING
		DPRINTF("ws_map_region: u_dev: user_addr 0x%lx  size 0x%x\n",
			user_addr, size);
#endif /* DEBUG_PRINT_MAPPING */
		error = u_dev_create(user_map, vp, (vm_offset_t) ap);
		VUNREF(vp);
		if (error) {
			printf("ws_map_region: failed to map\n");
			goto bad;
		}
		result = (caddr_t) user_addr;
		if (uaddr == 0)
			ws_user_addr = user_addr + rdbp->size;
	}
#else /* __alpha */
	if ( uaddr == (caddr_t) 0 ) uaddr = (caddr_t) ws_user_addr;
	if (!(result =
              ws_smmap(uaddr, size, prot, fd, ws_region_db[elem].offset,
		       &error)))
            goto bad;

#define M (1024 * 1024)
	if ( result == uaddr ) {
	      ws_user_addr += (M - (size % M)) + size;
	}
#undef M
#endif /* __alpha */

        /* the following returns the low order bits to the mapped result. */
        tmp1 = (long unsigned int)result;
        tmp1 |= (long unsigned int) kaddr & (CLBYTES-1);
        if (erroraddr)
            *erroraddr = 0;
        return ((caddr_t) tmp1);
bad:
        printf ("ws0: cannot map shared data structures\n");
        printf ("u_error = %d\n", error);
        if (erroraddr)
            *erroraddr = error;
        return ((caddr_t) NULL);
}

/*
 * mmap interface routine (cooperates with ws_map_region()).
 */
wsmmap(dev, off, prot)
        dev_t   dev;
        off_t   off;
        int     prot;
{

        static int last_elem = 0;
        int s;
        int elem;
        int i;
        caddr_t physaddr;
        struct ws_region_db_s *rdbp;
        struct ws_region_db_s elem_data;

        /*
         * Synchronize access to the region database.  Check if the offset
         * falls within the offset range of the last used element.  If so,
         * we'll use that element (and save the time for a search).
         */
        s = spltty();
        elem = -1;
        if (last_elem <= ws_region_db_max) {
            rdbp = &ws_region_db[last_elem];
            if ((rdbp->offset <= off) &&
                  (off < rdbp->offset + rdbp->size)) {
                elem = last_elem;
                elem_data = *rdbp;
            }
        }

        /*
         * If the last used database element wasn't the right one, scan the
         * database for one which matches.  After that, we're finished with
         * the database, so release our lock.
         */
        if (elem < 0) {
            for (i = 0; i <= ws_region_db_max; i++) {
                rdbp = &ws_region_db[i];
                if ((rdbp->offset <= off) &&
                      (off < rdbp->offset + rdbp->size)) {
                    elem = i;
                    elem_data = *rdbp;
                }
            }
        }
        splx(s);

        /*
         * If we didn't find a matching element, return a pfn of -1 (the
         * caller expects that as an error indication).  Otherwise, check to
         * see if the desired access is allowed (compare the prot argument
         * against the prot component in the database).  If everything is
         * okay, then figure out the pfn for the corresponding system
         * virtual address and return that instead.
         */
        if (elem < 0) {
#ifdef DEBUG_PRINT_ERRORS
DPRINTF("ws_device: wsmmap bad elem %d offset %016lx.\n", elem, off );
#endif /* DEBUG_PRINT_ERRORS */
            return (-1);
        } else {
            caddr_t kernva;
            last_elem = elem;
            if (prot != (prot & elem_data.prot)) {
#ifdef DEBUG_PRINT_ERRORS
DPRINTF("ws_device: wsmmap bad prot %d %d.\n", prot, elem_data.prot );
#endif /* DEBUG_PRINT_ERRORS */
                return (-1);
            }
           kernva = elem_data.start +
                        (unsigned long int)(off - elem_data.offset);
            if (svatophys(kernva, &physaddr) != KERN_SUCCESS) {
#ifdef DEBUG_PRINT_ERRORS
DPRINTF( "ws_device: wsmmap bad addr 0x%016lx 0x%016lx 0x%016lx.\n",
       kernva, physaddr, off );
#endif /* DEBUG_PRINT_ERRORS */
                return (-1);
            }
        }

        return (btop(physaddr));
}

/*
 * Start transmission
 */
wsstart(tp)
	register struct tty *tp;
{
	register int unit, c;
	register ws_info *wi = &ws_softc[0];
	register ws_screens *wsp = &screens[wi->ws.console_screen];
	int s, xcons_status;

	unit = minor(tp->t_dev);

	/*
 	 * If it's currently active, or delaying, no need to do anything.
 	 */
	if (tp->t_state & (TS_TIMEOUT|TS_BUSY|TS_TTSTOP))
		goto out;
	/*
 	 * Display chars until the queue is empty, if the second subchannel 
	 * is open direct them there. Drop characters from any lines other 
	 * than 0 on the floor. TANDEM is set on second subchannel for flow 
	 * control.
	 *
	 * NOTE: IPL is raised/lowered around each character's processing.
 	 */
	for (;;) 
	{
		tp->t_state &= ~TS_BUSY; 
		if (tp->t_state & TS_TTSTOP)
			break;

		s = spltty();
		
		if (tp->t_outq.c_cc <= 0) {
			splx(s);
			break;
		}

		if (unit == 0) {		/* console device */
			xcons_status = xcons_chkq();
		
			switch (xcons_status) {
			case XCONS_CLOSED:
				c = getc(&tp->t_outq);
				ws_dochar(wsp, c & 0xff);
				break;
		    
			case XCONS_BLOCKED:
				goto out;
				
			case XCONS_OK:
				c = getc(&tp->t_outq);
				xconsrint(c);
				break;
			}
		}
		else {
			c = getc(&tp->t_outq);
		}
		splx(s);
	}

	/*
 	 * Position the cursor to the next character location.
 	 */
	if (!(wi->dev_in_use & GRAPHIC_DEV))
	    ws_set_cursor_position
		(wsp->cf->cc, wsp, 
			wsp->sp->col * wsp->sp->f_width, 
			wsp->sp->row * wsp->sp->f_height);

	/*
 	 * If there are sleepers, and output has drained below low
 	 * water mark, wake up the sleepers.
 	 */
        tp->t_state &= ~TS_BUSY;
out:
	s = spltty();
        if (tp->t_outq.c_cc<=TTLOWAT(tp)) {
	    if (tp->t_state & TS_ASLEEP) {
		tp->t_state &= ~TS_ASLEEP;
		wakeup((caddr_t)&tp->t_outq);
	    }
	    select_wakeup(&tp->t_selq);
	}
	splx(s);
}

/*
 * Routine to stop output on the graphic device, e.g. for ^S/^Q
 * or output flush.
 */

/*ARGSUSED*/
wsstop(tp, flag)
	register struct tty *tp;
{
	register int s;

	/*
         * Block interrupts while modifying the state.
 	 */
	s = spltty();
	if (tp->t_state & TS_BUSY)
	    if ((tp->t_state&TS_TTSTOP)==0)
		tp->t_state |= TS_FLUSH;
	    else
		tp->t_state &= ~TS_BUSY;
	splx(s);
}

/*
 * Output a character to the screen
 */

ws_blitc(wsp, c)
register ws_screens *wsp;
register u_char c;
{
	register ws_screen_descriptor *wsdp = wsp->sp;
	register ws_info *wi = &ws_softc[0];
	register ws_devices *wsd = &devices[wi->ws.console_keyboard];
	register int i;

#ifdef DEBUG_PRINT_CHAR_DISABLED
DPRINTF("ws_blitc: entry\n");
#endif /* DEBUG_PRINT_CHAR */

	c &= 0xff;

	if (generic_console_active) {
	    generic_cons_blitc(c);
	    return(0);
	}

	switch (c) 
	{
	    case '\t':				/* tab		*/
		for (i = 8 - (wsdp->col & 0x7) ; i > 0 ; i--)
			ws_blitc(wsp, K_SPACE);
		return(0);
	    case '\r':				/* return	*/
		wsdp->col = 0;
		return(0);
	    case '\b':				/* backspace	*/
		if (--wsdp->col < 0)
			wsdp->col = 0;
		return(0);
	    case '\n':				/* linefeed	*/
		if (wsdp->row+1 >= wsdp->max_row) {
			if (wi->mouse_on)	
                           wsdp->row = 0;
                        else {    
                           wsdp->row -= (*(wsp->f->scroll_screen))
                                        (wsp->f->sc, wsp->sp);
		        }
		}
		else
			wsdp->row++;
		    /*
 	 	     * Position the cursor to the next character location.
 	 	     */
	    	if (!(wi->dev_in_use & GRAPHIC_DEV))
			ws_set_cursor_position (wsp->cf->cc, wsp, 
						wsdp->col * wsdp->f_width,
						wsdp->row * wsdp->f_height);
		return(0);
	    case '\007':			/* bell		*/
		(*(wsd->p.kp->ring_bell))(wsd->p.kp->kc, wsd->p.kp);
		return(0);
	    default:
		/*
		 * If the next character will wrap around then 
		 * increment row counter or scroll screen.
		 */
		if (wsdp->col >= wsdp->max_col) {
			wsdp->col = 0 ;
			if (wsdp->row+1 >= wsdp->max_row) {
				if (wi->mouse_on)	
				    wsdp->row = 0;
                                else {
				    wsdp->row -= (*(wsp->f->scroll_screen))
                                                (wsp->f->sc, wsp->sp);
				}
			}
			else
			    wsdp->row++;
		}
                (*(wsp->f->blitc)) (wsp->f->sc, wsp->sp,
				    wsdp->row, wsdp->col, c);
                wsdp->col += 1;
		return(0);
	}
}

/*
 * set the cursor to the specified position.  Sets the system's current
 * belief about where the cursor is, constrained by the size of the
 * screen.   Calls out to the appropriate hardware routine; old cursor
 * position is still in screen structure at time of call.  The driver
 * only knows about hot spot coordinates.
 */
int
ws_set_cursor_position(closure, wsp, x, y)
	register caddr_t closure;
	register ws_screens *wsp;
	register int x, y;
{
	register ws_screen_descriptor *screen = wsp->sp;
	register int value;

	if (y < 0) 		y = 0;
	if (y >= screen->height)y = screen->height - 1;
	if (x < 0)		x = 0;
	if (x >= screen->width)	x = screen->width - 1;
	value = (*(wsp->cf->set_cursor_position)) (closure, screen, x, y);
	screen->x = x;
	screen->y = y;
	return(value);
}

ws_cursor_position *
ws_get_cursor_position()
{
	return(&devices[ws_softc[0].ws.console_pointer].p.pp->position);
	/* whew! */
}

/*
 * Register a device with the generic driver.
 */

int ws_define_device(device_type, dp, event_size)
	register int device_type;
	register ws_device *dp;
	register int event_size;
{
	register ws_info *wi = &ws_softc[0];
	register ws_devices *wdp;
	register int i = wi->ws.num_devices_exist;
	register int j;
	int found = 0;

	/* run through and see if device type already defined */
	for (j=0; j<i; j++) {
	  if (devices[j].device_type == device_type) {
	    i = j;
	    found = 1;
	    break;
	  }
        }
	/* if new device type, increment num_devices_exist */
	if (!found) {
	  if (wi->ws.num_devices_exist >= NUMINPUTDEVICES)
	    return -1;
	  else
	    wi->ws.num_devices_exist++;
	}

	wdp = &devices[i];
	wdp->device_type = device_type;
	wdp->p.dp = dp;
	if (event_size > wi->max_event_size)
	    wi->max_event_size = event_size;
	if(wdp->p.dp->axis_count > wi->max_axis_count) 
	    wi->max_axis_count = wdp->p.dp->axis_count;
	return i;
}

/*
 * Register a screen with the generic driver.
 */
int ws_define_screen(sp, vp, dp, f, cmf, cf)
	ws_screen_descriptor *sp;
	ws_visual_descriptor *vp;
	ws_depth_descriptor *dp;
	ws_screen_functions *f;
	ws_color_map_functions *cmf;
	ws_cursor_functions *cf;
{
	register ws_info *wi = &ws_softc[0];
	register ws_screens *wsp;
	register int i = wi->ws.num_screens_exist;

	if (i >= NUMSCREENS) return (-1);
	sp->screen = i;
	dp->screen = i;
	wsp = &screens[i];
	wsp->sp = sp;
	wsp->vp = vp;
	wsp->dp = dp;
	wsp->f  = f;
	wsp->cmf= cmf;
	wsp->cf = cf;
	wsp->adj_screens.top   = -1;
	wsp->adj_screens.bottom = -1;
	wsp->adj_screens.left = -1;
	wsp->adj_screens.right = -1;
	if (wi->ws.num_screens_exist > 0) {
		wsp->adj_screens.left   = i - 1;
		screens[i - 1].adj_screens.right = i;
	}
	wi->ws.num_screens_exist += 1;

	/*
	 * If the first controller is being done, make sure that
	 * we call the routine which will initialize the mouse
	 * and keyboard. It may also initialize the particular graphics
	 * option as well, when graphics console has been selected.
	 *
	 * In the case of alternate console selection, only
	 * the mouse/keyboard initialization will be done;
	 * we assume that the normal controller "attach" routine
	 * has initialized the graphics option in the appropriate way.
	 */
	if (++ws_num_controllers == 1)
		ws_cons_init();
	
	return (i);
}

ws_update_event_queue_time(wi)
	register ws_info *wi;
{
	timeout (ws_update_event_queue_time, wi, TIMEOUT_FREQ);
        if ( wi->queue != NULL ) {
	   wi->queue->time = TOY;
        }
}

/*
 * take a keyboard event and put it in the event queue.
 */
ws_enter_keyboard_event(queue, ch, p, type)
	register ws_event_queue *queue;
	int ch;
	ws_pointer *p;
        int type;
{
	register ws_info *wi = &ws_softc[0];
	register ws_event *ev = wi->events;
	int i;

	if ((i = EVROUND(queue, queue->tail + 1)) == queue->head)
		return;

#ifdef DEBUG_PRINT_KB_EVENTS
DPRINTF("ws_enter_keyboard_event: 0x%x %s\n", ch,
	(type==BUTTON_DOWN_TYPE)?"DN":"UP");
#endif /* DEBUG_PRINT_KB_EVENTS */

	ev = (ws_event *) ( ((caddr_t) wi->events)
			  + queue->tail * queue->event_size );
	ev->time = queue->time = TOY;
	ev->screen = p->position.screen;
	ev->device = wi->ws.console_keyboard;
	ev->device_type = KEYBOARD_DEVICE;
	ev->type = type;
	ev->e.key.key = ch & 0xff;
	ev->e.key.pad = 0;
	ev->e.key.x   = p->position.x;
	ev->e.key.y   = p->position.y;
	queue->tail = i;
}

/*
 * enters an event into the queue, and timestamps it, otherwise does not
 * affect the event.
 */
ws_enter_event(event, size)
	ws_event *event;
	int size;
{
	register ws_info *wi = &ws_softc[0];
	register ws_event *ev = wi->events;
	register ws_event_queue *queue = wi->queue;
       	int i, s;

	if ((i = EVROUND(queue, queue->tail + 1)) == queue->head)
		return(ENOMEM);

	/* protect queue from mouse/keyboard interrupts */
	s = spltty();

	ev = (ws_event *)( (caddr_t) (wi->events)
			 + queue->tail * queue->event_size);
	bcopy((char *) event, (char *) ev, queue->event_size);
	ev->time = queue->time = TOY;
	queue->tail = i;
#ifdef FIXME
/* don't do any of this yet; we need to coordinate this with the X group... */
        if (event->type == MOTION_TYPE) {
	    ws_motion_history *mhp = wi->motion;
	    register short *ap;

	    mhp = (ws_motion_history *)(((caddr_t) mhp) + 
				wi->mb->next * wi->mb->entry_size);
	    mhp->time = ev->time;
	    mhp->device = event->device;
	    mhp->screen = event->screen;
	    ap = (short *) ((caddr_t)event + queue->event_size);
	    for(i = 0; i < devices[event->device].p.dp->axis_count; i++) 
		mhp->axis[i] = *ap++;
	    if (++wi->mb->next >= wi->mb->size) wi->mb->next = 0;
	}
#endif /* FIXME */
	splx(s);
	return(0);
}

/*
 * routine to process events which (potentially) move between screens
 */
ws_screen_descriptor *
ws_do_edge_work (wi, sp, p, edge)
	register ws_info *wi;
	register ws_screen_descriptor *sp;
	register ws_pointer *p;
	register int edge;
{
	register int ns = wi->ws.num_screens_exist;
	register int current_screen = p->position.screen;
	register ws_screens *wsp = &screens[current_screen];
	register int next_screen = current_screen;
	if (ns > 1) {
		if (edge & TOP_EDGE) {
			next_screen = wsp->adj_screens.top;
			if (next_screen != -1) {
				sp = screens[next_screen].sp;
				p->position.y = p->position.y + sp->height;
			}
		}
		else if (edge & BOTTOM_EDGE) {
			next_screen = wsp->adj_screens.bottom;
			if (next_screen != -1) {
				p->position.y = p->position.y - sp->height;
				sp = screens[next_screen].sp;
			}
		}
		else if (edge & LEFT_EDGE) {
			next_screen = wsp->adj_screens.left;
			if (next_screen != -1) {
				sp = screens[next_screen].sp;
				p->position.x = p->position.x + sp->width;
			}
		}
		else if (edge & RIGHT_EDGE) {
			next_screen = wsp->adj_screens.right;
			if (next_screen != -1) {
				p->position.x = p->position.x - sp->width;
				sp = screens[next_screen].sp;
			}
		}
	}
	if (next_screen != -1) p->position.screen = next_screen;
	if (p->position.x < 0)		p->position.x = 0;
	if (p->position.x >= sp->width)	p->position.x = sp->width - 1;
	if (p->position.y < 0)		p->position.y = 0;
	if (p->position.y >= sp->height)p->position.y = sp->height - 1;
	return sp;
}

/*
 * Routine to process a mouse report after the device-dependent code
 * has "standardized" the ws_pointer_report "new_rep".
 */
int
ws_process_mouse_report(wi, queue, wsp, p, last_rep, new_rep, open)
	register ws_info *wi;
	register ws_event_queue *queue;
	ws_screens *wsp;
	register ws_pointer *p;
	register ws_pointer_report *new_rep;
	ws_pointer_report *last_rep;
	int open;
{
        int cs = p->position.screen;
        register ws_screen_descriptor *sp = screens[cs].sp;
	ws_motion_history *mhp;
	unsigned int millis = TOY;
	register int i;
	register ws_event *ev;
	register char temp;
	register int j;
	register int edge = 0;

        wsp = &screens[cs];             /* ? do we trust our args??? NOT */

#ifdef DEBUG_PRINT_MS_EVENTS
DPRINTF("ws_process_mouse_report: 0x%x 0x%x 0x%x\n",
	new_rep->state, new_rep->dx, new_rep->dy);
#endif /* DEBUG_PRINT_MS_EVENTS */

	/*
	 * see if mouse position has changed
	 */
	if (new_rep->dx != 0 || new_rep->dy != 0) {
	    /*
 	     * Check to see if we have to accelerate the mouse
	     * I'm not willing to make this computation floating point...
 	     *
 	     */
	    if (p->pr.denominator > 0) {
		if (new_rep->dx >= p->pr.threshold)
		    new_rep->dx +=
			((new_rep->dx - p->pr.threshold) * p->pr.numerator) /
			  p->pr.denominator;
		if (new_rep->dy >= p->pr.threshold)
		    new_rep->dy +=
			((new_rep->dy - p->pr.threshold) * p->pr.numerator) /
			  p->pr.denominator;
	    }

	    /*
 	     * update mouse position
 	     */
	    if (new_rep->state & WSPR_X_SIGN) {
		p->position.x -= new_rep->dx;
		if (p->position.x < 0) 		edge |= LEFT_EDGE;
	    }
	    else {
		p->position.x += new_rep->dx;
		if (p->position.x >= sp->width)	edge |= RIGHT_EDGE;
	    }
	    if (new_rep->state & WSPR_Y_SIGN) {
		p->position.y -= new_rep->dy;
		if (p->position.y < 0) 		edge |= TOP_EDGE;
	    }
	    else {
		p->position.y += new_rep->dy;
		if (p->position.y >= sp->height) edge |= BOTTOM_EDGE;
	    }
	    if (p->constrain.enable && p->constrain.screen == cs) {
		if (p->position.y > p->constrain.box.bottom)
			p->position.y = p->constrain.box.bottom;
		else if (p->position.y < p->constrain.box.top)
			p->position.y = p->constrain.box.top;
		if (p->position.x > p->constrain.box.right)
			p->position.x = p->constrain.box.right;
		else if (p->position.x < p->constrain.box.left)
			p->position.x = p->constrain.box.left;
		edge = 0; /* reset edge mask */
	    }
	    if (edge) {
		sp = ws_do_edge_work (wi, sp, p, edge);
                if (cs != p->position.screen) {
                    /*
                     * If we moved onto a new screen, continue to track the
                     * pointer with the old pattern.  At some point in the
		     * future, the server will catch up and warp the pointer
		     * to where it thinks it should be.  Note that this will
		     * interfere with multi-pointer configurations!  Have to
		     * revisit this later...
                     */
                    (*(wsp->cf->cursor_on_off))(wsp->cf->cc, CURSOR_OFF);
                    cs = p->position.screen;
                    wsp = &screens[cs];
                    p->cursor.screen = cs;
                    (*(wsp->cf->load_cursor))(wsp->cf->cc, wsp->sp,
					      &(p->cursor), NOSYNC);
                    (*(wsp->cf->cursor_on_off))(wsp->cf->cc, CURSOR_ON);
                }
                if (p->position.y >= sp->height)
                    p->position.y = sp->height - 1;
            }

	    if (open)
                ws_set_cursor_position(wsp->cf->cc, wsp, p->position.x,
				       p->position.y);
	    mhp = wi->motion;
	    mhp = (ws_motion_history *)(((caddr_t)mhp) + 
				wi->mb->next * wi->mb->entry_size);

	    mhp->time = queue->time = millis;
	    mhp->device = wi->ws.console_pointer;
	    mhp->screen = cs;

	    /*
	     **  identify the current active screen... -bg
	     */
	    wi->mouse_screen = cs;

	    mhp->axis[0] = p->position.x;
	    mhp->axis[1] = p->position.y;
	    if (++wi->mb->next >= wi->mb->size)
		    wi->mb->next = 0;

	    if (p->suppress.enable &&
		p->position.y < p->suppress.box.bottom &&
		p->position.y >=  p->suppress.box.top &&
		p->position.x < p->suppress.box.right &&
		p->position.x >=  p->suppress.box.left)	 goto mbuttons;
	    p->suppress.enable = 0;	/* trash box */

	    if (EVROUND(queue, queue->tail + 1) == queue->head)
			goto mbuttons;

	    i = EVROUND(queue, queue->tail - 1);
	    if ((queue->tail != queue->head) && (i != queue->head)) {
		ev = (ws_event *) ( ((caddr_t) wi->events)
				  + queue->event_size * i);
	        if (ev->type == MOTION_TYPE
			&& ev->device_type ==
			devices[wi->ws.console_pointer].device_type) {
		    ev->screen = cs;
		    ev->time = queue->time = millis;
		    ev->e.pointer.x = p->position.x;
		    ev->e.pointer.y = p->position.y;
		    ev->device = wi->ws.console_pointer;
		    goto mbuttons;
		}
	    }

	    /*
 	     * Put event into queue and do select
 	     */
	    ev = (ws_event *)
		    (((caddr_t)wi->events) + queue->tail * queue->event_size);
	    ev->type = MOTION_TYPE;
	    ev->screen = cs;
	    ev->time = queue->time = millis;
	    ev->device_type = devices[wi->ws.console_pointer].device_type;
	    ev->e.pointer.x = p->position.x;
	    ev->e.pointer.y = p->position.y;
	    ev->e.pointer.buttons = new_rep->state & 0x07;
            queue->tail = EVROUND(queue, queue->tail + 1);
	}

	/*
	 * See if mouse buttons have changed.
	 */

 mbuttons:

	wi->new_switch = new_rep->state & 0x07;
	wi->old_switch = last_rep->state & 0x07;

	temp = wi->old_switch ^ wi->new_switch;
	if (temp) {
	    for (j = 1; j < 8; j <<= 1)  {/* check each button */
		if (!(j & temp))  /* did this button change? */
		    continue;

		/*
 		 * Check for room in the queue
 		 */
                if ((i = EVROUND(queue, queue->tail + 1)) == queue->head) 
		    return(0);

		/* put event into queue and do select */
		ev = (ws_event *)
			( ((caddr_t)wi->events)
			+ queue->tail * queue->event_size);

		switch (j) {
		case WSPR_RIGHT_BUTTON:
			ev->e.button.button = EVENT_RIGHT_BUTTON;
			break;
			
		case WSPR_MIDDLE_BUTTON:
			ev->e.button.button = EVENT_MIDDLE_BUTTON;
			break;

		case WSPR_LEFT_BUTTON:
			ev->e.button.button = EVENT_LEFT_BUTTON;
			break;
		}

		if (wi->new_switch & j)
			ev->type = BUTTON_DOWN_TYPE;
		else
			ev->type = BUTTON_UP_TYPE;
		ev->device_type = devices[wi->ws.console_pointer].device_type;
		ev->time = queue->time = millis;
		ev->e.button.x = p->position.x;
		ev->e.button.y = p->position.y;
		ev->screen = cs;
	        queue->tail = i;
	    }
	    /* update the last report */
#ifdef FIXME
	    *last_rep = current_rep;
#else
	    *last_rep = *new_rep;
#endif /* FIXME */
	    p->mswitches = wi->new_switch;
	} /* Pick up mouse input */
}

/*
 * Direct kernel console output to display destination
 */
wsputc(c)
	register char c;
{
	register ws_info *wi = &ws_softc[0];
	register ws_screens *wsp = &screens[wi->ws.console_screen];
	register ws_devices *wsk = &devices[wi->ws.console_keyboard];
	register ws_devices *wsm = &devices[wi->ws.console_pointer];
	register ws_screen_descriptor *wsdp = wsp->sp;
	register int xcons_status;
	static int ws_panic_init = 0;

#ifdef DEBUG_PRINT_CHAR_DISABLED
DPRINTF("wsputc: entry\n");
#endif /* DEBUG_PRINT_CHAR */

	if ((printstate & PANICPRINT) && (ws_panic_init == 0)) {
	        ws_panic_init = 1;
	        wi->mouse_on = 0;
		wsdp->row = wsdp->max_row - 1;
		(*(wsp->f->init_screen))(wsp->f->sc, wsp->sp);
		(*(wsp->cmf->init_color_map))(wsp->cmf->cmc);
		(*(wsp->cmf->video_on))(wsp->cmf->cmc);
		(*(wsk->p.kp->reset_keyboard))(wsk->p.kp->kc);
		if (wsm->p.pp->disable_pointer)
			(*(wsm->p.pp->disable_pointer))(wsm->p.pp->pc);
	}

	xcons_status = xcons_chkq();

	switch (xcons_status) {
	  case XCONS_CLOSED:
	    /* real console output */
	    ws_blitc(wsp, c & 0xff); 
	    break;
	    
	  case XCONS_BLOCKED:
	    break;
	    
	  case XCONS_OK:
	    /* not panic'ing, routing to alternate console */
	    xconsrint(c);
	    break;
	}
}

/*
 * Graphice device interrupt routine. 
 */
wskint(ch)
register int ch;
{
	register ws_info *wi = &ws_softc[0];
	register ws_devices *wsd = &devices[wi->ws.console_keyboard];
	ws_pointer *p = (ws_pointer *)devices[wi->ws.console_pointer].p.pp;
	register ws_screens *wsp = &screens[p->position.screen];
	ws_pointer_report *new_rep;
	struct tty *tp;
	register int unit;
	register u_short c;
	register int i, j;
	u_short data;
	ws_keyboard_state *kbd	= (ws_keyboard_state *)wsd->p.kp->kc;

	/*
 	 * Mouse state info
 	 */
	unit = (ch >> 8) & 3;
	new_rep = &current_rep;
	tp = &slu.slu_tty[unit];

	/*
 	 * If graphic device is turned on
 	 */
   	if (wi->mouse_on == 1) 	{
	    if (wi->queue == NULL) return;
  	    /*
 	     * Pick up keyboard input (if any)
 	     */
	    if (unit == 0)
                (*wsd->p.kp->process_keyboard_event)(kbd, wi->queue, ch, p);
	    /*
 	     * Pick up the mouse input (if any)
	     * NOTE: entire "report" built by interrupt handler
	     *       has been passed globally in "current_rep",
	     *	     which is pointed to by "new_rep".
 	     */
	    else if ((unit == 1) && (pointer_id == MOUSE_ID)) {
/* FIXME - should "*p->process_mouse_event" do only "standardizing", and */
/* FIXME - then we'll do a call FROM HERE to "ws_process_mouse_report" ??? */
		(*p->process_mouse_event)(wi, wi->queue, wsp,  p, 
				  &wi->last_rep, new_rep, 1);
#ifdef DEBUG_PRINT_MOUSE
DPRINTF("wskint: gfx mode: MOUSE data\n");
#endif /* DEBUG_PRINT_MOUSE */
	    }
	    else if ((unit == 1) && (pointer_id == TABLET_ID)) {
		(*p->process_tablet_event)(wi, wi->queue, p,
				      &wi->last_rep, new_rep, 
				      p->position.screen, 1);
#ifdef DEBUG_PRINT_MOUSE
DPRINTF("wskint: gfx mode: TABLET data\n");
#endif /* DEBUG_PRINT_MOUSE */
	    }

	    /*
 	     * If we have proc waiting, and event has happened, wake him up
 	     */
	    if (!queue_empty(&wi->rsel) &&
		(wi->queue->head != wi->queue->tail))
	    {
		select_wakeup (&wi->rsel);
	    } 
#if NOTDEF
	    else if (wi->queue->head != wi->queue->tail) ttwakeup(tp);
#endif
/* is this what he really means??? JH XXX
 *   or does he mean
 *
 *	    else if (tp->r_rsel && wi->queue->head != wi->queue->tail)
 *	    	ttwakeup(tp);
 *
 * if we want other serial lines to come through here, then we need the
 * ttwakeup call.  And if we do that we also needed to call the line disciple
 * input routine from here. We don't so I'll assume not. XXX
 */
	}
	else  { /* wi->mouse_on != 1 */
	    /*
	     * If the graphic device is not turned on, this is console input.
	     * Process a character from the keyboard, throw mouse input away.
	     */
	    if (unit == 0) {
	        data = ch & 0xff;
	        (*(wsd->p.kp->process_keyboard_char))(kbd, data);
	    }
#ifdef DEBUG_PRINT_MOUSE
	    else
		    DPRINTF("wskint: tty mode: MOUSE data\n");
#endif /* DEBUG_PRINT_MOUSE */
        }

	return;
} 

int ws_get_width(screen)
    int screen;
{
  	register ws_info *wi = &ws_softc[0];
	register ws_screens *wsp;
	if(screen >= wi->ws.num_screens_exist)
		return NULL;
	wsp = &screens[screen]; 
	return (wsp->sp->width);
}

int ws_get_height(screen)
    int screen;
{
  	register ws_info *wi = &ws_softc[0];
	register ws_screens *wsp;
	if(screen >= wi->ws.num_screens_exist)
		return NULL;
	wsp = &screens[screen]; 
	return (wsp->sp->height);
}

char *ws_get_module_name(screen)
    int screen;
{
  	register ws_info *wi = &ws_softc[0];
	register ws_screens *wsp;
	if(screen >= wi->ws.num_screens_exist)
		return NULL;
	wsp = &screens[screen]; 
	return (&wsp->sp->moduleID[0]);
}

char *ws_get_keyboard_name()
{
	return (keyboard.name);
}

char *ws_get_pointer_name()
{
	return (mouse.name);
}

ws_wakeup_any_pending()
{
  	register ws_info *wi = &ws_softc[0];
	/*
 	 * If we have proc waiting, and event has happened, wake him up
 	 */
	if (!queue_empty(&wi->rsel) && (wi->queue->head != wi->queue->tail)) {
		select_wakeup (&wi->rsel);
	}
}

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
extern	int (*vs_gdioctl)();
extern  int (*vs_gdmmap)();
extern	int (*vs_gdstop)();
extern  int (*vs_gdkint)();
extern char *(*ws_graphics_name_proc)();
extern char *(*ws_keyboard_name_proc)();
extern char *(*ws_pointer_name_proc)();
extern  int (*ws_graphics_get_width_proc)();
extern  int (*ws_graphics_get_height_proc)();

extern struct tty xcons_tty[];	/* xcons structure */

/*
 * Routine to do the board specific setup.
 *
 * This routine may get called during console initialization IFF we have
 *  a graphics console selected. The machine-specific console driver will
 *  make a call to the graphics controller "cons_init" routine, which will
 *  make a call to "ws_define_screen" when it's satisfied.
 *
 * It may also get called during the normal probing for devices when a
 *  graphics controller is found and its "attach" routine calls on the
 *  "ws_define_screen" routine to register it.
 *
 * NOTE: should *ONLY* be called from ws_define_screen() now; see that
 *       location for details. Suffice it to say, it will only call here
 *	 for the very *FIRST* screen defined.
 */

ws_cons_init()
{
	register ws_info *wi;
	register ws_screens *wsp;
	register ws_screen_descriptor *screen;
	register ws_devices *wsk, *wsm;
	register struct tty *tp;

	(void) ws_define_device(KEYBOARD_DEVICE, &keyboard, sizeof(ws_event));
	(void) ws_define_device(MOUSE_DEVICE, &mouse, sizeof(ws_event));

	wi = &ws_softc[0];
	wsk = &devices[wi->ws.console_keyboard];
	wsm = &devices[wi->ws.console_pointer];
	tp = &xcons_tty[XCONSDEV];

	/* only do this setup if graphics console selected */
	if (consDev == GRAPHIC_DEV) 
	{
		wsp = &screens[wi->ws.console_screen];
		screen = wsp->sp;

		/*
		 * Home the cursor.
		 * We want an LSI terminal emulation.  We want the graphics
		 * terminal to scroll from the bottom. So start at the bottom
		 */
		screen->row = screen->max_row - 1;
		screen->col = 0;
		
#ifdef VT100_EMUL
		/* init top and bottom margins to entire screen */
		top_marg = 0;
		bot_marg = screen->max_row - 1;
#endif /* VT100_EMUL */

		/* initialize the hardware */
		(*(wsp->f->init_screen))(wsp->f->sc, screen);
		(*(wsp->cmf->init_color_map))(wsp->cmf->cmc);
		(*(wsp->cf->load_cursor)) (wsp->cf->cc, screen,
					   &default_cursor, NOSYNC);
		(*(wsp->cf->cursor_on_off))(wsp->cf->cc, CURSOR_ON);
		
		/* and take over the console */
		/* FIXME - these should be in scc_cons_init */
		v_consputc = wsputc;
		v_consgetc = ((ws_keyboard_state *)wsk->p.kp->kc)->kb_getc;
	}

	(*(wsk->p.kp->reset_keyboard))(wsk->p.kp->kc);
	pointer_id = (*(wsm->p.pp->init_pointer))(wsm->p.pp->pc);
        queue_init(&wi->rsel); /* moved here from pointer init routines */
	queue_init(&tp->t_selq);

	vs_gdopen = wsopen;
	vs_gdkint = wskint;
	vs_gdclose = wsclose;
	vs_gdselect = wsselect;
	vs_gdioctl = wsioctl;
        vs_gdmmap = wsmmap;
	vs_gdstop = wsstop;

	ws_graphics_name_proc = ws_get_module_name;
	ws_keyboard_name_proc = ws_get_keyboard_name;
	ws_pointer_name_proc = ws_get_pointer_name;
	ws_graphics_get_width_proc = ws_get_width;
	ws_graphics_get_height_proc = ws_get_height;

	return (1);
}

caddr_t ws_where_option(name)
    caddr_t name;
{
    switch (cpu) {
     case DS_3100:
        return (caddr_t)0;

     default:
        return ((caddr_t) tc_where_option(name));
    }
}

wsread() { return EIO; }
wswrite() { return EIO; }

/*
 * ws_vm_callback
 */
static int
vm_callback( func, va )
int func;
vm_offset_t va;
{
    int i, status = 0;
    vm_map_t map;
    ws_info *wi = &ws_softc[0];

    for ( i = 0; i < wi->ws.num_screens_exist; i++ ) {
	if ( wi->server_proc_screens & (1<<i) &&
	     wi->server_vm_callback[i] != (int (*)()) NULL ) {
	    status = (*wi->server_vm_callback[i])( func, va,
						   wi->server_proc_data[i] );
	    if ( status != 0 ) {
		return (status);
	    }
	}
    }
    return (status);
}

/*
 * ws_register_vm_callback
 */
int
ws_register_vm_callback( screen, func, data )
int screen;
int (*func)();
caddr_t data;
{
    int s;
    vm_map_t map;
    ws_info *wi = &ws_softc[0];

    if ( screen < 0 || screen >= wi->ws.num_screens_exist ) {
        return (-1);
    }

    s = splvm();

    if ( wi->server_proc_ref_count && u.u_procp != wi->p_server_proc ) {
        /*
         * We can only handle one process per workstation device.
         * Assume existing process is dead or at least obsolete.  Clean up
         * structures and prepare for a new process.
         */
        if ( wi->p_server_proc->task != NULL &&
             wi->p_server_proc->task->map != NULL ) {
            vm_map_t map = wi->p_server_proc->task->map;

            pmap_clear_coproc_tbi( map->vm_pmap );
        }
        wi->server_proc_ref_count = 0;
        wi->server_proc_screens = 0;
    }

    if (!(wi->server_proc_screens & (1<<screen))) {
        if ( wi->server_proc_ref_count == 0 ) {
            wi->p_server_proc = u.u_procp;
            map = u.u_procp->task->map;
            pmap_set_coproc_tbi( map->vm_pmap, vm_callback );
        }
        wi->server_proc_screens |= (1<<screen);
        wi->server_proc_ref_count++;
        wi->server_vm_callback[screen] = func;
        wi->server_proc_data[screen] = data;
    }
    splx(s);

    return (0);
}


/*
 * ws_unregister_vm_callback( screen )
 */
int ws_unregister_vm_callback( screen )
int screen;
{
    int s;
    ws_info *wi = &ws_softc[0];

    if ( screen < 0 || screen >= wi->ws.num_screens_exist ) {
	return (-1);
    }

    s = splvm();
    if (wi->server_proc_screens & (1<<screen)) {
	wi->server_proc_screens &= ~(1<<screen);
	if ( --wi->server_proc_ref_count == 0 ) {
	    vm_map_t map;
	    if ( wi->p_server_proc->task != NULL &&
		 wi->p_server_proc->task->map != (vm_map_t) NULL ) {
		map = wi->p_server_proc->task->map;
		pmap_clear_coproc_tbi( map->vm_pmap );
		wi->server_vm_callback[screen] = (int (*)()) NULL;
	    }
	    else {
		wi->server_proc_ref_count = 0;
		wi->p_server_proc = NULL;
	    }
	}
    }
    splx(s);

    return (0);
}

/************************************************
 *
 * Start of generic console code
 *
 ************************************************/

caddr_t generic_cons_init_closure();
int generic_cons_null();
int generic_cons_blitc();

ws_screen_descriptor generic_screen = 
        {
	0, 
        MONITOR_VR297,
	"PMAX-CFB",
	10240, 10240,		/* width, height */
	0,			/* depth */
	1,			/* number of depths present		*/
	1,			/* number of visual types of screen 	*/
	0, 0,			/* current pointer position 		*/
	0, 0,			/* current text position		*/
	1024, 1024,		/* maximum row, col text position 	*/
	8, 15,			/* console font width and height	*/
	16, 16,			/* maximal size cursor for screen	*/
	1, 1,			/* min, max of visual types		*/
	};

ws_depth_descriptor generic_depth = 
	{			/* depth descriptor of root window */
	0, 0, 			/* which screen and depth		*/
	10240, 10240,		/* frame buffer size in pixels		*/
	8,			/* returns the depth (out)		*/
	8,			/* stride of pixel (out)		*/
	32,			/* scan line pad			*/
    	(caddr_t) 0,		/* bitmap starts at beginning */
	0,			/* only filled in when mapped		*/
	(caddr_t)0, 		/*plane mask offset */
	0,			/* only filled in when mapped		*/
	};

ws_visual_descriptor generic_visual = 
	{			/* visual descriptor */
	0,			/* which screen (in)			*/
    	0,			/* which visual of screen (in) 		*/
	PseudoColor,		/* class of visual 			*/
	8,			/* number of bits per pixel		*/
	0, 0, 0,		/* zero since pseudo; mask of subfields */
	8,			/* bits per RGB 			*/
	256,			/* color map entries */
    	};

ws_cursor_functions generic_cf =
	{
	generic_cons_init_closure,
	generic_cons_null,
	generic_cons_null,
	generic_cons_null,
	generic_cons_null,
	(caddr_t)0,
        };

ws_color_map_functions generic_cmf = 
	{
	generic_cons_init_closure,
	generic_cons_null,
	generic_cons_null,
        NULL,
	generic_cons_null,
	generic_cons_null,
	(caddr_t)0,
	};

ws_screen_functions generic_sf =
	{
	generic_cons_init_closure,
	generic_cons_null,
	generic_cons_null,
	generic_cons_null,
	generic_cons_blitc,
	generic_cons_null,
	NULL,			/* ioctl optional */
	NULL,			/* close optional */
	(caddr_t)0,
	};


install_generic_console()
{
    generic_console_active = 1;

    ws_define_screen(&generic_screen, &generic_visual, &generic_depth,
		     &generic_sf, &generic_cmf, &generic_cf);

/* NOTE: ws_cons_init() now done by ws_define_screen() */

    return(1); /* tell cons_init we were successful */
}

caddr_t
generic_cons_init_closure()
{
    return ((caddr_t)NULL);
}

generic_cons_null()
{
}

generic_cons_blitc(c)
int c;
{
#ifdef __alpha
    char str[3];

    str[0] = c & 0xff;
    str[1] = 0;
    if (c == '\n') {
	str[1] = '\r';
	str[2] = 0;
    }
	
    prom_puts(str);
#endif
}

/*
 * Pre-process output characters for escape sequences, to do ANSI emulation.
 */
ws_dochar(wsp, c)
register ws_screens *wsp;
register u_char c;
{
	register int i;

	/*
	 * make sure all the functions for the terminal
	 * emulation are available; if not, call ws_blitc and return...
	 */
	if (NO_EMUL_FUNCS)
	{
		ws_blitc(wsp, c);
		return;
	}
	
	/*
	 * process character, checking for escape sequences
	 */
	if (c == (K_ESC)) {
		if (esc_spt == esc_seq) {
			*(esc_spt++)=(K_ESC);
			*(esc_spt) = '\0';
#ifdef DEBUG_PRINT_CHAR
DPRINTF("ws_dochar: starting ESC seq\n");
#endif /* DEBUG_PRINT_CHAR */
		}
		else {
			ws_putc(wsp, (K_ESC));
			esc_spt = esc_seq;
#ifdef DEBUG_PRINT_ERRORS
DPRINTF("ws_dochar: got ESC but already parsing\n");
#endif /* DEBUG_PRINT_ERRORS */
		}
	}
	else if (c) {
		if (esc_spt - esc_seq) {
#ifdef DEBUG_PRINT_CHAR
DPRINTF("ws_dochar: got non-ESC; adding 0x%x to queue\n");
#endif /* DEBUG_PRINT_CHAR */
			*(esc_spt++) = c;
			*(esc_spt) = '\0';
			ws_parseesc(wsp);
		}
		else {
#ifdef DEBUG_PRINT_CHAR
DPRINTF("ws_dochar: got non-ESC (0x%x) but *not* parsing\n", c);
#endif /* DEBUG_PRINT_CHAR */
			ws_putc(wsp, c);
		}
	}
	else {	/*
		 * c == '\0' so it can't go into esc_seq or else
		 * ws_parseesc() gets confused.  Flush + reset.
		 */
#ifdef DEBUG_PRINT_ERRORS_DISABLED
DPRINTF("ws_dochar: got NULL char, recovering...\n");
#endif /* DEBUG_PRINT_ERRORS */
		for (i = 0; i < (esc_spt - esc_seq); i++)
			ws_putc(wsp, esc_seq[i]);
		ws_putc(wsp, c);
		esc_spt = esc_seq;
	}
}

/*
 *
 * Function ws_putc():
 *
 *	This function simply puts a character on the screen.  It does some
 *	special processing for linefeed, carriage return, backspace, tab,
 *	formfeed, and bell; all others presumed handled by the device dep.
 *	routine, the "charput" entry point in the emulator_functions struct.
 *
 * input	: character to be displayed
 * output	: character is displayed, or some action is taken
 *
 * NOTE: the emulator_functions "charput" routine is used to render a character;
 *	 it should *NEVER* be called to process cursor control or other special
 *	 characters like bell or formfeed, so all of those *MUST* be handled
 *	 in this routine.
 */

int sit_for_0 = 1; /* FIXME ?? */

ws_putc(wsp, ch)
register ws_screens *wsp;
register u_char	ch;
{
        register int i;
        
	if ((!ch) && sit_for_0) /* FIXME ?? */
		return; /* FIXME ?? */

	if (generic_console_active) {
	    generic_cons_blitc(ch);
	    return;
	}

#ifdef DEBUG_PRINT_CHAR
switch (ch) { 
 case '\t':
 case '\r':
 case '\b':
 case '\n':
 case '\007':
 case '\014':
  DPRINTF("ws_putc: control char: 0x%x\n", ch);
  break;
 default:
  DPRINTF("ws_putc: normal char: 0x%x\n", ch);
  break;
}
#endif /* DEBUG_PRINT_CHAR */

	switch (ch) { 
	case '\t':
		i = 8 - (wsp->sp->col & 7); /* FIXME??? */
		while (i-- > 0)
			ws_right(wsp);
		break;
	case '\r':
		ws_cr(wsp);
		break;
	case '\b':
		ws_left(wsp);
		break;
	case '\n':
#ifdef VT100_EMUL
		if (wsp->sp->col >= wsp->sp->max_col) {
#ifdef DEBUG_VT100_IMPDEL
xxputs("<NL> encountered after column 80...\n");
#endif /* DEBUG_VT100_IMPDEL */
			break; /* ignore newline if after col 80 */
		}
#ifdef FIXME
		ws_cr(wsp);
#endif /* FIXME */
#endif /* VT100_EMUL */
		ws_down(wsp);
		break;
	case '\007':
	{
		register ws_info *wi = &ws_softc[0];
		register ws_devices *wsd = &devices[wi->ws.console_keyboard];
		(*(wsd->p.kp->ring_bell))(wsd->p.kp->kc, wsd->p.kp);
		break;
	}
        case '\014':
                ws_cls(wsp);
		ws_home(wsp);
                break;
	default:
#ifdef VT100_EMUL
/* the last ws_right() may have left the cursor just beyond the current line */
/*  so we need to test for this case here, and do what's needed when */
/*  actually trying to write beyond the end of the line... */
		if (wsp->sp->col >= wsp->sp->max_col)
		{
			wsp->sp->col = 0;
			if (wsp->sp->row < (wsp->sp->max_row - 1))
				wsp->sp->row += 1;
			else
				ws_scrollup(wsp);
		}
#endif /* VT100_EMUL */
                CHARPUT(wsp->f->sc, wsp->sp, CUR_POS(wsp->sp), ch);
		ws_right(wsp);
		break;
	}
	return;
}

/*
 * ws_parseesc:
 *
 *	This routine begins the parsing of an escape sequence.  It uses the
 *	escape sequence array and the escape spot pointer to handle 
 *	asynchronous parsing of escape sequences.
 *
 * input	: String of characters prepended by an escape
 * output	: Appropriate actions are taken depending on the string as
 *		  defined by the ansi terminal specification
 *
 */
ws_parseesc(wsp)
register ws_screens *wsp;
{
	register u_char	*escp;

#ifdef DEBUG_PRINT_CHAR
DPRINTF("ws_parseesc: entry\n");
#endif /* DEBUG_PRINT_CHAR */

	escp = esc_seq + 1;		/* point to char following ESC */
	switch(*(escp)) {
	case 'c':
		ws_cls(wsp);
		ws_home(wsp);
		esc_spt = esc_seq;  /* reset spot in ESC sequence */
		break;
	case '[':
		escp++;
		ws_parserest(wsp, escp);
		break;
#ifdef VT100_EMUL
	case 'D': /* cursor down 1 line */
		ws_down(wsp);
#ifdef DEBUG_VT100_IMPDEL
xxputs("<ESC> D encountered...\n");
#endif /* DEBUG_VT100_IMPDEL */
		esc_spt = esc_seq; /* reset */
		break;
	case 'E': /* NEL */
		ws_cr(wsp);
		ws_down(wsp);
#ifdef DEBUG_VT100_IMPDEL
xxputs("<ESC> E encountered...\n");
#endif /* DEBUG_VT100_IMPDEL */
		esc_spt = esc_seq; /* reset */
		break;
	case 'H': /* set tabs this col all rows - *NOT* :-) */
#ifdef DEBUG_VT100_IMPDEL
xxputs("<ESC> H encountered...\n");
#endif /* DEBUG_VT100_IMPDEL */
		esc_spt = esc_seq; /* reset */
		break;
	case 'M': /* cursor up 1 line */
		ws_up(wsp);
#ifdef DEBUG_VT100_IMPDEL
xxputs("<ESC> M encountered...\n");
#endif /* DEBUG_VT100_IMPDEL */
		esc_spt = esc_seq; /* reset */
		break;
	case '7': /* save cursor - *NOT* :-) */
#ifdef DEBUG_VT100_IMPDEL
xxputs("<ESC> 7 encountered...\n");
#endif /* DEBUG_VT100_IMPDEL */
		saved_row = wsp->sp->row;
		saved_col = wsp->sp->col;
		esc_spt = esc_seq; /* reset */
		break;
	case '8': /* restore cursor - *NOT* :-) */
#ifdef DEBUG_VT100_IMPDEL
xxputs("<ESC> 8 encountered...\n");
#endif /* DEBUG_VT100_IMPDEL */
		if (saved_row != -1) { /* cursor info saved? */
			wsp->sp->row = saved_row;
			wsp->sp->col = saved_col;
			saved_row = saved_col = -1;
		}
		else { /* cursor info NOT saved... */
			wsp->sp->row = wsp->sp->col = 0; /* set to HOME */
		}
		esc_spt = esc_seq; /* reset */
		break;
	case '(': /* need to swallow the following char, too... */
		escp++;
		if (!(*escp)) /* if null, not done */
		    break;
		esc_spt = esc_seq; /* reset */
		break;
	case '>': /* turn off application keypad mode - *NOT* :-) */
#ifdef DEBUG_VT100_IMPDEL
xxputs("<ESC> > encountered...\n");
#endif /* DEBUG_VT100_IMPDEL */
		esc_spt = esc_seq; /* reset */
		break;
	case '=': /* turn on application keypad mode - *NOT* :-) */
#ifdef DEBUG_VT100_IMPDEL
xxputs("<ESC> = encountered...\n");
#endif /* DEBUG_VT100_IMPDEL */
		esc_spt = esc_seq; /* reset */
		break;
#endif /* VT100_EMUL */
	case '\0':
		break;			/* not enough info yet	*/
	default:
#ifdef DEBUG_PRINT_ERRORS
DPRINTF("ws_parseesc: default: invalid char 0x%x\n", *escp);
#endif /* DEBUG_PRINT_ERRORS */
                ws_putc(wsp, *escp);
		esc_spt = esc_seq;	/* invalid sequence char, reset */
		break;
	}
	return;
}


/*
 * ws_parserest:
 *
 *	This function will complete the parsing of an escape sequence and
 *	call the appropriate support routine if it matches correctly.  This
 *	function could be greatly improved by using a function jump table, and
 *	removing this bulky switch statement.
 *
 * input	: A string containing an escape sequence whose first 2 chars
 *		  are <ESC> [
 * output	: Appropriate action based on whether the string matches a
 *	 	  sequence acceptable to the ansi terminal specification
 *
 */
ws_parserest(wsp, cp)
register ws_screens *wsp;
register u_char	*cp;
{
	register ws_screen_descriptor *sp = wsp->sp;
	int	number, number1; /* cannot be register!! */

#ifdef VT100_EMUL
	/* some numbers are preceeded by a '?' */
	if (*cp == '?') {
		cp++;
		if (*cp == '\0')
			return; /* not enough yet... */
		cp += ws_atoi(cp, &number);
		if (*cp == '\0')
			return; /* not enough yet... */
		if (*cp == 'l' || *cp == 'h') {
			/* do nothing for now */
#ifdef DEBUG_VT100_IMPDEL
xxputs("<ESC> [ ? # l or h encountered...\n");
#endif /* DEBUG_VT100_IMPDEL */
		}
		else {
			ws_putc(wsp, '?'); /* error, print '?' */
		}
		esc_spt = esc_seq; /* reset, we're done */
		return;
	}
#endif /* VT100_EMUL */

	cp += ws_atoi(cp, &number);
	switch(*cp)
	{
#ifdef VT100_EMUL
	case 'g': /* number == 3, clear all tabs - *NOT* :-) */
#ifdef DEBUG_VT100_IMPDEL
xxputs("<ESC> [ # g encountered...\n");
#endif /* DEBUG_VT100_IMPDEL */
		esc_spt = esc_seq; /* reset */
		break;
#endif /* VT100_EMUL */
	case 'm':
	{
		int new_attr;
#ifdef DEBUG_PRINT_ESC
DPRINTF("ws_parserest: change attribute to %d\n", number);
#endif /* DEBUG_PRINT_ESC */
		switch(number)
		{
		default:
		case DEFAULT:
		case 0:
			new_attr = ATTR_NORMAL;
			break;
		case 7:
			new_attr = ATTR_REVERSE;
			break;
		}
		CHARATTR(wsp->f->sc, new_attr);
		esc_spt = esc_seq;
		break;
	}
	case '@':
#ifdef DEBUG_PRINT_ESC
DPRINTF("ws_parserest: doing ws_insch: %d\n", number);
#endif /* DEBUG_PRINT_ESC */
		if (number == DEFAULT)
			ws_insch(wsp, 1);
		else
			ws_insch(wsp, number);
		esc_spt = esc_seq;
		break;
	case 'H':
#ifdef DEBUG_PRINT_ESC
DPRINTF("ws_parserest: doing ws_home\n");
#endif /* DEBUG_PRINT_ESC */
		ws_home(wsp);
		esc_spt = esc_seq;
		break;
	case 'A':
#ifdef DEBUG_PRINT_ESC
DPRINTF("ws_parserest: doing ws_up: %d\n", number);
#endif /* DEBUG_PRINT_ESC */
		if (number == DEFAULT)
			ws_up(wsp);
		else
			while (number--)
				ws_up(wsp);
		esc_spt = esc_seq;
		break;
	case 'B':
#ifdef DEBUG_PRINT_ESC
DPRINTF("ws_parserest: doing ws_down: %d\n", number);
#endif /* DEBUG_PRINT_ESC */
		if (number == DEFAULT)
			ws_down(wsp);
		else
			while (number--)
				ws_down(wsp);
		esc_spt = esc_seq;
		break;
	case 'C':
#ifdef DEBUG_PRINT_ESC
DPRINTF("ws_parserest: doing ws_right: %d\n", number);
#endif /* DEBUG_PRINT_ESC */
		if (number == DEFAULT)
			ws_right(wsp);
		else
			while (number--)
				ws_right(wsp);
		esc_spt = esc_seq;
		break;
	case 'D':
#ifdef DEBUG_PRINT_ESC
DPRINTF("ws_parserest: doing ws_left: %d\n", number);
#endif /* DEBUG_PRINT_ESC */
		if (number == DEFAULT)
			ws_left(wsp);
		else
			while (number--)
				ws_left(wsp);
		esc_spt = esc_seq;
		break;
	case 'E':
#ifdef DEBUG_PRINT_ESC
DPRINTF("ws_parserest: doing CR then ws_down: %d\n", number);
#endif /* DEBUG_PRINT_ESC */
		ws_cr(wsp);
		if (number == DEFAULT)
			ws_down(wsp);
		else
			while (number--)
				ws_down(wsp);
		esc_spt = esc_seq;
		break;
	case 'F':
#ifdef DEBUG_PRINT_ESC
DPRINTF("ws_parserest: doing CR then ws_up: %d\n", number);
#endif /* DEBUG_PRINT_ESC */
		ws_cr(wsp);
		if (number == DEFAULT)
			ws_up(wsp);
		else
			while (number--)
				ws_up(wsp);
		esc_spt = esc_seq;
		break;
	case 'G':
#ifdef DEBUG_PRINT_ESC
DPRINTF("ws_parserest: doing set column: %d\n", number);
#endif /* DEBUG_PRINT_ESC */
		if (number == DEFAULT)
			number = 0;
		else
			if (number > 0)
				--number;	/* because number is from 1 */
#ifdef FIXME
		ws_setpos(BEG_OF_LINE(ws_curpos) + number); */
#else
		sp->col = number;
#endif /* FIXME */
		esc_spt = esc_seq;
		break;
	case ';':
		++cp;
		if (*cp == '\0')
			break;			/* not ready yet */
#ifdef DEBUG_PRINT_ESC
DPRINTF("ws_parserest: first numeric param: %d\n", number);
#endif /* DEBUG_PRINT_ESC */
		if (number == DEFAULT)
			number = 0;
		else if (number > 0)
			--number;		/* numbered from 1 */
#ifdef FIXME
		newpos = (number * ONE_LINE);   /* setup row */
#endif /* FIXME */
		cp += ws_atoi(cp, &number1);
		if (*cp == '\0')
			break;			/* not ready yet */
#ifdef DEBUG_PRINT_ESC
DPRINTF("ws_parserest: second numeric param: %d\n", number1);
#endif /* DEBUG_PRINT_ESC */
#ifdef FIXME
		if (number1 == DEFAULT)
			number1 = 0;
		else if (number1 > 0)
			number1--;		/* numbered from 1 */
		newpos += (number1);	/* setup column */
		if (newpos < 0)
			newpos = 0;		/* upper left */
		if (newpos > ONE_PAGE)
			newpos = (ONE_PAGE - 1); 
#endif /* FIXME */
		if (*cp == '\0')
			break;			/* *STILL* not ready... */
		if (*cp == 'H') { /* CUP - set cursor position */
#ifdef FIXME
			ws_setpos(newpos);
#else
			sp->row = number;
			if (number >= sp->max_row)
				sp->row = sp->max_row - 1;
			if (number1 == DEFAULT)
				number1 = 0;
			else if (number1 > 0)
				number1--;		/* numbered from 1 */
			sp->col = number1;
			if (number1 >= sp->max_col)
				sp->col = sp->max_col - 1;
#endif /* FIXME */
			esc_spt = esc_seq;	/* done, reset */
		}	
#ifdef VT100_EMUL
		else if (*cp == 'r') { /* set scrolling region */
/* FIXME FIXME - more bounds checking and stuff needed here... */
			if (number1 == DEFAULT || number1 == 0 ||
			    number1 >= sp->max_row)
				number1 = sp->max_row - 1;
			else if (number1 > 0)
				number1--;		/* numbered from 1 */
			if (number1 > number) { /* gotta be... :-) */
				top_marg = number;
				bot_marg = number1;
                                /* CursorSet(screen, 0, 0, term->flags); */
			}
			esc_spt = esc_seq;	/* done, reset */
#ifdef DEBUG_VT100_IMPDEL
xxputs("<ESC> [ # ; # r encountered...\n");
#endif /* DEBUG_VT100_IMPDEL */
		}
#endif /* VT100_EMUL */
		else
			esc_spt = esc_seq;
		break;				/* done or not ready */
	case 'J':
#ifdef DEBUG_PRINT_ESC
DPRINTF("ws_parserest: doing clto[bp]cur or cls: %d\n", number);
#endif /* DEBUG_PRINT_ESC */
		switch(number) {
		case DEFAULT:
		case 0:
			/* clears from current pos to bottom. */
			ws_cltobcur(wsp);
			break;
		case 1:
			/* clears from top to current pos. */	
			ws_cltopcur(wsp);
			break;
		case 2:
			ws_cls(wsp);
			break;
		default:
			break;
		}
		esc_spt = esc_seq;		/* reset it */
		break;
	case 'K':
#ifdef DEBUG_PRINT_ESC
DPRINTF("ws_parserest: doing cl{toe,frb}cur or eraseln: %d\n", number);
#endif /* DEBUG_PRINT_ESC */
		switch(number) {
		case DEFAULT:
		case 0:
			/* clears from current pos to eoln. */	
			ws_cltoecur(wsp);
			break;
		case 1:
			/* clears from begin of line to current pos. */
			ws_clfrbcur(wsp);
			break;
		case 2:
			ws_eraseln(wsp);	/* clear entire line */
			break;
		default:
			break;
		}
		esc_spt = esc_seq;
		break;
	case 'L':
#ifdef DEBUG_PRINT_ESC
DPRINTF("ws_parserest: doing ws_insln: %d\n", number);
#endif /* DEBUG_PRINT_ESC */
		if (number == DEFAULT)
			ws_insln(wsp, 1);
		else
			ws_insln(wsp, number);
		esc_spt = esc_seq;
		break;
	case 'M':
#ifdef DEBUG_PRINT_ESC
DPRINTF("ws_parserest: doing ws_delln: %d\n", number);
#endif /* DEBUG_PRINT_ESC */
		if (number == DEFAULT)
			ws_delln(wsp, 1);
		else
			ws_delln(wsp, number);
		esc_spt = esc_seq;
		break;
	case 'P':
#ifdef DEBUG_PRINT_ESC
DPRINTF("ws_parserest: doing ws_delch: %d\n", number);
#endif /* DEBUG_PRINT_ESC */
		if (number == DEFAULT)
			ws_delch(wsp, 1);
		else
			ws_delch(wsp, number);
		esc_spt = esc_seq;
		break;
	case 'S':
#ifdef DEBUG_PRINT_ESC
DPRINTF("ws_parserest: doing ws_scrollup: %d\n", number);
#endif /* DEBUG_PRINT_ESC */
		if (number == DEFAULT)
			ws_scrollup(wsp);
		else
			while (number--)
				ws_scrollup(wsp);
		esc_spt = esc_seq;
		break;
	case 'T':
#ifdef DEBUG_PRINT_ESC
DPRINTF("ws_parserest: doing ws_scrolldn: %d\n", number);
#endif /* DEBUG_PRINT_ESC */
		if (number == DEFAULT)
			ws_scrolldn(wsp);
		else
			while (number--)
				ws_scrolldn(wsp);
		esc_spt = esc_seq;
		break;
	case 'X':
#ifdef DEBUG_PRINT_ESC
DPRINTF("ws_parserest: doing ws_erase: %d\n", number);
#endif /* DEBUG_PRINT_ESC */
		if (number == DEFAULT)
			ws_erase(wsp, 1);
		else
			ws_erase(wsp, number);
		esc_spt = esc_seq;
		break;	
	case '\0':
		break;			/* not enough yet */
	default:
#ifdef DEBUG_PRINT_ERRORS
DPRINTF("ws_parserest: default - invalid char in seq: %d\n", *cp);
#endif /* DEBUG_PRINT_ERRORS */
		/* show invalid character */
                ws_putc(wsp, *cp);
		esc_spt = esc_seq;	/* invalid entry, reset */
		break;
	}
	return;
}



/*
 * ws_atoi:
 *
 *	This function converts an ascii string into an integer, and
 *	returns DEFAULT if no integer was found.  Note that this is why
 *	we don't use the regular atio(), because ZERO is ZERO and not
 *	the DEFAULT in all cases.
 *
 * input	: string
 * output	: a number or possibly DEFAULT, and the count of characters
 *		  consumed by the conversion
 *
 */
int
ws_atoi(cp, nump)
register u_char	*cp;
register int	*nump;
{
	register int	number;
	register u_char	*original;

	original = cp;
	for (number = 0; ('0' <= *cp) && (*cp <= '9'); cp++)
		number = (number * 10) + (*cp - '0');
	if (original == cp)
		*nump = DEFAULT;
	else
		*nump = number;
	return(cp - original);
}


/*
 * ws_scrollup:
 *
 *	This function scrolls the screen up one line using a memory
 *	copy.
 *
 * input	: None
 * output	: lines on screen appear shifted up one line
 *
 */
ws_scrollup(wsp)
register ws_screens *wsp;
{
	register ws_screen_descriptor *sp = wsp->sp;
	int to, from;
	int	count;

#ifdef DEBUG_PRINT_CHAR
DPRINTF("ws_scrollup: entry\n");
#endif /* DEBUG_PRINT_CHAR */

#ifdef VT100_EMUL
	/* scroll up */
	to = ROW_POS(sp, top_marg);
	from = ROW_POS(sp, top_marg+1);
	count = ONE_LINE(sp) * (bot_marg - top_marg);
	CHARMVUP(wsp->f->sc, sp, from, to, count);

	/* clear bottom line */
	to = ROW_POS(sp, bot_marg);
	count = ONE_LINE(sp);
	CHARCLEAR(wsp->f->sc, sp, to, count);
#else /* VT100_EMUL */
	/* scroll up */
	to = 0;
	from = ONE_LINE(sp);
	count = ONE_PAGE(sp) - ONE_LINE(sp);
	CHARMVUP(wsp->f->sc, sp, from, to, count);

	/* clear bottom line */
	to = BOTTOM_LINE(sp);
	count = ONE_LINE(sp);
	CHARCLEAR(wsp->f->sc, sp, to, count);
#endif /* VT100_EMUL */
	return;
}


/*
 * ws_scrolldn:
 *
 *	Scrolls the characters on the screen down one line.
 *
 * input	: None
 * output	: Lines on screen appear to be moved down one line
 *
 */
ws_scrolldn(wsp)
register ws_screens *wsp;
{
	register ws_screen_descriptor *sp = wsp->sp;
	int to, from;
	int	count;

#ifdef DEBUG_PRINT_CHAR
DPRINTF("ws_scrolldn: entry\n");
#endif /* DEBUG_PRINT_CHAR */

#ifdef VT100_EMUL
	/* move down */
	to 	= ROW_POS(sp, bot_marg+1) - 1;
	from 	= ROW_POS(sp, bot_marg) - 1;
	count 	= ONE_LINE(sp) * (bot_marg - top_marg);
	CHARMVDOWN(wsp->f->sc, sp, from, to, count);

	/* clear top line */
	to	= ROW_POS(sp, top_marg);
	count	= ONE_LINE(sp);
	CHARCLEAR(wsp->f->sc, sp, to, count);
#else /* VT100_EMUL */
	/* move down */
	to 	= ONE_PAGE(sp) - 1;
	from 	= ONE_PAGE(sp) - ONE_LINE(sp) - 1;
	count 	= ONE_PAGE(sp) - ONE_LINE(sp);
	CHARMVDOWN(wsp->f->sc, sp, from, to, count);

	/* clear top line */
	to	= TOP_LINE(sp);
	count	= ONE_LINE(sp);
	CHARCLEAR(wsp->f->sc, sp, to, count);
#endif /* VT100_EMUL */
	return;
}


/*
 * ws_cls:
 *
 *	This function clears the screen with spaces.
 *
 * input	: None
 * output	: Screen is cleared
 *
 */

ws_cls(wsp)
register ws_screens *wsp;
{
	register ws_screen_descriptor *sp = wsp->sp;

#ifdef DEBUG_PRINT_CHAR
DPRINTF("ws_cls: entry\n");
#endif /* DEBUG_PRINT_CHAR */

	CHARCLEAR(wsp->f->sc, sp, TOP_LINE(sp), ONE_PAGE(sp));
	return;
}


/*
 * ws_home:
 *
 *	This function will move the cursor to the home position on the screen,
 *	as well as set the internal cursor position (ws_curpos) to home.
 *
 * input	: None
 * output	: Cursor position is moved
 *
 */

ws_home(wsp)
register ws_screens *wsp;
{
#ifdef DEBUG_PRINT_CHAR
DPRINTF("ws_home: entry\n");
#endif /* DEBUG_PRINT_CHAR */

#ifdef FIXME
	ws_setpos(0);
#else
	wsp->sp->row = wsp->sp->col = 0;
#endif /* FIXME */
	return;
}


/*
 * ws_up:
 *
 *	This function moves the cursor up one line position.
 *
 * input	: None
 * output	: Cursor moves up one line, or screen is scrolled
 *
 */

ws_up(wsp)
	register ws_screens *wsp;
{
#ifdef DEBUG_PRINT_CHAR
DPRINTF("ws_up: entry\n");
#endif /* DEBUG_PRINT_CHAR */

#ifdef VT100_EMUL
	if (wsp->sp->row != top_marg) {
		ws_up_noscroll(wsp);
		return;
	}
	ws_scrolldn(wsp);
#else /* VT100_EMUL */
	if (wsp->sp->row < 1)
		ws_scrolldn(wsp);
	else
#ifdef FIXME
		ws_setpos(ws_curpos - ONE_LINE);
#else
		wsp->sp->row -= 1;
#endif /* FIXME */
#endif /* VT100_EMUL */
	return;
}

#ifdef VT100_EMUL
ws_up_noscroll(wsp)
	register ws_screens *wsp;
{
#ifdef DEBUG_PRINT_CHAR
DPRINTF("ws_up_noscroll: entry\n");
#endif /* DEBUG_PRINT_CHAR */

	if (wsp->sp->row > 0)
#ifdef FIXME
		ws_setpos(ws_curpos - ONE_LINE);
#else
		wsp->sp->row -= 1;
#endif /* FIXME */
	return;
}
#endif /* VT100_EMUL */

/*
 * ws_down:
 *
 *	This function moves the cursor down one line position.
 *
 * input	: None
 * output	: Cursor moves down one line or the screen is scrolled
 *
 */
ws_down(wsp)
	register ws_screens *wsp;
{
#ifdef DEBUG_PRINT_CHAR
DPRINTF("ws_down: entry\n");
#endif /* DEBUG_PRINT_CHAR */

#ifdef VT100_EMUL
	if (wsp->sp->row != bot_marg) {
		ws_down_noscroll(wsp);
		return;
	}
	ws_scrollup(wsp);
#else /* VT100_EMUL */
	if (wsp->sp->row >= (wsp->sp->max_row - 1))
		ws_scrollup(wsp);
	else
#ifdef FIXME
		ws_setpos(ws_curpos + ONE_LINE);
#else
		wsp->sp->row += 1;
#endif /* FIXME */
#endif /* VT100_EMUL */
	return;
}

#ifdef VT100_EMUL
ws_down_noscroll(wsp)
	register ws_screens *wsp;
{
#ifdef DEBUG_PRINT_CHAR
DPRINTF("ws_down_noscroll: entry\n");
#endif /* DEBUG_PRINT_CHAR */

	if (wsp->sp->row < (wsp->sp->max_row - 1))
#ifdef FIXME
		ws_setpos(ws_curpos + ONE_LINE);
#else
		wsp->sp->row += 1;
#endif /* FIXME */
	return;
}
#endif /* VT100_EMUL */

/*
 * ws_right:
 *
 *	This function moves the cursor one position to the right.
 *
 * input	: None
 * output	: Cursor moves one position to the right
 *
 */
ws_right(wsp)
register ws_screens *wsp;
{
	register ws_screen_descriptor *sp = wsp->sp;

#ifdef DEBUG_PRINT_CHAR
DPRINTF("ws_right: entry\n");
#endif /* DEBUG_PRINT_CHAR */

#ifdef FIXME
	if (ws_curpos < (ONE_PAGE - 1))
		ws_setpos(ws_curpos + 1);
	else {
		ws_scrollup(ws);
		ws_setpos(BEG_OF_LINE(ws_curpos));
	}
#else
#ifdef VT100_EMUL
	if (sp->col < sp->max_col)
		sp->col += 1;
	else if (sp->row < (sp->max_row - 1)) {
		sp->col = 1;
		sp->row += 1;
	}
	else {
		ws_scrollup(wsp);
		sp->col = 1;
	}
#else
	if (sp->col < (sp->max_col - 1))
		sp->col += 1;
	else if (sp->row < (sp->max_row - 1)) {
		sp->col = 0;
		sp->row += 1;
	}
	else {
		ws_scrollup(wsp);
		sp->col = 0;
	}
#endif /* VT100_EMUL */
#endif /* FIXME */
	return;
}


/*
 * ws_left:
 *
 *	This function moves the cursor one position to the left.
 *
 * input	: None
 * output	: Cursor moves one position to the left
 *
 */
ws_left(wsp)
register ws_screens *wsp;
{
	register ws_screen_descriptor *sp = wsp->sp;

#ifdef DEBUG_PRINT_CHAR
DPRINTF("ws_left: entry\n");
#endif /* DEBUG_PRINT_CHAR */

#ifdef FIXME
	if (0 < ws_curpos)
		ws_setpos(ws_curpos - 1);
#else
	if (0 < sp->col)
		sp->col -= 1;
	else if (0 < sp->row) {
		sp->row -= 1;
		sp->col = sp->max_col - 1;
	}
#endif /* FIXME */
	return;
}


/*
 * ws_cr:
 *
 *	This function moves the cursor to the beginning of the current
 *	line.
 *
 * input	: None
 * output	: Cursor moves to the beginning of the current line
 *
 */
ws_cr(wsp)
register ws_screens *wsp;
{
#ifdef DEBUG_PRINT_CHAR
DPRINTF("ws_cr: entry\n");
#endif /* DEBUG_PRINT_CHAR */

#ifdef FIXME
	ws_setpos(BEG_OF_LINE(ws_curpos));
#else
	wsp->sp->col = 0;
#endif /* FIXME */
	return;
}


/*
 * ws_cltobcur:
 *
 *	This function clears from the current cursor position to the bottom
 *	of the screen.
 *
 * input	: None
 * output	: Screen is cleared from current cursor postion to bottom
 *
 */
ws_cltobcur(wsp)
register ws_screens *wsp;
{
	register ws_screen_descriptor *sp = wsp->sp;
	int start;
	int	count;

#ifdef DEBUG_PRINT_CHAR
DPRINTF("ws_cltobcur: entry\n");
#endif /* DEBUG_PRINT_CHAR */

#ifdef FIXME
	start = ws_curpos;
	count = (ONE_PAGE - ws_curpos);
#else
	start = CUR_POS(sp);
	count = (ONE_PAGE(sp) - start);
#endif /* FIXME */
	CHARCLEAR(wsp->f->sc, sp, start, count);
	return;
}


/*
 * ws_cltopcur:
 *
 *	This function clears from the current cursor position to the top
 *	of the screen.
 *
 * input	: None
 * output	: Screen is cleared from current cursor postion to top
 *
 */
ws_cltopcur(wsp)
register ws_screens *wsp;
{
	register ws_screen_descriptor *sp = wsp->sp;
	register int	count;

#ifdef DEBUG_PRINT_CHAR
DPRINTF("ws_cltopcur: entry\n");
#endif /* DEBUG_PRINT_CHAR */

#ifdef FIXME
	count = (ws_curpos + 1);
#else
	count = CUR_POS(sp) + 1;
#endif /* FIXME */
	CHARCLEAR(wsp->f->sc, sp, 0, count);
	return;
}


/*
 * ws_cltoecur:
 *
 *	This function clears from the current cursor position to eoln. 
 *
 * input	: None
 * output	: Line is cleared from current cursor position to eoln
 *
 */
ws_cltoecur(wsp)
register ws_screens *wsp;
{
	register ws_screen_descriptor *sp = wsp->sp;
	register int i, hold;

#ifdef DEBUG_PRINT_CHAR
DPRINTF("ws_cltoecur: entry\n");
#endif /* DEBUG_PRINT_CHAR */

#ifdef FIXME
	hold = BEG_OF_LINE(ws_curpos) + ONE_LINE;
	for (i = ws_curpos; i < hold; i++) {
	    CHARPUT(wsp->f->sc, sp, i, K_SPACE);
	}
#else
/* FIXME - for VT100_EMUL, if CUR_POS(sp) is beyond end of line, OK?! */
	hold = CUR_ROW_POS(sp) + ONE_LINE(sp);
	for (i = CUR_POS(sp); i < hold; i++) {
	    CHARPUT(wsp->f->sc, sp, i, K_SPACE);
	}
#endif /* FIXME */
}


/*
 * ws_clfrbcur:
 *
 *	This function clears from the beginning of the line to the current
 *	cursor position.
 *
 * input	: None
 * output	: Line is cleared from beginning to current position
 *
 */
ws_clfrbcur(wsp)
register ws_screens *wsp;
{
	register ws_screen_descriptor *sp = wsp->sp;
	register int i;

#ifdef DEBUG_PRINT_CHAR
DPRINTF("ws_clfrbcur: entry\n");
#endif /* DEBUG_PRINT_CHAR */

#ifdef FIXME
	for (i = BEG_OF_LINE(ws_curpos); i <= ws_curpos; i++) {
		CHARPUT(wsp->f->sc, sp, i, K_SPACE);
	}
#else
/* FIXME - for VT100_EMUL, if CUR_POS(sp) is beyond end of line, bad! */
	for (i = CUR_ROW_POS(sp); i <= CUR_POS(sp); i++) {
		CHARPUT(wsp->f->sc, sp, i, K_SPACE);
	}
#endif /* FIXME */
}


/*
 * ws_delln:
 *
 *	This function deletes 'number' lines on the screen by effectively
 *	scrolling the lines up and replacing the old lines with spaces.
 *
 * input	: number of lines to delete
 * output	: lines appear to be deleted
 *
 */
ws_delln(wsp, number)
register ws_screens *wsp;
int	number;
{
	register ws_screen_descriptor *sp = wsp->sp;
	int to, from;
	int	delbytes;		/* num of bytes to delete */
	int	count;			/* num of words to move or fill */

#ifdef DEBUG_PRINT_CHAR
DPRINTF("ws_delln: entry\n");
#endif /* DEBUG_PRINT_CHAR */

	if (number <= 0)
		return;

	delbytes = number * ONE_LINE(sp);
#ifdef FIXME
	to = BEG_OF_LINE(ws_curpos);
#else
	to = CUR_ROW_POS(sp);
#endif /* FIXME */
	if (to + delbytes >= ONE_PAGE(sp))
		delbytes = ONE_PAGE(sp) - to;
	if (to + delbytes < ONE_PAGE(sp)) {
		from = to + delbytes;
		count = (ONE_PAGE(sp) - from);
		CHARMVUP(wsp->f->sc, sp, from, to, count);
	}

	to = ONE_PAGE(sp) - delbytes;
	count = delbytes;
	CHARCLEAR(wsp->f->sc, sp, to, count);
	return;
}


/*
 * ws_insln:
 *
 *	This function inserts a line above the current one by
 *	scrolling the current line and all the lines below it down.
 *
 * input	: number of lines to insert
 * output	: New lines appear to be inserted
 *
 */
ws_insln(wsp, number)
register ws_screens *wsp;
int	number;
{
	register ws_screen_descriptor *sp = wsp->sp;
	int to, from, top;
	int	count;
	int	insbytes;		/* num of bytes inserted */

#ifdef DEBUG_PRINT_CHAR
DPRINTF("ws_insln: entry\n");
#endif /* DEBUG_PRINT_CHAR */

	if (number <= 0)
		return;

#ifdef FIXME
	top = BEG_OF_LINE(ws_curpos);
#else
	top = CUR_ROW_POS(sp);
#endif /* FIXME */
	insbytes = number * ONE_LINE(sp);
	if (top + insbytes > ONE_PAGE(sp))
		insbytes = ONE_PAGE(sp) - top;
	to = ONE_PAGE(sp) - 1;
	from = to - insbytes;
	if (from > top) {
		count = (from - top + 1);
		CHARMVDOWN(wsp->f->sc, sp, from, to, count);
	}

	count = insbytes;
	CHARCLEAR(wsp->f->sc, sp, top, count);
	return;
}


/*
 * ws_delch:
 *
 *	This function deletes a number of characters from the current 
 *	position in the line.
 *
 * input	: number of characters to delete
 * output	: characters appear to be deleted
 *
 */
ws_delch(wsp, number)
register ws_screens *wsp;
int	number;
{
	register ws_screen_descriptor *sp = wsp->sp;
	int	count;			/* num words moved/filled */
	int	delbytes;		/* bytes to delete */
	int to, from, nextline, cur;

#ifdef DEBUG_PRINT_CHAR
DPRINTF("ws_delch: entry\n");
#endif /* DEBUG_PRINT_CHAR */

	if (number <= 0)
		return;

#ifdef FIXME
	nextline = BEG_OF_LINE(ws_curpos) + ONE_LINE;
#else
	nextline = CUR_ROW_POS(sp) + ONE_LINE(sp);
#endif /* FIXME */
	delbytes = number;
#ifdef FIXME
	if (ws_curpos + delbytes > nextline)
		delbytes = nextline - ws_curpos;
	if (ws_curpos + delbytes < nextline) {
		from = ws_curpos + delbytes;
		to = ws_curpos;
		count = (nextline - from) / ONE_SPACE;
		CHARMVUP(wsp->f->sc, sp, from, to, count);
	}
#else
	cur = CUR_POS(sp);
	if (cur + delbytes > nextline)
		delbytes = nextline - cur;
	if (cur + delbytes < nextline) {
		from = cur + delbytes;
		to = cur;
		count = (nextline - from);
		CHARMVUP(wsp->f->sc, sp, from, to, count);
	}
#endif /* FIXME */

	to = nextline - delbytes;
	count = delbytes;
	CHARCLEAR(wsp->f->sc, sp, to, count);
	return;
}


/*
 * ws_erase:
 *
 *	This function overwrites characters with a space starting with the
 *	current cursor position and ending in number spaces away.
 *
 * input	: number of characters to erase
 * output	: characters appear to be blanked or erased
 *
 */
ws_erase(wsp, number)
register ws_screens *wsp;
int	number;
{
	register ws_screen_descriptor *sp = wsp->sp;
	register int i, stop, cur;

#ifdef DEBUG_PRINT_CHAR
DPRINTF("ws_erase: entry\n");
#endif /* DEBUG_PRINT_CHAR */

#ifdef FIXME
	stop = ws_curpos + (number);	
	if (stop > BEG_OF_LINE(ws_curpos) + ONE_LINE)
		stop = BEG_OF_LINE(ws_curpos) + ONE_LINE;
	for (i = ws_curpos; i < stop; i++) {
		CHARPUT(wsp->f->sc, wsp->sp, i, K_SPACE);
	}
#else
/* FIXME - for VT100_EMUL, if CUR_POS(sp) is beyond end of line, OK? */
	cur = CUR_POS(sp);
	stop = cur + number;	
	if (stop > CUR_ROW_POS(sp) + ONE_LINE(sp))
		stop = CUR_ROW_POS(sp) + ONE_LINE(sp);/* FIXME - opt */
	for (i = CUR_POS(sp); i < stop; i++) {
		CHARPUT(wsp->f->sc, sp, i, K_SPACE);
	}
#endif /* FIXME */
	return;
}


/*
 * ws_eraseln:
 *
 *	This function erases the current line with spaces.
 *
 * input	: None
 * output	: Current line is erased
 *
 */
ws_eraseln(wsp)
register ws_screens *wsp;
{
	register ws_screen_descriptor *sp = wsp->sp;
	register int i, stop;

#ifdef DEBUG_PRINT_CHAR
DPRINTF("ws_eraseln: entry\n");
#endif /* DEBUG_PRINT_CHAR */

#ifdef FIXME
	stop = BEG_OF_LINE(ws_curpos) + ONE_LINE;
	for (i = BEG_OF_LINE(ws_curpos); i < stop; i++) {
		CHARPUT(wsp->f->sc, wsp->sp, i, K_SPACE);
	}
#else
/* FIXME - for VT100_EMUL, if CUR_POS(sp) is beyond end of line, OK? */
	stop = CUR_ROW_POS(sp) + ONE_LINE(sp);	/* FIXME - opt */
	for (i = CUR_ROW_POS(sp); i < stop; i++) {
		CHARPUT(wsp->f->sc, sp, i, K_SPACE);
	}
#endif /* FIXME */
	return;
}


/*
 * ws_insch:
 *
 *	This function inserts a blank at the current cursor position
 *	and moves all other characters on the line over.
 *
 * input	: number of blanks to insert
 * output	: Blanks are inserted at cursor position
 *
 */
ws_insch(wsp, number)
register ws_screens *wsp;
int	number;
{
	register ws_screen_descriptor *sp = wsp->sp;
	int to, from, nextline, cur;
	int	count;
	int	insbytes;		/* num of bytes inserted */

#ifdef DEBUG_PRINT_CHAR
DPRINTF("ws_insch: entry\n");
#endif /* DEBUG_PRINT_CHAR */

	if (number <= 0)
		return;

#ifdef FIXME
	nextline = BEG_OF_LINE(ws_curpos) + ONE_LINE;
#else
	nextline = CUR_ROW_POS(sp) + ONE_LINE(sp);
#endif /* FIXME */
	insbytes = number;

#ifdef FIXME
	if (ws_curpos + insbytes > nextline)
		insbytes = nextline - ws_curpos;

	to = nextline - 1;
	from = to - insbytes;
	if (from >= ws_curpos) {
		count = (from - ws_curpos + 1);
		CHARMVDOWN(wsp->f->sc, wsp->sp,
					 from, to, count);
	}

	count = insbytes;
	CHARCLEAR(wsp->f->sc, wsp->sp, ws_curpos, count);
#else
	cur = CUR_POS(sp);
	if (cur + insbytes > nextline)
		insbytes = nextline - cur;

	to = nextline - 1;
	from = to - insbytes;
	if (from >= cur) {
		count = (from - cur + 1);
		CHARMVDOWN(wsp->f->sc, sp, from, to, count);
	}

	count = insbytes;
	CHARCLEAR(wsp->f->sc, sp, cur, count);
#endif /* FIXME */
	return;
}

#ifdef FIXME
/*
 * ws_setpos:
 *
 *	This function sets the software and hardware cursor position
 *	on the screen, using device-specific code to actually move and
 *	display the cursor.
 *
 * input	: position on (or off) screen to move the cursor to
 * output	: cursor position is updated, screen has been scrolled
 *		  if necessary to bring cursor position back onto
 *		  screen.
 *
 */
ws_setpos(wsp, newpos)
register ws_screens *wsp;
csrpos_t newpos;
{
	register ws_screen_descriptor *screen = wsp->sp;

	if (newpos > ONE_PAGE) {
		ws_scrollup(wsp);
		newpos = BOTTOM_LINE;
	}
	if (newpos < 0) {
		ws_scrolldn(wsp);
		newpos = 0;
	}

	(*(wsp->f->charsetcursor)) (wsp->f->sc, wsp->sp, newpos);
}
#endif /* FIXME */

ws_register_emulation_functions(efp)
	ws_emulation_functions *efp;
{
	/* check for ALL functions available */
	if (!efp->charclear || !efp->charmvup || !efp->charmvdown ||
	    !efp->charattr  || !efp->charput) 
	{
		return(-1);
	}

	/* set pointer in ws_info structure */
	ws_softc[0].emul_funcs = efp;
	return(0);
}

ws_is_mouse_on()
{
	return(IS_MOUSE_ON);
}
