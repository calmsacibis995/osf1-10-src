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
static char	*sccsid = "@(#)$RCSfile: scc.c,v $ $Revision: 1.2.31.8 $ (DEC) $Date: 1993/12/10 20:36:03 $";
#endif
/* 
 * derived from scc.c	4.7      (ULTRIX)  1/22/91";
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
 * scc.c
 *
 * SCC SLU console driver
 *
 * Modification history
 *
 * 18-Sep-1991 - Andrew Duane
 *	Modified for support of ALPHA FLAMINGO
 *		and a bunch of portability fixes.
 *
 * 09-Aug-1991 - Mike Larson
 *	Take out break and parity processing (done in tty driver), set
 *	tp->t_param in sccopen(), supply return values to sccparam(),
 *	fix sccselect() formal parameters, call line disipline close
 *	function unconditionally in sccclose(), use CLOCAL instead of
 *	sccsoftCAR in sccstart() to determine if line is a modem, and
 *	conditionally compile debug printf's.
 *
 * 10-Jun-1991 - khh
 *	Passed correct number of parameters to line open routine, corrected
 *	sccparam routine, etc.
 *
 * 10-May-1991 - Paul Grist
 * 	Made handling of IOASIC Base address generic, to support other
 *	IOASIC-based systems like 3max+.
 *
 * 10-Apr-1991 - khh
 *	OSF work started above this line.
 *-----------------------------------------------------------------------
 *
 * 21-Jan-1991 - Randall Brown
 *	Modified use of tc_isolate_memerr() to now use tc_memerr_status
 *	struct.
 *
 * 06-Dec-1990 - Randall Brown 
 *	Added the call to tc_isolate_memerr() in the dma_xerror routine to
 *	log error information about the memory error.
 *
 * 06-Nov-1990 - pgt
 *      Added disabling of baud rate generator before time constants are
 *      set. Fixed the setting of breaks. Modify dma interrupt routines
 *      to do appropriate masking. Fixed handling of Speed Indicate.
 *	Did general clean-up. 
 * 
 * 13-Sep-90 Joe Szczypek
 *	Added new TURBOchannel console ROM support.  osconsole environment
 *	variable now returns 1 slot number if serial line, else 2 slot number
 *	if graphics.  Use this to determine how to do setup.  Note that new
 *	ROMs do not support multiple outputs...
 *
 * 7-Sept-1990 - pgt 
 *	Enabled modem control and break interrupts. Also fixed sccparam
 *	and added macros to handle enabling and disabling of modem and
 *	interrupts.
 *
 * 16-Aug-1990 - Randall Brown
 *	Enable use of mouse and keyboard ports.
 *
 * 20-Feb-1990 - pgt (Philip Gapuz Te)
 * 	created file.
 *
 */

#ifdef	REX_SUPPORT
#undef	REX_SUPPORT
#endif

#include <data/scc_data.c> 
#include <data/ws_data.c>
#include <data/lk201_data.c>
#include <data/vsxxx_data.c>
#include <machine/rpb.h>

int	scc_intr;

/*
 *  mjm - sync support: START.
 */

/* A SCC register save set for each line */
struct scc_saved_reg saved_regs[NSCCLINE];

/* Global interrupt switch tables */
struct SIR_Interrupt_sw	SIRsw[NSCC_DEV] = { 0, 0 };
struct SCC_Interrupt_sw SCCsw[NSCCLINE] = { 0, 0, 0, 0 };

/* Flags indicate which lines are configured as sync */
static u_char	syncConfigured[NSCCLINE] = { 0, 0, 0, 0 };

/* number of SCC devices on this cpu */
int nscc_dev = 0;

/* scc interrupt sources */
u_long scc_extstat[2] = { SCC_RR3_A_EXT_IP, SCC_RR3_B_EXT_IP };
u_long scc_rxspec[2] = { SCC_RR3_A_RIP, SCC_RR3_B_RIP };
u_long scc_txip[2] = { SCC_RR3_A_TIP, SCC_RR3_B_TIP };

/*  forward declarations of interrupt service routines */
int	scc_dma_rerror(), scc_dma_rint(), scc_dma_xerror();
int	scc_dma_xint(), sccisr(), scc_mkbd_rint();
int	scc_ext_rint(), scc_spec_rint();
int	scc_stub();

/*
 *  mjm - sync support: END.
 */

int	sccprobe(), sccprobe_reg(), sccattach(), sccattach_ctrl();
int	scc_dsr_check(), scc_tty_drop(), scc_cd_drop(); /* Modem */

/*
int	sccstart(), scc_dma_xint(), sccbaudrate();
*/
int	sccstart(), scc_dma_xint(), scc_dma_rint(), sccparam(); 
int	ttrstrt();
int     sccspeedi(); scc_half_speed();

extern int scc_temp_kint();

/* God help us if this is ever de-referenced on FLAMINGO! */
vm_offset_t scc_ioasic_base; 	   /* Variable base address of IOASIC*/

int	num_iic = 0;	/* identifies if DTi present on system */
struct driver sccdriver = { sccprobe_reg, 0, sccattach_ctrl, 0, 0, sccstd, 0, 0, "scc", sccinfo };

extern int cpu;			   /* Global cpu type value */
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
extern int prom_getenv();

/* These two routines don't seem to exist anywhere... */
#if 0
#define tty_def_close cons_def_close
#define tty_def_open cons_def_open
#endif

#define LINEMASK	0x03		/* line unit mask */

int scc_other[4] = { 3, 2, 1, 0 };

/* ioasic register bits */
u_long scc_xdma_en[4] = { 0, 0, SSR_COMM1_XEN, SSR_COMM2_XEN};
u_long scc_rdma_en[4] = { 0, 0, SSR_COMM1_REN, SSR_COMM2_REN};
u_long scc_xint[4] = { 0, 0, SIR_COMM1_XINT, SIR_COMM2_XINT };
u_long scc_rint[4] = { 0, 0, SIR_COMM1_RINT, SIR_COMM2_RINT };
u_long scc_xerror[4] = { 0, 0, SIR_COMM1_XERROR, SIR_COMM2_XERROR };
u_long scc_rerror[4] = { 0, 0, SIR_COMM1_RERROR, SIR_COMM2_RERROR };
u_long scc_int[4] = { 0, 0, SIR_SCC0, SIR_SCC1 };     /* mjm - sync support */

/*
 * Baud Rate Support
 *
 * When the baud rate on the right is specified, the line parameter register
 * is setup with the appropriate bits as specified in the left column.
 */
#define BAUD_UNSUPPORTED 0	/* Device does not provide this baud rate */
#define BAUD_SUPPORTED   1	/* Device does provide this baud rate     */

/*
 * The SCC manual provides a formula to compute the time constants for a
 * specified baudrate. For B110 and B134.5, the formula does not yield
 * whole integers. Thus, the chip baudrate in these two cases is very close
 * but not equal to the specified baudrate.
 */
struct speedtab scc_speeds[] = {
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
    {-1,	 -1}		    /* END OF TABLE */
};

struct speedtab ss_lo_speeds[] =
{
	B0,	0,
	B50,	SCC_WR12_B50_LO,
	B75,	SCC_WR12_B75_LO,
	B110, 	SCC_WR12_B110_LO,
	B134,	SCC_WR12_B134_5_LO,
	B150,	SCC_WR12_B150_LO,
	B200,	SCC_WR12_B200_LO,
	B300,	SCC_WR12_B300_LO,
	B600,	SCC_WR12_B600_LO,
	B1200,	SCC_WR12_B1200_LO,
	B1800,	SCC_WR12_B1800_LO,	
	B2400,	SCC_WR12_B2400_LO,
	B4800,	SCC_WR12_B4800_LO,
	B9600,	SCC_WR12_B9600_LO,
	B19200,	SCC_WR12_B19200_LO,
	B38400,	SCC_WR12_B38400_LO,
	-1, 	-1
};

struct speedtab ss_hi_speeds[] =
{
	B0,	0,
	B50,	SCC_WR13_B50_HI,
	B75,	SCC_WR13_B75_HI,
	B110, 	SCC_WR13_B110_HI,
	B134,	SCC_WR13_B134_5_HI,
	B150,	SCC_WR13_B150_HI,
	B200,	SCC_WR13_B200_HI,
	B300,	SCC_WR13_B300_HI,
	B600,	SCC_WR13_B600_HI,
	B1200,	SCC_WR13_B1200_HI,
	B1800,	SCC_WR13_B1800_HI,	
	B2400,	SCC_WR13_B2400_HI,
	B4800,	SCC_WR13_B4800_HI,
	B9600,	SCC_WR13_B9600_HI,
	B19200,	SCC_WR13_B19200_HI,
	B38400,	SCC_WR13_B38400_HI,
	-1, 	-1
};

extern int kdebug_state();
int console_line = SCC_COMM2;

#ifdef __alpha
/* These original SCCx_x_BASE macros were MIPS-specific (PHYS_TO_K1) */
/* They have been changed for FLAMINGO to offset from specific addresses */
/* that are mapped at boot time, or (with OSFPAL) to use PHYS_TO_KSEG. */
/* They are only used once, in the routine: scc_cons_init()  */
#define SCC0_B_BASE (PHYS_TO_KSEG((scc_ioasic_base) + 0x00200000))
#define SCC0_A_BASE (PHYS_TO_KSEG((scc_ioasic_base) + 0x00200010))
#define SCC1_B_BASE (PHYS_TO_KSEG((scc_ioasic_base) + 0x00300000))
#define SCC1_A_BASE (PHYS_TO_KSEG((scc_ioasic_base) + 0x00300010))
#else
#define SCC0_B_BASE (PHYS_TO_K1((scc_ioasic_base) + 0x00100000))
#define SCC0_A_BASE (PHYS_TO_K1((scc_ioasic_base) + 0x00100008))
#define SCC1_B_BASE (PHYS_TO_K1((scc_ioasic_base) + 0x00180000))
#define SCC1_A_BASE (PHYS_TO_K1((scc_ioasic_base) + 0x00180008))
#endif	/* __alpha */

/*
 *  mjm - sync support: START
 *  Modified macros to use wbflush() and remove DELAY().
 *  sc_saved_regs[] array was modified to be pointers.
 */

#ifdef __alpha
#define SCC_READ(rsp, reg, val)   { \
	     (rsp)->SCC_CMD = ((u_int)(reg))<<8; \
	     mb(); \
	     (val) = (((rsp)->SCC_CMD)>>8)&0xff; \
	     DELAY(10); \
	     }

#define SCC_WRITE(rsp, reg, val)  { \
	     (rsp)->SCC_CMD = ((u_int)(reg))<<8; \
	     mb(); \
	     (rsp)->SCC_CMD = ((u_int)(val))<<8; \
	     mb(); \
	     DELAY(10); \
	     }
#else
#define SCC_READ(rsp, reg, val)   { \
	     (rsp)->SCC_CMD = ((u_short)(reg))<<8; \
	     wbflush();\
	     (val) = (((rsp)->SCC_CMD)>>8)&0xff; \
	     }

#define SCC_WRITE(rsp, reg, val)  { \
	     (rsp)->SCC_CMD = ((u_short)(reg))<<8; \
	     (rsp)->SCC_CMD = ((u_short)(val))<<8; \
	     }
#endif	/* __alpha */

/* these macros can only set or clear one bit at a time */
#define SCC_MSET(unit, bit) { \
	     sccsc->sc_saved_regs[(unit)]->wr5 |= (bit); \
	     SCC_WRITE(sccsc->sc_regs[(unit)], SCC_WR5, \
		       sccsc->sc_saved_regs[(unit)]->wr5); \
             }

#define SCC_MCLR(unit, bit)   { \
	     sccsc->sc_saved_regs[(unit)]->wr5 &= ~(bit); \
	     SCC_WRITE(sccsc->sc_regs[(unit)], SCC_WR5, \
		       sccsc->sc_saved_regs[(unit)]->wr5); \
             }

#define SCC_MTEST(unit, bit)  (((sccsc->sc_regs[(unit)]->SCC_CMD)>>8)&(bit))

/* DTR makes use of the DTR of channel A, controlled by WR5 */
#define SCC_SET_DTR(unit)   SCC_MSET(scc_other[(unit)], SCC_WR5_DTR)
#define SCC_CLR_DTR(unit)   SCC_MCLR(scc_other[(unit)], SCC_WR5_DTR)

#define SCC_SET_RTS(unit)   SCC_MSET(scc_other[(unit)], SCC_WR5_RTS)
#define SCC_CLR_RTS(unit)   SCC_MCLR(scc_other[(unit)], SCC_WR5_RTS)

#define	SCC_SET_SS(unit)    SCC_MSET((unit), SCC_WR5_RTS)
#define	SCC_CLR_SS(unit)    SCC_MCLR((unit), SCC_WR5_RTS)

#define SCC_SET_BRK(unit)   SCC_MSET((unit), SCC_WR5_BRK)
#define SCC_CLR_BRK(unit)   SCC_MCLR((unit), SCC_WR5_BRK)

#define SCC_DSR(unit)   SCC_MTEST(scc_other[(unit)], SCC_RR0_SYNC)
#define SCC_CTS(unit)   SCC_MTEST((unit), SCC_RR0_CTS)
#define SCC_DCD(unit)   SCC_MTEST((unit), SCC_RR0_DCD)

#define SCC_SI(unit)	SCC_MTEST(scc_other[(unit)], SCC_RR0_CTS)
#define SCC_RI(unit)	SCC_MTEST(scc_other[(unit)], SCC_RR0_DCD)
#define SCC_XMIT(unit)  (SCC_DSR((unit)) && SCC_CTS((unit)) && SCC_DCD((unit)))

#define SCC_DTR(unit) ((sccsc->sc_saved_regs[scc_other[(unit)]]->wr5)&SCC_WR5_DTR)
#define SCC_RTS(unit) ((sccsc->sc_saved_regs[scc_other[(unit)]]->wr5)&SCC_WR5_RTS)

