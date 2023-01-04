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
#ifdef VMS
#define IDENT "X-2"
#include "module_ident.h"
#endif
/****************************************************************************
**                                                                          *
**                  COPYRIGHT (c) 1988, 1989, 1990 BY                       *
**             DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.                *
**			   ALL RIGHTS RESERVED                              *
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
/**
**++
**  FACILITY:
**
**      DDXCFB - VMS CFB server
**
**  ABSTRACT:
**
**      
**
**  AUTHORS:
**
**      Irene McCartney (from Joel McCormack)
**
**
**  CREATION DATE:     20-Nov-1991
**
**  MODIFICATION HISTORY:
**
**
** X-2		TLB0003		Tracy Bragdon			03-Dec-1991
**		modify references to GC structure for R5
**--
**/

/* $Header: /alphabits/u3/x11/ode/rcs/x11/src/server/ddx/dec/cfb/cfbdecbres.c,v 1.1.2.2 92/01/07 12:47:59 Jim_Ludwig Exp $ */
#include "X.h"
#include "cfb.h"
#ifdef MITR5
#include "cfbdec.h"
#include "cfbdecline.h"
#define __cfbdecbres__ 1
#include "cfbdecbres.h"
#undef __cfbdecbres__
#define Pixel8 		unsigned char
#define Pixel32		unsigned int
#else
#include "cfbline.h"
#include "cfbbres.h"
#endif

/* NOTE: Inner loops are not unrolled.  We expect memory time
   to be slow enough to suck away all loop overhead time. */


/* Solid left-to-right horizontal line. len > 0 */
void CFBHORZS(addr, len, gcPriv)
    register Pixel8	    *addr;	/* pointer to first point       */
    register int	    len;	/* length of line		*/
    cfbPrivGC		    *gcPriv;    /* Private GC info		*/
{
    register Pixel32    fgandbits, fgxorbits;

#ifdef MITR5
    fgandbits = gcPriv->and;
    fgxorbits = gcPriv->xor;
#else
    fgandbits = gcPriv->fgandbits;
    fgxorbits = gcPriv->fgxorbits;
#endif

    /* Write bytes until aligned to word boundary */
    while (((int) addr) & 3) {
	CFBFILL(addr, fgandbits, fgxorbits);
	addr++;
	len--;
	if (len == 0) return;
    }
    len -= 4;
    if (len >= 0) {
	do {
	    CFBFILL((Pixel32 *)addr, fgandbits, fgxorbits);
	    addr += 4;
	    len -= 4;
	} while (len >= 0);
    }
    len += 4;
    /* Write final bytes */
    while (len > 0) {
	CFBFILL(addr, fgandbits, fgxorbits);
	addr++;
	len--;
    }
}


/* Solid top-to-bottom vertical line. len > 0 */

void CFBVERTS(addr, snlwidth, len, gcPriv)
    register Pixel8	    *addr;	/* pointer to first point */
    register int	    snlwidth;	/* width in bytes of bitmap */
    register int	    len;	/* length of line */
    cfbPrivGC		    *gcPriv;    /* Private GC info		*/
{
    register Pixel32	    fgandbits, fgxorbits;

#ifdef MITR5
    fgandbits = gcPriv->and;
    fgxorbits = gcPriv->xor;
#else
    fgandbits = gcPriv->fgandbits;
    fgxorbits = gcPriv->fgxorbits;
#endif 
    do {
	CFBFILL(addr, fgandbits, fgxorbits);
	addr += snlwidth;
	len--;
    } while (len > 0);
}


/* Solid sloped line */

void CFBBRESS(addr, snlwidth, signdx, verticalish, e, e1, e2, len, gcPriv)
    register Pixel8 *addr;	    /* pointer to first point		*/
    register int    snlwidth;	    /* width in bytes of pixmap		*/
	     int    signdx;	    /* sign of x direction		*/
	     Bool   verticalish;    /* always step 1 in y direction?    */
    register int    e;		    /* error accumulator		*/
    register int    e1;		    /* bresenham increments		*/
    register int    e2;
    register int    len;	    /* length of line			*/
    cfbPrivGC       *gcPriv;	    /* Private GC info			*/
{
    register Pixel32	    fgandbits, fgxorbits;

#ifdef MITR5
    fgandbits = gcPriv->and;
    fgxorbits = gcPriv->xor;
#else
    fgandbits = gcPriv->fgandbits;
    fgxorbits = gcPriv->fgxorbits;
#endif

    /* NOTE: snlwidth has already been multiplied by signdy */

    if (! verticalish) {
	if (signdx > 0) {

	    /* Horizontal increments 1 pixel, vertical increments 0 or 1*/
	    /* Get to where we can do 4 at a time */
	    while (len & 3) {
		CPHW(0);
		addr++;
		len--;
	    }
	    /* Do 4 bytes at a time now */
	    if (len > 0) {
		do {
		    CPHW(0);  CPHW(1);  CPHW(2);  CPHW(3);
		    addr += 4;
		    len -= 4;
		} while (len > 0);
	    }

	} else {

	    /* Horizontal decrements 1 pixel, vertical increments 0 or 1 */
	    while (len & 3) {
		CNHW(0);
		addr--;
		len--;
	    }
	    if (len > 0) {
		do {
		    CNHW(0);  CNHW(-1);  CNHW(-2);  CNHW(-3);
		    addr -= 4;
		    len -= 4;
		} while (len > 0);
	    }
	}

    } /* if !verticalish */ else {

	if (signdx > 0) {
	    /* Set up to use same test as signdx < 0 */
	    e++;
	}
	/* Horizontal move 0 or +/- 1 pixels, vertical increments 1 */

	while (len & 3) {
	    CVW();
	    len--;
	}
	if (len > 0) {
	    do {
		CVW();  CVW();  CVW();  CVW();
		len -= 4;
	    } while (len > 0);
	}

    } /* else verticalish */
} 


