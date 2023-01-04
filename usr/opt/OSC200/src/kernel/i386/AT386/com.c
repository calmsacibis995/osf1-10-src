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
static char	*sccsid = "@(#)$RCSfile: com.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:07:59 $";
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

/* 
 *	Olivetti serial port driver v1.0
 *	Copyright Ing. C. Olivetti & C. S.p.A. 1988, 1989
 *	All rights reserved.
 *
 */ 
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
  NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
  WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
  */

#include <com.h>
#if NCOM > 0

#include <sys/param.h>
#include <sys/conf.h>
#include <ufs/dir.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/ioctl.h>
#include <sys/tty.h>
#include <sys/table.h>
#include <sys/systm.h>
#include <sys/uio.h>
#include <sys/file.h>
#include <sys/kernel.h>
#include <kern/queue.h>
#include <sys/termio.h>

#include <i386/ipl.h>
#include <i386/AT386/atbus.h>
#include <i386/AT386/comreg.h>
#include <i386/handler.h>
#include <i386/dispatcher.h>

#define COMIDEBUG

extern void 	splx();
extern int	spltty();
extern void 	timeout();
extern void 	ttrstrt();

/* 
 * Driver information for auto-configuration stuff.
 */

int 	comprobe(), comattach(), comintr(), comstart(), comstop(), comparam();

int (*comintrs[])() = {comintr, 0};
struct isa_driver comdriver = {comprobe, 0, comattach, "com", 0, 0, 0};
struct isa_dev *cominfo[NCOM];

#ifndef	PORTSELECTOR
#define ISPEED	B9600
#define IFLAGS	(EVENP|ODDP|ECHO|CRMOD)
#else
#define ISPEED	B4800
#define IFLAGS	(EVENP|ODDP)
#endif

struct tty	com_tty[NCOM];

struct speedtab com_speeds[] =
{
        B0,     0,
        B50,    0x900,
        B75,    0x600,
        B110,   0x417,
        B134,   0x359,
        B150,   0x300,
        B300,   0x180,
        B600,   0x0c0,
        B1200,  0x060,
        B1800,  0x040,
        B2400,  0x030,
        B4800,  0x018,
        B9600,  0x00c,
        B19200, 0x006,
        B38400, 0x003,
        -1,     -1
};

int     initstate[NCOM];
uchar	comsoftCAR[NCOM] = {0};

int comprobe(dev)
struct isa_dev *dev;
{
	caddr_t	spot = dev->dev_addr;
	
	outb(INTR_ENAB(spot), 0);

	return !inb(INTR_ENAB(spot));
}

static ihandler_t com_handler[NCOM];
static ihandler_id_t *com_handler_id[NCOM];

int comattach(dev)
struct isa_dev *dev;
{
	u_char		unit = dev->dev_unit;
	struct	tty	*tp = &com_tty[unit];
	caddr_t		addr = 	dev->dev_addr;
	register ihandler_t *chp = &com_handler[unit];;

	cominfo[unit] = dev;
	
	chp->ih_level = dev->dev_pic;
	chp->ih_handler = dev->dev_intr[0];
	chp->ih_resolver = i386_resolver;
	chp->ih_rdev = dev;
	chp->ih_stats.intr_type = INTR_DEVICE;
	chp->ih_stats.intr_cnt = 0;
	chp->ih_hparam[0].intparam = unit;
	if ((com_handler_id[unit] = handler_add(chp)) != NULL)
		handler_enable(com_handler_id[unit]);
	else
		panic("Unable to add com interrupt handler");

	tp->t_state = 0;
	tp->t_dev = unit;
	
	outb(INTR_ENAB(addr), 0);
	outb(MODEM_CTL(addr), 0);
	initstate[unit] = iOUT2;
	printf("com%d: (DOS COM%d) irq = %d\n", unit, unit+1, dev->dev_pic);

	while (!(inb(INTR_ID(addr)) & 1))		/* suck out chaff */
		;
}

