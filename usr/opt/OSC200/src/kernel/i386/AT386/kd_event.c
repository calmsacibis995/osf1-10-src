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
static char	*sccsid = "@(#)$RCSfile: kd_event.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:09:14 $";
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
 File:         kd_event.c
 Description:  Driver for event interface to keyboard.


 Copyright Ing. C. Olivetti & C. S.p.A. 1989.  All rights reserved.
********************************************************************** */

#include <mach/boolean.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/errno.h>
#include <kern/thread.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/kernel.h>
#include <sys/ioctl.h>
#include <sys/tty.h>
#include <sys/poll.h>
#include <i386/AT386/kd.h>
#include <i386/AT386/kd_queue.h>

/*
 * Code for /dev/kbd.   The interrupt processing is done in kd.c,
 * which calls into this module to enqueue scancode events when
 * the keyboard is in Event mode.
 */

/*
 * Note: These globals are protected by raising the interrupt level
 * via SPLKD.
 */

kd_event_queue kbd_queue;		/* queue of keyboard events */
struct queue_entry kbd_sel;           /* selecting process, if any */
short kbdpgrp = 0;		/* process group leader when dev is open */

int kbdflag = 0;
#define KBD_COLL	1		/* select collision */
#define KBD_ASYNC	2		/* user wants asynch notification */
#define KBD_NBIO	4		/* user wants non-blocking I/O */


void kbd_enqueue();

static boolean_t initialized = FALSE;


/*
 * kbdinit - set up event queue.
 */

kbdinit()
{
	int s = SPLKD();
	
	if (!initialized) {
		kdq_reset(&kbd_queue);
		queue_init(&kbd_sel);
		initialized = TRUE;
	}
	splx(s);
}


/*
 * kbdopen - Verify that open is read-only and remember process
 * group leader.
 */

/*ARGSUSED*/
kbdopen(dev, flags)
	dev_t dev;
	int flags;
{
	kbdinit();

	if (flags & FWRITE)
		return(ENODEV);
	
	if (kbdpgrp == 0)
		kbdpgrp = u.u_procp->p_pgid;
	return(0);
}


/*
 * kbdclose - Make sure that the kd driver is in Ascii mode and
 * reset various flags.
 */

/*ARGSUSED*/
kbdclose(dev, flags)
	dev_t dev;
	int flags;
{
	int s = SPLKD();

	kb_mode = KB_ASCII;
	kbdpgrp = 0;
	kbdflag = 0;
	kdq_reset(&kbd_queue);
	queue_init(&kbd_sel);
	splx(s);
}


/*
 * kbdioctl - handling for asynch & non-blocking I/O.
 */

/*ARGSUSED*/
kbdioctl(dev, cmd, data, flag)
	dev_t dev;
	int cmd;
	caddr_t data;
	int flag;
{
	int s = SPLKD();
	int err = 0;

	switch (cmd) {
	case KDSKBDMODE:
		kb_mode = *(int *)data;
		/* XXX - what to do about unread events? */
		/* XXX - should check that "data" contains an OK value */
		break;
	case KDGKBDTYPE:
		*(int *)data = KB_VANILLAKB;
		break;
	case K_X_KDB_ENTER:
		err = X_kdb_enter_init((struct X_kdb *) data);
		break;
	case K_X_KDB_EXIT:
		err = X_kdb_exit_init( (struct X_kdb *) data);
		break;
	case FIONBIO:
		if (*(int *)data)
			kbdflag |= KBD_NBIO;
		else
			kbdflag &= ~KBD_NBIO;
		break;
	case FIOASYNC:
		if (*(int *)data)
			kbdflag |= KBD_ASYNC;
		else
			kbdflag &= ~KBD_ASYNC;
		break;
	default:
		err = ENOTTY;
		break;
	}

	splx(s);
	return(err);
}


/*
 * kbdselect
 */

/*ARGSUSED*/
kbdselect(dev, events, revents, scanning)
dev_t dev;
int scanning;
short *events, *revents;
{
    int s = SPLKD();

    if (*events & POLLNORM)
    {
	if (scanning)
	{
	    if (!kdq_empty(&kbd_queue))
	    {
		*revents |= POLLNORM;
		splx(s);
		return(0);
	    }
	    select_enqueue(&kbd_sel);
	}
	else
	    select_dequeue(&kbd_sel);
    }

    splx(s);
    return(0);

}