#define SCC_MODEM_ON(unit)  { \
	SCC_WRITE(sccsc->sc_regs[scc_other[(unit)]], SCC_WR15, SCC_WR15_SYNC_IE); \
	sccsc->sc_regs[scc_other[(unit)]]->SCC_CMD = SCC_WR0_RESET_EXT_INT<<8; \
	sccsc->sc_regs[scc_other[(unit)]]->SCC_CMD = SCC_WR0_RESET_EXT_INT<<8; \
	SCC_WRITE(sccsc->sc_regs[(unit)], SCC_WR15, (SCC_WR15_CTS_IE|SCC_WR15_DCD_IE|SCC_WR15_BREAK_IE)); \
        sccsc->sc_regs[(unit)]->SCC_CMD = SCC_WR0_RESET_EXT_INT<<8; \
	sccsc->sc_regs[(unit)]->SCC_CMD = SCC_WR0_RESET_EXT_INT<<8; \
    }

#define SCC_MODEM_OFF(unit)  { \
	SCC_WRITE(sccsc->sc_regs[scc_other[(unit)]], SCC_WR15, 0x00); \
	sccsc->sc_regs[scc_other[(unit)]]->SCC_CMD = SCC_WR0_RESET_EXT_INT<<8; \
	sccsc->sc_regs[scc_other[(unit)]]->SCC_CMD = SCC_WR0_RESET_EXT_INT<<8; \
	SCC_WRITE(sccsc->sc_regs[(unit)], SCC_WR15, SCC_WR15_BREAK_IE); \
	sccsc->sc_regs[(unit)]->SCC_CMD = SCC_WR0_RESET_EXT_INT<<8; \
	sccsc->sc_regs[(unit)]->SCC_CMD = SCC_WR0_RESET_EXT_INT<<8; \
    }

#define SCC_INT_ON(unit) { \
	if ((unit) == SCC_KYBD || (unit) == SCC_MOUSE) { \
	    sccsc->sc_saved_regs[(unit)]->wr1 &= ~SCC_WR1_RINT; \
	    sccsc->sc_saved_regs[(unit)]->wr1 |= SCC_WR1_RINT_ALL; \
	    SCC_WRITE(sccsc->sc_regs[(unit)], SCC_WR1, sccsc->sc_saved_regs[(unit)]->wr1); \
        } else { \
	    sccsc->sc_saved_regs[scc_other[(unit)]]->wr1 |= SCC_WR1_EXT_IE; \
	    SCC_WRITE(sccsc->sc_regs[scc_other[(unit)]], SCC_WR1, sccsc->sc_saved_regs[scc_other[(unit)]]->wr1); \
	    sccsc->sc_saved_regs[(unit)]->wr1 &= ~SCC_WR1_RINT; \
	    sccsc->sc_saved_regs[(unit)]->wr1 |= (SCC_WR1_RINT_SPC| SCC_WR1_EXT_IE| SCC_WR1_WDMA_EN); \
	    SCC_WRITE(sccsc->sc_regs[(unit)], SCC_WR1, sccsc->sc_saved_regs[(unit)]->wr1); \
        } \
    }

#define PRINT_SIGNALS() { cprintf("Modem signals: "); \
	if (SCC_DSR(2)) cprintf(" DSR2 "); \
	if (SCC_CTS(2)) cprintf(" CTS2 "); \
	if (SCC_DCD(2)) cprintf(" CD2 "); \
	if (SCC_SI(2)) cprintf(" SI2 "); \
	if (SCC_RI(2)) cprintf(" RI2 "); \
	if (SCC_DSR(3)) cprintf(" DSR3 "); \
	if (SCC_CTS(3)) cprintf(" CTS3 "); \
	if (SCC_DCD(3)) cprintf(" CD3 "); \
	if (SCC_SI(3)) cprintf(" SI3 "); \
	if (SCC_RI(3)) cprintf(" RI3 "); \
	cprintf("\n"); }

#define PRINT_SIR(sir) { \
	    if ((sir) & SIR_COMM1_RINT)  cprintf("R1 "); \
	    if ((sir) & SIR_COMM2_RINT)  cprintf("R2 "); \
	    if ((sir) & SIR_COMM1_XINT)  cprintf("X1 "); \
	    if ((sir) & SIR_COMM2_XINT)  cprintf("X2 "); \
	    }

/* Look Out! Hardwired Address! */
#ifdef mips
#define PRINT_SSR(line) { \
	    if ((*(u_int *)(0xbc040100)) & scc_rdma_en[(line)]) \
	      cprintf("SSR: RDMA Enabled: %d\n", (line)); \
	    }
#endif

/*
 *  mjm - sync support: END
 */

sccprobe_reg(reg, ctrl)
int reg, ctrl;
{
#ifdef SCCDEBUG
	printf("sccprobe_reg called\n");
#endif
	return (1);
}

sccprobe(ctrl)
int ctrl;
{
#ifdef SCCDEBUG
	printf("sccprobe called\n");
#endif
    /* the intialization is done through scc_cons_init, so
     * if we have gotten this far we are alive so return a 1
     */
    sccattach(ctrl);	/* osf added */
    return(1);
}

sccattach_ctrl(ctrl)
struct controller *ctrl;
{
#ifdef SCCDEBUG
	printf("sccattach_ctrl called\n");
#endif

    /*
     *  mjm - sync support: START.
     *  Allow sync device to be probed and attached if it can be configured.
     *  For MAXine or Pelican using a console, do NOT probe or attach for sync driver.
     *  FLAMINGO is always probed for sync comms.
     */
#ifdef MIPS
    if (cpu != DS_MAXINE || consDev == GRAPHIC_DEV)
#endif
#ifdef __alpha
    if (cpu != DEC_3000_300 || consDev == GRAPHIC_DEV)
#endif
    {
	syncConfigured[SCC_COMM1] = sscc_Probe(SCC_COMM1);
	if (syncConfigured[SCC_COMM1]) {
	    sscc_Attach(SCC_COMM1);
	}			
    }

#ifdef MIPS
    if (consDev == GRAPHIC_DEV  &&  /* Console MUST NOT be using SCC_COMM2 */
	cpu != DS_MAXINE) {   	    /* MAXINE does NOT have SCC_COMM2 */
	syncConfigured[SCC_COMM2] = sscc_Probe(SCC_COMM2);
	if (syncConfigured[SCC_COMM2]) {
	    sscc_Attach(SCC_COMM2);
	}
    }
#endif

#ifdef SYNCSCCDEBUG
	printf("sccattach_ctrl: syncConfigured[COM1]: %d\n", syncConfigured[SCC_COMM1]);
#endif

    /*
     *  mjm - sync support: END.
     */

}

sccattach(ctrl)
int ctrl;
{
    register struct scc_softc *sc = sccsc;
    register int i;
    register struct scc_reg *rsp, *rsp0;
    register struct scc_saved_reg *ssp, *ssp0;
    vm_offset_t	phys;

    sccsoftCAR = 0xff;
    sccdefaultCAR = 0xff;

#ifdef SCCDEBUG
	printf("sccattach called\n");
#endif

#ifdef DS5000_100
    if(cpu != DS_MAXINE)
#endif
    /* Pelican is a special case. Since it has only one serial line, */
    /* which is console_line, we only need to set registers for     */
    /* that line.                                                   */
    if (cpu == DEC_3000_300)
        {
	
       /* If kdebug is active, don't touch the serial port that it's talking to */
	if (kdebug_state() == 0) {
	    if (consDev == GRAPHIC_DEV)
		scc_init(console_line);
	    
	    SCC_CLR_DTR(console_line);         /* clear modem control signals */
	    SCC_CLR_RTS(console_line);
	    SCC_CLR_SS(console_line);
	    /*
	     * pgt: can not use IOC_CLR(reg, mask) to clear system interrupt
	     * register bits because bits may change between the time the register
	     * value is saved and the time it is written back with the mask. See
	     * ioasic.h for definition.
	     */
	    IOC_WR(IOC_SIR, ~scc_rint[console_line]);
	    IOC_WR(IOC_SIR, ~scc_xerror[console_line]);
	    IOC_WR(IOC_SIR, ~scc_rerror[console_line]);
	    /* set rdma ptr, enable rdma */
	    svatophys((sc->rbuf[console_line][sc->rflag[console_line]] +
		       SCC_HALF_PAGE - SCC_WORD), &phys);
	    sc->ioc_regs[console_line]->RDMA_REG = (u_long)(phys << 3);
	    IOC_SET(IOC_SSR, scc_rdma_en[console_line]);
	}
        }
    else
       {
       /* If kdebug is active, don't touch the serial port that it's talking to */
       if (kdebug_state() == 0)
	   scc_init(SCC_COMM1);
	   
       if (consDev == GRAPHIC_DEV)
	   scc_init(console_line);
	   
       for (i = SCC_COMM1; i <= SCC_COMM2; i++)
	  {
	  if (kdebug_state() && (i == SCC_COMM1))
              continue;
	  SCC_CLR_DTR(i);          /* clear modem control signals */
	  SCC_CLR_RTS(i);
	  SCC_CLR_SS(i);
/*
 * pgt: can not use IOC_CLR(reg, mask) to clear system interrupt register bits 
 * because bits may change between the time the register value is saved and 
 * the time it is written back with the mask. See ioasic.h for definition.
 */
	  IOC_WR(IOC_SIR, ~scc_rint[i]);
	  IOC_WR(IOC_SIR, ~scc_xerror[i]);
	  IOC_WR(IOC_SIR, ~scc_rerror[i]);
	  /* set rdma ptr, enable rdma */
	  svatophys((sc->rbuf[i][sc->rflag[i]] + SCC_HALF_PAGE - SCC_WORD), &phys);
	  sc->ioc_regs[i]->RDMA_REG = (u_long)(phys << 3);
	  IOC_SET(IOC_SSR, scc_rdma_en[i]);
	  }
       }
    for (i = 0; i < NSCCLINE; i++)
	queue_init(&scc_tty[i].t_selq);
}

/*
 * scc_init() sets the SCC modes
 */
scc_init(unit)
     register int unit;
{
    register struct scc_softc *sc = sccsc;
    register struct scc_reg *rsp, *rsp0;
    register struct scc_saved_reg *ssp, *ssp0;
    register int line = unit;
    
    rsp = sc->sc_regs[line];
    ssp = sc->sc_saved_regs[line];  /* mjm - sync support */
    
    /*
     * set modes:
     *   WR9   force hardware reset
     *   WR4   line 1: odd parity, one stop bit, x16 clock
     *         lines 0, 2, 3: ~no parity, one stop bit, x16 clock
     *   WR1   ~W/DMA: DMA request, receive; parity is special condition
     *   WR2   interrupt vector 0x00
     *   WR3   8 bits/char
     *   WR5   8 bits/char
     *   WR9   interrupt disabled 0x00
     *   WR10  NRZ
     *   WR11  tx & rx clocks = brgen, TRxC input, RTxC ~no xtal
     *   WR12  low time constant
     *   WR13  hi time constant
     *         baud rate: line 0 or 1: 4800 baud
     *                    line 2 or 3: 9600 baud
     *   WR14  brgen source = pclk
     *         line 2 or 3: channel B: ~DTR/REQ: request function, transmit
     *                      channel A: ~DTR/REQ: ~DTR function
     */
    if (line == SCC_KYBD || line == SCC_MOUSE) {
	SCC_WRITE(rsp, SCC_WR9, SCC_WR9_RESETA);        
    } else {
        SCC_WRITE(rsp, SCC_WR9, SCC_WR9_RESETB);        
    }
/* rpbfix : do we need to delay this long ???? */
    DELAY(20);
    if (line == SCC_MOUSE )  {              /* odd parity for mouse */ 
        ssp->wr4 = ( SCC_WR4_PENABLE | SCC_WR4_ONESB | SCC_WR4_CLOCK16);
	SCC_WRITE(rsp, SCC_WR4, ssp->wr4);
    } else {                        /* no parity for mouse and comm. ports */
        ssp->wr4 = (SCC_WR4_ONESB | SCC_WR4_CLOCK16);
	SCC_WRITE(rsp, SCC_WR4, ssp->wr4);
    }
    ssp->wr1 = (SCC_WR1_DMA_REQ | SCC_WR1_WDMA_RX | SCC_WR1_PSPC);
    SCC_WRITE(rsp, SCC_WR1, ssp->wr1);
    SCC_WRITE(rsp, SCC_WR2, 0xf0);     /* interrupt vector 0x00 */
    ssp->wr3 = SCC_WR3_RBITS8;
    SCC_WRITE(rsp, SCC_WR3, ssp->wr3);
    ssp->wr5 = SCC_WR5_TBITS8;
    SCC_WRITE(rsp, SCC_WR5, ssp->wr5); 
    SCC_WRITE(rsp, SCC_WR9, 0x00);     /*  WR9 interrupt disabled */
    SCC_WRITE(rsp, SCC_WR10, SCC_WR10_NRZ);
     /*
      * TRxC pin should be an input because we can not allow output from
      * the TRxC pin for asynchronous communication. See SCC manual.
      */
    SCC_WRITE(rsp, SCC_WR11, SCC_WR11_TxC_BRGEN | SCC_WR11_RxC_BRGEN);
	      
    if (line == SCC_KYBD || line == SCC_MOUSE) {  /* keyboard or mouse 4800 BPS */
        SCC_WRITE(rsp, SCC_WR12, SCC_WR12_B4800_LO);
	SCC_WRITE(rsp, SCC_WR13, SCC_WR13_B4800_HI);
    } else {                       /* 9600 BPS */
        SCC_WRITE(rsp, SCC_WR12, SCC_WR12_B9600_LO);
	SCC_WRITE(rsp, SCC_WR13, SCC_WR13_B9600_HI);
    } 
    if (line == SCC_KYBD || line == SCC_MOUSE) {  /* channel A - keyboard or mouse */
        ssp->wr14 = SCC_WR14_BRGEN_PCLK;
	SCC_WRITE(rsp, SCC_WR14, ssp->wr14);
    } else { 
        /*
	 * Communication ports 1 & 2 use the A channels of the mouse and
	 * keyboard respectively. Since the ~DTR/REQ bit selects DTR when it
	 * is zero, no need to do anything here.
	 */
	ssp->wr14 = (SCC_WR14_BRGEN_PCLK | SCC_WR14_REQ);
	SCC_WRITE(rsp, SCC_WR14, ssp->wr14);
    }
    /* mjm - sync support */
    ssp->wr9 = SCC_WR9_MIE;
    SCC_WRITE(rsp, SCC_WR9, ssp->wr9);
}

