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
static char *rcsid = "@(#)$RCSfile: BusInterface.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:20:19 $";
#endif
/*
 * HISTORY
 */
#include <stdio.h>

#define PRINTWRITES 0
#define PRINTCYCLES 0
#define TGAMASTER_EN 

#ifdef PCI_BUS_INTERFACE
#include "macros.h"
#include "pct.h"
#include "pci.h"
#endif

#include "defs.h"
#include "types.h"
#include "vars.h"

#include "flv_structs.h"
#include "io_struct.h"
#include "vram.h"
#include "lyreTypes.h"
#include "MemBeh.h"
#include "Module.h"

extern unsigned char epromMem [ROMSIZE]; 
extern unsigned char paletteRAM [RAMDACS] [PALMASK + 1] [CHANNELS];

FILE *cmdFile;
char dmaBuff[2048] = {"0123456789 abcdefghijklmnop"};

extern void Deposit (Net_Entry *pnet, unsigned value);
extern int PCT_cmds;        /* batch file containst PCT specific cmds */

#ifdef NEW_BUS_INTERFACE
extern ROPREG         shadowRop;
extern MODEREG        shadowMode;
extern BWIDTHREG      shadowWidth;
extern DEEPREG        shadowDeep;
extern unsigned       shadowPlanemask;
extern unsigned       shadowVideoValid;
extern unsigned       shadowVbaseAddr;
extern CURSORXYREG    shadowCursorXY;
extern CURSORBASEREG  shadowCursorBase;
extern HORIZONTAL     shadowHcontrol;
extern VERTICAL       shadowVcontrol;
#else
ROPREG         shadowRop;
MODEREG        shadowMode;
BWIDTHREG      shadowWidth;
DEEPREG        shadowDeep;
unsigned       shadowPlanemask;
unsigned       shadowVideoValid;
unsigned       shadowVbaseAddr;
CURSORXYREG    shadowCursorXY;
CURSORBASEREG  shadowCursorBase;
HORIZONTAL     shadowHcontrol;
VERTICAL       shadowVcontrol;
#endif

unsigned long bogusDmaHigh32;	/* for software model only -
				   holds high 32-bits of DMA address */
SFBREG sfbreg;
SFBREG *sfbregptr = &sfbreg;	/* keeps dbx from getting confused */

int logFlag = 0;	/* don't write a log of commands executed */
FILE *cmdOut;		/* file which command scripts are written to */

int vramLine;
int cmdLine;

static int readFlag = 1;

void MakeIdle();
void MakeIdle1();

#ifndef PRESTO
extern struct io_struct io;
#endif

#ifdef PCI_BUS_INTERFACE
/* initialize the pct */
void init_pci() {

  pins_init();
  g_init();
  pct_init();

/*      PCT_CONFIG(# of param, #of masters, #of slaves, parity of masters,
**                      mode for slave 0, parity for slave 0, low_adrs for
**                      next slave, high address for next slave,
**                      mode for next slave, parity for next slave).
**
** this is from a mail message from Era
** (this is from PCI_VERIUSER.C by Don RGB::Low)
*/
    pct_config(13, 1, 2, 0, 1, 0, 0x0, 0x1ffff, 0, 0, 0x2000, 0x2fff, 0, 0);
				/* configure routine is called to activate
                                     13 parameters
				      1	one master,
				      2	two slaves,
				      0	master 1 has parity enabled (0),
				      1	slave 0 operates in default mode (1),
				      0	slave 0 has parity disable (0),
				      0	slave 1 low address is 0x0000,
				   1fff	slave 1 high address is 0x1fff (4K),
				      0	slave 1 operates in regular mode (0),
				      0	slave 1 has parity disabled (0)
				   2000	slave 2 low address is 0x2000,
				   2fff	slave 2 high address is 0x2fff (4K),
				      0	slave 2 operates in regular mode (0),
				      0	slave 2 has parity disabled (0)
				*/
  
  Deposit (&io._gntPIN, FALSE_L);
  Deposit (&io._framePIN, FALSE_L);
  Deposit (&io._irdyPIN, FALSE_L);

  /* fix initial startup state for tristate outputs (aren't enabled yet!) */
  Deposit (&io._trdyPIN, FALSE_L);
  Deposit (&io._stopPIN,  FALSE_L);
  Deposit (&io._devselPIN, FALSE_L);
  Deposit (&io.parPIN,  FALSE_L);
}

#endif

#ifndef PRESTO

int initSfb;
ChipReset(int mode)
{
  if (mode != 0) {
    initSfb = TRUE;
#ifdef NEW_BUS_INTERFACE
    BusReset(-1);
#elif PCI_BUS_INTERFACE
    Deposit (&io._resetTGA, TRUE_L);
#elif RSM
    Deposit (&io._reset, TRUE_L);
#else
    Deposit (&io._reset, TRUE_L);
    Deposit (&io._sel, FALSE_L);
    Deposit (&io._write, FALSE_L);
    Deposit (&io._ack, FALSE_L);
    Deposit (&io._err, FALSE_L);
#endif
  }
  else {
    initSfb = FALSE;
#ifdef NEW_BUS_INTERFACE
    BusReset(0);
#elif  PCI_BUS_INTERFACE
    Deposit (&io._resetTGA, FALSE_L);
#elif  RSM
    Deposit (&io._reset, FALSE_L);
#else
    Deposit (&io._reset, FALSE_L);
#endif
  }
}


ChipInit(int cycles)
{
  int i;

  Deposit (&io.Vdd, -1);
  Deposit (&io.Vss, 0);

  ChipReset(TRUE);

  for (i=0; i<cycles; ++i) {
    OneCycle( 1 );
    History( );
  }

  ChipReset(FALSE);

  for (i=0; i<cycles; ++i) {
    OneCycle( 0 );
    History( );
  }
}


void InitDacRam(void)
{
int i, j, dacNum;
/*
fprintf(stderr, "sfbTester: Initializing dac RAM w/ pseudorandom pattern..\n");
*/
  srandom(1);
  for(dacNum = 0; dacNum < RAMDACS; dacNum++)
    for(i=0; i<=PALMASK; i++) 
      for(j=0; j<CHANNELS; j++)
	paletteRAM [dacNum] [i] [j] = random() % 0xff;
}
void InitEeprom(void)
{
int i;
/*
fprintf(stderr, "sfbTester: Initializing EEPROM w/ pseudorandom pattern..\n");
*/
  srandom(1);
  for(i=0; i<ROMSIZE; i++) 
    epromMem [i] = random() & 0xff;
}

WrapperInit(void)
{
  init_io_struct ();

#ifdef PCI_BUS_INTERFACE
  init_pci();
#endif

  InitDacRam ();
  InitEeprom ();
  InitVRAM (RAM_TOTAL);
  DebugInit ();

  ChipInit(10);
}
#endif

#ifdef RSM

int interruptLine = 0;

/*
 * return true if the current operation involves big pixels
 * (4 bytes per pixel rather than 1 byte per pixel).
 *
 * this will be true on 32-plane systems for all visuals
 * except packed 8 bit.
 */
BigPixels(int writeReg)
{
  int copyRead = !writeReg &&
    (shadowMode.reg.mode == DMAWRITE
     || readFlag && shadowMode.reg.mode == COPY);

  if (copyRead) {
    return shadowMode.reg.visual != VISUAL8P;
  } else {
    return shadowRop.reg.visual  != VISUAL8P;
  }
}


void
BusCycle (unsigned ad, int sel_L, int wr_L)
{
  fprintf( stderr, "**** BusCycle command not implemented in this model!\n" );
}

/*
 * simulate a TURBOchannel write transaction.
 */
void
BusWriteBlock( unsigned addr, unsigned data[], unsigned mask )
{
  int i;
  void BusWrite(unsigned addr, unsigned data, unsigned mask);

  for (i=0; i<8; ++i)
    if (mask & (1<<i))
      BusWrite( (addr & ~0x1f) + 4*i, data[i], /* bytemask */ LWMASK );
}

unsigned BusRead( unsigned addr )
{
  if (addr & 3) {
    fprintf (stderr, "PANIC! unaligned address in BusRead, addr = %-.8x\n",
	     addr);
    exit (1);
  }

  /*
   * mask the address according to the
   * addrMask bits in the deep register.
   */
  addr &= (shadowDeep.reg.addrMask << 22) | 0x3fffff;

  if (addr < FIRSTREG_ADDRESS) {
    /*
     * must be a ROM access.
     */

    fprintf(stderr, "**** read from ROM, addr = %-.8x: not yet implemented\n", addr );
    exit (1);
  } else if (addr < FIRSTRAM_ADDRESS) {
    /*
     * must be a register access.
     * mask the address in order to provide aliases.
     * according to the spec, only the 10 low order bits are decoded.
     */
    addr &= 0x1003ff;

    switch( addr ) {
    case CPYBF0_ADDRESS:
    case CPYBF1_ADDRESS:
    case CPYBF2_ADDRESS:
    case CPYBF3_ADDRESS:
    case CPYBF4_ADDRESS:
    case CPYBF5_ADDRESS:
    case CPYBF6_ADDRESS:
    case CPYBF7_ADDRESS:
      /*
       * reads of copy buffer registers set readFlag
       * and assert rdCopyBuff of course.
       */
      MakeIdle();
      readFlag = 1;
      Deposit (&io.baddrIn, (addr & ~3));
      Deposit (&io.rdCopyBuff, -1);
      Tick();
      Tick();
      Tick();
      Tick();
      Deposit (&io.rdCopyBuff, 0);

      {
	unsigned data = addr & 4 ? io.brdData1.value : io.brdData0.value;

	if (logFlag)
	  fprintf (cmdOut, "0 %x %x\n", addr, data);

	return data;
      }

    case MODE_ADDRESS:
      return (shadowMode.u32);
      break;

    case BRES1_ADDRESS:
      return (sfbreg.bres1.u32);
      break;

    case BRES2_ADDRESS:
      return (sfbreg.bres2.u32);
      break;
      
    case BRES3_ADDRESS:
      return (sfbreg.bres3.u32);
      break;

    default:
      fprintf( stderr, "**** register read addr = %-.8x: not yet implemented\n",
	      addr );
      exit( 1 );
      break;
    }

  } else {
    /*
     * must be a frame buffer access.
     */
    ULONG64 data;

    MakeIdle();
    /*
     *  The decoded address is re-based to VRAMbase = 0 before being passed along.
     */
    if (addr < 0x400000)
      addr = addr & 0x1fffff;
    else if (addr < 0x800000)
      addr = addr - 0x400000;
    else if (addr < 0x1000000)
      addr = addr - 0x800000;
    else
      addr = addr - 0x1000000;

    {
      Visual visual;
      ULONG64 rd;

      visual.depth32      = shadowDeep.reg.deep;
      visual.unpacked8bit =(shadowMode.reg.visual == VISUAL8U);
      visual.bytePos      = shadowMode.reg.rotate;

      rd = ReadVRAM (addr >> 3, visual);
      if (shadowMode.reg.visual == VISUAL12) {
	data.lo = rd.lo & 0x000f0f0f;
	data.lo |= data.lo << 4;
	data.lo |= rd.lo & 0xff000000;

	data.hi = rd.hi & 0x000f0f0f;
	data.hi |= data.hi << 4;
	data.hi |= rd.hi & 0xff000000;
      } else if (shadowMode.reg.visual == VISUAL12HI) {
	data.lo = rd.lo & 0x00f0f0f0;
	data.lo |= data.lo >> 4;
	data.lo |= rd.lo & 0xff000000;

	data.hi = rd.hi & 0x00f0f0f0;
	data.hi |= data.hi >> 4;
	data.hi |= rd.hi & 0xff000000;
      } else {
	data.lo = rd.lo;
	data.hi = rd.hi;
      }
    }

    if (addr & 4)
      return data.hi;
    else
      return data.lo;
  }
}


