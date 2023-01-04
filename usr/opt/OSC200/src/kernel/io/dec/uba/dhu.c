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
static char *rcsid = "@(#)$RCSfile: dhu.c,v $ $Revision: 1.1.11.2 $ (DEC) $Date: 1993/07/13 18:44:11 $";
#endif
/*
 * dhu.c
 *
 * Modification history
 *
 * 17-Feb-1992 - Fernando Fraticelli
 *	Initial port from Ultrix to OSF
 *
 * OSF work started above this line
 *----------------------------------------------------------------------------
 * DH(QUV)11/CX(ABY)(8,16) terminal driver
 *
 *  4-Apr-84 - Larry Cohen
 *
 *	Sleep in close to ensure DTR stays down for at least one
 *	second - only if modem line.  Open delays until close
 *	finishes in this case. -001
 *
 * 16-Jan-86 - Larry Cohen
 *
 *	Add full DEC standard 52 support.
 *
 * 10-Mar-86 - Tim Burke
 *
 *	Modified probe routine to wait for dhu self-test to complete.
 *
 * 18-Mar-86 - jaw
 *
 *	br/cvec changed to NOT use registers.
 *
 * 14-Apr-86 - jaw
 *
 *	Remove MAXNUBA references.....use NUBA only!
 *
 * 26-Apr-86 - ricky palmer
 *
 *	Added new DEVIOCGET ioctl request code. V2.0
 *
 * 13-Jun-86 - jaw
 *
 *	Fix to uba reset and drivers.
 *
 * 11-Jul-86 - ricky palmer
 *
 *	Added adpt and nexus fields to DEVIOCGET code.
 *
 * 05-Aug-86 - Tim Burke
 *
 *	Modify dhurint to save present time in timestamp when
 *	carrier drops.
 *
 * 25-Aug-86 - Tim Burke
 *
 *	Fixes to Decstd52 modem control to close up line on false call, and
 *	insure that remaining processes are terminated.
 *
 * 26-Aug-86 - rsp (Ricky Palmer)
 *
 *	Cleaned up devioctl code to (1) zero out devget structure
 *	upon entry and (2) use strlen instead of fixed storage
 *	for bcopy's.
 *
 *  4-Dec-86 - Tim Burke
 *	
 *	Bug fix to modem control.  In dhu_tty_drop routine, clear the stopped
 *	state to prevent modem lines from hanging on close.  Also removed a
 *	#define DHUDEBUG which shouldn't be here.
 *
 * 15-Dec-86 - Tim Burke
 *
 *	When a break occurs, (interpreted as a framing error) set the variable
 *	c to be the interrupt character.  There was a problem here due to the
 *	fact that sign extension is done which causes unwanted side affects. To
 *	solve this bug, the character is stripped to 8 bits.
 *
 *	Fix DEVIOGET to return propper modem status information.
 *
 *  9-Jan-87 - Tim Burke
 *
 *	Bug fix to TIOCMODEM to clear modem flags if signals are not up.
 *
 * 22-Jan-87 - Tim Burke
 *
 *	Bug fix in dhuclose to prevent lines from hanging after another line has
 *	closed.  The problem is a result of not setting the correct line number
 *	in the csr and masking interrupts.
 *
 * 28-Jan-87 - Tim Burke
 *
 *	Added the capability to ignore the "DSR" modem signal.  This is being
 *	done to allow modems that do not follow DEC Standard 52 to still 
 *	function as they would have prior to the addition of DECSTD52 code
 *	into the drivers.  If the driver is setup to ignore "DSR" then it will
 *	not be following DECSTD52.  To follow DECSTD52 set dhudsr to "1", to
 *	ignore "DSR" set dhudsr to be "0";
 *
 * 23-Feb-87 - Tim Burke
 *
 *	Added full System V TERMIO functionality to terminal subsystem.
 *
 *
 * 10-Mar-87 - rsp (Ricky Palmer)
 *
 *	Added devioctl support for CX series of controllers.
 *
 *  1-Sept-87 - Tim Burke
 *
 *	Put a timer in dhustart to prevent possible system hang if the DMA
 *	start bit doesn't clear due to hardware failure.
 *
 *  2-Sept-87 - Tim Burke
 *
 *	Added support for hardware auto flow control on the outgoing side.  This
 *	will provide quick response to start/stop characters which will reduce
 *	buffer overflow on the receiving device.
 *
 *  7-Sep-87 - rsp
 *
 *      Added code in DEVIOCGET to "&" in LINEMASK with unit to
 *      correctly determine line number.
 *
 *  1-Dec-87 - Tim Burke
 *
 *	Added support for both System V termio(7) and POSIX termios(7).  These
 *	changes also include support for 8-bit canonical processing.  Changes
 *	involve:
 *
 *	- Default settings on first open depend on mode of open.  For termio
 *	  opens the defaults are "RAW" style, while non-termio opens default
 *	  to the traditional "cooked" style.
 *	- The driver now represents its terminal attributes and special 
 *	  characters in the POSIX termios data structure.  This contrasts the
 *	  original approach of storing attributes and special chars in the
 *	  t_flags, ltchars and tchars.
 *	- New termio ioctls: TCSANOW, TCSADRAIN, TCSADFLUSH, TCSETA, TESETAW,
 *	  TCSETAF.	
 *	- Addition of LPASS8 to local mode word for 8-bit canonical support.
 *
 * 24-Mar-88 - Tim Burke
 *
 *	In attach routine, determine the number of lines on this board so that
 *	the installation process knows how many lines to create.  (8 or 16)
 *
 * 16-May-88 - Tim Burke
 *
 * 	Call param routine for setting of local mode word because it can 
 * 	affect bit size and parity.
 *
 * 5-Aug-88 - Tim Burke
 *
 *	Return the 2 character sequence 0377, 0377 upon receipt of a valid
 *	0377 character only when PARMRK is set under the termio line disc.
 *
 * 18-Aug-88 - Tim Burke
 *
 *	If PARMRK is set and a BREAK occurs, return '\0377','\0','\0'.
 *
 * 02-Sep-88 - Tim Burke
 *
 *	Return EINVAL instead of ENOTTY for POSIX programs on invalid ioctls.
 *
 * 25-Jan-89 - Randall Brown
 *
 *	Changed cd_drop to look at LNOHANG.  Changed close routine to look
 *	at HUPCL.
 *
 * 24-May-89 - Randall Brown
 *
 *	Added support to run on MIPSFAIR systems.
 *
 * 12-Jun-89 - dws
 *
 *	Added trusted support.
 *
 * 21-Jul-89 - Randall Brown
 *
 *	Moved default open and default close code to tty.c and call it
 *	using tty_def_open() and tty_def_close().  In the close routine,
 *	don't clear the baudrate so that subsequent opens will keep the
 *	present attributes.  This only applies to a Berkeley environment.
 *
 * 15-Aug-89 - Randall Brown
 *
 *	Changed all references of TCSADFLUSH to TCSAFLUSH 
 *
 * 31-Oct-89 - Randall Brown
 *
 *	Added the support to allow the device to determine if baudrate is 
 *	supported before it is set in the tty data structures.
 *
 */


#include "dhu.h" /* defines number of NDHU */
#if NDHU > 0  || defined(BINARY)

#include <data/dhu_data.c>

#define mprintf printf	/* Joe Amato added this */

int dhudebug = 0;
int dhucdtime = 2;

/* Define TS_NEED_PARAM */

#define TS_NEED_PARAM	0x10000000

/*
 * Baud Rate Support
 *
 * When the baud rate on the right is specified, the line parameter register
 * is setup with the appropriate bits as specified in the left column.
 */