/* Make it 7 SCC_PAGE_SIZE pages, so 6 aligned pages can be used within it. */
/* If SCC_PAGE_SIZE evenly divides NBPG don't worry about being contiguous. */
char    line23_space[SCC_PAGE_SIZE * 7];

scc_cons_init()
{
    extern struct vcons_init_sw vcons_init[];
#ifdef __mips
    extern int rex_base;
#endif
    int i;
    char *space_pointer;
    register struct scc_reg *rsp;
    register struct scc_softc *sc;
    register struct scc_saved_reg *ssp;
    char *strptr;	/* Pointer to osconsole environment variable */
    int scc_mouse_init(), scc_mouse_putc(), scc_mouse_getc();
    int scc_kbd_init(), scc_kbd_putc(), scc_kbd_getc(), scc_putc();
    int dti_mouse_init(), dti_mouse_putc(), dti_mouse_getc();
    int dti_kbd_init(), dti_kbd_putc(), dti_kbd_getc(), scc_putc();

#if defined(SCCDEBUG) || defined (DEBUGGING)
printf("scc_cons_init\n");
#endif

    sccsc = &scc_softc[0];
 
    sc = sccsc;

    /*
     * Systems which make use of the IOASIC will have the
     * same registers with the same offsets, but may have
     * a different location for the base address of the
     * IOASIC.
     *
     */
    switch (cpu)
	{
#ifdef __mips
	  case DS_MAXINE:
	  case DS_5000_100: scc_ioasic_base = 0x1c000000 ;
	    break;

	  case DS_5000_300: scc_ioasic_base = 0x1F800000 ;
	    break;
#endif /* __mips */

#ifdef __alpha
	  case DEC_3000_500: scc_ioasic_base = 0x1f0000000;
	    break;

	  case DEC_3000_300: scc_ioasic_base = 0x1b0000000;
	    break;

#endif	/* __alpha */

	  default: panic("Unsupported cpu type in scc_cons_init");
	    break;
	}

    sc->sc_regs[SCC_KYBD] = (struct scc_reg *)SCC1_A_BASE;
    sc->sc_regs[SCC_MOUSE] = (struct scc_reg *)SCC0_A_BASE;
    sc->sc_regs[SCC_COMM1] = (struct scc_reg *)SCC0_B_BASE;
    sc->sc_regs[SCC_COMM2] = (struct scc_reg *)SCC1_B_BASE;

    sc->ioc_regs[SCC_COMM1] = (struct ioc_reg *)IOC_COMM1_DMA_BASE;
    sc->ioc_regs[SCC_COMM2] = (struct ioc_reg *)IOC_COMM2_DMA_BASE;

    vs_gdkint   = scc_temp_kint;

    /*
     *  mjm - sync support: START.
     */
    sc->sc_saved_regs[0] = &saved_regs[SCC1_A];	/* line 0 */
    sc->sc_saved_regs[1] = &saved_regs[SCC0_A];	/* line 1 */
    sc->sc_saved_regs[2] = &saved_regs[SCC0_B];	/* line 2 */
    sc->sc_saved_regs[3] = &saved_regs[SCC1_B];	/* line 3 */ 
  
    /*
     *  Begin initialising the Interrupt Switch Tables.
     */
     
    /* SCC0 is common to all types of cpu */
    SIRsw[0].RxOverrun = scc_dma_rerror;
    SIRsw[0].RxHalfPage = scc_dma_rint;
    SIRsw[0].TxReadError = scc_dma_xerror;
    SIRsw[0].TxPageEnd = scc_dma_xint;
    SIRsw[0].SCC = sccisr;

    /*
     *  mjm - sync support: END.
     */

    space_pointer = &line23_space[0];
    space_pointer = (char *)((vm_offset_t)(space_pointer + SCC_PAGE_SIZE - 1) & ~(SCC_PAGE_SIZE - 1));

#ifdef __alpha

/* Just use the kva as physical (uncached is implied by I/O space) */
#define dma_in_addr(kva,physaddr)	physaddr = kva;
#define dma_out_addr(kva,physaddr)	physaddr = kva;

#else	/* __alpha */

/* The incoming addresses are converted to uncached space for convenience */
/* The outgoing addresses are left alone, since cache is write-through */
#define dma_in_addr(kva,physaddr)	physaddr = (char *)K0_TO_K1(kva);
#define dma_out_addr(kva,physaddr)	physaddr = kva;

#endif	/* __alpha */

#ifdef __mips
    if(cpu == DS_MAXINE) 
	{
	console_line = MAXINE_CONSOLE_LINE;
        scc_intr = SCC_INTR & ~(RESERVED_SIR_BITS);

	/*
	 *  mjm - sync support: START.
	 */
	nscc_dev = 1;
	
	/*
	 * Complete initialisation of the Interrupt Switch Tables.
	 */

	/* MAXine does not have SCC_1 */
	SIRsw[1].RxOverrun = scc_stub;
	SIRsw[1].RxHalfPage = scc_stub;
	SIRsw[1].TxReadError = scc_stub;
	SIRsw[1].TxPageEnd = scc_stub;
	SIRsw[1].SCC = scc_stub;

	/* Unit 0 is not implemented on MAXine */
	SCCsw[0].RxSpecCond = scc_stub;
	SCCsw[0].ExtStatus = scc_stub;
	SCCsw[0].Transmit = scc_stub;

	/* Unit 1 only supplies modem control signals for unit 2 */
	SCCsw[1].RxSpecCond = scc_stub;
	SCCsw[1].ExtStatus = scc_ext_rint;
	SCCsw[1].Transmit = scc_stub;

	/* Unit 2 is the console on MAXine */
	SCCsw[2].RxSpecCond = scc_spec_rint;
	SCCsw[2].ExtStatus = scc_ext_rint;
	SCCsw[2].Transmit = scc_stub;

	/* Unit 3 is not implemented on MAXine */
	SCCsw[3].RxSpecCond = scc_stub;
	SCCsw[3].ExtStatus = scc_stub;
	SCCsw[3].Transmit = scc_stub;

	/*
	 *  mjm - sync support: END.
	 */

        dma_out_addr (space_pointer, sc->tbuf[console_line]);
        dma_in_addr (space_pointer + SCC_PAGE_SIZE, sc->rbuf[console_line][0]);
        dma_in_addr (space_pointer + (2 * SCC_PAGE_SIZE), sc->rbuf[console_line][1]);
	sc->rflag[console_line] = 0;
        }
    else 
#endif
	{
	if (cpu == DEC_3000_300)
	    console_line = 2;
	else 
	    console_line = DEFAULT_CONSOLE_LINE;

	scc_intr  = SCC_INTR;

	/*
	 *  mjm - sync support: START.
	 */
	nscc_dev = 2;	/* This cpu has 2 SCCs */
	
	/*
	 * Complete initialisation of the Interrupt Switch Tables
	 */

	/* SCC1 is second serial line */
	SIRsw[1].RxOverrun = scc_dma_rerror;
	SIRsw[1].RxHalfPage = scc_dma_rint;
	SIRsw[1].TxReadError = scc_dma_xerror;
	SIRsw[1].TxPageEnd = scc_dma_xint;
	SIRsw[1].SCC = sccisr;

	/* Unit 0 is keyboard & some of SCC_COMM2's modem control */
	SCCsw[0].RxSpecCond = scc_mkbd_rint;
	SCCsw[0].ExtStatus = scc_ext_rint;
	SCCsw[0].Transmit = scc_stub;

	/* Unit 1 is mouse & some of SCC_COMM1's modem control */
	SCCsw[1].RxSpecCond = scc_mkbd_rint;
	SCCsw[1].ExtStatus = scc_ext_rint;
	SCCsw[1].Transmit = scc_stub;
    
	/* Unit 2 is the SCC_COMM1 serial line */
	SCCsw[2].RxSpecCond = scc_spec_rint;
	SCCsw[2].ExtStatus = scc_ext_rint;
	SCCsw[2].Transmit = scc_stub;  /* not used by async driver */
	
	/* Unit 3 is SCC_COMM2 serial line, the console */
	SCCsw[3].RxSpecCond = scc_spec_rint;
	SCCsw[3].ExtStatus = scc_ext_rint;
	SCCsw[3].Transmit = scc_stub;  /* not used by async driver */

	/*
	 *  mjm - sync support: END.
	 */

        dma_out_addr (space_pointer, sc->tbuf[SCC_COMM1]);
        dma_in_addr (space_pointer + SCC_PAGE_SIZE, sc->rbuf[SCC_COMM1][0]);
        dma_in_addr (space_pointer + (2 * SCC_PAGE_SIZE), sc->rbuf[SCC_COMM1][1]);
        sc->rflag[SCC_COMM1] = 0;

        dma_out_addr (space_pointer + (3 * SCC_PAGE_SIZE), sc->tbuf[SCC_COMM2]);
        dma_in_addr (space_pointer + (4 * SCC_PAGE_SIZE), sc->rbuf[SCC_COMM2][0]);
        dma_in_addr (space_pointer + (5 * SCC_PAGE_SIZE), sc->rbuf[SCC_COMM2][1]);
        sc->rflag[SCC_COMM2] = 0;
	}

    /*
     *
     * Query the prom. The prom can be set such that the user 
     * could use either the alternate tty or the graphics console.
     * You get the graphics console if the first bit is set in
     * osconsole.  The user sets the console variable
     */

    {
	int cons_unit, console_slot, slot, found = 0;
	char *str;
	char *tc_slot_to_name();
	caddr_t tc_slot_to_addr(), addr;
	extern struct rpb_ctb *ctb;
	
	/*
	 * Copy LK_201-specific keyboard structure and
	 * VSXXX-specific mouse structures
	 * to the appropriate globals.
	 * It's done this way so that generic kernels are possible.
	 */
	keyboard = lk201_keyboard;
	mouse = vsxxx_mouse;
	mouse_closure = vsxxx_mouse_closure;
	
	/*
	 * init the slu structure no matter which console is active
	 */
	slu.mouse_init = scc_mouse_init;
	slu.mouse_putc = scc_mouse_putc;
	slu.mouse_getc = scc_mouse_getc;
	slu.kbd_init = scc_kbd_init;
	slu.kbd_putc = scc_kbd_putc;
	slu.kbd_getc = scc_kbd_getc;
	slu.slu_tty  = scc_tty;
	slu.slu_putc = scc_putc;
	
	cons_unit = *(long *)((long)ctb + 56); /* NOTE: *NOT* ctb->rpb_type! */
#ifdef DEBUGGING
printf("scc_cons_init: cons_unit %d  ctb->rpb_type %d\n",
       cons_unit, ctb->rpb_type);
#endif /* 0 */
	if (cons_unit == 0x3) {
	    
	    consDev = GRAPHIC_DEV; /* some xx_cons_inits need this set */

	    /* read which slot console is in */
	    console_slot = *(long *)((long)ctb + 240);
	    if (console_slot == 0) { /* embedded graphics */
		/*
		 * Note that these numbers are (slot number + 1)
		 * to mirror the console which returns the TC slot
		 * number as (slot + 1)
		 */
		switch (cpu) {
		  case DEC_3000_500:
		    console_slot = TC_CFB_SLOT;
		    break;

		  case DEC_3000_300:
		    console_slot = 7;
		    break;
		}
	    }
	    slot = console_slot - 1;
	    if (slot >= 0) {
	      str = tc_slot_to_name(slot);
	      if ((str != (char *) -1) && (str != (char *) 0))
	      { 
		  for ( i = 0; vcons_init[i].modname[0] != 0; i++) {
#ifdef DEBUGGING
		  printf ("checking slot %d=%s against %s\n",
			slot, str, vcons_init[i].modname);
#endif
		    if (!strcmp(str, vcons_init[i].modname)) {
			addr = tc_slot_to_addr(slot);
			
			/* check to see that driver will configure */
			/* if not, use generic console */
			if ((*vcons_init[i].cons_init)(addr, slot))
			    found = 1;
			break;
		    }
		  }
	      }
#ifdef DEBUGGING
	      else printf ("scc_cons_init ERROR: bad tc_slot_to_name(%d)\n",
		slot);
#endif
	    }
	    if (!found) {
		found = install_generic_console();
		if (!found)
		    printf("Error: unable to install generic console.\n");
		else
		    printf("Generic Console support installed.\n");
	    }
	    if (found)
		    return;

	    /* NOTE: if not found, fall through and use alt console!!! */

	    consDev = 0;	/* reset to alt console */

	    printf("Error: kernel not configured for graphics console\n");
	    printf("Attempting to switch to alternate console...\n");
	}
    }	    
#ifdef __mips
       if (!rex_base) {
	 if ((atoi(prom_getenv("osconsole")) & 0x1) == 1) {
	   slu.mouse_init = scc_mouse_init;
	   slu.mouse_putc = scc_mouse_putc;
	   slu.mouse_getc = scc_mouse_getc;
	   slu.kbd_init = scc_kbd_init;
	   slu.kbd_putc = scc_kbd_putc;
	   slu.kbd_getc = scc_kbd_getc;
	   slu.slu_tty  = scc_tty;
	   slu.slu_putc = scc_putc;

	   for( i = 0 ; vcons_init[i] ; i++ )
	     if ((*vcons_init[i])()) {	/* found a virtual console */
	       consDev = GRAPHIC_DEV;
	       goto fini;
	     }
	 }
	 if (cpu != DS_MAXINE)
	    {
	    scc_init(SCC_KYBD); /* need to initialize channel A for modem control */
	    scc_init(SCC_MOUSE);
	    }
	 /* 
	  * set up serial line as alternate console line: no parity, 9600 baud
	  */
	 scc_init(console_line);
       } else {	  /* We are rex based. */
          strptr = (char *)rex_getenv("osconsole");

	 if ((strlen(strptr)) > 1) 
	    {  /* ROM indicates graphic device */
	    if (num_iic == 0)
	       {  /* No dti present. */
	       slu.mouse_init = scc_mouse_init;
	       slu.mouse_putc = scc_mouse_putc;
	       slu.mouse_getc = scc_mouse_getc;
	       slu.kbd_init = scc_kbd_init;
	       slu.kbd_putc = scc_kbd_putc;
	       slu.kbd_getc = scc_kbd_getc;
	       }
	    else
	       { /* dti is present. */
	       slu.mouse_init = dti_mouse_init;
	       slu.mouse_putc = dti_mouse_putc;
	       slu.mouse_getc = dti_mouse_getc;
	       slu.kbd_init = dti_kbd_init;
	       slu.kbd_putc = dti_kbd_putc;
	       slu.kbd_getc = dti_kbd_getc;
	       }
	   slu.slu_tty  = scc_tty;
	   slu.slu_putc = scc_putc;
	   for( i = 0 ; vcons_init[i] ; i++ )
	     if ((*vcons_init[i])()) {	/* found a virtual console */
	       consDev = GRAPHIC_DEV;
	       return;
	     }
	 }  /* End ROM indicates graphic device. */

	 /* If we have no graphic device we have to set up the serial 	*/
	 /* line for use by the console. If cpu is MAXine do nothing 	*/
	 /* with lines 0 and 1 because they do not exist.		*/
	 if(cpu != DS_MAXINE) 
	     {
	     scc_init(SCC_KYBD); /* need to initialize channel A for modem control */
	     scc_init(SCC_MOUSE);
	     }
	 /* 
	  * set up serial line as alternate console line: no parity, 9600 baud
	  */
	 scc_init(console_line);
       }  /*End we are REX based. */
#endif	/* __mips */

    scc_init(console_line);

    /* enable functions */
    rsp = sc->sc_regs[console_line];
    ssp = sc->sc_saved_regs[console_line];    /*  mjm - sync support */
    ssp->wr14 |= SCC_WR14_BRGEN_EN;
    SCC_WRITE(rsp, SCC_WR14, ssp->wr14);      /*  WR14 BRG enable */
    ssp->wr5 |= SCC_WR5_TXEN;
    SCC_WRITE(rsp, SCC_WR5, ssp->wr5);        /*  WR5 Tx enable */
    ssp->wr3 |= SCC_WR3_RXEN;
    SCC_WRITE(rsp, SCC_WR3, ssp->wr3);	      /*  WR3 Rx enable */
}

