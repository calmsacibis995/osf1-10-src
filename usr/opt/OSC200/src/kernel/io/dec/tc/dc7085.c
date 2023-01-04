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
static char	*sccsid = "@(#)$RCSfile: dc7085.c,v $ $Revision: 1.2.11.2 $ (DEC) $Date: 1993/05/10 02:17:55 $";
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
 * derived from dc7085.c	4.6      (ULTRIX)  10/24/90";
 */

/*
 * dc7085.c
 *
 * DC7085 SLU console driver
 *
 *
 *   10-Oct-91	Mile Larson
 *	Conditionally print bootstrap debug messages.
 *	PTT conformance changes.
 *	Force DS5100 console serial port to be 8-bits, no parity, 9600 baud.
 *
 *   06-Jun-91 	khh
 *	Posix and Xopen comformance work done.
 *
 *   06-Mar-91  map (Mark Parenti)
 *	Modify to use new I/O data structures.
 *
 *   14-Dec-90	khh
 *	Merged mipsmate driver into dc7085.c.  The new driver will 
 *	support pmax, 3max, and mipsmate.
 *
 * OSF work started above this line.
 *-----------------------------------------------------------------------
 * Modification history
 *
 *   13-Sep-90 Joe Szczypek
 *	Added new TURBOchannel console ROM support.  osconsole environment
 *	variable now returns 1 slot number is serial line, else 2 slot numbers
 *	if graphics.  Use this to determine how to do setup.  Note that new
 *	ROMs do not support multiple outputs...
 *
 *   4-Aug-90 - Randall Brown
 *	Made modifications to allow the driver to be used on multiple controllers.
 *	Implemented the 'slu' interface for graphics drivers to callback in
 *	to the console driver to access mouse and keyboard.
 *
 *  6-Jul-1990	- Kuo-Hsiung Hsieh
 * 	Fixed data corrupted problem due to setting break condition
 *	on a transmission line.  On DC type of chip, a specific delay
 *	period has to be imposed on the transmission line if the next
 *	thing to transmit is a break condition.  Data could be corrupted
 *	even though TRDY bit may say it is ready to send the next character.
 *
 *  25-Apr-1990 - Kuo-Hsiung Hsieh
 *      Disable line transmission before cleaning up break bit.
 *      Prevented null character being generated following a break
 *      condition.
 *
 *  23-Mar-1990 - Kuo-Hsiung Hsieh
 *	Corrected the improper sequence of closing down the line.
 *	This correction will prevent tty inmaturely turning off
 *	flow control (fails to restart line drive) while there are
 *	still more data in the buffer. ttyclose() should be called 
 *	before clearing up any terminal attributes.  
 *
 *  8-Dec-1989 - Randall Brown
 *  	Added full modem support.  Used variables to describe register
 *	conditions instead of constants because pmax and 3max have the
 *	bits in different places in certain registers.
 *
 *  6-Dec-89 - Randall Brown
 *
 *	Added the support to allow the device to determine if baudrate is 
 *	supported before it is set in the tty data structures.
 *
 *  9-Nov-1989 - Randall Brown
 *	In dc_putc(), take the line number as an argument, don't figure it
 *	out from consDev.
 *
 * 29-Oct-1989 - Randall Brown
 *	Added support for cons_init.  The code from the probe routine is
 * 	now in dc_cons_init.  Added wbflush() to dc_putc to let data get
 *	to chip to clear interrupt.
 *
 * 16-Oct-1989 - Randall Brown
 *	Added autoconfiguration support.
 *
 * 11-Jul-1989 - Randall Brown
 *	Changed all sc->dc_tty to dc_tty since the dc_tty[] structure is not
 *	part of the softc structure anymore.
 *
 * 15-Aug-89 - Randall Brown
 *
 *	Changed all references of TCSADFLUSH to TCSAFLUSH 
 *
 * 21-Jul-89 - Randall Brown
 *
 *	Moved default open and default close code to tty.c and call it
 *	using tty_def_open() and tty_def_close().  In the close routine,
 *	don't clear the baudrate so that subsequent opens will keep the
 *	present attributes.  This only applies to a Berkeley environment.
 *
 * 12-Jun-1989 - dws
 *	Added trusted path support.
 *
 * 28-Apr-1989 - Randall Brown
 *	Changed a while loop in cn_putc to a DELAY(5).  This was to fix a 
 *	problem that the system would hang if there was output to the
 *	console and another serial line was active.
 *
 * 24-Feb-1989 - Randall Brown
 *	Changed close routine to look at HUPCL in cflag, instead of HUPCLS in
 *	state flag.
 *
 * 28-Dec-1988 - Randall Brown
 *	Changed when the break bits were being set in the chip.  Previously
 * 	they were being set in the ioctl, but this could cause data 
 *	corruption because the state of the chip is unknown.  The bits are
 *	now set in the transmitter interrupt routine, where the state of the
 * 	chip is known.
 *
 * 22-Dec-1988 - Randall Brown
 *	Changed the open routine to return ENXIO on the open of /dev/xcons
 *	when the graphic device is not being used.
 *
 * 16-Dec-1988 - Randall Brown
 *	Added Pseudo DMA code to the transmit side of the driver.  The start
 *	routine sets up the pointers in the pdma struct for the number
 * 	of continuous chars in the outq.  When a transmitter interrupt is
 *	serviced, the pdma struct is checked to see if there are any more
 *	chars to be output, if not it call dcxint(), to fill in the
 * 	pointers again. 
 *
 * 17-Nov-1988 - Randall Brown
 *	Added modem support.  The driver only looks at DSR, and ignores
 * 	CD and CTS.  Also cleaned up driver so that names are consistent.
 *	Also fixed some problems with byte reads and writes (ie. removed
 *	all byte read and writes, and made them half-word reads and writes)
 *
 *  7-Jul-1988 - rsp (Ricky Palmer)
 *	Created file. Contents based on ss.c file.
 *
 */

#include <data/dc_data.c>
#define cprintf printf

int	dcprobe(), dcprobe_reg(), dcattach(), dcattach_ctlr(), dcrint();
int	dc_dsr_check(), dc_tty_drop(), dc_cd_drop(); /* Modem */
int	cons;	/* make osf happy */
caddr_t dcstd[] = { 0 };

int rcline=0;	/* defined in dc_tty.c */

struct	driver dcdriver = { dcprobe_reg, 0, dcattach_ctlr, 0, 0, dcstd, 0, 0, "dc", dcinfo };

#define GRAPHIC_DEV 0x2 /* pick up from pm header file later */

#define LINEMASK	0x03		/* line unit mask */
#define LINEBITS	2		/* line unit mask */

/*
* The SLU doesn't interrupt on carrier transitions, so
* we have to use a timer to watch it.
*/
char	dc_timer = 0;		/* timer started? for dc0 */
char	dc_timer1 = 0;		/* timer started? for dc1 */
int	dc_base_board = 0;	/* defines whether there is a dc controller*/
                                /* on the base board that is used for the  */
				/* graphics devices 			   */

/*
 * Baud Rate Support
 *
 * When the baud rate on the right is specified, the line parameter register
 * is setup with the appropriate bits as specified in the left column.
 */
#define BAUD_UNSUPPORTED 0	/* Device does not provide this baud rate */
#define BAUD_SUPPORTED   1	/* Device does provide this baud rate     */

/*
 * PMAX does not support 19.2K, but on 3MAX,
 * if the BAUD38 bit of the System Control and Status Register is set the
 * chip can do 38400 baud in which case EXTB would be supported.  This bit
 * applies to all 4 lines such that it is not possible to simultaneously do
 * 19200 and 38400.  To keep things simple, only provide support for 19.2.
 *
 * The option card supports 19.2K.  The setting of this being supported is
 * taken care of in the attach routine.
 */

/* ttspeedtab() routine in tty.c can be used to return the corresponding 
   entry in the following table.
 */

struct speedtab dc_speeds[] = {
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
	{B19200,	BAUD_UNSUPPORTED},		/* EXTA  */
	{B38400,	BAUD_UNSUPPORTED}, 		/* EXTB  */
	-1,		-1

   };

struct speedtab ss_speeds[] = 
{
      B0,	0,
      B50,   	DC_B50,
      B75,   	DC_B75,
      B110,  	DC_B110,
      B134, 	DC_B134_5,
      B150, 	DC_B150,
      B300, 	DC_B300,
      B600, 	DC_B600,
      B1200, 	DC_B1200,
      B1800, 	DC_B1800,
      B2400, 	DC_B2400,
      B4800, 	DC_B4800,
      B9600, 	DC_B9600,
      B19200, 	DC_B19200,
      -1,    -1
};

extern int 	consDev;	/* Is console a graphic device ? 	*/
extern int	console_line;	/* Non graphic device console line # 	*/
extern int	console_baud;	/* Console baud rate			*/ 

#define DC_OPTION_OFFSET	0x80000		/* register offset from beginning of slot */

/*
int	dcstart(), dcxint(), dcbuadrate();
*/
int	dcstart(), dcxint(), dcparam();
int	ttrstrt();

/*
 * Graphics device driver entry points.
 * Used to call graphics device driver as needed.
 */
extern	(*vs_gdopen)();
extern	(*vs_gdclose)();
extern	(*vs_gdread)();
extern	(*vs_gdwrite)();
extern	(*vs_gdselect)();
extern	(*vs_gdkint)();
extern	(*vs_gdioctl)();
extern  (*vs_gdmmap)();
extern	(*vs_gdstop)();
extern int (*v_consgetc)();
extern int (*v_consputc)();
extern int pmcons_init();
extern int prom_getenv();
extern int rex_getenv();	/* New TURBOchannel interface */
extern int cpu;

extern int kdebug_state();

/* minumum delay value for setting a break condition.  If we set
 * a break condition without delaying this minimum interval, we
 * might corrupt character which is still in the shift register.
 * The delay values are calculated based on the following equation;
 * 12 (bits/char) * 256 (hz) / baudrate + 2 (safety factor).
 */
u_char    dc_delay[] =
{ 0,78,42,30,25,22,0,12,7,5,4,3,2,2,2,0 };

#ifdef DCDEBUG
int dcdebug = 0;

#define PRINT_SIGNALS() { cprintf("Modem signals: "); \
	if (sc->dcmsr&dc_rdsr[2]) cprintf(" DSR2 "); \
	if (sc->dcmsr&dc_rcts[2]) cprintf(" CTS2 "); \
	if (sc->dcmsr&dc_rcd[2]) cprintf(" CD2 "); \
	if (sc->dcmsr&dc_rdsr[3]) cprintf(" DSR3 "); \
	if (sc->dcmsr&dc_rcts[3]) cprintf(" CTS3 "); \
	if (sc->dcmsr&dc_rcd[3]) cprintf(" CD3 "); \
	cprintf("\n"); } \
/*	cprintf("sc->dcmsr %x : %x\n", &(sc->dcmsr), sc->dcmsr);*/
#endif /* DCDEBUG */


/*	osf calls cnprobe mips_init and pass the controller */
/*	cnprobe calls dcprobe in cons_sw.c */

dcprobe_reg(reg, ctlr)
int reg, ctlr;
{
#ifdef DCDEBUG
	if (dcdebug)
    		printf("dcprobe_reg called\n");
#endif
    return(1);
}

