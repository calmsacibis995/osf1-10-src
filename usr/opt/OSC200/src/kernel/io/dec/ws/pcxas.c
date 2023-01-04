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
static char *rcsid = "@(#)$RCSfile: pcxas.c,v $ $Revision: 1.1.5.5 $ (DEC) $Date: 1993/11/17 17:27:55 $";
#endif

/* #define DEBUG_PRINT_ENTRY */
/* #define DEBUG_PRINT_ERRORS */
/* #define DEBUG_PRINT_INFO */
/* #define DEBUG_PRINT_INTERRUPT */
/* #define DEBUG_PRINT_MOUSE_REPORT */
/* #define DEBUG_PRINT_EVENT */
/* #define MOUSE_TEST */
/* #define MOUSE_DEBUG */

/* #define DPRINTF jprintf */
#define DPRINTF printf
/* #define DPRINTF xxputs */

#ifdef DEBUG_PRINT_ERRORS
#define ERR_PRINTF DPRINTF
#else
#define ERR_PRINTF
#endif /* DEBUG_PRINT_ERRORS */

/*
 * support for PCXAS and successor mice.
 * written by J. Estabrook, 
 * based on VSXXX support by J. Gettys
 */

#include <data/ws_data.c>
#include <data/pcxas_data.c>

extern  ws_info ws_softc[];

void pcxas_tablet_scale();
int pcxas_intr();

enum mouse_state_enum {
	UNKNOWN_STATE,
	INIT_STATE,
	FIRST_ENABLE_STATE,
	FIRST_DISABLE_STATE,
	SECOND_ENABLE_STATE,
	DISABLED_STATE,
	ENABLED_STATE
};
enum mouse_state_enum mouse_state = UNKNOWN_STATE;

/*
**  under conditions of heavy load, the mouse interrupt handler
**  (pcxas_mouse_event()) switches screens well ahead of the
**  server's processing of the event queue. In those cases, some
**  cursor handling (particularly load_cursor()) to the 'old' screen
**  cause the cursor to hang. The flag (wi->mouse_screen) is updated
**  by pcxas_mouse_event() to point to the current screen, to help
**  keep the action on the straight and narrow path...
*/

/*
 * Routine to initialize the mouse. 
 *
 * Called from ws_cons_init via function dispatch table. ONLY CALLED ONCE!
 *
 * NOTE:
 *	This routine communicates with the mouse by using poll-mode
 *	routines to talk to the keyboard/mouse interface. This is allowed
 *	ONLY because the mouse is initialized before the system
 *	is up far enough to need the interface in interrupt mode.
 */
