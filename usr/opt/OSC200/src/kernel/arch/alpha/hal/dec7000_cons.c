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
static char     *sccsid = "@(#)ruby_cons.c      9.2     (ULTRIX/OSF)    10/24/91";
#endif lint
/*
 * Modification History: /sys/machine/alpha/adu_cons.c
 *
 * 14-Jan-92 -- prm
 *    - Modified routine names to reflect DUART device, rather than
 *      ruby, since this could be used on multiple platforms with the
 *      same serial line device.
 *    - Removed debug support routines and references.
 *
 * 06-Nov-91 -- prm
 *      Modify cnrw_8530 routine name to alpha_8530_cnrw_8530.
 *      Add cnrw_8530 routine pointer, for accessing machine specific
 *        8530 routines. Will contain address of alpha_8530_cnrw_8530 for ruby.
 *      Add initialization of cnrw_8530 pointer to alpha_8530_cons_init.
 *
 * 17-Sep-91 -- prm (Peter Mott)
 *      Ported to Ruby from adu_cons.c 9.6
 *
 */

/*      TO DO !!!!!!!!!!!!                      
 *
 *      This driver is presently setup to only utilize serial line #1 and
 *      does not provide any access to serial line #2 which has separate
 *      registers.
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
#include <io/dec/uba/ubavar.h> /* auto-config headers */
#include <hal/adudefs.h>
#include <vm/vm_kern.h>
#include <io/common/devdriver.h>
        
        /* machine specific includes */
#include <hal/kn7aa.h>
#include <machine/rpb.h>
        
/* #define PRM_DEBUG  1 */

/* #define KN7AA_ADU_DEBUG 1 */
#ifdef KN7AA_ADU_DEBUG
#include <hal/cons_sw.h>
#include <hal/cpuconf.h>
#endif /* KN7AA_ADU_DEBUG */

        
        /* local defines */
#define XON  0x11
#define XOFF 0x13
#define CONSOLE_CTB 0
        
        /*
         * Globals - with default values
         */
#ifdef KN7AA_ADU_DEBUG
        extern void dumpvec();
        extern void intrsetvec();
        
        extern struct cons_sw *cons_swp;
        extern int cpu;
#endif /* KN7AA_ADU_DEBUG */
        
        extern int alpha_8530_cnxint();
        extern int alpha_8530_cnrint();
        extern int cnxint();
        extern int cnrint();
        
        /*
         * This variable must be setup prior to calling the console init routine.  It
         * must be setup to be the base address (BB) of the Gbus uart registers.
         */
        /*
         * ***PRM FIX: to make this more generic gbus_uart0_pg_base should be
         * renamed to a more appropriate name, such as duart_pg_base. Also the
         * addresses, or offsets to the other duart registers would be better
         * table driven, than offset driven, since this would limit access
         * variants to initialization time only (of the address table)
         * and not to the run time access routine pointed to by cnrw_8530.
         */
        
        extern vm_offset_t *gbus_uart0_pg_base; /* base addr of gbus uart0 */
        
        long    alpha_8530_cons_intr_reg = 0;
        struct tty      cons;
        
        
        /*
         * Alpha Ruby - console routines
         */
        int     alpha_8530_cnstart();
        int     alpha_8530_cnrw_8530();
        int     (*cnrw_8530)();
        extern  int     ttrstrt();
        extern  int     ttydef_open();
        extern  int     ttydef_close();
        
        /* Macros */
        
        /*
         * Need a few items for autoconfig of devices on "ibus"
         */
        caddr_t rubycnstd[] = { 0 };
        
        int alpha_8530_cons_probe();
        int alpha_8530_cons_attach();
        
        struct controller *rubycninfo[1];
        struct  driver rubycndriver = { 
                alpha_8530_cons_probe, 0, alpha_8530_cons_attach, 
                0, 0, rubycnstd, 0, 0, "rubycn", rubycninfo };



alpha_8530_cons_probe(reg1, reg2)
        int reg1, reg2;
{
        /*
         * Must init select/wakeup queue
         */
        queue_init(&cons.t_selq);
        
        /*
         * The intialization is done through alpha_8530_cons_init, so
         * if we have gotten this far we are alive so return a 1
         */
        
        return(1);
}

alpha_8530_cons_attach(reg)
        int reg;
{
        /*
         * The intialization is done through alpha_8530_cons_init, so
         * if we have gotten this far we are alive so return a 1
         */
        return(1);
}



