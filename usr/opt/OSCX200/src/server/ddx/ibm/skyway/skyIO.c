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
 * $XConsortium: skyIO.c,v 1.6 91/09/09 13:21:26 rws Exp $
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
 * skyIO.c - initialize display, cfb, cursor, etc.
 */

#include <sys/hft.h>
#include "X.h"
#include "screenint.h"
#include "scrnintstr.h"
#include "cursorstr.h"
#include "pixmapstr.h"
#include "miscstruct.h"
#include "colormap.h"
#include "colormapst.h"
#include "resource.h"

#include "ibmScreen.h"
#include "OSio.h"
#include "ibmTrace.h"

#include "aixCursor.h"

#include "mipointer.h"
#include "misprite.h"
#include "gcstruct.h"

#include "skyHdwr.h"
#include "skyProcs.h"

#include <pgc/pgc.h>
#include <pgc/pgcProcs.h>

extern miPointerScreenFuncRec skyPointerScreenFuncs;
extern pointer pSkywayFrame;
extern pgcScreenRec pgcScreenInfo[] ;

extern Bool skyRealizeCursor();
extern Bool skyUnrealizeCursor();
extern Bool skyDisplayCursor();
extern void miRecolorCursor();

extern void skyValidateGC();
extern void cfbChangeGC(), cfbCopyGC(), cfbDestroyGC();
extern void cfbChangeClip(), cfbDestroyClip(), cfbCopyClip();
/*
 * By overloading the definition of the cfbGCFuncs struct with the
 * skyway ValidateGC proc, which adds the fast CopyArea func into
 * GC's with depth 8, we can do the hardware bitblt rather than the
 * slower software cfbCopyArea.
 *
 * It works, but keep in mind it depends on the libibm.a getting
 * linked ahead of the cfb routines!
 *
 * This is perhaps not the best way to do this , but the sample server has
 * no good way of adding new procs into GCs because the backingstore
 * routines put the cfb procs into a private area.
 */
GCFuncs cfbGCFuncs = {
    skyValidateGC,
    cfbChangeGC,
    cfbCopyGC,
    cfbDestroyGC,
    cfbChangeClip,
    cfbDestroyClip,
    cfbCopyClip,
};
Bool
skyScreenIO(index, pScreen, argc, argv)
    register int        index;
    register ScreenPtr  pScreen;
    int         argc;           /* these two may NOT be changed */
    char        **argv;
{
    ColormapPtr pColormap;
    Bool        retval;
    int         i;
    pgcScreenPtr pPGCScreen ;

    TRACE(("skyIO(%d,0x%x,%d,0x%x)\n",index,pScreen,argc,argv));

    /* Get pointer to the adapter structure. Everthing else can be      */
    /* pointed to from this structure.                                  */

    skyScreenInit(index);
    skyHdwrInit(index);

    pPGCScreen = &pgcScreenInfo[index] ;

    pPGCScreen->blit = SkywayBitBlt ;
    pPGCScreen->solidFill = SkywayFillSolid ;
    pPGCScreen->tileFill = SkywayTileRect ;
    pPGCScreen->setColor = skySetColor ;
    pPGCScreen->imageFill = skywayDrawColorImage ;
    pPGCScreen->imageRead = skywayReadColorImage ;

    pScreen->RealizeCursor=     skyRealizeCursor;
    pScreen->UnrealizeCursor=   skyUnrealizeCursor;
    pScreen->DisplayCursor=     skyDisplayCursor;
    pScreen->RecolorCursor=     miRecolorCursor;

    pScreen->SetCursorPosition= AIXSetCursorPosition;
    pScreen->CursorLimits=      AIXCursorLimits;
    pScreen->PointerNonInterestBox= AIXPointerNonInterestBox;
    pScreen->ConstrainCursor=   AIXConstrainCursor;

    pScreen->QueryBestSize = skyQueryBestSize;

    pScreen->InstallColormap = skyInstallColormap;
    pScreen->UninstallColormap = skyUninstallColormap;
    pScreen->ListInstalledColormaps = skyListInstalledColormaps;
    pScreen->StoreColors = skyStoreColors;

    if ( !cfbScreenInit(pScreen, pSkywayFrame, SKYWAY_WIDTH, SKYWAY_HEIGHT,
	                92, 92, SKYWAY_WIDTH) )
    {
	TRACE (("skyIO: cfbScreenInit failed\n"));
	return FALSE;
    }

    pScreen->PaintWindowBorder 	   = pgcPaintWindow ;
    pScreen->CopyWindow 	   = pgcCopyWindow ;
    pScreen->PaintWindowBackground = pgcPaintWindow ;

    pScreen->CloseScreen   = skyScreenClose;
    pScreen->SaveScreen    = ibmSaveScreen;
    pScreen->BlockHandler  = AIXBlockHandler;
    pScreen->WakeupHandler = AIXWakeupHandler;
    pScreen->blockData     = (pointer)0;
    pScreen->wakeupData    = (pointer)0;

    if ( !cfbCreateDefColormap (pScreen) )
    {
	TRACE (("skyIO: cfbCreateDefColormap failed\n"));
	return FALSE;
    }

    ibmScreen(index) = pScreen;
    return TRUE ;
}