int
pcxas_init_pointer(closure)
	caddr_t closure;
{
	int count;
	int status;
	int data;
	int reply[3];
        ws_info *wi = &ws_softc[0];


#ifdef DEBUG_PRINT_ENTRY
DPRINTF("pcxas_init_pointer: entry\n");
#endif /* DEBUG_PRINT_ENTRY */

	/* Global */
	reply[0] = reply[1] = reply[2] = 0xee;

	/* Flush */
	while(gpc_input() != -1)
		continue;

#ifdef MOUSE_TEST
	gpc_ctl_cmd(PK_CTL_MTEST);
	data = gpc_input();

	DPRINTF("pcxas_init_pointer: Results of mouse interface test: ");
	switch(data) {
#ifdef BY_THE_SPEC
		case 0: DPRINTF("No error.\n"); break;
		case 1: DPRINTF("Mouse Clock line stuck low.\n"); break;
#else /* BY_THE_SPEC */
		case 1: DPRINTF("No error.\n"); break;
		case 0: DPRINTF("Mouse Clock line stuck low.\n"); break;
#endif /* BY_THE_SPEC */
		case 2: DPRINTF("Mouse Clock line stuck high.\n"); break;
		case 3: DPRINTF("Mouse Data line stuck low.\n"); break;
		case 4: DPRINTF("Mouse Data line stuck high.\n"); break;
		default: DPRINTF("unknown response %d\n",data); break;
	}

#ifdef BY_THE_SPEC
	if (data != 0)
#else /* BY_THE_SPEC */
	if (data != 1)
#endif /* BY_THE_SPEC */
	{
		ERR_PRINTF("pcxas_init_pointer: failed interface test.\n");
#ifdef MTEST_ERR_ABORT
		goto OUT;
#else
		ERR_PRINTF("pcxas_init_pointer: *IGNORING* failed test.\n");
#endif /* MTEST_ERR_ABORT */
	}
#endif /* MOUSE_TEST */

	/*
	 * Enable the mouse in mode register.
	 * Note that interrupts are still off.
	 */
	gpc_ctl_cmd(PK_CTL_MENABLE);

	DELAY(100);

	gpc_ctl_cmd(PK_CTL_WRMOUSE);	/* Send data to mouse.		*/
	gpc_ctl_output(PM_RESET);	/* Reset.			*/
	count = 0;
	while(count < 3){
		reply[count] = gpc_input();
		if(reply[count] < 0) {
			ERR_PRINTF("pcxas_init_pointer: failed to reset.\n");
			goto OUT;
		}
		++count;
	}

	/*
	 * See logitech spec page 2-11.  The successful answer (reply) to 
	 * the mouse reset command is FA AA 00. 
	 */
	if ((reply[0] != 0xFA) || (reply[1] != 0xAA) || (reply[2] != 0x00)) {
		ERR_PRINTF("pcxas_init_pointer: improper reply to reset.\n");
		goto OUT;
	}

	/* Tell mouse it's alive.*/
	gpc_ctl_cmd(PK_CTL_WRMOUSE);
	/*
	 * See logitech spec page 2-13.  The successful answer (reply) to 
	 * the mouse enable command is FA. 
	 */
	if((data = gpc_output(PM_ENABLE)) != 0xFA) {
		ERR_PRINTF("pcxas_init_pointer: improper reply to enable.\n");
		goto OUT;
	}

#ifdef DEBUG_PRINT_INFO_DISABLED
	DPRINTF("pcxas_init_pointer: mouse successfully configured.\n");
#endif /* DEBUG_PRINT_INFO*/

	/* leave the mouse disabled after the init */
 OUT:
	gpc_ctl_cmd(PK_CTL_MDISABLE);	/* FIXME FIXME - to unclog xface?? */
	mouse_state = INIT_STATE;
	vs_gdpint = pcxas_intr;
	return(MOUSE_ID); /* set into "pointer_id" by caller ws_cons_init) */
}

void
pcxas_reset_pointer(closure)
	caddr_t closure;
{
#ifdef DEBUG_PRINT_ENTRY
DPRINTF("pcxas_reset_pointer: entry\n");
#endif /* DEBUG_PRINT_ENTRY */
}

void
pcxas_enable_pointer(closure)
	caddr_t closure;
{
#ifdef DEBUG_PRINT_ENTRY
DPRINTF("pcxas_enable_pointer: entry\n");
#endif /* DEBUG_PRINT_ENTRY */

	switch (mouse_state) 
	{
	case ENABLED_STATE:
	case FIRST_ENABLE_STATE:
	case SECOND_ENABLE_STATE:
	default:
		ERR_PRINTF("pcxas_enable_pointer: illegal state/transition\n");
	case UNKNOWN_STATE:
		return;

	case INIT_STATE:
		mouse_state = FIRST_ENABLE_STATE;
		break;

	case FIRST_DISABLE_STATE:
		mouse_state = SECOND_ENABLE_STATE;
		break;

	case DISABLED_STATE:
		mouse_state = ENABLED_STATE;
		break;
	}

	current_rep.bytcnt = 0;	/* clear byte count before enabling */
	gpc_ctl_cmd(PK_CTL_MENABLE); /* FIXME - interrupts already on? */
}

void
pcxas_disable_pointer(closure)
	caddr_t closure;
{
/* FIXME - raise IPL so that ongoing MOUSE interrupts are shut off? */
	int s = spltty();
	
#ifdef DEBUG_PRINT_ENTRY
DPRINTF("pcxas_disable_pointer: entry\n");
#endif /* DEBUG_PRINT_ENTRY */

	switch (mouse_state) 
	{
	case INIT_STATE:
	case FIRST_DISABLE_STATE:
	case DISABLED_STATE:
	default:
		ERR_PRINTF("pcxas_disable_pointer: illegal state/transition\n");
	case UNKNOWN_STATE:
		return;

	case ENABLED_STATE:
		mouse_state = DISABLED_STATE;
		break;

	case FIRST_ENABLE_STATE:
		mouse_state = FIRST_DISABLE_STATE;
		break;

	case SECOND_ENABLE_STATE:
		mouse_state = FIRST_DISABLE_STATE;
		break;
	}

	gpc_ctl_cmd(PK_CTL_MDISABLE);	/* FIXME - leave interrupts on?? */

/* FIXME FIXME - we do a WRMOUSE RESEND here to start off next time in sync? */

	gpc_ctl_cmd(PK_CTL_WRMOUSE);	/* Send data to mouse.		*/
	gpc_ctl_output(PM_RESEND);	/* Resend last transmission.	*/

	splx(s);
}