void BusWrite(unsigned addr, unsigned data, unsigned mask)
{
  static newAddr = 1;
  static newError = 1;
  static persistent = 0;
  unsigned copy64 = 0;
  unsigned loadReg = 0;
  unsigned flush = 0;
  unsigned loadLoBuff = 0;
  unsigned loadHiBuff = 0;

  if (addr & 3) {
    int lo2 = addr & 3;
    int unusedMask = 0xf >> (-lo2 & 3);

    if (mask & unusedMask) {
      fprintf (stderr, "PANIC! unaligned address in BusWrite, addr = %-.8x\n",
	       addr);
      exit (1);
    }
  }

#if PRINTCYCLES
  printf( "BusWrite( addr = %-.8x, data = %-.8x, mask = %-.1x )\n",
	 addr, data, mask );
#endif

  if (logFlag)
    fprintf (cmdOut, "1 %x %x %x\n", addr, data, mask);

  /*
   * mask the address according to the
   * addrMask bits in the deep register.
   */
  addr &= (shadowDeep.reg.addrMask << 22) | 0x3fffff;

  if (addr < FIRSTREG_ADDRESS) {
    /*
     * must be a ROM access.
     */

    if (addr < 0x80000 && (addr & 4) == 0) {
      /* address register alias */
      fprintf (stderr, "**** write to address reg. alias in ROM space:\n");
      fprintf (stderr, "     not yet implemented, addr = %-.8x\n", addr);
      exit (1);
      newAddr = 1;
      sfbreg.address = (Pixel8 *) (data & 0xffffff);
      return;
    } else {
      /* continue register alias */
      fprintf (stderr, "**** write to continue reg. alias in ROM space:\n");
      fprintf (stderr, "     not yet implemented, addr = %-.8x\n", addr);
      exit (1);
      addr = BCONT_ADDRESS;
    }
  } else if (addr < FIRSTRAM_ADDRESS) {
    /*
     * must be a register access.
     * mask the address in order to provide aliases.
     * according to the spec, only the 10 low order bits are decoded.
     */
    addr &= 0x1003ff;

    switch( addr ) {

    case FG_ADDRESS:
      sfbreg.foreground.data = data;
      return;

    case BG_ADDRESS:
      sfbreg.background.data = data;
      return;

    case PLANEMASK_ADDRESS:
      loadReg = 1;
      if (shadowRop.reg.visual == VISUAL8U) {
	unsigned m = data & 0xff;
	m |= m << 8;
	m |= m << 16;
	if (data != m) {
	  fprintf (stderr, "WARNING! unsupported planemask value (%-.8x) for\n", data);
	  fprintf (stderr, "8-bit unpacked visual.  using %-.8x instead.\n", m);
	  data = m;
	}
      }
      Deposit (&io.bmask, -1);
      shadowPlanemask = data;
      break;

    case PIXMSK_ADDRESS:
      sfbreg.pixelMask = data;
      persistent = 0;
      return;

    case PERSISTENTPIXMASK:
      sfbreg.pixelMask = data;
      persistent = 1;
      return;

    case MODE_ADDRESS:
      shadowMode.u32 = data;
      sfbreg.mode.u32 = data;
      sfbreg.oldMode = data & 7;
      return;

    case STENCIL_ADDRESS:
      sfbreg.stencil.u32 = data;
      return;

    case ROP_ADDRESS:
      shadowRop.u32 = data;
      sfbreg.rop.u32 = data;
      return;

    case PIXSHFT_ADDRESS:
      sfbreg.shift = data;
      readFlag = 1;
      return;

    case ADDRREG_ALIAS:
    case ADDRREG_ADDRESS:
      newAddr = 1;
      sfbreg.address = (Pixel8 *) (data & 0xffffff);
      return;

    case BRES1_ADDRESS:
      sfbreg.bres1.u32 = data;
      return;

    case BRES2_ADDRESS:
      sfbreg.bres2.u32 = data;
      return; 

    case BRES3_ADDRESS:
      newError = 1;
      sfbreg.bres3.u32 = data; 
      return;

    case DEEP_ADDRESS:
      {
	extern FILE *vid;

	shadowDeep.u32 = data;
	if (vid && shadowDeep.reg.deep)
	  fprintf (vid, "l 24\nv 8\nd 7\nn 0\n");
	sfbreg.depth.u32 = data;
	return;
      }

    case DATAREG_ADDRESS:
      sfbreg.dataReg.data = data;
      return;

    case BRESWIDTH_ADDRESS:
      shadowWidth.u32 = data;
      sfbreg.bresWidth.u32 = data;
      return;

    case BOGUS_DMA_HIGH32:
#ifdef __alpha
      bogusDmaHigh32 = ((unsigned long)data) << 32;
#else
      bogusDmaHigh32 = 0;
#endif
      return;

    case DMABASE:
      /*
       * Dma Base Register uses the same storage
       * as the Z Base Address Register.
       */
      sfbreg.dmaBase = data; 
      return;

    case ZADDR_BASE_ADDRESS:
      sfbreg.zAddr.u32 = data;
      return;

    case ZINC_FRACT_ADDRESS:
      sfbreg.zIncLo.u32 = data;
      return;

    case ZINC_WHOLE_ADDRESS:
      sfbreg.zIncHi.u32 = data;
      return;

    case ZVAL_FRACT_ADDRESS:
      sfbreg.zValLo.u32 = data;
      return;

    case ZVAL_WHOLE_ADDRESS:
      sfbreg.zValHi.u32 = data;
      return;

    case CPYBF0_ADDRESS:
      flush = 1;
      readFlag = 0;
      /* fall thru */

    case CPYBF2_ADDRESS:
    case CPYBF4_ADDRESS:
    case CPYBF6_ADDRESS:
      loadLoBuff = 1;
      MakeIdle();
      break;

    case CPYBF1_ADDRESS:
    case CPYBF3_ADDRESS:
    case CPYBF5_ADDRESS:
    case CPYBF7_ADDRESS:
      loadHiBuff = 1;
      MakeIdle();
      break;

    case RINC_ADDRESS:
      sfbreg.redInc.data = data;
      return;

    case RVAL_ADDRESS:
      sfbreg.redVal.data = data;
      return;

    case GINC_ADDRESS:
      sfbreg.grnInc.data = data;
      return;

    case GVAL_ADDRESS:
      sfbreg.grnVal.data = data;
      return;

    case BINC_ADDRESS:
      sfbreg.bluInc.data = data;
      return;

    case BVAL_ADDRESS:
      sfbreg.bluVal.data = data;
      return;

    case SPANWIDTH:
      addr = SLOPE_111;
      /* now fall through */

    case SLOPE_000:
    case SLOPE_001:
    case SLOPE_010:
    case SLOPE_011:
    case SLOPE_100:
    case SLOPE_101:
    case SLOPE_110:
    case SLOPE_111:
    case SLOPEN_000:
    case SLOPEN_001:
    case SLOPEN_010:
    case SLOPEN_011:
    case SLOPEN_100:
    case SLOPEN_101:
    case SLOPEN_110:
    case SLOPEN_111:
      {
	REGS val;
	unsigned dxdy;
	unsigned dx, dy, dxGEdy, signdx, signdy;
	unsigned dmajor, dminor, signdmajor, signdminor, amajor, aminor;
	unsigned a1, a2;

	dxdy = (addr >> 2) & 7;
	Deposit (&io.bdxdy, dxdy);

	val.data = data;
	dx = val.dxdy.dx;
	dy = val.dxdy.dy;
	signdy = ((dxdy >> 0) & 1) == 0;
	signdx = ((dxdy >> 1) & 1) == 0;
	dxGEdy = (dxdy >> 2) & 1;

	dmajor = (dxGEdy ? dx : dy);
	dminor = (dxGEdy ? dy : dx);

	signdmajor = (dxGEdy ? signdx : signdy);
	signdminor = (dxGEdy ? signdy : signdx);

	sfbreg.bres3.reg.lineLength = dmajor + sfbreg.mode.reg.capEnds;
	sfbreg.bres1.reg.e1 = dminor;
	sfbreg.bres2.reg.e2 = dmajor + ~dminor + 1;
	
	if(sfbreg.mode.reg.ntLines == FALSE)
	  sfbreg.bres3.reg.e = ((dminor<<1) + ~dmajor +
				(signdmajor ? 0 : 1)
				) >> 1;
	else
	  sfbreg.bres3.reg.e = ((dminor<<1) + ~dmajor +
				(dxGEdy ? (signdy ? 0 : 1) : (signdx ? 1 : 0))
				) >> 1;

	{
	  unsigned zpixels = (BigPixels(loadReg) ? 1 : 4);

	  amajor = (dxGEdy ? zpixels : sfbreg.bresWidth.reg.zbuffer);
	  aminor = (dxGEdy ? sfbreg.bresWidth.reg.zbuffer : zpixels);
	}

	a1 = (signdmajor ? ~amajor : amajor) + (signdmajor ? 1 : 0);
	a2 = (signdminor ? ~aminor : aminor)
	  + (signdminor ? 1 : 0) + a1;

	DepositAfterIdle1 (&io.bza1, a1);
	DepositAfterIdle1 (&io.bza2, a2);

	amajor = (dxGEdy ? 1 : sfbreg.bresWidth.reg.drawable);
	aminor = (dxGEdy ? sfbreg.bresWidth.reg.drawable : 1);

	a1 = (signdmajor ? ~amajor : amajor) + (signdmajor ? 1 : 0);
	a2 = (signdminor ? ~aminor : aminor)
	  + (signdminor ? 1 : 0) + a1;

	sfbreg.bres1.reg.a1 = a1;
	sfbreg.bres2.reg.a2 = a2;

	newError = 1;
#if 0
	fprintf(stderr, "dxdy = %-.8x, gxy = %x\n", val.data, dxdy);
	fprintf(stderr, "bres1: a1 = %-.8x, e1 = %-.8x\n", sfbreg.bres1.bres1.a1, sfbreg.bres1.bres1.e1);
	fprintf(stderr, "bres2: a2 = %-.8x, e2 = %-.8x\n", sfbreg.bres2.bres2.a2, sfbreg.bres2.bres2.e2);
	fprintf(stderr, "bres3: e = %-.8x, len = %d\n", sfbreg.bres3.bres3.e, sfbreg.bres3.bres3.lineLength);
#endif
      }
      if (addr < SLOPE_000) return;
      /*
       * all the registers are setup for the line.
       * kludge this up to look like a write to the continue register.
       */
      addr = BCONT_ADDRESS;
      data = sfbreg.dataReg.data;
      break;

    case BCONT_ADDRESS:
    case START_ADDRESS:
      break;

    case BLOCKCOLOR0:
    case BLOCKCOLOR1:
    case BLOCKCOLOR2:
    case BLOCKCOLOR3:
    case BLOCKCOLOR4:
    case BLOCKCOLOR5:
    case BLOCKCOLOR6:
    case BLOCKCOLOR7:
      loadReg = 1;
      Deposit (&io.bcolor, -1);
      break;

    case COMMANDSTAT0:
    case COMMANDSTAT1:
      MakeIdle();
      return;

    case COPY64SRC:
    case COPY64SRC2:
    case COPY64SRC3:
    case COPY64SRC4:
      copy64 = 1;
      readFlag = 1;
      break;

    case COPY64DST:
    case COPY64DST2:
    case COPY64DST3:
    case COPY64DST4:
      copy64 = 1;
      readFlag = 0;
      break;

    case CURSORBASE:
      shadowCursorBase.u32 = data;
      return;

    case CURSORXY:
      shadowCursorXY.u32 = data;
      return;

    case HCONTROL:
      shadowHcontrol.u32 = data;
      return;

    case VCONTROL:
      shadowVcontrol.u32 = data;
      return;

    case VIDEOBASE:
      sfbreg.vBase.u32 = data;
      shadowVbaseAddr = data;
      return;

    case INTERRUPT:
      fprintf(stderr, "**** writes to INTERRUPT not yet implemented\n");
      return;

    case VIDEOSHIFT:
      fprintf(stderr, "**** writes to VIDEOSHIFT not yet implemented\n");
      return;

    case VIDEOVALID:
      shadowVideoValid = data;
      return;

    default:
      fprintf(stderr, "**** writes to 0x%-.8x not yet implemented\n", addr);
      return;
    }
  } else {
    /*
     * must be a frame buffer access.
     */

    /*
     *  The decoded address is re-based to VRAMbase = 0 before being passed along.
     */
    if (addr < 0x400000)
      addr = addr & 0x1fffff;
    else if (addr < 0x800000)
      addr = addr - 0x400000;
    else if (addr < 0x1000000)
      addr = addr - 0x800000;
    else
      addr = addr - 0x1000000;

    /* now add in 16M so we won't confuse framebuffer
       addresses with register addresses. */
    addr += 0x1000000;
  }

  if (addr == BCONT_ADDRESS || addr == START_ADDRESS) {
    if (BigPixels(loadReg))
      Deposit (&io.baddrIn, (unsigned) sfbreg.address >> 2);
    else
      Deposit (&io.baddrIn, (unsigned) sfbreg.address);
  } else {
    /*
     * in lineMode, pick up 2 lo order address bits from data field.
     */
    if (sfbreg.oldMode == OPAQUELINE || sfbreg.oldMode == TRANSPARENTLINE) {
      REGS d;

      d.data = data;
      if (BigPixels(loadReg))
	Deposit (&io.baddrIn, addr >> 2);
      else
	Deposit (&io.baddrIn, (addr & ~3) + (d.bstart.addrLo & 3));
    } else {
      if (BigPixels(loadReg))
	Deposit (&io.baddrIn, addr >> 2);
      else
	Deposit (&io.baddrIn, addr);
    }
  }

/*  printf( "BusWrite( addr = %-.8x, data = %-.8x, mask = %-.1x )\n",
	 io.baddrIn.value, data, mask );
  DumpRegisters();*/

  if (BigPixels(loadReg))
    DepositAfterIdle1 (&io.bzBase, sfbreg.zAddr.u32 >> 2);
  else
    DepositAfterIdle1 (&io.bzBase, sfbreg.zAddr.u32);
  DepositAfterIdle (&io.bdmaBase, sfbreg.dmaBase);
  DepositAfterIdle1 (&io.bstencilRef, sfbreg.zValHi.reg.stencil);
  DepositAfterIdle1 (&io.bzValHi, sfbreg.zValHi.reg.whole);
  DepositAfterIdle1 (&io.bzValLo, sfbreg.zValLo.reg.fract);
  DepositAfterIdle1 (&io.bzIncHi, sfbreg.zIncHi.reg.whole);
  DepositAfterIdle1 (&io.bzIncLo, sfbreg.zIncLo.reg.fract);

  DepositAfterIdle1 (&io.bredinc,   sfbreg.redInc.rgval.value); /* color vals*/
  DepositAfterIdle1 (&io.bredval,  ~sfbreg.redVal.rgval.value); /* low true  */
  DepositAfterIdle1 (&io.bgreeninc, sfbreg.grnInc.rgval.value); /* for speed */
  DepositAfterIdle1 (&io.bgreenval,~sfbreg.grnVal.rgval.value);
  DepositAfterIdle1 (&io.bblueinc,  sfbreg.bluInc.rgval.value);
  DepositAfterIdle1 (&io.bblueval, ~sfbreg.bluVal.rgval.value);
  DepositAfterIdle1 (&io.bbg, sfbreg.background.data);
  DepositAfterIdle1 (&io.bfg, sfbreg.foreground.data);
  DepositAfterIdle1 (&io.bbresa1, sfbreg.bres1.reg.a1);
  DepositAfterIdle1 (&io.bbresa2, sfbreg.bres2.reg.a2);

  Deposit (&io.brow, ~sfbreg.redVal.rgval.rowcol); /* dither row is low true */
  Deposit (&io.bcol, ~sfbreg.grnVal.rgval.rowcol); /* dither col is low true */
  Deposit (&io.bbrese, sfbreg.bres3.reg.e);
  Deposit (&io.blineLength, sfbreg.bres3.reg.lineLength);
  Deposit (&io.bbrese1, sfbreg.bres1.reg.e1);
  Deposit (&io.bbrese2, sfbreg.bres2.reg.e2);

  Deposit (&io.bdataReg, sfbreg.dataReg.data);
  Deposit (&io.bdataIn, data);
  Deposit (&io.bcbdataIn, data);
  Deposit (&io.bdepth, sfbreg.depth.reg.deep ? -1 : 0);

#if 0
  fprintf (stderr, "newaddr = %d, newerror = %d\n", newAddr, newError);
  fprintf (stderr, "addr = %-.8x, data = %-.8x, ", io.baddrIn.value, io.bdataIn.value);

  fprintf (stderr, "bres1 = %-.8x, bres2 = %-.8x, bres3 = %-.8x\n",
	   sfbreg.bres1.data, sfbreg.bres2.data, sfbreg.bres3.data);
#endif

  /*
   * setup the mode register and all its fields.
   * since this stuff all inits to 0, we have to make
   * sure that bsimpleMode is consistent.
   */
  if (sfbreg.mode.u32 != io.bmode.value
      || (io.bmode.value == 0 && io.bsimpleMode.value != -1)) {

    MakeIdle();
    Deposit (&io.bmode, sfbreg.mode.u32);
    Deposit (&io.bvisualSrc, sfbreg.mode.reg.visual);
    Deposit (&io.brotateSrc, sfbreg.mode.reg.rotate);
    Deposit (&io.blineMode,
	     (sfbreg.mode.reg.mode&7)==OPAQUELINE
	     || (sfbreg.mode.reg.mode&7)==TRANSPARENTLINE ? -1 : 0);
    Deposit (&io.bcopyMode, (sfbreg.mode.reg.mode)==COPY ? -1 : 0);
    Deposit (&io.bdmaRdMode, sfbreg.mode.reg.mode == DMAREAD ||
                             sfbreg.mode.reg.mode == DMAREADDITHERED ? -1 : 0);
    Deposit (&io.selCpuData, io.bdmaRdMode.value);
    Deposit (&io.bdmaWrMode, (sfbreg.mode.reg.mode)==DMAWRITE ? -1 : 0);
    Deposit (&io.bstippleMode,
	     (sfbreg.mode.reg.mode&7)==OPAQUESTIPPLE
	     || (sfbreg.mode.reg.mode&7)==TRANSPARENTSTIPPLE ? -1 : 0);
    Deposit (&io.bsimpleMode, (sfbreg.mode.reg.mode&7)==SIMPLE ? -1 : 0);
  }

  DepositAfterIdle1 (&io.bzOp, sfbreg.stencil.reg.zOp ? -1 : 0);
  DepositAfterIdle1 (&io.bzTest, sfbreg.stencil.reg.zTest);
  DepositAfterIdle1 (&io.bszPass, sfbreg.stencil.reg.zPass);
  DepositAfterIdle1 (&io.bzFail, sfbreg.stencil.reg.zFail);
  DepositAfterIdle1 (&io.bsFail, sfbreg.stencil.reg.sFail);
  DepositAfterIdle1 (&io.bsTest, sfbreg.stencil.reg.sTest);
  DepositAfterIdle1 (&io.sWrMask, sfbreg.stencil.reg.sWrMask);
  DepositAfterIdle1 (&io.sRdMask, sfbreg.stencil.reg.sRdMask);

  Deposit (&io.bpixelMask, sfbreg.pixelMask);

  if (sfbreg.rop.reg.rOp != io.brop.value) {
    MakeIdle();
    Deposit (&io.brop, sfbreg.rop.reg.rOp);
  }
  if (sfbreg.rop.reg.visual != io.bvisualDst.value) {
    MakeIdle();
    Deposit (&io.bvisualDst, sfbreg.rop.reg.visual);
  }
  if (sfbreg.rop.reg.rotate != io.brotateDst.value) {
    MakeIdle();
    Deposit (&io.brotateDst, sfbreg.rop.reg.rotate);
  }

  /*
   * the actual TURBOchannel mask is the one's complement
   * of the mask that we've been using.  (Good thing we
   * never fab'ed a chip with that screwed up.)  Invert
   * the mask here.
   */
  DepositAfterIdle1 (&io.btcMask, ~mask);

  if (sfbreg.shift != io.bpixelShift.value) {
    MakeIdle();
    Deposit (&io.bpixelShift, sfbreg.shift);
  }

  /*
   * setup the newAddr and newError flags.
   */
  if (!newAddr && addr == BCONT_ADDRESS) {
    Deposit (&io.newAddr, 0);
  } else {
    Deposit (&io.newAddr, -1);
  }

  if (!newError && addr == BCONT_ADDRESS) {
    Deposit (&io.newError, 0);
  } else {
    Deposit (&io.newError, -1);
  }

  if (flush /*|| readFlag*/) Deposit (&io.flush, -1);
  else Deposit (&io.flush, 0);

  Deposit (&io.readFlag0, (readFlag ? -1 : 0));
  Deposit (&io.bcopy64, (copy64 ? -1 : 0));

  /***
   *** we're done loading registers.
   *** start simulating and perform the operation.
   ***/
  if (loadHiBuff) {
    /***
     *** high half copy buffer writes.
     ***/
    Deposit (&io.selCpuData, -1);
    Deposit (&io.loadHiBuff, -1);
    Tick();
    Deposit (&io.selCpuData, 0);
    Deposit (&io.loadHiBuff, 0);
    Tick();
  } else if (loadLoBuff) {
    /***
     *** low half copy buffer writes.
     ***/
    Deposit (&io.selCpuData, -1);
    Deposit (&io.loadLoBuff, -1);
    Tick();
    Deposit (&io.selCpuData, 0);
    Deposit (&io.loadLoBuff, 0);
    Tick();
  } else if (loadReg) {
    /***
     *** color register/planemask register load.
     ***/
    PerformOperation();
    Deposit (&io.bcolor, 0);
    Deposit (&io.bmask, 0);
  } else {
    /***
     *** framebuffer operation.
     ***/

    if (sfbreg.mode.reg.mode == COPY)
      /*
       * invert readFlag in copy mode.
       */
      readFlag = (readFlag == 0);
    
    newAddr = 0;
    newError = 0;
    if (persistent == 0)
      sfbreg.pixelMask = -1;
    sfbreg.bres3.reg.lineLength = 0;
    
    if (sfbreg.mode.reg.mode == BLOCKFILL)
      BlockFillSetup();
    else if (sfbreg.mode.reg.mode == OPAQUEFILL)
      BlockFillSetup();
    else if (sfbreg.mode.reg.mode == TRANSPFILL)
      BlockFillSetup();
    else if (sfbreg.mode.reg.mode == TRANSPBLKSTIPPL)
      BlockStippleSetup();
    else if (sfbreg.mode.reg.mode == DMAREAD || 
	     sfbreg.mode.reg.mode == DMAREADDITHERED)
      DmaReadSetup();
    else if (sfbreg.mode.reg.mode == DMAWRITE)
      DmaWriteSetup();
    
    PerformOperation();
    Deposit (&io.bcmdlast, 0);
    Deposit (&io.bloadDmaRdData, 0);
  }
}


