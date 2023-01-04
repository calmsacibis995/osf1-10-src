/* #define FORCE_ALT_CONSOLE */

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
static char *rcsid = "@(#)$RCSfile: gpc.c,v $ $Revision: 1.1.5.7 $ (DEC) $Date: 1993/12/09 20:25:33 $";
#endif

/* #define DEBUG_PRINT_ENTRY */
/* #define DEBUG_PRINT_ERRORS */
/* #define DEBUG_PRINT_HWRPB */
/* #define DEBUG_PRINT_INFO */
/* #define DEBUG_PRINT_INTERRUPT */
/* #define DEBUG_PRINT_EXIT */

/* #define DPRINTF jprintf */
#define DPRINTF printf
/* #define DPRINTF xxputs */

#include  <sys/types.h>
#include  <sys/tty.h>
#include  <sys/param.h>
#include  <sys/exec.h>
#include  <sys/conf.h>
#include  <io/common/devio.h>
#include  <io/common/devdriver.h>
#include  <sys/uio.h>

#include <data/ws_data.c>
#include <data/pcxal_data.c>
#include <data/pcxas_data.c>

#include <hal/cpuconf.h>
#include <hal/cons_sw.h>
#include <machine/rpb.h>

extern int cpu;			   /* Global cpu type value */
extern struct vcons_init_sw vcons_init[];

/*
 * Graphics device driver function pointers.
 * Used to call graphics device driver as needed.
 */
extern int  (*v_consputc)();
extern int  (*v_consgetc)();

extern int  (*vs_gdopen)();
extern int  (*vs_gdclose)();
extern int  (*vs_gdread)();
extern int  (*vs_gdwrite)();
extern int  (*vs_gdselect)();
extern int  (*vs_gdkint)();		/* keyboard interrupt */
extern int  (*vs_gdpint)();		/* pointer interrupt */
extern int  (*vs_gdioctl)();
extern int  (*vs_gdmmap)();
extern int  (*vs_gdstop)();


/*
 * define global base address register for gpc
 *
 * Register I/O access is based on this base address which
 * is set-up in gpc_cons_init.
 */
u_long	gpc_regbase = 0;

/*
 * I/O Macros
 *
 * define macros for doing register I/O. They use the
 * standard bus I/O routines,  and require the gpc_regbase
 * which is set up in gpc_cons_init.
 */
extern long read_io_port();
#define READ_STATUS(s)	s = READ_BUS_D8((gpc_regbase)+(PK_STAT))
#define READ_DATA(d)	d = READ_BUS_D8((gpc_regbase)+(PK_OB))
#define READ_RTCD(r)	r = READ_BUS_D8((gpc_regbase)+(RTCD))
#define WRITE_CMD(c)	WRITE_BUS_D8((gpc_regbase)+(PK_CMD),(c)); \
    mb()
#define WRITE_DATA(d)	WRITE_BUS_D8((gpc_regbase)+(PK_IB),(d)); \
    mb()
#define WRITE_RTCA(a)	WRITE_BUS_D8((gpc_regbase)+(RTCA),(a)); \
    mb()
#define WRITE_RTCD(d)	WRITE_BUS_D8((gpc_regbase)+(RTCD),(d)); \
    mb()



/* Stub routines for gpc driver to allow console switch to work. */

int gpcprobe_reg(), gpcattach_ctlr();

/* FIXME FIXME - put the following into a "data/gpc_data.c" ??? */

void gpc_ctl_cmd();
void gpc_ctl_output();

#define NGPCLINE   2
struct tty gpc_tty[NGPCLINE];
#define GPC_KEYBD  0
#define GPC_MOUSE  1

caddr_t gpcstd[] = { 0 };
struct  controller *gpcinfo[1];

struct driver gpcdriver = { gpcprobe_reg, 0, gpcattach_ctlr, 0, 0,
			    gpcstd, 0, 0, "gpc", gpcinfo };