/*
 * Routine to pre-process an hardware-dependent (PCXAS) mouse report into
 *  a "standardized" version for consumption by device-independent code.
 */
int
pcxas_mouse_event(wi, queue, wsp, p, last_rep, new_rep, open)
	register ws_info *wi;
	register ws_event_queue *queue;
	ws_screens *wsp;
	register ws_pointer *p;
	register ws_pointer_report *new_rep;
	ws_pointer_report *last_rep;
	int open;
{
	ws_pointer_report temp_rep;

#ifdef DEBUG_PRINT_ENTRY
DPRINTF("pcxas_mouse_event: entry\n");
#endif /* DEBUG_PRINT_ENTRY */

#ifdef FIXME
	/*
	 * First check for power up report.  If so, reset mouse.
	 */
        if((new_rep->state & 0xa0) == 0xa0) {
             DELAY(500000);
            (*slu.mouse_putc)(INCREMENTAL);
/* FIXME FIXME - anything else in here??? */
        }
#endif /* FIXME */

	/*
	 * Pre-process the report, setting the shorts "dx" and "dy" approp.,
	 * ie make them absolute (positive) values constrolled by sign bits.
	 * Also, the new state must conform as well to what the common
	 * code expects, according to sys/wsdevice.h.
	 *
	 */
	temp_rep.state = 0;
	temp_rep.dx = new_rep->dx;
	if (new_rep->state & PCXAS_X_SIGN) {
		temp_rep.dx |= 0xff00;	/* negative, so sign extend */
		temp_rep.dx = (~temp_rep.dx) + 1; /* make 2's comp positive */
		temp_rep.state |= WSPR_X_SIGN;
	}

	/*
	 * NOTE: it appears that PS/2 mice report Y positive for north...
	 */
	temp_rep.dy = new_rep->dy;
	if (new_rep->state & PCXAS_Y_SIGN) {
		temp_rep.dy |= 0xff00;	/* negative, so sign extend */
		temp_rep.dy = (~temp_rep.dy) + 1; /* make 2's comp positive */
	}
	else
		temp_rep.state |= WSPR_Y_SIGN;

	/* standard buttons are LMR, PCXAS buttons are MRL */
	temp_rep.state |= ((new_rep->state & 0x06) >> 1) |
			  ((new_rep->state & 0x01) << 2);

	temp_rep.bytcnt = 0;		/* just for consistency :-) */

#ifdef DEBUG_PRINT_EVENT
DPRINTF("pcxas_mouse_event: 0x%x 0x%x 0x%x\n",
	temp_rep.state, temp_rep.dx, temp_rep.dy);
#endif /* DEBUG_PRINT_EVENT */

	/*
	 * finally, actually process the "standardized" report
	 */
	return(ws_process_mouse_report(wi, queue, wsp, p, last_rep,
				       &temp_rep, open));
}