#define BAUD_UNSUPPORTED 0      /* Device does not provide this baud rate */
#define BAUD_SUPPORTED   1      /* Device does provide this baud rate     */

struct speedtab dhu_speeds[] = {
     	{0,		BAUD_UNSUPPORTED},		/* B0    */
	{B50,		BAUD_SUPPORTED},		/* B50   */
	{B75,		BAUD_SUPPORTED},		/* B75   */
	{B110,		BAUD_SUPPORTED},		/* B110  */
	{B134,		BAUD_SUPPORTED},		/* B134  */
	{B150,		BAUD_SUPPORTED},		/* B150  */
	{B200,		BAUD_UNSUPPORTED},		/* B200  */
	{B300,		BAUD_SUPPORTED},		/* B300  */
	{B600,		BAUD_SUPPORTED},		/* B600  */
	{B1200,		BAUD_SUPPORTED},		/* B1200 */
	{B1800,		BAUD_SUPPORTED},		/* B1800 */
	{B2400,		BAUD_SUPPORTED},		/* B2400 */
	{B4800,		BAUD_SUPPORTED},		/* B4800 */
	{B9600,		BAUD_SUPPORTED},		/* B9600 */
	{B19200,	BAUD_SUPPORTED},		/* EXTA  */
	{B38400,	BAUD_UNSUPPORTED}, 		/* EXTB  */
	-1,		-1

   };

struct speedtab cs_speeds[] = {
      B0,	0,
      B50,   	DHU_B50,
      B75,   	DHU_B75,
      B110,  	DHU_B110,
      B134, 	DHU_B134_5,
      B150, 	DHU_B150,
      B300, 	DHU_B300,
      B600, 	DHU_B600,
      B1200, 	DHU_B1200,
      B1800, 	DHU_B1800,
      B2400, 	DHU_B2400,
      B4800, 	DHU_B4800,
      B9600, 	DHU_B9600,
      B19200, 	DHU_B19200,
      -1,    -1
};


short dhu_valid_speeds = 0x7fbd; /* 0,1,1,1, 1,1,1,1, 1,0,1,1, 1,1,0,1 */

/*
 * Definition of the driver for the auto-configuration program.
 */
int	dhuprobe(), dhuattach(), dhurint(), dhuxint(), dhubaudrate();
int	dhu_cd_drop(), dhu_dsr_check(), dhu_cd_down(), dhu_tty_drop();
int	dhustart(), dhuparam(), ttrstrt();

struct	timeval dhuzerotime = {0,0};

caddr_t dhustd[] = { 0 };

struct	driver dhudriver =
	{ dhuprobe, 0, dhuattach, 0, 0, dhustd, 0, 0, "dhu", dhuinfo };

/*
 * dhu_self_test is used to hold the self test codes until they are saved
 * in the attach routine.
 */
int 	dhu_self_test[DHU_NUM_ERR_CODES];


#ifndef MODEM_CD
#define MODEM_CD   0x01
#define MODEM_DSR  0x02
#define MODEM_CTS  0x04
#define MODEM_DSR_START  0x08
#endif

#define LINEMASK 0x0f	/* mask of higher bits of csr to get a line # */


/*
 * The clist space is mapped by the driver onto each UNIBUS.
 * The UBACVT macro converts a clist space address for unibus uban
 * into an i/o space address for the DMA routine.
 */
#define UBACVT(x, uban) 	(cbase[uban] + ((x)-(char *)cfree))

/*
 * Routine for configuration to force a dhu to interrupt.
 * Set to transmit at 9600 baud, and cause a transmitter interrupt.
 */
/*ARGSUSED*/
dhuprobe(reg)
	caddr_t reg;
{
	register struct dhudevice *dhuaddr = (struct dhudevice *)reg;
	int totaldelay; 		/* Self-test timeout counter */
	int i;

#ifdef lint
	if (ndhu11 == 0) ndhu11 = 1;
	dhurint(0); dhuxint(0);
#endif
#ifdef DHUDEBUG
	if(dhudebug)
		mprintf("dhuprobe\n");
#endif DHUDEBUG
	/*
	 * If a self test is not being done, start one up.  Wait for the
	 * self-test (to a max of 4 sec.) to complete before interrupting.
	 */

	if ((dhuaddr->csr.low & DHU_MRESET) == 0) {
	    dhuaddr->csr.low |= DHU_MRESET;
	    wbflush();
	}
	totaldelay = 0;
	while ( (dhuaddr->csr.low & DHU_MRESET) && ( totaldelay <= 70) ){
	    totaldelay++;
	    DELAY(50000);
	}
	if (dhuaddr->csr.low & DHU_MRESET)
	    mprintf("Warning: DHU device failed to exit self-test\n");
	else if (dhuaddr->csr.high & DHU_DIAGFAIL)
	    mprintf("Warning: DHU self-test failure\n");
	else {
	    for (i = 0; i < DHU_NUM_ERR_CODES; i++) {
		dhu_self_test[i] = dhuaddr->run.rbuf;
#ifdef DHUDEBUG
		if (dhudebug) {
		    mprintf("dhu_self_test[%d] = %x\n", i, dhu_self_test[i]);
		}
#endif DHUDEBUG
		if (dhu_self_test[i] >= 0) { /* data valid bit not set */
		    mprintf("Warning: DHU device failed to return all error codes\n");
		}
	    }
	}
	/*
	 * Setup for DMA transfer.  This device does not do programmed I/O
	 * because it would generate too many interrupts for the system to
	 * handle (particularly on Qbus microvaxen with dhv devices).
	 */
	dhuaddr->csr.low = 0;  /* transmit on channel 0 */
	wbflush();
	dhuaddr->csr.high |= DHU_XIE; /* enable transmit interrupts */
	dhuaddr->tbuffad1 = 0;
	dhuaddr->tbuffcnt = 0;
	dhuaddr->lpr = DHU_B9600 | DHU_BITS7 | DHU_PENABLE;
	dhuaddr->tbuffad2.high |= DHU_XEN;
	dhuaddr->tbuffad2.low |= DHU_START;
	wbflush();
	DELAY(100000);		/* wait 1/10'th of a sec for interrupt */
	{ char temp = dhuaddr->csr.high; /* clear transmit action */ }
	dhuaddr->csr.high = 0;	 /* disable transmit interrupts */
	wbflush();

	if (cvec && cvec != 0x200) /* check to see if interrupt occurred */
		cvec -= 4;	   /* point to first interrupt vector (recv)*/

	return (sizeof (struct dhudevice));
}

/*
 * Routine called to attach a dhu.
 */
dhuattach(ctlr)
	struct controller *ctlr;
{
	register struct dhudevice *dhuaddr;
	int i;
	register int unit;

#ifdef DHUDEBUG
	if(dhudebug)
		mprintf("dhuattach %x, %d\n", ctlr->flags, ctlr->ctlr_num);
#endif DHUDEBUG
	dhusoftCAR[ctlr->ctlr_num] = ctlr->flags;
	dhudefaultCAR[ctlr->ctlr_num] = ctlr->flags;
	/*
	 * On a Q-bus system the device could be either 8 or 16 lines.
	 * Presently if the board does not do modem control it must
	 * be a 16 line board.
	 */
	dhuaddr = (struct dhudevice *)ctlr->addr;
	if (dhuaddr->fun.fs.stat & DHU_MDL) 
		dhu_lines[ctlr->ctlr_num] = 16;
	else
		dhu_lines[ctlr->ctlr_num] = 8;
	