int dcprobe(cntlr)
int cntlr;
{
        /* the intialization is done through dc_cons_init, so
	 * if we have gotten this far we are alive so return a 1
	 */
	dcattach(cntlr);
	return(1);
}

dcattach_ctlr(ctlr)
struct controller *ctlr;
{
    register int ctlr_num = ctlr->ctlr_num;
    register struct dc_softc *sc = &dc_softc[ctlr_num];
    register unsigned char modem_unit;
    int i;

#ifdef DCDEBUG
	if (dcdebug)
    		printf("dcattach_ctrl called\n");
#endif
    dcsoftCAR[ctlr_num] = 0x0f;
    dcdefaultCAR[ctlr_num] = 0x0f;
    dc_brk[ctlr_num] = 0;

    switch (cpu) {
	  case DS_5000:
	    if (ctlr_num != 0) {
		sc->sc_regs = (struct dc_reg *)(ctlr->addr + DC_OPTION_OFFSET);
		dc_cnt += NDCLINE;
	    }
	    break;
	  case DS_5000_100:
	    sc->sc_regs = (struct dc_reg *)(ctlr->addr + DC_OPTION_OFFSET);
	    dc_cnt += NDCLINE;
	    for (i = 0; dc_speeds[i].sp_speed != -1; i++) {
		if (dc_speeds[i].sp_speed == B19200)
	    	   dc_speeds[i].sp_code = BAUD_SUPPORTED;
	    }
	    break;
	  case DS_5100:
#ifdef DCDEBUG
	if (dcdebug)
    		printf("ctlr_num is %d\n", ctlr_num);
#endif	
	    if (ctlr_num == 1) {
		 sc->sc_regs = (struct dc_reg *)DS5100_DC1_BASE;
		 dc_cnt += NDCLINE;
	    }
	    if (ctlr_num == 2) {
		 sc->sc_regs = (struct dc_reg *)DS5100_DC2_BASE;
		 dc_cnt += NDCLINE;
	    }
	    if (ctlr_num == 0 || ctlr_num == 1) {
		if (ctlr_num == 0)
			modem_unit = 2;
		else
			modem_unit = 6;
#ifdef DCDEBUG
	if (dcdebug)
    		printf("dcattach : modem_unit is %d\n", modem_unit);
#endif
		dc_rdtr[modem_unit] = DS5100_DC_L2_DTR;
		dc_rrts[modem_unit] = DS5100_DC_L2_RTS;
	    	dc_rss[modem_unit] = DS5100_DC_L2_SS;
		dc_rcd[modem_unit] = DS5100_DC_L2_CD;
		dc_rdsr[modem_unit] = DS5100_DC_L2_DSR;
		dc_rcts[modem_unit] = DS5100_DC_L2_CTS;
		dc_xmit[modem_unit] = DS5100_DC_L2_XMIT;
		dc_modem_line[modem_unit] = 1;
	    	sc->dctcr &= ~(dc_rdtr[modem_unit] | dc_rrts[modem_unit]);
	    }
	    break;
    }
}

dcattach(ctrl)
int ctrl;
{
	register struct dc_softc *sc = &dc_softc[ctrl];
	register int i;
	register int unit;
	extern dcscan();

#ifdef DCDEBUG
	if(dcdebug)
    		printf("dcattach : ctrl is %d\n", ctrl);
#endif
	switch (cpu) {
	  case DS_3100:
	    dc_base_board = 1;
	    dc_modem_ctl = 0;	/* PMAX has limited modem control */
	    /* line 2 definitions */
	    dc_rdtr[2] = DS3100_DC_L2_DTR;
	    dc_rdsr[2] = DS3100_DC_L2_DSR;
	    dc_xmit[2] = DS3100_DC_L2_XMIT;
	    dc_rrts[2] = dc_rcts[2] = dc_rcd[2] = 0;
	    dc_modem_line[2] = 1;
	    /* line 3 definitions */
	    dc_rdtr[3] = DS3100_DC_L3_DTR;
	    dc_rdsr[3] = DS3100_DC_L3_DSR;
	    dc_xmit[3] = DS3100_DC_L3_XMIT;
	    dc_rrts[3] = dc_rcts[3] = dc_rcd[3] = 0;
	    dc_modem_line[3] = 1;
	    sc->dctcr &= ~(dc_rdtr[2] | dc_rrts[2] | dc_rdtr[3] | dc_rrts[3]);
	    break;

	  case DS_5000:
	     if (ctrl == 0) {
		dc_base_board = 1;
		dc_modem_ctl = 1;	/* 3max has full modem control */
		/* line 2 definitions */
		dc_rdtr[2] = DS5000_DC_L2_DTR;
		dc_rrts[2] = DS5000_DC_L2_RTS;
		dc_rcd[2] = DS5000_DC_L2_CD;
		dc_rdsr[2] = DS5000_DC_L2_DSR;
		dc_rcts[2] = DS5000_DC_L2_CTS;
		dc_xmit[2] = DS5000_DC_L2_XMIT;
		dc_modem_line[2] = 1;
		/* line 3 definitions */
		dc_rdtr[3] = DS5000_DC_L3_DTR;
		dc_rrts[3] = DS5000_DC_L3_RTS;
		dc_rcd[3] = DS5000_DC_L3_CD;
		dc_rdsr[3] = DS5000_DC_L3_DSR;
		dc_rcts[3] = DS5000_DC_L3_CTS;
		dc_xmit[3] = DS5000_DC_L3_XMIT; 
		dc_modem_line[3] = 1;
	    	sc->dctcr &= ~(dc_rdtr[2] | dc_rrts[2] | dc_rdtr[3] | dc_rrts[3]);
	    }
	    for (i = 0; dc_speeds[i].sp_speed != -1; i++) {
		if (dc_speeds[i].sp_speed == B19200)
	    		dc_speeds[i].sp_code = BAUD_SUPPORTED;
	    }
	    break;

	  case DS_5100:		/* mipsmate */
	    /*
	     * If the option card is in place this routine may end up
	     * being called more than once. Use dc_attach_called to
	     * insure that the body of this routine is only executed 
	     * once.
	     */
	     if (dc_attach_called == 0) {
		 dc_attach_called = 1;
		 dc_base_board = 0;
		 dc_modem_ctl = 1;	/* mipsmate has full modem control on
					   line 2 and line 6 */
		 /*
		  * Setup per-DC registers and software status.
		  */
		 if (nDC <= 0) {
			panic("dcattach: no units configured.\n");
		 }
/* With merged driver this check is obsolete. */
#ifdef	notdef
		 if (nDC > DS5100_MAX_NDC) {
			nDC = DS5100_MAX_NDC;
			cprintf("Too many DC chips configured: nDC = %d\n",nDC);
		 }
#endif	/* notdef */		
	        for (i = 0; dc_speeds[i].sp_speed != -1; i++) {
		    if (dc_speeds[i].sp_speed == B19200)
	    		dc_speeds[i].sp_code = BAUD_SUPPORTED;
	    	}
	     }
	     break;
	    
	  default:
		/* we cannot use printf here */
	    printf("Unknown cpu type in dcattach()\n");
	    break;
	}

	/*
 	 * Initialize the scanner process.  Since the chip does not 
	 * interrupt on modem transitions it is necessary to have a scanner
	 * thread that occasionally checks the modem leads and looks for
	 * changes.  Start up this to examine the modem leads once per second.
	 *
	/* Start the modem scan timer */
	/* here cause OSF/1 panic at timeout table overflow
	   the reason I belived is that callfree is not initialized in
	   OSF yet.
	if (!dc_timer) {
	   timeout(dcscan, (caddr_t)0, hz);
	   dc_timer = 1;
	}
	*/
	/* hack here for panic timeout table overflow, osf specific */
	for (unit = 0; unit < nDC * NDCLINE; unit++)
		queue_init(&dc_tty[unit].t_selq);
}

dc_cons_init()
{
       int i, temp_reg;
       int tmp1;                         /* ROM DCDEBUG only */
       register struct dc_softc *sc;

       extern int console_magic;
       extern int (*vcons_init[])();
       int dc_mouse_init(), dc_mouse_putc(), dc_mouse_getc();
       int dc_kbd_init(), dc_kbd_putc(), dc_kbd_getc(), dc_putc();

       sc = &dc_softc[CONSOLE_UNIT];
       switch (cpu) {
	 case DS_3100:	
		sc->sc_regs = (struct dc_reg *)DS3100_DC_BASE; 
		dc_cnt += NDCLINE;
		break;
	 case DS_5000:	
		sc->sc_regs = (struct dc_reg *)DS5000_DC_BASE; 
		dc_cnt += NDCLINE;
		break;
	 case DS_5100:	
		sc->sc_regs = (struct dc_reg *)DS5100_DC0_BASE; 
		dc_cnt += NDCLINE;
		break;
	 default:	printf("Unknown cpu type in dc_cons_init()\n"); break;
       }

       if(cpu == DS_5100) {
	register int prom_baud; 
	int speed_param;

	/* mipsmate console will default to 9600 baud.  This can be
	 * changed via the baud environment variable.  Read in the
	 * console baud rate for later use.
	 */
	console_line = DS5100_CONSOLE_LINE;	/* line 0 */

	prom_baud = atoi(prom_getenv("baud0"));

	switch (prom_baud) {
		case 75:	console_baud = B75; 	break;
		case 110:	console_baud = B110; 	break;
		case 150:	console_baud = B150; 	break;
		case 300:	console_baud = B300; 	break;
		case 600:	console_baud = B600; 	break;
		case 1200:	console_baud = B1200; 	break;
		case 2400:	console_baud = B2400; 	break;
		case 4800:	console_baud = B4800; 	break;
		case 9600:	console_baud = B9600; 	break;
		default:	console_baud = B9600;
	}
	speed_param = ttspeedtab (console_baud, ss_speeds);
	sc->dclpr = console_line | speed_param | BITS8 | DC_RE;

       } else {
       /*
	*
	* Query the prom. The prom can be set such that the user 
	* could use either the alternate tty or the graphics console.
	* You get the graphics console if the first bit is set in
	* osconsole.  The user sets the console variable
	*/
       int isgraphic;
       	if (console_magic != 0x30464354) 
	  isgraphic = ((atoi(prom_getenv("osconsole")) & 0x1) == 1) ? 1 : 0; 
       	else
	  isgraphic = ((strlen(rex_getenv("osconsole"))) > 1) ? 1 : 0;

	if (isgraphic) {
	   slu.mouse_init = dc_mouse_init;
	   slu.mouse_putc = dc_mouse_putc;
	   slu.mouse_getc = dc_mouse_getc;
	   slu.kbd_init = dc_kbd_init;
	   slu.kbd_putc = dc_kbd_putc;
	   slu.kbd_getc = dc_kbd_getc;
	   slu.slu_tty = dc_tty;
	   slu.slu_putc = dc_putc;
	   for( i = 0 ; vcons_init[i] ; i++ )
	       if ((*vcons_init[i])()) {	/* found a virtual console */
		   consDev = GRAPHIC_DEV;
		   break;
	       }
	} else {
	   rcline = 3; /* osf use rcline in pm drivers */
	   console_line = WS_DC_DIAG_LINE;	/* line 3 */
	   sc->dclpr = (DC_RE | DC_B9600 | BITS8 | console_line);  /* set up line 3 console line */
	   sc->dctcr = (DC_TCR_EN_3);  /* line 3 transmit enable */
	   sc->dccsr = (DC_MSE);	/* master scan enable */
        }
       } /* DS_3100 | DS_5000 */
}

