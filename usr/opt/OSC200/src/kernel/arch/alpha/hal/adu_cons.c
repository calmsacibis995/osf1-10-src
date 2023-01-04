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
static char *rcsid = "@(#)$RCSfile: adu_cons.c,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/06/14 18:24:34 $";
#endif
#ifndef lint
static char	*sccsid = "@(#)adu_cons.c	9.1	(ULTRIX/OSF)	10/21/91";
#endif lint

/************************************************************************
 *									*
 *			Copyright (c) 1991 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any	other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or	reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

/*
 * Modification History: /sys/machine/alpha/adu_cons.c
 *
 * 08-Aug-91 -- afd
 *      Changed driver to use the new controller and driver structs
 *
 * 07-Jan-91 -- Tim Burke
 *      Removed all pdma related code.
 *
 * 30-Nov-90 -- Tim Burke
 *      Changed to use the new register map scheme.
 *
 * 05-Oct-90 -- map
 *	Created this file for Alpha adu console support.
 */

/* 	TO DO !!!!!!!!!!!!			
 *
 *      This driver is presently setup to only utilize serial line #1 and
 *      does not provide any access to serial line #2 which has separate
 *      base, interrupt and doorbell registers.
 */

/*
 * Include files
 */
#include <sys/param.h>
#include <sys/conf.h>
#include <sys/time.h>
#include <sys/errno.h>
#include <sys/systm.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/user.h>
#include <sys/buf.h>
#include <sys/tty.h>
#include <sys/vmmac.h>
#include <machine/clock.h>
#include <machine/cpu.h>
#include <io/dec/uba/ubavar.h>	/* auto-config headers */
#include <hal/adudefs.h>
#include <vm/vm_kern.h>
#include <io/common/devdriver.h>

#define	RXSIZE 4064
#define	TXSIZE 4064

/*
 * Globals - with default values
 */
extern int adu_cnxint();
extern int adu_cnrint();

/*
 * This variable must be setup prior to calling the console init routine.  It
 * must be setup to be the base address (BB) of the IO module's TVBus registers.
 */
extern u_long *adu_tv_cons_base; /* base addr of regs on console I/O module */

/*
 * This array contains the offsets from the base (BB) register of the 
 * console related registers.
 */
struct adu_consregs adu_cons_regs = {
	(u_long *)ADU_CONS1_BASE,	/* Serial line #1	*/
	(u_long *)ADU_CONS1_ICR,
	(u_long *)ADU_CONS1_DB,
	(u_long *)ADU_CONS2_BASE,	/* Serial line #2	*/
	(u_long *)ADU_CONS2_ICR,
	(u_long *)ADU_CONS2_DB
};

struct	cons_ring {
	volatile int	rxsize;
	volatile int	txsize;
	volatile int	rxui;
	volatile int	txli;
	volatile int	txmode;
	volatile int	pad[3];
	volatile int	rxli;
	volatile int	txui;
	volatile long	pad1[3];
	volatile char	rxbuf[RXSIZE];	/* 4096 - 32 bytes */
	volatile char	txbuf[TXSIZE];	/* 4096 - 32 bytes */
};
struct	cons_ring  *adu_cnptr;
long	adu_cons_intr_reg = 0;
extern	struct tty cons[];


/*
 * Alpha adu - console routines
 */
int	adu_cnstart();
extern	int	ttrstrt();
extern	int	ttydef_open();
extern	int	ttydef_close();

/* Macros */

#define ADU_CONS_RING_DB1       			\
	*adu_cons_regs.db1 = 1;				\
	mb();

#define ADU_CONS_SET_ICR1       *adu_cons_regs.icr1 = adu_cons_intr_reg;

#define	ENABLE_INTERRUPT1(val)				\
        adu_cons_intr_reg |= (val); 			\
	ADU_CONS_SET_ICR1;				\
	mb();

#define	DISABLE_INTERRUPT1(val)				\
        adu_cons_intr_reg &= ~(val); 			\
	ADU_CONS_SET_ICR1;				\
	mb();

/*
 * Need a few items for autoconfig of devices on "ibus"
 */
caddr_t aducnstd[] = { 0 };

int adu_cons_probe();
int adu_cons_attach();

struct controller *aducninfo[1];
struct	driver aducndriver =
	{ adu_cons_probe, 0, adu_cons_attach, 0, 0, aducnstd, 0, 0, "aducn", aducninfo };