	/*
	 * Save the self test codes received in the probe routine in to
	 * the softc structure.
	 */
	for (i = 0; i < DHU_NUM_ERR_CODES; i++) {
	    dhu_softc[ctlr->ctlr_num].sc_self_test[i] = dhu_self_test[i];
	}
	for (unit = 0; unit < nNDHU * 16; unit++)
		queue_init(&dhu11[unit].t_selq);
}


/*
 * Open a DHU11 line, mapping the clist onto the uba if this
 * is the first dhu on this uba.  Turn on this dhu if this is
 * the first use of it.  Also wait for carrier.
 */
/*ARGSUSED*/
dhuopen(dev, flag)
	dev_t dev;
{
	register struct tty *tp;
	register int unit, dhu;
	register struct dhudevice *dhuaddr;
	register struct controller *ctlr;
	register int error;
	int s;
	int inuse;  /*hold state of inuse bit while blocked waiting for carr*/

	unit = minor(dev);
	dhu = unit >> 4;
	if (unit >= nNDHU*16 || (ctlr = dhuinfo[dhu])== 0 || ctlr->alive == 0){
		return (ENXIO);
	}
	tp = &dhu11[unit];
	if (tp->t_state&TS_XCLUDE && u.u_uid!=0)
		return (EBUSY);
#ifdef DHUDEBUG
	if (dhudebug)
		mprintf("dhuopen:  line=%d, state=%x, pid=%d, flag=(x)%x\n", unit,
			tp->t_state, u.u_procp->p_pid,flag);
#endif
	while (tp->t_state&TS_CLOSING) /* let DTR stay down for awhile */
		if (error = tsleep((caddr_t)&tp->t_rawq, TTIPRI | PCATCH, 
				ttopen, 0)) {
			mprintf("dhuopen: error sleeping for Closing\n");
			return(error);
		}
	dhuaddr = (struct dhudevice *)ctlr->addr;

	tp->t_addr = (caddr_t)dhuaddr;
	tp->t_oproc = dhustart;
	tp->t_param = dhuparam;
	tp->t_state |= TS_WOPEN;

	/*
	 * While setting up state for this uba and this dhu,
	 * block uba resets which can clear the state.
	 */
	s = spltty();
	while (tty_ubinfo[ctlr->bus_num] == -1)
		/* need this lock because uballoc can sleep */
		sleep(&tty_ubinfo[ctlr->bus_num], TTIPRI);
	if (tty_ubinfo[ctlr->bus_num] == 0) {
		tty_ubinfo[ctlr->bus_num] = -1;
		tty_ubinfo[ctlr->bus_num] =
		    uballoc(ctlr->bus_num, (caddr_t)cfree,
			nclist*sizeof(struct cblock), 0);
		wakeup(&tty_ubinfo[ctlr->bus_num]);
	}
	cbase[ctlr->bus_num] = tty_ubinfo[ctlr->bus_num]&0x3ffff;
	splx(s);

	if ((tp->t_state&TS_ISOPEN) == 0) {
	    tp->t_line = TTYDISC; 
	    tty_def_open(tp, dev, flag, (dhusoftCAR[dhu]&(1<<(unit&LINEMASK))));
	    dhumodem[unit] = MODEM_DSR_START; /* prevents spurious startups */
	    dhuparam(tp, &tp->t_termios);
	}

	/*
	 * Wait for carrier, then process line discipline specific open.
	 */

	s=spltty();
	if (dhuaddr->fun.fs.stat & DHU11) {
		/*
		 * only the dhu11 has a timer. manual says we have to
		 * point to line 0 before we set the timer
		 * This timer causes a delay before interrupting on the first
		 * character in hopes that more than one character can be
		 * processed by a single interrupt.
		 */
		dhuaddr->csr.low = DHU_RIE|(0 & LINEMASK); /* set to line 0 */
		wbflush();
		dhuaddr->run.rxtimer = 10;
		wbflush();
	}
	if (tp->t_cflag & CLOCAL) {
		/* this is a local connection - ignore carrier */
		tp->t_state |= TS_CARR_ON;
		dhumodem[unit] |= MODEM_CTS|MODEM_CD|MODEM_DSR;
		dhuaddr->csr.low = DHU_RIE|(unit & LINEMASK); /* set to line #*/
							  /* enable interrupts*/
		wbflush();
		dhuaddr->lnctrl &= ~(DHU_MODEM);
		dhuaddr->lnctrl |= (DHU_DTR|DHU_RTS|DHU_REN);
		wbflush();
		splx(s);
		return ((*linesw[tp->t_line].l_open)(dev, tp, flag));
	}
	dhuaddr->csr.low = DHU_RIE|(unit & LINEMASK);
	wbflush();
	dhuaddr->lnctrl |= (DHU_DTR|DHU_RTS|DHU_MODEM|DHU_REN);
	wbflush();

	/*
 	 * If the DSR signal is followed, give carrier 30 secs to come up,
	 * and do not transmit/receive data for the first 500ms.  Otherwise
	 * immediately go to dhu_dsr_check to look for CD and CTS.
 	 */
	if ((flag & (O_NDELAY|O_NONBLOCK)) == 0) {
		if (dhudsr) {
			if ((dhuaddr->fun.fs.stat)&DHU_DSR) {
				dhumodem[unit] |= (MODEM_DSR_START|MODEM_DSR);
				tp->t_dev = dev; /* need it for timeouts */
				timeout(dhu_dsr_check, tp, hz*30);
				timeout(dhu_dsr_check, tp, hz/2);
			}
		}
		else {
			dhumodem[unit] |= (MODEM_DSR_START|MODEM_DSR);
			dhu_dsr_check(tp);
		}
	}
#ifdef DHUDEBUG
	if (dhudebug)
		mprintf("dhuopen:  line=%d, state=%x, pid=%d\n", unit,
			tp->t_state, u.u_procp->p_pid);
#endif
	if (flag & (O_NDELAY|O_NONBLOCK))
		tp->t_state |= TS_ONDELAY;
	else {
		while ((tp->t_state&TS_CARR_ON)==0) {
			tp->t_state |= TS_WOPEN;	
			inuse = tp->t_state&TS_INUSE;
			/*
			 * Sleep until all the necessary modem signals
			 * come up.
			 */
			if (error = tsleep((caddr_t)&tp->t_rawq, 
				TTIPRI | PCATCH, ttopen, 0)) {
				splx(s);
				return (error);
			}
 			/*
 			 * See if wakeup is due to a false call.
 			 */
 			if (dhumodem[unit]&MODEM_BADCALL){
				splx(s);
 				return(EWOULDBLOCK);
			}
			/* if we opened "block if in use"  and
			 *  the terminal was not inuse at that time
			 *  but is became "in use" while we were
			 *  waiting for carrier then return
			 */
			if ((flag & O_BLKINUSE) && (inuse==0) &&
				(tp->t_state&TS_INUSE)) {
					splx(s);
					return(EALREADY);
			}
		}
	}
	splx(s);
	return ((*linesw[tp->t_line].l_open)(dev, tp));
}

/*
 * Close a DHU11 line.
 */