int ReadVirtual (int *psrc)
{
  extern FILE *vramFile;

  /* trap any unaligned addresses here */
  if ((int) psrc & 3) {
    fprintf (stderr, "PANIC! unaligned address in ReadVirtual (%lx)\n", psrc);
    exit (1);
  }

  /* we expect an int to be 32 bits (4 bytes). */
  if (sizeof(int) != 4) {
    fprintf (stderr, "PANIC! ReadVirtual assumption violated\n");
    exit (1);
  }

  if (vramFile == NULL) {
    int *p;

    if (cmdFile != NULL)
      p = (int *) (dmaBuff + ((unsigned long) psrc
			      - (unsigned long) sfbreg.dmaBase
			      - bogusDmaHigh32));
    else
      p = psrc;
    return (*p);
  } else {
    char *pbuf;
    unsigned addr, data;
    extern char *GetDmaLine();

    while ((pbuf=GetDmaLine()) && pbuf[0] == '\n')
      ;

    if (pbuf == NULL || pbuf[0] != 'r' || sscanf(pbuf, "r %x %x", &addr, &data) != 2) {
      fprintf (stderr, "expected dma read command: '%s'\n", pbuf);
      fprintf (stderr, "**** cmdLine = %d, vramLine = %d\n",
	       cmdLine, vramLine);
      exit (1);
    }

    if (addr != (int) psrc) { 
      fprintf (stderr, "**** incorrect dma read address: ");
      fprintf (stderr, "cmdLine = %d, vramLine = %d\n", cmdLine, vramLine);
      fprintf (stderr, "**** expected = %x, received =%x\n", addr, (int) psrc);
      exit (1);
    }
    return (data);
  }
  /* NOTREACHED */
}


