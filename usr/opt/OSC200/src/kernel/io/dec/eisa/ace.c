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
static char *rcsid = "@(#)$RCSfile: ace.c,v $ $Revision: 1.1.4.5 $ (DEC) $Date: 1993/12/09 20:25:22 $";
#endif

/*
 * Tim Notes: (as of Mar 3, 1993)
 * 
 * The driver is operational and generally "product quality".  I have tested
 * with the console serial line and another terminal on the second port.
 * Modem testing consisted of using "tip" to call out to another system, and
 * a "getty" with modem enabled was setup to allow incoming calls on the
 * modem.
 *
 * The following work remains to be done:
 * - mmap support (if any is actually needed)
 * - Any changes needed if the graphics driver is the console.  To isolate
 *   these changes look for the keyword GRAPHICSCONSOLE in this code.
 * - Small changes needed when the PAL provides separate interrupt SCB's
 *   for the 2 differnet lines.
 * The following testing needs to be performed:
 * - all the standards conformance test suites.
 * - DEC Standard 52 modem control tests.
 */

/*
 * ace.c
 *
 * Alpha PC console driver
 *
 * Modification history
 *
 * 01-Sept-1992 - Tim Burke
 * 	Comments and other cleanups.
 * Thu Jul 16 09:02:57 1992 tml	
 *	Removed ace_other[] int array.  Not used.
 *	Switch I/O to inVti/outVti instead of VM mapped or in/outportbxt.
 *	Initialized "ct" with rx char in rint.
 *	Booted OSF to single user.
 * 15-Jul-1992 - Win Treese
 * 	created file.
 *
 */

#include <data/ace_data.c> 
#include <io/dec/eisa/eisa.h>		/* EISA bus definitions		*/

#ifndef TS_NEED_PARAM
/*
 * This constant should be defined in tty.h!
 * Another accident waiting to happen when someone comes along and defines
 * this bit to mean something else in tty.h.
 */
#define TS_NEED_PARAM  0x40000000
#endif

/* GRAPHICSCONSOLE */
/* Currently this restricts baud rate changes on the console
 * tty line.  This is fine for the case where there is no graphics console,
 * however it is prohibitive otherwise since that line really isn't the
 * console.  Need to add a test to see if the graphics driver is being used.
 */
int	aceprobe(), aceprobe_reg(), aceattach(), aceattach_ctrl();
int	ace_dsr_check(), ace_tty_drop(), ace_cd_drop(); /* Modem */
void    ace_set_dtr(), ace_clear_dtr();
int	acestart(), acexint(), ace_rint(), aceparam(), aceintr(); 
int	ttrstrt();

struct driver acedriver = { aceprobe, 0, aceattach, 0, 0,
			      acestd, 0, 0, "ace", aceinfo };
extern int kdebug_state();



/*
 * Baud Rate Support
 *
 * When a tty ioctl is called to set the line parameters - the baud rate
 * specifically the ioctl is supposed to fail (according to POSIX) if the
 * baud rate is unsupported.  This array is used to declare which baud
 * rates are supported and which are not.
 */
#define BAUD_UNSUPPORTED 0	/* Device does not provide this baud rate */
#define BAUD_SUPPORTED   1	/* Device does provide this baud rate     */

#define	B56000	56000		/* rps - OK, so it isn't really supported */

struct speedtab ace_speeds[] = {
    {0,          BAUD_UNSUPPORTED}, /* B0    */
    {B50,	 BAUD_SUPPORTED},   /* B50   */
    {B75,	 BAUD_SUPPORTED},   /* B75   */
    {B110,	 BAUD_SUPPORTED},   /* B110  */
    {B134,    	 BAUD_SUPPORTED},   /* B134  */
    {B150,	 BAUD_SUPPORTED},   /* B150  */
    {B200,	 BAUD_SUPPORTED},   /* B200  */
    {B300,	 BAUD_SUPPORTED},   /* B300  */
    {B600,	 BAUD_SUPPORTED},   /* B600  */
    {B1200,	 BAUD_SUPPORTED},   /* B1200 */
    {B1800,	 BAUD_SUPPORTED},   /* B1800 */
    {B2400,	 BAUD_SUPPORTED},   /* B2400 */
    {B4800,	 BAUD_SUPPORTED},   /* B4800 */
    {B9600,	 BAUD_SUPPORTED},   /* B9600 */
    {B19200,   	 BAUD_SUPPORTED},   /* EXTA  */
    {B38400,   	 BAUD_SUPPORTED},   /* EXTB  */
    {B56000,   	 BAUD_SUPPORTED},   /* */
};

/*
 * The baud rate is specified as a 16-bit divisor value.  Since the
 * uart regs are 8-bits it takes 2 registers to represent this value.
 * Therefore a lo and hi pair is used to store the 16-bit value.
 */
struct speedtab ace_lo_speeds[] =
{
	B0,	0,
	B50,	ACE_B50_LO,
	B75,	ACE_B75_LO,
	B110, 	ACE_B110_LO,
	B134,	ACE_B134_5_LO,
	B150,	ACE_B150_LO,
	B300,	ACE_B300_LO,
	B600,	ACE_B600_LO,
	B1200,	ACE_B1200_LO,
	B1800,	ACE_B1800_LO,	
	B2400,	ACE_B2400_LO,
	B4800,	ACE_B4800_LO,
	B9600,	ACE_B9600_LO,
	B19200,	ACE_B19200_LO,
	B38400,	ACE_B38400_LO,
	B56000,	ACE_B56000_LO,
	-1, 	-1
};

struct speedtab ace_hi_speeds[] =
{
	B0,	0,
	B50,	ACE_B50_HI,
	B75,	ACE_B75_HI,
	B110, 	ACE_B110_HI,
	B134,	ACE_B134_5_HI,
	B150,	ACE_B150_HI,
	B300,	ACE_B300_HI,
	B600,	ACE_B600_HI,
	B1200,	ACE_B1200_HI,
	B1800,	ACE_B1800_HI,	
	B2400,	ACE_B2400_HI,
	B4800,	ACE_B4800_HI,
	B9600,	ACE_B9600_HI,
	B19200,	ACE_B19200_HI,
	B38400,	ACE_B38400_HI,
	B56000,	ACE_B56000_HI,
	-1, 	-1
};

/* read/write macros */
#define ACE_READ(base, off, val) val = READ_BUS_D8(((base)+(off)))
#define ACE_WRITE(base, off, val) WRITE_BUS_D8((base)+(off),(val)); \
    mb();

#define ACE_DO_STARTBREAK	0
#define ACE_DO_STOPBREAK	1

#define IP(x)         ( ((x)&ACE_INT_BIT) ? 0 : 1 )
#define RLS_INT(x)    ( ((x)&ACE_RLS_INT) == ACE_RLS_INT ? 1 : 0 )
#define RDA_INT(x)    ( ((x)&ACE_RDA_INT) == ACE_RDA_INT ? 1 : 0 )
#define THRE_INT(x)   ( ((x)&ACE_THRE_INT) == ACE_THRE_INT ? 1 : 0 )
#define MS_INT(x)     ( ((x)&0x0f) == ACE_MS_INT ? 1 : 0 )

aceprobe(addr, ctrl)
caddr_t	addr;
struct controller *ctrl;
{
    register struct ace_softc *sc = acesc;
    int slot;
    int unit;
    struct eisa_info *einf;
    struct e_port port;

    unit = ctrl->ctlr_num;
    slot = ctrl->slot;                 
    einf = ctrl->eisainfo;

#ifdef CONFIG_DEBUG                    /* turn these on for debugging */
    printf("probe: base address=0x%x\n", ctrl->addr );
    printf("        unit=%d\n", unit );
    printf("        physical=0x%x\n", ctrl->physaddr );
    printf("        slot=%d\n", slot );
    printf("        einf=0x%x\n", einf );
    printf("        ivnum=%d\n", ctrl->ivnum );
#endif /* CONFIG_DEBUG */

    if ( slot>0 && einf  )
    {
	if ( get_config( ctrl, EISA_PORT, "", &port, 0 ) == -1 )
	{
	    printf("aceprobe: error return from get_config()\n" );
	}

	sc->ace_regs[unit] = port.base_address;

#ifdef CONFIG_DEBUG
	printf("        base offset = 0x%x\n", sc->ace_regs[unit] );
#endif
    }
    aceinit( unit, NOISY ); 
    return 1;
}

aceattach(ctrl)
struct controller *ctrl;
{
    register struct ace_softc *sc = acesc;

    return 1;
}

/*
 * aceinit() sets the ACE modes
 */
