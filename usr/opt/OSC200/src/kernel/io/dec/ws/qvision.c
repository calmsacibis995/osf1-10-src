/* remove when EISA interrupt setup is fixed for multi-board configurations */
#define DOUBLE_CHECK_INTERRUPT

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
static char *rcsid = "@(#)$RCSfile: qvision.c,v $ $Revision: 1.1.4.8 $ (DEC) $Date: 1993/12/09 00:04:10 $";
#endif

/* FIXME FIXME - make sure arg lists of functions called via "af" are OK */

/* #define DEBUG_PRINT_ECU */
/* #define DEBUG_PRINT_ENTRY */
/* #define DEBUG_PRINT_MAPPING */
/* #define DEBUG_PRINT_CONSOLE_DUMP */
/* #define DEBUG_PRINT_PROBE */
/* #define DEBUG_PRINT_INTERRUPT */
/* #define DEBUG_PRINT_COLOR */
/* #define DEBUG_PRINT_CURSOR */
/* #define DEBUG_PRINT_VIDEO */
/* #define DEBUG_PRINT_INFO */

/* #define DPRINTF jprintf */
#define DPRINTF printf
/* #define DPRINTF xxputs */

/*
 * COMPAQ QVision VGA Controller driver,
 * based on:
 */
/*
 * Paradise specific code
 * Adapted to OSF1 device driver by Dean Anderson 4/91 
 */
/*
 * Copyright 1990,91 by Thomas Roell, Dinkelscherben, Germany.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Thomas Roell not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Thomas Roell makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * THOMAS ROELL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THOMAS ROELL BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Thomas Roell, roell@informatik.tu-muenchen.de
 */

#include <data/ws_data.c>
#include <data/vga_data.c>

#include <sys/proc.h>
#include <sys/user.h>
#include <io/dec/eisa/eisa.h>

typedef struct {
  vgaHWRec std;          /* std IBM VGA registers - must be first! */
  /* QVision special */
  unsigned char AddrMapHI;
  unsigned char AddrMapLO;
  unsigned char CntrlReg_0;
  unsigned char CntrlReg_1;
  unsigned char DACCmd_0;
  unsigned char DACCmd_1;
  unsigned char DACCmd_2;
  unsigned char OverFlow_1;
  unsigned char OverFlow_2;
  unsigned char BLTCnfg;
  unsigned char EnvStat_0;
} vgaQVISIONRec, *vgaQVISIONPtr;

/* macros for I/O register access */
#define INB(a,d)	d = read_io_port((a),1,BUS_IO)
#define OUTB(a,v)	{write_io_port((a),1,BUS_IO,(v));mb();}
#define OUTW(a,v)	{write_io_port((a),2,BUS_IO,(v));mb();}

/* FIXME FIXME - hardwired addresses for now... */
#define HIGHMAP_BASE	0x00100000	/* unit 0 highmap base address */
#define HIGHMAP_SIZE	0x00100000	/* highmap size is 1Mb */

#define NULL_HIGHMAP_BASE ((vm_offset_t)0xfffffc00deadbeefL) /* FIXME */
#define OLD_CONFIG_ADDR   ((vm_offset_t)0x01f00000) /* FIXME */

#define IOREGS_BASE	0x00000000	/* where IO regs mapping starts */
#define IOREGS_SIZE	0x00010000	/* size of IO regs mapped */

/* these should get dynamically allocated */
static  vgaQVISIONRec vgat1;
static  vgaQVISIONRec vgat2;

/* Controller EISA ID's */
#define QVISION_SPRUCE_ID	0x1130110e
#define QVISION_FIR_ID		0x1131110e

/*
 * Globals
 */
vm_offset_t qvision_get_highmap_address();

/* FB memory physical address on the bus:
 *   	global since *ALL* fb's need to be mapped there
 */
vm_offset_t qvision_fb_phys = NULL_HIGHMAP_BASE;

/* FB virtual address in Xserver's address space:
 *   	global since *ALL* fb's will appear there
 */
vm_offset_t qvision_fb_virt;

/* I/O registers virtual address in Xserver's address space:
 *   	global since *ALL* fb's I/O registers will appear there
 */
vm_offset_t qvision_io_virt;

/* mapped state, since we only want to do it *ONCE* for each Xserver instance */
int qvision_mapped_state = 0;
pid_t qvision_mapped_pid = -1;

/* configured state, since we only want to do it *ONCE* */
int qvision_configured = 0;

/* controller count */
int qvision_controllers = 0;

/* interrupt usable */
int qvision_interrupt_possible = 0;

/* console pointer */
struct vga_info *qvision_console = NULL;

int
qvision_select_controller(vp)
	register struct vga_info *vp;
{
	register int saved_id;

	if (qvision_controllers < 2) return(0);

	INB(0x83c4, saved_id); /* read Virtual Ctlr Select reg */
	saved_id &= 0x0f;

	if (saved_id != vp->unit) {
#ifdef DEBUG_PRINT_ENTRY_DISABLED
DPRINTF("qvision_select_controller: changing from %d to %d\n",
	saved_id, vp->unit);
#endif /* DEBUG_PRINT_ENTRY */
		OUTB(0x83c4, vp->unit); /* set Virtual Ctlr Select reg */
	}
	
	return(saved_id);
}

int
qvision_restore_controller(id)
	register int id;
{
	register int saved_id;

	if (qvision_controllers < 2) return(0);

	INB(0x83c4, saved_id);
	saved_id &= 0x0f;

	if (saved_id != id) {
#ifdef DEBUG_PRINT_ENTRY_DISABLED
DPRINTF("qvision_restore_controller: changing from %d to %d\n", saved_id, id);
#endif /* DEBUG_PRINT_ENTRY */
		OUTB(0x83c4, id); /* set Virtual Controller Select reg */
	}
}

/*
 * probe routine called during bus probing and during console init
 */