DmaReadSetup()
{
  int i;
  DMACMD d;
  int rdData;
  int *virtualAddr;
  extern FILE *vramOut;

  d.u32 = io.bdataIn.value;
  virtualAddr = (int *) ((unsigned long) sfbreg.dmaBase | bogusDmaHigh32);
  Deposit (&io.newAddr, -1);
  Deposit (&io.bloadDmaRdData, -1);

  for (i=0; i<d.rdCmd.count; ++i) {
    rdData = ReadVirtual (virtualAddr);

    if (logFlag)
      fprintf (vramOut, "r %x %x\n", (int) virtualAddr, rdData);

    Deposit (&io.bcbdataIn, rdData);
    PerformOperation();
    Deposit (&io.newAddr, 0);
    ++virtualAddr;
  }
  rdData = ReadVirtual (virtualAddr);

  if (logFlag)
    fprintf (vramOut, "r %x %x\n", (int) virtualAddr, rdData);

  Deposit (&io.bcbdataIn, rdData);
  Deposit (&io.bcmdlast, -1);
  /*
   * the count field was actually count-1.
   * we'll get one more operation after the return.
   */
  return;
}

DmaWriteSetup()
{
  int i;
  DMACMD d;

  d.u32 = io.bdataIn.value;
  
  i = d.wrCmd.count;
  Deposit (&io.newAddr, -1);
  while (i > 0) {
    PerformOperation();
    Deposit (&io.newAddr, 0);
    --i;
  }

  Deposit (&io.bcmdlast, -1);
  /*
   * the count field was actually count-1.
   * we'll get one more operation after the return.
   */
  return;
}


#define ones ((unsigned) 0xffffffff)
#define SFBLEFTSTIPPLEMASK(align, ones)         ((ones) << (align))
#define SFBRIGHTSTIPPLEMASK(alignedWidth, ones) ((ones) >> (-(alignedWidth) & 0x1f))
      
BlockStippleSetup()
{
  unsigned a, d, align, dataAlign;
      
  a = io.baddrIn.value;
  d = io.bdataIn.value;
  if (a & 0x1f) {
    /*
     * address isn't 32-pixel aligned.
     * we must split it into two operations.
     */

    /* Rotate data to 4-pixel alignment, assuming that a is a pixel address */
    dataAlign = a & 0x1c;
    d = (d << dataAlign) | (d >> (32-dataAlign));
    align = a & 0x1f;

    Deposit (&io.baddrIn, a & ~0x1f);
    Deposit (&io.bdataIn, d & SFBLEFTSTIPPLEMASK(align, ones));
	
    if (align) {
      PerformOperation();
      Deposit (&io.baddrIn, 32 + a & ~0x1f);
      Deposit (&io.bdataIn, d & SFBRIGHTSTIPPLEMASK(align, ones));
    }
  }
}

BlockFillSetup()
{
  /*
   * in this mode, the data is interpreted as:
   *
   *     31         18  16 15     11 10               0
   *     +------------+---+---------+-----------------+
   *     |   ignore   |adr|  ignore |  Pixel count-1  |
   *     +------------+---+---------+-----------------+
   */
  unsigned a, align, leftMask, rightMask, dataAlign, data;
  int width;
  BLKFILLREG d;
  unsigned opaque = shadowMode.reg.mode == OPAQUEFILL;
      
  d.u32 = io.bdataIn.value;

  if (BigPixels(0))
    a = io.baddrIn.value;
  else
    a = (io.baddrIn.value & ~0x3) + d.reg.addrLo;
      
  /* Rotate data to 4-pixel alignment,
     assuming that a is a pixel address */
  dataAlign = a & 0x1c;
  data = sfbreg.dataReg.data;
  data = (data << dataAlign) | (data >> (32-dataAlign));

  width = d.reg.pixCount + 1;
  align = a & 0x1f;
      
#if 0
  printf ("*** blockFill: a = %-.8x, d = %-.8x\n",
	  io.baddrIn.value, io.bdataIn.value);
  printf ("               a = %-.8x, d = %-.8x\n",
	  a, d.u32);
  printf ("               width = %d, align = %d\n",
	  width, align);
  printf ("               pixCount = %d, addrLo = %d\n",
	  d.reg.pixCount, d.reg.addrLo);
#endif
      
  a -= align;
  width += align;
  leftMask = SFBLEFTSTIPPLEMASK(align, ones);
  rightMask = SFBRIGHTSTIPPLEMASK(width, ones);
      
  
  if (width <= 32) {
    /*
     * we can do it in a single operation.
     * just set up the address and data.
     */
    Deposit (&io.baddrIn, a);
    Deposit (&io.bdataIn, leftMask & rightMask & data);
    if (opaque) Deposit (&io.bpixelMask, leftMask & rightMask);
  } else {
    /*
     * will require multiple operations.
     */
    Deposit (&io.baddrIn, a);
    Deposit (&io.bdataIn, leftMask & data);
    if (opaque) Deposit (&io.bpixelMask, leftMask);
    PerformOperation();
    
    for (width -= 64;
	 width > 0;
	 width -= 32) {
      a += 32;
      Deposit (&io.baddrIn, a);
      Deposit (&io.bdataIn, ones & data);
      if (opaque) Deposit (&io.bpixelMask, ones);
      PerformOperation();
    }
    Deposit (&io.baddrIn, a+32);
    Deposit (&io.bdataIn, rightMask & data);
    if (opaque) Deposit (&io.bpixelMask, rightMask);
  }
}