adu_cons_probe(reg1, reg2)
	int reg1, reg2;
{
	/*
	 * Must init select/wakeup queue
	 */
	queue_init(&cons->t_selq);

        /*
	 * The intialization is done through adu_cons_init, so
	 * if we have gotten this far we are alive so return a 1
	 */
	return(1);
}

adu_cons_attach(reg)
	int reg;
{
        /*
	 * The intialization is done through adu_cons_init, so
	 * if we have gotten this far we are alive so return a 1
	 */
	return(1);
}

/*
 * Space for cons_ring struct. MUST be page aligned, so allocate
 * 2 pages and set pointer to page boundary within.
 */
char fudge[2*8192];

adu_cons_init()
{
	register int vector_return;

	/*
	 * Initialize cons_ring struct.
	 */
	adu_cnptr = (struct cons_ring *)((long)(fudge + 8191) & ~8191);

	adu_cnptr->txli = 0;
	adu_cnptr->txui = 0;
	adu_cnptr->rxli = 0;
	adu_cnptr->rxui = 0;
	adu_cnptr->txmode = SERIAL_NORMAL;
	adu_cnptr->rxsize = RXSIZE;
	adu_cnptr->txsize = TXSIZE;
	/*
	 * The console register pointers are already initialized to the 
	 * offset from the base register.  Now add the base address of the
	 * IO module's TVBus registers to these offsets to get pointers to the
	 * registers themselves.
	 *
	 * Have to jump through hoops here to get the casting to work!
 	 */
	adu_cons_regs.base1 = (u_long *) ((u_long)adu_cons_regs.base1 +
					  (u_long)adu_tv_cons_base);
	adu_cons_regs.icr1 = (u_long *) ((u_long)adu_cons_regs.icr1 +
					  (u_long)adu_tv_cons_base);
	adu_cons_regs.db1 = (u_long *) ((u_long)adu_cons_regs.db1 +
					  (u_long)adu_tv_cons_base);
	adu_cons_regs.base2 = (u_long *) ((u_long)adu_cons_regs.base2 +
					  (u_long)adu_tv_cons_base);
	adu_cons_regs.icr2 = (u_long *) ((u_long)adu_cons_regs.icr2 +
					  (u_long)adu_tv_cons_base);
	adu_cons_regs.db2 = (u_long *) ((u_long)adu_cons_regs.db2 +
					  (u_long)adu_tv_cons_base);
	if (svatophys(adu_cnptr, adu_cons_regs.base1) != KERN_SUCCESS)
		panic("adu_cons can't get phys addr");
	mb();
	/*
	 * Allocate SCB vector to enable interrupt handling.  The params
	 * are (interrupt ipl, interrupt routine, int routine parameter).
	 * Use the returned value to setup the IRQNODE, TIRQCHAN, and RIRQCHAN 
	 * fields of the icr register.
 	 *
	 * Since this driver has 2 interrupt routines it is necessary to setup
	 * a separate vector for each one.
	 *
	 * Since there are only 2 SCB vectors at SPL21 and 20 at SPL20,
	 * request that the ipl be 20 instead of 21.  Thats why this has a 
	 * hardcoded value of 20 instead of SPLTTY which equals 21.
	 */
	vector_return = adu_vector_alloc(20, adu_cnrint, 0);
	if (vector_return == 0) {
		printf("adu_scsi_probe: adu_vector_alloc failed on cnrint.\n");
		return(0);
	}
	adu_cons_intr_reg = ((ADU_GETIRQNODE(vector_return) << 2) |
			     (ADU_GETIRQCHAN(vector_return) << 6));
	vector_return = adu_vector_alloc(20, adu_cnxint, 0);
	if (vector_return == 0) {
		printf("adu_scsi_probe: adu_vector_alloc failed on cnxint.\n");
		return(0);
	}
	adu_cons_intr_reg |= (ADU_GETIRQCHAN(vector_return) << 11);
}

adu_cnopen(dev, flag)
	dev_t dev;
	unsigned int flag;
{
	register struct tty *tp;
	register int s;
	register int clocal = TRUE;         /* console is a local connection */

	if (minor(dev) != 0) {
	    return (ENXIO);
	}
	tp = cons;
	tp->t_oproc = adu_cnstart;

	tty_def_open(tp, dev, flag, clocal);

	if ((tp->t_state & TS_XCLUDE) && (u.u_uid != 0)) {
		return (EBUSY);
	}

	s = spltty();
	ENABLE_INTERRUPT1(SERIAL_RE | SERIAL_TE);
	ADU_CONS_RING_DB1;
	splx(s);

	return((*linesw[tp->t_line].l_open)(dev, tp));
}

