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
#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader= "$Header: /alphabits/u3/x11/ode/rcs/x11/src/server/ddx/dec/cfb/cfbfgbg.c,v 1.1.2.2 92/01/07 12:48:53 Jim_Ludwig Exp $";	/* BuildSystemHeader */
#endif		/* BuildSystemHeader */
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

#include "gcstruct.h"
#include "cfb.h"
#include "cfbfgbg.h"

static FGBG hashtable[256];     /* Pointers to hash chains */
static int  hashempties[256];   /* Number of refcount == 0 in each chain */

#define Hash(fgandbits, fgxorbits, bgandbits, bgxorbits)    \
    ((  (fgandbits)					    \
      ^ (((fgxorbits) << 2) | ((fgxorbits) >> 6))	    \
      ^ (((bgandbits) << 4) | ((bgandbits) >> 4))	    \
      ^ (((bgxorbits) << 6) | ((bgxorbits) >> 2))	    \
     ) & 0xff)

FGBG cfbGetFGBG(fa, fx, ba, bx)
    register Pixel32 fa;    /* Foreground and bits */
    register Pixel32 fx;    /* Foreground xor bits */
    register Pixel32 ba;    /* Background and bits */
    register Pixel32 bx;    /* Background xor bits */
{
    register Pixel32    bb, fb, bf, ff;
    register Pixel32    key;
    register int	hash;
    register FGBG       p, pprev;

    /* Do we already have a record in the hash table ? Get hash value... */
    fa &= 0xff;
    fx &= 0xff;
    ba &= 0xff;
    bx &= 0xff;
    hash = Hash(fa, fx, ba, bx);

    /* ... and set up a key with all 32 bits of data provided */
    key = (fa << 24) | (fx << 16) | (ba << 8) | (bx);

    /* Look for it */
    for (pprev = NULL, p = hashtable[hash]; p != NULL; pprev = p, p = p->next) {
	if (p->key == key) {
	    /* Move to head of list and update refcount */
	    if (pprev != NULL) {
		pprev->next = p->next;
		p->next = hashtable[hash];
		hashtable[hash] = p;
	    }
	    hashempties[hash] -= (p->refcount == 0);
	    (p->refcount)++;
	    return p;
	}
    }

    /* Didn't find it, we'll just have to construct a new one. */
    p = (FGBG) xalloc(sizeof(FGBGRec));
    p->next = hashtable[hash];
    hashtable[hash] = p;
    p->hash = hash;
    p->key = key;
    p->refcount = 1;
    
#define MapBits(bytes32, bytes10) ((bytes32 << 16) | bytes10)

    /* Construct andbits part of table */
    bb = (ba << 8) | ba;
    bf = (ba << 8) | fa;
    fb = (fa << 8) | ba;
    ff = (fa << 8) | fa;

    p->map[ 0].andbits = MapBits(bb, bb);
    p->map[ 1].andbits = MapBits(bb, bf);
    p->map[ 2].andbits = MapBits(bb, fb);
    p->map[ 3].andbits = MapBits(bb, ff);

    p->map[ 4].andbits = MapBits(bf, bb);
    p->map[ 5].andbits = MapBits(bf, bf);
    p->map[ 6].andbits = MapBits(bf, fb);
    p->map[ 7].andbits = MapBits(bf, ff);

    p->map[ 8].andbits = MapBits(fb, bb);
    p->map[ 9].andbits = MapBits(fb, bf);
    p->map[10].andbits = MapBits(fb, fb);
    p->map[11].andbits = MapBits(fb, ff);

    p->map[12].andbits = MapBits(ff, bb);
    p->map[13].andbits = MapBits(ff, bf);
    p->map[14].andbits = MapBits(ff, fb);
    p->map[15].andbits = MapBits(ff, ff);

    /* Construct xorbits part of table */
    bb = (bx << 8) | bx;
    bf = (bx << 8) | fx;
    fb = (fx << 8) | bx;
    ff = (fx << 8) | fx;

    p->map[ 0].xorbits = MapBits(bb, bb);
    p->map[ 1].xorbits = MapBits(bb, bf);
    p->map[ 2].xorbits = MapBits(bb, fb);
    p->map[ 3].xorbits = MapBits(bb, ff);

    p->map[ 4].xorbits = MapBits(bf, bb);
    p->map[ 5].xorbits = MapBits(bf, bf);
    p->map[ 6].xorbits = MapBits(bf, fb);
    p->map[ 7].xorbits = MapBits(bf, ff);

    p->map[ 8].xorbits = MapBits(fb, bb);
    p->map[ 9].xorbits = MapBits(fb, bf);
    p->map[10].xorbits = MapBits(fb, fb);
    p->map[11].xorbits = MapBits(fb, ff);

    p->map[12].xorbits = MapBits(ff, bb);
    p->map[13].xorbits = MapBits(ff, bf);
    p->map[14].xorbits = MapBits(ff, fb);
    p->map[15].xorbits = MapBits(ff, ff);

    return p;
}