int
qvision_probe(addr)
	register unsigned long addr;
{
	register int prod_id, ctlr_id;

#ifdef DEBUG_PRINT_ENTRY_DISABLED
DPRINTF("qvision_probe(0x%lx): entry\n", addr);
#endif /* DEBUG_PRINT_ENTRY */

/* FIXME FIXME - for now, we assume EISA and look at the Product ID */

	prod_id = read_io_port(addr + 0x0c80, 4, BUS_IO);

	ctlr_id = read_io_port(addr + 0x0c85, 1, BUS_IO) & 0x0f;

#ifdef DEBUG_PRINT_PROBE
DPRINTF("qvision_probe(0x%lx) prod_id 0x%x  ctlr_id 0x%x\n",
	addr, prod_id, ctlr_id);
#endif /* DEBUG_PRINT_PROBE */

	/*
	 * check the EISA ID numbers for the controllers we support
	 */
	if (prod_id != QVISION_SPRUCE_ID && prod_id != QVISION_FIR_ID)
		return(0);

	return((ctlr_id == 0) ? 2 : 1); /* FIXME - 2 == PRIMARY */
}

/*
 * attach routine called during bus probing; *NOT* during console init
 */
int
qvision_attach(ctlr)
  register struct controller *ctlr;
{
#ifdef DEBUG_PRINT_ENTRY
DPRINTF("qvision_attach(0x%x): entry\n", ctlr->ctlr_num);
#endif /* DEBUG_PRINT_ENTRY */
	return(0);
}

/*
 *-----------------------------------------------------------------------
 * qvision_save --
 *      save the current video mode
 *
 * Results:
 *      pointer to the current mode record.
 *
 * Side Effects: 
 *      None.
 *-----------------------------------------------------------------------
 */
void
qvision_save(save)
     vgaQVISIONPtr save;
{
  int temp;
#ifdef DEBUG_PRINT_ENTRY
DPRINTF("qvision_save: entry\n");
#endif /* DEBUG_PRINT_ENTRY */

  /*
   * assume, since we are *ONLY* called from "first_screen", that
   *  the appropriate controller has been selected
   */

  OUTB(0x3CE, 0x0F); OUTB(0x3CF, save->EnvStat_0);
  OUTB(0x3CE, 0x0F); OUTB(0x3CF, 0x05);   /* unlock special regs */

  vgaHWSave(save);

/* FIXME - any QVision special register saving goes here... */

  OUTB(0x3CE, 0x49); INB(0x3CF, save->AddrMapHI);
  OUTB(0x3CE, 0x48); INB(0x3CF, save->AddrMapLO);

  OUTB(0x3CE, 0x10); INB(0X3CF, save->BLTCnfg);

  OUTB(0x3CE, 0x10); OUTB(0X3CF, 0x08);	/* unlock *more* special regs */

  OUTB(0x3CE, 0x40); INB(0X3CF, save->CntrlReg_0);
  INB(0x63ca, save->CntrlReg_1);

  INB(0x83C6, save->DACCmd_0);
  INB(0x13c8, save->DACCmd_1);
  INB(0x13c9, save->DACCmd_2);

  OUTB(0x3CE, 0x42); INB(0X3CF, save->OverFlow_1);
  OUTB(0x3CE, 0x51); INB(0X3CF, save->OverFlow_2);

  OUTB(0x3CE, 0x10); OUTB(0X3CF, save->BLTCnfg);

  return;
}

/*
 *-----------------------------------------------------------------------
 * qvision_restore --
 *      restore a video mode
 *
 * Results:
 *      nope.
 *
 * Side Effects: 
 *      the display enters a new graphics mode. 
 *
 * NOTE: the only time this is (currently) used is during "init_screen"
 *	 processing, which is called to put the screen into "text" mode.
 *-----------------------------------------------------------------------
 */

void
qvision_restore(restore)
     vgaQVISIONPtr restore;
{
#ifdef DEBUG_PRINT_ENTRY
DPRINTF("qvision_restore: entry\n");
#endif /* DEBUG_PRINT_ENTRY */

  /*
   * initial special register setting occurs here...
   *
   * NOTE: we *MUST* set the registers *BEFORE* we do the VGA-generic
   *  restore, since it stops and starts the Sequencer which is necessary
   *  for the change in some register contents to be effective.
   */
  OUTB(0x46e8, 0x10);		/* video enable register - setup mode */
  OUTB(0x0102, 0x01);		/* option select register - cmd enable */
  OUTB(0x46e8, 0x08);		/* video enable register - enable normal */

  OUTB(0x3ce, 0x0f); OUTB(0x3cf, 0x05);   /* unlock special regs */

  OUTB(0x3ce, 0x10); OUTB(0x3cf, 0x08);	/* unlock *more* special regs */

  OUTB(0x3ce, 0x40); OUTB(0x3cf, restore->CntrlReg_0);
  OUTB(0x63ca, restore->CntrlReg_1);

  OUTB(0x83C6, restore->DACCmd_0);
  OUTB(0x13c8, restore->DACCmd_1);
  OUTB(0x13c9, restore->DACCmd_2);

  OUTB(0x3ce, 0x42); OUTB(0x3cf, restore->OverFlow_1);
  OUTB(0x3ce, 0x51); OUTB(0x3cf, restore->OverFlow_2);

  OUTB(0x3ce, 0x10); OUTB(0x3cf, restore->BLTCnfg);

  /*
   * now do normal VGA restoration
   */
  vgaHWRestore(restore);

  /*
   * final special register setting occurs here... (if necessary)
   *
   * NOTE: the High Address Map registers are NOT restored here, since a
   *  stop/start of the Sequencer (in vgaHWRestore) zeroes its contents,
   *  and that's really all we want at this time...
   *
   * Also, the AVGA enable in Control Reg 0 is not touched, also since the
   *  Sequencer was stop/started, which also clears that bit as well...
   *
   * These instructions would set those registers to "graphics" mode settings:
   *
   *	OUTB(0x3ce, 0x40); OUTB(0x3cf, 0x01);
   *	OUTB(0x3ce, 0x48); OUTB(0x3cf, (qvision_fb_phys >> 20) & 0xff);
   *	OUTB(0x3ce, 0x49); OUTB(0x3cf, (qvision_fb_phys >> 28) & 0x0f);
   */

  return;
}


/*
 *-----------------------------------------------------------------------
 * qvision_init --
 *      Handle the initialization, etc. of a screen.
 *
 * Input:
 *      pointer to the new mode record.
 *      pointer to the original mode record.
 *
 * Results:
 *	new mode record contents initialized.
 *
 * Side Effects: 
 *
 * NOTE: this routine currently sets up "text" mode register contents, and
 *	 uses some original mode register contents.
 *-----------------------------------------------------------------------
 */
