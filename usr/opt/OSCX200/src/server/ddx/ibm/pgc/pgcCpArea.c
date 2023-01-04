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
 * $XConsortium: pgcCpArea.c,v 1.2 91/07/16 13:12:10 jap Exp $
 *
 * Copyright IBM Corporation 1987,1988,1989,1990,1991
 *
 * All Rights Reserved
 *
 * License to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of IBM not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 *
 * IBM DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS, AND 
 * NONINFRINGEMENT OF THIRD PARTY RIGHTS, IN NO EVENT SHALL
 * IBM BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 *
*/

#include "X.h"
#include "servermd.h"
#include "misc.h"
#include "regionstr.h"
#include "gcstruct.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "scrnintstr.h"

#include "ibmTrace.h"
#include "mi.h"

#include "cfb.h"
#include "pgc.h"
extern pgcScreenRec pgcScreenInfo[] ;

extern int cfbGCPrivateIndex;

RegionPtr
pgcCopyArea( pSrcDrawable, pDstDrawable,
	     pGC, srcx, srcy, width, height, dstx, dsty )
register DrawablePtr pSrcDrawable ;
register DrawablePtr pDstDrawable ;
GC *pGC ;
int srcx, srcy ;
int width, height ;
int dstx, dsty ;
{
	register BoxPtr pbox ;
	register int dx ;
	register int dy ;
	int nbox ;
	ScreenPtr pScreen ;
	RegionPtr prgnDst ;
	RegionPtr prgnExposed ;
	int alu ;
	unsigned long int pm ;
	/* temporaries for shuffling rectangles */
	xRectangle *origSource ;
	DDXPointRec *origDest ;
	cfbPrivGC *pPriv ;

        TRACE(("pgcCopyArea\n"));

	if ( !( pm = pGC->planemask ) || ( ( alu = pGC->alu ) == GXnoop ) )
		return NULL ;

	/*
	 * If the destination drawable is not a window then call the mi version.
	 */

	if ( pDstDrawable->type != DRAWABLE_WINDOW )
		return cfbCopyArea( pSrcDrawable, pDstDrawable, pGC,
				   srcx, srcy, width, height, dstx, dsty ) ;

	/* BY HERE, You know you are going to a Window */
	if ( !( (WindowPtr) pDstDrawable )->realized )
		return NULL ;

	if ( pSrcDrawable->type != DRAWABLE_WINDOW )
		return cfbCopyArea( (PixmapPtr) pSrcDrawable,
				      pDstDrawable,
				      pGC, srcx, srcy, width,
				      height, dstx, dsty ) ;

	pPriv = (cfbPrivGC *) ( pGC->devPrivates[cfbGCPrivateIndex].ptr ) ;
	/* BY HERE, You know you are going from a Window to a Window */
	if ( pPriv->fExpose ) {
		if ( !( origSource = (xRectangle *)
		    ALLOCATE_LOCAL( sizeof( xRectangle ) ) ) )
			return NULL ;
		origSource->x = srcx ;
		origSource->y = srcy ;
		origSource->width = width ;
		origSource->height = height ;
		if ( !( origDest = (DDXPointRec *)
		    ALLOCATE_LOCAL( sizeof( DDXPointRec ) ) ) ) {
			DEALLOCATE_LOCAL( origSource ) ;
			return NULL ;
		}
		origDest->x = dstx ;
		origDest->y = dsty ;
	}
	else {
		origSource = (xRectangle *) 0 ;
		origDest = (DDXPointRec *) 0 ;
	}

	/* clip the left and top edges of the source */
	if ( srcx < 0 ) {
		width += srcx ;
		srcx = pSrcDrawable->x ;
	}
	else
		srcx += pSrcDrawable->x ;
	if ( srcy < 0 ) {
		height += srcy ;
		srcy = pSrcDrawable->y ;
	}
	else
		srcy += pSrcDrawable->y ;

	pScreen = pDstDrawable->pScreen ;
	/* clip the source */
	{
		BoxRec srcBox ;

		srcBox.x1 = srcx ;
		srcBox.y1 = srcy ;
		srcBox.x2 = srcx + width ;
		srcBox.y2 = srcy + height ;

		prgnDst = (* pScreen->RegionCreate)( &srcBox, 1 ) ;
	}
	if ( pGC->subWindowMode == IncludeInferiors ) {
		register RegionPtr prgnSrcClip =
			NotClippedByChildren( (WindowPtr) pSrcDrawable ) ;
		(* pScreen->Intersect)( prgnDst, prgnDst, prgnSrcClip ) ;
		(* pScreen->RegionDestroy)( prgnSrcClip ) ;
	}
	else
		(* pScreen->Intersect)( prgnDst, prgnDst,
				&(((WindowPtr) pSrcDrawable)->clipList) ) ;

	dstx += pDstDrawable->x ;
	dsty += pDstDrawable->y ;

	dx = srcx - dstx ;
	dy = srcy - dsty ;

	/* clip the shape of the dst to the destination composite clip */
	(* pScreen->TranslateRegion)( prgnDst, -dx, -dy ) ;
	(* pScreen->Intersect)( prgnDst, prgnDst, pPriv->pCompositeClip ) ;

	/* nbox != 0 destination region is visable */
	if ( nbox = REGION_NUM_RECTS(prgnDst) ) {
		BoxPtr pboxTmp, pboxNext, pboxBase, pboxNew1, pboxNew2 ;

		pbox = REGION_RECTS(prgnDst);

		pboxNew1 = 0 ;
		pboxNew2 = 0 ;
		if ( nbox > 1 ) {
			if ( dy < 0 ) {
				/* walk source bottom to top */
				/* keep ordering in each band, */
				/* reverse order of bands */
				if ( !( pboxNew1 = (BoxPtr)
					ALLOCATE_LOCAL( nbox * sizeof (BoxRec) ) ) ) {
					(* pScreen->RegionDestroy)( prgnDst ) ;
					return NULL ;
				}
				for ( pboxBase = pboxNext = pbox + nbox - 1 ;
				      pboxBase >= pbox ;
				      pboxBase = pboxNext ) {
					while ( pboxNext >= pbox
					     && pboxBase->y1 == pboxNext->y1 )
						pboxNext-- ;
					for ( pboxTmp = pboxNext + 1 ;
					      pboxTmp <= pboxBase ;
					      *pboxNew1++ = *pboxTmp++ )
						/*DO NOTHING*/ ;
				}
				pbox = ( pboxNew1 -= nbox ) ;
			}
			if ( dx < 0 ) {
				/* walk source right to left */
				/* reverse order of rects in each band */
				if ( !( pboxNew2 = (BoxPtr)
				    ALLOCATE_LOCAL( sizeof (BoxRec) * nbox ) ) ) {
					(* pScreen->RegionDestroy)( prgnDst ) ;
					return NULL ;
				}
				for ( pboxBase = pboxNext = pbox ;
				      pboxBase < pbox + nbox ;
				      pboxBase = pboxNext ) {
					while ( pboxNext < pbox + nbox
					     && pboxNext->y1 == pboxBase->y1 )
						pboxNext++ ;
					for ( pboxTmp = pboxNext ;
					      pboxTmp != pboxBase ;
					      *pboxNew2++ = *--pboxTmp )
						/*DO NOTHING*/ ;
				}
				pbox = ( pboxNew2 -= nbox ) ;
			}
		}

		{ /* Here is the "REAL" copy. All clipped and GO. */
		void (*fnp)() ;
		int   index ;
		pgcScreenPtr	pPGCScreen ;

                index = pScreen->myNum ;
		pPGCScreen= &pgcScreenInfo[index];
		fnp = pPGCScreen->blit ;

		for ( ; nbox-- ; pbox++ )
		   (* fnp)( alu, pScreen , pm,
			 pbox->x1 + dx, pbox->y1 + dy,
			 pbox->x1, pbox->y1,
			 pbox->x2 - pbox->x1, pbox->y2 - pbox->y1 ) ;
		}
		/* free up stuff */
		if ( pboxNew1 )
			DEALLOCATE_LOCAL( pboxNew1 ) ;
		if ( pboxNew2 )
			DEALLOCATE_LOCAL( pboxNew2 ) ;

		if ( origSource ) {
			prgnExposed = miHandleExposures(
					pSrcDrawable, pDstDrawable, pGC,
			    		origSource->x, origSource->y,
			    		origSource->width, origSource->height,
			    		origDest->x, origDest->y ) ;
			DEALLOCATE_LOCAL( origSource ) ;
			DEALLOCATE_LOCAL( origDest ) ;
		}
		else
			prgnExposed = (RegionPtr) 0 ;
	}
	else /* nbox == 0 no visable destination region */
		prgnExposed = (RegionPtr) 0 ;

	(* pScreen->RegionDestroy)( prgnDst ) ;

	return prgnExposed ;
}
