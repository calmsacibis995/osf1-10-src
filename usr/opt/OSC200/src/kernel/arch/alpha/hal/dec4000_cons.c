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
#include <machine/scb.h>
#include <machine/cpu.h>
#include <io/dec/uba/ubavar.h>	/* auto-config headers */
#include <hal/adudefs.h>
#include <vm/vm_kern.h>
#include <io/common/devdriver.h>
#include <machine/rpb.h>

/* local defines */
#define XON  0x11
#define XOFF 0x13
#define CONSOLE_CTB 0


/*
 * macro interface to cobra_tt calls
 */

/*
 * Description of cobra_tt(command,unit,character)
 *
 * command	= 0:	init
 *     		= 1:	getc_ready
 *			in:	unit
 *			return val = 0, not ready
 *			           = 1, ready
 *		= 2:	getc
 *			in:    init
 *			return value = character
 *    		= 3:	putc_ready
 *			in:	unit
 *			out:	return val = 0, not ready
 *			   	   = 1, ready
 *     		= 4:	putc
 *			in:	unit, character
 *     		= 5:	rx_int
 *			in:	unit
 *				character = 0, disable
 *		    	    		  = 1, enable
 *			return val = 0, previously disabled
 *		   	   	   = 1, previously enabled
 *     		= 6:	tx_int
 *			in:	unit
 *				character = 0, disable
 *		    			  = 1, enable
 *			return val = 0, previously disabled
 *		   		   = 1, previously enabled
 */
#define CONS_INIT 	0
#define CONS_RXRDY	1
#define CONS_GETC 	2
#define CONS_TXRDY	3	
#define CONS_PUTC 	4
#define	CONS_RXINT	5
#define	CONS_TXINT	6
#define DISABLE		0
#define ENABLE		1
#define DUMMY		0

#define COBRA_TT_INIT(u)	cobra_tt(CONS_INIT, u, DUMMY)
#define COBRA_READ_RXDB(u)	cobra_tt(CONS_GETC, u, DUMMY)
#define COBRA_WRITE_TXDB(c,u)	cobra_tt(CONS_PUTC, u, c)
#define COBRA_RX_RDY(u)		cobra_tt(CONS_RXRDY, u, DUMMY)
#define COBRA_TX_RDY(u)		cobra_tt(CONS_TXRDY, u, DUMMY)
#define COBRA_ENABLE_UART_INTERRUPTS(u) {	\
	cobra_tt(CONS_TXINT, u, ENABLE);	\
	cobra_tt(CONS_RXINT, u, ENABLE);	\
}
#define COBRA_DISABLE_UART_INTERRUPTS(u) {	\
	cobra_tt(CONS_RXINT, u, DISABLE);	\
	cobra_tt(CONS_TXINT, u, DISABLE);	\
}

/*
 * kdebug support
 */
#define KDEBUG_INACTIVE	0
#define KDEBUG_ACTIVE	1
#define KDEBUG_UNIT	1
extern int kdebug_state();

/*
 * Globals - with default values
 */
extern int cnxint();
extern int cnrint();

long	cobra_cons_intr_reg = 0;
extern	struct tty cons[];

/*
 * cobra - console routines
 */
extern	int	cobra_cnxint();
extern	int	cobra_cnrint();
extern  int	cobra_cnstart();
extern	int	ttrstrt();
extern	int	ttydef_open();
extern	int	ttydef_close();

extern printstate;

/*
 * Need a few items for autoconfig of devices on "ibus"
 */
caddr_t cobracnstd[] = { 0 };

int cobra_cons_probe();
int cobra_cons_attach();

cobra_cons_probe(reg1, reg2)
	int reg1, reg2;	/* not used */
{
	/*
	 * Must init select/wakeup queue
	 */
	queue_init(&cons[0].t_selq);

	/*
	 * If kdebug is active, don't touch the serial port that
	 * its talkint to
	 */
	if (kdebug_state() == KDEBUG_INACTIVE)
		queue_init(&cons[1].t_selq);

	return(1);
}

