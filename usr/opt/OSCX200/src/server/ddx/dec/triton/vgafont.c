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
static char *rcsid = "@(#)$RCSfile: vgafont.c,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/08/02 20:38:32 $";
#endif

#include "X.h"
#include "Xproto.h"
#include "scrnintstr.h"
#include "windowstr.h"
#include "fontstruct.h"
#include "dixfontstr.h"

#include "vga.h"
#include "vgaprocs.h"

Bool
vgaRealizeFont(pScreen, pFont)
     ScreenPtr pScreen;
     FontPtr   pFont;
{
  return TRUE;
}


Bool
vgaUnrealizeFont(pScreen, pFont)
     ScreenPtr pScreen;
     FontPtr   pFont;
{
  return TRUE;
}

