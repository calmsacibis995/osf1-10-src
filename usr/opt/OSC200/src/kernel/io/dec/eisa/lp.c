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
static char *rcsid = "@(#)$RCSfile: lp.c,v $ $Revision: 1.1.4.5 $ (DEC) $Date: 1993/12/09 20:25:38 $";
#endif

/*
 * Centronics Parallel (Printer) Port Driver
 * Author: Tim Burke	(12/21/92)
 *
 * Some of the code was modeled after the parallel printer port stuff
 * from the old ULTRIX DMB driver.  Some of it is modeled after a i386
 * version from the bsd tree.
 *
 * The parts of the driver which may be Jensen specific are marked with
 * #ifdef JENSEN_LPT.
 */

/* Included all the files as in apc.c.  Clean up this list later. */
#include <sys/secdefines.h>
#if SEC_BASE
#include <sys/security.h>
#endif
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/ioctl.h>
#include <sys/tty.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/map.h>
#include <sys/buf.h>
#include <sys/vm.h>
#include <sys/conf.h>
#include <sys/file.h>
#include <sys/uio.h>
#include <sys/kernel.h>
#include <sys/devio.h>
#include <vm/vm_kern.h>
#include <kern/xpr.h>
#include <hal/cpuconf.h>
#include <io/common/devdriver.h>
#include <io/dec/eisa/eisa.h>		/* EISA bus definitions		*/
#include <machine/cpu.h>
#include "lp.h"

#define JENSEN_LPT	/* Marks off Jensen platform specifics */

#ifdef JENSEN_LPT
/*
 * JENSEN: This assumes that bit 4 of the Control Register is set to
 * 1 (by default) to enable the printer port (CS3).  (see jensen spec
 * 4.5.6.4).
 * This assumes that the default base address is 3BC-3BF and that the
 * extended mode control bit is set thereby disabling the bi-directional
 * capability.
 * If you can believe Table 50 of the Jensen spec these are valid assumptions.
 */

#define LPT1_BASE_ADDR	0x3BC		/* CSR Base Address - note this can
					 * change depending on the setting
					 * in the Control Register 0. */
#define LPT2_BASE_ADDR	0x378
#define LPT3_BASE_ADDR	0x278

#endif /* JENSEN_LPT */

/*
 ************ HARDWARE Definitions *************************************
 * 
 * The following defines the access to the device registers.
 */
/*
 * The spec defines the registers as register0, register1, register2.  I'm
 * assuming this means they are at offsets 0, 1, 2 respectively.
 */
#define LPT_REGISTER_0	0
#define LPT_REGISTER_1	1
#define LPT_REGISTER_2	2

#define LPT_DATA	LPT_REGISTER_0		/* line printer port data */
#define LPT_STAT	LPT_REGISTER_1		/* LPT port status 	  */
#define LPT_CTRL	LPT_REGISTER_2		/* LPT port control	  */

/*
 * Format of the port STATUS register:
 * (From VTI spec for VL82C106 page 6-192)
 * Bits 0 & 1 reserved, read as 1's.
 */
#define LPT_STAT_NIRQ	0x4	/* interrupt status bit			*/
#define LPT_STAT_NERROR	0x8	/* set to 0 to indicate error		*/
#define LPT_STAT_SLCT	0x10	/* status of SLCT lead from printer	*/
#define LPT_STAT_PE	0x20	/* set to 1 when out of paper		*/
#define LPT_STAT_NACK	0x40	/* acknowledge - set to 0 when ready	*/
#define LPT_STAT_NBUSY	0x80	/* busy status bit, 0=busy, 1=ready	*/

/*
 * Format of the port CONTROL register:
 * (From VTI spec for VL82C106 page 6-192)
 * Bits 6 & 7 reserved, read as 1's.
 */
#define LPT_CTRL_STROBE	0x1	/* Printer Strobe Control		 */
#define LPT_CTRL_AUTOFD	0x2	/* Auto Feed Control			 */
#define LPT_CTRL_NINIT	0x4	/* Initialize Printer Control		 */
#define LPT_CTRL_SLCT	0x8	/* Select Input Control			 */
#define LPT_CTRL_IRQ	0x10	/* Interrupt Request Enable Control	 */
#define LPT_CTRL_DIR	0x20	/* Direction control			 */


