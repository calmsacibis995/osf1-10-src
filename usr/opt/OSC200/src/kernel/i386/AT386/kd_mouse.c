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
static char	*sccsid = "@(#)$RCSfile: kd_mouse.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:09:21 $";
#endif 
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
 
/* **********************************************************************
 File:         kd_mouse.c
 Description:  mouse driver as part of keyboard/display driver

 Copyright Ing. C. Olivetti & C. S.p.A. 1989.
 All rights reserved.
********************************************************************** */
/*
  Copyright 1988, 1989 by Olivetti Advanced Technology Center, Inc.,
Cupertino, California.

		All Rights Reserved

  Permission to use, copy, modify, and distribute this software and
its documentation for any purpose and without fee is hereby
granted, provided that the above copyright notice appears in all
copies and that both the copyright notice and this permission notice
appear in supporting documentation, and that the name of Olivetti
not be used in advertising or publicity pertaining to distribution
of the software without specific, written prior permission.

  OLIVETTI DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS,
IN NO EVENT SHALL OLIVETTI BE LIABLE FOR ANY SPECIAL, INDIRECT, OR
CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
LOSS OF USE, DATA OR PROFITS, WHETHER IN ACTION OF CONTRACT,
NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUR OF OR IN CONNECTION
WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

/*
 * Hacked up support for serial mouse connected to COM1, using Mouse
 * Systems 5-byte protocol at 1200 baud.  This should work for
 * Mouse Systems, SummaMouse, and Logitek C7 mice.
 *
 * The interface provided by /dev/mouse is a series of events as
 * described in i386/AT386/kd.h.
 */

#include <mach/boolean.h>
#include <sys/types.h>
#include <sys/table.h>
#include <sys/file.h>
#include <sys/errno.h>
#include <kern/thread.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/kernel.h>
#include <sys/ioctl.h>
#include <sys/tty.h>
#include <sys/poll.h>
#include <i386/handler.h>
#include <i386/dispatcher.h>
#include <i386/ipl.h>
#include <i386/AT386/atbus.h>
#include <i386/AT386/kd.h>
#include <i386/AT386/kd_queue.h>
#include <i386/AT386/i8250.h>

static int (*oldvect)();		/* old interrupt vector */
static int oldunit;
extern	struct	isa_dev	*cominfo[];

static ihandler_t mouse_handler, *prev_mouse_handler;
static ihandler_id_t *mouse_handler_id;

#define MOUSEVECT	4

kd_event_queue mouse_queue;		/* queue of mouse events */
boolean_t mouse_in_use = FALSE;
struct queue_entry mouse_sel;           /* selecting process, if any */
short mousepgrp = 0;		/* process group leader when dev is open */

int mouseflag = 0;
#define MOUSE_COLL	1		/* select collision */
#define MOUSE_ASYNC	2		/* user wants asynch notification */
#define MOUSE_NBIO	4		/* user wants non-blocking I/O */

/*
 * The state of the 3 buttons is encoded in the low-order 3 bits (both
 * here and in other variables in the driver).
 */
u_char lastbuttons;		/* previous state of mouse buttons */
#define MOUSE_UP	1
#define MOUSE_DOWN	0
#define MOUSE_ALL_UP	0x7

int mouseintr();
void mouse_enqueue();

/*
 * init_mouse_hw - initialize the serial port.
 */
init_mouse_hw(base_addr)
{
        outb(base_addr + RIE, 0);
        outb(base_addr + RLC, LCDLAB);
        outb(base_addr + RDLSB, BCNT1200 & 0xff);
        outb(base_addr + RDMSB, (BCNT1200 >> 8) & 0xff);
        outb(base_addr + RLC, LC8);
        outb(base_addr + RMC, MCDTR | MCRTS | MCOUT2);
        outb(base_addr + RIE, IERD | IELS);
}


/*
 * mouseopen - Verify that the request is read-only, initialize,
 * and remember process group leader.
 */
