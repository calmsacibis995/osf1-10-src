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
static char *rcsid = "@(#)$RCSfile: ssc.c,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/06/03 11:05:43 $";
#endif

/***********************************************************************
 *
 * Modification History:	ssc.c
 *
 * 4/30/90 - Randall Brown
 *	Use the values from the cpu switch for todrzero. 
 *
 * 3/29/90 - gmm
 *	changed splhigh() to splextreme() since now splhigh() same as 
 *	splclock()
 *
 * 3/6/90 - jaw
 *	fix ipl handling in console putchar routine.
 *
 * 3/9/89 - created by burns. To collect all ssc related code into one
 *	place and decouple it from vaxes.
 */


/*
 * Include files
 */
#include <sys/conf.h>
#include <sys/errno.h>
#include <sys/types.h>

#include <sys/user.h>
#include <sys/tty.h>
#include <machine/ssc.h>
#include <hal/clock.h>
#include <hal/cons.h>
#include <machine/cpu.h>
#include <hal/cpuconf.h>

#include <kern/sched_prim.h>
#include <sys/secdefines.h>
#ifdef	SEC_BASE
#include <sys/security.h>
#endif	SEC_BASE

/*
 * Globals - with default values
 */
struct ssc_regs *ssc_ptr =
		(struct ssc_regs *)PHYS_TO_K1(DEFAULT_SSC_PHYS);
int ssc_console_timeout = DEFAULT_SSC_TIMEOUT;


/*
 * SSC - console routines
 *
 * These routines are generic in that they work on many implementations that
 * use the SSC chip.
 */
/*
 * 
 */
int	ssc_cnstart();
struct tty	cons;
extern	int	ttrstrt();
extern	int	ttydef_open();
extern	int	ttydef_close();


ssc_cninit()
{
        queue_init(&cons.t_selq);               /* initialize select operations
                                                   queue */

#ifdef  (UNIX_LOCKS && (NCPUS > 1))             /* if t_lock is defined */

        lock_init(&cons.t_lock,TRUE,0);         /* initialize it */
#endif
}