/*
 * I/O Macros for register reads and writes.  The base address
 * is set-up in attach.  For jensen,  the ctlr->physaddr is the
 * vti combo base address,  which the jensen r/w bus routines look
 * for to determine which swizzle to do.
 *
 */
#define LPT_READ(regname, value) \
    value = READ_BUS_D8((lpc->base)+(regname))
#define LPT_WRITE(regname, value) \
    WRITE_BUS_D8((lpc->base)+(regname),(value)); mb()


/*
 ************ SOFTWARE Definitions *************************************
 *
 * The following defines the representation of the software state of the
 * driver.
 */

#define LP_MAXBUF     0x100	/* Max. chars. per dma */
#define NLPn 10
				/* Somewhat bogus as there really is no dma. */

struct lpctrl
	{
	int			unit;	 /* Printer unit (minor) number    */
        u_long                  base;    /* Base address of unit registers */
	int			irq;     /* Interrupt used by controller   */
        struct controller	*ctrl;   /* Address of controller struct   */
	char			flags;   /* Printer state		   */
	int			error;   /* Error status		   */
	int			count;   /* Number of chars in buf	   */
	int			control; /* Software copies of the         */
					 /* correspondig hardware register */
	char			*cp;	 /* Offset within buf current char */
	char			buf[LP_MAXBUF]; /* Bogus DMA buffer        */
	} lp_ctrl[NLPn];

/*
 * Values for the flags field used to represent driver state in a lpctrl
 * data structure.
 */
#define LPF_ACTIVE 0x01
#define LPF_OPEN   0x02
#define LPF_BUSY   0x04
#define LPF_PAPER  0x08
#define LPF_ERROR  0x10
#define LPF_TIMEOUT  0x20

#define LPWATCH 2	/* Poll for errors every 2 seconds */

/*
 * Routine forward declarations.
 */
int lpprobe(), lpattach(), lpout();
void lpwatchdog(), lpintr(), lpiowait(), lpiodone();

/*
 * Driver structure to keep config/ioconf happy.
 */
caddr_t lpstd[] = { 0 };		/* dummy entry */
struct  controller *lpinfo[1];		/* dummy entry */
struct driver lpdriver = { lpprobe, 0, lpattach, 0, 0,
			      lpstd, 0, 0, "lp", lpinfo };

int
lpprobe(caddr_t *paddr, struct controller *ctrl)
{
    return 1;
    }

int 
lpattach(struct controller *ctrl)
{
    struct lpctrl *lpc;
    int slot;
    int unit;
    struct eisa_info *einf;
    struct e_port port;

    unit = ctrl->ctlr_num;
    slot = ctrl->slot;                 
    einf = ctrl->eisainfo;

    lpc = &lp_ctrl[unit];
    lpc->unit = unit;
    lpc->flags = LPF_ACTIVE;
    lpc->base = (u_long)ctrl->addr; 

    if ( slot>0 && einf  )
    {
	if ( get_config( ctrl, EISA_PORT, "", &port, 0 )== -1)
	{
	    printf("aceprobe: error return from get_config()\n" );
	}

	lpc->base = port.base_address;
    }
    else                                /* this is a baseboard port */
    {
        lpc->base += LPT1_BASE_ADDR;
	lpc->irq = 1;
    }

    /* 
     * Initialize the hardware registers. 
     * Of particular interest is that fact that the init bit is
     * clear here which will initiate a reset on the printer.  Delay
     * for 50 us to give it a chance to take affect.
     */
    lpc->control = 0;
    LPT_WRITE( LPT_CTRL, lpc->control);
    
    DELAY(50);	
    
    /*
     * Set the init bit to complete the reset.  Turn on the 
     * input select bit to cause the printer to go online.  (This may
     * not be the best place for that; perhaps in the open routine?)
     */
    lpc->control |= (LPT_CTRL_SLCT | LPT_CTRL_NINIT);
    LPT_WRITE( LPT_CTRL, lpc->control);
    return 1;
}

/*
** open device
*/