cobra_cons_attach(reg)
	int reg;
{
	/*
	 * The intialization is done through cobra_cons_init, so
	 * if we have gotten this far we are alive so return a 1
	 */
	return(1);
}

/*
 *  cobra_cons_init() - initialization for cobra console.
 */
cobra_cons_init()
{
	extern struct rpb_ctb *ctb;
	extern struct rpb *rpb;
	struct ctb_tt *ctb_tt;

	ctb_tt = (struct ctb_tt *)ctb;
	/*
	 * Setup generic interrupt dispatching to console interrupts.
	 */
	intrsetvec((int)ctb_tt->ctb_rivec, cobra_cnrint, 0);
	intrsetvec((int)ctb_tt->ctb_tivec, cobra_cnxint, 0);
	COBRA_TT_INIT(0);

	/*
	 * If kdebug is active, don't touch the serial port that
	 * its talkint to
	 */
	if (kdebug_state() == KDEBUG_INACTIVE){
		/* point to the next ctb entry */
		ctb_tt = (struct ctb_tt *)((u_long *)ctb+rpb->rpb_ctb_size/8);
		intrsetvec((int)ctb_tt->ctb_rivec, cobra_cnrint, 1);
		intrsetvec((int)ctb_tt->ctb_tivec, cobra_cnxint, 1);
		COBRA_TT_INIT(1);
	}
}
/*
 *  cobra_cnopen() - open console
 */
cobra_cnopen(dev, flag)
	dev_t dev; 
	unsigned int flag;
{
	register struct tty *tp;
	register int s;
	register int clocal = TRUE;       /* console is a local connection */
	register int unit;

	unit = minor(dev);
	if ((unit != 0) && (unit != 1)) {
		return (ENXIO);
	}

	/*
	 * don't allow open of the line kdebug is using
	 */
	if ((kdebug_state() == KDEBUG_ACTIVE) && (unit == KDEBUG_UNIT))
		return (ENXIO);

	tp = &cons[unit];

	tp->t_oproc = cobra_cnstart;
	
	if ((tp->t_state & TS_ISOPEN) == 0) {

		/* Set the default line discipline to termios */
		tp->t_line = TTYDISC;
		tty_def_open(tp, dev, flag, clocal);

		/*
		 * Specify console terminal attributes.  Do not allow modem control
		 * on the console.  Setup <NL> to <CR> <LF> mapping.
		 */

		/* modem control not supported on console */ 
		tp->t_cflag = CS8 | CREAD | CLOCAL;
		tp->t_state |= TS_CARR_ON;
		tp->t_flags = ANYP|ECHO|CRMOD;
		tp->t_iflag |= ICRNL; /* Map CRMOD */
		tp->t_oflag |= ONLCR; /* Map CRMOD */
		tp->t_ispeed = tp->t_ospeed = TTYDEF_SPEED;

	}

	if ((tp->t_state & TS_XCLUDE) && (u.u_uid != 0)) {
		return (EBUSY);
	}

	s = spltty();
	
	COBRA_ENABLE_UART_INTERRUPTS(unit);
	
	splx(s);
	
	return((*linesw[tp->t_line].l_open)(dev, tp, flag));
}

/*
 *	cobra_cnclose()
 */
cobra_cnclose(dev)
	dev_t dev;
{
	register struct tty *tp = cons;
	register int unit;

	unit = minor(dev);
	if ((unit != 0) && (unit != 1)) {
		return (ENXIO);
	}
	tp = &cons[unit];

	(*linesw[tp->t_line].l_close)(tp);

	if ((tp->t_state & TS_ISOPEN) == 0) {
		tp->t_state &= ~TS_CARR_ON;
	}
	
	ttyclose(tp);
	tty_def_close(tp);
	
	COBRA_DISABLE_UART_INTERRUPTS(unit);
}