int
gpc_cons_init()
{
    int i, found = 0;
    caddr_t addr;
    struct controller *ctlr, *get_ctlr_num();

#ifdef NOT_YET
    int gpc_mouse_init(), gpc_mouse_putc(), gpc_mouse_getc();
    int gpc_kbd_init(), gpc_kbd_putc(), gpc_kbd_getc(), gpc_putc();
#endif /* NOT_YET */

#if DPRINTF == jprintf
    consDev = GRAPHIC_DEV; /* FIXME HACK - to force jprintf to use xxputs */
#endif /* DPRINTF = jprintf */

#ifdef DEBUG_PRINT_ENTRY
DPRINTF("gpc_cons_init: entry\n");
#endif /* DEBUG_PRINT_ENTRY */

    /*
     * Systems which make use of the PS/2 keyboard controller
     * will have the same registers with the same offsets, but may have
     * a different location for the base address of the interface.
     *
     * Set-up global base address of registers which is used
     * in the gpc read and write I/O macros.  This base address
     * will differ depending on the system.  For Jensen,  this
     * base address is the address of the vti combo chip, the
     * jensen r/w bus routines check for the vti base address to
     * do a local I/O swizzle.  On other systems it will typically
     * be zero.
     *
     * Get the base address from the gpc ctlr structure,  which
     * is set up during the cpu initialization routine.  This keeps
     * us from having to case on cpu types in this driver.
     *
     */
    if( (ctlr = get_ctlr_num("gpc",0)) == (struct controller *)NULL)
	/* if we get here gpc is not in the config file */
	panic("cons_init: gpc0 ctlr struct not found");
    else
	{
	    /* guard against ctlr initialization */
	    if ( (u_long)ctlr->physaddr >=0 )
		gpc_regbase = (u_long)ctlr->physaddr;
	}


    /*
     * preliminary init/disable of keyboard/mouse interface just in case
     */
    gpc_init_keyboard();
    while (gpc_input() != -1)
	    continue;
    gpc_ctl_cmd(PK_CTL_RDMODE);
    i = gpc_ctl_input();
    i &= ~(PK_MODE_EKI|PK_MODE_EMI);
    gpc_ctl_cmd(PK_CTL_WRMODE);
    gpc_ctl_output(i);
    gpc_ctl_cmd(PK_CTL_DISABLE);
    gpc_ctl_cmd(PK_CTL_MDISABLE);

    /*
     *
     * Query the prom. The prom can be set such that the user 
     * could use either the alternate tty or the graphics console.
     * The user may set the "console" environment variable
     */
    {
	int cons_unit, console_slot, slot;
	char *str;
	extern struct rpb_ctb *ctb;
#ifdef FIXME
/* FIXME FIXME - does EISA support have something similar???? */
	char *eisa_slot_to_name();
	caddr_t eisa_slot_to_addr();
#endif /* FIXME */
	
	/*
	 * Copy PCXAL-specific keyboard structure and
	 *  PCXAS-specific mouse structures to the appropriate globals.
	 * It's done this way so that generic kernels are possible.
	 */
/* FIXME FIXME - should use pointers for "keyboard", "mouse", etc */
	keyboard = pcxal_keyboard;
	mouse = pcxas_mouse;
	mouse_closure = pcxas_mouse_closure;
	
	/*
	 * init the slu tty structure no matter which console is active
	 */
	slu.slu_tty  = gpc_tty;
	
	/*
	 * The console is recorded according to the SRM on Jensen
	 */
	cons_unit = ctb->rpb_type;

#ifdef DEBUG_PRINT_HWRPB
DPRINTF("gpc_cons_init: HWRPB ctb at 0x%lx\n", (unsigned long)ctb);
DPRINTF("gpc_cons_init: HWRPB cons_unit %d\n", cons_unit);
#endif /* DEBUG_PRINT_HWRPB */

#ifdef FORCE_ALT_CONSOLE
	if (0)
#else /* FORCE_ALT_CONSOLE */
	if (cons_unit == CONS_GRPH)
#endif /* FORCE_ALT_CONSOLE */
	{
	    consDev = GRAPHIC_DEV;
	    
#ifdef FIXME
	    /* FIXME FIXME - the EISA slot must be acquired somehow */
	    /* read which slot console is in as indicated by firmware */
	    console_slot = *(long *)((long)ctb + ???);

#ifdef DEBUG_PRINT_INFO
DPRINTF("gpc_cons_init: HWRPB console_slot = %d\n", console_slot);
#endif /* DEBUG_PRINT_INFO */

#else
	    console_slot = 1;
#endif /* FIXME */

#ifdef FOR_MORGAN
/* FIXME - must know what MORGAN firmware will return for built-in VGA */
	    if (console_slot == 0)
	    { /* embedded graphics */
		if (cpu == ALPHA_MORGAN)
		    console_slot = MORGAN_VGA_SLOT;	
	    }
#endif /* FOR_MORGAN */

	    slot = console_slot - 1;
	    if (slot >= 0) {
#ifdef FIXME
	      str = eisa_slot_to_name(slot);
	      addr = eisa_slot_to_addr(slot);
#else
	      str = "VGA--VGA"; /* HACK: matches VGA vcons_init entry */
	      addr = (caddr_t)0;	/* HACK - passed to vga_attach */
		    			/* HACK - from vga_cons_init, where */
		    			/* HACK - it currently triggers the */
		    			/* HACK - probing of all slots to */
		    			/* HACK - get a proper slot */
#endif /* FIXME */

	      /*
	       * look for module name in table of known modules
	       */
	      for ( i = 0; vcons_init[i].modname[0] != 0; i++) {
		if (!strcmp(str, vcons_init[i].modname)) {		    
		    /* check to see that driver will configure */
		    /* if not, use generic console */
		    if ((*vcons_init[i].cons_init)(addr, slot))
			found = 1;
		    break;
		}
	      }
	    }
#ifdef FIXME
	    if (!found) {
		install_generic_console();
	    }
#endif /* FIXME */
	}
    }	    

#ifdef DEBUG_PRINT_EXIT
DPRINTF("gpc_cons_init: exiting: found = %d\n", found);
#endif /* DEBUG_PRINT_EXIT */

    return(found);
}