alpha_8530_cons_init()
        /*
         * This routine is called (from ./kernel/arch/alpha/alpha_init.c)
         * through the console switch (./kernel/arch/alpha/cons_sw.c) using
         * the platform specific console switch structure
         * (./kernel/data/cons_sw_data.c)  
         *
         * For ruby this function serves the purpose of checking that the
         * console uart registers have been mapped; Sets the proper interrupt
         * vectors for the Console TX/RX interrupts and sets the pointer to
         * the uart chip access routine (specific to ruby, this module could
         * easily be generisized to drive any machine using an 8530 duart for 
         * its console port.)
         */
{
        register int vector_return;
        u_long vtop_addr;
        u_long alpha_8530_rxvec, alpha_8530_txvec;
        struct ctb_tt *ctb_tt;

        /* setup pointer to 8530 access routine... */
        cnrw_8530 = alpha_8530_cnrw_8530;
        
        if (svatophys( rpb, &vtop_addr) != KERN_SUCCESS)
                panic("ruby_cons: can't get phys addr of RPB");

        /*
         * Setup generic interrupt dispatching to console interrupts.
         * Use low order longword from CTB tx/rx interrupt fields to
         * set vector. 
         */

#ifdef PRM_DEBUG_OFF

        printf("***PRM: alpha_8530_cons_init: need sanity checks for CTB.\n");

#endif /* PRM_DEBUG */

        ctb_tt = (struct ctb_tt *)( (u_long) rpb  +
                                   rpb->rpb_ctb_off +
                                   (CONSOLE_CTB * rpb->rpb_ctb_size)); 
        
        alpha_8530_rxvec = (int)ctb_tt->ctb_rivec;
        alpha_8530_txvec = (int)ctb_tt->ctb_tivec;

        intrsetvec(alpha_8530_rxvec, alpha_8530_cnrint, 0);
        intrsetvec(alpha_8530_txvec, alpha_8530_cnxint, 0);

#ifdef PRM_DEBUG  

        printf("***PRM: ruby_cons: alpha_8530_cons_init: exit...\n");

#endif /* PRM_DEBUG */

        return(0);

}



alpha_8530_cnopen(dev, flag)
        dev_t dev;
        unsigned int flag;
        /*
         *
         */
{
        register struct tty *tp;
        register int s;
	register int clocal = TRUE;         /* console is a local connection */

        if (minor(dev) != 0) {
                return (ENXIO);
        }
        tp = &cons;
        tp->t_oproc = alpha_8530_cnstart;

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
        
        alpha_8530_enable_interrupts();
        
        splx(s);
        
        return((*linesw[tp->t_line].l_open)(dev, tp, flag));
}



alpha_8530_cnclose(dev)
        dev_t dev;
{
        register struct tty *tp = &cons;
        
        if (minor(dev) != 0) {
                return (ENXIO);
        }
        (*linesw[tp->t_line].l_close)(tp);

        if ((tp->t_state & TS_ISOPEN) == 0) {
                tp->t_state &= ~TS_CARR_ON;
        }
        
        ttyclose(tp);

	tty_def_close(tp);	
        
        alpha_8530_disable_uart_interrupts();
}

alpha_8530_cnread(dev, uio, flag)
        dev_t dev;
        struct uio *uio;
	unsigned int flag;
{
        register struct tty *tp = &cons;
        
        if (minor(dev) != 0) {
                return (ENXIO);
        }
        
        return ((*linesw[tp->t_line].l_read)(tp, uio, flag));
}

alpha_8530_cnwrite(dev, uio, flag)
        dev_t dev;
        struct uio *uio;
	unsigned int flag;
{
        register struct tty *tp = &cons;
        
        if (minor(dev) != 0) {
                return (ENXIO);
        }
        
        return ((*linesw[tp->t_line].l_write)(tp, uio, flag));
}



