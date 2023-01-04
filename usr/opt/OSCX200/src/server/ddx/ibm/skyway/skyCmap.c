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
 * $XConsortium: skyCmap.c,v 1.2 91/07/16 13:13:47 jap Exp $
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

/*
 * skyCmap.c - colormap routines
 *             copied from ibm/pgc/pgcCmap.c
 *             revised to be just for skyway
 */

#define NEED_EVENTS

#include "X.h"
#include "Xproto.h"
#include "resource.h"
#include "misc.h"
#include "scrnintstr.h"
#include "screenint.h"
#include "colormapst.h"
#include "colormap.h"
#include "pixmapstr.h"

#include "OScompiler.h"
#include "ibmTrace.h"

/* remember the ONE installed colormap here */
static ColormapPtr  InstalledColormap = NULL;

void
skyStoreColors( pmap, ndef, pdefs )
register ColormapPtr pmap ;
register int ndef ;
register xColorItem *pdefs ;
{
   register int i ;
   int index ;

   TRACE(("skyStoreColors(pmap=x%x)\n", pmap)) ;
   if ( pmap != InstalledColormap )
	return;

   index = pmap->pScreen->myNum ;

   while ( ndef-- ) {
	i = pdefs->pixel;
	skySetColor(i, pdefs->red, pdefs->green, pdefs->blue, index);
	pdefs++;
    }
}

int
skyListInstalledColormaps( pScreen, pCmapList )
register ScreenPtr pScreen ;
register Colormap *pCmapList ;
{
   int index ;

   TRACE(("skyListInstalledColormaps(InstalledColormap=x%x)\n", InstalledColormap)) ;
   index = pScreen->myNum ;

   if ( InstalledColormap ) {
	        *pCmapList = InstalledColormap->mid ;
	        return 1 ;
	}
	else {
	        *pCmapList = 0 ;
	        return 0 ;
	}
}

void
skyInstallColormap( pColormap )
register ColormapPtr pColormap ;
{
    register int i ;
    register Entry *pent;
    register VisualPtr pVisual = pColormap->pVisual;
    int index ;
    ScreenPtr pScreen = pColormap->pScreen ;
    unsigned int red, green, blue;

   TRACE(("skyInstallColormap(pColormap=x%x)\n", pColormap)) ;

   index = pScreen->myNum ;

   if ( !pColormap ) /* Re-Install of NULL map ?? */
	return ;

    if ((pVisual->class | DynamicClass) == DirectColor) {
	for (i = 0; i < 256; i++) {
	    pent = &pColormap->red[(i & pVisual->redMask) >>
	                      pVisual->offsetRed];
	    red = pent->co.local.red;
	    pent = &pColormap->green[(i & pVisual->greenMask) >>
	                        pVisual->offsetGreen];
	    green = pent->co.local.green;
	    pent = &pColormap->blue[(i & pVisual->blueMask) >>
	                       pVisual->offsetBlue];
	    blue = pent->co.local.blue;
	    skySetColor( i, red, green, blue, index );
	}
    } else {
	for (i = 0, pent = pColormap->red;
	     i < pVisual->ColormapEntries;
	     i++, pent++) {
	    if (pent->fShared) {
	        red = pent->co.shco.red->color;
	        green = pent->co.shco.green->color;
	        blue = pent->co.shco.blue->color;
	    }
	    else {
	        red = pent->co.local.red;
	        green = pent->co.local.green;
	        blue = pent->co.local.blue;
	    }
	    skySetColor( i, red, green, blue, index );
	}
    }

   if ( InstalledColormap != pColormap ) {
	if (InstalledColormap ) /* Remember it may not exist */
	    WalkTree( pScreen, TellLostMap,
	              &(InstalledColormap->mid) ) ;
	InstalledColormap = pColormap ;
	WalkTree( pScreen, TellGainedMap, &(pColormap->mid) ) ;
	/* should really recolor cursor here for new color map ??? */
    }

   return ;
}

void
skyUninstallColormap( pColormap )
register ColormapPtr pColormap ;
{
   register ColormapPtr pCmap ;

   TRACE(("skyUninstallColormap(pColormap=x%x)\n", pColormap)) ;
   if (pColormap != InstalledColormap)
	return;

   pCmap = (ColormapPtr) LookupIDByType( pColormap->pScreen->defColormap,
	                           RT_COLORMAP ) ;
   if ( pCmap != pColormap ) /* never uninstall the default map */
	(* pColormap->pScreen->InstallColormap)( pCmap ) ;

   return ;
}


void
skyRefreshColormaps(pScreen)
ScreenPtr pScreen ;
{
   TRACE(("skyRefreshColormaps(pColormap=x%x)\n", InstalledColormap)) ;
   skyInstallColormap(InstalledColormap);
}