int
pcxas_tablet_event(wi, queue, p, last_rep, new_rep, screen, open)
	ws_info *wi;
	ws_event_queue *queue;
	ws_pointer *p;
	register ws_pointer_report *new_rep;
	ws_pointer_report *last_rep;
	int screen;
	int open;
{
#ifdef DEBUG_PRINT_ENTRY
DPRINTF("pcxas_tablet_event: entry\n");
#endif /* DEBUG_PRINT_ENTRY */

#ifdef FIXME
    int cs = p->position.screen;        /* current screen */
    register ws_screen_descriptor *sp = screens[cs].sp;
    ws_motion_history *mhp;
    unsigned int millis = TOY;
    int x, y, edge = 0;
    register ws_event *ev;
    register char temp;
    register int i, j;

    /* update cursor position coordinates where 0,0 is top left */
    if (p->tablet_x_axis)
        x =(((p->tablet_max_x-new_rep->dx)*sp->width)/
            p->tablet_scale_x) - p->tablet_overhang;
    else
        x =((new_rep->dx * sp->width) / p->tablet_scale_x)
            - p->tablet_overhang;
    if (p->tablet_y_axis)
        y =((new_rep->dy * sp->height) / p->tablet_scale_y)
            - p->tablet_overhang;
    else
        y =(((p->tablet_max_y-new_rep->dy)*sp->height)/
            p->tablet_scale_y) - p->tablet_overhang;

    if (x >= sp->width) { edge |= RIGHT_EDGE;  x = sp->width - 1; }
    if (x < 0)          { edge |= LEFT_EDGE;   x = 0; }
    if (y < 0)          { edge |= TOP_EDGE;    y = 0; }
    if (y >= sp->height){ edge |= BOTTOM_EDGE; y = sp->height - 1; }
    /*
     * nb: once you've moved to a new screen, you can't get out of that
     *  screen until you've moved out of the "edgework" overhang area.
     *  otherwise, you'd never be able to stay in the middle screen.
     *  cursor repositioning only occurs if the puck has moved, but
     *  not if you're just futzing around in the overhang area.
     */
    if (edge && p->tablet_new_screen == 0) {
        sp = ws_do_edge_work (wi, sp, p, edge);
        if (cs != p->position.screen) {
            cs = p->position.screen;
            p->tablet_new_screen = 1;
            /* approx. same place, other screen... */
            if ((edge & RIGHT_EDGE) || (x >= sp->width))
                x = sp->width - 1;
            if ((edge & BOTTOM_EDGE) || (y >= sp->height))
                y = sp->height - 1;
            /* need to recompute scaling factors for new screen */
            vsxxx_tablet_scale(p, sp);
        }
        else
            edge = 0;
    }
    else {
        /*
         * the "edgework" is centered around the real edges and extend
         * tablet_overhang pixels on both sides of it.
         */
        if (edge == 0) {
            int right = sp->width - p->tablet_overhang;
            int bottom = sp->height - p->tablet_overhang;
            if (x >= p->tablet_overhang && x < right &&
                y >= p->tablet_overhang && y < bottom)
                {
                    p->tablet_new_screen = 0;
                }
        }
        edge = 0;
    }

    /*
    **  identify the current active screen...
    */
    wi->mouse_screen = cs;

    /*
     * see if the puck/stylus has moved, or moved to new screen
     */
    if (p->position.x != x || p->position.y != y || edge) {
        /*
         * update cursor position
         */
        p->position.x = x;
        p->position.y = y;

        if (open) {
            ws_set_cursor_position
                (screens[cs].cf->cc, &screens[cs],
                 p->position.x, p->position.y);
        }
        if (p->suppress.enable &&
            p->position.y < p->suppress.box.bottom &&
            p->position.y >=  p->suppress.box.top &&
            p->position.x < p->suppress.box.right &&
            p->position.x >=  p->suppress.box.left) goto tbuttons;
        p->suppress.enable = 0;         /* trash box */
        if (EVROUND(queue, queue->tail + 1) == queue->head)
            goto tbuttons;

        /*
         * Put event into queue and do select
         */
        ev =(ws_event *)
            (((caddr_t)wi->events) + queue->tail * queue->event_size);
        ev->type = MOTION_TYPE;
        ev->time = queue->time = millis;
        ev->device_type = devices[wi->ws.console_pointer].device_type;
        ev->screen = cs;
        ev->device = wi->ws.console_pointer;
        ev->e.pointer.x = p->position.x;
        ev->e.pointer.y = p->position.y;
        ev->e.pointer.buttons = new_rep->state & 0x1e;
        queue->tail = EVROUND(queue, queue->tail + 1);
    }

    /*
     * See if tablet buttons have changed.
     */
 tbuttons:
    wi->new_switch = new_rep->state & 0x1e;
    wi->old_switch = last_rep->state & 0x1e;
    temp = wi->old_switch ^ wi->new_switch;
    if (temp) {

        /* define the changed button and if up or down */
        for (j = 1; j <= 0x10; j <<= 1) { /* check each button */
            if (!(j & temp))            /* did this button change? */
                continue;

            if ((i=EVROUND(queue, queue->tail + 1)) == queue->head)
                return(0);

            /* put event into queue and do select */
            ev =(ws_event *)
                (((caddr_t)wi->events) + queue->tail * queue->event_size);

            switch (j) {
             case T_RIGHT_BUTTON:
                ev->e.button.button = EVENT_T_RIGHT_BUTTON;
                break;
             case T_FRONT_BUTTON:
                ev->e.button.button = EVENT_T_FRONT_BUTTON;
                break;
             case T_BACK_BUTTON:
                ev->e.button.button = EVENT_T_BACK_BUTTON;
                break;
             case T_LEFT_BUTTON:
                ev->e.button.button = EVENT_T_LEFT_BUTTON;
                break;
            }
            if (wi->new_switch & j)
                ev->type = BUTTON_DOWN_TYPE;
            else
                ev->type = BUTTON_UP_TYPE;
            ev->device_type =
                devices[wi->ws.console_pointer].device_type;
            ev->time = queue->time = millis;
            ev->e.button.x = p->position.x;
            ev->e.button.y = p->position.y;
            ev->screen = p->position.screen;
            ev->device = wi->ws.console_pointer;
            queue->tail =  i;
	}

        /* update the last report */
        *last_rep = current_rep;
        p->mswitches = wi->new_switch;
    }
#endif /* FIXME */
}                                       /* Pick up tablet input */

