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
static char SccsId[] = "@(#)gesegcr.c	1.2\t7/10/89";
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
**	GESEGCR			Does reqd bookeeping for creating a new segment
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
**	GNE 01/22/87 Created                            
**
**--
**/

#include "geGks.h"


extern	char          *geMalloc();
extern  struct GE_SEG *geGenSegL();

geSegCr()
{                                                                         
/*
 * If at least one segment in the chain has already been allocated -
 *indicated by the last segment being other than NULL, then update
 *Next and Previous pointers following allocation of current
 *segment.  Otherwise just null these pointers
 */
if (geState.ASegP = geGenSegL())
   {geState.ASegP->Next      	= (struct GE_SEG *)
      				geMalloc(sizeof(struct GE_SEG));

    geState.ASegP->Next->Prev 	= geState.ASegP;
    geState.ASegP            	= geState.ASegP->Next;
    geState.ASegP->Z            = geState.ASegP->Prev->Z + 1;
   }	
else
   {geState.ASegP = geSeg0 	= (struct GE_SEG *)
				geMalloc(sizeof(struct GE_SEG));
    geState.ASegP->Prev 	= NULL;
    geState.ASegP->Z            = 0;
   }	

geState.ASegP->Next			= NULL;
geState.ASegP->Xref			= NULL;
geState.ASegP->Private          	= NULL;
geState.ASegP->Col              	= geAttr.FilCol;
geState.ASegP->FillHT			= geAttr.FillHT;
geState.ASegP->FillStyle		= geAttr.FillStyle;
geState.ASegP->FillWritingMode		= geAttr.FillWritingMode;
geState.ASegP->Live             	= TRUE;
geState.ASegP->Visible          	= TRUE;
geState.ASegP->InFrame          	= TRUE;
geState.ASegP->WhiteOut         	= geState.WhiteOut;
geState.ASegP->ConLine          	= FALSE;
geState.ASegP->Talk          		= FALSE;
geState.ASegP->HotSpot          	= FALSE;
geState.ASegP->Co.x1            	= geState.ASegP->Co.y1 = 0;
geState.ASegP->Co.x2            	= geState.ASegP->Co.y2 = 0;
geState.ASegP->Descrip          	= NULL;
/*
 * Animation controls
 */
geState.ASegP->AnimCtrl.NumPackets      = 0;
geState.ASegP->AnimCtrl.CurFrame        = -1;
geState.ASegP->AnimCtrl.FirstFrame      = 0;
geState.ASegP->AnimCtrl.NumFrames       = 0;
geState.ASegP->AnimCtrl.LastFrame	= 0;
geState.ASegP->AnimCtrl.NumFramesRemaining = 0;
geState.ASegP->AnimCtrl.InterruptResponse = 0;
geState.ASegP->AnimCtrl.SumDx           = 0;
geState.ASegP->AnimCtrl.SumDy           = 0;
geState.ASegP->AnimCtrl.SumMagFX        = 100.0;
geState.ASegP->AnimCtrl.SumMagFY        = 100.0;
geState.ASegP->AnimCtrl.SumRot          = 0.0;
geState.ASegP->AnimCtrl.Frames          = NULL;
geState.ASegP->AnimCtrl.Animate         = FALSE;
geState.ASegP->AnimCtrl.Animated        = geState.ASegP->AnimCtrl.Animate;
geState.ASegP->AnimCtrl.Interrupt	= TRUE;
geState.ASegP->AnimCtrl.Interrupted	= FALSE;
geState.ASegP->AnimCtrl.Block		= FALSE;
geState.ASegP->AnimCtrl.Blocked		= FALSE;
geState.ASegP->AnimCtrl.StartForward    = TRUE;
geState.ASegP->AnimCtrl.CurDirForward   = geState.ASegP->AnimCtrl.StartForward;
geState.ASegP->AnimCtrl.ReverseAtEnd    = FALSE;
geState.ASegP->AnimCtrl.CdispOverride	= FALSE;
geState.ASegP->AnimCtrl.CZupdisp	= FALSE;
geState.ASegP->AnimCtrl.FastPlay	= FALSE;
geState.ASegP->AnimCtrl.Paused		= FALSE;

}