int comopen(dev, flag)
int dev;
int flag;
{
	int 		unit = minor(dev);
	struct isa_dev *isai;
	struct	tty	*tp;
	caddr_t		addr;
	int		s;
	int		error = 0;

	if (unit >= NCOM || (isai = cominfo[unit]) == 0 || isai->dev_alive == 0)
		return(ENXIO);
	tp = &com_tty[unit];
	if (tp->t_state & TS_XCLUDE && u.u_uid != 0)
		return(EBUSY);
	addr = isai->dev_addr;
	tp->t_addr = addr;
	tp->t_oproc = comstart;
        tp->t_param = comparam;

	if ((tp->t_state & TS_ISOPEN) == 0) {
		queue_init(&tp->t_selq);
		ttychars(tp);
		tp->t_iflag = TTYDEF_IFLAG;
		tp->t_oflag = TTYDEF_OFLAG;
		tp->t_lflag = TTYDEF_LFLAG;
		tp->t_cflag = CS8|CREAD|HUPCL;
		tp->t_line = 0;
		tp->t_ispeed = tp->t_ospeed = ISPEED;
		ttsetwater(tp);
		(void)comparam(tp, &tp->t_termios);
	}
	s = spltty();
	/* Get initial stat of carrier */
	if (comsoftCAR[unit] || (inb(MODEM_STAT(isai->dev_addr)) & iDCD))
		tp->t_state |= TS_CARR_ON;
	else
		tp->t_state &= ~TS_CARR_ON;
	while (!(tp->t_state&TS_CARR_ON) && !(tp->t_cflag&CLOCAL) &&
	    !(flag & (O_NDELAY|O_NONBLOCK))) {
		tp->t_state |= TS_WOPEN;
		if (error = ttysleep(tp, (caddr_t)&tp->t_rawq,
					TTIPRI | PCATCH, ttopen))
			break;
	}
	splx(s);
	if (error)
		return (error);
	return ((*linesw[tp->t_line].l_open)(dev, tp, flag));
}

int comclose(dev, flag)
int dev;
int flag;
{
	int 		unit = minor(dev);
	struct	tty	*tp = &com_tty[unit];
	caddr_t		addr = 	cominfo[unit]->dev_addr;

	(*linesw[tp->t_line].l_close)(tp);
	if ((tp->t_cflag&HUPCL) || (tp->t_state&TS_ISOPEN)==0) { 
		outb(INTR_ENAB(addr), 0);
		outb(MODEM_CTL(addr), iOUT2);
		tp->t_state &= ~TS_BUSY;
	} 
	ttyclose(tp);
	return 0;
}

int comread(dev, uio, flag)
int dev;
struct uio *uio;
int flag;
{
	struct tty *tp= &com_tty[minor(dev)];

	return ((*linesw[tp->t_line].l_read)(tp, uio,flag));
}

int comwrite(dev, uio,flag)
int dev;
struct uio *uio;
int flag;
{
	struct tty *tp= &com_tty[minor(dev)];

	return ((*linesw[tp->t_line].l_write)(tp, uio, flag));
}

static char combrkstr[] = "combrk";

int comioctl(dev, cmd, data, flag)
	int dev;
	int cmd;
	caddr_t data;
	int flag;
{
	int s, error;
	int unit = minor(dev);
	struct tty *tp = &com_tty[unit];
	caddr_t	dev_addr = cominfo[unit]->dev_addr;
	int timo;
	
	error = (*linesw[tp->t_line].l_ioctl)(tp, cmd, data, flag);
	if (error >= 0)
		return(error);
	error = ttioctl(tp, cmd, data, flag);
	if (error >= 0)
		return(error);

	s = spltty();
	switch (cmd) {
        case TCSBREAK:
		/* Wait for one character time before sending break */
		timo = (10*hz)/tp->t_ospeed;
		if (timo < 2)
			timo = 2;
		mpsleep((caddr_t)&com_tty[unit], PZERO-10, combrkstr, timo,
			NULL, 0);
		outb(LINE_CTL(dev_addr), inb(LINE_CTL(dev_addr)) | iSETBREAK);
		mpsleep((caddr_t)&com_tty[unit], PZERO-10, combrkstr,
			hz/4, NULL, 0);
		outb(LINE_CTL(dev_addr), inb(LINE_CTL(dev_addr)) & ~iSETBREAK);
                break;
                
	case TIOCSBRK:
		outb(LINE_CTL(dev_addr), inb(LINE_CTL(dev_addr)) | iSETBREAK);
		break;

	case TIOCCBRK:
		outb(LINE_CTL(dev_addr), inb(LINE_CTL(dev_addr)) & ~iSETBREAK);
		break;

	case TIOCSDTR:
		outb(MODEM_CTL(dev_addr), iOUT2|iDTR|iRTS);
		break;

	case TIOCCDTR:
		outb(MODEM_CTL(dev_addr), iOUT2);
		break;

	case TIOOFCARR:
		initstate[unit] = 0;
		comsoftCAR[unit] = 0;
		break;

	case TIOONCARR:
		comsoftCAR[unit] = 1;
		initstate[unit] = initstate[unit]|(iDTR|iRTS|iOUT2);
		outb(MODEM_CTL(dev_addr), iDTR|iRTS|iOUT2);
		break;

	case TIOCMSET:
	case TIOCMBIS:
	case TIOCMBIC:
	case TIOCMGET:
		uprintf("modem control not yet implemented\n");
	default:
		splx(s);
		return(ENOTTY);
	}
	splx(s);
	return(0);
}