/*ARGSUSED*/
mouseopen(dev, flags)
	dev_t dev;
	int flags;
{
        int unit = minor(dev);
        int mouse_pic = cominfo[unit]->dev_pic;
        caddr_t base_addr  = cominfo[unit]->dev_addr;
        int s;




	if (flags & FWRITE)
		return(ENODEV);

        if (mouse_in_use)
                return(EBUSY);

        s = splhi();            /* disable interrupts */

        kdq_reset(&mouse_queue);
        mouse_in_use = TRUE;            /* locking? */
        mousepgrp = u.u_procp->p_pgid;
        lastbuttons = MOUSE_ALL_UP;

	prev_mouse_handler = handler_override(NULL, mouse_pic);
	if (prev_mouse_handler == &mouse_handler)
		panic("Recursive mouse interrupt handler add");
	mouse_handler.ih_level = mouse_pic;
	mouse_handler.ih_handler = mouseintr;
	mouse_handler.ih_resolver = i386_resolver;
	mouse_handler.ih_rdev = cominfo[unit];
	mouse_handler.ih_stats.intr_type = INTR_DEVICE;
	mouse_handler.ih_stats.intr_cnt = 0;
	mouse_handler.ih_hparam[0].intparam = unit;
	if ((mouse_handler_id = handler_add(&mouse_handler)) != NULL)
		handler_enable(mouse_handler_id);
	else
		panic("Unable to add mouse interrupt handler");

        queue_init(&mouse_sel);
                /* XXX other arrays to init? */
        splx(s);                /* XXX - should come after init? */

        init_mouse_hw(base_addr);

        return(0);


}


/*
 * mouseclose - Disable interrupts on the serial port, reset driver flags, 
 * and restore the serial port interrupt vector.
 */
/*ARGSUSED*/
mouseclose(dev, flags)
	dev_t dev;
	int flags;
{
	int o_pri = splhi();		/* mutex with open() */
        int unit = minor(dev);
        int mouse_pic = cominfo[unit]->dev_pic;
        caddr_t base_addr  = cominfo[unit]->dev_addr;

	outb(base_addr + RIE, 0);	/* disable serial port */

	handler_disable(mouse_handler_id);
	if (handler_del(mouse_handler_id) == 0)
		handler_override(prev_mouse_handler, mouse_pic);
	else
		printf("Cannot restore mouse interrupt handler\n");
	mouse_handler_id = 0;
	prev_mouse_handler = 0;

	mousepgrp = 0;
	mouseflag = 0;
	mouse_in_use = FALSE;
	kdq_reset(&mouse_queue);		/* paranoia */
	(void)splx(o_pri);
}


/*
 * mouseioctl - handling for asynch & non-blocking I/O.
 */

/*ARGSUSED*/
mouseioctl(dev, cmd, data, flag)
	dev_t dev;
	int cmd;
	caddr_t data;
	int flag;
{
	int s = SPLKD();
	int err = 0;

	switch (cmd) {
	case FIONBIO:
		if (*(int *)data)
			mouseflag |= MOUSE_NBIO;
		else
			mouseflag &= ~MOUSE_NBIO;
		break;
	case FIOASYNC:
		if (*(int *)data)
			mouseflag |= MOUSE_ASYNC;
		else
			mouseflag &= ~MOUSE_ASYNC;
		break;
	default:
		err = ENOTTY;
		break;
	}

	splx(s);
	return(err);
}


/*
 * mouseselect - check for pending events, etc.
 */

/*ARGSUSED*/
mouseselect(dev, events, revents, scanning)
dev_t dev;
int scanning;
short *events, *revents;
{
    int s = SPLKD();

    if (*events & POLLNORM)
    {
	if (scanning)
	{
	    if (!kdq_empty(&mouse_queue))
	    {
		*revents |= POLLNORM;
		splx(s);
		return(0);
	    }
	    select_enqueue(&mouse_sel);
	}
	else
	    select_dequeue(&mouse_sel);
    }

    splx(s);
    return(0);
}



/*
 * mouseread - dequeue and return any queued events.
 */

/*ARGSUSED*/
mouseread(dev, uio)
	dev_t dev;
	struct uio *uio;
{
	int s = SPLKD();
	int err = 0;
	kd_event *ev;
	int i;
	char *cp;

	if (kdq_empty(&mouse_queue))
		if (mouseflag & MOUSE_NBIO) {
			err = EWOULDBLOCK;
			goto done;
		} else 
			while (kdq_empty(&mouse_queue)) {
				splx(s);
				if (err = tsleep((caddr_t)&mouse_queue, 
					TTIPRI | PCATCH, "mousein", 0))
					return (err);
				s = SPLKD();
			}