adu_cnclose(dev)
	dev_t dev;
{
	register struct tty *tp = cons;

	if (minor(dev) != 0) {
		return (ENXIO);
	}
	(*linesw[tp->t_line].l_close)(tp);

	ttyclose(tp);
	tty_def_close(tp);

	DISABLE_INTERRUPT1(SERIAL_RE | SERIAL_TE);
	ADU_CONS_RING_DB1;
}

adu_cnread(dev, uio)
	dev_t dev;
	struct uio *uio;
{
	register struct tty *tp = cons;

	if (minor(dev) != 0) {
		return (ENXIO);
	}

	return ((*linesw[tp->t_line].l_read)(tp, uio));
}

adu_cnwrite(dev, uio)
	dev_t dev;
	struct uio *uio;
{
	register struct tty *tp = cons;

	if (minor(dev) != 0) {
		return (ENXIO);
	}

	return ((*linesw[tp->t_line].l_write)(tp, uio));
}

adu_cnioctl(dev, cmd, addr, flag)
	dev_t dev;
	unsigned int cmd;
	caddr_t addr;
	unsigned int flag;
{
	register struct tty *tp = cons;
	int error;

	if (minor(dev) != 0) {
		return (ENXIO);
	}

	error = (*linesw[tp->t_line].l_ioctl)(tp, cmd, addr);
	if (error >= 0) {
		return(error);
	}
	error = ttioctl(tp, cmd, addr, flag);
	if (error < 0) {
		error = ENOTTY;
	}
	return (error);
}

/*
 * SCB hardware receive interrupt routine.
 *
 */