PerformOperation()
{
  int i;

#if 0
  printf ("*** operation: a = %-.8x, d = %-.8x, cd = %-.8x\n",
	  io.baddrIn.value, io.bdataIn.value, io.bcbdataIn.value);
#endif

  Deposit (&io.breq0, -1);
  Tick();

#ifdef LONGTIMEOUT
#define TIMEOUT 900
#else
#define TIMEOUT 200
#endif

  i = 0;
  while( io.bi_busy0.value != 0) {
    Tick();
    if (++i > TIMEOUT) break;
  }

#if 1
  if( io.bi_busy0.value != 0) {
    printf( "BusWrite: request not finished processing\n");
  }
#endif

  Deposit (&io.breq0, 0);
}

void MakeIdle()
{
  int i;

  if (logFlag)
    fprintf (cmdOut, "\n");

  i = 0;
  while( io.bidle.value == 0 ) {
    Tick();
    if (++i > TIMEOUT) break;
  }

  if( io.bidle.value == 0) {
    fprintf(stderr, "MakeIdle: request not finished processing: ");
    fprintf (stderr, "cmdLine = %d, vramLine = %d\n",
	     cmdLine, vramLine);
  }
}


void MakeIdle1()
{
  int i;

  i = 0;
  while( io.bLockReg.value != 0 ) {
    Tick();
    if (++i > TIMEOUT) break;
  }

  if( io.bLockReg.value != 0 ) {
    fprintf(stderr, "MakeIdle1: request not finished processing: ");
    fprintf (stderr, "cmdLine = %d, vramLine = %d\n",
	     cmdLine, vramLine);
  }
}

DepositAfterIdle1 (Net_Entry *pnet, unsigned value)
{
  if (pnet->value != value) {
    MakeIdle1 ();
    Deposit (pnet, value);
  }
}

DepositAfterIdle (Net_Entry *pnet, unsigned value)
{
  if (pnet->value != value) {
    MakeIdle ();
    Deposit (pnet, value);
  }
}

#else
/* Non RSM bus interface */
#ifndef PCI_BUS_INTERFACE
#ifndef NEW_BUS_INTERFACE
/*
 * simulate one TURBOchannel bus cycle
 * given values for ad, ~sel, and ~wr.
 */
void
BusCycle (unsigned ad, int sel_L, int wr_L)
{
#if PRINTCYCLES
  IOADDR a;
  static int lastSel_L = -1;

  if (ad == 0xffffffff && sel_L == -1 && wr_L == -1) {
    printf ("****\n");
  } else if (sel_L == 0 && lastSel_L != 0) {
    a.un = ad;
    printf ("**** addr = %-.8x %-.1x %d %d\n",
	    a.io.addr<<2, a.io.mask, sel_L & 1, wr_L & 1);
  } else {
    printf ("**** data = %-.8x   %d %d\n", ad, sel_L & 1, wr_L & 1);
  }
  lastSel_L = sel_L;
#endif
  Deposit (&io.ad, ad);
  Deposit (&io._sel, sel_L);
  Deposit (&io._write, wr_L);

  Tick();
/*  fprintf (stderr, "**** %08x %d %d\n", ad, sel_L, wr_L); */
}


static unsigned unusedBits = -1;
#define TIMEOUT 1024
#define BUSTIMEOUT 1024

unsigned BusRead( unsigned addr )
{
  IOADDR a;
  extern FILE *fp;
  unsigned readData;
  int i;
  int conflicts;

  if (fp) fprintf (fp, "tcread %08x#16\n", addr);
  a.un = 0;
  a.io.addr = (addr + (unusedBits << 22)) >> 2;
  a.io.mask = 0;
  unusedBits = unusedBits * 3141592821 + (addr >> 2);
  conflicts = 0;

 CONFLICTED:
  /*
   * assert TURBOchannel sel.
   * send address for 1 tick.
   */
  BusCycle (a.un, /*~sel*/ TRUE_L, /*~write*/ FALSE_L);

  BusCycle (0xffffffff, TRUE_L, FALSE_L);
  if (io._rdy.value == TRUE_L) {
    fprintf (stderr, "**** ~rdy returned too soon on a read.\n");
    fprintf (stderr, "cmdLine = %d, vramLine = %d\n",
	     cmdLine, vramLine);
  }

  /*
   * wait for data; keep ticking until rdy asserts.
   */
  i = 0;
  do {
    BusCycle (0xffffffff, TRUE_L, FALSE_L);
    if (++i >= BUSTIMEOUT) {
      fprintf( stderr, "**** BusRead timed out waiting for ~rdy.\n" );
      fprintf (stderr, "cmdLine = %d, vramLine = %d\n",
	       cmdLine, vramLine);
      exit (1);
    }
  } while (io._rdy.value == FALSE_L && io._conflict.value == FALSE_L);

  if (io._conflict.value == TRUE_L) {
    if (io._rdy.value != TRUE_L)
      fprintf( stderr, "**** ~conflict asserted without ~rdy\n");

    /*
     * deassert sel for one tick.
     */
    BusCycle (0xffffffff, FALSE_L, FALSE_L);

    if (++conflicts >= BUSTIMEOUT) {
      fprintf( stderr, "**** BusRead: exceeded conflict threshold.\n" );
      exit (1);
    }

    goto CONFLICTED;
  }

  if (io._rdy.value != TRUE_L)
    fprintf( stderr, "**** BusRead: unexpected value for ~rdy = %-.8x\n",
	    io._rdy.value );

  readData = io.ad.value;

  /*
   * then deassert sel for one tick.
   */

  BusCycle (0xffffffff, FALSE_L, FALSE_L);

/*
  fprintf (stderr, "**** BusRead( %-.8x ) = %-.8x\n", addr, readData);
*/

  if (logFlag)
    fprintf (cmdOut, "0 %x %x\n", addr, readData);

  return (readData);
}


/*
 * simulate a TURBOchannel write transaction.
 */
void
BusWrite( unsigned addr, unsigned data, unsigned mask )
{
  extern FILE *fp;
  IOADDR a;
  int i;
  int conflicts;

#if PRINTWRITES
  printf( "BusWrite( addr = %-.8x, data = %-.8x, mask = %-.1x )\n",
	 addr, data, mask );
#endif

  if (logFlag)
    fprintf (cmdOut, "1 %x %x %x\n", addr, data, mask);

  if (fp) fprintf (fp, "tcwrite %08x#16;%08x#16\n", addr, data);

  /*
   * implement our shadow registers.
   */
  if (addr == DEEP_ADDRESS)
    shadowDeep.u32 = data;
  else if (addr == ROP_ADDRESS)
    shadowRop.u32 = data;
  else if (addr == MODE_ADDRESS)
    shadowMode.u32 = data;
  else if (addr == BRESWIDTH_ADDRESS)
    shadowWidth.u32 = data;
  else if (addr == VIDEOBASE)
    shadowVbaseAddr = data;
  else if (addr == VIDEOVALID)
    shadowVideoValid = data;
  else if (addr == PLANEMASK_ADDRESS)
    shadowPlanemask = data;
  else if (addr == CURSORXY)
    shadowCursorXY.u32 = data;
  else if (addr == CURSORBASE)
    shadowCursorBase.u32 = data;
  else if (addr == VCONTROL) {
#if 0
    VERTICAL v;
    v.u32 = data;
    printf ("VCONTROL: active=%d, fp=%d, sync=%d, bp=%d\n",
	    v.reg.active, v.reg.fp, v.reg.sync, v.reg.bp);
#endif
    shadowVcontrol.u32 = data;
  }
  else if (addr == HCONTROL) {
#if 0
    HORIZONTAL h;
    h.u32 = data;
    printf ("HCONTROL: active=%d, fp=%d, sync=%d, bp=%d\n",
	    h.reg.active, h.reg.fp, h.reg.sync, h.reg.bp);
#endif
    shadowHcontrol.u32 = data;
  }

  a.un = 0;
  a.io.addr = (addr + (unusedBits << 22)) >> 2;
  /*
   * the actual TURBOchannel mask is the one's complement
   * of the mask that we've been using.  (Good thing we
   * never fab'ed a chip with that screwed up.)  Invert
   * the mask here.
   */
  a.io.mask = ~mask;
  unusedBits = unusedBits * 3141592821 + (addr >> 2);
  conflicts = 0;

 CONFLICTED:
  /*
   * assert TURBOchannel sel and wr.
   * send address for 1 tick.
   */
  BusCycle (a.un, TRUE_L, TRUE_L);

  /*
   * send data and keep ticking until rdy asserts.
   */

  i = 0;
  do {
    BusCycle (data, TRUE_L, TRUE_L);
    if (++i >= BUSTIMEOUT) {
      fprintf( stderr, "**** BusWrite timed out waiting for ~rdy.\n" );
      fprintf (stderr, "cmdLine = %d, vramLine = %d\n",
	       cmdLine, vramLine);
      exit (1);
    }
  } while (io._rdy.value == FALSE_L && io._conflict.value == FALSE_L);

  if (io._conflict.value == TRUE_L) {
    if (io._rdy.value != TRUE_L)
      fprintf( stderr, "**** ~conflict asserted without ~rdy\n");

    /*
     * deassert sel for one tick.
     */
    BusCycle (0xffffffff, FALSE_L, FALSE_L);

    if (++conflicts >= BUSTIMEOUT) {
      fprintf( stderr, "**** BusWrite: exceeded conflict threshold.\n" );
      exit (1);
    }

    goto CONFLICTED;
  }

#ifdef CMD_PARSER_ONLY
  ExpectNext((a.io.addr << 2), data, a.io.mask);
#endif

  if (io._rdy.value != TRUE_L)
    fprintf( stderr, "**** BusWrite: unexpected value for ~rdy = %-.8x\n",
	    io._rdy.value );

  /*
   * then deassert sel and wr for one tick.
   */
  BusCycle (0xffffffff, FALSE_L, FALSE_L);
}


/*
 * simulate a TURBOchannel write transaction.
 */