dcopen(dev, flag)
	dev_t dev;
{
	register struct dc_softc *sc;
	register struct tty *tp;
	register int ctrl, unit, linenum;
	register int maj, error;
	int inuse;  /*hold state of inuse bit while blocked waiting for carr*/

	if (!dc_timer) {
	   timeout(dcscan, (caddr_t)0, hz);
	   dc_timer = 1;
	}

	/* Start timer routine for dc1 if cpu equals DS_5100 and we have a
	   dht80 configured */
	if ((cpu == DS_5100) && (nDC > 1)) {
		if (!dc_timer1) { 
	   		timeout(dcscan, (caddr_t)1, hz);
	   		dc_timer1 = 1;
		}
	}

	maj = major(dev);
	unit = minor(dev);
	ctrl = unit >> LINEBITS; 

/* rpbfix: need to check to see if the board is actually in the system */
	sc = &dc_softc[ctrl];
	XPR(XPR_TTY, ("dcopen 0x%x", sc->dccsr, 0, 0, 0));

	/*
	 * If a diagnostic console is attached to SLU line,
	 * don't allow open of the printer port.
	 * This could cause lpr to write to the console.
	 *
	 * This is only true for the base board option.
	 */
	if (dc_base_board && (ctrl == 0))
	    if((consDev != GRAPHIC_DEV) && (unit == console_line))
		    return (ENXIO);

	/*
	 * don't allow open of the line kdebug is using
	 */
	if (kdebug_state() && (unit == 2))
	    return (ENXIO);

	/* don't allow open of minor device 0 of major device DCMAJOR */
	/* if this is a base board option, because it is already      */
	/* reserved for /dev/console */
	if (dc_base_board && (maj != CONSOLEMAJOR) && (unit == 0))
	    return (ENXIO);

#if !defined(OSF) && !defined(__OSF__)
	if(dc_base_board) {
	  /* only allow open of /dev/console of major device 0 */
	  if ((maj == CONSOLEMAJOR) && (unit != 0))
	      return (ENXIO);
        }
#endif

	if (dc_base_board)
	 if ((consDev != GRAPHIC_DEV) && (maj == CONSOLEMAJOR) && (unit == 0))
		unit = console_line;	/* diag console on SLU line 3 */
	linenum = unit & LINEMASK;

	if (unit >= dc_cnt)
		return (ENXIO);
	tp = &dc_tty[unit];
	/*
	 * Call the graphics device open routine
	 * if there is one and the open if for the fancy tube.
	 */
	if (dc_base_board && (ctrl == 0))
	    if (vs_gdopen && (unit <= 1)) {
		error = (*vs_gdopen)(dev, flag);
		/* osf call dcparam in pm routine. */
		/* turn on interrupts for kbd and mouse */
		/*
		if (error == 0)
		     dcparam(tp, &tp->t_termios); 
		*/
		return (error);
	    }

#if defined(OSF) || defined(__OSF__)
	if(dc_base_board) {
	  /* only allow open of /dev/console of major device 0 */
	  if ((maj == CONSOLEMAJOR) && (unit != console_line))
	      return (ENXIO);
	}
#endif

	while (tp->t_state&TS_CLOSING) { /* let DTR stay down for awhile */
		if (error = tsleep((caddr_t)&tp->t_rawq, TTIPRI | PCATCH,
				ttopen, 0))
			return (error);
	}
	tp->t_addr = (caddr_t)tp;
	tp->t_oproc = dcstart;
	tp->t_param = dcparam;
/*
	tp->t_baudrate = dcbaudrate;
*/

	if ((tp->t_state & TS_ISOPEN) == 0) {
		
#ifdef DCDEBUG
	if (dcdebug)
	  printf("dcopen unit = %d, t_dev = %d\n", unit, tp->t_dev);
#endif
	    tp->t_line = TTYDISC;
	    tty_def_open(tp, dev, flag, (dcsoftCAR[ctrl]&(1<<linenum)));

	    /*
	     * Prevent spurious startups by making the 500ms timer
	     * initially high.
	     */
	    dcmodem[unit] = MODEM_DSR_START;
	    /*
	     * Specify console terminal attributes.  Do not allow modem control
	     * on the console.  Setup <NL> to <CR> <LF> mapping.
	     */
            if((maj == CONSOLEMAJOR) && ((minor(dev) & LINEMASK) == 0)) {
		tp->t_cflag = CS8 | CREAD; 
		tp->t_ispeed = tp->t_ospeed = TTYDEF_SPEED;
		/*
		tp->t_cflag &= ~CBAUD;
		tp->t_cflag |= B9600;
		tp->t_cflag_ext &= ~CBAUD;
		tp->t_cflag_ext |= B9600;
		*/
	      /* modem control not supported on console */ 
		tp->t_cflag |= CLOCAL; 
		tp->t_flags = ANYP|ECHO|CRMOD;
		tp->t_iflag |= ICRNL; /* Map CRMOD */
		tp->t_oflag |= ONLCR; /* Map CRMOD */
	    }
	    dcparam(tp, &tp->t_termios);  /* enables interrupts */
	}
#if	SEC_BASE
	else if (tp->t_state&TS_XCLUDE && !privileged(SEC_ALLOWDACACCESS, 0)) 
#else
	else if (tp->t_state&TS_XCLUDE && u.u_uid != 0) 
#endif
	{
		return (EBUSY);
	}
	/* dcparam(tp, &tp->t_termios); */	/* enables interrupts */
	(void) spltty();

	/*
	 * No modem control provided for lines with softCAR set.
	 * Modem control provided only for lines 2 and 3 of the base board.
	 */
	if (dc_modem_line[unit] == 0) 
	   	tp->t_cflag |= CLOCAL;

	if (tp->t_cflag & CLOCAL) {
		/*
		 * This is a local connection - ignore carrier
		 * receive enable interrupts enabled above via dcparam()
		 */
		tp->t_state |= TS_CARR_ON;		/* dcscan sets */
		if (dc_modem_line[unit]) 
		   sc->dctcr |= (dc_rdtr[unit] | dc_rrts[unit] | dc_rss[unit]);
		/*
         	 * Set the CTS state flag if both DSR and CTS are active.
         	 * This is for non-modem connections that reset CLOCAL.
         	 */
		if (dc_modem_line[unit])
		 if (((sc->dcmsr)&dc_rdsr[unit]) && ((sc->dcmsr)&dc_rcts[unit]))
                	dcmodem[unit] |= MODEM_CTS;
		
		/*
		 * Set state bit to tell tty.c not to assign this line as the
		 * controlling terminal for the process which opens this line.
		 */
		 /* osf does not check this here.  This is a common code.
		    it should be deal in tty.c  !!!!!
		if ((flag & O_NOCTTY) && (u.u_procp->p_progenv == A_POSIX))
			tp->t_state |= TS_ONOCTTY;
		*/
		(void) spl0();
		return ((*linesw[tp->t_line].l_open)(dev, tp, flag));
	}
	/*
	 *  this is a modem line
	 */
	/* receive enable interrupts enabled above via dcparam() */

	sc->dctcr |= (dc_rdtr[unit] | dc_rrts[unit] | dc_rss[unit]);

	/*
	 * After DSR first comes up we must wait for the other signals
	 * before commencing transmission.
         */
#ifdef DCDEBUG
	if (dcdebug) 
	{
		cprintf("open flag : %x\n", flag);
		if (flag & (O_NDELAY|O_NONBLOCK)) 
			cprintf("flag & (O_NDELAY|O_NONBLOCK)\n");
	}
#endif /* DCDEBUG */
	if ((flag & (O_NDELAY|O_NONBLOCK)) == 0) {
		/*
		 * Delay before examining other signals if DSR is being followed
		 * otherwise proceed directly to dc_dsr_check to look for
		 * carrier detect and clear to send.
		 */
#ifdef DCDEBUG
		if (dcdebug) {
			cprintf("dcopen: ");
			PRINT_SIGNALS();
		}
#endif /* DCDEBUG */
		if ((sc->dcmsr)&dc_rdsr[unit]) {
			dcmodem[unit] |= (MODEM_DSR_START|MODEM_DSR);
			tp->t_dev = dev; /* need it for timeouts */
			if (!dc_modem_ctl) {
			    /*
			     * Assume carrier will come up in less
			     * than 1 sec. If not DSR will drop
			     * and the line will close
			     */
			    timeout(dc_dsr_check, tp, hz);
			} else {
			    /*
			     * Give CD and CTS 30 sec. to 
			     * come up.  Start transmission
			     * immediately, no longer need
			     * 500ms delay.
			     */
			    timeout(dc_dsr_check, tp, hz*30);
			    dc_dsr_check(tp);
			}
		}
	}
#ifdef DCDEBUG
	if (dcdebug)
		cprintf("dcopen:  line=%d, state=%x, tp=%x\n", unit,
			tp->t_state, tp);
#endif /* DCDEBUG */
	if (flag & (O_NDELAY|O_NONBLOCK))
		tp->t_state |= TS_ONDELAY;
	else
		while ((tp->t_state & TS_CARR_ON) == 0) {
			tp->t_state |= TS_WOPEN;
			inuse = tp->t_state&TS_INUSE;
#ifdef DCDEBUG
			if (dcdebug) {
				cprintf("dc_open: going to sleep\n");
			}
#endif /* DCDEBUG */
			if (error = tsleep((caddr_t)&tp->t_rawq, 
				TTIPRI | PCATCH, ttopen, 0)) {
				(void) spl0();
				return (error);
			}
			/*
			 * See if we were awoken by a false call to the modem
			 * line by a non-modem.
			 */
			if (dcmodem[unit]&MODEM_BADCALL){
				(void) spl0();
				return(EWOULDBLOCK);
			}
			/* if we opened "block if in use"  and
			 *  the terminal was not inuse at that time
			 *  but is became "in use" while we were
			 *  waiting for carrier then return
			 */
			if ((flag & O_BLKINUSE) && (inuse==0) &&
				(tp->t_state&TS_INUSE)) {
					(void) spl0();
					return(EALREADY);
			}
		}
	/*
	 * Set state bit to tell tty.c not to assign this line as the
	 * controlling terminal for the process which opens this line.
	 */
	/*
	if ((flag & O_NOCTTY) && (u.u_procp->p_progenv == A_POSIX))
		tp->t_state |= TS_ONOCTTY;
	*/
	(void) spl0();
	return ((*linesw[tp->t_line].l_open)(dev, tp, flag));
}