adu_cnrint(line_num)
	register int line_num;	/* parameter not used */
{
    register struct tty *tp = cons;
    int c;

    if( adu_cnptr->rxui == adu_cnptr->rxli)
	    return(0); /* No character */
    while(adu_dmagetc(&c)) {
	    if (tp->t_state & TS_ISOPEN) {
		    if (tp->t_iflag & ISTRIP) {
			    c &= 0177;
		    } else {
			    c &= 0377;
	    
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
    }
}

/*
 * SCB hardware transmitter interrupt routine.
 *
 * Interrupts are generated when the transmit ring transitions from a 
 * non-empty to an empty state.  Note that this does not generate a
 * per-character interrupt (unless the output consisted of only 1 character).
 * Call the start routine to see if there are any more characters waiting in
 * the clist to be transmitted.
 */
adu_cnxint(line_num)
    register int line_num;	/* not used */
{
    struct tty *tp = cons;

    tp->t_state &= ~TS_BUSY;

    if (tp->t_line) {
	(*linesw[tp->t_line].l_start)(tp);
    } else {
	    adu_cnstart(tp);
    }
}


/*
 * Start output if there are any characters in the output queue.
 */
adu_cnstart(tp)
	register struct tty *tp;
{
	register int s, cc;

	s = spltty();

	/*
	 * If we are already busy with an output or the driver is stopped
	 * bail out.
	 */
	if (tp->t_state & (TS_TIMEOUT|TS_BUSY|TS_TTSTOP)) {
		goto out;
	}
	if (tp->t_outq.c_cc <= tp->t_lowat) {
	    if (tp->t_state&TS_ASLEEP) {
		    tp->t_state &= ~TS_ASLEEP;
		    wakeup((caddr_t)&tp->t_outq);
	    }
	    /* osf select stuff */
	    select_wakeup(&tp->t_selq);
#ifdef notdef
	    if (tp->t_wsel) {
		    selwakeup(tp->t_wsel, tp->t_state & TS_WCOLL);
		    tp->t_wsel = 0;
		    tp->t_state &= ~TS_WCOLL;
	    }
#endif
	}
	if (tp->t_outq.c_cc == 0) {	/* output queue empty */
		goto out;
	}

	/*
	 * There are characters to be transmitted.  Call ndqb to determine
	 * the number characters up to a delay character there are in the 
	 * outq waiting to be output.  In raw mode you don't look for delay
	 * characters.
	 */
		cc = ndqb(&tp->t_outq, DELAY_FLAG);
		/*
		 * This indicates that a delay character has been encountered.
		 * Perform the specified delay and toss the delay character
		 * itself out.
		 */
		if (cc == 0) {
			cc = getc(&tp->t_outq);
			timeout(ttrstrt, (caddr_t)tp, (cc&0x7f) + 6);
			tp->t_state |= TS_TIMEOUT;
			goto out;
		}

	tp->t_state |= TS_BUSY;
	adu_cnptr->txmode = SERIAL_NORMAL;
	/* Move characters from outq onto the transmit ring */
	while (cc--) {
		if(adu_dmaputc(*(tp->t_outq.c_cf)))
		   break;		     /* transmit ring is full */
		ndflush(&tp->t_outq,1);	
	}
	ADU_CONS_RING_DB1;
	mb();

out:
	splx(s);
	return;
}

adustop(tp, flag)
	register struct tty *tp;
{
	register int s;

	s = spltty();
	if (tp->t_state & TS_BUSY) {
		adu_cnptr->txmode = SERIAL_PAUSE;
	}
	splx(s);
}


/*
 * Used for console printf.  This is a non-interrupt driven polled mode routine
 * used to put characters out to the console terminal one character at a time.
 */
adu_cnputc(c)
	register int c;
{
	register int t;
	register int s, timo, saveadu_ctcs;
	
	/* if spl is less then tty we must raise priority to block out
	   tty interrupts.  If IPL is higher, we don't want to lower it */

	if (mfpr_ipl() < SPLTTY) {
		s = spltty(); 
	} else {
		s = mfpr_ipl();
	}

	if (c == 0) {
		splx(s);
		return;
	}

	t = adu_cnptr->txli;
	t++;
	if (t >= adu_cnptr->txsize)
		t = 0;
	while(t == adu_cnptr->txui)
		;	/* wait */
	adu_cnptr->txbuf[t] = c & 0xff;
	mb();
	adu_cnptr->txli = t;
	mb();
	ADU_CONS_RING_DB1;

	if (c == '\n') {		/* map carriage return - line feed */
		adu_cnputc('\r');
	}
	splx(s);
}


/*
 * Console terminal getc routine.
 * This is a non-interrupt driven polled mode routine
 * used to read characters from to the console terminal one character at a time.
 */
adu_cngetc()
{
	volatile register int t;
	register int s, c;

	s = splextreme();
	t = adu_cnptr->rxui;
	/*
	 * wait for char (present when indices are not equal)
	 */
	while(t == adu_cnptr->rxli) 
		t = adu_cnptr->rxui;
	t++;
	if (t >= adu_cnptr->rxsize)
		t = 0;
	c = (adu_cnptr->rxbuf[t]) & 0xff;	/* get char */
	adu_cnptr->rxui = t;
	if (c == '\r')
		c = '\n';
	splx(s);
	return(c);
}

/*
 * This routine will only stuff characters if the txbuf is not full. Unlike
 * the adu_cnputc routine it will NOT spin if the buffer is full.  This 
 * routine is called by adu_start() to simulate dma.  It is expected that
 * when the buffer becomes non-full an interrupt will be generated and any
 * additional characters will be transmitted.
 */

adu_dmaputc(c)
	register int c;
{
	register int t;
	

	/*
	 * Set "t" to the index in the transmit ring where this driver
	 * will be placing the next character.
	 */
	t = adu_cnptr->txli;
	t++;
	if (t >= adu_cnptr->txsize)	/* wrap around in ring */
		t = 0;
	/*
	 * If the io module has not removed all the characters up to this
	 * position it indicates that the transmit ring is full.  Return 1 to
	 * signify that the ring is full and the adu_dmaputc of the character
	 * will be retried later when a transmitter interrupt occurs which
	 * indicates that there is now space in the transmit ring.
	 */
	if(t == adu_cnptr->txui)
		return(1);
	/* Stuff the character into the transmit ring */
	adu_cnptr->txbuf[t] = c & 0xff;
	mb();
	/*
	 * Tell the io module that another character is in the ring waiting
	 * to be output by storing the now incremented pointer.
	 */
	adu_cnptr->txli = t;
	mb();
	return(0);
}

/*
 * This routine will check if a character is available, and if not will
 * return 0.  If a character is available, it will fill in the character
 * and return 1.  This routine is called from the receiver interrupt
 * routine and is called from a loop that will continue as long as there
 * are characters to be read.
 */
adu_dmagetc(cptr)
	register int	*cptr;
{
	volatile register int t;
	register int s;

	s = splextreme();
	t = adu_cnptr->rxui;
	/*
	 * check for char (present when indices are not equal)
	 */
	if(t == adu_cnptr->rxli) {
		splx(s);
		return(0);
	}
	t++;
	if (t >= adu_cnptr->rxsize)
		t = 0;
	*cptr = (adu_cnptr->rxbuf[t]) & 0xff;	/* get char */
	adu_cnptr->rxui = t;
	mb();
	if (*cptr == '\r')
		*cptr = '\n';
	splx(s);
	return (1);
}