void
qvision_init(new, orig)
  register vgaQVISIONPtr new, orig;
{
  register int i;
  register unsigned char *src, *dst;
	
#ifdef DEBUG_PRINT_ENTRY
DPRINTF("qvision_init: entry\n");
#endif /* DEBUG_PRINT_ENTRY */

  vgaHWInit(new);

  /*
   * now we use some fields from the original (assumed "text") mode
   * to try to preserve the console firmare's colormap and attribute 
   */
  /* first the DACs */
  src = &orig->std.DAC[0];
  dst = &new->std.DAC[0];
  for (i = 0; i < 768; i++)
	  *dst++ = *src++;

  /* next the Palette */
  src = &orig->std.Attribute[0];
  dst = &new->std.Attribute[0];
  for (i = 0; i < 16; i++)
	  *dst++ = *src++;

  /* finally, the attribute */
  i = new->std.TextAttribute = orig->std.TextAttribute;
  vga_normal_attribute = i;
  vga_reverse_attribute = ((i >> 4) & 0x0f) | ((i << 4) & 0xf0);

/* FIXME - what to do about multiple controllers??? */

/* FIXME FIXME - don't know yet what special things need doing */

  /*
   * Now reset the high address map register; the QVision board uses
   *  it to enable the frame buffer memory to be seen in its entirety,
   *  but we don't want that now...
   */

  new->AddrMapHI = 0x00;
  new->AddrMapLO = 0x00;

  new->CntrlReg_0 = 0x00;
  new->CntrlReg_1 = 0x00;
  new->DACCmd_0 = 0x00;
  new->DACCmd_1 = 0x00;
  new->DACCmd_2 = 0x00;
  new->OverFlow_1 = 0x00;
  new->OverFlow_2 = 0x00;
  new->BLTCnfg = 0x00;

  new->EnvStat_0 = 0x05; /* to unlock some registers */

  return;
}

/*
 *-----------------------------------------------------------------------
 * qvision_adjust --
 *      adjust the current video frame to display the mousecursor
 *
 * Results:
 *      nope.
 *
 * Side Effects: 
 *      the display scrolls
 *-----------------------------------------------------------------------
 */

void 
qvision_adjust(vpos)
     struct vga_pos *vpos;
{
  int Base;
  unsigned char temp;

#ifdef DEBUG_PRINT_ENTRY
DPRINTF("qvision_adjust: entry\n");
#endif /* DEBUG_PRINT_ENTRY */

#ifdef FIXME
  Base = (vpos->y * vgaVideoMode.virtualX + vpos->x) >> 2;
  
  OUTB(0x3D4, 0x0C); OUTB(0x3D5, (Base & 0x00FF00) >>8);
  OUTB(0x3D4, 0x0D); OUTB(0x3D5, Base & 0x00FF);
  OUTB(0x3CE, 0x0D); INB(0x3CF, temp); 
  OUTB(0x3CF, ((Base & 0x030000) >> 13) | (temp & 0xE7));
#endif /* FIXME */
}

/*
 *-----------------------------------------------------------------------
 * qvision_print --
 *      Print the QVision special register contents from the specified
 *	 shadow area
 *
 * Results:
 *      nope.
 *
 * Side Effects: 
 *      the register contents are printed
 *-----------------------------------------------------------------------
 */
void qvision_print(wh)
     vgaQVISIONPtr wh;
{

#ifdef DEBUG_PRINT_CONSOLE_DUMP
  DPRINTF("AddrMapHI:  0x%x \n", wh->AddrMapHI);
  DPRINTF("AddrMapLO:  0x%x \n", wh->AddrMapLO);
  DPRINTF("CntrlReg_0: 0x%x \n", wh->CntrlReg_0);
  DPRINTF("CntrlReg_1: 0x%x \n", wh->CntrlReg_1);
  DPRINTF("DACCmd_0:   0x%x \n", wh->DACCmd_0);
  DPRINTF("DACCmd_1:   0x%x \n", wh->DACCmd_1);
  DPRINTF("DACCmd_2:   0x%x \n", wh->DACCmd_2);
  DPRINTF("OverFlow_1: 0x%x \n", wh->OverFlow_1);
  DPRINTF("OverFlow_2: 0x%x \n", wh->OverFlow_2);
  DPRINTF("BLTCnfg:    0x%x \n", wh->BLTCnfg);
  DPRINTF("EnvStat_0:  0x%x \n", wh->EnvStat_0);

  vgaHWPrint(wh);
#endif /* DEBUG_PRINT_CONSOLE_DUMP */
}

/*
 *-----------------------------------------------------------------------
 * qvision_interrupt --
 *      handle vertical retrace interrupt processing
 *
 * Results:
 *      Changed colormap entries (if necessary).
 *	Changed cursor shape (if necessary).
 *
 * Side Effects: 
 *      None.
 *-----------------------------------------------------------------------
 */
#ifdef DOUBLE_CHECK_INTERRUPT
int qvision_interrupt_checked = 0;

int
qvision_interrupt_timeout()
{
	qvision_interrupt_possible = 0;
	qvision_interrupt_checked = 2;
	return;
}
#endif /* DOUBLE_CHECK_INTERRUPT */

void
qvision_interrupt(ctlr, vp)
	register struct controller *ctlr;
	register struct vga_info *vp;
{
	register int qc;

#ifdef DEBUG_PRINT_INTERRUPT
DPRINTF("qvision_interrupt(0x%lx, 0x%lx): entry\n",
	(unsigned long)ctlr, (unsigned long)vp);
#endif /* DEBUG_PRINT_INTERRUPT */

	qc = qvision_select_controller(vp);

	/* clear the interrupt first... */
	vga_clear_interrupt(vp);

	if (vp->dirty_cursor)
		qvision_load_formatted_cursor(vp);

	if (vp->dirty_colormap)
		qvision_clean_color_map(vp);

	/* disable further interrupts here when necessary... */
	vga_disable_interrupt(vp);

	qvision_restore_controller(qc);

#ifdef DOUBLE_CHECK_INTERRUPT
	if (qvision_interrupt_checked == 1) {
		qvision_interrupt_checked = 2;
		untimeout(qvision_interrupt_timeout, 0);
	}
#endif /* DOUBLE_CHECK_INTERRUPT */
}

