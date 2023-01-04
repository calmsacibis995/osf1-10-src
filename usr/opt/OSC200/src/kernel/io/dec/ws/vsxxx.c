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
static char *rcsid = "@(#)$RCSfile: vsxxx.c,v $ $Revision: 1.1.4.6 $ (DEC) $Date: 1993/11/17 17:28:10 $";
#endif

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
 * 4 March 93  Jay Estabrook
 * 	Created by removing vsxxx-related stuff from ws_device.c
 *
 ************************************************************************/

#include <data/ws_data.c>
/* FIXME FIXME - should we get all these by "#include <data/vsxxx_data.c>"? */
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

#include  <sys/wsdevice.h>
#include  <sys/workstation.h>
#include  <sys/inputdevice.h>

#include  <io/dec/tc/xcons.h> 
#include  <io/dec/ws/vsxxx.h>
#include  <io/dec/ws/tablet.h>

extern  ws_info ws_softc[];

/* FIXME FIXME - should we get this via an "#include <data/vsxxx_data.c>"? */
extern  char vsxxxaa_name[];	/* mouse name */
extern  char vsxxxab_name[];	/* tablet name */

/* FIXME - should be able to remove this later, when most of
   "vsxxx_tablet_event" becomes common and migrates to ws_device.c... */
ws_screen_descriptor *ws_do_edge_work();

void vsxxx_set_tablet_overhang();
void vsxxx_tablet_scale();
/*
**  under conditions of heavy load, the mouse interrupt handler
**  (vsxxx_mouse_event()) switches screens well ahead of the
**  server's processing of the event queue. In those cases, some
**  cursor handling (particularly load_cursor()) to the 'old' screen
**  cause the cursor to hang. The flag (wi->mouse_screen) is updated
**  by vsxxx_mouse_event() to point to the current screen, to help
**  keep the action on the straight and narrow path... -bg
*/

/*
 * Routine to pre-process an hardware-dependent (VSXXX) mouse report into
 *  a "standardized" version for consumption by device-independent code.
 */
int
vsxxx_mouse_event(wi, queue, wsp, p, last_rep, new_rep, open)
	register ws_info *wi;
	register ws_event_queue *queue;
	ws_screens *wsp;
	register ws_pointer *p;
	register ws_pointer_report *new_rep;
	ws_pointer_report *last_rep;
	int open;
{
	ws_pointer_report temp_rep;
	
        ws_hardware_type *t = (ws_hardware_type *)(p->pc);

       /* First check for power up report.  If so, determine if
	  mouse or tablet and configure correctly  */
       if((new_rep->state & 0xa0) == 0xa0) {
         /* check for type of reset, mouse or tablet */
         if((new_rep->dx & 0x0f) == MOUSE_ID) {
           pointer_id = MOUSE_ID;
           p->hardware_type = MOUSE_DEVICE;
	   p->name = vsxxxaa_name;
	   if (t) t->buttons = 3; /* always (re)set to 3 */
         }
         else /* tablet */
         {
           pointer_id = TABLET_ID;
           p->hardware_type = TABLET_DEVICE;
	   p->name = vsxxxab_name;
           if (t) {
             switch ((new_rep->dy) & 0xff) {
              case 0x00:
                 t->buttons = 4;
                 break;
              case 0x11:
                 t->buttons = 2;
                 break;
              case 0x13:
                 printf("\nws: Tablet pointer not connected?\n");
              default:
                 printf("\nws: Tablet pointer unknown\n");
                 t->buttons = 4;
              }
            }
          }
          DELAY(500000);
          (*slu.mouse_putc)(INCREMENTAL);
          return;  /*Tablet/Mouse Reset finnished */
        }

	/*
	 * now build new "standardized" ws_pointer_report
	 */
	temp_rep.state = new_rep->state & 0x7 /* buttons are identical */
		| ((new_rep->state & VSXXX_X_SIGN)?0:WSPR_X_SIGN) /* sign rev */
		| ((new_rep->state & VSXXX_Y_SIGN)?WSPR_Y_SIGN:0);/* sign eq */
	temp_rep.dx = new_rep->dx;	/* dx already absolute */
	temp_rep.dy = new_rep->dy;	/* dy already absolute */
	temp_rep.bytcnt = 0;		/* just for consistency :-) */

	/*
	 * finally, actually process the "standardized" report
	 */
	return(ws_process_mouse_report(wi, queue, wsp, p, last_rep,
				       &temp_rep, open));
}

void vsxxx_set_tablet_overhang(p, sp, oh)
    ws_pointer *p;
    ws_screen_descriptor *sp;
    unsigned int oh;
{
    p->tablet_overhang = oh;
    vsxxx_tablet_scale(p, sp);
}

void vsxxx_tablet_scale(p, sp)
    ws_pointer *p;
    ws_screen_descriptor *sp;
{
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
}