/*
 * probe routine called during bus controller probing
 *  via probe field in the "gpc" driver struct.
 */
int
gpcprobe_reg(reg, ctrl)
	int reg, ctrl;
{
#ifdef DEBUG_PRINT_ENTRY
DPRINTF("gpcprobe_reg: entry\n");
#endif /* DEBUG_PRINT_ENTRY */
	return(1);
}

/*
 * controller attach routine called during bus controller probing
 *  via cattach field in the "gpc" driver struct.
 */
int
gpcattach_ctlr(ctlr)
	struct controller *ctlr;
{
#ifdef DEBUG_PRINT_ENTRY
DPRINTF("gpcattach_ctlr: entry: return zero (0) always\n");
#endif /* DEBUG_PRINT_ENTRY */
	return(0);
}

/*
 * routine called via console probe routine
 *  (abcprobe in arch/alpha/hal/dec2000_cons.c)
 *
 * FIXME - only reason this gets called from abcprobe is so that
 *         gpcattach can get called, which simply does queue_inits
 *	   on the gpc_tty structs. 
 */
int
gpcprobe(ctlr)
	int ctlr;
{
#ifdef DEBUG_PRINT_ENTRY
DPRINTF("gpcprobe: entry: returning one (1) always\n");
#endif /* DEBUG_PRINT_ENTRY */
	gpcattach(ctlr);
	return(1);
}

int gpcattach(ctlr)
	int ctlr;
{
	register int i;
	
#ifdef DEBUG_PRINT_ENTRY
DPRINTF("gpcattach: entry\n");
#endif /* DEBUG_PRINT_ENTRY */

	for (i = 0; i < NGPCLINE; i++)
		queue_init(&gpc_tty[i].t_selq);

	return(0);
}

gpcopen(dev, flag)
	dev_t dev;
	int flag;
{
	register int unit = minor(dev);

#ifdef DEBUG_PRINT_ENTRY
DPRINTF("gpcopen: entry: unit = %d\n", unit);
#endif /* DEBUG_PRINT_ENTRY */

	if (vs_gdopen)
		return((*vs_gdopen)(dev, flag));
	else
		return(ENODEV);	/* FIXME ?? */
}

int gpcclose(dev, flag)
	dev_t dev;
	int flag;
{
	register int unit = minor(dev);

#ifdef DEBUG_PRINT_ENTRY
DPRINTF("gpcclose: entry: unit = %d\n", unit);
#endif /* DEBUG_PRINT_ENTRY */

	if (vs_gdclose)
		return((*vs_gdclose)(dev, flag));
	else
		return(ENODEV);	/* FIXME ?? */
}

int
gpcread(dev, uio, flag)
	dev_t dev;
	struct uio *uio;
	int flag;
{
	register struct tty *tp;
	register int unit;
	
	unit = minor(dev);

#ifdef DEBUG_PRINT_ENTRY
DPRINTF("gpcread: entry: unit = %d\n", unit);
#endif /* DEBUG_PRINT_ENTRY */

	tp = &gpc_tty[unit];

	return ((*linesw[tp->t_line].l_read)(tp, uio, flag));
}