void
pcxas_set_tablet_overhang(p, sp, oh)
    ws_pointer *p;
    ws_screen_descriptor *sp;
    unsigned int oh;
{
#ifdef DEBUG_PRINT_ENTRY
DPRINTF("pcxas_set_tablet_overhang: entry\n");
#endif /* DEBUG_PRINT_ENTRY */

#ifdef FIXME
    p->tablet_overhang = oh;
    pcxas_tablet_scale(p, sp);
#endif /* FIXME */
}

void
pcxas_tablet_scale(p, sp)
    ws_pointer *p;
    ws_screen_descriptor *sp;
{
#ifdef FIXME
    /*
     * want the scaling factor to cause slight "overhang" so that
     * we can move between screens.  so, compute scale factor such
     * that 0 <= (N * M) / S <= M+(2*O).
     *          N = hardware position report
     *          M = max screen coordinate
     *          S = what we're trying to compute here
     *          O = overhang (for each edge).
     */
    p->tablet_scale_x =
      (p->tablet_max_x * sp->width) / (sp->width+(p->tablet_overhang<<1));

    p->tablet_scale_y =
      (p->tablet_max_y * sp->height) / (sp->height+(p->tablet_overhang<<1));
#endif /* FIXME */
}

int
pcxas_intr(status, data)
	int status, data;
{
	ws_pointer_report *new_rep;
	
#ifdef DEBUG_PRINT_INTERRUPT
DPRINTF("pcxas_intr(0x%x, 0x%x)\n", status, data);
#endif /* DEBUG_PRINT_INTERRUPT */

	switch (mouse_state) 
	{
	case INIT_STATE:
	default:
		/* FIXME- always print? */
		ERR_PRINTF("pcxas_intr: mouse state %d illegal for (0x%x, 0x%x).\n",
		       mouse_state, status, data);

		/* fall thru to ignore data */

	case UNKNOWN_STATE:
	case DISABLED_STATE:
	case FIRST_DISABLE_STATE:
		return(0);	/* ignore data */

	case SECOND_ENABLE_STATE:
		if (data == PM_ACK)	/* swallow ACK *//* FIXME always? */
		{
			mouse_state = FIRST_ENABLE_STATE;
			return(0);
		}
		/* else fall through, it is data */

	case FIRST_ENABLE_STATE: /* do not check for ACK, should be data */
		mouse_state = ENABLED_STATE;
		/* fall thru to process data */

	case ENABLED_STATE:
		break;
	}		

	new_rep = &current_rep; /* mouse report pointer */

#ifdef DEBUG_PRINT_INFO_DISABLED
	DPRINTF("pcxas_intr: MOUSE data 0x%x  bytcnt %d\n",
		data, new_rep->bytcnt);
#endif /* DEBUG_PRINT_INFO */
	
	if (pointer_id == MOUSE_ID) { /* mouse report */
		++new_rep->bytcnt;  /* inc report byte count */
		if (new_rep->bytcnt == 1) {
			/* 1st byte of report */
			/* check for bit which *MUST* be on */
			/* if it's NOT on, try NEXT byte as state... */
			if (data & 0x08)
				new_rep->state = data; /* OK now */
			else
				new_rep->bytcnt = 0; /* resetting... */
		}
		else if (new_rep->bytcnt == 2)
			/* 2nd byte */
			new_rep->dx = data;
		else if (new_rep->bytcnt == 3) {
			/* 3rd byte - done, so send off*/
			new_rep->dy = data;
			new_rep->bytcnt = 0;
			/* 0x100 says MOUSE/TABLET (unit 1) */
#ifdef DEBUG_PRINT_MOUSE_REPORT
			pcxas_dump_mouse_report(new_rep);
#endif /* DEBUG_PRINT_MOUSE_REPORT */
			return(wskint(0x100));
		}
		return(0); /* interim bytes exit here */
	}
#ifdef TABLET_NOT_YET
	else { /* tablet report */
		++new_rep->bytcnt;  /* inc report byte count */
		if (new_rep->bytcnt == 1) {
			/* 1st byte of report */
			new_rep->state = data;
			if (new_rep->bytcnt > 1)
				/* start new frame */
				new_rep->bytcnt = 1;
		}
		else if (new_rep->bytcnt == 2)
			/* 2nd byte */
			new_rep->dx = data & 0x3f;
		else if (new_rep->bytcnt == 3)
			/* 3rd byte */
			new_rep->dx |= (data & 0x3f) << 6;
		else if (new_rep->bytcnt == 4)
			/* 4th byte */
			new_rep->dy = data & 0x3f;
		else if (new_rep->bytcnt == 5) {
			/* 5th byte - done, so send off */
			new_rep->dy |= (data & 0x3f) << 6;
			new_rep->bytcnt = 0;
			/* 0x100 says MOUSE/TABLET (unit 1) */
			return(wskint(0x100));
		}
		return(0); /* interim bytes exit here */
	} /* end tablet report */
#else /* TABLET_NOT_YET */
	
#ifdef DEBUG_PRINT_INFO
	DPRINTF("pcxas_intr: TABLET data unexpected...\n");
#endif /* DEBUG_PRINT_INFO */
	
#endif /* TABLET_NOT_YET */
}

