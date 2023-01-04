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
typedef enum {
    /* 000 */ SIMPLE,
    /* 001 */ OPAQUESTIPPLE,
    /* 010 */ OPAQUELINE,
    /* 011 */ unusedmode0,
    /* 100 */ unusedmode1,
    /* 101 */ TRANSPARENTSTIPPLE,
    /* 110 */ TRANSPARENTLINE,
    /* 111 */ COPY
} SFBMode;

typedef enum {
    /* 00 */ DEPTH8,
    /* 01 */ DEPTH16,
    /* 10 */ DEPTH32,
    /* 11 */ unused
} SFBDepth;

#define TRANSPMASK 04

typedef enum { FALSE, TRUE } Bool;

typedef enum {
  /* graphics functions, from <X11/X.h> */
  GXclear,		/* 0 */
  GXand,		/* src AND dst */
  GXandReverse,		/* src AND NOT dst */
  GXcopy,		/* src */
  GXandInverted,	/* NOT src AND dst */
  GXnoop,		/* dst */
  GXxor,		/* src XOR dst */
  GXor,			/* src OR dst */
  GXnor,		/* NOT src AND NOT dst */
  GXequiv,		/* NOT src XOR dst */
  GXinvert,		/* NOT dst */
  GXorReverse,		/* src OR NOT dst */
  GXcopyInverted,	/* NOT src */
  GXorInverted,		/* NOT src OR dst */
  GXnand,		/* NOT src OR NOT dst */
  GXset			/* 1 */
} GXop;

typedef struct {
  unsigned  e1 : 16;	/* e1 is always positive */
  int	  a1 : 16;	/* address/error inc if e < 0 */
} BRES1;

typedef struct {
  unsigned e2 : 16;	/* e2 is positive (it's negated when used) */
  int	 a2 : 16; 	/* address/error inc if e > 0 */
} BRES2;

typedef struct {
  unsigned lineLength : 4;	/* line length count */
  unsigned ignored : 11;
  int e : 17;		/* e is sign-extended */
} BRES3;

typedef int Pixel32;
typedef int Bits32;
typedef char Pixel8;

typedef struct {
    SFBDepth	    depth;	    /* 8, 16, or 32 plane depth		    */
    COLORS	    foreground;
    COLORS	    background;
    GXop	    rop;	    /* X graphics function		    */
    unsigned	    planemask;	    /* X planemask			    */
    SFBMode	    mode;	    /* Which mode?			    */
    Bits32	    pixelMask;      /* Pixel mask register		    */
    Pixel8	    *address;       /* Pixel address register		    */
    Bits32	    more;	    /* Continuation data		    */
    BRES1	    bres1;	    /* a1, e1				    */
    BRES2	    bres2;	    /* a2, e2				    */
    BRES3	    bres3;	    /* e, count				    */
    unsigned	    shift;	    /* -7..+7 shift for copies		    */
} SFBREG;

SFBREG sfbreg;

/*
 * the following is a pseudo-register.
 * no storage actually exists for it.
 */
typedef struct { 
	unsigned linedata : 16;
	unsigned addrLo : 2;
} BSTART;

typedef union {
  unsigned data;
  BRES1 bres1;
  BRES2 bres2;
  BRES3 bres3;
  BSTART bstart;
} REGS;

unsigned readFlag;	/*bit set when you do a read for copy mode,*/
			/*used to keep track of what to do with the*/
			/*8 longword buffer, read or write*/
/*
 * edge triggered state devices w/ next states.
 */
COMBCTRL	actl;

SYNCCTRL	ctl, next_ctl;
BYTES		buf[2], next_buf[2];
ULONG64		dstData, next_dstData;
ULONG64		memdata, next_memdata;
int		bresError, next_bresError;
unsigned	sfbAddr, next_sfbAddr;
unsigned	nextBmask, next_nextBmask;
unsigned	wrnext, next_wrnext;
unsigned	wrptr, next_wrptr;
unsigned	readFlag, next_readFlag;
unsigned	memaddr, next_memaddr;
unsigned	membm, next_membm;
unsigned	addrIn, next_addrIn;
unsigned	writeIn, next_writeIn;
unsigned	dataIn, next_dataIn;
unsigned	bmIn, next_bmIn;
unsigned	hldAD, next_hldAD;
unsigned	hldWr, next_hldWr;
unsigned	buffA, next_buffA;
unsigned	buffWr, next_buffWr;
unsigned	buffD, next_buffD;
unsigned	lineLength, next_lineLength;
unsigned	lineDone, next_lineDone;
unsigned	iterCount, next_iterCount;
unsigned	addrReg;

unsigned done, lastOne, bytemask, pixptr, bmaskMask;
ULONG64 srcData;
ULONG64 cpuData;
unsigned tcAddr, tcWrite;

/*
 * TURBOchannel signals.
 */
unsigned sel, wr;