int
qvision_enable_interrupt(vp)
	register struct vga_info *vp;
{
	int s;

#ifdef DEBUG_PRINT_INTERRUPT
DPRINTF("qvision_enable_interrupt: entry\n");
#endif /* DEBUG_PRINT_INTERRUPT */

	if (!qvision_interrupt_possible || qvision_controllers > 1)
		return(0);

#ifdef DOUBLE_CHECK_INTERRUPT
	if (!qvision_interrupt_checked) {
		qvision_interrupt_checked = 1;
		timeout(qvision_interrupt_timeout, 0, hz); /* try 1 sec now */
	}
#endif /* DOUBLE_CHECK_INTERRUPT */

	s = splbio();

	/* FIXME - need to select approp. adaptor here... */

	vga_enable_interrupt(vp);

	/* FIXME - need to restore approp. adaptor here... */

	splx(s);

	return(1); /* return success */
}

/*
 *-----------------------------------------------------------------------
 * qvision_bot --
 *      initialize a QVision board "at the Beginning Of Time".
 *
 * Results:
 *      Hardware and data structures are initialized.
 *
 * Side Effects: 
 *      None.
 *-----------------------------------------------------------------------
 */
void
qvision_bot(vp)
  register struct vga_info *vp;
{
#ifdef DEBUG_PRINT_ENTRY
DPRINTF("qvision_bot: screen=%d entry\n", vp->screen.screen);
#endif /* DEBUG_PRINT_ENTRY */
}

qvision_first_screen(closure, address, unit, type)
	caddr_t closure;
	caddr_t address;
	int unit;
        int type;
{
	struct vga_info *vp = ((struct vga_info *)closure) + unit;
	register int i, qc;

#ifdef DEBUG_PRINT_ENTRY
DPRINTF("qvision_first_screen: screen=%d entry\n", vp->screen.screen);
#endif /* DEBUG_PRINT_ENTRY */

	qc = qvision_select_controller(vp);

	/*
	 * save the VGA register contents left behind by the console
	 */
	qvision_save(vga_orig_state);

#ifdef DEBUG_PRINT_CONSOLE_DUMP
	DPRINTF("QVision Console Register State\n");
	qvision_print(vga_orig_state);
#endif /* DEBUG_PRINT_CONSOLE_DUMP */

	/*
	 * do initialization of shadow register struct
	 * may want some original state contents...
	 */
	qvision_init(vga_new_state, vga_orig_state);

#ifdef DEBUG_PRINT_CONSOLE_DUMP
	/* dump out the VGA register contents inited just before */
	DPRINTF("QVision INITED Register State\n");
	qvision_print(vga_new_state);
#endif /* DEBUG_PRINT_CONSOLE_DUMP */

	qvision_restore_controller(qc);
}

/*
 * Screen Init Closure
 */
caddr_t
qvision_screen_init_closure (closure, address, unit, type)
	caddr_t closure;
	caddr_t address;
	int unit;
        int type;
{
	struct vga_info *vp = (struct vga_info *)closure;
	register int i;

	vp = vp + unit;

#ifdef DEBUG_PRINT_ENTRY
DPRINTF("qvision_screen_init_closure: screen=%d entry\n", vp->screen.screen);
#endif /* DEBUG_PRINT_ENTRY */

	/*
	 * Once and only once, the first time we are called:
	 *
	 *   1. must inquire where the ECU wants us to map the frame buffer
	 *      on the bus...
	 *
	 *   2. init the VGA global state structure pointers for later use
	 *	by *all* boards
	 */
	if (!qvision_configured) {

		vga_orig_state = (void *)&vgat1;
		vga_new_state  = (void *)&vgat2;
	}

	for (i = 0; i < vp->screen.allowed_depths; i++) {
		vp->depth[i].physaddr = (caddr_t) NULL_HIGHMAP_BASE;
		vp->depth[i].plane_mask_phys = (caddr_t) IOREGS_BASE;

#ifdef DEBUG_PRINT_MAPPING_DISABLED
DPRINTF("qvision_screen_init_closure: unit %d depth[%d] fb 0x%lx regs 0x%lx\n",
	unit, i, vp->depth[i].physaddr, vp->depth[i].plane_mask_phys);
#endif /* DEBUG_PRINT_MAPPING */
	}

	vp->unit = unit;
	vp->screen_on = SCREEN_ON;
	vp->cursor_on = CURSOR_OFF;
	
	/* fetch Virtual Controller ID */
	INB((vm_offset_t)address + 0x0c85, i);
	vp->board_id = i & 0x0f; /* only low 4 bits meaningful... */

	/* set Virtual Controller ID to unit no. */
	OUTB((u_long)address + 0x0c85, unit & 0x0f);
#ifdef DEBUG_PRINT_INFO
if (unit != vp->board_id) {
	DPRINTF("qvision_screen_init_closure: board_id %d -> unit %d\n",
		vp->board_id, unit & 0x0f);
}
#endif /* DEBUG_PRINT_INFO */

	/*
	 * Once and only once, the first time we are called:
	 *
	 *   1. save original screen attributes and init new ones,
	 *      in the global VGA structures used by *all* boards.
	 */
	if (!qvision_configured) {
		qvision_first_screen(closure, address, unit, type);
		qvision_configured = 1;
	}

	/* must wait until *after* "first_screen" to do this... */
	vp->attribute = vga_normal_attribute;

	qvision_controllers++;
	if (qvision_controllers == 1)
		qvision_console = vp;

	return (caddr_t) vp;
}

/*
 * QVision routine to init a VGA screen.
 * Could be called from:
 *	ws_cons_init - for the console screen.
 *	wsopen       - for each screen, when going to graphics mode.
 *	wsputc       - for console screen, when panic printf needed.
 */
