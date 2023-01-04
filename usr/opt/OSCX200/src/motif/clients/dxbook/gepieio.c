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
static char SccsId[] = "@(#)gepieio.c	1.3\t4/17/89";
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
**	GEPIEIO	             	       Pie object IO handler
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
**	GNE 03/07/88 Created
**
**--
**/

#include "geGks.h"


extern char  *geMalloc();

gePieIO(cmd, ASegP)
long     	cmd;
struct GE_SEG	*ASegP;
{                       
int              i;
struct GE_PIE   *priv;

priv = (struct GE_PIE *)ASegP->Private;

if (cmd == GEWRITE)
  {if (geVersionOut == GECURRENTVERSION)
     {/*
       * Writing out LATEST version
       */
      if (geGenSIOTM (cmd, &geCTM))  		                return;
      fprintf(geFdO, " %d %d %d %d %d %d %d %d %d",
	      ASegP->Z,
	      priv->Pt0.x,
	      priv->Pt0.y,
	      priv->Pta.x,
	      priv->Pta.y,
	      priv->Ptm.x,
	      priv->Ptm.y,
	      priv->Ptb.x,
	      priv->Ptb.y);
      if (geGenSIOCol(cmd, &priv->Col))                         return;
      if (geGenSIOCol(cmd, &ASegP->Col))                        return;
      if (geGenSIOWP (cmd, &priv->LinW, &priv->Pat))            return;
      if (geGenSIOFil(cmd, &ASegP->FillStyle, &ASegP->FillHT))  return;
      fprintf(geFdO, " %d", ASegP->FillWritingMode);
      if (geGenSIOFil(cmd, &priv->LineStyle, &priv->LineHT))    return;
      fprintf(geFdO, " %d %d %d",
	      priv->WritingMode,
	      priv->CapStyle,
	      priv->JoinStyle);
      geGenSIOFlags(cmd, ASegP);
      if (geGenSIODesc(cmd, ASegP))			        return;
      if (geGenSIOAnim(cmd, ASegP))				return;
     }
  else                                         /* Version 5                 */
     {fprintf(geFdO, " %d %d %d %d %d %d %d %d %d %d",
	      ASegP->Z,
	      ASegP->Visible,
	      priv->Pt0.x,
	      priv->Pt0.y,
	      priv->Pta.x,
	      priv->Pta.y,
	      priv->Ptm.x,
	      priv->Ptm.y,
	      priv->Ptb.x,
	      priv->Ptb.y);
      if (geGenSIOCol(cmd, &priv->Col))                         return;
      if (geGenSIOCol(cmd, &ASegP->Col))                        return;
      if (geGenSIOWP (cmd, &priv->LinW, &priv->Pat))            return;
      if (geGenSIOFil(cmd, &ASegP->FillStyle, &ASegP->FillHT))  return;
      fprintf(geFdO, " %d", ASegP->FillWritingMode);
      if (geGenSIOFil(cmd, &priv->LineStyle, &priv->LineHT))    return;
      fprintf(geFdO, " %d %d %d",
	      priv->WritingMode,
	      priv->CapStyle,
	      priv->JoinStyle);
     }

   return;
  }

/*
 * Must be reading in - the first case = latest version
 */
switch (geVersionIn)
  {case 8:
   case 7:
     /*
      * Diff between 7 and prev - this contains the transformation matrix
      * associated with the object.
      */
     ASegP->Private    = geMalloc(sizeof(struct GE_PIE));
     priv 		  = (struct GE_PIE *)ASegP->Private;
     if (geGenSIOTM (cmd, &geCTM))  		                return;
     geFscanf9(geFdI, " %d %d %d %d %d %d %d %d %d",
		&ASegP->Z,
		&priv->Pt0.x,
		&priv->Pt0.y,
		&priv->Pta.x,
		&priv->Pta.y,
		&priv->Ptm.x,
		&priv->Ptm.y,
		&priv->Ptb.x,
		&priv->Ptb.y);
     geGenRC(priv->Pta.x,priv->Pta.y, priv->Ptm.x,priv->Ptm.y,
	     priv->Ptb.x,priv->Ptb.y, &priv->C.x,&priv->C.y, &priv->Rad,
	     &priv->Alpha, &priv->Beta);
     if (geGenSIOCol(cmd, &priv->Col))               break;
     if (geGenSIOCol(cmd, &ASegP->Col))              break;
     if (geGenSIOWP (cmd, &priv->LinW, &priv->Pat)) break;
     if (geGenSIOFil(cmd, &ASegP->FillStyle, &ASegP->FillHT))  break;
     geFscanf1(geFdI, " %d",&ASegP->FillWritingMode);
     if (geGenSIOFil(cmd, &priv->LineStyle, &priv->LineHT))    break;
     geFscanf3(geFdI, " %d %d %d",
	       &priv->WritingMode,
	       &priv->CapStyle,
	       &priv->JoinStyle);
     geGenSIOFlags(cmd, ASegP);
     if (geGenSIODesc(cmd, ASegP))			       break;
     if (geGenSIOAnim(cmd, ASegP))			       break;
     break;

   case 6:
   case 5:
   case 4:
   case 3:
        /*
         * Diff between 3 and 2:
         *    - 3 is reading and writing the Visible flag
         */
        ASegP->Private    = geMalloc(sizeof(struct GE_PIE));
	priv 		  = (struct GE_PIE *)ASegP->Private;
	geFscanf10(geFdI, " %d %d %d %d %d %d %d %d %d %d",
		   &ASegP->Z,
		   &i,
		   &priv->Pt0.x,
		   &priv->Pt0.y,
		   &priv->Pta.x,
		   &priv->Pta.y,
		   &priv->Ptm.x,
		   &priv->Ptm.y,
		   &priv->Ptb.x,
		   &priv->Ptb.y);
        ASegP->Visible = i;
	geGenRC(priv->Pta.x,priv->Pta.y, priv->Ptm.x,priv->Ptm.y,
		priv->Ptb.x,priv->Ptb.y, &priv->C.x,&priv->C.y, &priv->Rad,
		&priv->Alpha, &priv->Beta);
	if (geGenSIOCol(cmd, &priv->Col))               break;
	if (geGenSIOCol(cmd, &ASegP->Col))              break;
	if (geGenSIOWP (cmd, &priv->LinW, &priv->Pat)) break;
	if (geGenSIOFil(cmd, &ASegP->FillStyle, &ASegP->FillHT))  break;
	geFscanf1(geFdI, " %d",&ASegP->FillWritingMode);
	if (geGenSIOFil(cmd, &priv->LineStyle, &priv->LineHT))    break;
	geFscanf3(geFdI, " %d %d %d",
	       &priv->WritingMode,
	       &priv->CapStyle,
	       &priv->JoinStyle);
	break;

    case 2:
    case 1:
        ASegP->Private    = geMalloc(sizeof(struct GE_PIE));
	priv 		  = (struct GE_PIE *)ASegP->Private;
	geFscanf9(geFdI, " %d %d %d %d %d %d %d %d %d",
		  &ASegP->Z,
		  &priv->Pt0.x,
		  &priv->Pt0.y,
		  &priv->Pta.x,
		  &priv->Pta.y,
		  &priv->Ptm.x,
		  &priv->Ptm.y,
		  &priv->Ptb.x,
		  &priv->Ptb.y);
	geGenRC(priv->Pta.x,priv->Pta.y, priv->Ptm.x,priv->Ptm.y,
		priv->Ptb.x,priv->Ptb.y, &priv->C.x,&priv->C.y, &priv->Rad,
		&priv->Alpha, &priv->Beta);
	if (geGenSIOCol(cmd, &priv->Col))               break;
	if (geGenSIOCol(cmd, &ASegP->Col))              break;
	if (geGenSIOWP (cmd, &priv->LinW, &priv->Pat)) break;
	if (geGenSIOFil(cmd, &ASegP->FillStyle, &ASegP->FillHT))  break;
	geFscanf3(geFdI, " %d %d %d",
		  &priv->WritingMode,
		  &priv->CapStyle,
		  &priv->JoinStyle);
        ASegP->Visible         = TRUE;
        ASegP->FillWritingMode = GXcopy;
	if (!priv->LinW)
	  priv->LineStyle        = GETRANSPARENT;
	else
	  priv->LineStyle        = FillSolid;
        priv->LineHT           = 100;
	break;

    default:
	break;
   }
/*
 * These did not exist prior to version 7 - the equivalent is that they
 * were OFF.
 */
if (geVersionIn < 7)
  {ASegP->WhiteOut = FALSE;
   ASegP->ConLine  = FALSE;
  }
/* Kludge BL 10 (at least) X server crashes on "vertical, patterned lines" with
 * "LineDoubleDash" - this isn't being used anyway, since PostScript can't do
 * it, so snag it here and convert it to "LineOnOffDash"
 */
if (priv->Pat.LinP == LineDoubleDash) priv->Pat.LinP = LineOnOffDash;

/*
 * The following is for the benefit of MOPS - it does not deal in FillSolid
 */
if (ASegP->FillStyle == FillSolid)
  {ASegP->FillStyle = FillOpaqueStippled;
   ASegP->FillHT    = 100;
 }
if (priv->LineStyle == FillSolid)
  {priv->LineStyle = FillOpaqueStippled;
   priv->LineHT    = 100;
 }

}
