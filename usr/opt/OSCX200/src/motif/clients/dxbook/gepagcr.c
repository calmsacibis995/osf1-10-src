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
static char SccsId[] = "@(#)gepagcr.c	1.4\t5/25/89";
/*
*****************************************************************************
**                                                                          *
**                         COPYRIGHT (c) 1988 BY                            *
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
*****************************************************************************
**++
**  FACILITY:
**
**	GEPAGCR			Does reqd bookeeping for creating a new page
**
**  ABSTRACT:
**
**
**  ENVIRONMENT:
**
**	VAX/VMS, ULTRIX Operating Systems
**
**  MODIFICATION HISTORY:
**
**	GNE 10/05/87 Created                            
**
**--
**/

#include "geGks.h"


extern	char          *geMalloc();
extern  struct GE_SEG *geGenSegL();
extern  struct GE_PAG *geGenPagL();

gePagCr(PagName)
char	*PagName;
{                                                                         
/*
 * If at least one page in the chain has already been allocated -
 *indicated by the last page being other than NULL, then update
 *Next and Previous pointers following allocation of current
 *page.  Otherwise just null these pointers
 */
if (geState.APagP = geGenPagL())
   {geState.APagP->Next      	= (struct GE_PAG *)
      				geMalloc(sizeof(struct GE_PAG));

    geState.APagP->Next->Prev 	= geState.APagP;
    geState.APagP            	= geState.APagP->Next;
   }	
else
   {geState.APagP = gePag0 	= (struct GE_PAG *)
				geMalloc(sizeof(struct GE_PAG));
    geState.APagP->Prev 	= NULL;
   }	

geState.APagP->Next		= NULL;
geGrf(GECREATE, NULL);                          /* Create a blank graph      */
geState.APagP->Seg0 = geSeg0	= geGenSegL();	/* Page root segment         */
if (PagName)
   {geState.APagP->Name		= geMalloc(strlen(PagName) + 1);
    strcpy(geState.APagP->Name, PagName);
   }
else if (geDefRags)
   {geState.APagP->Name		= geMalloc(strlen(geDefRags) + 1);
    strcpy(geState.APagP->Name, geDefRags);
   }
else
  {geState.APagP->Name		= geMalloc(3);
   strcpy(geState.APagP->Name, "  ");
  }

geState.APagP->Descrip		= NULL;

GEVEC(GEBOUNDS, geSeg0);
geState.APagP->Crop.x1 		= geState.APagP->Crop.y1 =
geState.APagP->Crop.x2 		= geState.APagP->Crop.y2 = 0;
geState.APagP->Grid             = geState.Grid;
geState.APagP->Grid.MajorX	= geState.APagP->Grid.DivX *
                                  geState.APagP->Grid.MinorX;
geState.APagP->Grid.MajorY	= geState.APagP->Grid.DivY *
                                  geState.APagP->Grid.MinorY;
geState.APagP->Zoom.ZoomF	= 100;
geState.APagP->Zoom.ZoomCum	= 100.;
geState.APagP->XDoP		= NULL;
geState.APagP->Surf.self	= NULL;
geState.APagP->Modified		= FALSE;
geState.APagP->PixmapAnim	= TRUE;
geState.APagP->Pixmap		= NULL;
geState.APagP->ViewPortXoff	= 0;
geState.APagP->ViewPortYoff	= 0;
geState.APagP->ViewPortWidth	= 0;
geState.APagP->ViewPortHeight	= 0;
geState.APagP->Surf.background_pixel   = geAttr.PagCol.RGB.pixel;
geState.APagP->Seg0->Col        = geAttr.PagCol;
geState.APagP->Seg0->FillHT     = geAttr.PagHT;
geState.APagP->Seg0->FillStyle  = geAttr.PagStyle;
}