dhuclose(dev, flag)
	dev_t dev;
	int flag;
{
	register struct tty *tp;
	register int unit, s;
	register struct dhudevice *dhuaddr;
	register int dhu;
	extern int wakeup();
	int turnoff = 0;

	unit = minor(dev);
	dhu = unit >> 4;
	tp = &dhu11[unit];
	dhuaddr = (struct dhudevice *)tp->t_addr;
	tp->t_state |= TS_CLOSING;
	(*linesw[tp->t_line].l_close)(tp);
	if ((tp->t_cflag&HUPCL) || (tp->t_state&TS_ISOPEN)==0) {
		s = spltty();
		turnoff++;
		dhuaddr->csr.low = DHU_RIE|(unit & LINEMASK);
		wbflush();
		dhuaddr->lnctrl &= ~(DHU_DTR|DHU_RTS);  /* turn off DTR */
		wbflush();
		tp->t_state &= ~TS_CARR_ON; /* prevents recv intr. timeouts */
		if ((tp->t_cflag & CLOCAL) == 0) {
#ifdef DHUDEBUG
			if (dhudebug)
				mprintf("dhuclose: DTR drop line=%d, state=%x, pid=%d\n",
			unit, tp->t_state, u.u_procp->p_pid);
#endif

			/*
			 * Wait an additional 5 seconds for DSR to drop if
			 * the DSR signal is being watched.
			 */
			if (dhudsr && (dhuaddr->fun.fs.stat&DHU_DSR)) {
				timeout(wakeup, (caddr_t) &tp->t_dev, 5*hz);
				sleep((caddr_t)&tp->t_dev, PZERO-10);
			}
			timeout(wakeup, (caddr_t) &tp->t_dev, hz/5);
			sleep((caddr_t)&tp->t_dev, PZERO-10);

			tp->t_state &= ~(TS_CLOSING);
 			wakeup((caddr_t)&tp->t_rawq); /* wake up anyone in dhuopen */
		}
		splx(s);
	}
#ifdef DHUDEBUG
	if (dhudebug)
		mprintf("dhuclose: line=%d, state=%x, pid=%d\n", unit,
			tp->t_state, u.u_procp->p_pid);
#endif
	dhusoftCAR[dhu] &= ~(1<<(unit&LINEMASK));
	dhusoftCAR[dhu] |= (1<<(unit&LINEMASK)) & dhudefaultCAR[dhu];
 	ttyclose(tp);  /* remember this will clear out t_state */
 
 	if (turnoff) {
 		/* we have to do this after the ttyclose so that output
 		 * can still drain
 		 */
		s = spltty();
		dhuaddr->csr.low = DHU_RIE|(unit & LINEMASK);
		wbflush();
 		dhuaddr->lnctrl = NULL; /* turn off interrupts also */
		wbflush();
		splx(s);
	}
	dhumodem[unit] = 0;
	tty_def_close(tp);
 
}


dhuread(dev, uio)
	dev_t dev;
	struct uio *uio;
{
	register struct tty *tp = &dhu11[minor(dev)];
	
	return ((*linesw[tp->t_line].l_read)(tp, uio));
}

dhuwrite(dev, uio)
	dev_t dev;
	struct uio *uio;
{
	register struct tty *tp = &dhu11[minor(dev)];

	return ((*linesw[tp->t_line].l_write)(tp, uio));
}
/*
 * DHU11 receiver interrupt.
 */
dhurint(dhu)
	int dhu; /* module number */
{
	register struct tty *tp;
	register int ct, c, flg;
	register struct dhudevice *dhuaddr;
	struct tty *tp0;
	register struct controller *ctlr;
	int overrun = 0;
	register u_char *modem0, *modem;
	int unit; 
	int modem_cont;

	ctlr = dhuinfo[dhu];
	if (ctlr == 0 || ctlr->alive == 0) {
		return;
	}
	dhuaddr = (struct dhudevice *)ctlr->addr;
	tp0 = &dhu11[dhu<<4];  /* first tty structure that corresponds
				* to this dhu11 module
				*/
	modem0 = &dhumodem[dhu<<4];
	/*
	 * Loop fetching characters from receive fifo for this
	 * dhu until there are no more in the receive fifo.
	 */
	while ((c = dhuaddr->run.rbuf) < 0) {
		/* if c < 0 then data valid is set */
		unit = (c>>8)&LINEMASK;
		tp = tp0 + unit; /* tty struct for this line */
		flg = tp->t_iflag;		
		modem = modem0 + unit;
#ifdef DHUDEBUG
		if (dhudebug > 7)
			mprintf("dhurint0: c=%x, tp=%x\n", c, tp);
#endif
		/* check for modem transitions */
		if ((c & DHU_STAT)==DHU_STAT) {
			if (c & DHU_DIAG){ /* ignore diagnostic info */
				continue;
			}
#ifdef DHUDEBUG
			if (dhudebug > 4)
				mprintf("dhurint: c=%x, tp=%x\n", c, tp);
#endif
			/*
			 * Don't respond to modem status changes on a 
			 * direct connect line.  Actually there should not
			 * be any modem status interrupts on a direct connect
			 * line because the link type is set to non-modem.
			 */
			if (tp->t_cflag & CLOCAL)  {
				continue;
			}

			/* set to line #*/
			dhuaddr->csr.low = DHU_RIE|(unit & LINEMASK);
			wbflush();
			modem_cont = 0;


			/* examine modem status */

			/*
			 * Drop DTR immediately if DSR has gone away.
			 * If really an active close then do not
			 *    send signals.
			 */

			if ((dhuaddr->fun.fs.stat&DHU_DSR)==0) {
				if (tp->t_state&TS_CLOSING) {
					untimeout(wakeup, (caddr_t) &tp->t_dev);
					wakeup((caddr_t) &tp->t_dev);
					continue;
				}
				if (tp->t_state&TS_CARR_ON) {
#ifdef DHUDEBUG
					if (dhudebug)
						mprintf("dhurint: DSR dropped,line=%d\n",unit);
#endif DHUDEBUG
					/*
 					 * Only drop if DSR is being followed.
 					 */
					if (dhudsr) {
						dhu_tty_drop(tp);
						/*
						 * Moved the continue here, so
						 * that if both DSR & CD change
						 * in same interval, we'll see
						 * both.
						 */
						continue;
					}
				}
			}

			/*
			 * Check for transient CD drops.
			 * Only drop DTR if CD is down for more than 2 secs.
			 */
			if (tp->t_state&TS_CARR_ON)
			    if ((dhuaddr->fun.fs.stat&DHU_CD)==0){
				if ( *modem & MODEM_CD) {
				    /* only start timer once */
#ifdef DHUDEBUG
				    if (dhudebug)
					mprintf("dhurint, cd_drop, tp=%x\n", tp);
#endif DHUDEBUG
				    *modem &= ~MODEM_CD;
				    dhutimestamp[minor(tp->t_dev)] = time;
				    timeout(dhu_cd_drop, tp, hz*dhucdtime);
				    modem_cont = 1;
				}
			    } else
				/*
				 * CD has come up again.
				 * Stop timeout from occurring if set.
				 * If interval is more than 2 secs then
				 *  drop DTR.
				 */
				if ((*modem&MODEM_CD)==0) {
					untimeout(dhu_cd_drop, tp);
					if (dhu_cd_down(tp)) {
						/* drop connection */
						dhu_tty_drop(tp);
					}
					*modem |= MODEM_CD;
				        modem_cont = 1;
				}

			/* CTS flow control check */

			if (tp->t_state&TS_CARR_ON)
				if ((dhuaddr->fun.fs.stat&DHU_CTS)==0) {
					tp->t_state |= TS_TTSTOP;
					*modem &= ~MODEM_CTS;
#ifdef DHUDEBUG
					if (dhudebug)
					   mprintf("dhurint: CTS stop, line=%d\n", unit);
#endif DHUDEBUG
					dhustop(tp, 0);
					continue;
				} else if ((*modem&MODEM_CTS)==0) {
					    tp->t_state &= ~TS_TTSTOP;
					    *modem |= MODEM_CTS;
#ifdef DHUDEBUG
					    if (dhudebug)
					       mprintf("dhurint: CTS start, line=%d\n", unit);
#endif DHUDEBUG
					    dhustart(tp);
					    continue;
					}

			/*
			 * Avoid calling dhu_start_tty for a CD transition if
			 * the connection has already been established.
			 */
			if (modem_cont) {
				continue;
			}
			/*
			 * If 500 ms timer has not expired then dont
			 * check anything yet.
			 * Check to see if DSR|CTS|CD are asserted.
			 * If so we have a live connection.
			 * If DSR is set for the first time we allow
			 * 30 seconds for a live connection.
			 *
		    	 * 
		    	 * If dhudsr is set look for DSR|CTS|CD, otherwise look 
		    	 * for CD|CTS only.
		    	 */
			if (dhudsr) {
				if ((dhuaddr->fun.fs.stat&DHU_XMIT)==DHU_XMIT
			    	&& (*modem&MODEM_DSR_START)==0)
					dhu_start_tty(tp);
				else
			    	if ((dhuaddr->fun.fs.stat&DHU_DSR) &&
					(*modem&MODEM_DSR)==0) {
					*modem |= (MODEM_DSR_START|MODEM_DSR);
					/*
				 	* we should not look for CTS|CD for
				 	* about 500 ms.
				 	*/
					timeout(dhu_dsr_check, tp, hz*30);
					timeout(dhu_dsr_check, tp, hz/2);
			    	}
			}
			/* 
			 * Ignore DSR.
			 */
			else {
				if ((dhuaddr->fun.fs.stat&(DHU_CD|DHU_CTS))==(DHU_CD|DHU_CTS))
					dhu_start_tty(tp);
			}



			/*
			 * Examination of modem status character is complete.
			 * Go back for the next character to avoid passing this
			 * status info in the receiver buffer back to the user
			 * as a valid character.
			 */
			continue;
		}

		if ((tp->t_state&TS_ISOPEN)==0) {
			if((tp->t_state&TS_WOPEN)==0)
				wakeup((caddr_t)&tp->t_rawq);
			continue;
		}

		ct = c & TTY_CHARMASK;

		/* DHU_FERR is interpreted as a break */
		if (c & DHU_FERR) {
#ifdef DHUDEBUG
		if (dhudebug)
			mprintf("dhurint: Framming Error\n");
#endif DHUDEBUG
		
			ct |= TTY_FE;
			/* osf does not provide trusted path */	
		}
		/* Parity Error */
		if (c & DHU_PERR){
#ifdef DHUDEBUG
			if (dhudebug > 5)
				mprintf("dhurint: Parity Error, tp=%x\n", tp);
#endif DHUDEBUG
			ct |= TTY_PE;
		}

		/*
		 * Set if one or more previous characters on this line were
		 * lost because of a full FIFO, or failure to service the
		 * UART (possible indication of unibus overloading).
		 */
		if ((c & DHU_OVERR) && overrun == 0) {
			printf("dhu%d, line%d: recv. fifo overflow\n",dhu,unit);
			overrun = 1;
		}

#ifdef DHUDEBUG
		if (dhudebug > 7)
			mprintf("dhurint: c=%c, line=%d\n",c,unit);
#endif DHUDEBUG
#if NHC > 0
		if (tp->t_line == HCLDISC) {
			HCINPUT(c, tp);
		} else
#endif
			(*linesw[tp->t_line].l_rint)(ct, tp);
	}
}

