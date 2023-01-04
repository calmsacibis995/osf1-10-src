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
/****************************************************************************
**                                                                          *
**                       COPYRIGHT (c) 1990, 1991 BY                        *
**              DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.               *
**			     ALL RIGHTS RESERVED                            *
**                                                                          *
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED  *
**  ONLY  IN  ACCORDANCE  WITH  THE  TERMS  OF  SUCH  LICENSE AND WITH THE  *
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR  ANY  OTHER  *
**  COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY  *
**  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY  *
**  TRANSFERRED.                                                            *
**                                                                          *
**  THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE  WITHOUT  NOTICE  *
**  AND  SHOULD  NOT  BE  CONSTRUED  AS  A COMMITMENT BY DIGITAL EQUIPMENT  *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE OR  RELIABILITY  OF  ITS  *
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.                 *
**                                                                          *
****************************************************************************/

/****************************************************************************
 *                 Smart Frame Buffer Register Definitions                  *
 ***************************************************************************/

/* Modes that sfb can operate in. */
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

/* Depths (bits/pixel) that sfb can operate upon. */
typedef enum {
    /* 00 */  SFBDEPTH8,
    /* 01 */  SFBDEPTH8TOO,
    /* 10 */  SFBDEPTH16,
    /* 11 */  SFBDEPTH32
} SFBDepth;

#define DISABLE_INTERRUPTS 0
#define ENABLE_INTERRUPTS  1

#if SFBBUSBITS == 32
typedef Pixel32     PixelWord;
typedef Bits32	    CommandWord;
#elif SFBBUSBITS == 64
typedef Pixel64     PixelWord; 
typedef Bits64      CommandWord;
#endif

#define SFBBUFFERWORDS	    (SFBCOPYBITS * SFBPIXELBITS / SFBBUSBITS)


/* Command registers. */
typedef volatile struct {
    PixelWord   buffer[SFBBUFFERWORDS];/* Port to read/write copy buffer    */
    PixelWord   foreground;     /* Foreground register (minimum 32 bits)    */
    PixelWord   background;     /* Background register (minimum 32 bits)    */
    PixelWord   planemask;      /* Planemask (minimum 32 bits)		    */
    CommandWord	pixelmask;      /* Pixel mask register			    */
    SFBMode     mode;		/* Hardware mode			    */
    unsigned    rop;		/* Raster op for combining src, dst	    */
    int		shift;		/* -SFBALIGNMASK..+SFBALIGNMASK copy shift  */
    Pixel32      address;	/* Pixel address register		    */
    Bits32      bres1;		/* a1, e1				    */
    Bits32	bres2;		/* a2, e2				    */
    Bits32	bres3;		/* e, count				    */
    Bits32	brescont;	/* Continuation data for lines		    */
    SFBDepth	depth;		/* 8, 16, or 32 bits/pixel?		    */
    CommandWord	start;		/* Start operation if using address reg     */
    CommandWord clear_interrupt;/* Clear Interrupt register		    */
    Bits32 	test_register;	/* ??? 					    */
    Bits32 	refresh_count;  /* interval between refresh reads	    */
    Bits32 	horizontal_setup;/* horizontal video state machine	    */
    Bits32	vertical_setup; /* vertical video state machine		    */
    Bits32	base_address;	/* base row address for starting scan line  */
    CommandWord video_valid;	/* writes to video registers have completed */
    CommandWord enable_disable_interrupt; /* low order bit determines       */
    Bits32	tcclk_counter;	/* oscillator clock counters		    */
    Bits32	vidclk_counter; /* oscillator clock counters		    */
} SFBRec, *SFB;