int comintr(unit)
int unit;
{
	register struct tty *tp = &com_tty[unit];
	register u_char interrupt_id;
	struct isa_dev *isai = cominfo[unit];
	caddr_t		addr = isai->dev_addr;
	int		intr = 0;
	
#ifdef	DEBUG
	{int modem = inb(MODEM_CTL(addr));
		if (! (modem & iOUT2)) {
			printf("comintr: OUT2\n");
			outb(MODEM_CTL(addr), s|iOUT2);
		}
	}
#endif	DEBUG
	/*
	 * Com interrupts are prioritized, in the order
	 * (from highest to lowest):
	 *
	 *	line status
	 *	receive buffer full
	 *	transmit buffer empty
	 *	modem status
	 *
	 * On each interrupt, we poll for all conditions that are
	 * at or below the priority of the interrupting condition.
	 */
	while(!((interrupt_id = inb(INTR_ID(addr))) & 1)) {
		++intr;
		/*
		 * Do not change the order of the cases in
		 * the following switch statement!
		 */
		switch (interrupt_id >> 1) {
		case iLSR_INTR:
		case iRBF_INTR:
			comrbf(tp, isai);
			/* and fall into ... */
		case iXBE_INTR:
			comxbe(tp, isai);
			/* and fall into ... */
		case iMDM_INTR:
			commdm(tp, isai);
			break;
		default:
			printf("com%d: unknown com interrupt %d\n", unit,
				interrupt_id>>1);
			break;
		}
	}
	return intr;
}

void comxbe(tp, isai)
register struct tty *tp;
struct isa_dev *isai;
{
	register int lstatus;
	register caddr_t addr = isai->dev_addr;
#ifdef COMIDEBUG
	extern int handler_trace;

	if (handler_trace == 3 || handler_trace == 4)
		printf("comxbe(%x)\n", isai->dev_addr);
#endif
	lstatus = inb(LINE_STAT(addr));
	if (!(tp->t_state & TS_BUSY) || !(lstatus&iTHRE))
		return;
	tp->t_state &= ~TS_BUSY;
	if (tp->t_state&TS_FLUSH)
		tp->t_state &= ~TS_FLUSH;
	(*linesw[tp->t_line].l_start)(tp);
}

int comoverrun = 0;

void comrbf(tp, isai)
	register struct tty *tp;
	struct isa_dev *isai;
{
	int line;
	register int c;
#ifdef COMIDEBUG
	extern int handler_trace;

	if (handler_trace == 3 || handler_trace == 4)
		printf("comrbf(%x)\n", isai->dev_addr);
#endif
	line = inb(LINE_STAT(isai->dev_addr));
	c = inb(TXRX(isai->dev_addr)) & TTY_CHARMASK;

	if ((tp->t_state&TS_ISOPEN)==0) {
		wakeup((caddr_t)&tp->t_rawq);
#ifdef	PORTSELECTOR
		if ((tp->t_state&TS_WOPEN) == 0)
#endif
			return;
	}
	/*
	 * Controller doesn't provide a way to disable receiver,
	 * so we fake it when CREAD flag is off by discarding
	 * all characters and errors.
	 */
	if (!(tp->t_cflag & CREAD))
		return;

	if (line & iPE)
		c |= TTY_PE;

	if (line & iOR) {
		if (comoverrun == 0)
			printf("com%d: overrun\n", tp->t_dev);
		comoverrun++;
	}

	if (line & (iFE | iBRKINTR))
		c |= TTY_FE;
#if	NBK > 0
	if (tp->t_line == NETLDISC) {
		c &= 0177;
		BKINPUT(c, tp);
	} else
#endif
	(*linesw[tp->t_line].l_rint)(c, tp);
}

void commdm(tp, isai)
register struct tty *tp;
struct isa_dev *isai;
{
	int ret, flag, status;
	register caddr_t addr = isai->dev_addr;

	status = inb(MODEM_STAT(addr));
	flag = (status & iDCD) != 0;
	if (status & iDDCD) {
		ret = (*linesw[tp->t_line].l_modem)(tp, flag);
		if (!flag && !ret) {
			outb(MODEM_CTL(addr), iOUT2);
			tp->t_state &= ~TS_BUSY;
		}
	}
}

