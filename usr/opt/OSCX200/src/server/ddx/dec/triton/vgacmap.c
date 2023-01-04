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
static char *rcsid = "@(#)$RCSfile: vgacmap.c,v $ $Revision: 1.1.4.3 $ (DEC) $Date: 1993/10/07 18:45:04 $";
#endif

#include <stdio.h>
#include "X.h"
#include "Xproto.h"
#include "scrnintstr.h"
#include "colormapst.h"
#include "resource.h"

#include "vga.h"
#include "vgaprocs.h"

static ColormapPtr pInstalledMap = (ColormapPtr) 0;

#ifndef TRITON
void
vgaStoreColors(pmap, ndef, pdefs)
     ColormapPtr pmap;
     int         ndef;
     xColorItem  *pdefs;
{
  SetPaletteFuncPtr SetPaletteFunc;
  int            idef;

  if (pmap != pInstalledMap)
    return;

  SetPaletteFunc = ((DrawFuncs *)(pmap->pScreen->devPrivate))->SetPaletteFunc;

  for (idef = 0; idef < ndef; idef++) {
    (*SetPaletteFunc)(pdefs[idef].pixel,
		      pdefs[idef].red,
		      pdefs[idef].green,
		      pdefs[idef].blue);
  }

	/* Inform color cursor code that colors have changed */
	colorCursorStoreColors(pmap, ndef, pdefs);
}
#endif /* TRITON */

/*
  pScreen->CreateColormap() is called by DIX CreateColormap() after the 
  colormap has been created.  It is DDX's chance to initialize the
  the colormap.  If this is a Static colormap, this is the time to
  fill in the colormap's values.
  This is a do nothing version of cfbInitializeColormap() in CFB
*/
Bool
vgaCreateColormap(pmap)
     ColormapPtr pmap;
{
  return TRUE;
}

void
vgaDestroyColormap(pmap)
     ColormapPtr pmap;
{
  return;
}

/*
  From CFB
*/
int
vgaListInstalledColormaps(pScreen, pmaps)
     ScreenPtr  pScreen;
     Colormap   *pmaps;
{
  /* By the time we are processing requests, we can guarantee that there
   * is always a colormap installed */
  *pmaps = pInstalledMap->mid;
  return (1);
}

#ifndef TRITON
void
vgaInstallColormap(pmap)
     ColormapPtr        pmap;
{
  SetPaletteFuncPtr SetPaletteFunc;
  int      entries;
  Pixel    pixel;
  xrgb     rgb;
  int	   sts;

  if(pmap == pInstalledMap)
    return;

  SetPaletteFunc = ((DrawFuncs *)(pmap->pScreen->devPrivate))->SetPaletteFunc;

  /* Uninstall pInstalledMap. No hardware changes required, just
   * notify all interested parties. */
  if(pInstalledMap != (ColormapPtr)0)
    WalkTree(pmap->pScreen, TellLostMap, (char *)&pInstalledMap->mid);

  /* Install pmap */
  pInstalledMap = pmap;
  entries = pmap->pVisual->ColormapEntries;
  for (pixel=0; pixel<entries; pixel++) {
    sts = QueryColors(pmap, 1, &pixel, &rgb);
    (*SetPaletteFunc)(pixel, rgb.red, rgb.green, rgb.blue);
  }
  WalkTree(pmap->pScreen, TellGainedMap, (char *)&pmap->mid);

/* Inform color cursor code that the colormap has changed */
/* DEC */	colorCursorInstallColormap(pmap);

}
#endif /* TRITON */

void
vgaRefreshInstalledColormap()
{
  SetPaletteFuncPtr SetPaletteFunc;
  int      entries;
  Pixel    pixel;
  xrgb     rgb;

  SetPaletteFunc =
    ((DrawFuncs *)(pInstalledMap->pScreen->devPrivate))->SetPaletteFunc;

  entries = pInstalledMap->pVisual->ColormapEntries;
  for (pixel=0; pixel<entries; pixel++) {
    QueryColors(pInstalledMap, 1, &pixel, &rgb);
    (*SetPaletteFunc)(pixel, rgb.red, rgb.green, rgb.blue);
  }
}  

void
vgaUninstallColormap(pmap)
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


/*
  From CFB
*/
void
vgaResolveColor(pred, pgreen, pblue, pVisual)
     unsigned short     *pred, *pgreen, *pblue;
     register VisualPtr pVisual;
{
  int shift = 16 - pVisual->bitsPerRGBValue;
  unsigned lim = (1 << pVisual->bitsPerRGBValue) - 1;

  if ((pVisual->class == PseudoColor) || (pVisual->class == DirectColor)) {
    /* rescale to rgb bits */
    *pred = ((*pred >> shift) * 65535) / lim;
    *pgreen = ((*pgreen >> shift) * 65535) / lim;
    *pblue = ((*pblue >> shift) * 65535) / lim;
  }
  else if (pVisual->class == GrayScale) {
    /* rescale to gray then rgb bits */
    *pred = (30L * *pred + 59L * *pgreen + 11L * *pblue) / 100;
    *pblue = *pgreen = *pred = ((*pred >> shift) * 65535) / lim;
  }
  else if (pVisual->class == StaticGray) {
    unsigned limg = pVisual->ColormapEntries - 1;
    /* rescale to gray then [0..limg] then [0..65535] then rgb bits */
    *pred = (30L * *pred + 59L * *pgreen + 11L * *pblue) / 100;
    *pred = ((((*pred * (limg + 1))) >> 16) * 65535) / limg;
    *pblue = *pgreen = *pred = ((*pred >> shift) * 65535) / lim;
  }
  else {
    unsigned limr, limg, limb;
    
    limr = pVisual->redMask >> pVisual->offsetRed;
    limg = pVisual->greenMask >> pVisual->offsetGreen;
    limb = pVisual->blueMask >> pVisual->offsetBlue;
    /* rescale to [0..limN] then [0..65535] then rgb bits */
    *pred = ((((((*pred * (limr + 1)) >> 16) *
                65535) / limr) >> shift) * 65535) / lim;
    *pgreen = ((((((*pgreen * (limg + 1)) >> 16) *
                  65535) / limg) >> shift) * 65535) / lim;
    *pblue = ((((((*pblue * (limb + 1)) >> 16) *
                 65535) / limb) >> shift) * 65535) / lim;
  }
}


/*
  From CFB
*/
Bool
vgaCreateDefColormap(pScreen)
     ScreenPtr pScreen;
{
  unsigned short        zero = 0, ones = ~0;
  VisualPtr     pVisual;
  ColormapPtr   cmap;
    
  for (pVisual = pScreen->visuals;
       pVisual->vid != pScreen->rootVisual;
       pVisual++)
    ;

  if (CreateColormap(pScreen->defColormap, pScreen, pVisual, &cmap,
                     (pVisual->class & DynamicClass) ? AllocNone : AllocAll,
                     0)
      != Success)
    return FALSE;
  if ((AllocColor(cmap, &ones, &ones, &ones, &(pScreen->whitePixel), 0)
       != Success) ||
      (AllocColor(cmap, &zero, &zero, &zero, &(pScreen->blackPixel), 0)
       != Success))
    return FALSE;
  (*pScreen->InstallColormap)(cmap);
  return TRUE;
}