int
gpcwrite(dev, uio, flag)
	dev_t dev;
	struct uio *uio;
	int flag;
{
	register struct tty *tp;
	extern struct tty *constty;
	register int unit;
	
	unit = minor(dev);
	
#ifdef DEBUG_PRINT_ENTRY
DPRINTF("gpcwrite: entry: unit = %d\n", unit);
#endif /* DEBUG_PRINT_ENTRY */
	
	/*
	 * Don't allow writes to the mouse,
	 * just fake the I/O and return.
	 */
	if (unit == GPC_MOUSE)
	{
		uio->uio_offset = uio->uio_resid;
		uio->uio_resid = 0;
		return(0);
	}

	tp = &gpc_tty[unit];

	return ((*linesw[tp->t_line].l_write)(tp, uio, flag));
}

int
gpcselect(dev, events, revents, scanning)
	dev_t dev;
	short *events, *revents;
	int scanning;
{
	register int unit = minor(dev);
	
#ifdef DEBUG_PRINT_ENTRY
DPRINTF("gpcselect: entry: unit = %d\n", unit);
#endif /* DEBUG_PRINT_ENTRY */

	if (unit == GPC_MOUSE) {
	    if (vs_gdselect)
		return((*vs_gdselect)(dev, events, revents, scanning));
	    else
		return(ENODEV);	/* FIXME ?? */
	}
	else
	    return(ttselect(dev, events, revents, scanning));
}

int
gpcioctl(dev, cmd, data, flag)
	dev_t dev;
	u_int cmd;
	register caddr_t data;
{
#ifdef DEBUG_PRINT_ENTRY
DPRINTF("gpcioctl: entry: calling wsioctl\n");
#endif /* DEBUG_PRINT_ENTRY */
	if (vs_gdioctl)
		return((*vs_gdioctl)(dev, cmd, data, flag));
	else
		return(ENODEV);	/* FIXME ?? */
}

int
gpcputc(c)
	char c;
{
#ifdef DEBUG_PRINT_ENTRY_DISABLED
DPRINTF("gpcputc: entry: calling v_consputc\n");
#endif /* DEBUG_PRINT_ENTRY */

	/*
	 * shouldn't get here unless we are graphics console,
	 *  but double check anyhow...
	 */
        if (v_consputc) {
		(*v_consputc)(c);
		if (c == '\n')
			(*v_consputc)('\r');
		return;
        }
	/* FIXME - error condition, but how to report it??? */
}

int
gpcgetc()
{
	char c;

#ifdef DEBUG_PRINT_ENTRY
DPRINTF("gpcgetc: entry: calling v_consgetc\n");
#endif /* DEBUG_PRINT_ENTRY */

	c = gpc_input();

	if (v_consgetc)
		return ((*v_consgetc)(c));
	else
		return (c);
}

int gpcrint()
{
#ifdef DEBUG_PRINT_ENTRY
DPRINTF("gpcrint: entry\n");
#endif /* DEBUG_PRINT_ENTRY */
	return(0);
}

int gpcxint()
{
#ifdef DEBUG_PRINT_ENTRY
DPRINTF("gpcxint: entry\n");
#endif /* DEBUG_PRINT_ENTRY */
	return(0);
}

int gpcstart()
{
#ifdef DEBUG_PRINT_ENTRY
DPRINTF("gpcstart: entry\n");
#endif /* DEBUG_PRINT_ENTRY */
	return(0);
}

int
gpcintr(ctlr)
	int ctlr;
{
	int status, data;

	READ_STATUS(status);

/* FIXME FIXME - should look for error status in STATUS register */

	if (status & PK_STAT_OBF)
	{
		READ_DATA(data);
		data &= 0xff;

#ifdef DEBUG_PRINT_INTERRUPT
DPRINTF("gpcintr: entry: status 0x%x  data 0x%x\n", status, data);
#endif /* DEBUG_PRINT_INTERRUPT */

		/*
		 * If we have a pointer interrupt here, we need to
		 * do some pre-processing of the data, since the
		 * mouse/tablet routines in the WS layer expect
		 * a full "report" before being called.
		 */
		if (status & PK_STAT_ODS) /* means pointer data */
		{
			if (vs_gdpint)
				return((*vs_gdpint)(status, data));		
			else
				return(ENODEV);	/* FIXME ?? */
		}
		
		/*
		 * look here for some things we should try to process,
		 *  like RESEND and ACK
		 */
		if (data == PK_RESEND)
		{
			gpc_keyboard_resend();
			return(0);
		}
		else if (data == PK_ACK) 
		{
			gpc_keyboard_ack();
			return(0);
		}

		/*
		 * "normal" data gets sent off to keyboard interrupt
		 *  routine for it to do its thing...
		 */
		if (vs_gdkint)
			return((*vs_gdkint)(data));		
		else
			return(ENODEV);	/* FIXME ?? */
	}
#if defined(DEBUG_PRINT_ERRORS) || defined(DEBUG_PRINT_INTERRUPT)
	else 
	{
		DPRINTF("gpcintr: no data available: status 0x%x\n", status);
		READ_DATA(data);
		data &= 0xff; /* FIXME - read anyway?? */
	}
#endif /* DEBUG_PRINT_ERRORS */
}