/*
 *	cobra_cnread()
 */
cobra_cnread(dev, uio, flag)
	dev_t dev;
	struct uio *uio;
	unsigned int flag;
{
	register struct tty *tp = cons;
	register int unit;

	unit = minor(dev);
	if ((unit != 0) && (unit != 1)) {
		return (ENXIO);
	}
	tp = &cons[unit];
	
	return ((*linesw[tp->t_line].l_read)(tp, uio, flag));
}

/*
 *	cobra_cnwrite()
 */
cobra_cnwrite(dev, uio, flag)
	dev_t dev;
	struct uio *uio;
	unsigned int flag;
{
	register struct tty *tp;
	register int unit;

	unit = minor(dev);
	if ((unit != 0) && (unit != 1)) {
		return (ENXIO);
	}
	tp = &cons[unit];
	
	return ((*linesw[tp->t_line].l_write)(tp, uio, flag));
}

/*
 *	cobra_cnioctl()
 */
cobra_cnioctl(dev, cmd, addr, flag)
	dev_t dev;
	unsigned int cmd;
	caddr_t addr;
	unsigned int flag;
{
	register struct tty *tp;
	int error;
	register int unit;

	struct devget *devget;

	unit = minor(dev);
	if ((unit != 0) && (unit != 1)) {
		return (ENXIO);
	}
	tp = &cons[unit];
	
	if ((error = (*linesw[tp->t_line].l_ioctl)(tp, cmd, addr, flag))  >= 0) {
		return(error);
	}

	if ((error = ttioctl(tp, cmd, addr, flag)) >= 0){
		return (error);
	}
	
	switch (cmd) {
	      case DEVIOCGET:							/* device status */
		devget = (struct devget *)addr;
		bzero(devget,sizeof(struct devget));
		devget->category = DEV_TERMINAL;				/* terminal cat.*/
		devget->bus = DEV_NB;						/* NO bus	*/
		bcopy(DEV_VS_SLU, devget->interface, strlen(DEV_VS_SLU));	/* interface	*/
		bcopy(DEV_UNKNOWN, devget->device, strlen(DEV_UNKNOWN));	/* terminal	*/
		devget->slave_num = unit;					/* line number	*/
		devget->unit_num = unit;					/* line 	*/
		break;
		
	      default:
		return (ENOTTY);
	}
    return (0);
}

/*
 * SCB hardware receive interrupt routine.
 *
 */