void
BusWriteBlock( unsigned addr, unsigned data[], unsigned mask )
{
  extern FILE *fp;
  IOADDR a;
  int i, bit;
  int conflicts;
  unsigned blkAddr;

  for (bit=0; bit<8; ++bit)
    if (mask & (1<<bit))
      break;

#if PRINTWRITES
  printf( "BusWriteBlock( addr = %-.8x, data = %-.8x",
	 (addr & ~0x1f) + (bit<<2), data[bit] );
#endif

  if (logFlag)
    fprintf (cmdOut, "1 %x %x f\n", (addr & ~0x1f) + (bit<<2), data[bit]);

  if (fp) fprintf (fp, "tcwrite %08x#16;%08x#16\n", (addr & ~0x1f) + (bit<<2), data[bit]);

  a.un = 0;
  a.io.addr = ((addr & ~0x1f) + (bit<<2) + (unusedBits << 22)) >> 2;
  a.io.mask = 0;
  unusedBits = unusedBits * 3141592821 + (addr >> 2);
  conflicts = 0;

 CONFLICTED:
  /*
   * assert TURBOchannel sel and wr.
   * send address for 1 tick.
   */
  BusCycle (a.un, TRUE_L, TRUE_L);

  /*
   * send data and keep ticking until rdy asserts.
   */

  i = 0;
  do {
    BusCycle (data[bit], TRUE_L, TRUE_L);
    if (++i >= BUSTIMEOUT) {
      fprintf( stderr, "**** BusWriteBlock timed out waiting for ~rdy.\n" );
      fprintf (stderr, "cmdLine = %d, vramLine = %d\n",
	       cmdLine, vramLine);
      exit (1);
    }
  } while (io._rdy.value == FALSE_L && io._conflict.value == FALSE_L);

  if (io._conflict.value == TRUE_L) {
    if (io._rdy.value != TRUE_L)
      fprintf( stderr, "**** ~conflict asserted without ~rdy\n");

    /*
     * deassert sel for one tick.
     */
    BusCycle (0xffffffff, FALSE_L, FALSE_L);

    if (++conflicts >= BUSTIMEOUT) {
      fprintf( stderr, "**** BusWriteBlock: exceeded conflict threshold.\n" );
      exit (1);
    }

    goto CONFLICTED;
  }

#ifdef CMD_PARSER_ONLY
  ExpectNext((a.io.addr << 2), data[bit], a.io.mask);
  blkAddr = (a.io.addr << 2)+4;
#endif

  if (io._rdy.value != TRUE_L)
    fprintf( stderr, "**** BusWriteBlock: unexpected value for ~rdy = %-.8x\n",
	    io._rdy.value );

  /*
   * During a block write there's one dead
   * cycle between ~rdy and subsequent data.
   */
  BusCycle (data[bit], TRUE_L, TRUE_L);

  /*
   * Send the remaining data.
   */
  ++bit;
  while (mask & (1<<bit)) {
#if PRINTWRITES
    printf( ", %-.8x", data[bit] );
#endif

    if (logFlag)
      fprintf (cmdOut, "1 %x %x f\n", (addr & ~0x1f) + (bit<<2), data[bit]);

    if (fp) fprintf (fp, "tcwrite %08x#16;%08x#16\n", (addr & ~0x1f) + (bit<<2), data[bit]);

    BusCycle (data[bit], TRUE_L, TRUE_L);

#ifdef CMD_PARSER_ONLY
    ExpectNext(blkAddr, data[bit], a.io.mask);
    blkAddr += 4;
#endif

    ++bit;
  }

  /*
   * then deassert sel and wr for one tick.
   */
  BusCycle (0xffffffff, FALSE_L, FALSE_L);

#if PRINTWRITES
    printf( "\n" );
#endif
}
#endif

#ifndef PRESTO

#ifdef NEW_BUS_INTERFACE
#define TIMEOUT 1024
#endif

void MakeIdle()
{
  int i;

  if (logFlag)
    fprintf (cmdOut, "\n");

  i = 0;
  while( (i++ < TIMEOUT) && BusRead(COMMANDSTAT0) ) ;

  if (i >= TIMEOUT) {
    fprintf(stderr, "MakeIdle: request not finished processing: ");
    fprintf (stderr, "cmdLine = %d, vramLine = %d\n",
	     cmdLine, vramLine);
  }
}
#endif
#else


/* PCI specific bus cycles RRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR */

/* watch bus status */
void bus_mon()
{
  int idselLocal;
  idselLocal = ((p->ad & 0x10000) ? FALSE_L : TRUE_L);
  if (idselLocal) idselLocal = 1;

  if (p->frame_l == 0) 
     {
      printf("-------------------------------------------------------------------- \n");
     }
  printf(
"Src Count Clk Rst    ad    cbe Frame Devsel Trdy Irdy Req Gnt Stop Lock Par Perr idsel \n");
  printf("PCT%5d %3x %3x %8x %3x %5x %6x %4x %4x %3x %3x %4x %4x %3x %4x %3x\n",
  pct->global_timer, p->clk, p->rst, p->ad, p->c_be_l, p->frame_l, p->devsel_l, p->trdy_l,
  p->irdy_l, p->req_l, p->gnt_l, p->stop_l, p->lock_l, p->par, p->err_l,idselLocal);
/*
  printf("TGA%5x %3x %3x %8x %3x %5x %6x %4x %4x %3x %3x %4x %4x %3x %4x\n",
  pct->global_timer, p->clk,
     (io._reset.value ? 1 : 0 ),
      io.ad.value,
     ((io._cbe[3].value ? 1 : 0)*8 + (io._cbe[2].value ? 1 : 0)*4 +
      (io._cbe[1].value ? 1 : 0)*2 + (io._cbe[0].value ? 1 : 0))
     (io._frame.value  ? 1 : 0),

p->rst, p->ad, p->c_be_l, p->frame_l, p->devsel_l, p->trdy_l,
*/
}

/*
 * simulate one PCI bus cycle
 * Do one pct clock, and one simulation tick
 */
void
BusCycle (int tick_en)
{
#if PRINTCYCLE
#endif

  int i, ad_temp, temp;

  /* first set PCT pins for inputs and bidirectionals to be the values
     which were passed into this shell.
  */

  p->frame_l  = (io._framePIN.value  ? 1 : 0);
  p->irdy_l   = (io._irdyPIN.value   ? 1 : 0);
  p->req_l    = (io._reqPIN.value    ? 1 : 0);  /* This is tga's req signal */
  p->c_be_l   = (io._cbePIN[3].value ? 1 : 0)*8 + (io._cbePIN[2].value ? 1 : 0)*4 +
                (io._cbePIN[1].value ? 1 : 0)*2 + (io._cbePIN[0].value ? 1 : 0);
  p->trdy_l   = (io._trdyPIN.value   ? 1 : 0);
  p->stop_l   = (io._stopPIN.value   ? 1 : 0);
  p->devsel_l = (io._devselPIN.value ? 1 : 0);
  p->par      = (io.parPIN.value     ? 1 : 0);

  ad_temp = 0;
  for (i = 0; i < 32; i++) { /* loop to pull in the ad value to pct */
    ad_temp = (ad_temp + ((io.adPIN[i].value ? 1 : 0) << i));
  }
  p->ad = ad_temp;

/*  p->rst      = (io._resetIO.value  ? 1 : 0);
*/

  /* Now call the actual PCT routine
  ** Execute it once with clk low to sample the pins, and again with clk
  ** high to drive the outputs.
  ** Note: This should only happen to the rising edge of the clock
  */
  /************************************************************/

    /* do a fake GDAL clock cycle */
    p->clkout = 0;         /* input clock */
    g->phi1 = 1;           /* makes PCI clock low */
    g->phi2 = 0;
    g->phi3 = 0;
    g->phi4 = 0;
    pct_main();            /* call PCT */

    check_model_flags();   /* check and clear bidirectinal enables */

    p->clkout = 1;         /* input clock */
    g->phi1 = 1;           /* makes PCI clock high */
    g->phi2 = 0;
    g->phi3 = 0;
    g->phi4 = 0;
    pct_main();            /* call PCT */

 /************************************************************/

  /* Now copy states of output and bidirectional into argument
     variables so the simulator can access the updated values.
     These are all bidirectionals.
  */



  if (pct->mast[1].frame_l_enable)
       {
      Deposit (&io._framePIN, (p->frame_l ? FALSE_L : TRUE_L) );
      }



  if (pct->mast[1].irdy_l_enable)
     Deposit (&io._irdyPIN, (p->irdy_l ? FALSE_L : TRUE_L) );


  if (pct->slave[0].trdy_l_enable || pct->slave[1].trdy_l_enable) {
    printf("PCT asserts trdy\n");
    Deposit (&io._trdyPIN, (p->trdy_l ? FALSE_L : TRUE_L) );
  }

  if (pct->slave[0].stop_l_enable || pct->slave[1].stop_l_enable) {
    Deposit (&io._stopPIN, (p->stop_l ? FALSE_L : TRUE_L) );
    printf("PCT enables stop\n");
  }
  if (pct->slave[0].devsel_l_enable || pct->slave[1].devsel_l_enable) {
    printf("PCT is driving devsel\n");
    Deposit (&io._devselPIN, (p->devsel_l ? FALSE_L : TRUE_L) );
  }

  if (pct->mast[1].par_enable ||
      pct->slave[0].par_enable || pct->slave[1].par_enable)
    Deposit (&io.parPIN, (p->par ? FALSE_L : TRUE_L) );

  if (pct->mast[1].ad_enable ||
        pct->slave[0].ad_enable || pct->slave[1].ad_enable) 
    {
     /* loop added to deposit each bit of ad */
     for (i = 0; i < 32; i++) 
        { 
         Deposit (&io.adPIN[i], (((p->ad >> i) & 0x00000001) ? -1 : 0) );
        }
     Deposit (&io.idselPIN, ((p->ad & 0x10000) ? FALSE_L : TRUE_L) );
  }

  if (pct->mast[1].c_be_l_enable) {
    Deposit (&io._cbePIN[0], (( (0x01 & p->c_be_l) )     ? FALSE_L : TRUE_L) );
    Deposit (&io._cbePIN[1], (( (0x02 & p->c_be_l) >> 1) ? FALSE_L : TRUE_L) );
    Deposit (&io._cbePIN[2], (( (0x04 & p->c_be_l) >> 2) ? FALSE_L : TRUE_L) );
    Deposit (&io._cbePIN[3], (( (0x08 & p->c_be_l) >> 3) ? FALSE_L : TRUE_L) );
  }

  /* gnt_l is the grant signal from arbitratior, it goes to tga
     gnt_l is just an output, not a bidirectional.
  */
    Deposit (&io._gntPIN, (p->gnt_l ? FALSE_L : TRUE_L) );

    /* bus_mon();  */

  if (tick_en)
   {
    Tick();                /* do a simulation clock */
   }
}
/*** timeout should be longer than devtimeout *****/
#define TIMEOUT 1024
#define DEVTIMEOUT 64