static char cnbrkstr[] = "cnbrk";

/*
 * Ioctl for DHU11.
 */
dhuioctl(dev, cmd, data, flag)
	dev_t dev;
	register int cmd;
	caddr_t data;
	int flag;
{
	register int dhu, unit;
	register struct dhudevice *dhuaddr;
	register struct tty *tp;
	register int s;
	struct controller *ctlr;
	struct dhu_softc *sc;
	struct devget *devget;
	int error, i;

	unit = minor(dev);
	tp = &dhu11[unit];
	dhu = unit >> 4;	   /* module number */
	ctlr = dhuinfo[dhu];
	sc = &dhu_softc[dhu];
#ifdef DHUDEBUG
	if (dhudebug > 1)
		mprintf("dhuioctl: unit=%d, cmd=%d\n", unit, cmd&0xff);
#endif
	error = (*linesw[tp->t_line].l_ioctl)(tp, cmd, data, flag);
	if (error >= 0)
		return (error);
	error = ttioctl(tp, cmd, data, flag);
	if (error >= 0) {
		return (error);
	}
	dhuaddr = (struct dhudevice *)tp->t_addr;
#ifdef DHUDEBUG
	if (dhudebug > 1)
		mprintf("dhuioctl: unit=%d, cmd=%d\n", unit, cmd&0xff);
#endif
	switch (cmd) {

	case TCSBREAK:
		s = spltty();
		dhuaddr->csr.low = DHU_RIE|(unit & LINEMASK);
		wbflush();
		dhuaddr->lnctrl |= DHU_BREAK;
		wbflush();
		splx(s);
                mpsleep((caddr_t)&sc->sc_flags[unit], PZERO-10, cnbrkstr,
                        hz/4, NULL, 0);
		s = spltty();
		dhuaddr->csr.low = DHU_RIE|(unit & LINEMASK);
		wbflush();
		dhuaddr->lnctrl &= ~DHU_BREAK;
		wbflush();
		splx(s);
		break;
	case TIOCSBRK:
		s = spltty();
		dhuaddr->csr.low = DHU_RIE|(unit & LINEMASK);
		wbflush();
		dhuaddr->lnctrl |= DHU_BREAK;
		wbflush();
		splx(s);
		break;

	case TIOCCBRK:
		s = spltty();
		dhuaddr->csr.low = DHU_RIE|(unit & LINEMASK);
		wbflush();
		dhuaddr->lnctrl &= ~DHU_BREAK;
		wbflush();
		splx(s);
		break;

	case TIOCSDTR:
		s = spltty();
		dhuaddr->csr.low = DHU_RIE|(unit & LINEMASK);
		wbflush();
		dhuaddr->lnctrl |= (DHU_DTR|DHU_RTS);
		wbflush();
		splx(s);
		break;

	case TIOCCDTR:
		s = spltty();
		dhuaddr->csr.low = DHU_RIE|(unit & LINEMASK);
		wbflush();
		dhuaddr->lnctrl &= ~(DHU_DTR|DHU_RTS);
		wbflush();
		splx(s);
		break;

	case TIOCNMODEM:  /* ignore modem status */
		s = spltty();
		dhusoftCAR[dhu] |= (1<<(unit&LINEMASK));
		if (*(int *)data) /* make mode permanent */
			dhudefaultCAR[dhu] |= (1<<(unit&LINEMASK));
		tp->t_state |= (TS_CARR_ON|TS_ISOPEN);
		dhuaddr->csr.low = DHU_RIE|(unit & LINEMASK);
		wbflush();
		dhuaddr->lnctrl &= ~(DHU_MODEM);
		wbflush();
		dhumodem[unit] = MODEM_CTS|MODEM_CD|MODEM_DSR;
		tp->t_cflag |= CLOCAL;		/* Map to termio */
		splx(s);
		break;

	case TIOCMODEM:  /* dont ignore modem status  */
		s = spltty();
		dhusoftCAR[dhu] &= ~(1<<(unit&LINEMASK));
		if (*(int *)data) /* make mode permanent */
			dhudefaultCAR[dhu] &= ~(1<<(unit&LINEMASK));
		dhuaddr->csr.low = DHU_RIE|(unit & LINEMASK);
		wbflush();
		dhuaddr->lnctrl |= DHU_MODEM;
		wbflush();
		/* 
		 * If dhudsr is set look for DSR|CTS|CD, otherwise look 
		 * for CD|CTS only.
		 */
		if ((dhudsr && ((dhuaddr->fun.fs.stat&DHU_XMIT)==DHU_XMIT)) ||
		   ((dhudsr == 0) && ((dhuaddr->fun.fs.stat&(DHU_CD|DHU_CTS)) ==
		    (DHU_CD|DHU_CTS)))) {
			tp->t_state |= TS_CARR_ON;
			tp->t_state &= ~TS_ONDELAY;
			dhumodem[unit] = MODEM_CTS|MODEM_CD|MODEM_DSR;
		}
		else {
			dhumodem[unit] &= ~(MODEM_CTS|MODEM_CD|MODEM_DSR);
			tp->t_state &= ~(TS_CARR_ON|TS_ISOPEN);
		}
		tp->t_cflag &= ~(CLOCAL);	/* Map to termio */
		splx(s);
		break;

	case TIOCMGET:
		s = spltty();
		dhuaddr->csr.low = DHU_RIE|(unit & LINEMASK);
		wbflush();
		*(int *)data = dhutodm(dhuaddr->lnctrl,dhuaddr->fun.fs.stat);
		splx(s);
		break;

	case TIOCMSET:
		s = spltty();
		dhuaddr->csr.low = DHU_RIE|(unit & LINEMASK);
		wbflush();
		dhuaddr->lnctrl = dmtodhu(*(int *)data);
		wbflush();
		splx(s);
		break;

	case TIOCMBIS:
		s = spltty();
		dhuaddr->csr.low = DHU_RIE|(unit & LINEMASK);
		wbflush();
		dhuaddr->lnctrl |= dmtodhu(*(int *)data);
		wbflush();
		splx(s);
		break;

	case TIOCMBIC:
		s = spltty();
		dhuaddr->csr.low = DHU_RIE|(unit & LINEMASK);
		wbflush();
		dhuaddr->lnctrl &= ~(dmtodhu(*(int *)data));
		wbflush();
		splx(s);
		break;

	case DEVIOCGET: 			/* device status */
		devget = (struct devget *)data;
		bzero(devget,sizeof(struct devget));

		if (tp->t_cflag & CLOCAL) {
			sc->sc_category_flags[unit&LINEMASK] |= DEV_MODEM;
			sc->sc_category_flags[unit&LINEMASK] &= ~DEV_MODEM_ON;
		}
		else
			sc->sc_category_flags[unit&LINEMASK] |= (DEV_MODEM|DEV_MODEM_ON);

		devget->category = DEV_TERMINAL;

		if (dhu_lines[ctlr->ctlr_num] == 16)
			bcopy(DEV_CXAB16,devget->interface,
				strlen(DEV_CXAB16));
		else
			bcopy(DEV_DHQVCXY,devget->interface,
			      	strlen(DEV_DHQVCXY));


		bcopy(DEV_UNKNOWN,devget->device,
		      strlen(DEV_UNKNOWN));		/* terminal	*/
		devget->adpt_num = 0; 			/* which adapter*/
		devget->nexus_num = 0		;	/* which nexus	*/
		devget->bus_num = 0;			/* which UBA/QB */
		devget->ctlr_num = dhu; 		/* which interf.*/
		devget->slave_num = unit&LINEMASK;	/* which line	*/
		bcopy("dhu",devget->dev_name,4);	/* Ultrix "dhu" */
		devget->unit_num = unit&LINEMASK;	/* dh[vu] line #*/
		devget->soft_count =
		      sc->sc_softcnt[unit&LINEMASK];	/* soft er. cnt.*/
		devget->hard_count =
		      sc->sc_hardcnt[unit&LINEMASK];	/* hard er cnt. */
		devget->stat = sc->sc_flags[unit&LINEMASK]; /* status	*/
		devget->category_stat =
		      sc->sc_category_flags[unit&LINEMASK]; /* cat. stat.*/
		break;

	default:
		return (ENOTTY);
	}
	return (0);
}