qvision_init_screen(closure, screen)
	caddr_t closure;
	ws_screen_descriptor *screen;
{
	register struct vga_info *vp = (struct vga_info *)closure;
	register ws_screen_descriptor *sp = &vp->screen;
	register int ret, qc;

#ifdef DEBUG_PRINT_ENTRY
DPRINTF("qvision_init_screen: screen=%d entry\n", vp->screen.screen);
#endif /* DEBUG_PRINT_ENTRY */

	qc = qvision_select_controller(vp);

	/*
	 * must move the screen "window" text to the top page
	 *  of screen memory, so that we can remain based there.
	 * but only need to do this when we are in text mode and
	 *  on the graphics console, so this will only happen
	 *  from ws_cons_init or wsputc (panic printf), although
	 *  we really only care about from ws_cons_init, so that
	 *  we can preserve the screen contents after the hand-off
	 *  from the console firmware...
	 */
	if (vp->unit == 0 && !ws_is_mouse_on()) 
	{
		int hi, lo, ov, offset;
		
		/* get the latest register settings... */
		OUTB(0x3d4, 0x0c); INB(0x3d5, hi); hi &= 0xff;
		OUTB(0x3d4, 0x0d); INB(0x3d5, lo); lo &= 0xff;
		OUTB(0x3ce, 0x0f); OUTB(0x3cf, 0x05); /* unlock... */
		OUTB(0x3ce, 0x42); INB(0x3cf, ov); ov  = (ov >> 2) & 0x03;
		
		offset = (ov << 16) + (hi << 8) + lo;
		if (offset)
			/*                          from,to,how_many */
			vga_charmvup(closure, sp, offset, 0, 80*25);
	}

	/*
	 * use generic VGA screen init routine
	 */
	ret = vga_init_screen(closure, screen);

	qvision_restore_controller(qc);

	return(ret);
}

/*
 * Routine to clear the screen.
 *
 * NOTE: currently, this is *only* called from wsclose, when going
 *       from graphics mode to text mode, so it only needs to operate
 *	 in text mode.
 */
qvision_clear_screen(closure, screen)
	caddr_t closure;
	ws_screen_descriptor *screen;
{
	struct vga_info *vp = (struct vga_info *)closure;
	register ws_screen_descriptor *sp = &vp->screen;
	register int ret, qc;

#ifdef DEBUG_PRINT_ENTRY
DPRINTF("qvision_clear_screen: screen=%d entry\n", vp->screen.screen);
#endif /* DEBUG_PRINT_ENTRY */

	qc = qvision_select_controller(vp);

	ret = vga_clear_screen(closure, screen);

	qvision_restore_controller(qc);

	return(ret);
}

/*
 * Scroll Screen
 */
qvision_scroll_screen(closure, screen)
	caddr_t closure;
	ws_screen_descriptor *screen;
{
	register struct vga_info *vp = (struct vga_info *)closure;
	register ws_screen_descriptor *sp = &vp->screen;
	register int qc;
	int ret;
	
#ifdef DEBUG_PRINT_ENTRY_DISABLED
DPRINTF("qvision_scroll_screen: screen=%d entry\n", vp->screen.screen);
#endif /* DEBUG_PRINT_ENTRY */

	qc = qvision_select_controller(vp);

	ret = vga_scroll_screen(closure, screen);

	qvision_restore_controller(qc);

	return(ret);
}

qvision_blitc(closure, sp, row, col, ch)
	caddr_t closure;
	register ws_screen_descriptor *sp;
	register int row, col, ch;
{
	register struct vga_info *vp = (struct vga_info *)closure;
	register int qc;
	int ret;

#ifdef DEBUG_PRINT_ENTRY_DISABLED
DPRINTF("qvision_blitc: screen=%d entry\n", vp->screen.screen);
#endif /* DEBUG_PRINT_ENTRY */

	qc = qvision_select_controller(vp);

	ret = vga_blitc(closure, sp, row, col, ch);

	qvision_restore_controller(qc);

	return(ret);
}

/*
 * Map/Unmap Screen
 */
int
qvision_map_unmap_screen(closure, depths, screen, mp)
	caddr_t closure;
	ws_depth_descriptor *depths;
	ws_screen_descriptor *screen;
	ws_map_control *mp;
{
	register struct vga_info *vp = (struct vga_info *)closure;
	register ws_depth_descriptor *dp;
	register int qc;
	caddr_t temp;
	int nbytes;

#ifdef DEBUG_PRINT_ENTRY
DPRINTF("qvision_map_unmap_screen: screen=%d entry\n", vp->screen.screen);
#endif /* DEBUG_PRINT_ENTRY */

	dp = depths + mp->which_depth;

	/* UNMAP not yet (if ever) implemented... */
        /* when this gets done, must find a way to save the depth */
        /* at which the screen was mapped, so it can be unmapped properly */
	if (mp->map_unmap == UNMAP_SCREEN)
		return (EINVAL);

	qc = qvision_select_controller(vp);

	/*
	 * only do the mapping for the *FIRST* fb; all others just get info
	 */
	if (qvision_mapped_state && u.u_procp->p_pid == qvision_mapped_pid) {
		dp->physaddr = (caddr_t) qvision_fb_phys;
		dp->pixmap = (caddr_t) qvision_fb_virt;
		dp->plane_mask = (caddr_t) qvision_io_virt;
		OUTB(0x3CE, 0x48); OUTB(0x3CF, (qvision_fb_phys >> 20) & 0xff);
		OUTB(0x3CE, 0x49); OUTB(0x3CF, (qvision_fb_phys >> 28) & 0x0f);
		qvision_restore_controller(qc);
		return(0);
	}
/*
FIXME - NOTE that the frame buffer address (physaddr) is "handled" using bytes;
this is so that no low-order bits are present in the swizzled address
that (eventually) gets passed into "ws_map_region" and thence into the VM
subsystem
*/
	/*
	 * NOTE: here we do one-time inits based on ECU info, like:
	 *
	 *	get highmap address
	 *	get interrupt status (see if board interrupt is enabled)
	 */
	if (qvision_fb_phys == NULL_HIGHMAP_BASE) {
		qvision_fb_phys = qvision_get_highmap_address(vp->unit);
		dp->physaddr = (caddr_t) qvision_fb_phys;
		qvision_set_interrupt_status(vp->unit); /* FIXME - location? */
	}

	temp = (caddr_t) get_io_handle(dp->physaddr, 1, BUS_MEMORY);
	temp = (caddr_t) PHYS_TO_KSEG(temp);

	/*
	 * swizzle the size of the frame buffer, to get the address space
	 * size which is appropriate for the machine we are running on
	 */
	/* FIXME FIXME - this may be specific to Jensen/EISA bus????? */
	/* FIXME FIXME - is this different for Morgan/PCI????? */
	nbytes = get_io_handle(HIGHMAP_SIZE, 1, BUS_MEMORY) -
		 get_io_handle(0x000000, 1, BUS_MEMORY);

        dp->pixmap = ws_map_region(temp, NULL, nbytes, 0600, (int *)NULL);

#ifdef DEBUG_PRINT_MAPPING
DPRINTF("qvision_map_unmap_screen: fb: nbytes 0x%x phys 0x%lx virt 0x%lx\n",
	nbytes, dp->physaddr, dp->pixmap);
#endif /* DEBUG_PRINT_MAPPING */

	if ( dp->pixmap == (caddr_t) NULL )
		return(ENOMEM);

	qvision_fb_virt = (vm_offset_t) dp->pixmap;

	/*
	 * register set is IOREGS_SIZE of IO space; swizzle as appropriate for
	 * the machine we are running on
	 */
	/* FIXME FIXME - this may be specific to Jensen/EISA bus????? */
	/* FIXME FIXME - is this different for Morgan/PCI????? */
	/* FIXME FIXME - For now, map the entire register space... :-( */
	nbytes = get_io_handle(IOREGS_SIZE, 1, BUS_MEMORY) -
		 get_io_handle(0x00000, 1, BUS_MEMORY);

	temp = (caddr_t) get_io_handle(dp->plane_mask_phys, 1, BUS_IO);
	temp = (caddr_t) PHYS_TO_KSEG(temp);

	dp->plane_mask = ws_map_region(temp, NULL, nbytes, 0600, (int *)NULL);

#ifdef DEBUG_PRINT_MAPPING
DPRINTF("qvision_map_unmap_screen: regs: nbytes 0x%x phys 0x%lx virt 0x%lx\n",
	nbytes, dp->plane_mask_phys, dp->plane_mask);
#endif /* DEBUG_PRINT_MAPPING */

	if (dp->plane_mask == (caddr_t)NULL)
		return(ENOMEM);

	qvision_io_virt = (vm_offset_t) dp->plane_mask;

	OUTB(0x3CE, 0x48); OUTB(0x3CF, (qvision_fb_phys >> 20) & 0xff);
	OUTB(0x3CE, 0x49); OUTB(0x3CF, (qvision_fb_phys >> 28) & 0x0f);

	qvision_mapped_state = 1;
	qvision_mapped_pid = u.u_procp->p_pid;

	qvision_restore_controller(qc);

	return (0);
}