int
lpopen(dev, flag)
	dev_t dev;
	int flag;
{
	int unit = minor(dev);
	struct lpctrl *lpc = &lp_ctrl[unit];
	unsigned char val;
	register int s;
 
	/*
	** make sure device is configured 
	*/

	if ((unit >= NLPn) || ((lpc->flags & LPF_ACTIVE) == 0))
		return(ENXIO);

	/*
	** only one open !
	*/

	if (lpc->flags & LPF_OPEN)
		return(EBUSY);

	/*
	**	check direction, write-only; no read interface provided.
	*/

	if (flag & FREAD)
		return(ENODEV);

	/*
	** check printer status
	*/

	LPT_READ( LPT_STAT, val);
	if (val & LPT_STAT_PE) {
		/* Consider changing to uprintf */
		printf("lp%d: out of paper\n", unit);
		return(EIO);
	}
	if ((val & LPT_STAT_NERROR) == 0) {
		/* Consider changing to uprintf */
		printf("lp%d: printer error\n", unit);
		return(EIO);
	}
	if ((val & LPT_STAT_SLCT) == 0) {
		/* Consider changing to uprintf */
		printf("lp%d: printer offline\n", unit);
		return(EIO);
	}

	/*
	** start watchdog timer
	*/

	timeout(lpwatchdog, lpc, LPWATCH*hz);
	
	/*
	** enable interrupts and mark as opened
	*/

	lpc->control |= (LPT_CTRL_IRQ);
	s = spltty();
	LPT_WRITE( LPT_CTRL, lpc->control);
	splx(s);

	lpc->flags |= LPF_OPEN;

	return(0);
}

/*
** close device
*/

int 
lpclose(dev, flag)
	dev_t dev;
	int flag;
{
	int unit = minor(dev);
	struct lpctrl *lpc = &lp_ctrl[unit];
	register int s;

	if ((unit >= NLPn) || ((lpc->flags & LPF_OPEN) == 0))
		return(ENXIO);

	/*
	** disable interrupts but keep selected, mark as closed
	*/
	s = spltty();
	lpc->control &= ~(LPT_CTRL_IRQ);
	LPT_WRITE( LPT_CTRL, lpc->control);
	splx(s);
	untimeout(lpintr, unit);
	lpc->flags &= ~LPF_OPEN;
	return(0);
}

int
lpwrite(dev,uio)
	dev_t dev;
	struct uio *uio;
{
	register unsigned int n;
	register int error;

	int unit = minor(dev);
	struct lpctrl *lpc = &lp_ctrl[unit];
	register int s;

	unit = minor(dev);

	if (lpc->flags & LPF_ERROR)
		return(EIO);
	while(n = min(LP_MAXBUF,(unsigned)uio->uio_resid)) {
		uio->uio_rw = UIO_WRITE; 
		if (error = uiomove(&lpc->buf[0],(int)n,uio)) {
			printf("uio move error\n");
			return(error);
		}
		s = spltty();
		error = lpout(unit,&lpc->buf[0],n);
		splx(s);
		if (error)
			return(error);
	}
	return(0);
}

/*
** fetch next buffer and start output
*/

int 
lpout(unit, addr, len)
	int unit;
	caddr_t addr;
	int len;
	{
	struct lpctrl *lpc = &lp_ctrl[unit];

	if ( unit > NLPn ) 
        {
		printf("lpout: invalid unit %d.\n",unit);
		return(1); 
	}

	lpc->count = len;
	lpc->cp = addr;
	lpc->flags |= LPF_BUSY;
	lpc->error = 0;

	lpintr(unit);
	
	/*
	** wait for i/o to complete
	*/

	lpiowait(lpc);

	return(lpc->error);
}

/*
** output next characters until printer becomes busy
** interrupt driven except for the first time
*/
 