dhutodm(lnctrl,lstat)
	register u_short lnctrl;
	register char lstat;
{
	register int b = 0;
	if (lnctrl&DHU_RTS)  b |= TIOCM_RTS;
	if (lnctrl&DHU_DTR)  b |= TIOCM_DTR;
	if (lstat&DHU_CD)  b |= TIOCM_CD;
	if (lstat&DHU_CTS)  b |= TIOCM_CTS;
	if (lstat&DHU_RING)  b |= TIOCM_RI;
	if (lstat&DHU_DSR)  b |= TIOCM_DSR;
	return(b);
}


dmtodhu(bits)
	register int bits;
{
	register u_short lnctrl = 0;
	if (bits&TIOCM_RTS) lnctrl |= DHU_RTS;
	if (bits&TIOCM_DTR) lnctrl |= DHU_DTR;
	return(lnctrl);
}

/*
 * Set parameters from open or stty into the DHU hardware
 * registers.
 */
dhuparam(tp, t)
	register struct tty *tp;
	register struct termios *t;
{
	register struct dhudevice *dhuaddr;
	register int lpar;
	int unit = minor(tp->t_dev);
	int s;
	int ospeed = ttspeedtab(t->c_ospeed, cs_speeds);


	/*if (tp->t_state & TS_BUSY) {
	    tp->t_state |= TS_NEED_PARAM;
	    return;
	}*/

	dhuaddr = (struct dhudevice *)tp->t_addr;
	/*
	 * Block interrupts so parameters will be set
	 * before the line interrupts.
	 */
	s = spltty();
	dhuaddr->csr.low = DHU_RIE|(unit&LINEMASK);
	wbflush();

	/* check requested parameters */ 

	if ((tp->t_state & TS_ISOPEN) && !(tp->t_cflag & CLOCAL) &&
							(t->c_ospeed == B0)) {
	dhu_tty_drop(tp);
	splx(s);
	return(0);	
	}

	/* If input baud rate is set to zero, the input baud rate will
	   be specified by the value of the output baud rate */
	if (ospeed < 0 || (t->c_ispeed ? (t->c_ispeed != t->c_ospeed) 
			: !(t->c_ispeed = t->c_ospeed))) {
		return EINVAL;
	}
	
	if (ttspeedtab(t->c_ospeed, dhu_speeds) == BAUD_UNSUPPORTED)  {
		return (EINVAL);
	}
        /* and copy to tty */
        tp->t_ispeed = t->c_ispeed;
        tp->t_ospeed = t->c_ospeed;
        tp->t_cflag = t->c_cflag;

	lpar = ospeed | ospeed >> 4 | unit & LINEMASK;
 	/*
	 * Set device registers according to the specifications of the termio
	 * structure.
 	 */
	if (tp->t_cflag & CREAD) 
 		dhuaddr->lnctrl |= DHU_REN;
	else
 		dhuaddr->lnctrl &= ~DHU_REN;
	wbflush();
 	if (tp->t_cflag & CSTOPB)
 		lpar |= DHU_TWOSB;
 	else
 		lpar &= ~DHU_TWOSB;
 	/* parity is enable */
 	if (tp->t_cflag & PARENB) {
 		if ((tp->t_cflag & PARODD) == 0)
 			/* else set even */
 			lpar |= DHU_PENABLE|DHU_EVENPAR;
 		else
 			/* set odd */
 			lpar = (lpar | DHU_PENABLE)&~DHU_EVENPAR;
 	}
 	/*
 	 * character size.
 	 * clear bits and check for 6,7,and 8, else its 5 bits.
 	 */
 	lpar &= ~DHU_BITS8;
 	switch(tp->t_cflag&CSIZE) {
 		case CS6:
 			lpar |= DHU_BITS6;
 			break;
 		case CS7:
 			lpar |= DHU_BITS7;
 			break;
 		case CS8:
 			lpar |= DHU_BITS8;
 			break;
 	}
	dhuaddr->lnctrl &= ~DHU_XFLOW;
	wbflush();
	dhuaddr->lpr = lpar;
	wbflush();
#ifdef DHUDEBUG
	if (dhudebug)
		mprintf("dhuparam: tp=%x, lpr=%x\n", tp, lpar);
#endif
	splx(s);
	return(0);
}