/* FIXME FIXME - do-nothing routines */

caddr_t
pcxas_init_closure(address) 
	caddr_t address;
{
#ifdef DEBUG_PRINT_ENTRY
DPRINTF("pcxas_init_closure: entry\n");
#endif /* DEBUG_PRINT_ENTRY */

/* FIXME FIXME - does this need to return something??? */

	return(address);
}

int
pcxas_do_nothing_int()
{
#ifdef DEBUG_PRINT_ENTRY
DPRINTF("pcxas_do_nothing_int: entry\n");
#endif /* DEBUG_PRINT_ENTRY */

}

void
pcxas_do_nothing_void()
{
#ifdef DEBUG_PRINT_ENTRY
DPRINTF("pcxas_do_nothing_void: entry\n");
#endif /* DEBUG_PRINT_ENTRY */

}

/* end FIXME FIXME - do-nothing routines */

#ifdef MOUSE_DEBUG
/*
 * A debug routine called from the mouse init routine to test mouse
 * functionality.
 */
#define MOUSE_SETRESOLUT        0xE8
#define MOUSE_SET11SCALE        0xE6

int
mouse_debug_rtn()
{
        int data;

        DPRINTF("mouse_debug_rtn: routine entered.\n");

        /*
         * Tell mouse to SET PROMPT MODE.
         * See logitech spec page 2-14.  The successful answer (reply) to 
         * the mouse enable command is FA. 
         */
#define MOUSE_SETPROMPT 0xF0
#define MOUSE_SETSTREAM 0xEA

        gpc_ctl_cmd(PK_CTL_WRMOUSE);
        if ((data = gpc_output(MOUSE_SETPROMPT)) != 0xFA) {
                mouse_disable(); 
                DPRINTF("mouse_debug_rtn: failed SETPROMPT.\n");
                mouse_read_status();
                return(-1); 
        }
        else 
        {
                DPRINTF("mouse_debug_rtn: SETPROMPT success\n");
                mouse_read_status();
        }

        /*
         * use a "sequential" command to test for # of switches/firmware rev
         */
        gpc_ctl_cmd(PK_CTL_WRMOUSE);   
        if ((data = gpc_output(MOUSE_SETRESOLUT)) != 0xFA) {
                mouse_disable(); 
                DPRINTF("mouse_debug_rtn: failed to SETRESOLUT.\n");
                return(-1); 
        }
        gpc_ctl_cmd(PK_CTL_WRMOUSE);   
        if ((data = gpc_output(0)) != 0xFA) {
                mouse_disable(); 
                DPRINTF("mouse_debug_rtn: failed to SETRESOLUT data.\n");
                return(-1);
        }
        gpc_ctl_cmd(PK_CTL_WRMOUSE);   
        if ((data = gpc_output(MOUSE_SET11SCALE)) != 0xFA) {
                mouse_disable(); 
                DPRINTF("mouse_debug_rtn: failed first SET11SCALE.\n");
                return(-1);
        }
        gpc_ctl_cmd(PK_CTL_WRMOUSE);   
        if ((data = gpc_output(MOUSE_SET11SCALE)) != 0xFA) {
                mouse_disable(); 
                DPRINTF("mouse_debug_rtn: failed second SET11SCALE.\n");
                return(-1);
        }
        gpc_ctl_cmd(PK_CTL_WRMOUSE);   
        if ((data = gpc_output(MOUSE_SET11SCALE)) != 0xFA) {
                mouse_disable(); 
                DPRINTF("mouse_debug_rtn: failed third SET11SCALE.\n");
                return(-1); 
        }
        (void) mouse_read_status();

        /*
         * now spin until RMB is DOWN...
         */
        DPRINTF("mouse_debug_rtn: hold LMB DOWN to sample.\n");
        DPRINTF("mouse_debug_rtn: push RMB DOWN to exit.\n");
        do
        {
                data = mouse_read_report();
        } while (!(data & 2));

        /*
         * reset back to STREAM mode
         */
        gpc_ctl_cmd(PK_CTL_WRMOUSE);
        if ((data = gpc_output(MOUSE_SETSTREAM)) != 0xFA) {
                mouse_disable(); 
                DPRINTF("mouse_debug_rtn: failed to SETSTREAM.\n");
                return(-1);
        }
        else 
        {
                DPRINTF("mouse_debug_rtn: SETSTREAM success\n");
                mouse_read_status();
        }

        /*
         * reset back to ENABLED mode
         */
        gpc_ctl_cmd(PK_CTL_WRMOUSE);
        if ((data = gpc_output(MOUSE_ENABLE)) != 0xFA) {
                mouse_disable(); 
                DPRINTF("mouse_debug_rtn: failed to ENABLE.\n");
                return(-1);
        }
        else 
        {
                DPRINTF("mouse_debug_rtn: ENABLE success\n");
        }

        (void) mouse_read_status();
        return(0);
}

