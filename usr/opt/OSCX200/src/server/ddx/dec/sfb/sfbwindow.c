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
#define MODULE_NAME SFBWINDOW
#include "module_ident.h"
#endif
/****************************************************************************
**                                                                          *
**                 COPYRIGHT (c) 1988, 1989, 1990, 1991 BY                  *
**              DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.               *
**			     ALL RIGHTS RESERVED                            *
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
/*
**++
**  FACILITY:
**
**      DDXSFB - VMS SFB server
**
**  ABSTRACT:
**
**
**  AUTHORS:
**
**      Irene McCartney (from Joel McCormack)
**
**  CREATION DATE:     19-Nov-1991
**
**  MODIFICATION HISTORY:
**
** X-002	BIM0011		Irene McCartney			27-Jan-1992
**		Add edit history
**		Merge latest changes from Joel
**
**--
**/


#include "X.h"
#include "scrnintstr.h"
#include "windowstr.h"
#include "mistruct.h"
#include "regionstr.h"

#include "sfb.h"
#include "sfbwindow.h"
#include "sfbblt.h"

extern WindowPtr *WindowTable;

void sfbCopyWindow(pWin, ptOldOrg, prgnSrc)
    WindowPtr pWin;
    DDXPointRec ptOldOrg;
    RegionPtr prgnSrc;
{
    DDXPointPtr		    pptSrc;
    register DDXPointPtr    ppt;
    RegionRec		    rgnDst;
    register BoxPtr	    pbox;
    register int	    dx, dy;
    register int	    nbox;
    WindowPtr		    pwinRoot;
    sfbScreenPrivPtr	    scrPriv;
    SFB			    sfb;

    pwinRoot = WindowTable[pWin->drawable.pScreen->myNum];

    (* pWin->drawable.pScreen->RegionInit) (&rgnDst, NullBox, 0);

    dx = ptOldOrg.x - pWin->drawable.x;
    dy = ptOldOrg.y - pWin->drawable.y;
    (* pWin->drawable.pScreen->TranslateRegion)(prgnSrc, -dx, -dy);
    (* pWin->drawable.pScreen->Intersect)(&rgnDst, &pWin->borderClip, prgnSrc);

    pbox = REGION_RECTS(&rgnDst);
    nbox = REGION_NUM_RECTS(&rgnDst);
    if(!nbox || 
	!(pptSrc = (DDXPointPtr )ALLOCATE_LOCAL(nbox * sizeof(DDXPointRec))))
    {
	(* pWin->drawable.pScreen->RegionUninit) (&rgnDst);
	return;
    }

    for (ppt = pptSrc; nbox != 0; nbox--, ppt++, pbox++) {
        ppt->x = pbox->x1 + dx;
        ppt->y = pbox->y1 + dy;
    }
 
    scrPriv = SFBSCREENPRIV(pwinRoot->drawable.pScreen);
    scrPriv->lastGC = (GCPtr)NULL;
    sfb = scrPriv->sfb;
    SFBROP(sfb, GXcopy);
    SFBPLANEMASK(sfb, SFBBUSALL1);
    sfbBitbltScrScr((DrawablePtr)pwinRoot, (DrawablePtr)pwinRoot,
		    &rgnDst, pptSrc);
    DEALLOCATE_LOCAL(pptSrc);
    (* pWin->drawable.pScreen->RegionUninit)(&rgnDst);
}