/* FIXME FIXME FIXME */

/***************************************************************************/
/*									   */
/*	Polled Mode Routines for keyboard/mouse communication		   */
/*									   */
/***************************************************************************/


void gpc_ctl_output();

int gpc_last_data;
enum why_ack {NOT_WAITING, SET_LEDS, DATA_ACK};
enum why_ack gpc_ack_mode = NOT_WAITING;

int
gpc_keyboard_resend()
{
	gpc_ctl_output(gpc_last_data);
}

int
gpc_keyboard_ack()
{
	switch (gpc_ack_mode)
	{
	case SET_LEDS:
		gpc_send_leds();
		break;

	case DATA_ACK:
		gpc_finish_leds();
		break;

	case NOT_WAITING:
/*		DPRINTF("gpc_keyboard_ack: unexpected ACK"); */
		break;

	default:
		DPRINTF("gpc_keyboard_ack: invalid ACK mode");
		break;
	}
}

#define MAX_LEDS_QUEUE 16
int leds_setters = 0;
int leds_next = 0;
int leds_cur = 0;
int leds_mask[MAX_LEDS_QUEUE];

int
gpc_set_leds(new_leds)
	int new_leds;
{
	/* process new request for setting keyboard LEDs */

	int s = splbio(); /* prevent keyboard interrupts for the duration */

	/*
	 * first, place data in queue *always*.
	 * then check for current setting already taking place;
	 * if so, we are done with this request,
	 * else start processing this one.
	 */
	leds_mask[leds_next] = new_leds;
	leds_next = (leds_next + 1) % MAX_LEDS_QUEUE;

        if (++leds_setters > 1) {
                splx(s);
                return(0);
        }

	gpc_ctl_output(PK_SETLEDS);
	gpc_ack_mode = SET_LEDS;

	splx(s);
}

int
gpc_send_leds()
{
	/* received ACK for the SETLEDS command, so now send the LEDs bits */
	/* this is done during interrupt processing, so no SPL needed */

	gpc_ctl_output(leds_mask[leds_cur]);
	leds_cur = (leds_cur + 1) % MAX_LEDS_QUEUE;
	gpc_ack_mode = DATA_ACK;
}

int
gpc_finish_leds()
{
	/* received ACK for the LEDs bits, so check for another to do... */
	/* this is done during interrupt processing, so no SPL needed */
	
        if (--leds_setters > 0) {
		gpc_ctl_output(PK_SETLEDS);
		gpc_ack_mode = SET_LEDS;
        }
	else
		gpc_ack_mode = NOT_WAITING;
}

/*
 * Write the specified command out to the keyboard controller command port.
 * Wait for output to complete, but do not wait for response.
 */
void
gpc_ctl_cmd(int c)
{
	int status;
        /*
	 * Wait for previous output to complete - paranoia.
         * Send out the specified command.
         * Wait for output to complete.
	 */
        do
		READ_STATUS(status);
	while ((status & PK_STAT_IBF) != 0);

        WRITE_CMD(c);

#ifdef GPC_PARANOID
        do
		READ_STATUS(status);
	while ((status & PK_STAT_IBF) != 0);
#endif /* GPC_PARANOID */
}

/*
 * Write the specified data out to the keyboard controller data port.
 * Do not wait for any response.
 */
void
gpc_ctl_output(int d)
{
	int status;

	/* Wait for previous output to complete. */
	do
		READ_STATUS(status);
	while ((status & PK_STAT_IBF) != 0);

        WRITE_DATA(d);
	gpc_last_data = d; /* FIXME FIXME */
}

/*
 * Read data (ostensibly) from the keyboard controller via the data port.
 */