/*
 * DHU11 transmitter interrupt.
 * Restart each line which used to be active but has
 * terminated transmission since the last interrupt.
 */
dhuxint(dhu)
	int dhu;  /* module number */
{
	register struct tty *tp;
	register struct dhudevice *dhuaddr;
	register struct controller *ctlr;
	register int unit, totaldelay;
	u_short cntr;
	char csrxmt;

	ctlr = dhuinfo[dhu];
	dhuaddr = (struct dhudevice *)ctlr->addr;
	while ((csrxmt = dhuaddr->csr.high) < 0) {
		if (csrxmt & DHU_DMAERR) {
			mprintf("dhu%d:%x DMA ERROR\n", dhu, csrxmt&0xff);
		}
		if (csrxmt & DHU_DIAGFAIL)
			mprintf("dhu%d: DIAG. FAILURE\n", dhu);
		unit = dhu * 16;
		unit |= csrxmt&LINEMASK;
		tp = &dhu11[unit];
#ifdef DHUDEBUG
		if (dhudebug > 4) 
			mprintf("dhuxint: unit=%x, tp=%x, c_cc=%d\n",
				unit, tp, tp->t_outq.c_cc);
#endif
		tp->t_state &= ~TS_BUSY;
		dhuaddr->csr.low = (unit&LINEMASK)|DHU_RIE;
		wbflush();
		totaldelay = 0;
		while ((dhuaddr->tbuffad2.low & DHU_START) && (totaldelay <= 100)){
		    totaldelay++;
		    DELAY(10000);
		}
		if (dhuaddr->tbuffad2.low & DHU_START) {
		    mprintf("dmbxint: Resetting DMA START bit on line %d\n",unit);
		    dhuaddr->tbuffad2.low &= ~DHU_START;
		}
		if (tp->t_state&TS_FLUSH) {
			tp->t_state &= ~TS_FLUSH;
		}
		else {
			/*
			 * Determine number of chars transmitted
			 * so far and flush these from the tty
			 * output queue.
			 * Do arithmetic in a short to make up
			 * for lost 16&17 bits (in tbuffad2).
			 */
			cntr = dhuaddr->tbuffad1 -
				UBACVT(tp->t_outq.c_cf, ctlr->bus_num);
			ndflush(&tp->t_outq, cntr);
		}

		/*if (tp->t_state & TS_NEED_PARAM) {
		    tp->t_state &= ~TS_NEED_PARAM;
		    dhuparam(tp, &tp->t_termios);
		}*/

		if (tp->t_line) {
			(*linesw[tp->t_line].l_start)(tp);
		}
		else {
			dhustart(tp);
		}
	}
}

/*
 * Start (restart) transmission on the given DHU11 line.
 */
dhustart(tp)
	register struct tty *tp;
{
	register struct dhudevice *dhuaddr;
	register int car, dhu, unit, nch;
	register int totaldelay;
	int line, s;

	unit = minor(tp->t_dev);
	dhu = unit >> 4;
	line = unit & LINEMASK; /* unit now equals the line number */
	dhuaddr = (struct dhudevice *)tp->t_addr;

	/*
	 * Must hold interrupts in following code to prevent
	 * state of the tp from changing.
	 */
	s = spltty();
	/*
	 * If it's currently active, or delaying, no need to do anything.
	 * Also do not transmit if not CTS
	 */
	if ((tp->t_state&(TS_TIMEOUT|TS_BUSY|TS_TTSTOP)) ||
		 ((tp->t_state&TS_CARR_ON) && (dhumodem[unit]&MODEM_CTS)==0))
		goto out;
	/*
	 * If there are sleepers, and output has drained below low
	 * water mark, wake up the sleepers.
	 */
#ifdef DHUDEBUG
	if (dhudebug > 9) 
		mprintf("dhustart0: tp=%x, LO=%d, cc=%d \n", tp,
			tp->t_lowat, tp->t_outq.c_cc);
#endif
	if (tp->t_outq.c_cc <= tp->t_lowat) {
		if (tp->t_state&TS_ASLEEP) {  /* wake up when output done */
			tp->t_state &= ~TS_ASLEEP;
			wakeup((caddr_t)&tp->t_outq);
		}
		select_wakeup(&tp->t_selq);
	}
	/*
	 * Now restart transmission unless the output queue is
	 * empty.
	 */
	if (tp->t_outq.c_cc == 0)
		goto out;
	nch = ndqb(&tp->t_outq, DELAY_FLAG);
	/*
	 * If first thing on queue is a delay process it.
	 */
	if (nch == 0) {
		nch = getc(&tp->t_outq);
		timeout(ttrstrt, (caddr_t)tp, (nch&0x7f)+6);
		tp->t_state |= TS_TIMEOUT;
		goto out;
	}
	/*
	 * If characters to transmit, restart transmission.
	 */
	if (nch) {
#ifdef DHUDEBUG
		if (dhudebug > 9) 
			mprintf("dhustart1: line=%x, nch=%d\n", line, nch);
#endif
		dhuaddr->csr.low = DHU_RIE|line; /* select line */
		wbflush();
		/*
		 * Wait for dma start to clear to a maximum of 1 second to
		 * prevent a system hang on hardware failure.  After the bit
		 * has stuck once, do not repeat this check.
		 */
		/*
		 * TODO:
 		 * This should be changed to use a timeout instead of calling
		 * microdelay!!!!!  Timeouts can be of granularity of 10 ms.
		 * Do this in the DMB driver also.
		 */
		totaldelay = 0;
		while ((dhuaddr->tbuffad2.low & DHU_START) && (totaldelay <= 100))
                        if ((dhu_softc[dhu].sc_flags[unit&LINEMASK]) == 0) {
	    			totaldelay++;
	    			DELAY(10000);
			}
			else
				totaldelay = 90000;
		if ((dhuaddr->tbuffad2.low & DHU_START) && 
		  ((dhu_softc[dhu].sc_flags[unit&LINEMASK]) == 0)) {
				mprintf("dhu%d,line%d DHU HARDWARE ERROR.  TX.DMA.START failed\n",dhu,line); 
				/*
				 * Prevent further activity on this line by
				 * setting state flag to dead.
				 */
				dhu_softc[dhu].sc_flags[unit&LINEMASK]++;
				splx(s);
				return;
		}
		if (dhuaddr->lnctrl & DHU_XABORT) /* clear abort if already set */
			dhuaddr->lnctrl &= ~(DHU_XABORT);
		dhuaddr->csr.high = DHU_XIE;
		/*
		 * If cblocks are ever malloc'ed we must insure that they
		 * never cross VAX physical page boundaries.
		 */
		/*
		 * Give the device the starting address of a DMA transfer.  
		 * Translate the system virtual address into a physical
		 * address.
		 */
		car = UBACVT(tp->t_outq.c_cf, dhuinfo[dhu]->bus_num);
		dhuaddr->tbuffad1 = car;
		dhuaddr->tbuffcnt = nch;
		/*
		 * If Outgoing auto flow control is enabled, the hardware will
		 * control the transmit enable bit.
		 */
		dhuaddr->tbuffad2.high = DHU_XEN;
		/* get extended address bits and start DMA output */
		dhuaddr->tbuffad2.low = ((car>>16)&0x3f)|DHU_START;
		wbflush();
		tp->t_state |= TS_BUSY;
	}
out:
	splx(s);
}