dcclose(dev, flag)
	dev_t dev;
{
	register struct dc_softc *sc;
	register struct tty *tp;
	register int unit, ctrl, maj, linenum;
	register int s;
	extern int wakeup();

	unit = minor(dev);
	ctrl = unit >> LINEBITS;
	maj = major(dev);
	sc = &dc_softc[ctrl];

	if (dc_base_board) {	/* This is a graphic workstation */
	  if((consDev != GRAPHIC_DEV) && (maj == CONSOLEMAJOR) && (unit == 0))
		unit |= console_line;	/* diag console on SLU line */
	  /*
	   * Call the craphics device close routine
	   * if ther is one and the close is for it.
	   */
	  if (ctrl == 0)
	    if (vs_gdclose && (unit <= 1)) {
		(*vs_gdclose)(dev, flag);
		return;
	    }
	}
	linenum = unit & LINEMASK;

	if (unit >= dc_cnt)
		return (ENXIO);

	XPR(XPR_TTY, ("cnclose 0x%x", sc->dccsr, 0, 0, 0));

	tp = &dc_tty[unit];
	/*
	 * Do line discipline specific close functions then return here
	 * in the old line disc for final closing.
	 */
/*
	osf calls line specific close routine. 
	if (tp->t_line)
*/
		(*linesw[tp->t_line].l_close)(tp);
	/*
	 * dcbrk is write-only and sends a BREAK (SPACE condition) until
	 * the break control bit is cleared. Here we are clearing any
	 * breaks for this line on close.
	 */
	if (dc_brk[ctrl] & (1 << (linenum + 0x8))) {
	  s = spltty();
	  switch (cpu) {
	      case DS_3100: {
		     short save_tcr = sc->dctcr;
		     sc->dctcr &= 0xfff0;
		     sc->dcbrk_tbuf = (dc_brk[ctrl] &= ~(1 << (linenum + 0x8)));
		     wbflush();
		     sc->dctcr = save_tcr;
	  	    break;
	      }
	      case DS_5100: /* break is byte writeable in mipsmate 	   */ 
	      case DS_5000: /* dcbrk is a char on 3max and option card */
	      case DS_5000_100:
	  	  sc->dcbrk = ((dc_brk[ctrl] &= ~(1 << (linenum + 0x8))) >> 8);
	  	  wbflush();
	  }
	  splx(s);
	}
	if ((tp->t_cflag&HUPCL) || (tp->t_state&TS_WOPEN) ||
	    (tp->t_state&TS_ISOPEN)==0) {
		tp->t_state &= ~TS_CARR_ON;   /* prevents recv intr. timeouts */
		/*
		 * Drop appropriate signals to terminate the connection.
		 */
		dcmodem_active[ctrl] &= ~(1 << linenum);
		sc->dctcr &= ~(dc_rdtr[unit] | dc_rrts[unit]);
		if ((tp->t_cflag & CLOCAL) == 0) {
		    s = spltty();
		    /*drop DTR for at least a sec. if modem line*/
		    sc->dctcr &= ~(dc_rdtr[unit] | dc_rrts[unit]);
		    tp->t_state |= TS_CLOSING;
#ifdef DCDEBUG
	if (dcdebug)
			cprintf("dcclose: DTR drop, state =%x\n"
				,tp->t_state);
#endif /* DCDEBUG */
		    /*
		     * Wait at most 5 sec for DSR to go off.
		     * Also hold DTR down for a period.
		     */
		    if ((sc->dcmsr & dc_rdsr[unit]) || 
			!(sc->dcmsr & dc_rcd[unit])) {
			timeout(wakeup,(caddr_t)&tp->t_dev,5*hz);
			sleep((caddr_t)&tp->t_dev, PZERO-10);
		    }
		    /*
		     * Hold DTR down for 200+ ms.
		     */
		    timeout(wakeup, (caddr_t) &tp->t_dev, hz/5);
		    sleep((caddr_t)&tp->t_dev, PZERO-10);
		    
		    tp->t_state &= ~(TS_CLOSING);
		    wakeup((caddr_t)&tp->t_rawq);
		    splx(s);
		}
		/*
		 * No disabling of interrupts is done.	Characters read in on
		 * a non-open line will be discarded.
		 */
	}
	/* reset line to default mode */
	dcsoftCAR[ctrl] &= ~(1<<(unit&LINEMASK));
	dcsoftCAR[ctrl] |= (1<<(unit&LINEMASK)) & dcdefaultCAR[ctrl];
	dcmodem[unit] = 0;
	/* ttyclose() must be called before clear up termio flags */
	ttyclose(tp);
/*
	tty_def_close(tp);
*/
}

dcread(dev, uio, flag)
	dev_t dev;
	struct uio *uio;
	int flag;	/* don't know why need flag , khh */
{
	register struct tty *tp;
	register int unit;

	unit = minor(dev);

	if(dc_base_board)	/* There is a graphic head */
	  if((consDev != GRAPHIC_DEV) && (major(dev) == CONSOLEMAJOR) && (unit == 0))
		unit |= console_line; 	/* diag console on SLU line */
#ifdef DCDEBU
	cprintf("dcread: unit = %d, tp = 0x%x\n", unit, tp);
#endif
	
	tp = &dc_tty[unit];
	return ((*linesw[tp->t_line].l_read)(tp, uio, flag));
}

dcwrite(dev, uio, flag)
	dev_t dev;
	struct uio *uio;
	int flag;	/* don't know why need flag */
{
	register struct tty *tp;
	register int unit, ctrl;

	unit = minor(dev);
	ctrl = unit >> LINEBITS;

	if(dc_base_board) {	
	  if((consDev != GRAPHIC_DEV) && (major(dev) == CONSOLEMAJOR) && (unit == 0))
		unit |= console_line; 	/* diag console on SLU line */
	
	  /*
	   * Don't allow writes to the mouse,
	   * just fake the I/O and return.
	   */
	  if (ctrl == 0) 
	    if (vs_gdopen && (unit == 1)) {
		uio->uio_offset = uio->uio_resid;
		uio->uio_resid = 0;
		return(0);
	    }
	
	}
#ifdef DCDEBU
	cprintf("dcwrite: unit = %d, tp = 0x%x\n", unit, tp);
#endif
	tp = &dc_tty[unit];
	return ((*linesw[tp->t_line].l_write)(tp, uio, flag));
}

/*
dcselect(dev, rw)
dev_t dev;
dcselect(dev_t dev, short *events, short *revents, int scanning)
*/
dcselect(dev, events, revents, scanning)
dev_t dev;
short *events, *revents;
int scanning;
{
	register int ctrl, unit;

	unit = minor(dev);
	ctrl = unit >> LINEBITS;
#ifdef DCDEBUG
	if (dcdebug)
	  printf("dcselect, dev = %x, ctrl = %d, unit = %d\n", dev, ctrl, unit);
#endif
	if((consDev != GRAPHIC_DEV) && (major(dev) == CONSOLEMAJOR) && (unit == 0))
		dev |= console_line;
	if (dc_base_board && (ctrl == 0))
	    if ((unit == 1) && vs_gdselect) {
#ifdef DCDEBUG
	if (dcdebug)
	  printf("calling vs_gdselect = %x\n", vs_gdselect);	       
#endif
		/*
		return((*vs_gdselect)(dev, rw));
		*/
		return((*vs_gdselect)(dev, events, revents, scanning));
	    }
	/*
	return(ttselect(dev, rw));
	*/
	return(ttselect(dev, events, revents, scanning));
}

/* osf pass ep pointer, need to find out the controller number */
dcintr(ctrl)
unsigned int ctrl;
{
	register struct dc_softc *sc;
	register unsigned int csr;

	/* int ctrl = 0; */       /* hack, just to make the code work. khh */
	sc = &dc_softc[ctrl];

	csr = sc->dccsr;
	if (csr & DC_TRDY)
		_dcpdma(ctrl);
	if (csr & DC_RDONE) 
		dcrint(ctrl);
}

cn_intr(ep)
	unsigned int *ep;
{
	dcintr(0);
}

int mips_pollc = 0;

cnpollc(flag)
	int flag;
{
	if (flag)
		mips_pollc++;
	else
		mips_pollc--;
}


/*
 * Used to pass mouse (or tablet) reports to the graphics
 * device driver interrupt service routine.
 * Entire report passed instead of byte at a time.
 */
struct	mouse_report	current_rep;
#define MOUSE_ID	0x2
u_short pointer_id = MOUSE_ID;

dcrint(ctrl)
int ctrl;
{
	register struct dc_softc *sc;
	register struct tty *tp;
	register u_short c;	/* rbuf register is 16 bits long */
	register int ct;
	register int unit, linenum;
	register int flg;
	int overrun = 0;
	struct mouse_report *new_rep;
	u_short data;
	int counter = 0;

	/* OSF stuff */
	if (mips_pollc)
		return;

	sc = &dc_softc[ctrl];
	XPR(XPR_TTY, ("cnrint 0x%x", sc->dccsr, 0, 0, 0));
	new_rep = &current_rep;			/* mouse report pointer */
	while (sc->dccsr&DC_RDONE) {		/* character present */
		c = sc->dcrbuf;

		linenum = ((c >> 8) & LINEMASK);
		unit = (ctrl * NDCLINE) + linenum;
		tp = &dc_tty[unit];

		if (tp >= &dc_tty[dc_cnt])
			continue;
		/*
		 * If console is a graphics device,
		 * pass keyboard input characters to
		 * its device driver's receive interrupt routine.
		 * Save up complete mouse report and pass it.
		 */
		if ((dc_base_board) && (ctrl == 0) && (unit <= 1) && vs_gdkint) {
		    if(unit == 0) {		/* keyboard char */
			(*vs_gdkint)(c);
			continue;
		    } else {			/* mouse or tablet report */
			if (pointer_id == MOUSE_ID) { /* mouse report */
			    data = c & 0xff;	/* get report byte */
			    ++new_rep->bytcnt;	/* inc report byte count */

			    if (data & START_FRAME) { /* 1st byte of report? */
				new_rep->state = data;
				if (new_rep->bytcnt > 1)
				    new_rep->bytcnt = 1;  /* start new frame */
			    }

			    else if (new_rep->bytcnt == 2) {	/* 2nd byte */
				new_rep->dx = data;
			    }

			    else if (new_rep->bytcnt == 3) {	/* 3rd byte */
				    new_rep->dy = data;
				    new_rep->bytcnt = 0;
				    (*vs_gdkint)(0400); /* 400 says line 1 */
			    }
			    continue;
			} else { /* tablet report */
			    data = c;	/* get report byte */
			    ++new_rep->bytcnt;	/* inc report byte count */

			    if (data & START_FRAME) { /* 1st byte of report? */
				new_rep->state = data;
				if (new_rep->bytcnt > 1)
				    new_rep->bytcnt = 1;  /* start new frame */
			    }

			    else if (new_rep->bytcnt == 2)	/* 2nd byte */
				    new_rep->dx = data & 0x3f;

			    else if (new_rep->bytcnt == 3)	/* 3rd byte */
				    new_rep->dx |= (data & 0x3f) << 6;

			    else if (new_rep->bytcnt == 4)	/* 4th byte */
				    new_rep->dy = data & 0x3f;

			    else if (new_rep->bytcnt == 5){	/* 5th byte */
				    new_rep->dy |= (data & 0x3f) << 6;
				    new_rep->bytcnt = 0;
				    (*vs_gdkint)(0400); /* 400 says line 1 */
			    }
			    continue;
			}
		    }
		}
		if ((tp->t_state & TS_ISOPEN) == 0) {
			wakeup((caddr_t)&tp->t_rawq);

			/*
			 * force a break if kdebug's ui is trying to interrupt
			 */
			if (kdebug_state() && (unit == 2)) {
				kdebug_fakebreak();
				return;
			}

/* osf stuff */
#ifdef PORTSELECTOR
                        if ((tp->t_state&TS_WOPEN) == 0)
#endif
			continue;
		}

		ct = c & TTY_CHARMASK;
#ifdef DCDEBUG
	if (dcdebug)
			cprintf("dcrint: c = %d, ct = %c\n", c, ct);
#endif /* DCDEBUG */

		/* DC_FE is interpreted as a break */
		if (c & DC_FE) {
			if (unit == rcline) gimmeabreak();
#ifdef DCDEBUG
	if (dcdebug)
			cprintf("dcrint: Framming Error\n");
#endif /* DCDEBUG */
			ct |= TTY_FE;
			/* osf does not provide trusted path */
		}
		/* Parity Error */
		if (c & DC_PE){
#ifdef DCDEBUG
	if (dcdebug)
			printf("dcrint: Parity Error\n");
#endif /* DCDEBUG */
			ct |= TTY_PE;
		}

		/* SVID does not say what to do with overrun errors */
		if (c&DC_DO) {
			if(overrun == 0) {
				printf("dc%d: input silo overflow\n", ctrl);
				overrun = 1;
			}
			sc->sc_softcnt[linenum]++;
		}

#if NHC > 0
		if (tp->t_line == HCLDISC) {
			HCINPUT(c, tp);
		} else
#endif
		(*linesw[tp->t_line].l_rint)(ct, tp);
	}
}