void mouse_read_status(void)
{
        int count, data, reply[3];
#define MOUSE_READSTATUS        0xE9

        gpc_ctl_cmd(PK_CTL_WRMOUSE);
        if ((data = gpc_output(MOUSE_READSTATUS)) != 0xFA) {
                mouse_disable(); 
                DPRINTF("mouse_read_status: improper reply to READSTATUS\n");
                return; 
        }
        count = 0;
        while (count < 3)
        {
                reply[count] = gpc_input();
                if (reply[count] < 0) {
                        DPRINTF("mouse_read_status: failed, count = %d.\n",
                               count);
                        mouse_disable(); 
                        return; 
                }
                else 
                        DPRINTF("mouse_read_status: reply[%d] = %x\n",
                               count, reply[count]);
                ++count;
        }
}

int
mouse_read_report(void)
{
        int count, data, reply[3];
#define MOUSE_READREPORT        0xEB

        gpc_ctl_cmd(PK_CTL_WRMOUSE);
        if ((data = gpc_output(MOUSE_READREPORT)) != 0xFA) {
                mouse_disable(); 
                DPRINTF("mouse_read_report: improper reply to READREPORT\n");
                return(-1);
        }
        count = 0;
        reply[0] = 0;
        while (count < 3)
        {
                reply[count] = gpc_input();
                if (reply[count] < 0) {
                        DPRINTF("mouse_read_report: failed, count = %d.\n",
                               count);
                        mouse_disable(); 
                        return(-1); 
                }
                else if (reply[0] & 1)
                        DPRINTF("mouse_read_report: reply[%d] = %x\n",
                               count, reply[count]);

                ++count;
        }
        return(reply[0]);
}

#endif /* MOUSE_DEBUG */

#ifdef DEBUG_PRINT_MOUSE_REPORT
pcxas_dump_mouse_report(new_rep)
	register ws_pointer_report *new_rep;
{
	printf("pcxas_dump_mouse_report: 0x%x 0x%x 0x%x\n",
		new_rep->state, new_rep->dx, new_rep->dy);
}
#endif /* DEBUG_PRINT_MOUSE_REPORT */