void
qvision_close(closure)
	caddr_t closure;
{
	register struct vga_info *vp = (struct vga_info *)closure;
	register int qc;

#ifdef DEBUG_PRINT_ENTRY
DPRINTF("qvision_close: screen=%d entry\n", vp->screen.screen);
#endif /* DEBUG_PRINT_ENTRY */

	qc = qvision_select_controller(vp);

	vga_close(closure);

	if (qvision_console)
		/* force selection of "console" QVision board */
		qvision_select_controller(qvision_console);
	else
		qvision_restore_controller(qc);
}

/******************************************************************************
 * Cursor-related routines
 */

qvision_load_cursor(closure, screen, cursor, sync)
    caddr_t closure;
    ws_screen_descriptor *screen;
    ws_cursor_data *cursor;
{
	register struct vga_info *vp = (struct vga_info *)closure;

#ifdef DEBUG_PRINT_CURSOR
DPRINTF("qvision_load_cursor: screen=%d entry\n", vp->screen.screen);
#endif /* DEBUG_PRINT_CURSOR */

	vp->x_hot = cursor->x_hot;
	vp->y_hot = cursor->y_hot;

	qvision_set_cursor_position(closure, screen, screen->x, screen->y);

	qvision_reformat_cursor(vp, cursor);

	vp->dirty_cursor = 1;

	/*
	 * if vblank synchronization important...
	 */
	if (sync) {
		if (qvision_enable_interrupt(vp))
			return 0;
	}

	/*
	 * can't enable load at vblank or don't care, then just do it
	 */
	return (qvision_load_formatted_cursor(vp));
}

qvision_reformat_cursor(vp, cursor)
        register struct vga_info *vp;
        register ws_cursor_data *cursor;
{
        unsigned char *src, *dst;
        register int row, col, wid, hgt, xinc;

#ifdef DEBUG_PRINT_CURSOR_DISABLED
DPRINTF("qvision_reformat_cursor: entry\n");
#endif /* DEBUG_PRINT_CURSOR */

	dst = (unsigned char *) vp->bits;
        bzero(dst, 1024);

	wid = cursor->width;
	xinc = (wid <= 32) ? 0 : 4;
	if (wid > 32) wid = 32;
	hgt = cursor->height;
	if (hgt > 32) hgt = 32;

	/*
	 * NOTE: we fill the holding area with the following,
	 *  based on the fact that the cursor RAM is always
	 *  32 rows x 32 colums, and we need to load the whole
	 *  thing up when we do...
	 *
	 *  cursor data = 32 rows of 4 bytes each
	 *  cursor mask = 32 rows of 4 bytes each
	 *
	 * NOTE also that we've cleared out the holding area, so we
	 *  need to process only those bytes holding data/mask for
	 *  the selected cursor size...
	 */

	/* first the cursor data... */
	for (row = 0, src = (unsigned char *)cursor->cursor;
	     row < 32;
	     row++, src += xinc)
	{
		for (col = 0; col < 32; col += 8)
		{
			if ((col < wid) && (row < hgt))
			{
				*dst = reverseByte(*src);
			}
			dst++, src++;
		}
	}

	/* ...then the mask data */
	for (row = 0, src = (unsigned char *)cursor->mask;
	     row < 32;
	     row++, src += xinc)
	{
		for (col = 0; col < 32; col += 8)
		{
			if ((col < wid) && (row < hgt))
			{
				*dst = reverseByte(*src);
			}
			dst++, src++;
		}
	}
}


/*
 * given precomputed cursor, load it.
 */
qvision_load_formatted_cursor(vp)
	register struct vga_info *vp;
{
	register char *cbp = (char *) vp->bits;
	register int i;
	register int qc;

#ifdef DEBUG_PRINT_CURSOR_DISABLED
DPRINTF("qvision_load_formatted_cursor: screen=%d entry\n", vp->screen.screen);
#endif /* DEBUG_PRINT_CURSOR */

	qc = qvision_select_controller(vp);

	OUTB(0x3c8, 0x00);
	for (i = 0; i < 256; i++)
		OUTB(0x13c7, *cbp++);

	vp->dirty_cursor = 0;              /* no longer dirty */

	qvision_restore_controller(qc);

	return(0);
}