scc_enable_keyboard()
{
    SCC_INT_ON(SCC_KYBD);
}

scc_enable_pointer()
{
    SCC_INT_ON(SCC_MOUSE);
}

sccopen(dev, flag)
     dev_t dev;
{
    register struct scc_softc *sc = sccsc;
    register struct tty *tp;
    register int unit;
    register int maj, error;
    int inuse;  /*hold state of inuse bit while blocked waiting for carr*/
    register struct scc_reg *rsp, *rsp0;
    register struct scc_saved_reg *ssp0;

    maj = major(dev);
    unit = minor(dev);

#ifdef SYNCSCCDEBUG
    printf("sccopen: unit:%d syncConfigured:%d consDev:%d maj:%d cpu:%d\n", unit, syncConfigured[unit], consDev, maj, cpu);
#endif

    /*
     *  mjm - sync support.
     *  Check if this unit has been configured for sync operation.
     */
    if (syncConfigured[unit])
	return (ENXIO);

    /*
     * If a diagnostic console is attached to serial line,
     * don't allow open of the printer port which is also on this line.
     * This could cause lpr to write to the console.
     */
    if((consDev != GRAPHIC_DEV) && (unit == console_line))
	return (ENXIO);

     /*
     * don't allow open of the line kdebug is using
     */
    if (kdebug_state() && (unit == SCC_COMM1))
        return (ENXIO);

    
    /* don't allow open of minor device 0 of major device SCCMAJOR */
    /* because it is already reserved for /dev/console */
    if ((maj != CONSOLEMAJOR) && (unit == 0))
	return (ENXIO);
#if !defined(OSF) && !defined(__OSF__)
    /* only allow open of /dev/console of major device 0 */
    if ((maj == CONSOLEMAJOR) && (unit != 0)) 
	return (ENXIO);
#endif
    /* If we are opening the console on the serial line set the unit */
    /* number correctly.					     */
    if ((consDev != GRAPHIC_DEV) && (maj == CONSOLEMAJOR) && (unit == 0))
	unit |= console_line;	/* diag console on SLU line 3 */
    
    /*
     * Check for valid unit number. For all SCC-based systems except MAXine,
     * unit numbers of 0, 1, 2, 3 are valid since these systems have four
     * serial lines. The MAXine has only one serial line and allows access
     * to units 0, 1 2.
     */
    if (unit >= scc_cnt)
	return (ENXIO);

#ifdef __alpha
    /* there is no unit 3 on Pelican */
    if (cpu == DEC_3000_300 && unit == 3)
	return (ENXIO);
#endif
#ifdef DS5000_100
    if (cpu == DS_MAXINE && unit == 3)
	return (ENXIO);
#endif

    /*
     * Call the graphics device open routine
     * if there is one and the open is for a graphic device.
     * If we go down this path we return from this function after
     * the graphic open.
     */
    if (vs_gdopen && (unit <= SCC_MOUSE)) {
	error = (*vs_gdopen)(dev, flag);
#ifdef NOT_ANY_MORE
/* interrupt enabling is now done in the "vs_gdopen" routine */
	if (error == 0) 
	{
#ifdef DS5000_100
	    if (cpu != DS_MAXINE)
	       /* sccparam(unit); */   /* osf call sccparam from graphic driver */
#endif
	       SCC_INT_ON(unit); /* turn on interrupts for kbd and mouse */
#ifdef __alpha
	    /* if this is MOUSE, do KYBD too... */
	    /* NOTE: assume that vs_gdopen for KYBD is *NOT* needed */
            if (unit == SCC_MOUSE)
                    SCC_INT_ON(SCC_KYBD);
#endif /* __alpha */
	}
#endif /* NOT_ANY_MORE */
	
	return(error);
    }

    /* 
     * This code is executed if we are opening a non graphic device.
     */
    tp = &scc_tty[unit];
    rsp = sc->sc_regs[unit];
    if (tp->t_state&TS_XCLUDE && u.u_uid != 0) {
	return (EBUSY);
    }
    
    while (tp->t_state&TS_CLOSING) { /* let DTR stay down for awhile */
	if (error = tsleep((caddr_t)&tp->t_rawq, TTIPRI | PCATCH,
				ttopen, 0))
		return (error);
    }
    tp->t_addr = (caddr_t)tp;
    tp->t_oproc = sccstart;
    tp->t_param = sccparam;
/*
    tp->t_baudrate = sccbaudrate;
    tty_def_open(tp, dev, flag, (sccsoftCAR&(1<<(unit&LINEMASK))));
*/
    
    if ((tp->t_state & TS_ISOPEN) == 0) {

	/* Set the default line discipline to termios */
	tp->t_line = TTYDISC;
	tty_def_open(tp, dev, flag, (sccsoftCAR&(1<<(unit&LINEMASK))));
	/*
	 * Prevent spurious startups by making the 500ms timer
	 * initially high.
	 */
	sccmodem[unit] = MODEM_DSR_START;
	/*
	 * Specify console terminal attributes.  Do not allow modem control
	 * on the console.  Setup <NL> to <CR> <LF> mapping.
	 */
	if((maj == CONSOLEMAJOR) && ((minor(dev)&3) == 0)) {
	    /* modem control not supported on console */ 
	    tp->t_cflag = CS8 | CREAD | CLOCAL;
	    tp->t_flags = ANYP|ECHO|CRMOD;
	    tp->t_iflag |= ICRNL; /* Map CRMOD */
	    tp->t_oflag |= ONLCR; /* Map CRMOD */
	    tp->t_ispeed = tp->t_ospeed = TTYDEF_SPEED;
	}
    	sccparam(tp, &tp->t_termios);
    	SCC_INT_ON(unit);    /* enable interrupts */
    }
#if     SEC_BASE
    else if (tp->t_state&TS_XCLUDE && !privileged(SEC_ALLOWDACACCESS, 0))
#else
    else if (tp->t_state&TS_XCLUDE && u.u_uid != 0)
#endif
    {
		return (EBUSY);
    }

    /* sccparam(tp, &tp->t_termios); 	
    SCC_INT_ON(unit);   */ /* enable interrupts */
    (void) spltty();

    /*
     * mjm - sync support.
     * Only the com port or printer port should play with these signals.
     * This fixes a coordination problem with the sync driver.
     * (Should this also apply to remainder of this routine???)
     */
    if ((unit == SCC_COMM1) || (unit == SCC_COMM2)) {
	/*
	 *  Enable modem interrupts and signals.
	 */
	SCC_MODEM_ON(unit);
	SCC_SET_DTR(unit);
	SCC_SET_RTS(unit);
	SCC_SET_SS(unit);
    }

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
	if(SCC_DSR(unit) && SCC_CTS(unit))
		sccmodem[unit] |= MODEM_CTS;

	(void) spl0();
	return ((*linesw[tp->t_line].l_open)(dev, tp, flag));
    }
    /*
     * After DSR first comes up we must wait for the other signals
     * before commencing transmission.
     */
    if ((flag & (O_NDELAY|O_NONBLOCK)) == 0) {
	/*
	 * Delay before examining other signals if DSR is being followed
	 * otherwise proceed directly to scc_dsr_check to look for
	 * carrier detect and clear to send.
	 */
	if (SCC_DSR(unit)) {
	    sccmodem[unit] |= (MODEM_DSR_START|MODEM_DSR);
	    tp->t_dev = dev; /* need it for timeouts */
	    /*
	     * Give CD and CTS 30 sec. to 
	     * come up.  Start transmission
	     * immediately, no longer need
	     * 500ms delay.
	     */
	    timeout(scc_dsr_check, tp, hz*30);
	    scc_dsr_check(tp);
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
			return (error);
	    }
	    /*
	     * See if we were awoken by a false call to the modem
	     * line by a non-modem.
	     */
	    if (sccmodem[unit]&MODEM_BADCALL){
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

sccclose(dev, flag)
     dev_t dev;
{
    register struct scc_softc *sc = sccsc;
    register struct tty *tp;
    register int unit, maj;
    register int s;
    extern int wakeup();
    
    unit = minor(dev);
    maj = major(dev);
    if((consDev != GRAPHIC_DEV) && (major(dev) == CONSOLEMAJOR) && 
       ((unit&LINEMASK) == 0))
	unit |= console_line;	/* diag console on SLU line 3 */
    /*
     * Call the graphics device close routine
     * if ther is one and the close is for it.
     */
    if (vs_gdclose && (unit <= SCC_MOUSE)) {
	(*vs_gdclose)(dev, flag);
	return;
    }
    tp = &scc_tty[unit];

    /*
     * Do line discipline specific close functions, including waiting
     * for output to drain.
     */
    (*linesw[tp->t_line].l_close)(tp);

    /*
     * Clear breaks for this line on close.
     */
    s = spltty();
    SCC_CLR_BRK(unit);
    splx(s);
    if ((tp->t_cflag&HUPCL) || (tp->t_state&TS_WOPEN) || (tp->t_state&TS_ISOPEN)==0) {
	tp->t_state &= ~TS_CARR_ON;   /* prevents recv intr. timeouts */
	/*
	 * Drop appropriate signals to terminate the connection.
	 */
	SCC_CLR_DTR(unit);
	SCC_CLR_RTS(unit);
	SCC_CLR_SS(unit);
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
	    if (SCC_DSR(unit) || (!SCC_DCD(unit))) {
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
    sccsoftCAR &= ~(1<<(unit&LINEMASK));
    sccsoftCAR |= (1<<(unit&LINEMASK)) & sccdefaultCAR;
    sccmodem[unit] = 0;
    /* ttyclose() must be called before clear up termio flags */
    ttyclose(tp);
    tty_def_close(tp);
}

sccread(dev, uio, flag)
     dev_t dev;
     struct uio *uio;
     int flag;
{
    register struct tty *tp;
    register int unit;
    
    unit = minor(dev);
    if((consDev != GRAPHIC_DEV) && (major(dev) == CONSOLEMAJOR) && ((unit&LINEMASK) == 0))
	unit = console_line;	/* diag console on SLU line 3 */
    tp = &scc_tty[unit];
    return ((*linesw[tp->t_line].l_read)(tp, uio, flag));
}

sccwrite(dev, uio, flag)
     dev_t dev;
     struct uio *uio;
     int flag;
{
    register struct tty *tp;
    register int unit;
    
    unit = minor(dev);
    if((consDev != GRAPHIC_DEV) && (major(dev) == CONSOLEMAJOR) && ((unit&LINEMASK) == SCC_KYBD))
	unit = console_line;	/* diag console on SLU line 3 */
    /*
     * Don't allow writes to the mouse,
     * just fake the I/O and return.
     */
    if (vs_gdopen && (unit == SCC_MOUSE)) {
	uio->uio_offset = uio->uio_resid;
	uio->uio_resid = 0;
	return(0);
    }
    tp = &scc_tty[unit];
    return ((*linesw[tp->t_line].l_write)(tp, uio, flag));
}

sccselect(dev, events, revents, scanning)
dev_t dev;
short *events, *revents;
int scanning;
{
    register int unit = minor(dev);
    
    if((consDev != GRAPHIC_DEV) && (major(dev) == CONSOLEMAJOR) && (unit == SCC_KYBD))
	dev |= console_line;
    if ((unit == SCC_MOUSE) && vs_gdselect) {
	return((*vs_gdselect)(dev, events, revents, scanning));
    }
    return(ttselect(dev, events, revents, scanning));
}

/*
 *  mjm - sync support:  START.
 *  Modify interrupt dispatching to support the sync driver.  The next
 *  two routines are needed for sync support.
 */
#ifdef _ANSI_C_SOURCE
sccintr(u_int ctrl)
#else
sccintr(ctrl)
u_int ctrl;
#endif
{
    register int sir;   
    register int device;
    register int unit;
    
    IOC_RD(IOC_SIR, sir);
    while (sir & scc_intr) {
	/*
	 *  Process all interrupts generated for each SCC.
	 */
	for (device = 0; device < nscc_dev; device++) {
	    unit = device + 2;
	    
	    if (sir & scc_rerror[unit])
		(*SIRsw[device].RxOverrun)(unit);
	    
	    if (sir & scc_rint[unit])
		(*SIRsw[device].RxHalfPage)(unit);
	    
	    if (sir & scc_xerror[unit])
		(*SIRsw[device].TxReadError)(unit);
	    
	    if (sir & scc_xint[unit])
		(*SIRsw[device].TxPageEnd)(unit);
	    
	    if (sir & scc_int[unit])
		(*SIRsw[device].SCC)(unit);

	    IOC_RD(IOC_SIR, sir);
	    
	} /* for (each device) */
    } /* while (sir pending) */
}

/*
 *  sccisr()
 * 
 *  This routine defines how the SCC interrupts are dispatched.
 *  At boot time, the SCCsw ISR routines are initialised (see cons_init).
 *
 *  Parameter:
 *
 *	unit	-   SCC_COMM1 or SCC_COMM2.
 *
 */
sccisr(unit)
register int unit;
{
    register struct scc_softc *sc = sccsc;
    register struct scc_reg *rsp;
    register int	line;
    register int	ch;
    register int	ip;
    
    rsp = sc->sc_regs[scc_other[unit]];  /* channel A of unit */
    SCC_READ(rsp, SCC_RR3, ip); /* read RR3A for interrupts pending */

    /*
     *  For both channels, service any interrupts pending.
     *
     *  NOTE:  The order in which these interrupts are serviced is
     *  significant.  As described in the SCC tech. manual, the interrupt
     *  priorities are Rx -> Tx -> ExtStat, with CHA having a higher
     *  priority than CHB.
     */
    for (ch=SCC_CHA, line=scc_other[unit]; ch <= SCC_CHB; ch++, line=unit) {

	if (ip & scc_rxspec[ch])
	    (*SCCsw[line].RxSpecCond)(line);	/* Rx Interrupt */

	if (ip & scc_txip[ch])
	    (*SCCsw[line].Transmit)(line);	/* Tx Interrupt */

	if (ip & scc_extstat[ch])
	    (*SCCsw[line].ExtStatus)(line);	/* ExtStat Interrupt */

    } /* for (each cahnnel on device) */
}

/*
 *  mjm - sync support: END.
 */

/* should never happen */
scc_dma_rerror(line)
     register int line;
{
    register struct scc_softc *sc = sccsc;
    vm_offset_t phys;

    printf("Com. Port. Receive DMA Overrun, line = %d\n", line);
    /* reset DMA pointer */
    svatophys(sc->rbuf[line][sc->rflag[line]] + SCC_HALF_PAGE - SCC_WORD, &phys);
    sc->ioc_regs[line]->RDMA_REG = (u_long)(phys << 3);
    IOC_WR(IOC_SIR, ~scc_rerror[line]);	      /* clear error bit to restart */
}

scc_dma_xerror(line)
     register int line;
{
    register struct scc_softc *sc = sccsc;
    caddr_t pa;
    struct tc_memerr_status status;

    pa = (caddr_t)((sc->ioc_regs[line]->XDMA_REG) >> 3);
    printf("Com. Port. Transmit DMA Read Error, line = %d\n", line);
    status.pa = pa;
    status.va = 0;
    status.log = TC_LOG_MEMERR;
    status.blocksize = 1;
    tc_isolate_memerr(&status);
    IOC_SET(IOC_SSR, scc_xdma_en[line]);     /* enable transmit DMA */
    IOC_WR(IOC_SIR, ~scc_xerror[line]);
}

/*
 * Used to pass mouse (or tablet) reports to the graphics
 * device driver interrupt service routine.
 * Entire report passed instead of byte at a time.
 */
scc_mkbd_rint(unit)
     register int unit;
{
    register struct scc_softc *sc = sccsc;
    register u_short ch;	/* ch is 16 bits long */
    register u_short c;
    register ws_pointer_report *new_rep;
    u_short data;
    register struct scc_reg *rsp;
    register char ip;
    
    /*
     * If console is a graphics device,
     * pass keyboard input characters to
     * its device driver's receive interrupt routine.
     * Save up complete mouse report and pass it.
     */
    if ((unit <= SCC_MOUSE) && vs_gdkint) {
	new_rep = &current_rep;			/* mouse report pointer */
	rsp = sc->sc_regs[unit];                      /* line = mouse or keybd */
	SCC_READ(rsp, SCC_RR3, ip);                   /* read IP bits from RR3A */
	while (ip & SCC_RR3_A_RIP) {                /* channel A receiver */
	    c = (rsp->SCC_DATA)>>8;			/* read char */
	    ch = ( unit<<8 ) | ( c & 0xff );            /* encode line in ch */
	    if(unit == SCC_KYBD) {		/* keyboard char */
		(*vs_gdkint)(ch);
	    } else {			/* mouse or tablet report */
		if (pointer_id == MOUSE_ID) { /* mouse report */
		    data = ch & 0xff;	/* get report byte */
		    ++new_rep->bytcnt;	/* inc report byte count */
		    if (data & START_FRAME) { /* 1st byte of report? */
			new_rep->state = data;
			if (new_rep->bytcnt > 1)
			    new_rep->bytcnt = 1;  /* start new frame */
                      }
                    else
                      {
                       if((new_rep->state & 0xff) == 0xa0) /* Reset Command */
                         {
                        if (new_rep->bytcnt == 2)
                                new_rep->dx = data;        /* pointer_id */

                         else if (new_rep->bytcnt == 3)    /* 3nd byte */
                                new_rep->dy = data;        /* button cnfg */

                         else if (new_rep->bytcnt == 4) {  /* 4th byte */
                                new_rep->bytcnt = 0;  /* Quit no pass event*/
                                (*vs_gdkint)(0400); } /* 400 says line 1 */
                         }
                       else
                         {
                         if (new_rep->bytcnt == 2) {             /* 2nd byte */
                         new_rep->dx = data;
                           }
                         else if (new_rep->bytcnt == 3) {        /* 3rd byte */
                             new_rep->dy = data;
                             new_rep->bytcnt = 0;
                             (*vs_gdkint)(0400); /* 400 says line 1 */
                             }
                         }
                       }
                    }
		else { /* tablet report */
		    data = ch & 0xff;	/* get report byte */
		    ++new_rep->bytcnt;	/* inc report byte count */
		    
		    if (data & START_FRAME) { /* 1st byte of report? */
			new_rep->state = data;
			if (new_rep->bytcnt > 1)
			    new_rep->bytcnt = 1;  /* start new frame */
		      }
		    else /* Collect Body of Report */
                      {
                      if((new_rep->state & 0xff) == 0xa0) /* Reset Command */
                        {
                        if (new_rep->bytcnt == 2)
                               new_rep->dx = data;        /* pointer_id */

                        else if (new_rep->bytcnt == 3)    /* 3nd byte */
                               new_rep->dy = data;        /* button cnfg */

                        else if (new_rep->bytcnt == 4) {  /* 4th byte */
                               new_rep->bytcnt = 0;  /* Quit no pass event*/
                               (*vs_gdkint)(0400); } /* 400 says line 1 */
                        }
                      else
                        {
                        if (new_rep->bytcnt == 2)       /* 2nd byte */
                          new_rep->dx = data & 0x3f;

                        else if (new_rep->bytcnt == 3)  /* 3rd byte */
                          new_rep->dx |= (data & 0x3f) << 6;

                        else if (new_rep->bytcnt == 4)  /* 4th byte */
                          new_rep->dy = data & 0x3f;

                        else if (new_rep->bytcnt == 5){ /* 5th byte */
                          new_rep->dy |= (data & 0x3f) << 6;
                          new_rep->bytcnt = 0;
                          (*vs_gdkint)(0400); /* 400 says line 1 */
                             }
                        }
                     }
		}
	    }
	    SCC_READ(rsp, SCC_RR3, ip);         /* read IP bits again */
	} /* while */
    } /* if */ 
}

int scckint_arg=0;
int scc_temp_kint(arg)
int arg;
{
	register struct tty *tp;
	scckint_arg = arg;
	tp = &scc_tty[arg>>8];
	(*linesw[tp->t_line].l_rint)('\r', tp);
}


scc_spec_rint(line)
     register int line;
{
    scc_dma_rint(line);
}


scc_dma_rint(unit)
     register int unit;
{
    register struct scc_softc *sc = sccsc;
    register struct tty *tp = &scc_tty[unit];
    register u_short c;	
    register int ct;	/* NOTE: ttyinput() takes a LONG */
    register int status, vector, status0;
    register int num;
    register char *endptr;
    register char *ptr;
    register struct scc_reg *rsp;
    register char ip;
    int overrun = 0;
    int counter = 0;
    register int c_mask;
    vm_offset_t phys;

    IOC_CLR(IOC_SSR, scc_rdma_en[unit]);    /* disable rdma */
    IOC_WR(IOC_SIR, ~(scc_rint[unit]));       /* clear rdma int */

    /* compute the offset from the beginning of the page */
    /* read rdma ptr */
    endptr = (char *)(((sc->ioc_regs[unit]->RDMA_REG) >> 3) & 0xfff); 
    /* add the base of the receive buffer to the endptr */
    num = sc->rflag[unit];     /* save flag */
    /* pgt - check for boundary crossing */
    if (endptr == 0) 
	endptr += (long)(sc->rbuf[unit][num]) + SCC_PAGE_SIZE ;
    else 
        endptr += (long)(sc->rbuf[unit][num]);
    endptr -= SCC_WORD;        /* move to previous word */

    sc->rflag[unit] ^= 1;      /* toggle buffer flag */
    /*
     * set rdma ptr to other buffer 
     */
    svatophys(sc->rbuf[unit][sc->rflag[unit]] + SCC_HALF_PAGE - SCC_WORD,
	    &phys);
    sc->ioc_regs[unit]->RDMA_REG = (u_long)(phys << 3);

    
    /* unit = comm1 or comm2 */
    rsp = sc->sc_regs[scc_other[unit]];   /* channel A */
    SCC_READ(rsp, SCC_RR3, ip);		/* read IP from RR3A */
    if (ip & SCC_RR3_B_RIP) {             /* channel B receiver */
	rsp = sc->sc_regs[unit];   /* channel B */
	SCC_READ(rsp, SCC_RR1, status);	/* read status */    
	rsp->SCC_CMD = (SCC_WR0_ERROR_RESET)<<8;
    } else {
	status = 0;
    }
    
    IOC_SET(IOC_SSR, scc_rdma_en[unit]);     /* enable rdma */
    if ((tp->t_state & TS_ISOPEN) == 0) {
	wakeup((caddr_t)&tp->t_rawq);
	return;
    }
    
    ptr = sc->rbuf[unit][num] + SCC_HALF_PAGE - SCC_WORD;

    /* need to do following because SCC sets unused bits to ones */
    switch(tp->t_cflag&CSIZE) {
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
    while (ptr < endptr) {
	/* FIX ME FIX ME FIX ME !!! */
#ifdef __alpha
	c = ((*(u_int *)ptr)>>8)&c_mask;
#else
	c = ((*(u_short *)ptr)>>8)&c_mask;
#endif
	ptr += SCC_WORD;

	ct = c & TTY_CHARMASK;
	(*linesw[tp->t_line].l_rint)(ct, tp);
    }

    /* FIX ME FIX ME FIX ME !!! */
#ifdef __alpha
    c = ((*(u_int *)ptr)>>8)&c_mask;
#else
    c = ((*(u_short *)ptr)>>8)&c_mask;
#endif
    ct = c & TTY_CHARMASK;

    if (status) {
	/* SCC_FE is interpreted as a break */
	if (status & SCC_RR1_FE)
		ct |= TTY_FE;

	/* Parity Error */
	if (status & SCC_RR1_PE)
		ct |= TTY_PE;

	/* SVID does not say what to do with overrun errors */
	if (status & SCC_RR1_DO) {
		if(overrun == 0) {
			printf("scc%d: input silo overflow\n", 0);
			overrun = 1;
		}
		sc->sc_softcnt[unit]++;
	}
    }

    (*linesw[tp->t_line].l_rint)(ct, tp);

}

static char cnbrkstr[] = "cnbrk";

/*ARGSUSED*/
sccioctl(dev, cmd, data, flag)
     dev_t dev;
     register unsigned int cmd;
     caddr_t data;
     int flag;
{
    register struct scc_softc *sc = sccsc;
    register int unit;
    register struct tty *tp;
    register int s;
    struct uba_device *ui;
    struct devget *devget;
    int error;
    register struct scc_reg *rsp;
    register int status;
    register int timo;

    unit = minor(dev);
    if((consDev != GRAPHIC_DEV) && (major(dev) == CONSOLEMAJOR) && ((unit&LINEMASK) == SCC_KYBD))
	unit |= console_line;	/* diag console on SLU line 3 */
    
    /*
     * If there is a graphics device and the ioctl call
     * is for it, pass the call to the graphics driver.
     */
    if (vs_gdioctl && (unit <= SCC_MOUSE)) {
	return((*vs_gdioctl)(dev, cmd, data, flag));
    }
    tp = &scc_tty[unit];
    error = (*linesw[tp->t_line].l_ioctl)(tp, cmd, data, flag);
    if (error >= 0)
	return (error);
    error = ttioctl(tp, cmd, data, flag);
    if (error >= 0) 
	return (error);
    
    switch (cmd) {

      case TCSBREAK:
     	s = spltty();
	rsp = sc->sc_regs[unit];
	SCC_READ(rsp, SCC_RR1, status); /* read status */
	for(timo=10000; timo > 0; --timo) {
	    if (status & SCC_RR1_ALL_SENT)
		break;
	    else {
		SCC_READ(rsp, SCC_RR1, status);
	    }
	}
	SCC_SET_BRK(unit);
	mpsleep((caddr_t)&sc->sc_flags[unit], PZERO-10, cnbrkstr,  hz/4,
		NULL, 0);
	SCC_CLR_BRK(unit);
	splx(s);
	break;

      case TIOCSBRK:	/* ULTRIX compatible ioctl */
	s = spltty();
	rsp = sc->sc_regs[unit];
	SCC_READ(rsp, SCC_RR1, status);	/* read status */
	for(timo=10000; timo > 0; --timo) {
	    if (status & SCC_RR1_ALL_SENT) 
		break;	
	    else {
		SCC_READ(rsp, SCC_RR1, status);
	    }
	}

	SCC_SET_BRK(unit);
	splx(s);
	break;
	
      case TIOCCBRK:	/* ULTRIX compatible ioctl */
	s = spltty();
	SCC_CLR_BRK(unit);
	splx(s);
	break;
      case TIOCSDTR:
	(void) sccmctl(dev, DC_DTR | DC_RTS, DMBIS);
	break;
	
      case TIOCCDTR:
	(void) sccmctl(dev, DC_DTR | DC_RTS, DMBIC);
	break;
	
      case TIOCMSET:
	(void) sccmctl(dev, dmtoscc(*(int *)data), DMSET);
	break;
	
      case TIOCMBIS:
	(void) sccmctl(dev, dmtoscc(*(int *)data), DMBIS);
	break;
	
      case TIOCMBIC:
	(void) sccmctl(dev, dmtoscc(*(int *)data), DMBIC);
	break;
	
      case TIOCMGET:
	*(int *)data = scctodm(sccmctl(dev, 0, DMGET));
	break;
	
      case TIOCNMODEM:  /* ignore modem status */
	/*
	 * By setting the software representation of modem signals
	 * to "on" we fake the system into thinking that this is an
	 * established modem connection.
	 */
	s = spltty();
	sccsoftCAR |= (1<<(unit&LINEMASK));
	if (*(int *)data) /* make mode permanent */
	    sccdefaultCAR |= (1<<(unit&LINEMASK));
	tp->t_state |= (TS_CARR_ON|TS_ISOPEN);
	tp->t_cflag |= CLOCAL;		/* Map to termio */
	SCC_MODEM_OFF(unit);
	splx(s);
	break;
	
      case TIOCMODEM:  
	s = spltty();
	SCC_MODEM_ON(unit);
	sccsoftCAR &= ~(1<<(unit&LINEMASK));
	if (*(int *)data) /* make mode permanent */
	    sccdefaultCAR &= ~(1<<(unit&LINEMASK));
	/*
	 * See if signals necessary for modem connection are present
	 */
	if (SCC_XMIT(unit)) {
	    tp->t_state &= ~(TS_ONDELAY);
	    tp->t_state |= TS_CARR_ON;
	    sccspeedi(unit);  /* check speed indicate */
	    sccmodem[unit] = MODEM_CTS|MODEM_CD|MODEM_DSR;
	} else {
	    tp->t_state &= ~(TS_CARR_ON|TS_ISOPEN);
	    sccmodem[unit] &= ~(MODEM_CTS|MODEM_CD|MODEM_DSR);
	}
	tp->t_cflag &= ~CLOCAL;		/* Map to termio */
	splx(s);
	break; 
	
#if 0
/* TIOCWONLINE is not supported under OSF/1 - see ioctl_compat.h */
      case TIOCWONLINE: /* look at modem status - sleep if no carrier */
	s = spltty();
	/*
	 * See if signals necessary for modem connection are present
	 */
	if (SCC_XMIT(unit)) {
	    tp->t_state |= TS_CARR_ON;
	    sccspeedi(unit);
	    tp->t_state &= ~(TS_ONDELAY);
	    sccmodem[unit] = MODEM_CTS|MODEM_CD|MODEM_DSR;
	} else {
	    while ((tp->t_state & TS_CARR_ON) == 0)
		if (error = tsleep((caddr_t)&tp->t_rawq,
			TTIPRI | PCATCH, ttyin, 0)) {
		        splx(s);
			return (error);
		}
	}
	splx(s);
	break;
#endif /* 0 */

      case DEVIOCGET:				/* device status */
	devget = (struct devget *)data;
	bzero(devget,sizeof(struct devget));
	if (tp->t_cflag & CLOCAL) {
	    sc->sc_category_flags[unit&LINEMASK] |= DEV_MODEM;
	    sc->sc_category_flags[unit&LINEMASK] &= ~DEV_MODEM_ON;
	} else
	    sc->sc_category_flags[unit&LINEMASK] |= (DEV_MODEM|DEV_MODEM_ON);
	devget->category = DEV_TERMINAL;	/* terminal cat.*/
	devget->bus = DEV_NB;			/* NO bus	*/
	bcopy(DEV_VS_SLU,devget->interface,
	      strlen(DEV_VS_SLU));		/* interface	*/
	bcopy(DEV_UNKNOWN,devget->device,
	      strlen(DEV_UNKNOWN));		/* terminal	*/
	devget->adpt_num = 0;			/* NO adapter	*/
	devget->nexus_num = 0;			/* fake nexus 0 */
	devget->bus_num = 0;			/* NO bus	*/
	devget->ctlr_num = 0;			/* cntlr number */
	devget->slave_num = unit&LINEMASK;	/* line number	*/
	bcopy("scc", devget->dev_name, 3);	/* Ultrix "scc"	*/
	devget->unit_num = unit&LINEMASK;	/* scc line?	*/
	devget->soft_count =
	    sc->sc_softcnt[unit&LINEMASK];	/* soft err cnt */
	devget->hard_count =
	    sc->sc_hardcnt[unit&LINEMASK];	/* hard err cnt */
	devget->stat = sc->sc_flags[unit&LINEMASK]; /* status	*/
	devget->category_stat =
	    sc->sc_category_flags[unit&LINEMASK]; /* cat. stat. */
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

sccmmap(dev, off, prot)
     dev_t dev;
     off_t off;
     int prot;
{
    int unit;
    extern int noconsmmap();

    unit = minor(dev);
    /*
     * If there is no graphics console or we are talking about a unit
     * other than units 0 or 1, then tell the caller that we don't support
     * mapping of that address.
     */
    if ((consDev != GRAPHIC_DEV) || (unit > 1)) {
        return (noconsmmap(dev, off, prot));
    }

    /*
     * If there is a graphics device pass the call to the graphics driver.
     */
    if (vs_gdmmap && (unit <= 1)) {
        return((*vs_gdmmap)(dev, off, prot));
    }
}

dmtoscc(bits)
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
#ifdef _ANSI_C_SOURCE
scctodm(register int bits)
#else
scctodm(bits)
     register int bits;
#endif
{
    register int b;
    
    b = (bits << 1) & 0360;
    if (bits & DC_DSR) b |= SML_DSR;
    if (bits & DC_DTR) b |= SML_DTR;
    if (bits & DC_ST) b |= SML_ST;
    if (bits & DC_RTS) b |= SML_RTS;
    return(b);
}

#ifdef _ANSI_C_SOURCE
sccparam(register struct tty *tp, struct termios *t)
#else
sccparam(tp, t)
register struct tty *tp;
register struct termios *t;
#endif
{
    register struct scc_softc *sc = sccsc;
    register struct scc_reg *rsp, *rsp0;
    register struct scc_saved_reg *ssp, *ssp0;
    register int s, status;
    register int timo;
    int unit = minor(tp->t_dev);
    unsigned char lospeed = ttspeedtab(t->c_ospeed, ss_lo_speeds);    
    unsigned char hispeed = ttspeedtab(t->c_ospeed, ss_hi_speeds);    
 
    if ((consDev != GRAPHIC_DEV) && (major(tp->t_dev) == CONSOLEMAJOR) 
			&& (unit == SCC_KYBD)) 
	unit = console_line;

    s = spltty();
    if (tp->t_state & TS_BUSY) {
        tp->t_state |= TS_NEED_PARAM;
	splx(s);
	return(0);
    }

    rsp = sc->sc_regs[unit];
    SCC_READ(rsp, SCC_RR1, status);	/* read status */
    for(timo=10000; timo > 0; --timo) {
        if (status & SCC_RR1_ALL_SENT) 
	    break;	
	else {
	    SCC_READ(rsp, SCC_RR1, status);
	}
    }

    /* check requested parameters */

    if ((tp->t_state & TS_ISOPEN) && !(tp->t_cflag & CLOCAL) &&
						(t->c_ospeed == B0)) {
	SCC_CLR_DTR(unit);	/* hang up line */
	SCC_CLR_RTS(unit);
	SCC_CLR_SS(unit);
	splx(s);
	return(0);
    }

    ssp = sc->sc_saved_regs[unit];  /* mjm - sync support */

    /* If input baud rate is set to zero, the input baud rate will
       be specified by the value of the output baud rate */

    if (t->c_ospeed < 0 || (t->c_ispeed ? (t->c_ispeed != t->c_ospeed)
		    : !(t->c_ispeed = t->c_ospeed))) {
	splx(s);
	return EINVAL;
    }
    status = ttspeedtab(t->c_ospeed, scc_speeds);
    if (status == BAUD_UNSUPPORTED || status == -1) {
	splx(s);
	return (EINVAL);
    }

#ifdef mips
    if((cpu == DS_5000_100) && ( t->c_ospeed == B38400))
        return (EINVAL);
#endif

    /* copy termios to tty */
    tp->t_ispeed = t->c_ispeed;
    tp->t_ospeed = t->c_ospeed;
    tp->t_cflag = t->c_cflag;

    /*
     * If diagnostic console on serial line,
     * line parameters must be: 9600 BPS, 8 BIT, NO PARITY, 1 STOP.
     */
    if ((unit == console_line) && (consDev != GRAPHIC_DEV)) {
	/* 
         * do nothing here because console_line parameters have already 
	 * been set in scc_cons_init.
	 */
	;
    } else if (unit == SCC_COMM1 || unit == SCC_COMM2) {
        /*
	 * Set parameters in accordance with user specification.
	 */
	ssp->wr4 = SCC_WR4_CLOCK16;
	/*
	 * Berkeley-only dinosaur
	if (tp->t_line != TERMIODISC) {
	    if ((tp->t_cflag_ext&CBAUD) == B110)
		tp->t_cflag |= CSTOPB;
	}
	 */
	/*
	 * Set device registers according to the specifications of the
	 * termio structure.
	 */
	
	if (tp->t_cflag & CSTOPB) 
	    ssp->wr4 |= SCC_WR4_TWOSB;
	else
	    ssp->wr4 |= SCC_WR4_ONESB;
	if (tp->t_cflag & PARENB) {
	    if ((tp->t_cflag & PARODD) == 0) 
		/* set even */
		ssp->wr4 |= (SCC_WR4_EPAR | SCC_WR4_PENABLE);
	    else
		/* else set odd */
		ssp->wr4 |= SCC_WR4_PENABLE;
	}
	SCC_WRITE(rsp, SCC_WR4, ssp->wr4);
	/*
	 * character size.
	 * clear bits and check for 5, 6, 7 & 8 bits.
	 */
	ssp->wr3 &= ~(SCC_WR3_RBITS|SCC_WR3_RXEN);
	ssp->wr5 &= ~(SCC_WR5_TBITS|SCC_WR5_TXEN);
	switch(tp->t_cflag&CSIZE) {
	  case CS5:
	    ssp->wr3 |= SCC_WR3_RBITS5;
	    ssp->wr5 |= SCC_WR5_TBITS5;
	    break;
	  case CS6:
	    ssp->wr3 |= SCC_WR3_RBITS6;
	    ssp->wr5 |= SCC_WR5_TBITS6;
	    break;
	  case CS7:
	    ssp->wr3 |= SCC_WR3_RBITS7;
	    ssp->wr5 |= SCC_WR5_TBITS7;
	    break;
	  case CS8:
	    ssp->wr3 |= SCC_WR3_RBITS8;
	    ssp->wr5 |= SCC_WR5_TBITS8;
	    break;
	}
	SCC_WRITE(rsp, SCC_WR3, ssp->wr3);
	SCC_WRITE(rsp, SCC_WR5, ssp->wr5);
	ssp->wr14 &= ~(SCC_WR14_BRGEN_EN);
	SCC_WRITE(rsp, SCC_WR14, ssp->wr14);	     /*	 WR14 BRG disable */
	/*
	SCC_WRITE(rsp, SCC_WR12, scc_speeds[tp->t_cflag&CBAUD].baud_lo);
	SCC_WRITE(rsp, SCC_WR13, scc_speeds[tp->t_cflag&CBAUD].baud_hi);
	*/
	SCC_WRITE(rsp, SCC_WR12, lospeed);
	SCC_WRITE(rsp, SCC_WR13, hispeed);
	/*
	scc_cbaud[unit] = tp->t_cflag&CBAUD;
	*/
	scc_cbaud[unit] = tp->t_ospeed;
	if ((tp->t_cflag & CLOCAL) == 0)
	    sccspeedi(unit);  /* check speed indicate */

	/* 
	 * enable functions 
	 */
	ssp->wr14 |= SCC_WR14_BRGEN_EN;
	SCC_WRITE(rsp, SCC_WR14, ssp->wr14);	     /*	 WR14 BRG enable */
	if (tp->t_cflag & CREAD){ 
	    ssp->wr3 |= SCC_WR3_RXEN;
	    SCC_WRITE(rsp, SCC_WR3, ssp->wr3);	      /*  WR3 Rx enable */
	}
	ssp->wr5 |= SCC_WR5_TXEN;
	SCC_WRITE(rsp, SCC_WR5, ssp->wr5);        /*  WR5 Tx enable */
    }
    splx(s);

    return(0);
}


/*
 * scc_dma_xint(unit) - transmit DMA interrupt service routine
 *	
 * algorithm:
 *	-clear transmit DMA interrupt
 *  	-disable transmit DMA for comm. port designated by 'unit'
 * 	-same as dcxint with pdma stuff removed and new ndflush:
 *		-get tty for 'unit'
 *		-reset 'unit' to 3 if line 0 is console
 *		-set t_state 
 *		-flush 'cc' bytes from output queue
 *              -if we need param, call sccparam
 *		-invoke start routine
 *
 */
scc_dma_xint(unit)
     register int unit; 	
{
    register struct scc_softc *sc = sccsc;
    register struct tty *tp;
    register char *ptr;
    register struct scc_reg *rsp;
    register int cc;

    /* must disable the transmit DMA before clearing the interrupt */
    IOC_CLR(IOC_SSR, scc_xdma_en[unit]);    /* disable transmit DMA */
    IOC_WR(IOC_SIR, ~(scc_xint[unit]));       /* clear transmit int */
    /* read transmit DMA offset */
    ptr = (char *)(((sc->ioc_regs[unit]->XDMA_REG) >> 3) & 0xfff);
    if (ptr == 0) 
	ptr += (u_long)sc->tbuf[unit] + SCC_PAGE_SIZE;
    else 
	ptr += (u_long)sc->tbuf[unit];
    
    tp = &scc_tty[unit];
    if ((consDev != GRAPHIC_DEV) && (unit == SCC_KYBD) && /* ? */
	(major(tp->t_dev) == CONSOLEMAJOR)) {
	unit = console_line;
    }
    
    tp->t_state &= ~TS_BUSY;

#ifdef __alpha
    cc = (u_int *)ptr - (u_int *)sc->tptr[unit]; /* cc bytes */
#else
    cc = (u_short *)ptr - (u_short *)sc->tptr[unit]; /* cc bytes */
#endif
    ndflush(&tp->t_outq, cc); /* cc bytes */

    if (tp->t_state & TS_NEED_PARAM) {
        tp->t_state &= ~TS_NEED_PARAM;
	sccparam(tp, &tp->t_termios);
    }

    (*linesw[tp->t_line].l_start)(tp);
}


sccstart(tp)
     register struct tty *tp;
{	
    register struct scc_softc *sc = sccsc;
    register int cc;
    int s, unit;
    register char *bp;
    register char *cp;
    register int c_mask;
    vm_offset_t phys;
    
    s = spltty();

    unit = minor(tp->t_dev) & 3;
    if ((consDev != GRAPHIC_DEV) && (unit == SCC_KYBD) && 
		(major(tp->t_dev) == CONSOLEMAJOR))
	unit = console_line;

    /*
     * Do not do anything if currently delaying, or active.  Also only
     * transmit when CTS is up.
     */
    if ((tp->t_state & (TS_TIMEOUT|TS_BUSY|TS_TTSTOP)) || 
	( ((tp->t_cflag & CLOCAL) == 0) &&
         (tp->t_state&TS_CARR_ON) && ((sccmodem[unit]&MODEM_CTS)==0)) )
	goto out;

    if (tp->t_outq.c_cc <= tp->t_lowat) {
	if (tp->t_state&TS_ASLEEP) {
	    tp->t_state &= ~TS_ASLEEP;
	    wakeup((caddr_t)&tp->t_outq);
	}
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

    /*
    if ((tp->t_lflag_ext & PRAW) || (tp->t_oflag_ext & PLITOUT) ||
    */
    if ( 
	((tp->t_oflag & OPOST) == 0)) {
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
    
    /*
     * prepare for DMA:
     *	-point cp to first char in clist block,
     * 	-point bp to cc words from end of page,
     *	-save bp in scc_softc for use by ndflush.
     *	-using cp and bp, copy cc bytes from clist to
     *		word-aligned DMA page.
     */
    cp = tp->t_outq.c_cf;
    bp = sc->tbuf[unit] + SCC_PAGE_SIZE - (cc * SCC_WORD);
    sc->tptr[unit] = bp;
    /*
     * need to do the following because when character size is set to five,
     * the data format allows character sizes of one to five. See SCC
     * manual.
     */
    if ((tp->t_cflag&CSIZE) == CS5)
       c_mask = 0x1f;
    else
       c_mask = 0xff;
    while (cc-- > 0) {
#ifdef __alpha
	*(u_int *)bp = (((u_int)*cp++)&c_mask)<<8;
#else
	*(u_short *)bp = (((u_short)*cp++)&c_mask)<<8;
#endif
	bp += SCC_WORD;
    }
    /*
     * set DMA transmit ptr
     */
    svatophys(sc->tptr[unit], &phys);
    sc->ioc_regs[unit]->XDMA_REG = (u_long)(phys << 3);

    IOC_SET(IOC_SSR, scc_xdma_en[unit]);     /* enable transmit DMA */
  out:	

    splx(s);
}

sccstop(tp, flag)
     register struct tty *tp;
{
    register struct scc_softc *sc = sccsc;
    register int s, cc;
    register char *ptr;
    int	unit;
    
    /*
     * If there is a graphics device and the stop call
     * is for it, pass the call to the graphics device driver.
     */
    unit = minor(tp->t_dev);
    if ((consDev != GRAPHIC_DEV) && (unit == SCC_KYBD) && /* ? */
	(major(tp->t_dev) == CONSOLEMAJOR)) {
	unit = console_line;
    }

    if (vs_gdstop && (unit <= SCC_MOUSE)) {
	(*vs_gdstop)(tp, flag);
	return;
    }
    
    s = spltty();
    if (tp->t_state & TS_BUSY) {
	/* disable transmit DMA */
	IOC_CLR(IOC_SSR, scc_xdma_en[unit]); 
	/* 
	 * pgt - need to clear transmit int to handle boundary condition, 
	 * otherwise scc_dma_xint could be called and queue would be 
	 * ndflushed twice
	 */
	IOC_WR(IOC_SIR, ~(scc_xint[unit]));       /* clear transmit int */
	/* line discipline will flush entire queue */
	if ((tp->t_state&TS_TTSTOP)==0) {
	    ;
	} else { /* suspend */
	    /* read transmit DMA ptr */
	    ptr = (char *)(((sc->ioc_regs[unit]->XDMA_REG) >> 3) & 0xfff);
	    /* if we made it to a page boundary pointer will be zero */
	    if (ptr == 0)
		ptr += (long)(sc->tbuf[unit]) + SCC_PAGE_SIZE ;
	    else 
		ptr += (long)(sc->tbuf[unit]);
	    cc = (u_int *)ptr - (u_int *)sc->tptr[unit]; /* cc bytes */
	    ndflush(&tp->t_outq, cc); /* cc bytes */
	}
	tp->t_state &= ~TS_BUSY;
    }
    splx(s);
}


sccmctl(dev, bits, how)
     dev_t dev;
     int bits, how;
{
    register struct scc_softc *sc = sccsc;
    register int unit, mbits;
    int b, s;
    
    unit = minor(dev);
    if ((unit != SCC_COMM1) && (unit != SCC_COMM2))
	return(0);	/* only line 2 and 3 has modem control */
    s = spltty();
    mbits = (SCC_DTR(unit)) ? DC_DTR : 0;
    mbits |= (SCC_RTS(unit)) ? DC_RTS : 0;
    mbits |= (SCC_DCD(unit)) ? DC_CD : 0;
    mbits |= (SCC_DSR(unit)) ? DC_DSR : 0;
    mbits |= (SCC_CTS(unit)) ? DC_CTS : 0;
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
    if (mbits & DC_DTR) {
	SCC_SET_DTR(unit);
	SCC_SET_RTS(unit);
	SCC_SET_SS(unit);
    } else {
	SCC_CLR_DTR(unit);
	SCC_CLR_RTS(unit);
	SCC_CLR_SS(unit);
    }
    (void) splx(s);
    return(mbits);
}


/*
 *  mjm - sync support.
 *  Parameter is 'line' to support new interrupt dispatcher.
 */
scc_ext_rint(line)
     register line;
{
    register struct scc_softc *sc = sccsc;
    register struct tty *tp;
    register struct scc_reg *rsp;
    register int brk;
    register int i;

    /* mjm - sync support */
    i = (line < 2 ? scc_other[line] : line);  /* i = SCC_COMM1 or SCC_COMM2 */

    tp = &scc_tty[i];
    /*
     * assume break condition will last long enough for it to be processed by
     * this routine. otherwise, null characters would be DMA'd into the receive
     * buffer.
     */
    if (i == SCC_COMM1 || i == SCC_COMM2) {
        rsp = sc->sc_regs[i];
	brk = ((rsp->SCC_CMD)>>8) & SCC_RR0_BREAK;
	/* check if there is a state change in BREAK */
	if (brk && !scc_brk[i])  {
	    scc_brk[i] = 1;	
	    sccbrkint(i);
	} else if (!brk && scc_brk[i]) {
	    scc_brk[i] = 0;
	    sccbrkint(i);
	} else if (brk && scc_brk[i]) {
	    /*
	     * handle the case where successive breaks arrive
	     * need to call brkint twice to handle the termination of an
	     * earlier break and the beginning of the next break
	     */
	    scc_brk[i] = 0;
	    sccbrkint(i);
	    scc_brk[i] = 1;
	    sccbrkint(i);
	}
    } 

    if ((tp->t_cflag & CLOCAL) == 0) {
	sccspeedi(i);	/* check for speed indicate changes */
	/*
	 * Drop DTR immediately if DSR has gone away.
	 * If really an active close then do not
	 *    send signals.
	 */
	if (!(SCC_DSR(i))) {
	    if (tp->t_state&TS_CLOSING) {
		untimeout(wakeup, (caddr_t) &tp->t_dev);
		wakeup((caddr_t) &tp->t_dev);
	    }
	    if (tp->t_state&TS_CARR_ON) {
		scc_tty_drop(tp);
	    }
	} else {		/* DSR has come up */
	    /*
	     * If DSR comes up for the first time we allow
	     * 30 seconds for a live connection.
	     */
	    if ((sccmodem[i] & MODEM_DSR)==0) {
		sccmodem[i] |= (MODEM_DSR_START|MODEM_DSR);
		/*
		 * we should not look for CTS|CD for about
		 * 500 ms.
		 */
		timeout(scc_dsr_check, tp, hz*30);
		scc_dsr_check(tp);}

	}
	
	/*
	 * look for modem transitions in an already
	 * established connection.
	 */
	if (tp->t_state & TS_CARR_ON) {
	    if (SCC_DCD(i)) {
		/*
		 * CD has come up again.
		 * Stop timeout from occurring if set.
		 * If interval is more than 2 secs then
		 * drop DTR.
		 */
		if ((sccmodem[i] & MODEM_CD) == 0) {
		    untimeout(scc_cd_drop, tp);
		    if (scc_cd_down(tp)) {
			/* drop connection */
			scc_tty_drop(tp);
		    }
		    sccmodem[i] |= MODEM_CD;
		}
	    } else {
		/*
		 * Carrier must be down for greater than
		 * 2 secs before closing down the line.
		 */
		if (sccmodem[i] & MODEM_CD) {
		    /* only start timer once */
		    sccmodem[i] &= ~MODEM_CD;
		    /*
		     * Record present time so that if carrier
		     * comes up after 2 secs, the line will drop.
		     */
		    scctimestamp[i] = time;
		    timeout(scc_cd_drop, tp, hz * 2);
		}
	    }
	    
	    /* CTS flow control check */
	    
	    if (!(SCC_CTS(i))) {
		/*
		 * Only allow transmission when CTS is set.
		 */
		tp->t_state |= TS_TTSTOP;
		sccmodem[i] &= ~MODEM_CTS;
		sccstop(tp, 0);
	    } else if (!(sccmodem[i] & MODEM_CTS)) {
		/*
		 * Restart transmission upon return of CTS.
		 */
		tp->t_state &= ~TS_TTSTOP;
		sccmodem[i] |= MODEM_CTS;
		sccstart(tp);
	    }
	}
	
	/*
	 * See if a modem transition has occured.  If we are waiting
	 * for this signal, cause action to be take via
	 * scc_start_tty.
	 */
	if ((SCC_XMIT(i)) &&
	    (!(sccmodem[i] & MODEM_DSR_START)) &&
	    (!(tp->t_state & TS_CARR_ON))) {
	    scc_start_tty(tp);
	}
    }
    /* mjm - sync support */
    sc->sc_regs[line]->SCC_CMD = SCC_WR0_RESET_EXT_INT << 8;
#ifdef __alpha
    /* Alpha seems to need the MB and the second reset */
    mb();
    sc->sc_regs[line]->SCC_CMD = SCC_WR0_RESET_EXT_INT << 8;
#endif
}

/*
 * Note: When a break condition occurs, the SCC chip detects the condition,
 *       sets the BREAK bit and generates an ext/status interrupt. Upon
 *       termination of the break, the receive FIFO will contain a single 
 *       NULL character. This NULL character will cause a dma rint. The
 *       Framing Error bit will not be set for this character, but if odd
 *       parity has been selected, the parity error bit will be set. 
 */ 
sccbrkint(unit)
     register int unit;
{
    register struct scc_softc *sc = sccsc;
    register struct tty *tp;
    register struct scc_reg *rsp;
    register struct scc_saved_reg *ssp;
    register u_short c;	
    register int ct;
    register int flg;
    
    tp = &scc_tty[unit];
    rsp = sc->sc_regs[unit];
    ssp = sc->sc_saved_regs[unit];  /* mjm - sync support */
    c = (unit<<8)| 0x0000;
    ct = c & TTY_CHARMASK;			/* OSF character info. */
    ct |= TTY_FE;				/* framing error flag */
    /*
     * We need to disable both the SCC rint and the IOASIC rdma int
     * because we just want to read and discard the NULL character
     * that will be deposited in the receive FIFO upon termination
     * of the break.
     */
    if (scc_brk[unit]) {
	ssp->wr1 &= ~SCC_WR1_RINT;   /* disable rint on special conditions */
	SCC_WRITE(rsp, SCC_WR1, ssp->wr1);
	IOC_CLR(IOC_SSR, scc_rdma_en[unit]);    /* disable rdma */
	
	if ((tp->t_state & TS_ISOPEN) == 0) {   /* process break */
	    wakeup((caddr_t)&tp->t_rawq);
	    return;
	}

	(*linesw[tp->t_line].l_rint)(ct, tp);

    } else {
	c = ((rsp->SCC_DATA)>>8)&0xff;	/* read NULL character */
	IOC_SET(IOC_SSR, scc_rdma_en[unit]);  /* enable RDMA */
	ssp->wr1 &= ~SCC_WR1_RINT;		/* clear rint bits */
	ssp->wr1 |= SCC_WR1_RINT_SPC;		/* enable rint */
	SCC_WRITE(rsp, SCC_WR1, ssp->wr1);
    }
}

int  sccputc();
int  sccgetc();

sccputc(c)
     register int c;
{
    if (consDev == GRAPHIC_DEV) {
	if ( v_consputc ) {
	    (*v_consputc) (c);
	    if ( c == '\n' )
		(*v_consputc)( '\r' );
	    return;
	}
    }
    scc_putc(console_line, c);
    if ( c == '\n') {
	scc_putc(console_line, '\r');
    }
}

/*
 * polled-mode DMA: need to do this because SCC can not be touched in
 * scc_putc.
 */
scc_putc(unit, c)
     int unit;
     register int c;
{
    register struct scc_softc *sc = sccsc;
    register int s;
    register struct scc_reg *rsp;
    char *ptr;
    int intr_pending = 0, save_word;
    u_int sir, save_SSR, save_xmt_ptr;
    vm_offset_t phys;
    int timeout_ctr = 0;

    if (unit == SCC_COMM1 || unit == SCC_COMM2) {
	/* set pointer to be the word before the end of the page */
	ptr = sc->tbuf[unit] + SCC_PAGE_SIZE - SCC_WORD;

	if ((s = getspl()) < SPLTTY)
		s = spltty();
	IOC_RD(IOC_SSR, save_SSR);	/* save copy of SSR */
	IOC_WR(IOC_SSR, (save_SSR & ~(scc_xdma_en[unit]))); /* disable XMT DMA */
	IOC_RD(IOC_SIR, sir);
	if (sir & scc_xint[unit]) {
	    intr_pending = 1;
	    IOC_WR(IOC_SIR, ~(scc_xint[unit]));
	} else {
	    save_xmt_ptr = sc->ioc_regs[unit]->XDMA_REG;
	    save_word = *(int *)ptr;
	}
#ifdef __alpha
	*(u_int *)ptr = ((u_int)c) << 8;
#else
	*(u_short *)ptr = ((u_short)c) << 8;
#endif
	svatophys(ptr, &phys);
	sc->ioc_regs[unit]->XDMA_REG = (u_long)(phys << 3);

	IOC_SET(IOC_SSR, scc_xdma_en[unit]);
	IOC_RD(IOC_SIR, sir);
	while (((sir & scc_xint[unit]) == 0) && (timeout_ctr++ < 100000))
	    IOC_RD(IOC_SIR, sir);
	
	if (intr_pending == 0) {
	    IOC_CLR(IOC_SSR, scc_xdma_en[unit]);
	    IOC_WR(IOC_SIR, ~(scc_xint[unit]));
	    sc->ioc_regs[unit]->XDMA_REG = save_xmt_ptr;
	    *(int *)ptr = save_word;
	    IOC_WR(IOC_SSR, save_SSR);
	}
	splx(s);
    } else {
	if ((s = getspl()) < SPLTTY)
		s = spltty();
	rsp = sc->sc_regs[unit];
	while ((((rsp->SCC_CMD)>>8) & SCC_RR0_TBUF_EMPTY) == 0)
	    ;
	rsp->SCC_DATA = (c&0xff)<<8;        /* output char */  
#ifdef __alpha
	mb();
#endif
	while ((((rsp->SCC_CMD)>>8) & SCC_RR0_TBUF_EMPTY) == 0)
	    ;
	splx(s);
    }
}

/* pgt - new sccgetc() */
sccgetc()
{
    register int c;
    register int line;
    
    /*
     * Line number we expect input from. 
     */
    if (consDev == GRAPHIC_DEV)
	line = 0x0;
    else
	line = console_line;

    c = scc_getc(line);

    if (v_consgetc)
	return ((*v_consgetc)(c));
    else
	return (c);
}

scc_getc(unit)
     int unit;
{
    register struct scc_softc *sc = sccsc;
    register u_char c, status;
    register int timo;
    register struct scc_reg *rsp;

    rsp = sc->sc_regs[unit];
    SCC_WRITE(rsp, SCC_WR9, 0x00); /* disable MIE ? */
    if (unit == SCC_COMM1 || unit == SCC_COMM2)
	IOC_CLR(IOC_SSR, scc_rdma_en[unit]);    /* disable rdma ? */

    for(timo=1000000; timo > 0; --timo) {
	if (((rsp->SCC_CMD)>>8) & SCC_RR0_RCHAR_AVAIL) {
	    SCC_READ(rsp, SCC_RR1, status);
	    c = ((rsp->SCC_DATA)>>8)&0xff;      /* read data */
	    if (status & (SCC_RR1_PE | SCC_RR1_DO | SCC_RR1_FE)) 
	        continue;
	    break;
	}
    }
    DELAY(50000);
    SCC_WRITE(rsp, SCC_WR9, SCC_WR9_MIE); /* enable MIE ? */
    if (unit == SCC_COMM1 || unit == SCC_COMM2)
	IOC_SET(IOC_SSR, scc_rdma_en[unit]);    /* enable rdma ? */
    if (timo == 0)
	return(-1);
    else
	return(c & 0xff);
}


scc_mouse_init()
{
    register struct scc_softc *sc = sccsc;
    register struct scc_reg *rsp;
    register struct scc_saved_reg *ssp;
    register int unit = SCC_MOUSE;

    rsp = sc->sc_regs[unit];
    ssp = sc->sc_saved_regs[unit];  /* mjm - sync support */
    scc_init(unit);
    /* 
     * enable functions ?
     */
    ssp->wr14 |= SCC_WR14_BRGEN_EN;
    SCC_WRITE(rsp, SCC_WR14, ssp->wr14);	     /*	 WR14 BRG enable */
    ssp->wr3 |= SCC_WR3_RXEN;
    SCC_WRITE(rsp, SCC_WR3, ssp->wr3);	      /*  WR3 Rx enable */
    ssp->wr5 |= SCC_WR5_TXEN;
    SCC_WRITE(rsp, SCC_WR5, ssp->wr5);        /*  WR5 Tx enable */
}

scc_mouse_putc(c)
int c;
{
    scc_putc(SCC_MOUSE, c);
}

scc_mouse_getc()
{
    return (scc_getc(SCC_MOUSE));
}

scc_kbd_init()
{
    register struct scc_softc *sc = sccsc;
    register struct scc_reg *rsp;
    register struct scc_saved_reg *ssp;
    register int unit = SCC_KYBD;

    rsp = sc->sc_regs[unit];
    ssp = sc->sc_saved_regs[unit];  /* mjm - sync support */
    scc_init(unit);
    /* 
     * enable functions ?
     */
    ssp->wr14 |= SCC_WR14_BRGEN_EN;
    SCC_WRITE(rsp, SCC_WR14, ssp->wr14);	     /*	 WR14 BRG enable */
    ssp->wr3 |= SCC_WR3_RXEN;
    SCC_WRITE(rsp, SCC_WR3, ssp->wr3);	      /*  WR3 Rx enable */
    ssp->wr5 |= SCC_WR5_TXEN;
    SCC_WRITE(rsp, SCC_WR5, ssp->wr5);        /*  WR5 Tx enable */
}

scc_kbd_putc(c)
int c;
{
    scc_putc(SCC_KYBD, c);
}

scc_kbd_getc()
{
    return (scc_getc(SCC_KYBD));
}


/*
 * Modem Control Routines
 */

/*
 *
 * Function:
 *
 *	scc_cd_drop
 *
 * Functional description:
 *
 * 	Determine if carrier has dropped.  If so call scc_tty_drop to terminate
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
scc_cd_drop(tp)
     register struct tty *tp;
{
    register struct scc_softc *sc = sccsc;
    register int unit;
    
    unit = minor(tp->t_dev);
    if ((tp->t_state & TS_CARR_ON) && (!(SCC_DCD(unit)))) {
	scc_tty_drop(tp);
	return;
    }
    sccmodem[unit] |= MODEM_CD;
}

/*
 *
 * Function:
 *
 *	scc_dsr_check
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
scc_dsr_check(tp)
     register struct tty *tp;
{
    register struct scc_softc *sc = sccsc;
    register int unit;
    
    unit = minor(tp->t_dev);
    if (sccmodem[unit] & MODEM_DSR_START) {
	sccmodem[unit] &= ~MODEM_DSR_START;
	/*
	 * since dc7085 chip on PMAX only provides DSR then assume that CD
	 * has come up after 1 sec and start tty.  If CD has not
	 * come up the modem should deassert DSR thus closing the line
	 *
	 * On 3max, we look for DSR|CTS|CD before establishing a
	 * connection.
	 */
	if (SCC_XMIT(unit)) {
	    scc_start_tty(tp);
	}
	return;
    }
    if ((tp->t_state&TS_CARR_ON)==0)
	scc_tty_drop(tp);
}

/*
 *
 * Function:
 *
 *	scc_cd_down
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
scc_cd_down(tp)
     register struct tty *tp;
{
    register int msecs, unit;
    
    unit = minor(tp->t_dev);
    msecs = 1000000 * (time.tv_sec - scctimestamp[unit].tv_sec) + 
	(time.tv_usec - scctimestamp[unit].tv_usec);
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
 *	scc_tty_drop
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
scc_tty_drop(tp)
     struct tty *tp;
{
    register struct scc_softc *sc = sccsc;
    register int unit;
    
    unit = minor(tp->t_dev);
    if (tp->t_flags & NOHANG)
	return;
    /* 
     * Notify any processes waiting to open this line.  Useful in the
     * case of a false start.
     */
    sccmodem[unit] = MODEM_BADCALL;
    tp->t_state &= ~(TS_CARR_ON|TS_TTSTOP|TS_BUSY|TS_ISUSP);
    wakeup((caddr_t)&tp->t_rawq);
    pgsignal(tp->t_pgrp, SIGHUP, 1);
    pgsignal(tp->t_pgrp, SIGCONT, 1);
    SCC_CLR_DTR(unit);
    SCC_CLR_RTS(unit);
    SCC_CLR_SS(unit);
}


/*
 *
 * Function:
 *
 *	scc_start_tty
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
scc_start_tty(tp)
     register struct tty *tp;
{
    register int unit;
    
    unit = minor(tp->t_dev);
    tp->t_state &= ~(TS_ONDELAY);
    tp->t_state |= TS_CARR_ON;
    sccspeedi(unit);
    if (sccmodem[unit] & MODEM_DSR)
	untimeout(scc_dsr_check, tp);
    sccmodem[unit] |= MODEM_CD|MODEM_CTS|MODEM_DSR;
    scctimestamp[unit].tv_sec = scctimestamp[unit].tv_usec = 0;
    if (tp->t_state & TS_MODEM_ON) {
	tp->t_state |= TS_ISOPEN;
    }
    wakeup((caddr_t)&tp->t_rawq);
}

sccbaudrate(speed)
     int speed;
{
    return/*( scc_speeds[speed & CBAUD].baud_support )*/;
}

/* checks for SI */
sccspeedi(unit)
     register int unit;
{
    register struct scc_softc *sc = sccsc;
    register struct tty *tp;
    register struct scc_reg *rsp;
    register struct scc_saved_reg *ssp;
    unsigned char lospeed, hispeed;

    tp = &scc_tty[unit];
    rsp = sc->sc_regs[unit];
    ssp = sc->sc_saved_regs[unit];  /* mjm - sync support */

    ssp->wr14 &= ~(SCC_WR14_BRGEN_EN);
    SCC_WRITE(rsp, SCC_WR14, ssp->wr14);	     /*	 WR14 BRG disable */
    if (SCC_SI(unit))  {  /* if Speed Indicate is set */
        if (scc_cbaud[unit] != (tp->t_ospeed)) { /* not full speed */ 
	    scc_cbaud[unit] = tp->t_ospeed;
    	    lospeed = ttspeedtab(scc_cbaud[unit], ss_lo_speeds);    
    	    hispeed = ttspeedtab(scc_cbaud[unit], ss_hi_speeds);    
	    SCC_WRITE(rsp, SCC_WR12, lospeed);
	    SCC_WRITE(rsp, SCC_WR13, hispeed);
	}
    } else {
        if (scc_cbaud[unit] == (tp->t_ospeed)) { /* full speed */
	    scc_cbaud[unit] = scc_half_speed(scc_cbaud[unit]);
    	    lospeed = ttspeedtab(scc_cbaud[unit], ss_lo_speeds);    
    	    hispeed = ttspeedtab(scc_cbaud[unit], ss_hi_speeds);    
	    SCC_WRITE(rsp, SCC_WR12, lospeed);
	    SCC_WRITE(rsp, SCC_WR13, hispeed);
        } 
    }
    ssp->wr14 |= (SCC_WR14_BRGEN_EN);
    SCC_WRITE(rsp, SCC_WR14, ssp->wr14);	     /*	 WR14 BRG enable */
}   

scc_half_speed(cbaud)
     register int cbaud;
{
    register int ncbaud = cbaud;

    switch (cbaud) {
    case B38400: case B19200: case B9600: 
    case B4800: case B1200: case B600: 
        ncbaud--;
	break;
    case B2400: ncbaud = B1200;
	break;
    case B300: ncbaud = B150;
	break;
    case B150: ncbaud = B75;
	break;
    default:ncbaud = cbaud; /* no half-speed counterpart */
	break;
    }
    return(ncbaud);
}

/*
 *  mjm - sync support.
 *  Required for new interrupt dispatcher.
 */
scc_stub()
{
    return (0);
}