/**********************************************************/
unsigned BusIORead( unsigned addr )
{
  extern FILE *fp;
  unsigned readData;
  int i, j, ad_temp1;
  int devtmo = 0;

 if (fp) fprintf (fp, "PCIIOread %08x#16\n", addr);

  /* issue a io_read PCI transaction */
  if (PCT_cmds == 0) 
    {
     pct_command(6, 1, addr, 2, 0, 1, 0);
    }

  /* prime PCT to get Frame asserted */
     BusCycle(!p->gnt_l || io.dmaBusy.value);
     BusCycle(!p->gnt_l || io.dmaBusy.value);

  /* wait until both irdy and trdy assert, else ready or devsel timeout */
  for (i = 0; i < TIMEOUT; i++) {
    BusCycle (1);
    if ((io._trdyPIN.value == TRUE_L) && 
        (io._irdyPIN.value == TRUE_L) && 
        (!io.dmaBusy.value)) {
      BusCycle(1);         /* to finish PCT transaction */
      break;
    }
    if ( (i >= DEVTIMEOUT) && (io._devselPIN.value == FALSE_L) ) {
      devtmo = 1;
      break;
    }
  }

  if (i == TIMEOUT ) {
    printf("*** command timeout **** \n"); 
    fprintf( stderr, "**** BusIORead timed out waiting for (t|i)rdy.\n" );
    fprintf (stderr, "cmdLine = %d, vramLine = %d\n",
	     cmdLine, vramLine);
#ifdef DOEXIT
    exit (1);
#endif
  }

  if (devtmo == 1 ) {
    printf("*** bus select timeout **** \n"); 
    fprintf( stderr, "**** BusIORead device select timed out.\n" );
    fprintf (stderr, "cmdLine = %d, vramLine = %d\n",
	     cmdLine, vramLine);
#ifdef DOEXIT
    exit (1);
#endif
  }

/*  put in a loop here to assemble a 32-bit int from io.ad */
  ad_temp1 = 0;
  for (j = 0; j < 32; j++) { /* loop to pull in the ad value to pct */
    ad_temp1 = (ad_temp1 + ((io.adPIN[j].value ? 1 : 0) << j));
  }

  readData = ad_temp1;
  return (readData);
}


/**********************************************************/
unsigned BusRead( unsigned addr )
{
  extern FILE *fp;
  unsigned readData;
  int i, j, ad_temp1;
  int devtmo = 0;

 if (fp) fprintf (fp, "PCIread %08x#16\n", addr);

  /* issue a mem_read PCI transaction */
  if (PCT_cmds == 0) 
    {
     pct_command(6, 1, addr, MEMRD, 0, 1, 0);
    }
  
  /* prime PCT to get Frame asserted */
     BusCycle(!p->gnt_l || io.dmaBusy.value);
     BusCycle(!p->gnt_l || io.dmaBusy.value);

  /* wait until both irdy and trdy assert, else ready or devsel timeout */
  for (i = 0; i < TIMEOUT; i++) {
    BusCycle (1);
    if ((io._trdyPIN.value == TRUE_L) && 
        (io._irdyPIN.value == TRUE_L) && 
        (!io.dmaBusy.value)) {
      BusCycle(1);         /* to finish PCT transaction */
      break;
    }
    if ( (i >= DEVTIMEOUT) && (io._devselPIN.value == FALSE_L) ) {
      devtmo = 1;
      break;
    }
  }

  if (i == TIMEOUT ) {
    printf("*** command timeout **** \n"); 
    fprintf( stderr, "**** BusRead timed out waiting for (t|i)rdy.\n" );
    fprintf (stderr, "cmdLine = %d, vramLine = %d\n",
	     cmdLine, vramLine);
#ifdef DOEXIT
    exit (1);
#endif
  }

  if (devtmo == 1 ) {
    printf("*** bus select timeout **** \n"); 
    fprintf( stderr, "**** BusRead device select timed out.\n" );
    fprintf (stderr, "cmdLine = %d, vramLine = %d\n",
	     cmdLine, vramLine);
#ifdef DOEXIT
    exit (1);
#endif
  }

/*  put in a loop here to assemble a 32-bit int from io.ad */
  ad_temp1 = 0;
  for (j = 0; j < 32; j++) { /* loop to pull in the ad value to pct */
    ad_temp1 = (ad_temp1 + ((io.adPIN[j].value ? 1 : 0) << j));
  }

  readData = ad_temp1;
  return (readData);

}


/**********************************************************/
/*
 * simulate a PCI write transaction.
 */
void
BusWrite( unsigned addr, unsigned data, unsigned mask )
{
  extern FILE *fp;
  int i;
  int devtmo = 0;

  if (fp) fprintf (fp, "PCIwrite %08x#16;%08x#16\n", addr, data);

  /*
   * implement our shadow registers.
   */
  if (addr == DEEP_ADDRESS)
    shadowDeep.u32 = data;
  else if (addr == ROP_ADDRESS)
    shadowRop.u32 = data;
  else if (addr == MODE_ADDRESS)
    shadowMode.u32 = data;
  else if (addr == BRESWIDTH_ADDRESS)
    shadowWidth.u32 = data;
  else if (addr == VIDEOBASE)
    shadowVbaseAddr = data;
  else if (addr == VIDEOVALID)
    shadowVideoValid = data;
  else if (addr == PLANEMASK_ADDRESS)
    shadowPlanemask = data;
  else if (addr == CURSORXY)
    shadowCursorXY.u32 = data;
  else if (addr == CURSORBASE)
    shadowCursorBase.u32 = data;
  else if (addr == VCONTROL) {
#if 0
    VERTICAL v;
    v.u32 = data;
    printf ("VCONTROL: active=%d, fp=%d, sync=%d, bp=%d\n",
	    v.reg.active, v.reg.fp, v.reg.sync, v.reg.bp);
#endif
    shadowVcontrol.u32 = data;
  }
  else if (addr == HCONTROL) {
#if 0
    HORIZONTAL h;
    h.u32 = data;
    printf ("HCONTROL: active=%d, fp=%d, sync=%d, bp=%d\n",
	    h.reg.active, h.reg.fp, h.reg.sync, h.reg.bp);
#endif
    shadowHcontrol.u32 = data;
  }

#if PRINTWRITES
  printf( "BusWrite( addr = %-.8x, data = %-.8x, mask = %-.1x )\n",
	 addr, data, mask );
#endif

  /* issue a mem_write PCI transaction */
  /* if not a PCT parsed command     */
  if (PCT_cmds == 0) 
    {
     pct_command(6, 1, addr, MEMWR, 0, 1, 0);
     pct_setdata(3, 1, 0, data);
     pct_setbyte(3, 1, 0, ~mask);      /* TC masks are inverted from PCI */
    }

  /* prime PCT to get Frame asserted */
  BusCycle(!p->gnt_l || io.dmaBusy.value);
  BusCycle(!p->gnt_l || io.dmaBusy.value);

 /* wait until both irdy and trdy assert, else ready or devsel timeout */
  for (i = 0; i < TIMEOUT; i++) {
    BusCycle (1);
    if ((io._trdyPIN.value == TRUE_L) && 
        (io._irdyPIN.value == TRUE_L) && 
        (!io.dmaBusy.value)) {
      BusCycle(!p->gnt_l || io.dmaBusy.value);         /* to finish PCT transaction */
      break;
    }
    if ( (i >= DEVTIMEOUT) && (io._devselPIN.value == FALSE_L) ) {
      devtmo = 1;
      break;
    }
  }
  if (i == TIMEOUT ) {
    printf("*** command timeout **** \n"); 
    fprintf( stderr, "**** BusWrite timed out waiting for (t|i)rdy.\n" );
    fprintf (stderr, "cmdLine = %d, vramLine = %d\n",
	     cmdLine, vramLine);
#ifdef DOEXIT
    exit (1);
#endif
  }

  if (devtmo == 1 ) {
    printf("*** bus select timeout **** \n"); 
    fprintf( stderr, "**** BusWrite device select timed out.\n" );
    fprintf (stderr, "cmdLine = %d, vramLine = %d\n",
	     cmdLine, vramLine);
#ifdef DOEXIT
    exit (1);
#endif
  }

}

/**********************************************************/
/*
 * simulate a PCI write transaction.
 */
void
BusIOWrite( unsigned addr, unsigned data, unsigned mask )
{
  extern FILE *fp;
  int i;
  int devtmo = 0;

  if (fp) fprintf (fp, "PCIIOwrite %08x#16;%08x#16\n", addr, data);

  /*
   * implement our shadow registers.
   */
  if (addr == DEEP_ADDRESS)
    shadowDeep.u32 = data;
  else if (addr == ROP_ADDRESS)
    shadowRop.u32 = data;
  else if (addr == MODE_ADDRESS)
    shadowMode.u32 = data;
  else if (addr == BRESWIDTH_ADDRESS)
    shadowWidth.u32 = data;
  else if (addr == VIDEOBASE)
    shadowVbaseAddr = data;
  else if (addr == VIDEOVALID)
    shadowVideoValid = data;
  else if (addr == PLANEMASK_ADDRESS)
    shadowPlanemask = data;
  else if (addr == CURSORXY)
    shadowCursorXY.u32 = data;
  else if (addr == CURSORBASE)
    shadowCursorBase.u32 = data;
  else if (addr == VCONTROL) {
#if 0
    VERTICAL v;
    v.u32 = data;
    printf ("VCONTROL: active=%d, fp=%d, sync=%d, bp=%d\n",
	    v.reg.active, v.reg.fp, v.reg.sync, v.reg.bp);
#endif
    shadowVcontrol.u32 = data;
  }
  else if (addr == HCONTROL) {
#if 0
    HORIZONTAL h;
    h.u32 = data;
    printf ("HCONTROL: active=%d, fp=%d, sync=%d, bp=%d\n",
	    h.reg.active, h.reg.fp, h.reg.sync, h.reg.bp);
#endif
    shadowHcontrol.u32 = data;
  }

#if PRINTWRITES
  printf( "BusIOWrite( addr = %-.8x, data = %-.8x, mask = %-.1x )\n",
	 addr, data, mask );
#endif

  /* issue a mem_write PCI transaction */
  /* if not a PCT parsed command     */
  if (PCT_cmds == 0) 
    {
     pct_command(6, 1, addr, 3, 0, 1, 0);
     pct_setdata(3, 1, 0, data);
     pct_setbyte(3, 1, 0, ~mask);      /* TC masks are inverted from PCI */
    }

  /* prime PCT to get Frame asserted */
  BusCycle(!p->gnt_l || io.dmaBusy.value);
  BusCycle(!p->gnt_l || io.dmaBusy.value);

 /* wait until both irdy and trdy assert, else ready or devsel timeout */
  for (i = 0; i < TIMEOUT; i++) {
    BusCycle (1);
    if ((io._trdyPIN.value == TRUE_L) && 
        (io._irdyPIN.value == TRUE_L) && 
        (!io.dmaBusy.value)) {
      BusCycle(!p->gnt_l || io.dmaBusy.value);         /* to finish PCT transaction */
      break;
    }
    if ( (i >= DEVTIMEOUT) && (io._devselPIN.value == FALSE_L) ) {
      devtmo = 1;
      break;
    }
  }
  if (i == TIMEOUT ) {
    printf("*** command timeout **** \n"); 
    fprintf( stderr, "**** BusIOWrite timed out waiting for (t|i)rdy.\n" );
    fprintf (stderr, "cmdLine = %d, vramLine = %d\n",
	     cmdLine, vramLine);
#ifdef DOEXIT
    exit (1);
#endif
  }

  if (devtmo == 1 ) {
    printf("*** bus select timeout **** \n"); 
    fprintf( stderr, "**** BusIOWrite device select timed out.\n" );
    fprintf (stderr, "cmdLine = %d, vramLine = %d\n",
	     cmdLine, vramLine);
#ifdef DOEXIT
    exit (1);
#endif
  }

}