/* OnOff and double dashed bresenham lines */

/*

I have a few different strategies for dashed lines.  First, I assume that
the number of dots in each dash is fairly small.  This means I don't try to do
4 at a time.  It also means that when I paint nothing, I just go through the
normal Bresenham step loop rather than do some costly divides.  Finally, I don't
really care about dashed lines as much as normal lines: 100-pixel lines are
already better than half-speed of solid lines, so I'm done tuning.

*/

#define NextDash()						\
{								\
    /* Move to next dash */					\
    major++;							\
    if (major == numDashes) major = 0;				\
    minor = pDash[major];					\
    which = ~which;						\
} /* NextDash */


void CFBOOBRESS
    (addr, snlwidth, signdx, verticalish, e, e1, e2, len,
     gcPriv, pDesc, pPos, newPos)
    register Pixel8 *addr;	    /* pointer to first point		*/
    register int    snlwidth;	    /* width in bytes of pixmap		*/
	     int    signdx;	    /* sign of x direction		*/
	     Bool   verticalish;    /* always step 1 in y direction?    */
    register int    e;		    /* error accumulator		*/
    register int    e1;		    /* bresenham increments		*/
    register int    e2;
    register int    len;	    /* length of line			*/
    cfbPrivGC       *gcPriv;	    /* Private GC info			*/
    DashDesc	    *pDesc;
    DashPos	    *pPos;
    Bool	    newPos;	    /* Return new position in pPos?     */
{
    register int    dlen;	    /* Length for this dash		*/
    register int    major, minor;   /* Data from pDesc, pPos		*/
    register unsigned char *pDash;
    register int    numDashes;
    register int    which, curWhich;
    
    register Pixel32	    fgandbits, fgxorbits;

#ifdef MITR5
    fgandbits = gcPriv->and;
    fgxorbits = gcPriv->xor;
#else
    fgandbits = gcPriv->fgandbits;
    fgxorbits = gcPriv->fgxorbits;
#endif

    major = pPos->major;
    minor = pPos->minor;
    which = pPos->which;

    pDash = pDesc->pDash;
    numDashes = pDesc->numDashes;
    
   /* NOTE: snlwidth has already been multiplied by signdy */

    if (!verticalish) {
	if (signdx > 0) {
	    /* Horizontal increments 1 pixel, vertical increments 0 or 1*/
	    while (len != 0) {
		curWhich = which;
		dlen = len;
		minor -= len;
		if (minor <= 0) {
		    dlen = minor + len;
		    NextDash();
		}
		len -= dlen;

		if (curWhich == EVENDASH) {
		    /* Actually paint something */
		    do {
			CPHW(0);
			addr++;
			dlen--;
		    } while (dlen != 0);
		} else {
		    /* Make blank spot.  Assume dlen small, and faster to
		       loop than multiply/div/mod */
		    addr += dlen;
		    do {
			CPH();
			dlen--;
		    } while (dlen != 0);
		}
	    }

	} else {
	    /* Horizontal decrements 1 pixel, vertical increments 0 or 1 */
	    while (len != 0) {
		curWhich = which;
		dlen = len;
		minor -= len;
		if (minor <= 0) {
		    dlen = minor + len;
		    NextDash();
		}
		len -= dlen;

		if (curWhich == EVENDASH) {
		    /* Actually paint something */
		    do {
			CNHW(0);
			addr--;
			dlen--;
		    } while (dlen != 0);
		} else {
		    /* Make blank spot.  Assume dlen small, and faster to
		       loop than multiply/div/mod */
		    addr -= dlen;
		    do {
			CNH();
			dlen--;
		    } while (dlen != 0);
		}
	    }
	}

    } /* if !verticalish */ else {

	if (signdx > 0) {
	    /* Set up to use same test as signdx < 0 */
	    e++;
	}
	/* Horizontal move 0 or +/- 1 pixels, vertical increments 1 */

	while (len != 0) {
	    curWhich = which;
	    dlen = len;
	    minor -= len;
	    if (minor <= 0) {
		dlen = minor + len;
		NextDash();
	    }
	    len -= dlen;

	    if (curWhich == EVENDASH) {
		/* Actually paint something */
		do {
		    CVW();
		    dlen--;
		} while (dlen != 0);
	    } else {
		/* Make blank spot */
		do {
		    CV();
		    dlen--;
		} while (dlen != 0);
	    }
	}
    } /* else verticalish */

    /* Now return current dash information */
    if (newPos) {
	pPos->major = major;
	pPos->minor = minor;
	pPos->which = which;
    }
} 