cobra_cnrint(unit)
	register int unit;
{
	register struct tty *tp;
	register int c;

	if(unit > 1)
	    return;

	tp = &cons[unit];
	
	/* loop while rxcs indicates an outstanding character */

	while ( COBRA_RX_RDY(unit) ) {
		
		c = (COBRA_READ_RXDB(unit) & 0xff);
		
		if (tp->t_state & TS_ISOPEN) {
			if (tp->t_iflag & ISTRIP) {
				c &= 0177;
			} else {
				c &= 0377;
				
				/* If ISTRIP is not set a valid character of 
				 * 377 is read as 0377,0377 to avoid ambiguity
				 * with the PARMARK sequence.
				 */ 
				if ((c == 0377) && (tp->t_line == TERMIODISC)){
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
cobra_cnxint(unit)
	register int unit;
{
	struct tty *tp;
	
	if(unit > 1)
	    return;

	tp = &cons[unit];

	tp->t_state &= ~TS_BUSY;
	
	if (tp->t_line) {
		(*linesw[tp->t_line].l_start)(tp);
	} else {
		cobra_cnstart(tp);
	}
}

/*
 * Start output if there are any characters in the output queue.
 */
cobra_cnstart(tp)
	register struct tty *tp;
{
	register int s, cc;
	register int unit;
	int save_print;

	unit = minor(tp->t_dev);
	s = spltty();
	
	/*
	 * If we are already busy with an output or the driver is stopped
	 * bail out.
	 */
	if (tp->t_state & (TS_TIMEOUT | TS_BUSY | TS_TTSTOP)) {
		goto out;
	}
	if (tp->t_outq.c_cc <= tp->t_lowat) {
		if (tp->t_state&TS_ASLEEP) {
			tp->t_state &= ~TS_ASLEEP;
			wakeup((caddr_t)&tp->t_outq);
		}
		/* osf select stuff */
		select_wakeup(&tp->t_selq);
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
	
	/* this is commented out in the flamingo driver *./
	tp->t_state |= TS_BUSY;
	
	/* Write the flow on character and jam it out.  The delay shouldn't
	 * be too bad because it only happens in cnstart.
	 */
/* See if this works: Removed because this is invoked on every keystroke. */
/*	COBRA_WRITE_TXDB(XON,unit); */

	while( ! COBRA_TX_RDY(unit) )
		;	/* wait */
	
	/* Move characters from outq onto the transmit ring */
	while (cc--) {
		if(cobra_dmaputc(*(tp->t_outq.c_cf), unit))
			break;		     /* transmit ring is full */
		ndflush(&tp->t_outq,1);	
	}
	mb();
	
      out:
	splx(s);
	return;
}

cobrastop(tp, flag)
	register struct tty *tp;
{
	register int s;
	register int unit;

	unit = minor(tp->t_dev);
	s = spltty();
	if (tp->t_state & TS_BUSY) {
		COBRA_WRITE_TXDB(XOFF,unit);
	}
	splx(s);
}


/*
 * Used for console printf.  This is a non-interrupt driven polled mode routine
 * used to put characters out to the console terminal one character at a time.
 */
cobra_cnputc(c)
	register int c;
{
	register int s;
	register int unit=0;
	
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
	
	while( ! COBRA_TX_RDY(unit) )
		;	/* wait */
	
	COBRA_WRITE_TXDB(c,unit);
	mb();
	
	if (c == '\n') {		/* map carriage return - line feed */
		cobra_cnputc('\r',unit);
	}
	splx(s);
	
}

/*
 * Console terminal getc routine.
 * This is a non-interrupt driven polled mode routine used to read 
 * characters from to the console terminal one character at a time.
 */
cobra_cngetc()
{
	volatile register int t;
	register int s;
	register int c;
	register int unit=0;
	
	s = splextreme();
	
	while( ! COBRA_RX_RDY(unit) )
		;	/* wait for character */
	
	c = COBRA_READ_RXDB(unit);	/* get char */

	splx(s);
	
	return(c);
}


/*
 * This routine will only stuff characters if the txbuf is not full. Unlike
 * the cobra_cnputc routine it will NOT spin if the buffer is full.  This 
 * routine is called by cobra_start() to simulate dma.  It is expected that
 * when the buffer becomes non-full an interrupt will be generated and any
 * additional characters will be transmitted.
 */

cobra_dmaputc(c, unit)
	register int c;
	register int unit;
{
	/*
	 * If the transmit buffer is not empty return 1 to signify so,
	 * the adu_dmaputc of the character will be retried later when
	 * a transmitter interrupt occurs which indicates that the
	 * transmit buffer is empty.
	 */
	if (! COBRA_TX_RDY(unit) )
		return(1);
	
	/* Stuff the character into the transmit ring */
	COBRA_WRITE_TXDB(c,unit);
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
cobra_dmagetc(cptr, unit)
	register char	*cptr;
	register int unit;
{
	volatile register int t;
	register int s;
	
	s = splextreme();
	
	/*
	 * If not character; restore entry ipl and return 0.
	 */
	if( ! COBRA_RX_RDY(unit) ) {
		splx(s);
		return(0);
	}
	
	/* get character */
	*cptr = (char)(COBRA_READ_RXDB(unit) & 0xff);	/* get char */

	/* restore ipl and return */
	splx(s);
	return (1);
}