/*
 * simulate a PCI block write transaction.
 */
void
BusWriteBlock( unsigned addr, unsigned data[], unsigned mask[], int count)
{
  extern FILE *fp;
  int i, bit;
  int devtmo = 0;

  if (fp) fprintf (fp, "PCIwriteBlock %08x#16;%08x#16\n", addr, data);

#if PRINTWRITES
  printf( "BusWriteBlock: addr = %-.8x, ", addr );
#endif

/* Fill the pct structures first */

  /* if not a PCT parsed command     */
  if (PCT_cmds == 0) 
    {
      pct_command(6, 1, addr, MEMWR, 0, count, 0);        /* set address */

      for (i = 1; i <= count; i++) 
          {
            pct_setdata(3, 1, i, data[i-1]);
            pct_setbyte(3, 1, i, ~mask[i-1]);      /* TC masks are inverted from PCI */
           }
    }

  /* prime PCT to get Frame asserted */

  BusCycle(!p->gnt_l || io.dmaBusy.value);
  BusCycle(!p->gnt_l || io.dmaBusy.value);

  /* now count off data transfers */
  for (bit = 0; bit < count; bit++) {
#if PRINTWRITES
    printf( ", %-.8x", data[bit] );
#endif
  /* wait until both irdy and trdy assert, else ready or devsel timeout */
    for (i = 0; i < TIMEOUT; i++) {
      BusCycle (1);
    if ((io._trdyPIN.value == TRUE_L) && 
        (io._irdyPIN.value == TRUE_L) && 
        (!io.dmaBusy.value)) {
        break;
      }
      if ( (i >= DEVTIMEOUT) && (io._devselPIN.value == FALSE_L) ) {
        devtmo = 1;
        break;
      }
    }

    /* see if xfer went ok */
    if (i == TIMEOUT ) {
    printf("*** command timeout **** \n"); 
      fprintf( stderr, "**** BusWriteBlock timed out waiting for (t|i)rdy.\n" );
      fprintf (stderr, "cmdLine = %d, vramLine = %d\n",
	       cmdLine, vramLine);
#ifdef DOEXIT
      exit (1);
#endif
    }
    if (devtmo == 1 ) {
    printf("*** bus select timeout **** \n"); 
      fprintf( stderr, "**** BusWriteBlock device select timed out.\n" );
      fprintf (stderr, "cmdLine = %d, vramLine = %d\n",
	       cmdLine, vramLine);
#ifdef DOEXIT
      exit (1);
#endif
    }
  }

  BusCycle(!p->gnt_l || io.dmaBusy.value);         /* for next PCT transaction */

#if PRINTWRITES
    printf( "\n" );
#endif
}
/*
* Simulate a PCI configuration read cycle
*/
unsigned PCIConfigRead( unsigned addr )
{
  extern FILE *fp;
  unsigned readData;
  int i, j, ad_temp2;
  int devtmo = 0;
 if (fp) fprintf (fp, "PCIConfigread %08x#16\n", addr);

  /* issue a config read PCI transaction */
  /* if not a PCT parsed command     */
  if (PCT_cmds == 0) 
          pct_command(6, 1, addr, CNFRD, 0, 1, 0);

  /* prime PCT to get Frame asserted */
  BusCycle(!p->gnt_l || io.dmaBusy.value);
  BusCycle(!p->gnt_l || io.dmaBusy.value);

  /* wait until both irdy and trdy assert, else ready or devsel timeout */
  for (i = 0; i < TIMEOUT; i++) {
    BusCycle (1);
    if ((io._trdyPIN.value == TRUE_L) && 
        (io._irdyPIN.value == TRUE_L) && 
        (!io.dmaBusy.value)) {
      BusCycle(1);         /* to finish PCT transaction */
      break;
    }
    if ( (i >= DEVTIMEOUT) && (io._devselPIN.value == FALSE_L) ) {
      devtmo = 1;
      break;
    }
  }

  if (i == TIMEOUT ) {
    printf("*** command timeout **** \n"); 
    fprintf( stderr, "**** PCIConfigRead timed out waiting for (t|i)rdy.\n" );
    fprintf (stderr, "cmdLine = %d, vramLine = %d\n",
	     cmdLine, vramLine);
#ifdef DOEXIT
    exit (1);
#endif
  }

  if (devtmo == 1 ) {
    printf("*** bus select timeout **** \n"); 
    fprintf( stderr, "**** PCIConfigRead device select timed out.\n" );
    fprintf (stderr, "cmdLine = %d, vramLine = %d\n",
	     cmdLine, vramLine);
#ifdef DOEXIT
    exit (1);
#endif
  }

/*  put in a loop here to assemble a 32-bit int from io.ad */
  ad_temp2 = 0;
  for (j = 0; j < 32; j++) { /* loop to pull in the ad value to pct */
    ad_temp2 = (ad_temp2 + ((io.adPIN[j].value ? 1 : 0) << j));
  }

  readData = ad_temp2;
  return (readData);
}


/*
 * simulate a PCI configuration write transaction.
 */
void
PCIConfigWrite( unsigned addr, unsigned data, unsigned mask )
{
  extern FILE *fp;
  int i;
  int devtmo = 0;


  if (fp) fprintf (fp, "PCIConfigwrite %08x#16;%08x#16\n", addr, data);

  /*
   * implement our shadow registers.
   */
  if (addr == DEEP_ADDRESS)
    shadowDeep.u32 = data;
  else if (addr == ROP_ADDRESS)
    shadowRop.u32 = data;
  else if (addr == MODE_ADDRESS)
    shadowMode.u32 = data;
  else if (addr == BRESWIDTH_ADDRESS)
    shadowWidth.u32 = data;
  else if (addr == VIDEOBASE)
    shadowVbaseAddr = data;
  else if (addr == VIDEOVALID)
    shadowVideoValid = data;
  else if (addr == PLANEMASK_ADDRESS)
    shadowPlanemask = data;
  else if (addr == CURSORXY)
    shadowCursorXY.u32 = data;
  else if (addr == CURSORBASE)
    shadowCursorBase.u32 = data;
  else if (addr == VCONTROL) {
#if 0
    VERTICAL v;
    v.u32 = data;
    printf ("VCONTROL: active=%d, fp=%d, sync=%d, bp=%d\n",
	    v.reg.active, v.reg.fp, v.reg.sync, v.reg.bp);
#endif
    shadowVcontrol.u32 = data;
  }
  else if (addr == HCONTROL) {
#if 0
    HORIZONTAL h;
    h.u32 = data;
    printf ("HCONTROL: active=%d, fp=%d, sync=%d, bp=%d\n",
	    h.reg.active, h.reg.fp, h.reg.sync, h.reg.bp);
#endif
    shadowHcontrol.u32 = data;
  }

#if PRINTWRITES 
  printf( "PCIConfigWrite( addr = %-.8x, data = %-.8x, mask = %-.4x )\n",
	 addr, data, mask );
#endif 

  /* issue a config_write PCI transaction */
  /* if not a PCT parsed command     */
  if (PCT_cmds == 0) 
    {
     pct_command(6, 1, addr, CNFWR, 0, 1, 0);
     pct_setdata(3, 1, 0, data);
     pct_setbyte(3, 1, 0, ~mask);      /* TC masks are inverted from PCI */
    }

  /* prime PCT to get Frame asserted */
  BusCycle(!p->gnt_l || io.dmaBusy.value);
  BusCycle(!p->gnt_l || io.dmaBusy.value);


 /* wait until both irdy and trdy assert, else ready or devsel timeout */
  for (i = 0; i < TIMEOUT; i++) {
    BusCycle (1);
    if ((io._trdyPIN.value == TRUE_L) && 
        (io._irdyPIN.value == TRUE_L) && 
        (!io.dmaBusy.value)) {
      BusCycle(!p->gnt_l || io.dmaBusy.value);         /* to finish PCT transaction */
      break;
    }
    if ( (i >= DEVTIMEOUT) && (io._devselPIN.value == FALSE_L) ) {
      devtmo = 1;
      break;
    }
  }
  if (i == TIMEOUT ) {
    printf("*** command timeout **** \n"); 
    fprintf( stderr, "**** PCIConfigWrite timed out waiting for (t|i)rdy.\n" );
    fprintf (stderr, "cmdLine = %d, vramLine = %d\n",
	     cmdLine, vramLine);
#ifdef DOEXIT
    exit (1);
#endif
  }

  if (devtmo == 1 ) {
    printf("*** bus select timeout **** \n"); 
    fprintf( stderr, "**** PCIConfigWrite device select timed out.\n" );
    fprintf (stderr, "cmdLine = %d, vramLine = %d\n",
	     cmdLine, vramLine);
#ifdef DOEXIT
    exit (1);
#endif
  }
}

#ifndef PRESTO

void MakeIdle()
{
  int i;

  if (logFlag)
    fprintf (cmdOut, "\n");

  i = 0;
  while( io.statusReg.value != 0 || pct->mast_empty == 0) 
    {
     BusCycle(1);
     if (++i > TIMEOUT) break;
    }

  if( io.statusReg.value != 0) 
    {
     fprintf(stderr, "MakeIdle: request not finished processing: ");
     fprintf (stderr, "cmdLine = %d, vramLine = %d\n",
    	     cmdLine, vramLine);
    }
}
#endif
#endif /* end of PCI specific bus cycles */

#endif