int
gpc_ctl_input(void)
{
        int status, data;
        
        /* Wait for data to become available. */
	do
		READ_STATUS(status);
        while ((status & PK_STAT_OBF) == 0);

#ifdef MOUSE_DEBUG
        if (status & PK_STAT_ODS)
                DPRINTF("gpc_ctl_input: MOUSE input!\n");
#endif /* MOUSE_DEBUG */
        
        /* The read of this data register will clear the OBF condition. */
	READ_DATA(data);
        return (data & 0xff);
}

/*
 * Routine to get a scancode from PCXAL keyboard.
 * NOTE: lowest level routine; goes to hardware to fetch character,
 *	 and operates in POLLED mode with a timeout.
 * (roughly equivalent to scc_getc from io/dec/tc/scc.c)
 */
gpc_ctl_getcode()
{
	int status, data;
	register int timo;
	
	for (timo = 1000000; timo > 0; --timo) {
		READ_STATUS(status);
		if ( !(status & PK_STAT_OBF) )
			continue;           /* Nothing there */
		READ_DATA(data);
		data &= 0xff;
		if ((status & PK_STAT_PERR) || (status & PK_STAT_GTO))
			continue;
		break;
	}
	if (timo == 0)
		return(-1);
	else
		return(data);
}

/*
 * Write the specified data out to the keyboard controller data port.
 * Wait for non-RESEND response back.
 */
int
gpc_output(int c)
{
        int d, status;

	gpc_last_data = c; /* FIXME FIXME */
        do {
                /* Wait for previous output to complete. */
                do
			READ_STATUS(status);
		while ((status & PK_STAT_IBF) != 0);
                WRITE_DATA(c);
        } while ((d = gpc_input()) == PK_RESEND);
        return (d);     
}

/*
 * Write the specified data out to the keyboard controller data port.
 * Wait for any response back.
 */
int
gpc_output_noverify(int c)
{
	int status;

	/* Wait for previous output to complete. */
	do
		READ_STATUS(status);
        while ((status & PK_STAT_IBF) != 0);

        WRITE_DATA(c);
	gpc_last_data = c; /* FIXME FIXME */

        return(gpc_input());
}

/*
 * Spin for a limited amount of time waiting for data to come in from
 * the keyboard controller.  Returns (-1) if no data is available within
 * the timeout or if an error occured on the previous transmission (??)
 *
 * NOTE: This routine is NOT typically used to read in characters that users
 * have typed as input to the keyboard.  Rather it is typically used to
 * read keyboard or controller responses which are read as "input" rather
 * than via direct CSR access.
 */
int
gpc_input(void)
{
        int     n;
        int     status, data;

#ifdef ORIGINAL_CODE
        n = 1000000;
#else
        n = 500000;
#endif /* ORIGINAL_CODE */
        do {
                READ_STATUS(status);
                /*
                 * Check to see if a timeout error has occured.  This means
                 * that transmission was started but did not complete in the
                 * normal time cycle.  PERR is set when a parity error occured
                 * in the last transmission.
                 */
                if (status & (PK_STAT_GTO | PK_STAT_PERR)) {
#ifdef DEBUG_PRINT_ERRORS
                        DPRINTF("gpc_input: error on GTO or PERR.\n"); 
#endif /* DEBUG_PRINT_ERRORS */
                        return (-1);
                }
                /*
                 * Wait for input data to become available.  This bit will
                 * then be cleared by the following read of the DATA
                 * register.
                 */
                if (status & PK_STAT_OBF) {
#ifdef MOUSE_DEBUG
                        if (status & PK_STAT_ODS)
                                DPRINTF("gpc_input: MOUSE input!\n");
#endif /* MOUSE_DEBUG */
			READ_DATA(data);
                        return (data & 0xff);
                }
        } while (--n);
        return (-1);	/* timed-out if fell through to here... */
}

gpc_init_keyboard()
{	
	int d;
	
        /* 
         * Select PS/2 mode for the PCXAL in Chip Select Control Register #1.
         */
        WRITE_RTCA(RTCCR1);	/* set chip select addr to 6A (ctl reg #1) */
	READ_RTCD(d);		/* read the contents of that register */
        d &= ~ATKBD;		/* clear the AT keybd bit */
        WRITE_RTCA(RTCCR1);	/* set that address again */
        WRITE_RTCD(d);		/* write WITHOUT ATKBD bit set */
}