static char cnbrkstr[] = "cnbrk";

/*ARGSUSED*/
dcioctl(dev, cmd, data, flag)
	dev_t dev;
	register int cmd;
	caddr_t data;
	int flag;
{
	register struct dc_softc *sc;
	register int ctrl, unit, linenum;
	register struct tty *tp;
	register int s;
	struct device *device;
	struct devget *devget;
	int error;

	unit = minor(dev);
	ctrl = unit >> LINEBITS;
	sc = &dc_softc[ctrl];

	XPR(XPR_TTY, ("dcioctl 0x%x", sc->dccsr, 0, 0, 0));
	if (dc_base_board) {
	   if((consDev != GRAPHIC_DEV) && (major(dev) == CONSOLEMAJOR) 
		&& (unit == 0))
		unit |= console_line;	/* diag console on SLU line */
	}
	linenum = unit & LINEMASK;
	/*
	 * If there is a graphics device and the ioctl call
	 * is for it, pass the call to the graphics driver.
	 */
	if (dc_base_board && (ctrl == 0))
	    if (vs_gdioctl && (unit <= 1)) {
		return((*vs_gdioctl)(dev, cmd, data, flag));
	    }
	tp = &dc_tty[unit];
	error = (*linesw[tp->t_line].l_ioctl)(tp, cmd, data, flag);
	if (error >= 0)
		return (error);
	error = ttioctl(tp, cmd, data, flag);

	if (error >= 0)
                return (error);

	switch (cmd) {
	case TCSBREAK: {
                /* Wait for one character time before sending break */
                int tmo = (12*hz)/tp->t_ospeed + 2;
		if ((dc_brk[ctrl] & (1 << (linenum + 0x8))) == 0) {
                  mpsleep((caddr_t)&sc->sc_flags[linenum], PZERO-10, cnbrkstr,
                        tmo, NULL, 0);
                  s = spltty();
		  switch (cpu) {
		    case DS_3100: {
		       short save_tcr = sc->dctcr;
		       /* disable all lines transmission */
		       sc->dctcr &= 0xfff0;
		       sc->dcbrk_tbuf = (dc_brk[ctrl] |= (1 << (linenum + 0x8)));
		       wbflush();
		       /* restore lines transmission */
		       sc->dctcr = save_tcr;
                       break;
		    }
                    case DS_5100: /* break is byte writeable in mipsmate */
                    case DS_5000: /* dcbrk is a char on 3MAX and option card */
		    case DS_5000_100:
                       sc->dcbrk = ((dc_brk[ctrl] |= (1 << (linenum + 0x8))) >>
 8);
                  }
                  wbflush();
		  splx(s);
		}
                mpsleep((caddr_t)&sc->sc_flags[linenum], PZERO-10, cnbrkstr,
                        hz/4, NULL, 0);
		if (dc_brk[ctrl] & (1 << (linenum + 0x8))) {
		  s = spltty();
		  switch (cpu) {
		     case DS_3100: {
		       short save_tcr = sc->dctcr;
		       /* disable all lines transmission */
		       sc->dctcr &= 0xfff0;
		       sc->dcbrk_tbuf = (dc_brk[ctrl] &= ~(1 << (linenum + 0x8)));
		       wbflush();
		       /* restore lines transmission */
		       sc->dctcr = save_tcr;
		       break;
		     }
	       	     case DS_5100: /* break is byte writeable in mipsmate  */ 
	  	     case DS_5000: /* dcbrk is a char on 3max and option card */
		     case DS_5000_100:
		        sc->dcbrk = ((dc_brk[ctrl] &= ~(1 << (linenum + 0x8))) 
				>> 8);
                  	wbflush();
		  }
                  splx(s);
		}
                break;
	}
		
	case TIOCSBRK: 		/* ULTRIX compatible ioctl */
		{
		/* only set break bit if it is not already set */
		int tmo = (12*hz)/tp->t_ospeed + 2;
		if ((dc_brk[ctrl] & (1 << (linenum + 0x8))) == 0) {
		   mpsleep((caddr_t)&sc->sc_flags[linenum], PZERO-10, 
		     cnbrkstr, tmo, NULL, 0);
		  s = spltty();
		  switch (cpu) {
		    case DS_3100: {
		       short save_tcr = sc->dctcr;
		       /* disable all lines transmission */
		       sc->dctcr &= 0xfff0;
		       sc->dcbrk_tbuf = (dc_brk[ctrl] |= (1 << (linenum + 0x8)));
		       wbflush();
		       /* restore lines transmission */
		       sc->dctcr = save_tcr;
		       break;
		    }
		    case DS_5100: /* break is byte writeable in mipsmate     */
		    case DS_5000: /* dcbrk is a char on 3MAX and option card */
		    case DS_5000_100:
			sc->dcbrk = ((dc_brk[ctrl] |= (1 << (linenum + 0x8))) >> 8);
		  }
		  splx(s);
		}
		break;
	}
	case TIOCCBRK:		/* ULTRIX compatible ioctl */
		if (dc_brk[ctrl] & (1 << (linenum + 0x8))) {
		  s = spltty();
		  switch (cpu) {
		     case DS_3100: {
		       short save_tcr = sc->dctcr;
		       /* disable all lines transmission */
		       sc->dctcr &= 0xfff0;
		       sc->dcbrk_tbuf = (dc_brk[ctrl] &= ~(1 << (linenum + 0x8)));
		       wbflush();
		       /* restore lines transmission */
		       sc->dctcr = save_tcr;
			  break;
		     }
	  	     case DS_5100: /* break is byte writeable in mipsmate  */ 
	  	     case DS_5000: /* dcbrk is a char on 3max and option card */
		    case DS_5000_100:
			  sc->dcbrk = ((dc_brk[ctrl] &= ~(1 << (linenum + 0x8))) >> 8);
/*
		      	  wbflush();
*/
		  }
		  splx(s);
		}
		break;

	case TIOCSDTR:
		(void) dcmctl(dev, DC_DTR | DC_RTS, DMBIS);
		break;

	case TIOCCDTR:
		(void) dcmctl(dev, DC_DTR | DC_RTS, DMBIC);
		break;

	case TIOCMSET:
		(void) dcmctl(dev, dmtodc(*(int *)data), DMSET);
		break;

	case TIOCMBIS:
		(void) dcmctl(dev, dmtodc(*(int *)data), DMBIS);
		break;

	case TIOCMBIC:
		(void) dcmctl(dev, dmtodc(*(int *)data), DMBIC);
		break;

	case TIOCMGET:
		*(int *)data = dctodm(dcmctl(dev, 0, DMGET));
		break;

	case TIOCNMODEM:  /* ignore modem status */
		/*
		 * By setting the software representation of modem signals
		 * to "on" we fake the system into thinking that this is an
		 * established modem connection.
		 */
		s = spltty();
		dcsoftCAR[ctrl] |= (1<<(linenum&LINEMASK));
		if (*(int *)data) /* make mode permanent */
			dcdefaultCAR[ctrl] |= (1<<(linenum&LINEMASK));
		tp->t_state |= (TS_CARR_ON|TS_ISOPEN);
		tp->t_cflag |= CLOCAL;		/* Map to termio */
		splx(s);
		break;

	case TIOCMODEM:  /* look at modem status - sleep if no carrier */
		s = spltty();
		dcsoftCAR[ctrl] &= ~(1<<(linenum&LINEMASK));
		if (*(int *)data) /* make mode permanent */
			dcdefaultCAR[ctrl] &= ~(1<<(linenum&LINEMASK));
		/*
		 * See if signals necessary for modem connection are present
		 *
		 * dc7085 chip on PMAX only provides DSR
		 *
		 */
		if ((sc->dcmsr & dc_xmit[unit]) == dc_xmit[unit]) {
			tp->t_state &= ~(TS_ONDELAY);
			tp->t_state |= TS_CARR_ON;
			dcmodem[unit] = MODEM_CTS|MODEM_CD|MODEM_DSR;
		} else {
			tp->t_state &= ~(TS_CARR_ON|TS_ISOPEN);
			dcmodem[unit] &= ~(MODEM_CTS|MODEM_CD|MODEM_DSR);
		}
		tp->t_cflag &= ~CLOCAL;		/* Map to termio */
		splx(s);
		break;

/*	undocumented feature	*/
	case DEVIOCGET:				/* device status */
		devget = (struct devget *)data;
		bzero(devget,sizeof(struct devget));
		if (dc_modem_line[unit])
		    if (tp->t_cflag & CLOCAL) {
			sc->sc_category_flags[unit&LINEMASK] |= DEV_MODEM;
			sc->sc_category_flags[unit&LINEMASK] &= ~DEV_MODEM_ON;
		    } else
			sc->sc_category_flags[unit&LINEMASK] |= (DEV_MODEM|DEV_MODEM_ON);
		devget->category = DEV_TERMINAL; 	/* terminal cat.*/
		devget->bus = DEV_NB;		 	/* NO bus	*/
		bcopy(DEV_VS_SLU,devget->interface,
		      strlen(DEV_VS_SLU));	 	/* interface	*/
		bcopy(DEV_UNKNOWN,devget->device,
		      strlen(DEV_UNKNOWN));	 	/* terminal	*/
		devget->adpt_num = 0;		 	/* NO adapter	*/
		devget->nexus_num = 0;		 	/* fake nexus 0 */
		devget->bus_num = 0;		 	/* NO bus	*/
		devget->ctlr_num = ctrl;	 	/* cntlr number */
		devget->slave_num = unit&LINEMASK; 	/* line number	*/
		bcopy("dc", devget->dev_name, 3); 	/* Ultrix "dc"	*/
		devget->unit_num = unit&LINEMASK; 	/* dc line?	*/
		devget->soft_count =
		      sc->sc_softcnt[unit&LINEMASK]; 	/* soft err cnt */
		devget->hard_count =
		      sc->sc_hardcnt[unit&LINEMASK]; 	/* hard err cnt */
		devget->stat = sc->sc_flags[unit&LINEMASK];  /* status	*/
		devget->category_stat =
		      sc->sc_category_flags[unit&LINEMASK];  /* cat. stat. */
		break; 

	default:
	/*
		if (u.u_procp->p_progenv == A_POSIX) 
			return (EINVAL);
	*/
		return (ENOTTY);
	}
	return (0);
}