qvision_recolor_cursor(closure, screen, fg, bg)
	caddr_t closure;
	ws_screen_descriptor *screen;
	ws_color_cell *fg, *bg;
{
	register struct vga_info *vp = (struct vga_info *)closure;

#ifdef DEBUG_PRINT_CURSOR_DISABLED
DPRINTF("qvision_recolor_cursor: screen=%d entry\n", vp->screen.screen);
#endif /* DEBUG_PRINT_CURSOR */

	vp->cursor_fg = *fg;
	vp->cursor_bg = *bg;

	qvision_restore_cursor_color(closure);

	return(0);
}

qvision_restore_cursor_color(closure)
	caddr_t closure;
{
	register struct vga_info *vp = (struct vga_info *)closure;
	register int qc;

#ifdef DEBUG_PRINT_CURSOR_DISABLED
DPRINTF("qvision_restore_cursor_color: entry\n");
#endif /* DEBUG_PRINT_CURSOR */

	qc = qvision_select_controller(vp);

	OUTB(0x83c8, 0x00);
	OUTB(0x83c9, vp->cursor_fg.red >> 8);
	OUTB(0x83c9, vp->cursor_fg.green >> 8);
	OUTB(0x83c9, vp->cursor_fg.blue >> 8);

	OUTB(0x83c8, 0x01);
	OUTB(0x83c9, vp->cursor_bg.red >> 8);
	OUTB(0x83c9, vp->cursor_bg.green >> 8);
	OUTB(0x83c9, vp->cursor_bg.blue >> 8);

	OUTB(0x83c8, 0x02);
	OUTB(0x83c9, vp->cursor_fg.red >> 8);
	OUTB(0x83c9, vp->cursor_fg.green >> 8);
	OUTB(0x83c9, vp->cursor_fg.blue >> 8);

	OUTB(0x83c8, 0x03);
	OUTB(0x83c9, vp->cursor_bg.red >> 8);
	OUTB(0x83c9, vp->cursor_bg.green >> 8);
	OUTB(0x83c9, vp->cursor_bg.blue >> 8);

	qvision_restore_controller(qc);

	return(0);
}

qvision_set_cursor_position(closure, sp, x, y)
	caddr_t closure;
	ws_screen_descriptor *sp;
	register int x, y;
{
	register struct vga_info *vp = (struct vga_info *)closure;
	register int xt, yt;
	register int qc;

#ifdef DEBUG_PRINT_CURSOR_DISABLED
DPRINTF("qvision_set_cursor_position: screen=%d entry\n", vp->screen.screen);
#endif /* DEBUG_PRINT_CURSOR */

	qc = qvision_select_controller(vp);

	/*
	 * if mouse is NOT on, handle text mode cursor positioning
	 *  in the VGA generic code...
	 */
	if (vp->unit == 0 && !ws_is_mouse_on()) 
	{
		vga_set_cursor_position(closure, sp, x, y);
		qvision_restore_controller(qc);
		return(0);
	}
	

	/* bias position by where the hot spot is... */
	xt = x - vp->x_hot;
	yt = y - vp->y_hot;

	/*
	 * The following code keeps the cursor
	 *  POSITION up-to-date, even while the cursor is off,
	 *  so that code (qvision_cursor_on_off) which 
	 *  simply turns the cursor
	 *  on will find it at the desired spot.
	 */
	/*
	 * do Cursor X Register, then Cursor Y Register, word writes,
	 *  so that Y high is *LAST* set, which triggers hardware updating...
	 */
	OUTW(0x93c8, (xt + 32));
	OUTW(0x93c6, (yt + 32));

	qvision_restore_controller(qc);

	return(0);
}

qvision_cursor_on_off(closure, on_off)
	caddr_t closure;
	int on_off;
{
	register struct vga_info *vp = (struct vga_info *)closure;
	register int qc;
	int data;
	
#ifdef DEBUG_PRINT_CURSOR
DPRINTF("qvision_cursor_on_off: screen=%d %s entry\n", vp->screen.screen,
	(on_off==CURSOR_ON)?"ON":"OFF");
#endif /* DEBUG_PRINT_CURSOR */

	vp->cursor_on = on_off;

	if (on_off == CURSOR_ON && !ws_is_mouse_on())
		/* if ON and not graphics mode, ignore for now */
		return(0);

	qc = qvision_select_controller(vp);

	/*
	 * The following assumes that the cursor POSITION is kept
	 *  up-to-date (by qvision_cursor_set_position), so that
	 *  simply turning the cursor back on will find it at
	 *  the desired spot...
	 */
	if (on_off == CURSOR_ON)
	{
		/*
		 * We turn ON the cursor by setting the DAC Command Reg 2
		 *  Cursor Mode Select (bits 0-1) to three (3). This is
		 *  known as "X-Window's Cursor" in the docs...
		 */
		INB(0x13c9, data);
		OUTB(0x13c9, data | 3);
	}
	else
	{
		/*
		 * We turn OFF the cursor by setting the DAC Command Reg 2
		 *  Cursor Mode Select (bits 0-1) to zero (0).
		 */
		INB(0x13c9, data);
		OUTB(0x13c9, data & ~3);
	}
	
	qvision_restore_controller(qc);

	return(0);
}

/**************************************************************************
 *
 *     Colormap-related routines
 */

qvision_init_color_map(closure)
        caddr_t closure;
{
	register struct vga_info *vp = (struct vga_info *)closure;
	register unsigned char *cp = &((vgaHWPtr)vga_new_state)->DAC[0];
	register int i, qc;

#ifdef DEBUG_PRINT_ENTRY
DPRINTF("qvision_init_color_map: screen=%d entry\n", vp->screen.screen);
#endif /* DEBUG_PRINT_ENTRY */

	qc = qvision_select_controller(vp);

	OUTB(0x3C8, 0);

	/* NOTE: using 8-bit values for Triton mode */
	for (i = 0; i < 768; i++)
		OUTB(0x3c9, *cp++);

	qvision_restore_controller(qc);

	return(0);
}

