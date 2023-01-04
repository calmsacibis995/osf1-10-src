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
static char *rcsid = "@(#)$RCSfile: ffbsetsp.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:17:37 $";
#endif
/*
 */

#include "X.h"

#include "gcstruct.h"
#include "windowstr.h"
#include "regionstr.h"
#include "pixmapstr.h"
#include "scrnintstr.h"

#include "ffb.h"
#include "cfbsetsp.h"

/*
 * Set a list of spans with a list of pixels
 */

void ffbSetSpans(pDraw, pGC, psrc, pptInit, pwidthInit, nInit, fSorted)
    DrawablePtr     pDraw;
    GC              *pGC;
    Pixel8          *psrc;
    DDXPointPtr     pptInit;    /* pointer to start points                  */
    int             nInit;      /* number of spans to fill                  */
    int             *pwidthInit;/* pointer to widths                        */
    int             fSorted;
{
    ffbScreenPrivPtr scrPriv;
    FFB             ffb;

    WRITE_MEMORY_BARRIER();
    CHECKSTATE(pDraw->pScreen, pDraw, scrPriv, ffb, pGC);
    FFBMODE(ffb, SIMPLE);
    FFBSYNC(ffb);
    /* Who gives a shit?  No one calls this anyway. */
    cfbSetSpans(pDraw, pGC, psrc, pptInit, pwidthInit, nInit, fSorted);
}

/*
 * HISTORY
 */