dcmmap(dev, off, prot)
        dev_t dev;
        off_t off;
        int prot;
{
        register struct dc_softc *sc;
        int ctrl, unit;
        extern int noconsmmap();

        unit = minor(dev);
        ctrl = unit >> LINEBITS;
        sc = &dc_softc[ctrl];

        XPR(XPR_TTY, ("dcmmap 0x%x", sc->dccsr, 0, 0, 0));
        if (dc_base_board) {
           if((consDev != GRAPHIC_DEV) && (major(dev) == CONSOLEMAJOR)
                && (unit == 0))
                unit |= console_line;   /* diag console on SLU line */
        }
        /*
         * If there is a graphics device pass the call to the graphics
         * driver.  Otherwise, tell the caller that we don't support mapping
         * of that offset.
         */
        if (dc_base_board && (ctrl == 0))
            if (vs_gdioctl && (unit <= 1)) {
                return((*vs_gdmmap)(dev, off, prot));
            }

        return (noconsmmap(dev, off, prot));
}

dmtodc(bits)
	register int bits;
{ 
	register int b;

	b = (bits >>1) & 0370;
	if (bits & SML_ST) b |= DC_ST;
	if (bits & SML_RTS) b |= DC_RTS;
	if (bits & SML_DTR) b |= DC_DTR;
	if (bits & SML_LE) b |= DC_LE;
	return(b);
}

dctodm(bits)
	register int bits;
{
	register int b;

	b = (bits << 1) & 0360;
	if (bits & DC_DSR) b |= SML_DSR;
	if (bits & DC_DTR) b |= SML_DTR;
	if (bits & DC_ST) b |= SML_ST;
	if (bits & DC_RTS) b |= SML_RTS;
	return(b);
}

/*
dcparam(unit)
	register int unit;
*/
#ifdef _ANSI_C_SOURCE
dcparam(register struct tty *tp, register struct termios *t)
#else
dcparam(tp, t)
register struct tty *tp;
register struct termios *t;
#endif
{
	register struct dc_softc *sc;
	register u_short lpr;
	register int ctrl, s;
	int unit = minor(tp->t_dev);
	int ospeed = ttspeedtab(t->c_ospeed, ss_speeds);
	int retval = 0;

	ctrl = unit >> LINEBITS;
	sc = &dc_softc[ctrl];
#ifdef DCDEBUG
	if (dcdebug)
	  printf("dcparam unit = %d, t_dev = %d\n", unit, tp->t_dev);
#endif
	s = spltty();

/*
 *	Reversing the order of the following two lines fixes the
 *	problem where the console device locks up if you type a
 *	character during autoconf and you must halt/continue to
 *	unlock the console. Interrupts were being enabled on the SLU
 *	before the ssact flag was set, so the driver would just return
 *	and not process the waiting character (caused by you typing).
 *	This locked up the cosnole SLU (interrupt posted but never really
 *	servcied). Halting the system caused the console firmware to unlock
 *	the SLU because it needs to use it.
 *	Should dcparam() be called as spl5??????
 */
	if ((tp->t_state & TS_ISOPEN) && !(tp->t_cflag & CLOCAL) &&
						        (t->c_ospeed == B0)) {
	    dcmodem_active[ctrl] &= ~(1 << (unit & LINEMASK));
	    sc->dctcr &= ~(dc_rdtr[unit] | dc_rrts[unit]); /* hang up line */
	    splx(s);
	    return 0;
	}
	/*
	 * If diagnostic console on line 3,
	 * line parameters must be: 9600 BPS, 8 BIT, NO PARITY, 1 STOP.
	 * Same for color/monochrome video, except 4800 BPS.
	 * Mouse/tablet: 4800 BPS, 8 BIT, ODD PARITY, 1 STOP.
	 * If none of the above, assume attached console on line 0,
	 * same paramaters as diagnostic console on line 3.
	 *
	 * Force line parameters on MipsMate, too.
	 */
	if ((dc_base_board && (ctrl == 0)) ||
	    ((cpu == DS_5100) && (ctrl == 0) &&
	     ((unit & LINEMASK) == DS5100_CONSOLE_LINE))) {

	/* This is non-graphic console.  The unit number is reset
	 * to 3 in open.  However, the information in tp is still
	 * set to minor 0.
	 */

	if (ttspeedtab(t->c_ospeed, dc_speeds) == BAUD_UNSUPPORTED)  
		retval = EINVAL;	/* don't set an invalid speed */

	if ((unit == 0) && (consDev != GRAPHIC_DEV)) {
		sc->dclpr = (DC_RE | DC_B9600 | BITS8 | 3);
		sc->dccsr = DC_MSE | DC_TIE | DC_RIE;
		splx(s);
		return(retval);
	    }
	if (unit <= 1) {
		sc->dccsr = DC_MSE |DC_TIE | DC_RIE;
		splx(s);
		return(retval);
	    }
	}

	/* check requested parameters */ 

	/* If input baud rate is set to zero, the input baud rate will
	   be specified by the value of the output baud rate */

	if (ospeed < 0 || (t->c_ispeed ? (t->c_ispeed != t->c_ospeed) 
			: !(t->c_ispeed = t->c_ospeed)))
		return EINVAL;
	
	if (ttspeedtab(t->c_ospeed, dc_speeds) == BAUD_UNSUPPORTED)  
		return (EINVAL);

        /* and copy to tty */
        tp->t_ispeed = t->c_ispeed;
        tp->t_ospeed = t->c_ospeed;
        tp->t_cflag = t->c_cflag;

	lpr = (DC_RE) | ospeed | (unit & LINEMASK);
/*
	lpr = (DC_RE) | (dc_speeds[tp->t_ospeed&CBAUD].baud_param) | (unit & LINEMASK);
	lpr = (dc_speeds[tp->t_ospeed].baud_param) | (unit & LINEMASK);
*/
	/*
	 * Berkeley-only dinosaur
	 */
	 /*
	 OSF does not have this
	if (tp->t_line != TERMIODISC) {
	    if ((tp->t_cflag_ext&CBAUD) == B110){
		lpr |= TWOSB;
		tp->t_cflag |= CSTOPB;
	    }
	}
	*/
	/*
	 * Set device registers according to the specifications of the
	 * termio structure.
	 */
	if ((tp->t_cflag & CREAD) == 0)
	    lpr &= ~DC_RE;	/* This was set from speeds */
	if (tp->t_cflag & CSTOPB)
	    lpr |= TWOSB;
	else
	    lpr &= ~TWOSB;
	if (tp->t_cflag & PARENB) {
	    if ((tp->t_cflag & PARODD) == 0)
		/* set even */
		lpr = (lpr | PENABLE)&~OPAR;
	    else
		/* else set odd */
		lpr |= PENABLE|OPAR;
	}
	/*
	 * character size.
	 * clear bits and check for 6,7,and 8, else its 5 bits.
	 */
	lpr &= ~BITS8;
	switch(tp->t_cflag&CSIZE) {
	  case CS6:
	    lpr |= BITS6;
	    break;
	  case CS7:
	    lpr |= BITS7;
	    break;
	  case CS8:
	    lpr |= BITS8;
	    break;
	}
#ifdef DCDEBUG
	if (dcdebug)
	  printf("dcparam: lpr = 0x%x, t_cflag = 0x%x\n", lpr, tp->t_cflag);
#endif
	sc->dclpr = lpr;
	
	sc->dccsr = DC_MSE | DC_TIE | DC_RIE;
	splx(s);
	/* osf need to return 0 */
	return 0;
}

_dcpdma(ctrl)
int ctrl;
{
    	register struct dc_softc *sc;
	register struct dcpdma *dp;
	register int unit, linenum;

	sc = &dc_softc[ctrl];

	linenum = (sc->dccsr >> 8) & LINEMASK;
	unit = (ctrl * NDCLINE) + linenum;
	dp = &sc->dc_pdma[linenum];
	
	if (dp->p_mem == dp->p_end) {
	    dcxint(unit);
	} else {
	    sc->dcbrk_tbuf = ((dc_brk[ctrl]) | (unsigned char)(*dp->p_mem++));
	}
}

dcxint(unit)
	register int unit;
{
	register struct dc_softc *sc;
	register struct dcpdma *dp;
	register struct tty *tp;
	register int ctrl;

	ctrl = unit >> LINEBITS;
	sc = &dc_softc[ctrl];

	if (dc_base_board && (consDev != GRAPHIC_DEV))
	  if ((unit == 0) && (major(tp->t_dev) == CONSOLEMAJOR)) {
		unit = console_line;
	}
	tp = &dc_tty[unit];


	tp->t_state &= ~TS_BUSY;
	dp = &sc->dc_pdma[unit & LINEMASK];
	if (tp->t_state & TS_FLUSH) {
		tp->t_state &= ~TS_FLUSH;
	} 
	else {
	    	ndflush(&tp->t_outq, dp->p_mem-tp->t_outq.c_cf);
		dp->p_end = dp->p_mem = tp->t_outq.c_cf;
	}
	

	/* call line start routine */
	(*linesw[tp->t_line].l_start)(tp);

	/* The BUSY flag will not be set in two cases:		*/
	/*   1. if there are no more chars in the outq OR	*/
	/*   2. there are chars in the outq but tty is in	*/
	/*      stopped state.					*/
	if ((tp->t_state&TS_BUSY) == 0) {
	    	sc->dctcr &= ~(1<< (unit & LINEMASK));
	}
}

