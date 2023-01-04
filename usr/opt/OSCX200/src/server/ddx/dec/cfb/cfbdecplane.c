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
#define MODULE_NAME CFBDECPLANE
#include "module_ident.h"
#endif
/****************************************************************************
**                                                                          *
**                   COPYRIGHT (c) 1988, 1989, 1990 BY                      *
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

/* $Header: /alphabits/u3/x11/ode/rcs/x11/src/server/ddx/dec/cfb/cfbdecplane.c,v 1.1.2.5 92/06/18 12:03:19 Jim_Ludwig Exp $ */
/* Author: Joel McCormack, derived from Todd Newman's mi code */

# include "pixmapstr.h"
# include "Xmd.h"
#if defined(MITR5)
# include "servermd.h"
#endif
# include "scrnintstr.h"

/* cfbGetPlane -- gets a pixmap representing one plane of pDraw
 * A helper used for cfbCopyPlane.  Go through each of the damn pixels and pull
 * out the appropriate bit.
 */

PixmapPtr
cfbGetPlane(pDraw, planeNum, sx, sy, w, h)
    DrawablePtr	    pDraw;
    int		    planeNum;	/* number of the bitPlane */
    int		    sx, sy, w, h;
{
    PixmapPtr	    result;
    int		    i, j, k, width;
    DDXPointRec     pt;
    Pixel	    pixel;
    Pixel	    bit;
    int		    delta;

#if (BITMAP_SCANLINE_UNIT == 8)
#define SCANUNIT CARD8
#endif
#if (BITMAP_SCANLINE_UNIT == 16)
#define SCANUNIT CARD16
#endif
#if (BITMAP_SCANLINE_UNIT == 32)
#define SCANUNIT CARD32
#endif
#if (BITMAP_SCANLINE_UNIT == 64)
#define SCANUNIT unsigned long
#endif
    SCANUNIT	*pOut;

    sx += pDraw->x;
    sy += pDraw->y;

    result = (PixmapPtr)(*pDraw->pScreen->CreatePixmap)
		       (pDraw->pScreen, w, h, 1);

    if (result == NULL) {
	ErrorF( "cfbGetPlane can't make temp pixmap\n");
	return NULL;
    }

    pOut = (SCANUNIT *) result->devPrivate.ptr;
    bzero(pOut, result->devKind * result->drawable.height);
    *pOut = (SCANUNIT)0;
    for (i = sy; i < sy + h; i++) {
	delta = (result->devKind / (BITMAP_SCANLINE_UNIT / 8)) -
		    (w / BITMAP_SCANLINE_UNIT);
	k = 0;
	for (j = 0; j < w; j++) {
	    pt.x = sx + j;
	    pt.y = i;
	    width = 1;
	    /* Fetch the next pixel */
	    (*pDraw->pScreen->GetSpans)(pDraw, width, &pt, &width, 1, &pixel);
	    /*
	     * Now get the bit and insert into a bitmap in XY format.
	     */
	    bit = (Pixel) ((pixel >> planeNum) & 1);
	    bit <<= k;
	    *pOut |= (SCANUNIT) bit;
	    k++;
	    if (k == BITMAP_SCANLINE_UNIT) {
		pOut++;
		k = 0;
	    }
	}
	pOut += delta;
    }
    return(result);    
}
