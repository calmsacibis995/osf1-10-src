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
typedef union {
  unsigned un;
  struct {
    unsigned unused : 1;
    unsigned mask : 4;
    unsigned addr : 27;
  } io;
} IOADDR;

typedef union {
  struct {
    /*
     * register load signals.
     */
    unsigned ldMode;
    unsigned ldPixelShift;
    unsigned ldPixelMask;
    unsigned ldPlnmsk;
    unsigned ldBoolop;
    unsigned ldAddrReg;
    unsigned useAddrReg;
    unsigned ldDeep;
    unsigned ldBres1;
    unsigned ldBres2;
    unsigned ldFore;
    unsigned ldBack;
    unsigned ldCopyBuffHi;
    unsigned ldCopyBuffLo;
    unsigned rdCopyBuff;
    unsigned setMask0;	/* set PixelMask to all 1's */
    unsigned setMask;	/* set PixelMask to all 1's */

    unsigned ldBresErr;	/* load output of Bresenham adder */
    unsigned ldSfbAddr;	/* load output of address adder */
    unsigned selAddr;	/* select registered or incoming address */
    unsigned stepBres;	/* adder inputs for Bresenham step */
    unsigned flush;		/* flush (empty) copy buffer */
    unsigned shift;		/* shift/load register in copy buffer */
    unsigned wait;		/* memory wait */
    unsigned pixshift;		/* pixel shift value */

    unsigned tcRdy;	/* tcint: TURBOchannel ready */
    unsigned wrRdy;		/* synchronous ready for write cycles */
    unsigned rdRdy;		/* synchronous ready for read cycles */
    unsigned driveTC;	/* drive read data onto TC */

    unsigned q3;		/* FSM state bit */
    unsigned q2;		/* FSM state bit */
    unsigned q1;		/* FSM state bit */
    unsigned q0;		/* FSM state bit */
    unsigned setReadFlag;	/* set control for readFlag */
    unsigned resetReadFlag;	/* reset control for readFlag */
    unsigned busy;	/* first iteration */
    unsigned d1busy;	/* first iteration, delayed 1 tick */

    unsigned next;		/* next iteration */

    unsigned readRq;		/* memory read request */
    unsigned rq;		/* vram cycle request */

    unsigned enadr;		/* tcint: enable address reg */
    unsigned endat;		/* tcint: enable data reg */
    unsigned tc0;		/* tcint FSM state bit */
    unsigned tc1;		/* tcint FSM state bit */
    unsigned tc2;		/* tcint FSM state bit */
    unsigned tc3;		/* tcint FSM state bit */
    unsigned tcreq;		/* tcint command request pending */
    unsigned selBuff;		/* tcint: select buffered cmd */

    unsigned mc0;		/* mc FSM state bit */
    unsigned mc1;		/* mc FSM state bit */
    unsigned mc2;		/* mc FSM state bit */
    unsigned mc3;		/* mc FSM state bit */
    unsigned memidle;		/* mc idle */
  } sig;
  unsigned rec;
} SYNCCTRL;

typedef union {
  struct {
    unsigned rowMatch;	/* next VRAM row same as last */
    unsigned i_busy;	/* tcint: from sfb machine, immediate busy */
    unsigned i_enadr;	/* tcint: enable address reg */
    unsigned i_enhld;	/* tcint: enable incoming reg */
    unsigned i_selBuff;	/* tcint: select buffered cmd */
  } sig;
  unsigned rec;
} COMBCTRL;

typedef union {
  unsigned data;
  unsigned char byte[4];
} COLORS;

typedef struct {
  unsigned lo;
  unsigned hi;
} ULONG64;

typedef union {
  unsigned char b[8];
  unsigned short p16[4];
  unsigned long	p32[2];
  ULONG64 l;
} BYTES;


ULONG64 do_rams();
ULONG64 StippleData ();
ULONG64 Buffer ();
ULONG64 BoolOp ();
unsigned BusRead ();
unsigned Datapath ();
void BusWrite ();