aceinit(register int unit, int verbosity)
{
    register struct ace_softc *sc = acesc;
    register u_long rsp;
    register struct ace_saved_reg *ssp;
    int x, ace_type;
    unsigned int rpat1, rpat2;

    if (unit >= ace_cnt)
    {
	printf("aceinit: Attempt to configure unit %d, max allowed is %d.\n",
	       unit, ace_cnt);
	return ENXIO;
    }
    rsp = sc->ace_regs[unit];
    ssp = &sc->ace_saved_regs[unit];
    sc->ace_fifo_size[unit] = 1;

    ACE_WRITE( rsp, ACE_FCR, ACE_FCR_FENB );	/* assuming there is a fifo */
						/* enable it */

    /* Now, check to see if the port exists.  Read the interrupt register.  
     * If any of bits [5 6] are set, then we are seeing an empty space
     * on the bus.
     */
    ACE_READ( rsp, ACE_IIR, x );
    if ( x & ACE_IE_INVALID )
        {
        sc->ace_type[unit] = ACE_TYPE_NONE;
	printf("aceinit: unit %d, invalid IE bits - 0x%x\n", unit, x );
	return ENXIO;
        }

    ace_type = x >> 6;				 
    switch( ace_type ) 
        {
        case 0:
	    sc->ace_type[unit] = ACE_TYPE_16450;
	    break;

        case 1:
	    sc->ace_type[unit] = ACE_TYPE_UNKNOWN;
	    break;

        case 2:
	    sc->ace_type[unit] = ACE_TYPE_16550;
	    break;

        case 3:
	    sc->ace_type[unit] = ACE_TYPE_16550A;
	    sc->ace_fifo_size[unit] = 16;
	    break;

	default:
            sc->ace_type[unit] = ACE_TYPE_NONE;
	    sc->ace_fifo_size[unit] - 0;

	}

    if ( sc->ace_type[unit] == ACE_TYPE_16450 )
	{
	ACE_READ( rsp, ACE_SCR, x );
	ACE_WRITE( rsp, ACE_SCR, ACE_TYPE_PATTERN_1 );
	ACE_READ( rsp, ACE_SCR, rpat1 );
	ACE_WRITE( rsp, ACE_SCR, ACE_TYPE_PATTERN_2 );
	ACE_READ( rsp, ACE_SCR, rpat2 );
	ACE_WRITE( rsp, ACE_SCR, x );
	if ( rpat1 != ACE_TYPE_PATTERN_1 ||
	     rpat2 != ACE_TYPE_PATTERN_2 )
	    sc->ace_type[unit] = ACE_TYPE_8250;
	}

    if ( sc->ace_type[unit] == ACE_TYPE_16550A ) /* clear FIFO's, if any */
	{
	ACE_WRITE( rsp, ACE_FCR, ACE_FCR_CLEAR );
        ACE_WRITE( rsp, ACE_FCR, ACE_FCR_FENB );
	}

    if ( sc->ace_type[unit] != ACE_TYPE_NONE );
        ace_clean_intr( unit );		/* clear out the interrupt regs. */


/* 
 * Let the user know what we found!
 */
    if ( verbosity != QUIET )
    {
        printf("Configured: serial unit %d, type=",unit);
	switch(sc->ace_type[unit]) 
        {
	case ACE_TYPE_UNKNOWN:
            printf("UNKNOWN\n");
            break;
	    
        case ACE_TYPE_8250:
            printf("8250\n");
            break;
	    
        case ACE_TYPE_16450:
            printf("16450\n");
            break;
	    
        case ACE_TYPE_16550:
            printf("16550\n");
            break;
	    
        case ACE_TYPE_16550A:
            printf("16550A\n");
            break;
	    
        default:
            printf("(out of band, %d)\n", sc->ace_type[unit] );
            return;
        }
    }

    /* set the following:
     *    parity -none
     *    stop bits - one stop bit
     *    baud rate - 9600 baud
     *    break control? - disable for now
     *    as we set, save result in ssp->registers
     */

    /* enable data latch access bit until we write our baud rate */
    /* When accessing the baud rate portion of the lcr is is
     * necessary to set the DLAB bit.  Specifically when accessing
     * dllsb and dlmsb, DLAB must be set.  Otherwise the bit must
     * not be set. (What an awesome chip!!)
     */
    ssp->lcr = (ACE_LCR_8DATA | ACE_LCR_ONESTPBIT | ACE_LCR_DLAB );
    ACE_WRITE( rsp, ACE_LCR, ssp->lcr );

    ssp->dllsb = ACE_B9600_LO;
    ACE_WRITE( rsp, ACE_DLLSB, ssp->dllsb );
    ssp->dlmsb = ACE_B9600_HI;
    ACE_WRITE( rsp, ACE_DLMSB, ssp->dlmsb );

    /* turn off DLAB bit so default reads will go the RB,THR, etc. */
    ssp->lcr = (ACE_LCR_8DATA | ACE_LCR_ONESTPBIT );
    ACE_WRITE( rsp, ACE_LCR, ssp->lcr );

    /* clear out any residual bogusity before enabling interrupts */
    ACE_READ( rsp, ACE_LSR, x );
    ACE_READ( rsp, ACE_RBR, x );

}

/* Initialize the console. */
ace_cons_init()
{
    register struct ace_softc *sc;
    struct controller *ctlr, *get_ctlr_num();
    int i, unit;

    acesc = &ace_softc[0];		/* Set the global var. */
    sc = acesc;
    for ( i=0; i<ace_cnt; i++ ) sc->ace_type[i] = ACE_TYPE_NONE;
    /*
     * Need to find ctlr struct for ace0 (for jensen initialized
     * in kn121_configure_io).  The base address for any ace unit is
     * the ctrl->physaddr + the COMM* offset.  For the Jensen local
     * VTI combo devices, ctrl->physaddr holds the vti combo base
     * address, by encoding this is bus read/write calls for I/O,
     * the jensen read/write bus routines can seamlessly handle
     * VTI combo I/O along with bus I/O requests.  This will make
     * the driver more portable at the only expense of dis-allowing
     * the use of ever calling the generic bus read/write routine
     * with a memory access confilicting with 0x1C00000000,  which
     * is unlikely given this driver will probably not deal with
     * any bus-memory-capable ace devices.
     *
     * For bus ace devices, ctlr->physaddr should be the base physical
     * address of the EISA I/O slot for EISA options and zero for ISA
     * options.
     */
    if( (ctlr = get_ctlr_num("ace",0)) == (struct controller *)NULL)
	/* if we get here ace0 is not in the config file */
	panic("cons_init: ace0 ctlr struct not found");

    /* We know we have exactly two devices. */
    sc->ace_regs[0] = (u_long)ctlr->physaddr + ACE_COMMA_BASE;

    /* Set up the line parameters for the 2 lines.  This includes parity,
     * character size, baud rate, stop bits.
     */
    aceinit( ACE_CONSOLE_UNIT, QUIET ); 

    /* This setup of line1 shouldn't be strictly necessary as it is
     * now done in the aceparam routine.  Its harmless enough though
     * to call it here.  Don't mess with the line if kdebug is running.
     */
    sc->ace_regs[1] = (u_long)ctlr->physaddr + ACE_COMMB_BASE;
    aceinit( ACE_KDEBUG_UNIT, QUIET );

    acesoftCAR = 0x0f;		/* Defaults line to no-modem */
    acedefaultCAR = 0x0f;	/* Defaults line to no-modem */

    for( unit=0; unit<ace_cnt; unit++ )    /* prepare for terminal subsys */
    {
	ace_brk[unit] = 0;
	queue_init(&ace_tty[unit].t_selq);
	if ( (kdebug_state() != 1 || unit != ACE_KDEBUG_UNIT) && unit <= ACE_KDEBUG_UNIT)
	{
	    ace_clear_dtr(unit);           /* Turn off the DSR & RTS signals */
	}
    }
}