void 
lpintr(unit)
int unit;
{
	struct lpctrl *lpc = &lp_ctrl[unit];
	char val;
	int spincount = 0;

	if (unit > NLPn) {
		printf("lpintr: bogus unit %d\n",unit);
	}
	/*
	** check for spurious interrupt
	*/
	if (!(lpc->flags & LPF_BUSY)) {
		return;
	}

	/*
	 * Make sure that there are no previous timeout calls posted to this
	 * routine when the printer was busy.
	 */
	untimeout(lpintr, unit);
	/*
	** any more to print
	*/
	while (lpc->count) {
		LPT_READ( LPT_STAT, val);
		/* A "0" indicates that the printer is busy and not ready
		 * to accept data. */
		if (!(val & LPT_STAT_NIRQ)) {
			/* The miserable manual doesn't tell you
			 * how to dismiss the interrupt!  I'm guessing that
			 * a hack way to do it is to clear the interrupt
			 * enable bit - gack!
			 */
			lpc->control &= ~(LPT_CTRL_IRQ);
			LPT_WRITE( LPT_CTRL, lpc->control);
		}
		/* 
		 * This stupid chip will generate an ACK even if it puts
		 * the printer into a BUSY state.  The problem is that once
		 * the printer is no longer BUSY it isn't smart enough to
		 * generate another interrupt.  So to work around this feature
		 * you could either spin or use a timeout mechanism.  I 
		 * chose the latter approach because printers can be slow;
		 * no need to tie up the while system spinning at high IPL
		 * just for a printer.
		 */
		if ((val & LPT_STAT_NBUSY) == 0) {
			timeout(lpintr, unit, hz/70);
			/* The above hack cleared this bit in a feeble attempt
		 	 * to dismiss the interrupt.  Reset the interrupt
		 	 * enable bit here for the next character.
		 	 */
			lpc->control |= (LPT_CTRL_IRQ);
			LPT_WRITE( LPT_CTRL, lpc->control);
			return;
		}
		/* The above cleared this bit in a feeble
		 * attempt to dismiss the interrupt.  Reset the interrupt
		 * enable bit here for the next character.
		 */
		lpc->control |= (LPT_CTRL_IRQ);
		LPT_WRITE( LPT_CTRL, lpc->control);
		/*
		** output the character
		*/

		LPT_WRITE( LPT_DATA, *lpc->cp++);
		--lpc->count;

		/*
		** valid data must be present for at least 0.5 us
		*/
		DELAY(1);	/* granularity? */

		/*
		** strobe for 1 us to cause the printer to latch the data.
		*/

		lpc->control |= LPT_CTRL_STROBE;
		LPT_WRITE( LPT_CTRL, lpc->control);
		DELAY(1);	/* granularity? */
		lpc->control &= ~(LPT_CTRL_STROBE);
		LPT_WRITE( LPT_CTRL, lpc->control);

		/*
		** valid data must be present for at least 0.5 us
		*/
		DELAY(1);	/* granularity? */
	}

	lpiodone(lpc);
}

/*
** wait for i/o to finish
*/

void 
lpiowait(lpc)
	struct lpctrl *lpc;
{
	int s;

	s = spltty();
	while (lpc->flags & LPF_BUSY)  {
		/* sleep priority?  Have a timeout? */
		sleep((caddr_t)lpc, PZERO+1);
	}
	splx(s);
}

/*
** wakeup strategy function
*/
void 
lpiodone(lpc)
	struct lpctrl *lpc;
{
	lpc->flags &= ~LPF_BUSY;
	wakeup(lpc);
}

/*
** handle watchdog timeout
** This is used to check for errors on the printer and display an error 
** message to the console.
*/

void 
lpwatchdog(lpc)
struct lpctrl *lpc;
{
    char val;
    int s;
    int unit = lpc->unit;

    /*
    ** are we still running ?
    */

    s = spltty();	

    if ((lpc->flags & LPF_OPEN) == 0) 
        {
    	splx(s);
    	return;
        }

    /*
    ** check printer status
    */
    LPT_READ( LPT_STAT, val);

    /*
    ** report errors only once
    */

    if (val & LPT_STAT_PE && (lpc->flags & LPF_PAPER) == 0) {
        printf("lp%d: out of paper, please fix\n", lpc->unit);
    	lpc->flags |= LPF_PAPER;
    }
    if (lpc->flags & LPF_PAPER && (val & LPT_STAT_PE) == 0)
    	lpc->flags &= ~LPF_PAPER;

    if ((val & LPT_STAT_NERROR) == 0 && (lpc->flags & LPT_STAT_SLCT) == 0) {
    	printf("lp%d: not ready, please fix\n", lpc->unit);
    	lpc->flags |= LPF_ERROR;
    }
    if (lpc->flags & LPF_ERROR && (val & LPT_STAT_NERROR) == 0)
    	lpc->flags &= ~LPF_ERROR;
    
    /*
    ** restart watchdog timer
    */

    timeout(lpwatchdog, lpc, LPWATCH*hz);
    splx(s);
}