	while (!kdq_empty(&mouse_queue) && uio->uio_resid >= sizeof(kd_event)) {
		ev = kdq_get(&mouse_queue);
		for (cp = (char *)ev, i = 0; i < sizeof(kd_event);
		     ++i, ++cp) {
			err = ureadc(*cp, uio);
			if (err)
				goto done;
		}
	}

done:
	splx(s);
	return(err);
}


/*
 * mouseintr - Get a byte and pass it up for handling.  Called at SPLKD.
 */
mouseintr(unit)
{
	caddr_t base_addr = cominfo[unit]->dev_addr;
	unsigned char id, ls;

	/* get reason for interrupt and line status */
	id = inb(base_addr + RID);
	ls = inb(base_addr + RLS);

	/* handle status changes */
	if (id == IDLS) {
		if (ls & LSDR) {
			inb(base_addr + RDAT);	/* flush bad character */
		}
		return 1;			/* ignore status change */
	}

	if (id & IDRD) {
		mouse_handle_byte((u_char)(inb(base_addr + RDAT) & 0xff));
	}
	return 1;
}


/*
 * handle_byte - Accumulate bytes until we have an entire packet.
 * If the mouse has moved or any of the buttons have changed state (up
 * or down), enqueue the corresponding events.
 * Called at SPLKD.
 * XXX - magic numbers.
 */
mouse_handle_byte(ch)
	u_char ch;
{
#define MOUSEBUFSIZE	5		/* num bytes def'd by protocol */
	static u_char mousebuf[MOUSEBUFSIZE];	/* 5-byte packet from mouse */
	static short mbindex = 0;	/* index into mousebuf */
	u_char buttons, buttonchanges;
	struct mouse_motion moved;

	if (mbindex == 0 && ((ch & 0xf8) != 0x80))
		return;			/* not start of packet */
	mousebuf[mbindex++] = ch;
	if (mbindex < MOUSEBUFSIZE)
		return;
	
	/* got a packet */
	mbindex = 0;
	buttons = mousebuf[0] & 0x7;	/* get current state of buttons */
	buttonchanges = buttons ^ lastbuttons;
	moved.mm_deltaX = (char)mousebuf[1] + (char)mousebuf[3];
	moved.mm_deltaY = (char)mousebuf[2] + (char)mousebuf[4];

	if (moved.mm_deltaX != 0 || moved.mm_deltaY != 0)
		mouse_moved(moved);
	
	if (buttonchanges != 0) {
		lastbuttons = buttons;
		if (buttonchanges & 1)
			mouse_button(MOUSE_RIGHT, buttons & 1);
		if (buttonchanges & 2)
			mouse_button(MOUSE_MIDDLE, (buttons & 2) >> 1);
		if (buttonchanges & 4)
			mouse_button(MOUSE_LEFT, (buttons & 4) >> 2);
	}
}


/*
 * Enqueue a mouse-motion event.  Called at SPLKD.
 */
mouse_moved(where)
	struct mouse_motion where;
{
	kd_event ev;

	ev.type = MOUSE_MOTION;
	ev.time = time;
	ev.value.mmotion = where;
	mouse_enqueue(&ev);
}


/*
 * Enqueue an event for mouse button press or release.  Called at SPLKD.
 */
mouse_button(which, direction)
	kev_type which;
	u_char direction;
{
	kd_event ev;

	ev.type = which;
	ev.time = time;
	ev.value.up = (direction == MOUSE_UP) ? TRUE : FALSE;
	mouse_enqueue(&ev);
}


/*
 * mouse_enqueue - enqueue an event and wake up selecting processes, if
 * any.  Called at SPLKD.
 */

void
mouse_enqueue(ev)
	kd_event *ev;
{
	if (kdq_full(&mouse_queue))
		printf("mouse: queue full\n");
	else
		kdq_put(&mouse_queue, ev);

	select_wakeup(&mouse_sel);
	if (mouseflag & MOUSE_ASYNC)
		gsignal(mousepgrp, SIGIO);
	wakeup((caddr_t)&mouse_queue);
}
