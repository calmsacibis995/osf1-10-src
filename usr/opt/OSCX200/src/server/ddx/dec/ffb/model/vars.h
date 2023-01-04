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
/*
 * @(#)$RCSfile: vars.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:46:41 $
 */
typedef enum {
    /* 000 */ GL_GEQUAL,
    /* 001 */ GL_ALWAYS,
    /* 010 */ GL_NEVER,
    /* 011 */ GL_LESS,
    /* 100 */ GL_EQUAL,
    /* 101 */ GL_LEQUAL,
    /* 110 */ GL_GREATER,
    /* 111 */ GL_NOTEQUAL
} GL_TEST;

typedef enum {
  GL_KEEP,
  GL_ZERO,
  GL_REPLACE,
  GL_INCR,
  GL_DECR,
  GL_INVERT 
} GLStencilOps;

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

#define OPAQUEFILL	0x21
#define TRANSPFILL	0x25
#define TRANSPBLKSTIPPL	0x0d
#define BLOCKFILL	0x2d
#define DMAREAD 	0x17
#define DMAREADDITHERED	0x37
#define DMAWRITE 	0x1f

#define BLOCKMASK	0x5f
#define COLORINTERPMASK	0x08
#define ZBUFFEREDMASK	0x10
#define DITHEREDMASK	0x20
#define DMAMASK		0x17

typedef enum {
  VISUAL8P	= 0x0,  /*  8 bit packed   */
  VISUAL8U	= 0x1,  /*  8 bit unpacked */
  VISUAL12	= 0x2,  /* 12 bit (low src)*/
  VISUAL24	= 0x3,  /* 24 bit          */
  VISUAL12HI	= 0x6	/* 12 bit (hi src) */
} SFBVisual;

typedef enum {
  DEPTH8  = 0x0,
  DEPTH32 = 0x1
} SFBDepth;

#define VISUAL32MASK VISUAL12

#define TRANSPMASK unusedmode1

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

#define TRUE_L 0
#define FALSE_L -1

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
  unsigned rOp    	: 4;
  unsigned fill   	: 4;
  unsigned  visual	: 2;	/* defines type of dst
				 * visual on 32-plane systems */
  unsigned rotate	: 2;	/* defines position of dst 8-bit
				 * visual on 32-plane systems */
} ROP;

typedef struct {
  unsigned  e1 : 16;	/* e1 is always positive */
  int       a1 : 16;	/* address/error inc if e < 0 */
} BRES1;

typedef struct {
  unsigned e2 : 16;	/* e2 is positive (it's negated when used) */
  int	   a2 : 16; 	/* address/error inc if e >= 0 */
} BRES2;

typedef struct {
  unsigned lineLength : 4;	/* line length count */
  unsigned ignored : 11;
  int      e : 17;		/* e is sign-extended */
} BRES3;

typedef struct {
  unsigned value    : 20;		/* color value */
  unsigned ignored  : 7;
  unsigned rowcol   : 5;		/* row/column dither index */
} RGVAL;

typedef struct {
  unsigned mode		: 7;	/* basic mode */
  unsigned ignored	: 1;
  unsigned  visual	: 3;	/* defines type of src
				 * visual on 32-plane systems */
  unsigned rotate	: 2;	/* defines position of src 8-bit
				 * visual on 32-plane systems */
  unsigned ntLines	: 1;	/* Windows32 GQI style lines  */
  unsigned z16	        : 1;	/* Z buffer size is 16 bits   */
  unsigned capEnds	: 1;	/* cap ends of lines (or not) */
} MODE;

typedef struct {
  unsigned sWrMask      : 8;
  unsigned sRdMask      : 8; 
  unsigned sTest	: 3;	/* comparison to perform on stencil buffer */
  unsigned sFail	: 3;	/* op if stencil test fails */
  unsigned zFail	: 3;	/* op if stencil test passes, Z test fails */
  unsigned zPass	: 3;	/* op if stencil test passes, Z test passes */
  unsigned zTest	: 3;	/* comparison to perform on Z-buffer */
  unsigned zOp		: 1;	/* 0 -> KEEP, 1 -> REPLACE */
} STENCIL; 

typedef struct {
  unsigned active       : 11;
  unsigned fp           : 5;
  unsigned sync 	: 6;
  unsigned bp       	: 6;
} VERT; 

typedef struct {
  unsigned active       : 9;
  unsigned fp           : 5;
  unsigned sync 	: 7;
  unsigned bp       	: 7;
  unsigned ignore       : 3;
  unsigned odd          : 1;
} HORIZ; 

typedef struct {
  unsigned x       : 12;
  unsigned y       : 12;
} CURSORXYSTRUCT; 

typedef struct {
  unsigned ignore       :  4;
  unsigned base         :  6;
  unsigned rowsMinusOne :  6;
} CURSORBASESTRUCT; 

typedef int Pixel32;
typedef int Int32;
typedef char Pixel8;
/*
 * the following is a pseudo-register.
 * no storage actually exists for it.
 */
typedef struct {
  unsigned linedata : 16;
  unsigned addrLo : 2;
} BSTART;

typedef struct {
  unsigned pixCount : 11;
  unsigned ignore1 : 5;
  unsigned addrLo : 2;
  unsigned ignore2 : 14;
} BLKFILL;

typedef struct {
  unsigned left1 : 4;
  unsigned left2 : 4;
  unsigned right1 : 4;
  unsigned right2 : 4;
  unsigned count : 11;
  unsigned ignore : 5;
} DMAREADCMD;

typedef struct {
  unsigned left : 8;
  unsigned right : 8;
  unsigned count : 11;
  unsigned ignore : 5;
} DMAWRITECMD;