aceopen(dev, flag)
dev_t dev;
int flag;
{
    register struct ace_softc *sc = acesc;
    register struct tty *tp;
    register int unit;
    register int maj, error;
    int inuse;  /*hold state of inuse bit while blocked waiting for carr*/
    register u_long rsp;
    register struct ace_saved_reg *ssp0;
    register int data;

    maj = major(dev);
    unit = minor(dev);

    sc->ace_regs[1] = sc->ace_regs[0] - ACE_COMMA_BASE + ACE_COMMB_BASE;
    aceinit( 1, QUIET );
    /*
     * Check for valid unit number. For all Alpha PC systems,
     * unit numbers of 0 or 1 are valid since these systems have two
     * serial lines.
     */

    if (unit >= ace_cnt || sc->ace_type[unit] == ACE_TYPE_NONE )
	return (ENXIO);

    /* Don't allow the line to be opened if kdebug is using it. */
    if ((kdebug_state() == 1) && (unit == ACE_KDEBUG_UNIT))
	return (ENXIO);

    tp = &ace_tty[unit];
    rsp = sc->ace_regs[unit];
#if     SEC_BASE
    if (tp->t_state&TS_XCLUDE && !privileged(SEC_ALLOWDACACCESS, 0)) 
#else
    if (tp->t_state&TS_XCLUDE && u.u_uid != 0) 
#endif
    {
	return (EBUSY);
    }
    
    while (tp->t_state&TS_CLOSING) { /* let DTR stay down for awhile */
	    if (error = tsleep((caddr_t)&tp->t_rawq, TTIPRI | PCATCH, 
                ttopen, 0)) {
		return error;
	    }
    }
    tp->t_addr = (caddr_t)tp;
    tp->t_oproc = acestart;
    tp->t_param = aceparam;
    
    if ((tp->t_state & TS_ISOPEN) == 0) {
	/* Set the default line discipline to termios */
	tp->t_line = TTYDISC;
	tty_def_open(tp, dev, flag, (acesoftCAR&(1<<unit)));

	/*
	 * Prevent spurious startups by making the 500ms timer
	 * initially high.
	 */
	acemodem[unit] = MODEM_DSR_START;
	/* GRAPHICSCONSOLE */
	/*
	 * Specify console terminal attributes.  Do not allow modem control
	 * on the console.  Setup <NL> to <CR> <LF> mapping.
	 * Add check to see if graphics console in use.
	 */
	if((maj == CONSOLEMAJOR) && (unit == ACE_CONSOLE_UNIT)) {
	    /* modem control not supported on console */ 
	    tp->t_cflag = CS8 | CREAD | CLOCAL;
	    tp->t_flags = ANYP|ECHO|CRMOD;
	    tp->t_iflag |= ICRNL; /* Map CRMOD */
	    tp->t_oflag |= ONLCR; /* Map CRMOD */
	    tp->t_ispeed = tp->t_ospeed = TTYDEF_SPEED;
	}
    	aceparam(tp, &tp->t_termios);
    }
#if     SEC_BASE
    else if (tp->t_state&TS_XCLUDE && !privileged(SEC_ALLOWDACACCESS, 0))
#else
    else if (tp->t_state&TS_XCLUDE && u.u_uid != 0)
#endif
    {
	    return (EBUSY);
    }

    (void) spltty();

    /* enable interrupts */
  /*
   * tim - previously there was no "if" around this.  I guessed it may
   * be possible to disable an xint if the line was re-opened while
   * currently busy.  The xint could be missed because this setting of
   * the interrupt enable does not enable interrupts on xmit buffer empty.
   *
   * In retrospect this setting looks sleazy.  I would have expected this
   * to be done in the param() routine anyways or in the start routine. ???
   */
  if ((tp->t_state & TS_BUSY) == 0) { 
    ACE_WRITE( rsp, ACE_IE, ACE_IE_RDINT | ACE_IE_RLSINT | ACE_IE_RLSINT |
	ACE_IE_MSINT );
  }


    /*
     *  Enable modem interrupts and signals.
     *  Acutally modem interrupts are enabled below if clocal isn't set.
     *  Here DTR and RTS are asserted.
     */
    ace_set_dtr(unit);

    /*
     * No modem control provided for lines with softCAR set.
     */
    if (tp->t_cflag & CLOCAL) {
	/*
	 * This is a local connection - ignore carrier
	 * receive enable interrupts enabled above
	 */
	tp->t_state |= TS_CARR_ON;		

	/*
	 * Set the CTS state flag if both DSR and CTS are active.
	 * This is for non-modem connections that reset CLOCAL.
	 */
        ACE_READ( rsp, ACE_MSR, data );
	if (((data & ACE_MSR_DSR)) && ((data & ACE_MSR_CTS)))
		acemodem[unit] |= MODEM_CTS;

	(void) spl0();
	return ((*linesw[tp->t_line].l_open)(dev, tp, flag));
    }
    /* For a modem connection, enable modem status interrupts. */
    ACE_READ( rsp, ACE_IE, data );
    data |= ACE_IE_MSINT;
    ACE_WRITE( rsp, ACE_IE, data );
    /* Now read in the status of the modem pins for later investigation. */
    ACE_READ( rsp, ACE_MSR, data );
    /*
     * After DSR first comes up we must wait for the other signals
     * before commencing transmission.
     */
    if ((flag & (O_NDELAY|O_NONBLOCK)) == 0) {
	/*
	 * Delay before examining other signals if DSR is being followed
	 * otherwise proceed directly to ace_dsr_check to look for
	 * carrier detect and clear to send.
	 */
	if ((data & ACE_MSR_DSR)) {
	    acemodem[unit] |= (MODEM_DSR_START|MODEM_DSR);
	    tp->t_dev = dev; /* need it for timeouts */
	    /*
	     * Give CD and CTS 30 sec. to 
	     * come up.  Start transmission
	     * immediately, no longer need
	     * 500ms delay.
	     */
	    timeout(ace_dsr_check, tp, hz*30);
	    ace_dsr_check(tp);
	}
    }
    if (flag & (O_NDELAY|O_NONBLOCK))
	tp->t_state |= TS_ONDELAY;
    else
	while ((tp->t_state & TS_CARR_ON) == 0) {
	    tp->t_state |= TS_WOPEN;
	    inuse = tp->t_state&TS_INUSE;
	    if (error = tsleep((caddr_t)&tp->t_rawq,
			TTIPRI | PCATCH, ttopen, 0)) 
	    {
			(void) spl0();
			return error;
	    }
	    /*
	     * See if we were awoken by a false call to the modem
	     * line by a non-modem.
	     */
	    if (acemodem[unit]&MODEM_BADCALL){
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
    (void) spl0();
    return((*linesw[tp->t_line].l_open)(dev, tp, flag));
}

aceclose(dev, flag)
dev_t dev;
{
    register struct ace_softc *sc = acesc;
    register struct tty *tp;
    register int unit, maj;
    register int s;
    extern int wakeup();
    register int data;
    register u_long rsp;

    unit = minor(dev);
    maj = major(dev);

    if (unit >= ace_cnt || sc->ace_type[unit] == ACE_TYPE_NONE )
	return (ENXIO);

    tp = &ace_tty[unit];
    rsp = sc->ace_regs[unit];

    /*
     * Do line discipline specific close functions, including waiting
     * for output to drain.
     */

    (*linesw[tp->t_line].l_close)(tp);

    /*
     * Clear breaks for this line on close.
     */
    s = spltty();
    ACE_READ( rsp, ACE_LCR, data);
    data &= ~ACE_LCR_BRKEN;
    ACE_WRITE( rsp, ACE_LCR, data );

    splx(s);
    if ((tp->t_cflag&HUPCL) || (tp->t_state&TS_WOPEN) || 
					(tp->t_state&TS_ISOPEN)==0) {
	tp->t_state &= ~TS_CARR_ON;   /* prevents recv intr. timeouts */
	/*
	 * Drop appropriate signals to terminate the connection.
    	 * Turn off the DSR & RTS signals 
	 */
    	ace_clear_dtr(unit);
	if ((tp->t_cflag & CLOCAL) == 0) {
	    s = spltty();
	    /*drop DTR for at least a sec. if modem line*/
	    tp->t_state |= TS_CLOSING;
	    /*
	     * Wait at most 5 sec for DSR to go off.
	     * Also hold DTR down for a period.
	     *
	     * Aditionally, DTR should stay down for 5secs if both CD
	     * and DSR go off.  This requirement is in the latest DEC 
	     * standard 52 (10/1990).
	     */
    	    ACE_READ( rsp, ACE_MSR, data );
	    if (((data & ACE_MSR_DSR)) || (!(data & ACE_MSR_CD))) {
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
    acesoftCAR &= ~(1<<unit);
    acesoftCAR |= (1<<unit) & acedefaultCAR;
    acemodem[unit] = 0;
    /* ttyclose() must be called before clear up termio flags */
    ttyclose(tp);
    tty_def_close(tp);

    /* disable all interrupts */
    ACE_WRITE( rsp, ACE_IE, 0);
}

aceread(dev, uio, flag)
dev_t dev;
struct uio *uio;
int flag;
{
    register struct ace_softc *sc = acesc;
    register struct tty *tp;
    register int unit;

    unit = minor(dev);
    if (unit >= ace_cnt || sc->ace_type[unit] == ACE_TYPE_NONE )
        {
	return (ENXIO);
        }
    tp = &ace_tty[unit];
    return((*linesw[tp->t_line].l_read)(tp, uio, flag));
}

acewrite(dev, uio, flag)
dev_t dev;
struct uio *uio;
int flag;
{
    register struct ace_softc *sc = acesc;
    register struct tty *tp;
    extern struct tty *constty;
    register int unit;

    unit = minor(dev);
    if (unit >= ace_cnt || sc->ace_type[unit] == ACE_TYPE_NONE )
        {
	return (ENXIO);
        }
    tp = &ace_tty[unit];
    return ((*linesw[tp->t_line].l_write)(tp, uio, flag));
}

aceselect(dev, events, revents, scanning)
dev_t dev;
short *events, *revents;
int scanning;
{
    register int retval;
    register int unit;

    unit = minor(dev);
    retval = ttselect(dev, events, revents, scanning);
    return(retval);
}


/*
 * This is the interrupt service interface that is called when an interrupt
 * is generated.  This interrupt could be for either of the 2 ace units
 * corresponding to lines 0 & 1.  This routine determines the source of
 * the interrupt, dispatches to the appropriate routine to handle the
 * interrupt and makes sure the interrupt condition is dismissed.
 */
aceintr( u_int unit )
{
    register u_long rsp;
    register struct ace_softc *sc = acesc;
    register int iunit = 0;
    register int i;
    int intr_id[10]; /*rps*/
    
    if ( unit < 2 )		/* serial lines 0 and 1 are "tied" together */
        for( i=0; i<2; i++ )
	{
            rsp = sc->ace_regs[i];
    	    ACE_READ( rsp, ACE_IIR, intr_id[i] );
	    iunit |= IP(intr_id[i])<<i;
	}
    else 
    {
        rsp = sc->ace_regs[unit];
        ACE_READ( rsp, ACE_IIR, intr_id[unit] );
        iunit |= IP(intr_id[unit])<<unit;
    }

    if ( sc->ace_type[unit] == ACE_TYPE_NONE )   /* make sure port exists */
    {
	printf("aceintr: spurrious interrupt for nonexistent unit %d\n", unit );
	for( i=0; i<ace_cnt; i++ )
	{
	    rsp = sc->ace_regs[i];
	    ACE_READ( rsp, ACE_IIR, intr_id[i] );
	    if ( IP(intr_id[i]) )
	    {
		printf("There was one on unit %d though.\n", i );
		unit = i;
	    }
	    iunit |= IP(intr_id[i])<<i;
	}
    }
    /*
     * First check all the interrupt bits to be sure that at least one of
     * the interrupt bits is set.
     */
    if ( !iunit || unit > ace_cnt )
    {
        printf("interrupt unit=%x, iunit=%x \n", unit, iunit );
    }

    /*
     * Examine each of the interrupt types for each of the 2 ace units.
     */
    while( iunit )
    {
        if ( unit < 2 )		/* serial lines 0 and 1 are "tied" together */
            for( i=0; i<2; i++ )
	    {
                if ( sc->ace_type[i] != ACE_TYPE_NONE )
		{
                    if( RLS_INT( intr_id[i] ) )	/* Receiver line status */
                        acerint( i );
		    
                    if( RDA_INT( intr_id[i] ) )	/* Received Data Available */
                        acerint( i );
		    
                    if( THRE_INT( intr_id[i] ) ) /* Ready to transmit */
                        acexint( i );
		    
                    if( MS_INT( intr_id[i] ) )	/* Modem status interrupt */
                        ace_modem_int( i );
		}
	    }
        else
	{
            if( RLS_INT( intr_id[unit] ) )	/* Receiver line status */
                acerint( unit );
	    
            if( RDA_INT( intr_id[unit] ) )	/* Received Data Available */
                acerint( unit );
	    
            if( THRE_INT( intr_id[unit] ) ) 	/* Ready to transmit */
                acexint( unit );
	    
            if( MS_INT( intr_id[unit] ) )	/* Modem status interrupt */
                ace_modem_int( unit );
	}
	
        iunit = 0;			/* reset our marker */
        if ( unit < 2 )		/* serial lines 0 and 1 are "tied" together */
            for( i=0; i<2; i++ )
	    {
                rsp = sc->ace_regs[i];
		ACE_READ( rsp, ACE_IIR, intr_id[i] );
		iunit |= IP(intr_id[i])<<i;
	    }
        else 
	{
            rsp = sc->ace_regs[unit];
            ACE_READ( rsp, ACE_IIR, intr_id[unit] );
            iunit |= IP(intr_id[unit])<<unit;
	}
    }
}


/*
 * This routine is called in response to interrupts under these conditions:
 *	- receive data is available	(RDA_INT)
 *	- OE - overrun error		(RLS_INT)
 *	- PE - parity error		(RLS_INT)
 *	- FE - framing error		(RLS_INT)
 *	- BI - break interrupt		(RLS_INT)
 */
acerint(unit)
register int unit;
{
    register struct ace_softc *sc = acesc;
    register struct tty *tp = &ace_tty[unit];
    register u_long rsp;
    register int status;
    int flg;
    int data;
    u_short c;
    register int c_mask;
    int overrun = 0;
    int ct;
    u_short c2;
    int timo;
    int status2;

    if (unit >= ace_cnt || sc->ace_type[unit] == ACE_TYPE_NONE )
	return;

    /* get status from line status register */
    rsp = sc->ace_regs[unit];  
    ACE_READ( rsp, ACE_LSR, status );

    if ((tp->t_state & TS_ISOPEN) == 0) {
	/* Read out the input buffer to remove the character so that
 	 * you won't get an overflow error when the next one comes in.
	 * This read also clears the interrupt condition.
         */
        ACE_READ( rsp, ACE_RBR, data );
	wakeup((caddr_t)&tp->t_rawq);
	return;
    }
    flg = tp->t_iflag;
    
    /* need to do following because ACE sets unused bits to ones */
    /* will leave this just in case ace does the same */
/* rps - test not needed? */
    switch(tp->t_cflag&CSIZE) 
    {
      case CS5:
	c_mask = 0x1f;
	break;
      case CS6:
	c_mask = 0x3f;
	break;
      case CS7:
	c_mask = 0x7f;
	break;
      case CS8:
	c_mask = 0xff;
    }
    if (status&ACE_LSR_DR) 
    {
        /* read char from the read buffer register, this read will clear
         *    the interrupt condition
         */
        ACE_READ( rsp, ACE_RBR, data );
    }
    else {
	/* There is no character available.  This interrupt has been generated
	 * to signal a receiver line status interrupt.  The interrupt
	 * condition will be cleared in the read to the LSR.  There is no
	 * need to read a character out of the receive buffer.
	 */
	c = 0;
    }
    c = (u_short)data & c_mask;
    ct = c & TTY_CHARMASK;

    /*
     * This will be true if the previous character input was a break
     * condition.  The chip is generous enough to toss in an extra null
     * character which must be discarded from the input stream.
     */
    if (ace_brk[unit]) {
	ace_brk[unit] = 0;
	if (ct == 0) {
		return;
	}
    }

    /* Check status for errors (bits set other than Data Ready) */
    if ( (status & 0x1F) != ACE_LSR_DR) {
	    /* ACE_FE is interpreted as a break */
	    if ((status & ACE_LSR_FE) || (status & ACE_LSR_BI)) {
	      ct |= TTY_FE;
	      /* Just to be miserable, this chip inputs a null character
	       * upon termination of the break condition.  So record the
	       * state that the next character expected to be received on this 
	       * line is a null character so that it can be tossed out of
	       * the input stream.
	       */
	      if (status & ACE_LSR_BI) {
		ace_brk[unit] = 1;
	      }
	    }

	    /* Parity Error */
	    if (status & ACE_LSR_PE)
		ct |= TTY_PE;

	    /* Not servicing interrupts fast enough. */
	    if (status & ACE_LSR_OE) {
		    if(overrun == 0) {
			    printf("ace%d.%d: input silo overflow\n", 0, unit);
			    overrun = 1;
		    }
		    sc->ace_softcnt[unit]++;
	    }
    }
    (*linesw[tp->t_line].l_rint)(ct, tp);
}


/* Passed in call to sleep for TCSBREAK */
static char ace_cnbrkstr[] = "ace_cnbrk";

/*ARGSUSED*/
aceioctl(dev, cmd, data, flag)
dev_t dev;
register unsigned int cmd;
caddr_t data;
int flag;
{
    register struct ace_softc *sc = acesc;
    register int unit;
    register struct tty *tp;
    register int s;
    register u_long rsp;
    register int ie;
    struct uba_device *ui;
    struct devget *devget;
    int error;

    unit = minor(dev);
    if (unit >= ace_cnt || sc->ace_type[unit] == ACE_TYPE_NONE )
        {
	return (ENXIO);
        }

    tp = &ace_tty[unit];
    rsp = sc->ace_regs[unit];  
    error = (*linesw[tp->t_line].l_ioctl)(tp, cmd, data, flag);
    if (error >= 0)
        {
	return (error);
        }
    error = ttioctl(tp, cmd, data, flag);
    if (error >= 0) 
        {
	return (error);
        }

    switch (cmd) {

      case TCSBREAK:
	ace_do_break(unit, ACE_DO_STARTBREAK);
	mpsleep((caddr_t)&sc->ace_flags[unit], PZERO-10, ace_cnbrkstr,  hz/4,
		NULL, 0);
	ace_do_break(unit, ACE_DO_STOPBREAK);
	break;

      case TIOCSBRK:	/* ULTRIX compatible ioctl */
	ace_do_break(unit, ACE_DO_STARTBREAK);
	break;
	
      case TIOCCBRK:	/* ULTRIX compatible ioctl */
	ace_do_break(unit, ACE_DO_STOPBREAK);
	break;

	
	/* Tim hack - even though there isn't any modem control support	
	 * allow this to succeed.  Its needed to run the cmx exerciser.
	 * Formerly is was below with the other modem ioctls.
 	 */
      case TIOCNMODEM:  /* ignore modem status */
	/*
	 * By setting the software representation of modem signals
	 * to "on" we fake the system into thinking that this is an
	 * established modem connection.
	 */
	s = spltty();
	acesoftCAR |= (1<<unit);
	if (*(int *)data) /* make mode permanent */
	    acedefaultCAR |= (1<<unit);
	tp->t_state |= (TS_CARR_ON|TS_ISOPEN);
	tp->t_cflag |= CLOCAL;		/* Map to termio */
        /* For a modem connection, disable modem status interrupts. */
        ACE_READ( rsp, ACE_IE, ie );
        ie &= ~ACE_IE_MSINT;
        ACE_WRITE( rsp, ACE_IE, ie );
	splx(s);
	break;

      case TIOCSDTR:
	s = spltty();
	(void) acemctl(dev, TIOCM_DTR | TIOCM_RTS, DMBIS);
	ACE_READ( rsp, ACE_IE, ie );
	ie |= ACE_IE_MSINT;
	ACE_WRITE( rsp, ACE_IE, ie );
	splx(s);
	break;
	
      case TIOCCDTR:
	(void) acemctl(dev, TIOCM_DTR | TIOCM_RTS, DMBIC);
	break;
	
      case TIOCMSET:
	(void) acemctl(dev, dmtoace(*(int *)data), DMSET);
	break;
	
      case TIOCMBIS:
	(void) acemctl(dev, dmtoace(*(int *)data), DMBIS);
	break;
	
      case TIOCMBIC:
	(void) acemctl(dev, dmtoace(*(int *)data), DMBIC);
	break;
	
      case TIOCMGET:
	*(int *)data = acetodm(acemctl(dev, 0, DMGET));
	break;
	
      case TIOCMODEM:  
	s = spltty();

	tp->t_cflag &= ~CLOCAL;		/* Map to termio */
        /* For a modem connection, enable modem status interrupts. */
        ACE_READ( rsp, ACE_IE, ie );
        ie |= ACE_IE_MSINT;
        ACE_WRITE( rsp, ACE_IE, ie );

	acesoftCAR &= ~(1<<unit);
	if (*(int *)data) /* make mode permanent */
	    acedefaultCAR &= ~(1<<unit);
	/*
	 * See if signals necessary for modem connection are present
	 */
        ACE_READ( rsp, ACE_MSR, ie );
	if ((ie & ACE_XMIT_BITS) == ACE_XMIT_BITS) {
	    tp->t_state &= ~(TS_ONDELAY);
	    tp->t_state |= TS_CARR_ON;
	    acemodem[unit] = MODEM_CTS|MODEM_CD|MODEM_DSR;
	} else {
	    tp->t_state &= ~(TS_CARR_ON|TS_ISOPEN);
	    acemodem[unit] &= ~(MODEM_CTS|MODEM_CD|MODEM_DSR);
	}
	splx(s);
	break; 
	
	/*
	 * Note: there is on TIOCWONLINE ioctl implemented here.  It is not
	 * supported under OSF/1.  See ioctl_compat.h.
	 */
	
      case DEVIOCGET:				/* device status */
	devget = (struct devget *)data;
	bzero(devget,sizeof(struct devget));
	devget->category = DEV_TERMINAL;	/* terminal cat.*/
	devget->bus = DEV_EISA;			/* EISA bus	*/
	bcopy(DEV_VTI_ACE,devget->interface,
	      strlen(DEV_VTI_ACE));		/* interface	*/
	bcopy(DEV_UNKNOWN,devget->device,
	      strlen(DEV_UNKNOWN));		/* terminal	*/
	devget->adpt_num = 0;			/* NO adapter	*/
	devget->nexus_num = 0;			/* fake nexus 0 */
	devget->bus_num = 0;			/* NO bus	*/
	devget->ctlr_num = 0;			/* cntlr number */
	devget->slave_num = unit;		/* line number	*/
	bcopy("ace", devget->dev_name, 3);	/* "ace"	*/
	devget->unit_num = unit;		/* scc line?	*/
	devget->soft_count = sc->ace_softcnt[unit];
	devget->hard_count = sc->ace_hardcnt[unit];
	devget->stat = sc->ace_flags[unit];	/* status	*/
	if (tp->t_cflag & CLOCAL) {
	    sc->ace_category_flags[unit] |= DEV_MODEM;
	    sc->ace_category_flags[unit] &= ~DEV_MODEM_ON;
	} else {
	    sc->ace_category_flags[unit] |= (DEV_MODEM|DEV_MODEM_ON);
	}
	devget->category_stat = sc->ace_category_flags[unit];
	break;
	
      default:
	return (ENOTTY);
    }
    return (0);
}      

acemmap(dev, off, prot)
dev_t dev;
off_t off;
int prot;
{
    register struct ace_softc *sc = acesc;
    int unit;
    extern int noconsmmap();

    unit = minor(dev);
    if (unit >= ace_cnt || sc->ace_type[unit] == ACE_TYPE_NONE )
	return (ENXIO);
    /* return (noconsmmap(dev, off, prot)); */
    return(EINVAL);	/* No support for mmap on this driver. */
}

aceparam(register struct tty *tp, struct termios *t)
{
    register struct ace_softc *sc = acesc;
    register u_long rsp;
    register int s, status;
    register int timo;
    struct ace_saved_reg *ssp;
    int unit = minor(tp->t_dev);
    int maj = major(tp->t_dev);
 
    if (unit >= ace_cnt || sc->ace_type[unit] == ACE_TYPE_NONE )
	return (ENXIO);

    rsp = sc->ace_regs[unit];
    ssp = &sc->ace_saved_regs[unit];

    s = spltty();
    if (tp->t_state & TS_BUSY) {
	/* Defer setting of line parameters until after the character being
	 * currently transmitted has completed.  Wouldn't want to pull the
	 * rug out from under the in progress transmission.
   	 */
        tp->t_state |= TS_NEED_PARAM;
	return(0);
    }

    /* Wait for empty transmit hold register to insure that a previous
     * character is not sitting in the output buffer waiting to go.
     * Perform this check so that the line parameters are not set while
     * a character is being transmitted.  (Note: the check for TS_BUSY
     * above should cover this case, but just to be sure...)
     */
    timo = 10000;
    ACE_READ( rsp, ACE_LSR, status );
    while( status&ACE_LSR_TEMT  == 0 ){
	    ACE_READ( rsp, ACE_LSR, status );
		    if (--timo == 0) {
			break;  /* waited long enough */
		    }
    }

    /* check requested parameters */
    /* If input baud rate is set to zero, the input baud rate will
     * be specified by the value of the output baud rate.
     * This uart does not allow split baud rates.  Consequently you can't
     * set the input speed and output speed to different values.
     */
    if (t->c_ospeed < 0 || (t->c_ispeed ? (t->c_ispeed != t->c_ospeed)
		    : !(t->c_ispeed = t->c_ospeed)))
        {
	return (EINVAL);
        }
    /* Verify that the requested baud rate is supported on this uart */
    if (ttspeedtab(t->c_ospeed, ace_speeds) == BAUD_UNSUPPORTED)
        {
	return (EINVAL);
        }

    /* GRAPHICSCONSOLE */
    /*
     * Don't allow the line parameters to be changed on the console.
     * If you did and it didn't match the console firmware's version of
     * the parameters then things may fall appart between boots.
     * This restriction may be reconsidered at a later date.
     */
    if (maj != CONSOLEMAJOR || unit != ACE_CONSOLE_UNIT) {
	ssp->lcr = 0;
	/* First set the number of stop bits and the parity. */
	if (tp->t_cflag & CSTOPB) 
	    ssp->lcr |= ACE_LCR_TWOSTPBIT;
	else
	    ssp->lcr |= ACE_LCR_ONESTPBIT;
	if (tp->t_cflag & PARENB) {
	    if ((tp->t_cflag & PARODD) == 0) 
		/* set even */
		ssp->lcr |= (ACE_LCR_EPAR | ACE_LCR_PENABLE);
	    else
		/* else set odd */
		ssp->lcr |= (ACE_LCR_PENABLE);
	}
	/* Set the character size. */
	switch(tp->t_cflag&CSIZE) {
	  case CS5:
	    ssp->lcr |= ACE_LCR_5DATA;
	    break;
	  case CS6:
	    ssp->lcr |= ACE_LCR_6DATA;
	    break;
	  case CS7:
	    ssp->lcr |= ACE_LCR_7DATA;
	    break;
	  case CS8:
	    ssp->lcr |= ACE_LCR_8DATA;
	    break;
	}
    	/* Set the baud rate */
    	/* When accessing the baud rate portion of the lcr it is
     	 * necessary to set the DLAB bit.  Specifically when accessing
     	 * dllsb and dlmsb, DLAB must be set.  Otherwise the bit must
     	 * not be set. (What an awesome chip!!)
     	 */
    	ssp->lcr |= ACE_LCR_DLAB;
    	ACE_WRITE( rsp, ACE_LCR, ssp->lcr );
	
    	ssp->dllsb = ttspeedtab(t->c_ospeed, ace_lo_speeds);
    	ACE_WRITE( rsp, ACE_DLLSB, ssp->dllsb );
    	ssp->dlmsb = ttspeedtab(t->c_ospeed, ace_hi_speeds);
    	ACE_WRITE( rsp, ACE_DLMSB, ssp->dlmsb );

    	/* turn off DLAB bit so default reads will go the RB,THR, etc. */
    	ssp->lcr &= ~(ACE_LCR_DLAB);
    	ACE_WRITE( rsp, ACE_LCR, ssp->lcr );
    
    	/* Should interrupts be disabled
     	 * at the beginning of this routine before doodling with the
     	 * line pamameter registers? (It appears to work as written.) */
    	/* clear out any residual bogosity before enabling interrupts */
/* rps
    	ACE_READ( rsp, ACE_LSR, timo );
    	ACE_READ( rsp, ACE_RBR, timo );
*/
        ace_clean_intr( unit );
    }
else

    /* copy termios to tty */
    tp->t_ispeed = t->c_ispeed;
    tp->t_ospeed = t->c_ospeed;
    tp->t_cflag = t->c_cflag;

    /*
     * Setting the baud rate to B0 implies that all of the modem control
     * signals should be dropped to hangup the modem connection.
     */
    if ((tp->t_state & TS_ISOPEN) && t->c_ospeed == B0) {
    	    /* Turn off the DSR & RTS signals */
	    ace_clear_dtr(unit);
	splx(s);
	return(0);
    }
    splx(s);

    return(0);
}

/* clean out the interrupts of a give unit... */
ace_clean_intr( unit )
    {
    register struct ace_softc *sc = acesc;
    register u_long rsp;
    register int s;

    rsp = sc->ace_regs[unit];

    ACE_READ( rsp, ACE_MSR, s );
    ACE_READ( rsp, ACE_LSR, s );
    ACE_READ( rsp, ACE_IIR, s );
    ACE_READ( rsp, ACE_RBR, s );
    }
/*
 * Interrupt service routine called when the transmitter holding register
 * empties out and then the ACE_LSR_THRE gets set in the line status register.
 */
acexint(int unit)
{
    register struct ace_softc *sc = acesc;
    register struct tty *tp;
    register char *ptr;
    register u_long rsp;
    register int cc;

    
    if (unit >= ace_cnt || sc->ace_type[unit] == ACE_TYPE_NONE )
	return;

    tp = &ace_tty[unit];
    tp->t_state &= ~TS_BUSY;

    /*
     * The transmitter interrupt signals the completion of transmission of
     * a single character.  Call ndflush with a value of 1 to remove this
     * character from the line's output queue to signify that the transmission
     * has been accomplished.
     */
    ndflush(&tp->t_outq, 1); 

    /*
     * There may have been a call to set the line parameters while the
     * driver was currently busy transmitting on this line.  In that case
     * the actual setting of parameters has been deferred.  Now that the
     * transmission has completed it is safe to go ahead and setup the new
     * line parameters.
     */
    if (tp->t_state & TS_NEED_PARAM) {
        tp->t_state &= ~TS_NEED_PARAM;
	aceparam(tp, &tp->t_termios);
    }

    /* See if there are any more characters waiting to be output. */
    if (tp->t_line) {
	(*linesw[tp->t_line].l_start)(tp);
    } else  {
	acestart(tp);
    }
}

acestart(tp)
     register struct tty *tp;
{	
    register struct ace_softc *sc = acesc;
    register int cc;
    int s, unit;
    register char *cp;
    register int c_mask;
    register u_long rsp;
    int c;
    register int count;
    register unsigned int iereg;
    register int timo = 1000000;
    s = spltty();
    unit = minor(tp->t_dev) & ACELINEMASK;

    if (unit >= ace_cnt || sc->ace_type[unit] == ACE_TYPE_NONE )
	goto out;

    /*
     * Do not do anything if currently delaying, or active.  Also only
     * transmit when CTS is up.
     */
    if ((tp->t_state & (TS_TIMEOUT|TS_BUSY|TS_TTSTOP)) || 
	( ((tp->t_cflag & CLOCAL) == 0) &&
         (tp->t_state&TS_CARR_ON) && ((acemodem[unit]&MODEM_CTS)==0)) )
	goto out;

    if (tp->t_outq.c_cc <= tp->t_lowat) {
	if (tp->t_state&TS_ASLEEP) {
	    tp->t_state &= ~TS_ASLEEP;
	    wakeup((caddr_t)&tp->t_outq);
	}
	select_wakeup(&tp->t_selq);
    }
    if (tp->t_outq.c_cc == 0)
	goto out;

    if ((tp->t_oflag & OPOST) == 0) {
	cc = ndqb(&tp->t_outq, 0);
    } else {
	cc = ndqb(&tp->t_outq, DELAY_FLAG);
	if (cc == 0) {
	    cc = getc(&tp->t_outq);
	    timeout(ttrstrt, (caddr_t)tp, (cc&0x7f) + 6);
	    tp->t_state |= TS_TIMEOUT;
	    goto out;
	}
    }
    tp->t_state |= TS_BUSY;
    cp = tp->t_outq.c_cf;

    /*
     * need to do the following because when character size is set to five,
     * the data format allows character sizes of one to five. See ACE
     * manual.
     */
    if ((tp->t_cflag&CSIZE) == CS5)
       c_mask = 0x1f;
    else
       c_mask = 0xff;

    rsp = sc->ace_regs[unit];

    /* Set the interrupt enable bits.  The only reason this register is
     * read first is to preserve the setting of the ACE_IE_MSINT bit.  It is
     * not sufficient to only set that if CLOCAL isn't set because tip 
     * operates with clocal cleared.
     * I hope this doesn't immediately interrupt because the xmit buffer
     * is empty.
     */
    ACE_READ( rsp, ACE_IE, iereg );
    iereg |= (ACE_IE_RDINT | ACE_IE_THREINT | ACE_IE_RLSINT);
    ACE_WRITE( rsp, ACE_IE, iereg);

    count = MIN( tp->t_outq.c_cc, sc->ace_fifo_size[unit] );
    while ( count-- )
        {
        c = *cp++ & c_mask;
        ACE_WRITE( rsp, ACE_THR, c );
        }

  out:	
    splx(s);
}

acestop(tp, flag)
     register struct tty *tp;
{
    register struct ace_softc *sc = acesc;
    register int s;
    int	unit;
    register u_long rsp;
    int status;
    register int timo = 1000000;

    unit = minor(tp->t_dev);

    if (unit >= ace_cnt || sc->ace_type[unit] == ACE_TYPE_NONE )
	return;

    s = spltty();
    if (tp->t_state & TS_BUSY) {
	    /* wait for transmitter to be cleared */
	    rsp = sc->ace_regs[unit];

	    /* wait for empty transmit hold register */
	    ACE_READ( rsp, ACE_LSR, status );
	    while( status&ACE_LSR_THRE  == 0 ){
		    ACE_READ( rsp, ACE_LSR, status );
		    if (--timo == 0) {
			panic("acestop: timo expired.\n");
		    }
	    }

	    /* wait for empty transmitter to make sure char got out */
	    timo = 1000000;
	    while( status&ACE_LSR_TEMT  == 0 ){
		    ACE_READ( rsp, ACE_LSR, status );
		    if (--timo == 0) {
			panic("acestop 2: timo expired.\n");
		    }
	    }

	    /* line discipline will flush entire queue */
	    if ((tp->t_state&TS_TTSTOP)==0) {
		    ; /*skip*/
	    } else { /* suspend */
		    ndflush(&tp->t_outq, 1); /* cc bytes */
	    }
	    tp->t_state &= ~TS_BUSY;
    }
    splx(s);
}

acemctl(dev, bits, how)
     dev_t dev;
     int bits, how;
{
    register struct ace_softc *sc = acesc;
    register int unit, mbits;
    register u_long rsp;
    register int data;
    int b, s;
    
    unit = minor(dev);
    if (unit >= ace_cnt || sc->ace_type[unit] == ACE_TYPE_NONE )
	return (0);
    s = spltty();

    /* Assemble the current modem control settings into mbits. */
    rsp = sc->ace_regs[unit];
    ACE_READ( rsp, ACE_MCR, data );
    mbits = 0;
    if ((data & ACE_MCR_DTR)) {
	mbits |= TIOCM_DTR;
    }
    if ((data & ACE_MCR_RTS))
	mbits |= TIOCM_RTS;
    ACE_READ( rsp, ACE_MSR, data );
    if ((data & ACE_MSR_DSR))
	mbits |= TIOCM_DSR;
    if ((data & ACE_MSR_CTS))
	mbits |= TIOCM_CTS;
    if ((data & ACE_MSR_CD))
	mbits |= TIOCM_CD;

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
    if (mbits & TIOCM_DTR) {
    	/*  Here DTR and RTS are asserted.
     	 */
	ace_set_dtr(unit);
    } else {
    	/* Turn off the DSR & RTS signals 
     	 */
	ace_clear_dtr(unit);
    }
    (void) splx(s);
    return(mbits);
}

/*
 * Called from the interrupt service routine when a modem status interrupt
 * has been generated.
 */
ace_modem_int(i)
     register i;
{
    register struct ace_softc *sc = acesc;
    register struct tty *tp;
    register u_long rsp;
    register int msr;
    register int brk;

    tp = &ace_tty[i];
    rsp = sc->ace_regs[i];

    ACE_READ( rsp, ACE_MSR, msr );
    if ((tp->t_cflag & CLOCAL) == 0) {
	/*
	 * Drop DTR immediately if DSR has gone away.
	 * If really an active close then do not
	 *    send signals.
	 */
	if (!(msr & ACE_MSR_DSR)) {
	    if (tp->t_state&TS_CLOSING) {
		untimeout(wakeup, (caddr_t) &tp->t_dev);
		wakeup((caddr_t) &tp->t_dev);
	    }
	    if (tp->t_state&TS_CARR_ON) {
		ace_tty_drop(tp);
	    }
	} else {		/* DSR has come up */
	    /*
	     * If DSR comes up for the first time we allow
	     * 30 seconds for a live connection.
	     */
	    if ((acemodem[i] & MODEM_DSR)==0) {
		acemodem[i] |= (MODEM_DSR_START|MODEM_DSR);
		/*
		 * we should not look for CTS|CD for about
		 * 500 ms.
		 */
		timeout(ace_dsr_check, tp, hz*30);
		ace_dsr_check(tp);}

	}
	
	/*
	 * look for modem transitions in an already
	 * established connection.
	 */
	if (tp->t_state & TS_CARR_ON) {
	    if ((msr & ACE_MSR_CD)) {
		/*
		 * CD has come up again.
		 * Stop timeout from occurring if set.
		 * If interval is more than 2 secs then
		 * drop DTR.
		 */
		if ((acemodem[i] & MODEM_CD) == 0) {
		    untimeout(ace_cd_drop, tp);
		    if (ace_cd_down(tp)) {
			/* drop connection */
			ace_tty_drop(tp);
		    }
		    acemodem[i] |= MODEM_CD;
		}
	    } else {
		/*
		 * Carrier must be down for greater than
		 * 2 secs before closing down the line.
		 */
		if (acemodem[i] & MODEM_CD) {
		    /* only start timer once */
		    acemodem[i] &= ~MODEM_CD;
		    /*
		     * Record present time so that if carrier
		     * comes up after 2 secs, the line will drop.
		     */
		    acetimestamp[i] = time;
		    timeout(ace_cd_drop, tp, hz * 2);
		}
	    }
	    
	    /* CTS flow control check */
	    
	    if (!(msr & ACE_MSR_CTS)) {
		/*
		 * Only allow transmission when CTS is set.
		 */
		tp->t_state |= TS_TTSTOP;
		acemodem[i] &= ~MODEM_CTS;
		acestop(tp, 0);
	    } else if (!(acemodem[i] & MODEM_CTS)) {
		/*
		 * Restart transmission upon return of CTS.
		 */
		tp->t_state &= ~TS_TTSTOP;
		acemodem[i] |= MODEM_CTS;
		acestart(tp);
	    }
	}
	
	/*
	 * See if a modem transition has occured.  If we are waiting
	 * for this signal, cause action to be take via
	 * ace_start_tty.
 	 * (Note: this next "if" statement came from the other drivers.  This
	 * chip has in its low order bits for the msr indications if a change
 	 * has occurred.  That would be a cleaner way to detect change.)
	 */
	if (((msr & ACE_XMIT_BITS) == ACE_XMIT_BITS) &&
	    (!(acemodem[i] & MODEM_DSR_START)) &&
	    (!(tp->t_state & TS_CARR_ON))) {
	    ace_start_tty(tp);
	}
    }
}

/*
 * Modem Control Routines
 */

/*
 *
 * Function:
 *
 *	ace_cd_drop
 *
 * Functional description:
 *
 * 	Determine if carrier has dropped.  If so call ace_tty_drop to terminate
 * 	the connection.
 *
 * Arguments:
 *
 *	register struct tty *tp  -  terminal pointer ( for terminal attributes )
 *
 * Return value:
 *
 *	none
 *
 */
ace_cd_drop(tp)
     register struct tty *tp;
{
    register struct ace_softc *sc = acesc;
    register int unit;
    register int msr;
    register u_long rsp;
    
    unit = minor(tp->t_dev);
    if (unit >= ace_cnt || sc->ace_type[unit] == ACE_TYPE_NONE )
	return;
    rsp = sc->ace_regs[unit];
    ACE_READ( rsp, ACE_MSR, msr );
    if ((tp->t_state & TS_CARR_ON) && (!(msr & ACE_MSR_CD))) {
	ace_tty_drop(tp);
	return;
    }
    acemodem[unit] |= MODEM_CD;
}

/*
 *
 * Function:
 *
 *	ace_dsr_check
 *
 * Functional description:
 *
 *	DSR must be asserted for a connection to be established.  Here we 
 *	either start or terminate a connection on the basis of DSR.
 *
 * Arguments:
 *
 *	register struct tty *tp  -  terminal pointer (for terminal attributes)
 *
 * Return value:
 *
 *	none
 *
 */
ace_dsr_check(tp)
     register struct tty *tp;
{
    register struct ace_softc *sc = acesc;
    register int unit;
    register int msr;
    register u_long rsp;
    
    unit = minor(tp->t_dev);
    if (unit >= ace_cnt || sc->ace_type[unit] == ACE_TYPE_NONE )
	return;
    if (acemodem[unit] & MODEM_DSR_START) {
	acemodem[unit] &= ~MODEM_DSR_START;
    	rsp = sc->ace_regs[unit];
    	ACE_READ( rsp, ACE_MSR, msr );
	if ( msr & ACE_MSR_CTS )
	    acemodem[unit] |= MODEM_CTS;
#ifdef rps
	/*
	 * Look for DSR|CTS|CD before establishing a connection.
	 */
	if ((msr & ACE_XMIT_BITS) == ACE_XMIT_BITS) {
#else
        /*
         * Look for only DSR and CTL - for now...
         */
        if ( msr & ( ACE_MSR_CTS | ACE_MSR_DSR ) ) {
#endif
	    ace_start_tty(tp);
	}
	return;
    }
    if ((tp->t_state&TS_CARR_ON)==0)
	ace_tty_drop(tp);
}

/*
 *
 * Function:
 *
 *	ace_cd_down
 *
 * Functional description:
 *
 *	Determine whether or not carrier has been down for > 2 sec.
 *
 * Arguments:
 *
 *	register struct tty *tp  -  terminal pointer ( for terminal attributes )
 *
 * Return value:
 *
 *	1 - if carrier was down for > 2 sec.
 *	0 - if carrier down <= 2 sec.
 *
 */
ace_cd_down(tp)
     register struct tty *tp;
{
    register int msecs, unit;
    
    unit = minor(tp->t_dev);
    msecs = 1000000 * (time.tv_sec - acetimestamp[unit].tv_sec) + 
	(time.tv_usec - acetimestamp[unit].tv_usec);
    if (msecs > 2000000){
	return(1);
    }
    else{
	return(0);
    }
}

/*
 *
 * Function:
 *
 *	ace_tty_drop
 *
 * Functional description:
 *
 *	Terminate a connection.
 *
 * Arguments:
 *
 *	register struct tty *tp  -  terminal pointer ( for terminal attributes )
 *
 * Return value:
 *
 *	none
 *
 */
ace_tty_drop(tp)
     struct tty *tp;
{
    register struct ace_softc *sc = acesc;
    register int unit;
    register int mcr;
    register u_long rsp;
    
    unit = minor(tp->t_dev);
    if (tp->t_flags & NOHANG)
	return;
    /* 
     * Notify any processes waiting to open this line.  Useful in the
     * case of a false start.
     */
    acemodem[unit] = MODEM_BADCALL;
    tp->t_state &= ~(TS_CARR_ON|TS_TTSTOP|TS_BUSY|TS_ISUSP);
    wakeup((caddr_t)&tp->t_rawq);
    pgsignal(tp->t_pgrp, SIGHUP, 1);
    pgsignal(tp->t_pgrp, SIGCONT, 1);
    /* Turn off the DSR & RTS signals 
     */
    ace_clear_dtr(unit);
}

/*
 *
 * Function:
 *
 *	ace_start_tty
 *
 * Functional description:
 *
 *	Establish a connection.
 *
 * Arguments:
 *
 *	register struct tty *tp  -  terminal pointer ( for terminal attributes )
 *
 * Return value:
 *
 *	none
 *
 */
ace_start_tty(tp)
     register struct tty *tp;
{
    register int unit;
    
    unit = minor(tp->t_dev);
    tp->t_state &= ~(TS_ONDELAY);
    tp->t_state |= TS_CARR_ON;
    if (acemodem[unit] & MODEM_DSR)
	untimeout(ace_dsr_check, tp);
    acemodem[unit] |= MODEM_CD|MODEM_CTS|MODEM_DSR;
    acetimestamp[unit].tv_sec = acetimestamp[unit].tv_usec = 0;
    if (tp->t_state & TS_MODEM_ON) {
	tp->t_state |= TS_ISOPEN;
    }
    wakeup((caddr_t)&tp->t_rawq);
}

aceputc(c)
     register int c;
{
    ace_putc(0, c);
}

/*
 * polled-mode DMA: need to do this because ACE can not be touched in
 * ace_putc.
 */
ace_putc(unit, c)
     int unit;
     register int c;
{
    register struct ace_softc *sc = acesc;
    register u_long rsp;
    register int s;
    int status;
    register int timo = 1000000;

	s = spltty();
	rsp = sc->ace_regs[unit];

        /* wait for empty transmit hold register */
        ACE_READ( rsp, ACE_LSR, status );
        while( (status&ACE_LSR_THRE)  == 0 ){
	    ACE_READ( rsp, ACE_LSR, status );
		    if (--timo == 0) {
			panic("ace_putc: timo expired.\n");
		    }
	}

        /* write data to transmit register */
        ACE_WRITE( rsp, ACE_THR, c );


	splx(s);
}

/* pgt - new acegetc() */
acegetc()
{
    register int c;
    register int line;

    /*
     * Line number we expect input from. 
     */
    line = 0x0;
    c = ace_getc(line);
    return( c );
}

ace_getc(unit)
     int unit;
{
    register struct ace_softc *sc = acesc;
    int c;
    int status;
    register int timo;
    register u_long rsp;
    register int s;

    if (unit >= ace_cnt || sc->ace_type[unit] == ACE_TYPE_NONE )
	return (-1);

    s = splextreme();

    rsp = sc->ace_regs[unit];

    /* disable interrupts */
    ACE_WRITE( rsp, ACE_IE, 0x0 );

    for(timo=1000000; timo > 0; --timo) {
        
        ACE_READ( rsp, ACE_LSR, status );
	if( (status&ACE_LSR_DR) != 0 ){
	    ACE_READ( rsp, ACE_RBR, c );
	    if( status & ( ACE_LSR_PE | ACE_LSR_OE | ACE_LSR_FE ) )
	      continue;
	    break;
	}
    }

    splx(s);

    if (timo == 0)
	return(-1);
    else
	return(c & 0xff);
}


/*
 * ace_do_break
 *
 * Perform the functions necessary to send out a break sequence or to
 * end a break sequence.
 */
int
ace_do_break(unit, action)
	int unit;
	int action;
{
        register struct ace_softc *sc = acesc;
	register int s;
	register u_long rsp;
	register struct ace_saved_reg *ssp;
	register int timo;
	register int status;

        if (unit >= ace_cnt || sc->ace_type[unit] == ACE_TYPE_NONE )
	    return(ENXIO);

	rsp = sc->ace_regs[unit];
	ssp = &sc->ace_saved_regs[unit];
        timo = 10000;

     	s = spltty();
        /* Wait for empty transmit buffers to insure that a previous
         * character is not sitting in the output buffer waiting to go.
         * Perform this check so that the break doesn't zap out the previous
         * character.  The manual also says it is necessary to wait for the
	 * transmitter to be idle prior to clearing the break.
         */
        ACE_READ( rsp, ACE_LSR, status );
        while( status&ACE_LSR_TEMT  == 0 ){
	    ACE_READ( rsp, ACE_LSR, status );
		    if (--timo == 0) {
			break; /* waited long enough */
		    }
        }
	if (action == ACE_DO_STARTBREAK) {
	    /* To avoid sending out an extra character you are
    	     * supposed to load a "0" pad character and then set the break bit
	     * upon conclusion of that transmission.  Wait for the pad char
	     * to be consumed before setting the break enable.
	     */
    	    ACE_WRITE( rsp, ACE_THR, 0 );
	    ACE_READ( rsp, ACE_LSR, status );
	    while( status&ACE_LSR_TEMT  == 0 ){
		    ACE_READ( rsp, ACE_LSR, status );
			    if (--timo == 0) {
				break; /* waited long enough */
			    }
	    }
	    ssp->lcr |= ACE_LCR_BRKEN;
	    ACE_WRITE( rsp, ACE_LCR, ssp->lcr );
	}
	else if (action == ACE_DO_STOPBREAK) {
	    ssp->lcr &= ~(ACE_LCR_BRKEN);
	    ACE_WRITE( rsp, ACE_LCR, ssp->lcr );
	}
	else {
		return(EINVAL);
	}
	splx(s);
	return(0);
}

/*
 * These are generic modem control routines used to translate from 
 * controller specific settings to the generic representation of the
 * modem control leads.  Since this driver is smart enough to represent
 * them internally via the generic defines this routine doesn't have to
 * do anything.
 */
dmtoace(bits)
     register int bits;
{
    return(bits);
}
acetodm(bits)
     register int bits;
{
    return(bits);
}

/*
 * Tim - Note - WARNING:
 *
 * (Final update 3/2/92) - The following describes a hardware problem 
 * with accessing the MCR register.  In order to workaround this problem
 * all accesses to write this register have been isolated to the
 * following routines: ace_set_dtr & ace_clear_dtr.
 *
 * Apparently there is a potential problem with bit MCR<3> as described 
 * below which are extracts from relevent mail messages.  
 *
 * ..............
 *		Further to the serial line interrupt problems that have been 
 * seen on Jensen, described in the attatched mail. The decision looks likely 
 * to be that the hardware will not be altered, and that we will have to get 
 * round the issue in firmware/software. The thinking is that we set the MCR<3> 
 * bit in the console and the bit is never touched again. This means that :-
 *
 * 1. The OS serial line drivers should never clear MCR<3> in order to disable
 *    interrupts but should use the IER register.
 * 2. Customers should never attempt to program the serail line register, ie the
 *    MCR, directly.
 *
 *		We have come across a problem on the Theta2, which is also on
 * the Jensen, where at poweron the serial line interrupts are active. This 
 * would seem to be because the 'interrupt enable' bits (MCR<3>) in both serial 
 * line controllers are set to 0, ie interrutps disabled. This causes floating 
 * outputs on the IRQ outputs from the COMBO, which are pulled up, and an active
 * signal into the EV4.
 *
 *		If the MCR<3> bits are set, ie interrupts enabled, the 
 * individual Interrupt Enable Registers can be used to control interrupts, but
 * if anyone were to clear the 'Interrupt Enable' bit they will get a constant
 * interrupt at the EV4, and a hang. Are you aware of this ? We are considering
 * putting pull-downs on the IRQ outputs from the COMBO. 
 *
 * Tim notes: I didn't see the connection between MCR<3> and interrupts.
 * Here's additional info from Sean:
 *		Aren't these manuals great. What you say is true, the MCR<3>
 * bit is called OUT2 in the register definitions. If you look at the small
 * logic diagram of the interrupt hardware on the chip, MCR<3> magically becomes
 * master interrupt enable and is used to drive the interrupt signal out of the
 * device. It has nothing to do with the IER register. That is used to 
 * individually enable and disable the interrupt sources, eg Tx, Rx etc.
 *
 *		If the MCR<3> bit is cleared, ie disabled, the IRQ output is
 * pulled high. This causes a constant active interrupt to the EV4 regardless of
 * the state of the IER register.
 *
 * Tim - final update on this mess!  Whenever the MCR register is modified
 * the current value is read in first and then the appropriate bits get
 * masked in and out.  By doing this the setting of MCR<3> should never get
 * altered by this driver.  (This is an accident waiting to happen.)
 */
/*
 * Used to turn on both the DTR & RTS lead.
 */
void
ace_set_dtr(unit)
	register int unit;
{
    	register struct ace_softc *sc = acesc;
    	register u_long rsp;
    	register int data;

    	rsp = sc->ace_regs[unit];
    	ACE_READ( rsp, ACE_MCR, data );
	if (!(data & ACE_MCR_OUT2)) { /* should never be true */
		data |= ACE_MCR_OUT2;
	}
	/*  Here DTR and RTS are asserted.
     	 *  Note: leaves the dreaded MCR<3> bit unchanged. 
     	 */
    	data |= (ACE_MCR_DTR | ACE_MCR_RTS);
    	ACE_WRITE( rsp, ACE_MCR, data );
}
/*
 * Used to turn off both the DTR & RTS lead.
 */
void
ace_clear_dtr(unit)
	register int unit;
{
    	register struct ace_softc *sc = acesc;
    	register u_long rsp;
    	register int data;

    	rsp = sc->ace_regs[unit];
    	ACE_READ( rsp, ACE_MCR, data );
	if (!(data & ACE_MCR_OUT2)) { /* should never be true */
		data |= ACE_MCR_OUT2;
	}
    	/* Turn off the DSR & RTS signals 
     	 *  Note: leaves the dreaded MCR<3> bit unchanged. 
     	 */
    	data &= ~(ACE_MCR_DTR|ACE_MCR_RTS);
    	ACE_WRITE( rsp, ACE_MCR, data );
}

#ifdef CONFIG_DEBUG
dump_ace_softc( struct ace_softc *sc, int unit )
{
    int nani, hajime, shinda;

    if ( unit < 0 )
    {
        hajime = 0;
        shinda = ace_cnt-1;
    }
    else
        hajime = shinda = unit;

    for ( nani = hajime; nani <= shinda; nani++ )
    {
        printf("unit: %d\n", nani );
	printf("    ace_regs           : 0x%x\n", sc->ace_regs[nani] );
	printf("    ace_type           = 0x%x\n", sc->ace_type[nani] );
        printf("    ace_fifo_size      = %d\n", sc->ace_fifo_size[nani] );
        printf("    ace_saved_regs     : 0x%x\n", sc->ace_saved_regs[nani] );
        printf("    ace_flags          = 0x%x\n", sc->ace_flags[nani] );
        printf("    ace_category_flags = 0x%x\n", sc->ace_category_flags[nani] );
	printf("    ace_softcnt        = 0x%x\n", sc->ace_softcnt[nani] );
	printf("    ace_hardcnt        = 0x%x\n", sc->ace_hardcnt[nani] );
    }
}
#endif





