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
static char *rcsid = "@(#)$RCSfile: wgacmap.c,v $ $Revision: 1.1.4.3 $ (DEC) $Date: 1993/10/07 18:45:59 $";
#endif

/*
 *	This module has most of the routines that are used
 *	to manipulate the colormaps for the Qvision.
 *
 *	Written: 1-May-1993, Henry R. Tumblin
 */

#include "X.h"
#include "Xproto.h"
#include "scrnintstr.h"
#include "colormapst.h"
#include "resource.h"

#include "vga.h"
#include "vgaprocs.h"
#include "wga.h"
#include "wgaio.h"

#if defined(DWDOS286)
#include "conio.h"
#endif

#if defined(DWDOS386)    
#define CLI _inline(0xfa)
#define STI _inline(0xfb)
#endif

static ColormapPtr pInstalledMap = (ColormapPtr) 0;

void
pwgaStoreColors(pmap, ndef, pdefs)
     ColormapPtr pmap;
     int         ndef;
     xColorItem  *pdefs;
{
  int            idef;

  if (pmap != pInstalledMap)
    return;

  for (idef = 0; idef < ndef; idef++) {
    outp (PALETTE_WRITE, pdefs[idef].pixel);
    outp (PALETTE_DATA, ((pdefs[idef].red >> 10) * 0xff));
    outp (PALETTE_DATA, ((pdefs[idef].green >> 10) * 0xff));
    outp (PALETTE_DATA, ((pdefs[idef].blue >> 10) * 0xff));

  }
}


/*
  pScreen->CreateColormap() is called by DIX CreateColormap() after the 
  colormap has been created.  It is DDX's chance to initialize the
  the colormap.  If this is a Static colormap, this is the time to
  fill in the colormap's values.
  This is a do nothing version of cfbInitializeColormap() in CFB
*/
Bool
pwgaCreateColormap(pmap)
     ColormapPtr pmap;
{
  return TRUE;
}

void
pwgaDestroyColormap(pmap)
     ColormapPtr pmap;
{
  return;
}


/*
  From CFB
*/
int
pwgaListInstalledColormaps(pScreen, pmaps)
     ScreenPtr  pScreen;
     Colormap   *pmaps;
{
  /* By the time we are processing requests, we can guarantee that there
   * is always a colormap installed */
  *pmaps = pInstalledMap->mid;
  return (1);
}


void
pwgaInstallColormap(pmap)
     ColormapPtr        pmap;
{
  int      entries;
  Pixel    pixel;
  xrgb     rgb;

  if(pmap == pInstalledMap)
    return;

  /* Uninstall pInstalledMap. No hardware changes required, just
   * notify all interested parties. */
  if(pInstalledMap != (ColormapPtr)0)
    WalkTree(pmap->pScreen, TellLostMap, (char *)&pInstalledMap->mid);

  /* Install pmap */
  pInstalledMap = pmap;
  entries = pmap->pVisual->ColormapEntries;
  for (pixel=0; pixel<entries; pixel++) {
    QueryColors(pmap, 1, &pixel, &rgb);
#if defined(DWDOS386)    
    CLI;
#endif
    outp (PALETTE_WRITE, pixel);
    outp (PALETTE_DATA, ((rgb.red >> 10) & 0xff));
    outp (PALETTE_DATA, ((rgb.green >> 10) & 0xff));
    outp (PALETTE_DATA, ((rgb.blue >> 10) & 0xff));

#if defined(DWDOS386)    
    STI;
#endif
  }
  WalkTree(pmap->pScreen, TellGainedMap, (char *)&pmap->mid);
}


void
pwgaUninstallColormap(pmap)
     ColormapPtr        pmap;
{
  ColormapPtr defColormap;
  
  if (pmap != pInstalledMap)
    return;
  
  defColormap = (ColormapPtr) LookupIDByType(pmap->pScreen->defColormap,
                                             RT_COLORMAP);
  if (defColormap == pInstalledMap)
    return;

  (*pmap->pScreen->InstallColormap)(defColormap);
}