qvision_load_color_map_entry(closure, map, entry)
        caddr_t closure;
        int map;                /* not used; only single map in this device */
        register ws_color_cell *entry;
{
	register struct vga_info *vp = (struct vga_info *)closure;
        register int index = entry->index;
        int s;
        
#ifdef DEBUG_PRINT_ENTRY_DISABLED
DPRINTF("qvision_load_color_map_entry: screen=%d entry\n", vp->screen.screen);
#endif /* DEBUG_PRINT_ENTRY */

        if (index >= 256 || index < 0) 
                return(-1);

        s = splbio();

        vp->cells[index].red   = entry->red   >> 8;
        vp->cells[index].green = entry->green >> 8;
        vp->cells[index].blue  = entry->blue  >> 8;
        vp->cells[index].dirty_cell = 1;

        if (index < vp->min_dirty)
		vp->min_dirty = index;
        if (index > vp->max_dirty)
		vp->max_dirty = index;

        /*
         * this just enables a vblank intr to load dirty color cells.
         * if vblank intr's aren't being used, do it now...
         */
        if (!vp->dirty_colormap) {
		vp->dirty_colormap = 1;
		if (!qvision_enable_interrupt(vp))
			qvision_clean_color_map(closure);
        }

        splx(s);

	return(0);
}

void
qvision_clean_color_map(closure)
        caddr_t closure;
{
	struct vga_info *vp = (struct vga_info *)closure;
	register struct vga_color_cell *entry = &vp->cells[vp->min_dirty];
	register int i, s, lasti = -2;
	register int qc;

#ifdef DEBUG_PRINT_ENTRY_DISABLED
DPRINTF("qvision_clean_color_map: screen=%d entry\n", vp->screen.screen);
#endif /* DEBUG_PRINT_COLOR */

	if (vp->dirty_colormap == 0) 
		return;

	/* no intrs when we're using */
	/* autoinc mode of vdac please! */
	s = splbio();

	qc = qvision_select_controller(vp);

	/*
	 * change the "dirty" entries...
	 */
	for (i = vp->min_dirty; i <= vp->max_dirty ; i++, entry++)
	{
		if (entry->dirty_cell)
		{
			if (i != (lasti + 1))
				OUTB(0x3C8, i);

			/* NOTE: using 8-bit values for Triton mode */
			OUTB(0x3c9, entry->red);
			OUTB(0x3c9, entry->green);
			OUTB(0x3c9, entry->blue);

			entry->dirty_cell = 0;
			lasti = i;
		}
	}

	/*
	 * reset to "clean" status
	 */
	vp->min_dirty = 256;
	vp->max_dirty = 0;
	vp->dirty_colormap = 0;

	qvision_restore_controller(qc);

	splx(s);
}

qvision_video_on(closure)
	caddr_t closure;
{
	struct vga_info *vp = (struct vga_info *)closure;
	register int qc;

#ifdef DEBUG_PRINT_VIDEO
DPRINTF("qvision_video_on: screen=%d entry\n", vp->screen.screen);
#endif /* DEBUG_PRINT_VIDEO */

	if (vp->screen_on != SCREEN_ON) 
	{
		qc = qvision_select_controller(vp);

		vp->screen_on = SCREEN_ON;
		vga_video_on(closure);

		qvision_restore_controller(qc);
	}

	qvision_cursor_on_off(closure, vp->cursor_on);

	return(0);
}

qvision_video_off(closure)
	caddr_t closure;
{
	struct vga_info *vp = (struct vga_info *)closure;
	register int qc;

#ifdef DEBUG_PRINT_VIDEO
DPRINTF("qvision_video_off: screen=%d entry\n", vp->screen.screen);
#endif /* DEBUG_PRINT_VIDEO */

	if (vp->screen_on == SCREEN_OFF)
		return(0);

	qc = qvision_select_controller(vp);

	vga_video_off(closure);
	vp->screen_on = SCREEN_OFF;

	qvision_restore_controller(qc);

	return(0);
}

vm_offset_t
qvision_get_highmap_address(unit)
    int unit;
{
    struct bus_mem memry;
    struct controller *ctlr = vgainfo[unit];
    int handle = 0;
    io_handle_t	mem_sysmap;
    
#ifdef DEBUG_PRINT_ECU
printf("qvision_get_highmap_address: unit 0x%x  ctlr 0x%lx  slot 0x%x\n",
       unit, (unsigned long) ctlr, ctlr->slot);
#endif /* DEBUG_PRINT_ECU */

    handle = eisa_get_config(ctlr, EISA_MEM, "VID;HMA", (void *)&memry, handle);

#ifdef DEBUG_PRINT_ECU
printf(" eisa_get_config returned (%d): 0x%x 0x%x 0x%x 0x%lx 0x%lx\n",
       handle, memry.isram, memry.decode, memry.unit_size, memry.size,
       memry.start_addr);
#endif /* DEBUG_PRINT_ECU */

    /*
     * check for old config file memory address or invalid size;
     *  if so, use the default highmap base address...
     */
    /*--------------------------------------------------------------*/
    /* Strip off sysmap from memry.start_addr so the compare works. */
    /* Fix for next base level.					    */
    /*--------------------------------------------------------------*/
    if (((memry.start_addr & 0x0ffffffffL) == (OLD_CONFIG_ADDR)) ||
	(memry.size != HIGHMAP_SIZE))
	    return HIGHMAP_BASE; /* use hardwired value */
    else
	    return memry.start_addr; /* use value from config file */
}

qvision_set_interrupt_status(unit)
    int unit;
{
    struct irq intr;
    struct controller *ctlr = vgainfo[unit];
    int handle = 0;
    
#ifdef DEBUG_PRINT_ECU
printf("qvision_set_interrupt_status: unit 0x%x  ctlr 0x%lx  slot 0x%x\n",
       unit, (unsigned long) ctlr, ctlr->slot);
#endif /* DEBUG_PRINT_ECU */

    handle = eisa_get_config(ctlr, EISA_IRQ, NULL, (void *)&intr, handle);

#ifdef DEBUG_PRINT_ECU
printf(" eisa_get_config returned (%d): 0x%x 0x%x 0x%x\n",
       handle, intr.channel, intr.trigger, intr.is_shared);
#endif /* DEBUG_PRINT_ECU */

    /*
     * check for interrupt not enabled
     *  if so, set flag to so indicate for later use
     */
    qvision_interrupt_possible = (intr.channel != EISA_NO_CONFIG);
}