int
vsxxx_tablet_event(wi, queue, p, last_rep, new_rep, screen, open)
	ws_info *wi;
	ws_event_queue *queue;
	ws_pointer *p;
	register ws_pointer_report *new_rep;
	ws_pointer_report *last_rep;
	int screen;
	int open;
{
    int cs = p->position.screen;        /* current screen */
    register ws_screen_descriptor *sp = screens[cs].sp;
    ws_motion_history *mhp;
    unsigned int millis = TOY;
    int x, y, edge = 0;
    register ws_event *ev;
    register char temp;
    register int i, j;
    ws_hardware_type *t = (ws_hardware_type *)(p->pc);

    /* First check for power up report.  If so, determine if
    mouse or tablet and configure correctly  */
    if((new_rep->state & 0xa0) == 0xa0) {
      /* check for type of reset, mouse or tablet */
      if((new_rep->dx & 0x0f) == MOUSE_ID) {
        pointer_id = MOUSE_ID;
        p->hardware_type = MOUSE_DEVICE;
	p->name = vsxxxaa_name;
	if (t) t->buttons = 3; /* always (re)set to 3 */
        }
      else /* tablet */
        {
        pointer_id = TABLET_ID;
        p->hardware_type = TABLET_DEVICE;
	p->name = vsxxxab_name;
        if (t) {
          switch ((new_rep->dy) & 0xff) {
           case 0x00:
              t->buttons = 4;
              break;
           case 0x11:
              t->buttons = 2;
              break;
           case 0x13:
              printf("\nws: Tablet pointer not connected?\n");
           default:
              printf("\nws: Tablet pointer unknown\n");
              t->buttons = 4;
            }
          }
        }
        DELAY(500000);
        (*slu.mouse_putc)(INCREMENTAL);
         return;  /*Tablet/Mouse Reset finnished */
     }

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
    **  identify the current active screen... -bg
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
#ifdef FIXME
        *last_rep = current_rep;
#else
        *last_rep = *new_rep;
#endif /* FIXME */
        p->mswitches = wi->new_switch;
    }
}                                       /* Pick up tablet input */

/*
 * Routine to initialize the mouse. 
 *
 * NOTE:
 *	This routine communicates with the mouse by directly
 *	manipulating the PMAX SLU registers. This is allowed
 *	ONLY because the mouse is initialized before the system
 *	is up far enough to need the SLU in interrupt mode.
 */
int 
vsxxx_mouse_init()
{
	register int	i;
	int	id_byte[4];
	int 	s;
	int pointer_id = 0;
	register ws_info *wi = &ws_softc[0];
        ws_screen_descriptor *sp = screens[0].sp;
        int NORESPONSE, retries;
	ws_pointer *p = (ws_pointer *)devices[wi->ws.console_pointer].p.pp;
	ws_hardware_type *t = (ws_hardware_type *)(p->pc);

/*	s = spltty();	*/

	/*
 	 * Set SLU line parameters for mouse communication.
 	 */
	(*slu.mouse_init)();

        NORESPONSE = TRUE;
        retries = 0;

        while(NORESPONSE && (retries <= 15)) {
          pointer_id = 0;
          (*slu.mouse_init)();
          (*slu.mouse_putc)(PROMPT);
          (*slu.mouse_putc)(SELF_TEST);
          /*
           * Pick up the four bytes returned by mouse or tablet self test.
           */
          for (i= 0; i < 4; i++) {
                id_byte[i] = (*slu.mouse_getc)();
                if (id_byte[i] < 0)  {
                   /* CAUTION: the 3 trailing newlines are needed! */
                    pointer_id = MOUSE_ID; /* Default */
                    goto OUT12;
                }
            }
OUT12:
          DELAY(50000)
          pointer_id = (id_byte[1] & 0x0f);
          if ((id_byte[0] & 0xff) == 0xa0) {
            if((id_byte[1] & 0x0f) == TABLET_ID) {
              pointer_id = TABLET_ID;             /* XXX */
              switch (id_byte[2] & 0xff) {
                 case 0x00:                     /* puck */
                    NORESPONSE=FALSE;
                    break;
                 case 0x11:                     /* stylus */
                    NORESPONSE = FALSE;
                    break;
                 case 0x13:                     /* nothing */
                    NORESPONSE = FALSE;
                    printf("\nws: Tablet pointer not connected?\n");
                 default: ;
                    printf("\nws: Tablet pointer unknown\n");
                 }
              }
            else {
               pointer_id = MOUSE_ID;
               NORESPONSE = FALSE;
              }
          }
        retries++;
        } /* while NORESPONSE */
        if (retries >= 12 ) printf("Mouse/Tablet has failed to reset.\n"); 
	/*
 	 * Set the operating mode
 	 *
 	 * We set the mode for both mouse and the tablet to 
	 * "Incremental stream mode".  XXX (some tablet use is absolute!!)
 	 */
	pointer_id = (id_byte[1] & 0x0f);

        if (pointer_id == TABLET_ID) {
            p->hardware_type = TABLET_DEVICE; /* XXX */
	    p->name = vsxxxab_name;
            if (t) {
                switch (id_byte[2] & 0xff) {
                 case 0x00:                     /* puck */
                    t->buttons = 4;
                    break;
                 case 0x11:                     /* stylus */
                    t->buttons = 2;
                    break;
                 case 0x13:                     /* nothing */
                    printf("\nws: Tablet pointer not connected?\n");
                 default:
                    printf("\nws: Tablet pointer unknown\n");
                    t->buttons = 0;             /* let server decide? */
                }
                t->hardware_type = TABLET_DEVICE;
            }
            vsxxx_tablet_scale(p, sp);
        }
        else if (pointer_id == MOUSE_ID) {
            p->hardware_type = MOUSE_DEVICE; /* XXX */
	    p->name = vsxxxaa_name;
            if (t) t->buttons = 3;
            vsxxx_tablet_scale(p, sp);  /* So the mouse and tablet can be exchanged */ 
	}
        else {
	    pointer_id = MOUSE_ID;
            p->hardware_type = MOUSE_DEVICE; /* XXX */
	    p->name = vsxxxaa_name;
            if (t) t->buttons = 3;
	    dprintf("\nws: Pointer device unknown: assuming 3-button mouse.\n");
	}

	(*slu.mouse_putc)(INCREMENTAL);

OUT:
/*	splx(s);	*/
	return(pointer_id);
}