comparam(tp, t)
	register struct tty *tp;
	register struct termios *t;
{
        int unit = minor(tp->t_dev);
	caddr_t addr = (caddr_t)tp->t_addr;
	int lpar;
	int s;
	int ospeed = ttspeedtab(t->c_ospeed, com_speeds);

	/*
	 * Block interrupts so parameters will be set
	 * before the line interrupts.
	 */
	s = spltty();

        /* check requested parameters */
        if (ospeed < 0 || (ospeed && t->c_ispeed && t->c_ispeed != t->c_ospeed)) {
		splx(s);
                return(EINVAL);
        }

        /* and copy to tty */
        tp->t_ispeed = t->c_ispeed;
        tp->t_ospeed = t->c_ospeed;
        tp->t_cflag = t->c_cflag;

	if (ospeed == 0) {
		tp->t_cflag |= HUPCL;
		outb(MODEM_CTL(addr), iOUT2);
		splx(s);
		return(0);
	}
	
	outb(LINE_CTL(addr), iDLAB);
	outb(BAUD_LSB(addr), (u_short)ospeed & 0xff);
	outb(BAUD_MSB(addr), (u_short)ospeed >>8);
	
	/*
	 * Set device registers according to the specifications of the
	 * termios structure.
	 */
	switch (tp->t_cflag & CSIZE) {
	case CS5:
		lpar = i5BITS;
		break;
	case CS6:
		lpar = i6BITS;
		break;
	case CS7:
		lpar = i7BITS;
		break;
	case CS8:
		lpar = i8BITS;
		break;
	default:
		lpar = i8BITS;
		break;
	}
	if ((tp->t_cflag & CSTOPB) || tp->t_ispeed == B110 /* !! */)
		lpar |= iSTB;

	if (tp->t_cflag & PARENB) {
		lpar |= iPEN;
		if ((tp->t_cflag & PARODD) == 0)
			lpar |= iEPS;
	}
	
	outb(LINE_CTL(addr), lpar);
	outb(INTR_ENAB(addr), iRX_ENAB | iTX_ENAB | iERROR_ENAB | iMODEM_ENAB);
	outb(MODEM_CTL(addr), iDTR|iRTS|iOUT2);
	/*
	 * Enabling interrupts when they were previously disabled
	 * will immediately cause a transmit buffer empty interrupt
	 * to be posted.
	 */
	comintr(unit);
	splx(s);
        return(0);
}

int comstart(tp)
struct tty *tp;
{
	int s = spltty();
	caddr_t addr = (caddr_t)tp->t_addr;
	register int cc;

	if (tp->t_state & (TS_TIMEOUT|TS_TTSTOP|TS_BUSY))
		goto out;

#ifdef	DEBUG
	{int modem = inb(MODEM_CTL(addr));
		if (! (modem & iOUT2)) {
			printf("comstart: OUT2\n");
			outb(MODEM_CTL(addr), s|iOUT2);
		}
	}
#endif	DEBUG
	if (tp->t_outq.c_cc <= tp->t_lowat) {
		if (tp->t_state & TS_ASLEEP) {
			tp->t_state &= ~TS_ASLEEP;
			wakeup ((caddr_t)&tp->t_outq);
		}
		select_wakeup(&tp->t_selq);
	}
	if (tp->t_outq.c_cc == 0)
		goto out;

	cc = ndqb(&tp->t_outq, 1);
	if (cc == 0) {
		cc = getc(&tp->t_outq);
		timeout(ttrstrt, (caddr_t)tp, (cc&0x7f) + 6);
		tp->t_state |= TS_TIMEOUT;
		goto out;
	}
	cc = getc(&tp->t_outq);
	tp->t_state |= TS_BUSY;
	outb(TXRX(addr),  cc);
out:
	splx(s);
}

int comstop(tp, flag)
struct tty *tp;
{
	int s = spltty();

	if (tp->t_state & TS_BUSY) {
		if ((tp->t_state&TS_TTSTOP)==0)
			tp->t_state |= TS_FLUSH;
	}
	splx(s);
}

compr(unit)
{
	compr_addr(cominfo[unit]->dev_addr);
	return 0;
}

compr_addr(addr)
{
	printf("TXRX(%x) %x, INTR_ENAB(%x) %x, INTR_ID(%x) %x, LINE_CTL(%x) %x,\n\
MODEM_CTL(%x) %x, LINE_STAT(%x) %x, MODEM_STAT(%x) %x\n",
		TXRX(addr), inb(TXRX(addr)),
		INTR_ENAB(addr), inb(INTR_ENAB(addr)),
		INTR_ID(addr), inb(INTR_ID(addr)),
		LINE_CTL(addr), inb(LINE_CTL(addr)),
		MODEM_CTL(addr), inb(MODEM_CTL(addr)),
		LINE_STAT(addr), inb(LINE_STAT(addr)),
		MODEM_STAT(addr), inb(MODEM_STAT(addr)));
}
#endif NCOM