typedef struct {
  unsigned  dx : 16;	/* magnitude of delta-x */
  unsigned  dy : 16;	/* magnitude of delta-y */
} DXDY;

typedef struct {
  unsigned  drawable : 16;	/* width of drawable */
  unsigned  zbuffer : 16;	/* width of zbuffer */
} BWIDTH;

typedef struct {
  unsigned base : 24;
  unsigned ignored : 8;
} ZADDR;

typedef struct {
  unsigned base : 9;
  unsigned ignored : 23;
} VBASE;

typedef struct {
  unsigned fract   : 32;
} ZINCLO;

typedef struct {
  unsigned whole   : 4;
  unsigned ignored : 28;
} ZINCHI;

typedef struct {
  unsigned fract   : 32;
} ZVALLO;

typedef struct {
  unsigned whole   : 4;
  unsigned ignore  : 20;
  unsigned stencil : 8;
} ZVALHI;

typedef struct {
  unsigned deep : 1;		/* frame buffer depth - 8bpp or 32bpp */
  unsigned ignored : 1;
  unsigned addrMask : 3;	/* mask for system address bits 24:22 */
  unsigned blockType : 4;	/* type of block mode in VRAMs */
  unsigned colSize : 1;         /* column size of the VRAMs */
  unsigned samSize : 1;		/* set if shift register has 256 entries */
  unsigned parity : 1;          /* parity bit used by diagnostics */
  unsigned writeEnROM : 1;      /* write enable Flash ROM */
  unsigned blkOff : 1;          /* TURBOchannel block mode disable */
  unsigned slowDac : 1;         /* set for slow Ramdacs (485) */
  unsigned dma128 : 1;          /* set if 128 word DMA's allowed  */
  unsigned hSync : 1;           /* set of separate hSync and vSync needed */
} DEEP;

typedef union {
  unsigned u32;
  VERT reg;
} VERTICAL;

typedef union {
  unsigned u32;
  HORIZ reg;
} HORIZONTAL;

typedef union {
  unsigned u32;
  CURSORXYSTRUCT reg;
} CURSORXYREG;

typedef union {
  unsigned u32;
  CURSORBASESTRUCT reg;
} CURSORBASEREG;

typedef union {
  unsigned u32;
  ROP reg;
} ROPREG;

typedef union {
  unsigned u32;
  DEEP reg;
} DEEPREG;

typedef union {
  unsigned u32;
  MODE reg;
} MODEREG;

typedef union {
  unsigned u32; 
  STENCIL reg;
} STENCILREG;

typedef union {
  unsigned u32;
  BWIDTH reg;
} BWIDTHREG;

typedef union {
  unsigned u32;
  BRES1 reg;
} BRES1REG;

typedef union {
  unsigned u32;
  BRES2 reg;
} BRES2REG;

typedef union {
  unsigned u32;
  BRES3 reg;
} BRES3REG;

typedef union {
  unsigned u32;
  BSTART reg;
} BSTARTREG;

typedef union {
  unsigned u32;
  BLKFILL reg;
} BLKFILLREG;

typedef union {
  unsigned u32;
  DMAREADCMD rdCmd;
  DMAWRITECMD wrCmd;
} DMACMD;

typedef union {
  unsigned u32;
  RGVAL reg;
} RGVALREG;

typedef union {
  unsigned u32;
  DXDY reg;
} DXDYREG;

typedef union {
  unsigned u32;
  ZADDR reg;
} ZADDRREG;

typedef union {
  unsigned u32;
  ZINCLO reg;
} ZINCLOREG;

typedef union {
  unsigned u32;
  VBASE reg;
} VBASEREG;

typedef union {
  unsigned u32;
  ZINCHI reg;
} ZINCHIREG;

typedef union {
  unsigned u32;
  ZVALLO reg;
} ZVALLOREG;

typedef union {
  unsigned u32;
  ZVALHI reg;
} ZVALHIREG;

typedef union {
  unsigned data;
  BRES1 bres1;
  BRES2 bres2;
  BRES3 bres3;
  BSTART bstart;
  RGVAL rgval;
  MODE mode;
  STENCIL stencil;
  DXDY dxdy;
} REGS;

typedef struct {
    DEEPREG	    depth;	    /* 8, 16, or 32 plane depth		    */
    COLORS	    foreground;
    COLORS	    background;
    ROPREG          rop;	    /* X graphics function		    */
    unsigned	    planeMask;	    /* X planemask			    */
    SFBMode	    oldMode;	    /* Which mode?			    */
    Int32	    pixelMask;      /* Pixel mask register		    */
    Pixel8	    *address;       /* Pixel address register		    */
    Int32	    more;	    /* Continuation data		    */
    BRES1REG	    bres1;	    /* a1, e1				    */
    BRES2REG	    bres2;	    /* a2, e2				    */
    BRES3REG	    bres3;	    /* e, count				    */
    unsigned	    shift;	    /* -7..+7 shift for copies		    */
    REGS	    redInc;	    /* red increment			    */
    REGS	    grnInc;	    /* green increment			    */
    REGS	    bluInc;	    /* blue increment			    */
    REGS	    redVal;	    /* red value/dither row		    */
    REGS	    grnVal;	    /* green value/dither col		    */
    REGS	    bluVal;	    /* blue value     			    */
    MODEREG	    mode;	    /* mode register  			    */
    STENCILREG      stencil;        /* stencil register                     */
    REGS	    dataReg;	    /* data register (for use w/ slope)     */
    BWIDTHREG       bresWidth;      /* width of scanline for FB and Z       */
    ZADDRREG        zAddr;
    VBASEREG        vBase;
    ZINCLOREG       zIncLo;
    ZINCHIREG       zIncHi;
    ZVALLOREG       zValLo;
    ZVALHIREG       zValHi;
    unsigned        dmaBase;
} SFBREG;