void CFBDBRESS
    (addr, snlwidth, signdx, verticalish, e, e1, e2, len,
     gcPriv, pDesc, pPos, newPos)
    register Pixel8 *addr;	    /* pointer to first point		*/
    register int    snlwidth;	    /* width in bytes of pixmap		*/
	     int    signdx;	    /* sign of x direction		*/
	     Bool   verticalish;    /* always step 1 in y direction?    */
    register int    e;		    /* error accumulator		*/
    register int    e1;		    /* bresenham increments		*/
    register int    e2;
    register int    len;	    /* length of line			*/
    cfbPrivGC       *gcPriv;	    /* Private GC info			*/
    DashDesc	    *pDesc;
    DashPos	    *pPos;
    Bool	    newPos;	    /* Return new position in pPos?     */
{
    register int    dlen;	    /* Length for this dash		*/
    register int    major, minor;   /* Data from pDesc, pPos		*/
    register unsigned char *pDash;
    register int    numDashes;
    register int    which, curWhich;
    
    register Pixel32	    fgandbits, fgxorbits;

    major = pPos->major;
    minor = pPos->minor;
    which = pPos->which;

    pDash = pDesc->pDash;
    numDashes = pDesc->numDashes;
    
    /* NOTE: snlwidth has already been multiplied by signdy */

    if (!verticalish) {
	if (signdx > 0) {
	    /* Horizontal increments 1 pixel, vertical increments 0 or 1*/
	    while (len != 0) {
		curWhich = which;
		dlen = len;
		minor -= len;
		if (minor <= 0) {
		    dlen = minor + len;
		    NextDash();
		}
		len -= dlen;

		if (curWhich == EVENDASH) {
#ifdef MITR5
		    fgandbits = gcPriv->and;
		    fgxorbits = gcPriv->xor;
#else
		    fgandbits = gcPriv->fgandbits;
		    fgxorbits = gcPriv->fgxorbits;
		} else {
		    fgandbits = gcPriv->bgandbits;
		    fgxorbits = gcPriv->bgxorbits;
#endif
		}
		do {
		    CPHW(0);
		    addr++;
		    dlen--;
		} while (dlen != 0);
	    }

	} else {
	    /* Horizontal decrements 1 pixel, vertical increments 0 or 1 */
	    while (len != 0) {
		curWhich = which;
		dlen = len;
		minor -= len;
		if (minor <= 0) {
		    dlen = minor + len;
		    NextDash();
		}
		len -= dlen;

		if (curWhich == EVENDASH) {
#ifdef MITR5
		    fgandbits = gcPriv->and;
		    fgxorbits = gcPriv->xor;
#else
		    fgandbits = gcPriv->fgandbits;
		    fgxorbits = gcPriv->fgxorbits;
		} else {
		    fgandbits = gcPriv->bgandbits;
		    fgxorbits = gcPriv->bgxorbits;
#endif
		}
		do {
		    CNHW(0);
		    addr--;
		    dlen--;
		} while (dlen != 0);
	    }
	}

    } /* if !verticalish */ else {

	if (signdx > 0) {
	    /* Set up to use same test as signdx < 0 */
	    e++;
	}
	/* Horizontal move 0 or +/- 1 pixels, vertical increments 1 */

	while (len != 0) {
	    curWhich = which;
	    dlen = len;
	    minor -= len;
	    if (minor <= 0) {
		dlen = minor + len;
		NextDash();
	    }
	    len -= dlen;

	    if (curWhich == EVENDASH) {
#ifdef MITR5
		fgandbits = gcPriv->and;
		fgxorbits = gcPriv->xor;
#else
		fgandbits = gcPriv->fgandbits;
		fgxorbits = gcPriv->fgxorbits;
	    } else {
		fgandbits = gcPriv->bgandbits;
		fgxorbits = gcPriv->bgxorbits;
#endif
	    }
	    do {
		CVW();
		dlen--;
	    } while (dlen != 0);
	}
    } /* else verticalish */

    /* Now return current dash information */
    if (newPos) {
	pPos->major = major;
	pPos->minor = minor;
	pPos->which = which;
    }
} 