ssc_cnopen(dev, flag)
    dev_t dev;
    unsigned int flag;
{
    register struct tty *tp;
    register int s;
    register int ld_return;
    register int clocal = TRUE;		/* console is a local connection */

    if (minor(dev) != 0) {
	return (ENXIO);
    }
    tp = &cons;
    TTY_LOCK(tp);
    tp->t_oproc = ssc_cnstart;

    if ((tp->t_state & TS_ISOPEN) == 0) {
        tp->t_line = TTYDISC;
        tty_def_open(tp,dev,flag,clocal);
        tp->t_state |= TS_CARR_ON;
    }

#ifdef  SEC_BASE
    if ((tp->t_state & TS_XCLUDE) && (!privileged(SEC_ALLOWDACACCESS,0))) {
#else
    if ((tp->t_state & TS_XCLUDE) && (u.u_uid != 0)) {
#endif  SEC_BASE
        TTY_UNLOCK(tp);
        return (EBUSY);
    }

    s = spltty();
    ssc_ptr->ssc_crcs |= RXCS_IE; /* enable receiver interrupts */
    splx(s);

    ld_return = (*linesw[tp->t_line].l_open)(dev, tp);
    TTY_UNLOCK(tp);
    return(ld_return);
}


ssc_cnclose(dev)
    dev_t dev;
{
    register struct tty *tp = &cons;

    if (minor(dev) != 0) {
	return (ENXIO);
    }

    TTY_LOCK(tp);
    (*linesw[tp->t_line].l_close)(tp);

    ssc_ptr->ssc_crcs &= ~RXCS_IE; /* disable receiver interrupts */
    ttyclose(tp);

    tty_def_close(tp);
    TTY_UNLOCK(tp);
}


ssc_cnread(dev, uio, flag)
    dev_t dev;
    struct uio *uio;
    int flag;
{
    register struct tty *tp = &cons;
    register int ld_return;

    if (minor(dev) != 0) {
	return (ENXIO);
    }
    TTY_LOCK(tp);
    ld_return = (*linesw[tp->t_line].l_read)(tp, uio, flag);
    TTY_UNLOCK(tp);
    return(ld_return);
}


ssc_cnwrite(dev, uio, flag)
    dev_t dev;
    struct uio *uio;
    int flag;
{
    register struct tty *tp = &cons;
    register int ld_return;

    if (minor(dev) != 0) {
	return (ENXIO);
    }

    TTY_LOCK(tp);
    ld_return = (*linesw[tp->t_line].l_write)(tp, uio);
    TTY_UNLOCK(tp);
    return(ld_return);
}


ssc_cnioctl(dev, cmd, addr, flag)
    dev_t dev;
    unsigned int cmd;
    caddr_t addr;
    unsigned int flag;
{
    register struct tty *tp = &cons;
    int error;

    if (minor(dev) != 0) {
	return (ENXIO);
    }

    TTY_LOCK(tp);
    error = (*linesw[tp->t_line].l_ioctl)(tp, cmd, addr);
    if (error >= 0) {
	TTY_UNLOCK(tp);
	return(error);
    }
    error = ttioctl(tp, cmd, addr, flag);
    if (error < 0) {
	error = ENOTTY;
    }
    TTY_UNLOCK(tp);
    return (error);
}


ssc_cnrint(dev)
    dev_t dev;
{
    register struct tty *tp = &cons;
    register int c;

    TTY_LOCK(tp);
    c = ssc_ptr->ssc_crdb;


    if (tp->t_state & TS_ISOPEN) {
        if (tp->t_iflag & ISTRIP) {
            c &= 0177;
        } else {
            c &= TTY_CHARMASK;

            /* If ISTRIP is not set a valid character of 377
             * is read as 0377,0377 to avoid ambiguity with
             * the PARMARK sequence.
             */
            if ((c == 0377) && (tp->t_line == TERMIODISC)) {
                (*linesw[tp->t_line].l_rint)(c, tp);
            }
        }
        (*linesw[tp->t_line].l_rint)(c, tp);
    }
    TTY_UNLOCK(tp);
}


ssc_cnxint(dev)
    dev_t dev;
{
    struct tty *tp = &cons;


    TTY_LOCK(tp);
    tp->t_state &= ~TS_BUSY;

    if (tp->t_line) {
	(*linesw[tp->t_line].l_start)(tp);
    } else {
	ssc_cnstart(tp);
    }
    if ((tp->t_state & TS_BUSY) == 0) {
	ssc_ptr->ssc_ctcs &= ~TXCS_IE; /* no chars to send diable transmitter intr */
    }
    TTY_UNLOCK(tp);
}


ssc_cnstart(tp)
    register struct tty *tp;
{
    register int c, s;

    if (tp->t_state & (TS_TIMEOUT|TS_BUSY|TS_TTSTOP)) {
        goto out;
    }
    if (tp->t_outq.c_cc <= tp->t_lowat) {
        if (tp->t_state & TS_ASLEEP) {
            tp->t_state &= ~TS_ASLEEP;
            wakeup((caddr_t)&tp->t_outq);
        }
        /* OSF select stuff */
        select_wakeup(&tp->t_selq);

    }
    if (tp->t_outq.c_cc == 0) {
        goto out;
    }

    s = spltty();

    c = getc(&tp->t_outq);

    if ((tp->t_cflag & CS8) != CS8) {
        c &= 0177; /* mask to 7 bits */
    } else {
        c &= TTY_CHARMASK;
    }
    tp->t_state |= TS_BUSY;
    ssc_ptr->ssc_ctdb = c;
    ssc_ptr->ssc_ctcs |= TXCS_IE;       /* let chip interrupt for next char */

    splx(s);

out:
        return;
}


ssc_cnputc(c)
    register int c;
{
    register int s, timo, savessc_ctcs;
    
    /* if spl is less then tty we must raise priority to block out
       tty interrupts.  If IPL is higher, we don't want to lower it */

    if (whatspl(getspl()) < SPLTTY) {
    	s = spltty(); 
    } else {
    	s = getspl();
    }
    timo = ssc_console_timeout;

    while ((ssc_ptr->ssc_ctcs&TXCS_RDY) == 0) {
	if (--timo == 0) {
	    break;
	}
    }

    if (c == 0) {
	splx(s);
	return;
    }

    savessc_ctcs = ssc_ptr->ssc_ctcs; /* save present state of control register */
    ssc_ptr->ssc_ctcs = 0;	   /* disable transmitter interrupts */
    ssc_ptr->ssc_ctdb = c & 0xff; /* output char to transmitter buffer */

    if (c == '\n') {		/* map carriage return - line feed */
	ssc_cnputc('\r');
    }
/*    ssc_cnputc(0);*/

    ssc_ptr->ssc_ctcs = savessc_ctcs; /* restore state of control register */

    splx(s);
}

ssc_cngetc()
{
	register int s, c;

	s = spl7();
	while ((ssc_ptr->ssc_crcs & RXCS_DONE) == 0)
		;
	c = (ssc_ptr->ssc_crdb) & 0xff;
	if (c == '\r')
		c = '\n';
	splx(s);
/*	cnputc(c);*/ 
	return (c);
}


/* taken from vax/machdep.c -- burns
 */

/*
 * ssc_readtodr - read ssc toy clock register.
 */
ssc_readtodr()
{
	return((int)ssc_ptr->ssc_toy);
}

/*
 * ssc_writetodr - write ssc toy clock register.
 */
ssc_writetodr(yrtime)
unsigned long yrtime;
{
        extern struct cpusw *cpup;

	ssc_ptr->ssc_toy = (cpup->todrzero + (100 * yrtime));
}