/*
 * kbdread - dequeue and return any queued events.
 */

/*ARGSUSED*/
kbdread(dev, uio)
	dev_t dev;
	struct uio *uio;
{
	int s = SPLKD();
	int err = 0;
	kd_event *ev;
	int i;
	char *cp;

	if (kdq_empty(&kbd_queue))
		if (kbdflag & KBD_NBIO) {
			err = EWOULDBLOCK;
			goto done;
		} else 
			while (kdq_empty(&kbd_queue)) {
				splx(s);
				if (err = tsleep((caddr_t)&kbd_queue, 
					TTIPRI | PCATCH, ttyin, 0))
					return (err);
				s = SPLKD();
			}

	while (!kdq_empty(&kbd_queue) && uio->uio_resid >= sizeof(kd_event)) {
		ev = kdq_get(&kbd_queue);
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
 * kd_enqsc - enqueue a scancode.  Should be called at SPLKD.
 */

void
kd_enqsc(sc)
	Scancode sc;
{
	kd_event ev;

	ev.type = KEYBD_EVENT;
	ev.time = time;
	ev.value.sc = sc;
	kbd_enqueue(&ev);
}


/*
 * kbd_enqueue - enqueue an event and wake up selecting processes, if
 * any.  Should be called at SPLKD.
 */

void
kbd_enqueue(ev)
	kd_event *ev;
{
	if (kdq_full(&kbd_queue))
		printf("kbd: queue full\n");
	else
		kdq_put(&kbd_queue, ev);
	select_wakeup(&kbd_sel);
	if (kbdflag & KBD_ASYNC)
		gsignal(kbdpgrp, SIGIO);
	wakeup((caddr_t)&kbd_queue);
}


u_int X_kdb_enter_str[512], X_kdb_exit_str[512];
int   X_kdb_enter_len = 0,  X_kdb_exit_len = 0;

kdb_in_out(p)
u_int *p;
{
register int t = p[0];

	switch (t & K_X_TYPE) {
		case K_X_IN|K_X_BYTE:
			inb(t & K_X_PORT);
			break;

		case K_X_IN|K_X_WORD:
			inw(t & K_X_PORT);
			break;

		case K_X_IN|K_X_LONG:
			inl(t & K_X_PORT);
			break;

		case K_X_OUT|K_X_BYTE:
			outb(t & K_X_PORT, p[1]);
			break;

		case K_X_OUT|K_X_WORD:
			outw(t & K_X_PORT, p[1]);
			break;

		case K_X_OUT|K_X_LONG:
			outl(t & K_X_PORT, p[1]);
			break;
	}
}

X_kdb_enter()
{
register u_int *u_ip, *endp;

/*	if (kb_mode == KB_EVENT)*/
		for (u_ip = X_kdb_enter_str, endp = &X_kdb_enter_str[X_kdb_enter_len];
		     u_ip < endp;
		     u_ip += 2)
		/* outb( port,   value); */
		    kdb_in_out(u_ip);
}

X_kdb_exit()
{
register u_int *u_ip, *endp;

/*	if (kb_mode == KB_EVENT)*/
		for (u_ip = X_kdb_exit_str, endp = &X_kdb_exit_str[X_kdb_exit_len];
		     u_ip < endp;
		     u_ip += 2)
		/* outb( port,   value); */
		   kdb_in_out(u_ip);
}

X_kdb_enter_init(kp)
struct X_kdb *kp;
{

	int error = 0;

/*	if (kb_mode != KB_EVENT)
		error = EPERM;
	else 
*/	if (kp->size > sizeof X_kdb_enter_str)
		error = ENOENT;
	else if(copyin(kp->ptr, X_kdb_enter_str, kp->size) == EFAULT)
		error = EFAULT;

	X_kdb_enter_len = kp->size>>2;
	return (error);
}

X_kdb_exit_init(kp)
struct X_kdb *kp;
{

	int error = 0;

/*	if (kb_mode != KB_EVENT)
		error = EPERM;
	else 
*/	if (kp->size > sizeof X_kdb_exit_str)
		error = ENOENT;
	else if(copyin(kp->ptr, X_kdb_exit_str, kp->size) == EFAULT)
		error = EFAULT;

	X_kdb_exit_len = kp->size>>2;
	return (error);
}