/*
 * Stop output on a line, e.g. for ^S/^Q or output flush.
 */
dhustop(tp, flag)
	register struct tty *tp;
{
	register struct dhudevice *dhuaddr;
	register int unit, s;

	dhuaddr = (struct dhudevice *)tp->t_addr;
	/*
	 * Block input/output interrupts while messing with state.
	 */
	s = spltty();
	if (tp->t_state & TS_BUSY) {
		/*
		 * Device is transmitting; stop output.
		 * We can continue later
		 * by examining the address where the dhu stopped.
		 */
		unit = minor(tp->t_dev);
		dhuaddr->csr.low = (unit&LINEMASK)|DHU_RIE;
		wbflush();
		if ((tp->t_state&TS_TTSTOP)==0)
			tp->t_state |= TS_FLUSH;
		dhuaddr->lnctrl |= DHU_XABORT;  /* abort DMA transmission */
		wbflush();
	}
	splx(s);
}

/*
 * Reset state of driver if UBA reset was necessary.
 * Reset the csr and lpr registers on open lines, and
 * restart transmitters.
 */
dhureset(uban)
	int uban;
{
	register int dhu, unit, s;
	register struct tty *tp;
	register struct controller *ctlr;
	register struct dhudevice *dhuaddr;
	int i;

	if (tty_ubinfo[uban] == 0)
		return;  /* there are no dhu11's in use */
	cbase[uban] = tty_ubinfo[uban]&0x3ffff;
	dhu = 0;
	for (dhu = 0; dhu < nNDHU; dhu++) {
		ctlr = dhuinfo[dhu];
		if (ctlr == 0 || ctlr->alive == 0 || ctlr->bus_num != uban)
			continue;
		dhuaddr = (struct dhudevice *)ctlr->addr;
		unit = dhu * 16;
		for (i = 0; i < 16; i++) {
			tp = &dhu11[unit];
			if (tp->t_state & (TS_ISOPEN|TS_WOPEN)) {
				dhuparam(tp, &tp->t_termios);
				s = spltty();
				dhuaddr->csr.low = (i&LINEMASK)|DHU_RIE;
				wbflush();
				dhuaddr->lnctrl |= DHU_DTR|DHU_RTS|DHU_REN;
				wbflush();
				splx(s);
				tp->t_state &= ~TS_BUSY;
				dhustart(tp);
			}
			unit++;
		}
	}
	dhutimer();
}

/*
 * At software clock interrupt time or after a UNIBUS reset
 * empty all the dh silos.
 */
dhutimer()
{
	register int dhu;
	register int s = spltty();

	for (dhu = 0; dhu < nNDHU; dhu++)
		dhurint(dhu);
	splx(s);
}

dhu_cd_drop(tp)
register struct tty *tp;
{
	register struct dhudevice *dhuaddr = (struct dhudevice *)tp->t_addr;
	register int unit = minor(tp->t_dev);

	dhuaddr->csr.low = DHU_RIE|(unit & LINEMASK);
	wbflush();
	if ((tp->t_state&TS_CARR_ON) &&
		((dhuaddr->fun.fs.stat&DHU_CD) == 0)) {
		if (dhudebug)
		    mprintf("dhu_cd:  no CD, tp=%x\n", tp);
		dhu_tty_drop(tp);
		return;
	}
	dhumodem[minor(tp->t_dev)] |= MODEM_CD;
	if (dhudebug)
	    mprintf("dhu_cd:  CD is up, tp=%x\n", tp);
}

dhu_dsr_check(tp)
register struct tty *tp;
{
	int unit = minor(tp->t_dev);
	register struct dhudevice *dhuaddr = (struct dhudevice *)tp->t_addr;
	if (dhumodem[unit]&MODEM_DSR_START) {
		if (dhudebug)
		    mprintf("dhu_dsr_check0:  tp=%x\n", tp);
		dhumodem[unit] &= ~MODEM_DSR_START;
		dhuaddr->csr.low = DHU_RIE|(unit&LINEMASK);
		wbflush();
		/* 
		 * If dhudsr is set look for DSR|CTS|CD, otherwise look 
		 * for CD|CTS only.
		 */
		if (dhudsr) {
			if ((dhuaddr->fun.fs.stat&DHU_XMIT)==DHU_XMIT)
				dhu_start_tty(tp);
		}
		else
			if ((dhuaddr->fun.fs.stat&(DHU_CD|DHU_CTS))==(DHU_CD|DHU_CTS))
				dhu_start_tty(tp);
		return;
	}
	if ((tp->t_state&TS_CARR_ON)==0)  {
		dhu_tty_drop(tp);
		if (dhudebug)
		    mprintf("dhu_dsr_check:  no carrier, tp=%x\n", tp);
	}
	else
		if (dhudebug)
		    mprintf("dhu_dsr_check:  carrier is up, tp=%x\n", tp);
}

/*
 *  cd_down return 1 if carrier has been down for at least 2 secs.
 */
dhu_cd_down(tp)
struct tty *tp;
{
	int msecs;
	int unit = minor(tp->t_dev);

	msecs = 1000000 * (time.tv_sec - dhutimestamp[unit].tv_sec) +
		(time.tv_usec - dhutimestamp[unit].tv_usec);
	if (msecs > 2000000)
		return(1);
	else
		return(0);
}

dhu_tty_drop(tp)
struct tty *tp;
{
	register struct dhudevice *dhuaddr = (struct dhudevice *)tp->t_addr;
	register int unit;
	if (tp->t_flags&NOHANG)
		return;
	unit = minor(tp->t_dev);
	dhumodem[unit] = MODEM_BADCALL;
  	tp->t_state &= ~(TS_CARR_ON|TS_TTSTOP|TS_BUSY|TS_ISUSP);
	wakeup((caddr_t)&tp->t_rawq);
	gsignal(tp->t_pgrp, SIGHUP);
	gsignal(tp->t_pgrp, SIGCONT);
	dhuaddr->csr.low = DHU_RIE|(unit&LINEMASK);
	wbflush();
	dhuaddr->lnctrl &= ~(DHU_DTR|DHU_RTS);  /* turn off DTR */
	wbflush();
}
dhu_start_tty(tp)
	register struct tty *tp;
{
	int unit = minor(tp->t_dev);
	tp->t_state &= ~TS_ONDELAY;
	tp->t_state |= TS_CARR_ON;
	if (dhudebug)
	       mprintf("dhu_start_tty:  tp=%x\n", tp);
	if (dhumodem[unit]&MODEM_DSR)
		untimeout(dhu_dsr_check, tp);
	dhumodem[unit] |= MODEM_CD|MODEM_CTS|MODEM_DSR;
	dhutimestamp[unit] = dhuzerotime;
	if (tp->t_state & TS_MODEM_ON) {
		tp->t_state |= TS_ISOPEN;
		tp->t_state &= ~TS_WOPEN;
	}
	wakeup((caddr_t)&tp->t_rawq);
}

/*
dhubaudrate(speed)
register int speed;
{
    if (dhu_valid_speeds & (1 << speed))
	return (1);
    else
	return (0);
}
*/
#endif