alpha_8530_cnioctl(dev, cmd, addr, flag)
        dev_t dev;
        unsigned int cmd;
        caddr_t addr;
        unsigned int flag;
{
        register struct tty *tp = &cons;
        int error;
	int unit = 0;

	struct devget *devget;
        
        if (minor(dev) != unit) {
                return (ENXIO);
        }
        
	if ((error = (*linesw[tp->t_line].l_ioctl)(tp, cmd, addr, flag))  >= 0) {
		return(error);
	}

	if ((error = ttioctl(tp, cmd, addr, flag)) >= 0){
		return (error);
	}
	
	switch (cmd) {
	      case DEVIOCGET:					/* device status */
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
alpha_8530_cnrint(line_num)
        register int line_num;  /* parameter not used */
{
        register struct tty *tp = &cons;
        int c;
        
        /* loop while rxcs indicates an outstanding character */
        
        if ( ! alpha_8530_rx_rdy() )
                return(0);
        
        while( alpha_8530_dmagetc(&c) ) {
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
alpha_8530_cnxint(line_num)
        register int line_num;  /* not used */
{
        struct tty *tp = &cons;
        
        tp->t_state &= ~TS_BUSY;
        
        if (tp->t_line) {
                (*linesw[tp->t_line].l_start)(tp);
        } else {
                alpha_8530_cnstart(tp);
        }
}



/*
 * Start output if there are any characters in the output queue.
 */
alpha_8530_cnstart(tp)
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
        }
        if (tp->t_outq.c_cc == 0) {     /* output queue empty */
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
        
/***PRM: Removed because this is invoked on every keystroke. */
/*        alpha_8530_write_txdb(XON); */
        
        /* Move characters from outq onto the transmit ring */
        while (cc--) {
                if(alpha_8530_dmaputc(*(tp->t_outq.c_cf)))
                        break;               /* transmit ring is full */
                ndflush(&tp->t_outq,1);
        }
        mb();
        
 out:
        splx(s);
        return;
}



rubystop(tp, flag)
        register struct tty *tp;
{
        register int s;
        
        s = spltty();
        if (tp->t_state & TS_BUSY) {
                alpha_8530_write_txdb(XOFF);
        }
        splx(s);
}




/*
 * Used for console printf.  This is a non-interrupt driven polled mode routine
 * used to put characters out to the console terminal one character at a time.
 */
alpha_8530_cnputc(c)
        register int c;
{
        register int s;
        
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
        
        while( ! alpha_8530_tx_rdy() )
                ;       /* wait */
        
        alpha_8530_write_txdb(c);
        mb();
        
        if (c == '\n') {                /* map carriage return - line feed */
                alpha_8530_cnputc('\r');
        }
        splx(s);
}




/*
 * Console terminal getc routine.
 * This is a non-interrupt driven polled mode routine
 * used to read characters from to the console terminal one character at a time.
 */
alpha_8530_cngetc()
{
        volatile register int t;
        register int s, c;
        
        s = splextreme();
        
        
        while( ! alpha_8530_rx_rdy() )
                ;       /* wait for character */
        
        c = alpha_8530_read_rxdb();     /* get char */
        c &= 0xff;

        splx(s);
        return(c);
}




/*
 * This routine will only stuff characters if the txbuf is not full. Unlike
 * the alpha_8530_cnputc routine it will NOT spin if the buffer is full.  This 
 * routine is called by alpha_8530_start() to simulate dma.  It is expected that
 * when the buffer becomes non-full an interrupt will be generated and any
 * additional characters will be transmitted.
 */

alpha_8530_dmaputc(c)
        register int c;
{
        register int t;
        
        /*
         * If the transmit buffer is not empty return 1 to signify so,
         * the adu_dmaputc of the character will be retried later when
         * a transmitter interrupt occurs which indicates that the
         * transmit buffer is empty.
         */
        if (! alpha_8530_tx_rdy() )
        {
                return(1);
        }
        
        /* Stuff the character into the transmit ring */
        alpha_8530_write_txdb(c);
        return(0);
}



/*
 * This routine will check if a character is available, and if not will
 * return 0.  If a character is available, it will fill in the character
 * and return 1.  This routine is called from the receiver interrupt
 * routine and is called from a loop that will continue as long as there
 * are characters to be read.
 */
alpha_8530_dmagetc(cptr)
        register u_char   *cptr;
{
        volatile register int t;
        register int s;
        
        s = splextreme();
        
        /*
         * If not character; restore entry ipl and return 0.
         */
        if( ! alpha_8530_rx_rdy() ) {
                splx(s);
                return(0);
        }
        
        /* get character */
        *cptr = alpha_8530_read_rxdb() & 0xff;  /* get char */

        /* restore ipl and return */
        splx(s);
        return (1);
}



/*
 * Routine to access to 8530 UART data and control registers.
 *
 * cnrw_8350 routine parameters:
 *   read : boolean. If true then perform read, else do write.
 *   unit : Not implemented... will be used to provide access to the
 *          other uart registers...
 *   reg  : used for a 'switch' RR0, RR8, other. performs direct access
 *          for RR0 and RR8, performs indexed access for other.
 *   data : pointer to data buffer. data used to write UART, or buffer
 *          used to store data read from UART.
 *
 * It is left to the caller to control the interrupt priority level.
 */

#define WRITE 0
#define READ 1

/* Make sure referance is byte only, not quadword. */

int alpha_8530_cnrw_8530(u_long read, u_long unit, u_long reg, char *data) {
        char *base;
        char s;
        
        base = (char *)gbus_uart0_pg_base + GBUS_UART0A_PG_OFFSET;
        
        switch (reg) {
        case RR0: {
                if (read) *data = *base;
                else {
                        *base = *data;
                        mb();
                }
                break;
        }
        case RR8: {
                base = base+0x40;
                if (read) *data = *base;
                else {
                        *base = *data;
                        mb();
                }
                break;
        }
        default: {
                
                *base = (char)reg;      /* setup next access for correct register */
                mb();
                
                if (read) *data = *base;
                else {
                        *base = *data;
                        mb();
                }
                
                break;
        }
        }
}



alpha_8530_read_rxdb()
{
        char c;
        
        cnrw_8530(READ, UART_LOCAL, RR8, &c);
        
        return (c);
}

alpha_8530_write_txdb(c)
        register char c;
{
        alpha_8530_write_8530(WR8, c);
        while( ! alpha_8530_tx_rdy() )
                ;       /* wait */
}

alpha_8530_rx_rdy()
{  
        char cs;
        
        cnrw_8530(READ, UART_LOCAL, RR0, &cs);
        /*    cs &= (1<<GBUS_UART_RxAV); */
        cs &= (1<<00);
        
        return (cs & 0x01);
}

alpha_8530_tx_rdy()
{
        char cs;
        
        cnrw_8530(READ, UART_LOCAL, RR0, &cs);
        cs &= (1<<GBUS_UART_TxEM);
        
        return (cs);
}



alpha_8530_puts(str)
        register char *str;
{
        int s;
        
        s = splextreme();
        
        while (*str)
                cnputc(*str++);
        
        while( ! alpha_8530_tx_rdy() )
                ;       /* wait for tx_rdy */
        
        splx(s);
}



alpha_8530_cons_gets(cp)
        char *cp;
{
        register char *lp;
        register c;
        
        lp = cp;
        for (;;) {
                c = cngetc() & 0177;
                switch (c) {
                case '\n':
                case '\r':
                        *lp++ = '\0';
                        alpha_8530_puts('\n');
                        return;
                case '\177':
                        lp--;
                        if (lp < cp)
                                lp = cp;
                        continue;
                default:
                        cnputc(c);
                        *lp++ = c;
                }
        }
}



alpha_8530_enable_interrupts ()
{
        u_char    cs = UART_TXINTENA | UART_RXINTENA;
        struct ctb_tt *ctb_tt;

#ifdef PRM_DEBUG_OFF
        
        printf ("\n***PRM: alpha_8530_enable_interrupts\n");
        
#endif /* PRM_DEBUG */

        ctb_tt = (struct ctb_tt *)( (u_long) rpb  +
                                   rpb->rpb_ctb_off +
                                   (CONSOLE_CTB * rpb->rpb_ctb_size));

        /*
         * indicate to the console that rx/tx interrupts are being enabled.
         * These fields in the RPB use the high order bit to indicate
         * interrupt enable. Clear that field.
         */
        ctb_tt->ctb_tivec |= 0x8000000000000000;
        ctb_tt->ctb_rivec |= 0x8000000000000000;
        
        /* Can't determine current state of interrupts so just blast the */
        /* enable bits by writing WR1... */
        alpha_8530_write_8530 (WR1, cs);
        
}



alpha_8530_disable_uart_interrupts ()
{
        u_char    cs = 0;
        struct ctb_tt *ctb_tt;

#ifdef PRM_DEBUG_OFF
        
        printf ("\n***PRM: alpha_8530_disable_interrupts\n");
        
#endif /* PRM_DEBUG */
        
        ctb_tt = (struct ctb_tt *)( (u_long) rpb  +
                                   rpb->rpb_ctb_off +
                                   (CONSOLE_CTB * rpb->rpb_ctb_size)); 

        /*
         * indicate to the console that rx/tx interrupts are being disabled.
         * These fields in the RPB use the high order bit to indicate
         * interrupt enable. Clear that field.
         */
        ctb_tt->ctb_tivec &= ~0x8000000000000000;
        ctb_tt->ctb_rivec &= ~0x8000000000000000;
        
        /* Can't determine current state of interrupts so just blast the */
        /* enable bits by writing WR1... */
        alpha_8530_write_8530 (WR1, cs);
}



u_char alpha_8530_read_8530 (reg)
        u_int reg;
{
        u_char   *base, alpha_8530_read_byte();
        
        /* set up write pointer register for next access */
        base = (u_char *)gbus_uart0_pg_base + GBUS_UART0A_PG_OFFSET;
        alpha_8530_write_byte (base, (u_char)reg);
        mb ();
        
        return (alpha_8530_read_byte (base));
}

alpha_8530_write_8530(reg, data)
        u_int  reg;
        u_char data;
{
        u_char   *base;
        
        /* set up write pointer register for next access */
        base = (u_char *)gbus_uart0_pg_base + GBUS_UART0A_PG_OFFSET;
        alpha_8530_write_byte (base, (u_char)reg);
        mb ();
        
        alpha_8530_write_byte (base, data);
        mb ();
        
}

u_char alpha_8530_read_byte (addr)
        u_char *addr;
{
        u_int   longword;
        
        longword = *(u_int *)addr;
        
        return ((u_char)(longword));
}



alpha_8530_write_byte (addr, byte)
        u_char *addr, byte;
{
        u_long  quadword;
        
        quadword = (byte);      
        
        *(u_long *)(addr) = quadword;
}