dcstart(tp)
	register struct tty *tp;
{
	register struct dc_softc *sc;
	register struct dcpdma *dp;
	register int cc;
	int s, unit, ctrl;

	unit = minor(tp->t_dev);
	ctrl = unit >> LINEBITS;
	sc = &dc_softc[ctrl];

#ifdef DCDEBUG
	if (dcdebug == 4)
	printf("dcstart unit = %d, t_dev = %d\n", unit, tp->t_dev);
#endif
	s = spltty();
	/*
	 * Do not do anything if currently delaying, or active.  Also only
	 * transmit when CTS is up.
	 */
	if ((tp->t_state & (TS_TIMEOUT|TS_BUSY|TS_TTSTOP)) ||
	(dc_modem_line[unit] && ((tp->t_cflag & CLOCAL) == 0) 
		&& (tp->t_state&TS_CARR_ON) && ((dcmodem[unit]&MODEM_CTS)==0)))
		goto out;
/*
	if (tp->t_outq.c_cc <= TTLOWAT(tp)) {
*/
	if (tp->t_outq.c_cc <= tp->t_lowat) {
		if (tp->t_state&TS_ASLEEP) {
			tp->t_state &= ~TS_ASLEEP;
			wakeup((caddr_t)&tp->t_outq);
		}
		/* osf select staff */
		select_wakeup(&tp->t_selq);
/*
		if (tp->t_wsel) {
			selwakeup(tp->t_wsel, tp->t_state & TS_WCOLL);
			tp->t_wsel = 0;
			tp->t_state &= ~TS_WCOLL;
		}
*/
	}
	if (tp->t_outq.c_cc == 0)
		goto out;

#ifdef DCDEBUG
	if (dcdebug == 4)
	printf("dcstart: unit = %d, tp = %x, c_cc = %d\n", unit, tp, tp->t_outq.c_cc);
#endif 
	/* OSF does not have this, why do we need to check the output
	   process for delaying 

	if ((tp->t_lflag_ext & PRAW) || (tp->t_oflag_ext & PLITOUT) ||
	    ((tp->t_oflag & OPOST) == 0))
		cc = ndqb(&tp->t_outq, 0);
	else {
	*/
		cc = ndqb(&tp->t_outq, DELAY_FLAG);
		if (cc == 0) {
			cc = getc(&tp->t_outq);
			timeout(ttrstrt, (caddr_t)tp, (cc&0x7f) + 6);
			tp->t_state |= TS_TIMEOUT;
			goto out;
		}
	/* } */
	tp->t_state |= TS_BUSY;
	/* 
	 * A workstation without graphic monitor.
         */
	if (dc_base_board && (consDev != GRAPHIC_DEV))
	  if ((unit == 0) && (major(tp->t_dev) == CONSOLEMAJOR))
		unit = console_line;
	dp = &sc->dc_pdma[unit & LINEMASK];
	dp->p_end = dp->p_mem = tp->t_outq.c_cf;
	dp->p_end += cc;
#ifdef DCDEBUG
	if (dcdebug == 4)
	printf("dcstart: unit = %d, dp->mem = %x, c_cc = %d\n", unit, dp->p_mem, cc);
#endif 
	sc->dccsr |= DC_TIE;
	sc->dctcr |= (1<< (unit & LINEMASK));
out:
	splx(s);
}

dcstop(tp, flag)
	register struct tty *tp;
{
	register struct dc_softc *sc;
	register struct dcpdma *dp;
	register int s;
	int	unit, ctrl;

	/*
	 * If there is a graphics device and the stop call
	 * is for it, pass the call to the graphics device driver.
	 */
	unit = minor(tp->t_dev);
	ctrl = unit >> LINEBITS;
	sc = &dc_softc[ctrl];

	if ((consDev != GRAPHIC_DEV) && (unit == 0) && (major(tp->t_dev) == CONSOLEMAJOR)) {
		unit = console_line;
	}
	if (dc_base_board && (ctrl == 0)) 
	    if (vs_gdstop && (unit <= 1)) {
		(*vs_gdstop)(tp, flag);
		return;
	    }
	dp = &sc->dc_pdma[unit & LINEMASK];
	s = spltty();
	if (tp->t_state & TS_BUSY) {
	    	dp->p_end = dp->p_mem;
		if ((tp->t_state&TS_TTSTOP)==0)
			tp->t_state |= TS_FLUSH;
	}
	splx(s);
}

dcmctl(dev, bits, how)
	dev_t dev;
	int bits, how;
{
	register struct dc_softc *sc;
	register int unit, ctrl, mbits;
	int b, s;

	unit = minor(dev);
	ctrl = unit >> LINEBITS;
	sc = &dc_softc[ctrl];

	if (dc_modem_line[unit] == 0)
		return(0);	/* only line 2 and 3 on base board have modem control */
	s = spltty();
	mbits = (sc->dctcr & dc_rdtr[unit]) ? DC_DTR : 0;
	mbits |= (sc->dctcr & dc_rrts[unit]) ? DC_RTS : 0;
	mbits |= (sc->dcmsr & dc_rcd[unit]) ? DC_CD : 0;
	mbits |= (sc->dcmsr & dc_rdsr[unit]) ? DC_DSR : 0;
	mbits |= (sc->dcmsr & dc_rcts[unit]) ? DC_CTS : 0;
	switch (how) {
	case DMSET:
		mbits = bits;
		break;

	case DMBIS:
		mbits |= bits;
		break;

	case DMBIC:
		mbits &= ~bits;
		break;

	case DMGET:
		(void) splx(s);
		return(mbits);
	}
	if (mbits & DC_DTR)
		sc->dctcr |= (dc_rdtr[unit] | dc_rrts[unit]);
	else
		sc->dctcr &= ~(dc_rdtr[unit] | dc_rrts[unit]);
	(void) splx(s);
	return(mbits);
}

#ifdef DCDEBUG
int dcscan_ctr = 1;
#endif DCDEBUG

/*
 *	WARNING:	This routine should only be called if the dc is on the 
 *			base board.  The option card has NO modem control .
 */
dcscan(ctrl)
int ctrl;
{
	register int i;
	register struct dc_softc *sc = &dc_softc[ctrl];
	register struct tty *tp;
	register u_short dcscan_modem;
	static	u_short dcscan_previous[2];		/* Used to detect modem transitions */
	int loop_start, loop_end;

#ifdef DCDEBUG
	if (dcdebug) {
		if (dcscan_ctr++ == 45) {
			cprintf("dcscan: ");
			PRINT_SIGNALS();
			dcscan_ctr = 1;
		}
	}
#endif DCDEBUG

	if (cpu == DS_5100) {
		if (ctrl == 0) {
			loop_start = 2;
			loop_end = 3;
		}
		if (ctrl == 1) {
			loop_start = 6;
			loop_end = 7;
		}
	}
	else {
		loop_start = 2;
		loop_end = 4;
	}

	dcscan_modem = sc->dcmsr;	/* read copy of modem status register */
	if (dcscan_modem == dcscan_previous[ctrl]) {
	    /* no work to do reschedule scan routine */
	    if (dcmodem_active)
		timeout(dcscan, (caddr_t)ctrl, hz/40);
	    else
		timeout(dcscan, (caddr_t)ctrl, hz);
	    return;
	}
	/* mipsmate need to check the  line number */
	for (i = loop_start; i < loop_end; i++) {
	    tp = &dc_tty[i];
	    if ((tp->t_cflag & CLOCAL) == 0) {
		/*
		 * Drop DTR immediately if DSR has gone away.
		 * If really an active close then do not
		 *    send signals.
		 */
		if (!(dcscan_modem & dc_rdsr[i])) {
		    if (tp->t_state&TS_CLOSING) {
			untimeout(wakeup, (caddr_t) &tp->t_dev);
			wakeup((caddr_t) &tp->t_dev);
		    }
		    if (tp->t_state&TS_CARR_ON) {
			dc_tty_drop(tp);
		    }
		} else {		/* DSR has come up */
		    /*
		     * If DSR comes up for the first time we allow
		     * 30 seconds for a live connection.
		     */
		    if ((dcmodem[i] & MODEM_DSR)==0) {
			dcmodem[i] |= (MODEM_DSR_START|MODEM_DSR);
			if (!dc_modem_ctl) {
			    /*
			     * Assume carrier will come up in less
			     * than 1 sec. If not DSR will drop
			     * and the line will close
			     */
			    timeout(dc_dsr_check, tp, hz);
			} else {
			    /*
			     * we should not look for CTS|CD for about
			     * 500 ms.
			     */
			    timeout(dc_dsr_check, tp, hz*30);
			    dc_dsr_check(tp);
			}
		    }
		}

		/*
		 * look for modem transitions in an already
		 * established connection.
		 *
		 * Ignore CD and CTS for PMAX.  These signals 
		 * don't exist on the PMAX.
		 */
		if (dc_modem_ctl) {
		    if (tp->t_state & TS_CARR_ON) {
			if (dcscan_modem & dc_rcd[i]) {
			    /*
			     * CD has come up again.
			     * Stop timeout from occurring if set.
			     * If interval is more than 2 secs then
			     * drop DTR.
			     */
			    if ((dcmodem[i] & MODEM_CD) == 0) {
				untimeout(dc_cd_drop, tp);
				if (dc_cd_down(tp)) {
				    /* drop connection */
				    dc_tty_drop(tp);
				}
				dcmodem[i] |= MODEM_CD;
			    }
			} else {
			    /*
			     * Carrier must be down for greater than
			     * 2 secs before closing down the line.
			     */
			    if (dcmodem[i] & MODEM_CD) {
				/* only start timer once */
				dcmodem[i] &= ~MODEM_CD;
				/*
				 * Record present time so that if carrier
				 * comes up after 2 secs, the line will drop.
				 */
				dctimestamp[i] = time;
				timeout(dc_cd_drop, tp, hz * 2);
			    }
			}
			
			/* CTS flow control check */
			
			if (!(dcscan_modem & dc_rcts[i])) {
			    /*
			     * Only allow transmission when CTS is set.
			     */
			    tp->t_state |= TS_TTSTOP;
			    dcmodem[i] &= ~MODEM_CTS;
#ifdef DCDEBUG
	if (dcdebug)
				cprintf("dcscan: CTS stop, tp=%x,line=%d\n",tp,i);
#endif DCDEBUG
			    dcstop(tp, 0);
			} else if (!(dcmodem[i] & MODEM_CTS)) {
			    /*
			     * Restart transmission upon return of CTS.
			     */
			    tp->t_state &= ~TS_TTSTOP;
			    dcmodem[i] |= MODEM_CTS;
#ifdef DCDEBUG
	if (dcdebug)
				cprintf("dcscan: CTS start, tp=%x,line=%d\n",tp,i);
#endif DCDEBUG
			    dcstart(tp);
			}
		    }

		    /*
		     * See if a modem transition has occured.  If we are waiting
		     * for this signal, cause action to be take via
		     * dc_start_tty.
		     */
		    if (((dcscan_modem & dc_xmit[i]) == dc_xmit[i]) &&
			(!(dcmodem[i] & MODEM_DSR_START)) &&
			(!(tp->t_state & TS_CARR_ON))) {
#ifdef DCDEBUG
	if (dcdebug)
			    cprintf("dcscan: MODEM transition: dcscan_modem = %x, dcscan_previous = %x\n", dcscan_modem, dcscan_previous[ctrl]);
#endif DCDEBUG
			dc_start_tty(tp);
		    }
		}
	    }
	}
	
	dcscan_previous[ctrl] = dcscan_modem; /* save for next iteration */

	if (dcmodem_active)
	    timeout(dcscan, (caddr_t)ctrl, hz/40);
	else
	    timeout(dcscan, (caddr_t)ctrl, hz);
}

int	dcputc();
int	dcgetc();

dcputc(c)
	register int c;
{
    	if (consDev == GRAPHIC_DEV) 
    	{
        	if ( v_consputc ) {
	    	(*v_consputc) (c);
	    	if ( c == '\n' )
	        	(*v_consputc)( '\r' );
	    	return;
        	}
    	}
    	dc_putc(console_line, c);
    	if ( c == '\n')
		dc_putc(console_line, '\r');
}

/*
 * This routine outputs one character to the console.
 * Characters must be printed without disturbing
 * output in progress on other lines!
 * This routines works with the SLU in interrupt or
 * non-interrupt mode of operation. BULL!
 * Characters are output as follows:
 *	spl5, remember if console line active.
 *	set console line tcr bit.
 *	wait for TRDY on console line (save status if wrong line).
 *	start output of character.
 *	wait for output complete.
 *	if any lines were active, set their tcr bits,
 *	otherwise clear the xmit ready interrupt.
 *
 */