void cfbFreeFGBG(p)
    register FGBG   p;
{
    register int    hash, empties;
    register FGBG   pprev, pnext;

    (p->refcount)--;
    if (p->refcount == 0) {
	hash = p->hash;
	empties = hashempties[hash] + 1;

	if (empties > 3) {
	    /* We have too many refcount == 0 entries on this hash chain.  Go
	       free them all at once. */

	    /* Reclaim all refcount == 0 at head of list */
	    for (p = hashtable[hash]; p != NULL && p->refcount == 0; p = pnext){
		pnext = p->next;
		xfree(p);
		empties--;
	    }
	    hashtable[hash] = p;

	    /* Reclaim anyone refcount == 0 past head of list.  Note that
	       list may already be empty at this point */
	    for ( ; p != NULL && empties != 0; p = pnext) {
		pnext = p->next;
		if (p->refcount == 0) {
		    pprev->next = pnext;
		    xfree(p);
		    empties--;
		} else {
		    pprev = p;
		}
	    }
	} /* if too many empties */
	hashempties[hash] = empties;
    }
}

/*
 * These routines just create, if needed, the three types of FGBG tables
 * currently used, then return the map portion of the table.
 *
 * This conveniently allows us to delay creating the table until we actually
 * need it, which we may never, thus saving boatloads of memory.
 */

FGBGMap cfbGetITFGBGMap(pGC)
    GCPtr   pGC;
{
    register cfbPrivGC  *gcPriv;

    gcPriv = (cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr);
    if (gcPriv->itfgbg == NULL) {
	/* itfgbg gets fg, bg, as if raster op were GXcopy */
	Pixel32 planemask;
	planemask = pGC->planemask;
	gcPriv->itfgbg = cfbGetFGBG(~planemask, pGC->fgPixel & planemask,
				    ~planemask, pGC->bgPixel & planemask);
    }
    return gcPriv->itfgbg->map;
}


FGBGMap cfbGetOSFGBGMap(pGC)
    GCPtr   pGC;
{
    register cfbPrivGC  *gcPriv;

    gcPriv = (cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr);
    if (gcPriv->osfgbg == NULL) {
	/* osfgbg gets fg, bg according to raster op */
	gcPriv->osfgbg = cfbGetFGBG(gcPriv->fgandbits, gcPriv->fgxorbits,
				    gcPriv->bgandbits, gcPriv->bgxorbits);
    }
    return gcPriv->osfgbg->map;
}


FGBGMap cfbGetTSFGBGMap(pGC)
    GCPtr   pGC;
{
    register cfbPrivGC  *gcPriv;

    gcPriv = (cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr);
    if (gcPriv->tsfgbg == NULL) {
	/* tsfgbg gets fg, bg according to raster op for fg, noop for bg */
	gcPriv->tsfgbg = cfbGetFGBG(gcPriv->fgandbits, gcPriv->fgxorbits,
				    -1,                0);

    }
    return gcPriv->tsfgbg->map;
}