dc_putc(unit, c)
        int unit;
	register int c;
{
	register struct dc_softc *sc = &dc_softc[CONSOLE_UNIT];
	register int	s; 
	register u_short tcr;
	register int	ln, tln = unit, timo;

	s = splhigh();
	tcr = (sc->dctcr & (1<<tln));
	sc->dctcr |= (1<<tln);
	while (1) {
		timo = 1000000;
		while ((sc->dccsr&DC_TRDY) == 0)	/* while not ready */
			if(--timo == 0)
				break;
		if(timo == 0)
			break;
		/*
		* The DC chip has stoppped on a line that is ready to be
		* loaded with another transmit character.  If the line ready
		* for transmission is not the console line then clear
		* transmitter enable for the non-console line so that we
		* won't be bothered by that line again until the completion
		* of this putc operation.
		*/
		ln = (sc->dccsr>>8) & LINEMASK;
		if (ln != tln) {
			tcr |= (1 << ln);
			sc->dctcr &= ~(1 << ln);
			continue;
		}
		
		/* stuff char out, don't upset upper 8 bits. 
		 * Output the character by writing into the transmitter
		 * buffer. Provide a delay to allow time for the char to go
		 * out. The registers should not be read for 1.4
		 * microseconds. Should this routine check to see if the
		 * console line is presently asserting a break?
		 * For consistant, half word write (16 bits word) is used
		 * to output character.
		 */
		sc->dcbrk_tbuf = ((dc_brk[CONSOLE_UNIT]) | (c&0xff));
		DELAY(5); /* alow time for TRDY bit to become valid */
		while (1) {
			timo = 1000000;
			while ((sc->dccsr&DC_TRDY) == 0) {/* while not ready */
				if(--timo == 0)
					break;
			}
			ln = (sc->dccsr>>8) & LINEMASK;
			if (ln != tln) {
				tcr |= (1 << ln);
				sc->dctcr &= ~(1 << ln);
				continue;
			}
			break;
		}
		break;
	}
	sc->dctcr &= ~(1<<tln);
	if (tcr != 0)
		sc->dctcr |= tcr;
	wbflush();
	splx(s);
}

/*
 * This routine operates on the following assumptions:
 * 1. putc must have happened first, so SLU already inited.
 * 2. getc will happed before slu reveive interrupt enabled so
 *    don't need to worry about int_req or int_msk registers.
 */
dcgetc()
{
    register u_short c;
    register int linenum;

    /*
     * Line number we expect input from.
     */
	if(consDev == GRAPHIC_DEV) 
		linenum = 0x0;
	else
		linenum = console_line;

	c = dc_getc(linenum);

	if(v_consgetc)
		return((*v_consgetc)(c & TTY_CHARMASK));
	else
		return(c & TTY_CHARMASK);
}


dc_getc(linenum)
int linenum;
{
    struct dc_softc *sc = &dc_softc[CONSOLE_UNIT];
    register int timo;
    register u_short c;

    for (timo=1000000; timo > 0; --timo) {
	if (sc->dccsr&DC_RDONE) {
		c = sc->dcrbuf;
    		DELAY(50000);
		if(((c >> 8) & LINEMASK) != linenum)
			continue;
		/*
		 * Toss the character away if there is an error.
		 * I wonder if throwing away parity errors is a bit
		 * harsh.
		 */
		if(c&(DC_DO|DC_FE|DC_PE))
			continue;
		return (c & TTY_CHARMASK);
	}
    }
    return (-1);

}

dc_mouse_init()
{
	struct	dc_softc *sc = &dc_softc[0];

	/*
 	 * Set SLU line 1 parameters for mouse communication.
 	 */
	sc->dclpr = (SER_POINTER | SER_CHARW | SER_PARENB | SER_ODDPAR
		| SER_SPEED | SER_RXENAB );
}

dc_mouse_putc(c)
int c;
{
    dc_putc(1, c);
}

dc_mouse_getc()
{
    return (dc_getc(1));
}

dc_kbd_init()
{
    struct	dc_softc *sc = &dc_softc[0];
    
    /*
     * Set the line parameters on SLU line 0 for
     * the LK201 keyboard: 4800 BPS, 8-bit char, 1 stop bit, no parity.
     */
    sc->dclpr = (SER_RXENAB | SER_KBD | SER_SPEED | SER_CHARW);
}

dc_kbd_putc(c)
int c;
{
    dc_putc(0, c);
}

dc_kbd_getc()
{
    return (dc_getc(0));
}
/*
 * Modem Control Routines
 */

/*
 *
 * Function:
 *
 *	dc_cd_drop
 *
 * Functional description:
 *
 * 	Determine if carrier has dropped.  If so call dc_tty_drop to terminate
 * 	the connection.
 *
 * Arguements:
 *
 *	register struct tty *tp  -  terminal pointer ( for terminal attributes )
 *
 * Return value:
 *
 *	none
 *
 */
dc_cd_drop(tp)
register struct tty *tp;
{
        register struct dc_softc *sc;
	register int unit, ctrl;

	unit = minor(tp->t_dev);
	ctrl = unit >> LINEBITS;
	sc = &dc_softc[ctrl];

	if ((tp->t_state & TS_CARR_ON) && (!(sc->dcmsr & dc_rcd[unit]))) {
#ifdef DCDEBUG
	if (dcdebug)
		cprintf("dc_cd_drop: no CD, tp = %x, line = %d\n", tp, unit);
#endif /* DCDEBUG */
	    dc_tty_drop(tp);
	    return;
	}
	dcmodem[unit] |= MODEM_CD;
#ifdef DCDEBUG
	if (dcdebug)
	    cprintf("dc_cd_drop:  CD is up, tp = %x, line = %d\n", tp, unit);
#endif /* DCDEBUG */
}
 
/*
 *
 * Function:
 *
 *	dc_dsr_check
 *
 * Functional description:
 *
 *	DSR must be asserted for a connection to be established.  Here we 
 *	either start or terminate a connection on the basis of DSR.
 *
 * Arguements:
 *
 *	register struct tty *tp  -  terminal pointer (for terminal attributes)
 *
 * Return value:
 *
 *	none
 *
 */
dc_dsr_check(tp)
register struct tty *tp;
{
	register struct dc_softc *sc;
	register int unit, ctrl;

	unit = minor(tp->t_dev);
	ctrl = unit >> LINEBITS;
	sc = &dc_softc[ctrl];

#ifdef DCDEBUG
	if (dcdebug) {
       	    cprintf("dc_dsr_check0:  tp=%x\n", tp);
	    PRINT_SIGNALS();
	}
#endif /* DCDEBUG */

	if (dcmodem[unit] & MODEM_DSR_START) {
	    dcmodem[unit] &= ~MODEM_DSR_START;
	    /*
	     * since dc7085 chip on PMAX only provides DSR then assume that CD
	     * has come up after 1 sec and start tty.  If CD has not
	     * come up the modem should deassert DSR thus closing the line
	     *
	     * On 3max, we look for DSR|CTS|CD before establishing a
	     * connection.
	     */
	    if ((!dc_modem_ctl) || 
		((sc->dcmsr & dc_xmit[unit]) == dc_xmit[unit])) {
		dc_start_tty(tp);
	    }
	    return;
	}
	if ((tp->t_state&TS_CARR_ON)==0)
		dc_tty_drop(tp);
}

/*
 *
 * Function:
 *
 *	dc_cd_down
 *
 * Functional description:
 *
 *	Determine whether or not carrier has been down for > 2 sec.
 *
 * Arguements:
 *
 *	register struct tty *tp  -  terminal pointer ( for terminal attributes )
 *
 * Return value:
 *
 *	1 - if carrier was down for > 2 sec.
 *	0 - if carrier down <= 2 sec.
 *
 */
dc_cd_down(tp)
register struct tty *tp;
{
        register int msecs, unit;

	unit = minor(tp->t_dev);
	msecs = 1000000 * (time.tv_sec - dctimestamp[unit].tv_sec) + 
		(time.tv_usec - dctimestamp[unit].tv_usec);
	if (msecs > 2000000){
#ifdef DCDEBUG
	if (dcdebug)
			cprintf("dc_cd_down: msecs > 20000000\n");
#endif /* DCDEBUG */
		return(1);
	}
	else{
#ifdef DCDEBUG
	if (dcdebug)
			cprintf("dc_cd_down: msecs < 20000000\n");
#endif /* DCDEBUG */
		return(0);
	}
}

/*
 *
 * Function:
 *
 *	dc_tty_drop
 *
 * Functional description:
 *
 *	Terminate a connection.
 *
 * Arguements:
 *
 *	register struct tty *tp  -  terminal pointer ( for terminal attributes )
 *
 * Return value:
 *
 *	none
 *
 */
dc_tty_drop(tp)
struct tty *tp;
{
	register struct dc_softc *sc;
	register int unit, ctrl;

	unit = minor(tp->t_dev);
	ctrl = unit >> LINEBITS;
	sc = &dc_softc[ctrl];

	dcmodem_active[ctrl] &= ~(1 << (unit & LINEMASK));
	if (tp->t_flags & NOHANG)
		return;
#ifdef DCDEBUG
	if (dcdebug)
		cprintf("dc_tty_drop: unit=%d\n",minor(tp->t_dev));
#endif DCDEBUG
	/* 
	 * Notify any processes waiting to open this line.  Useful in the
	 * case of a false start.
	 */
	dcmodem[unit] = MODEM_BADCALL;
	tp->t_state &= ~(TS_CARR_ON|TS_TTSTOP|TS_BUSY|TS_ISUSP);
	wakeup((caddr_t)&tp->t_rawq);
	pgsignal(tp->t_pgrp, SIGHUP, 1);
	pgsignal(tp->t_pgrp, SIGCONT, 1);
	sc->dctcr &= ~(dc_rdtr[unit] | dc_rrts[unit]);
}
/*
 *
 * Function:
 *
 *	dc_start_tty
 *
 * Functional description:
 *
 *	Establish a connection.
 *
 * Arguements:
 *
 *	register struct tty *tp  -  terminal pointer ( for terminal attributes )
 *
 * Return value:
 *
 *	none
 *
 */
dc_start_tty(tp)
	register struct tty *tp;
{
        register int unit, ctrl;

	unit = minor(tp->t_dev);
	ctrl = unit >> LINEBITS;

	dcmodem_active[ctrl] |= (1 << (unit & LINEMASK));
	tp->t_state &= ~(TS_ONDELAY);
	tp->t_state |= TS_CARR_ON;
#ifdef DCDEBUG
	if (dcdebug)
	       cprintf("dc_start_tty:  tp=%x\n", tp);
#endif /* DCDEBUG */
	if (dcmodem[unit] & MODEM_DSR)
		untimeout(dc_dsr_check, tp);
	dcmodem[unit] |= MODEM_CD|MODEM_CTS|MODEM_DSR;
	dctimestamp[unit].tv_sec = dctimestamp[unit].tv_usec = 0;
	if (tp->t_state & TS_MODEM_ON) {
		tp->t_state |= TS_ISOPEN;
	}
	wakeup((caddr_t)&tp->t_rawq);
}
/*
 *	Return 1 if the baud rate is supported, 0 if not supported.
 */
/*
dcbaudrate(speed)
int speed;
{
    return(dc_speeds[speed & CBAUD].baud_support);
}
*/


