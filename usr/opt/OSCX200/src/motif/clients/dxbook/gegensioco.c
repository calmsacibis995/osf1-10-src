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
static char SccsId[] = "@(#)gegensioco.c	1.1\t11/22/88";
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
**	GEGENSIOCO	             	       Read-Write Coords
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
**	GNE 03/19/87 Created
**
**--
**/

#include "geGks.h"
#include <ctype.h>
#include <math.h>

extern	char          *geMalloc();

geGenSIOCo(cmd, P)
long     	cmd;
struct GE_CO	*P;
{                       
char	*error_string;

switch(cmd)
   {case GEWRITE:
	fprintf(geFdO, " %d %d %d %d",
		P->x1,
		P->y1,
		P->x2,
		P->y2);
	break;

    case GEREAD:
	  geFscanf4(geFdI, " %d %d %d %d",
		    &P->x1,
		    &P->y1,
		    &P->x2,
		    &P->y2);
	if (geFeof(geFdI)) 
 	  {error_string = (char *) geFetchLiteral("GE_ERR_BADMETA", MrmRtypeChar8);
           if (error_string != NULL) 
	     {geError(error_string, FALSE);
	      XtFree(error_string);
	     }
	   return(GERRBADMETA);
          }
	break;

    default:
	break;
   }
return(0);
}

geGenSIOTM(cmd, P)
long     	cmd;
struct GE_TM	*P;
{                       
char	*error_string;
int     c;

switch(cmd)
   {case GEWRITE:
        if (P->a == 1.0 && !P->b  && !P->c  &&
	    P->d == 1.0 && !P->tx && !P->ty)
	  fprintf(geFdO, " []");
	else
	  fprintf(geFdO, " [%f %f %f %f %f %f]",
		  P->a,
		  P->b,
		  P->c,
		  P->d,
		  P->tx,
		  P->ty);
	break;

    case GEREAD:
	c = geFgetc(geFdI);                     /* Get parm seperator - " "  */
	if (geFeof(geFdI) || (char)c != ' ')
	  {geFclose(geFdI);                     /* Some problem              */
	   return(GERRBADMETA);
	  }

	c = geFgetc(geFdI);                  	/* Get matrix start char     */
	if (geFeof(geFdI) || c != '[')          /* Something wrong with file */
	  {geErrorReport(GERRBADMETA, FALSE);
	   return(GERRBADMETA);
	   break;
	  }
	/*
	 * Test the next character for being "]", which indicates this is
	 * the UNITY transform and there is no data associated with it.
	 */
	c = geFgetc(geFdI);
	if (geFeof(geFdI))          		/* Something wrong with file */
	  {geErrorReport(GERRBADMETA, FALSE);
	   return(GERRBADMETA);
	   break;
	  }

	if (c == ']')          			/* UNITY transform           */
	  {P->a  = 1.;
	   P->b  = 0.;
	   P->c  = 0.;
	   P->d  = 1.;
	   P->tx = 0.;
	   P->ty = 0.;
	   break;
	  }
	/*
	 * Data coming up - put test char back.
	 */
	geUngetc((char)c, geFdI);
	/*
	 * Read the matrix values
	 */
	geFscanff6(geFdI, "%f %f %f %f %f %f",
		  &P->a,
		  &P->b,
		  &P->c,
		  &P->d,
		  &P->tx,
		  &P->ty);
	if (geFeof(geFdI)) 
	  {geErrorReport(GERRBADMETA, FALSE);
	   return(GERRBADMETA);
	  }
	c = geFgetc(geFdI);                  	/* Get matrix end char       */
	if (geFeof(geFdI) || c != ']')          /* Something wrong with file */
	  {geErrorReport(GERRBADMETA, FALSE);
	   return(GERRBADMETA);
	   break;
	  }
	break;

    default:
	break;
   }
return(0);
}

geGenSIOFlags(cmd, ASegP)
long     	cmd;
struct GE_SEG	*ASegP;
{                       
char	*error_string;
int     c;
long	Flags;

switch(cmd)
   {case GEWRITE:
	/*
	 * Pack up the flags
	 */
        Flags = 0;
	if (ASegP->Visible)   Flags |= GEBIT_SEG_VISIBLE;
	if (ASegP->WhiteOut)  Flags |= GEBIT_SEG_WHITEOUT;
	if (ASegP->ConLine)   Flags |= GEBIT_SEG_CONLINE;
	if (ASegP->Talk)      Flags |= GEBIT_SEG_TALK;
	if (ASegP->HotSpot)   Flags |= GEBIT_SEG_HOTSPOT;
	fprintf(geFdO, " %d\n",
		Flags);
	break;

    case GEREAD:
	/*
	 * Read the flags
	 */
	geFscanf1(geFdI, "%d",
		  &Flags);

	c = geFgetc(geFdI);                     /* Read past new line        */

	if (geFeof(geFdI)) 
	  {geErrorReport(GERRBADMETA, FALSE);
	   return(GERRBADMETA);
	  }
	/*
	 * Unpack the flags
	 */
	ASegP->Visible   = (Flags & GEBIT_SEG_VISIBLE   ? TRUE : FALSE);
	ASegP->WhiteOut  = (Flags & GEBIT_SEG_WHITEOUT  ? TRUE : FALSE);
	ASegP->ConLine   = (Flags & GEBIT_SEG_CONLINE   ? TRUE : FALSE);
	ASegP->Talk      = (Flags & GEBIT_SEG_TALK      ? TRUE : FALSE);
	ASegP->HotSpot   = (Flags & GEBIT_SEG_HOTSPOT   ? TRUE : FALSE);

	break;

    default:
	break;
   }
return(0);
}

geGenSIOAnim(cmd, P)
long     	cmd;
struct GE_SEG	*P;
{                       
int     i,c;
long	Flags;
float	DrawingRes, ResScaleFactor, ft;


DrawingRes = (float)geState.APagP->PixWidth /
             ((float)geState.APagP->MMWidth / 25.4);

if (fabs(geRun.CurRes - DrawingRes) > GEMIN_DPI_DELTA)
  ResScaleFactor = geRun.CurRes / DrawingRes;
else
  ResScaleFactor = 0.0;

switch(cmd)
   {case GEWRITE:
	/*
	 * Pack up the control structure flags
	 */
        Flags = 0;
	if (P->AnimCtrl.Animate)  	Flags |= GEANIMCTRL_ANIMATE;
	if (P->AnimCtrl.Interrupt)	Flags |= GEANIMCTRL_INTERRUPT;
	if (P->AnimCtrl.Block)		Flags |= GEANIMCTRL_BLOCK;
	if (P->AnimCtrl.StartForward)  	Flags |= GEANIMCTRL_STARTFORWARD;
	if (P->AnimCtrl.ReverseAtEnd)  	Flags |= GEANIMCTRL_REVERSEATEND;
	if (P->AnimCtrl.CdispOverride)	Flags |= GEANIMCTRL_CDISPOVERRIDE;
	if (P->AnimCtrl.CZupdisp)       Flags |= GEANIMCTRL_CZUPDISP;
	/*
	 * Write out the control structure data
	 */
	fprintf(geFdO, "%d %d %d %d %d %d\n",
		P->AnimCtrl.FirstFrame,
		P->AnimCtrl.NumFrames,
		P->AnimCtrl.LastFrame,
		Flags,
		P->AnimCtrl.InterruptResponse,
		P->AnimCtrl.NumPackets);
	/*
	 * Write out animation frames, if any
	 */
	if (P->AnimCtrl.NumPackets > 0)
	  {for (i = 0; i < P->AnimCtrl.NumPackets; i++)
	     {/*
	       * Pack up the frame flags
	       */
	      Flags = 0;
	      if (P->AnimCtrl.Frames[i].Erase)
		Flags |= GEANIMCTRL_FRAME_ERASE;
	      if (P->AnimCtrl.Frames[i].Redraw)
		Flags |= GEANIMCTRL_FRAME_REDRAW;
	      /*
	       * Write out the frame data
	       */
	      fprintf(geFdO, "%d %d %d %d %d %f %f %f %d\n",
		      P->AnimCtrl.Frames[i].Repeat,
		      P->AnimCtrl.Frames[i].DrawDelay,
		      P->AnimCtrl.Frames[i].EraseDelay,
		      P->AnimCtrl.Frames[i].Dx,
		      P->AnimCtrl.Frames[i].Dy,
		      P->AnimCtrl.Frames[i].MagFX,
		      P->AnimCtrl.Frames[i].MagFY,
		      P->AnimCtrl.Frames[i].Rot,
		      Flags);
	     }
	  }
	break;

    case GEREAD:
	/*
	 * Reading in - the input format is dependant on the version number.
	 * First case is latest version.
	 */
	switch (geVersionIn)
	  {case 8:
	     /*
	      * Diff between 8 and prev:
	      *		- Added magnification and rotation controls
	      *		  and InterruptResponse
	      *
	      * Read the control structure data and flags
	      */

	     geFscanf6(geFdI, "%d %d %d %d %d %d",
		       &P->AnimCtrl.FirstFrame,
		       &P->AnimCtrl.NumFrames,
		       &P->AnimCtrl.LastFrame,
		       &Flags,
		       &P->AnimCtrl.InterruptResponse,
		       &P->AnimCtrl.NumPackets);

	     c = geFgetc(geFdI);                /* Read past new line        */
	     if (geFeof(geFdI)) 
	       {geErrorReport(GERRBADMETA, FALSE);
		return(GERRBADMETA);
	       }
	     /*
	      * Unpack the control structure flags
	      */
	     P->AnimCtrl.Animate = (Flags & GEANIMCTRL_ANIMATE ? TRUE : FALSE);
	     P->AnimCtrl.Interrupt =
	       (Flags & GEANIMCTRL_INTERRUPT ? TRUE : FALSE);
	     P->AnimCtrl.Block =
	       (Flags & GEANIMCTRL_BLOCK ? TRUE : FALSE);
	     P->AnimCtrl.StartForward =
	       (Flags & GEANIMCTRL_STARTFORWARD  ? TRUE : FALSE);
	     P->AnimCtrl.ReverseAtEnd =
	       (Flags & GEANIMCTRL_REVERSEATEND ? TRUE : FALSE);
	     P->AnimCtrl.CdispOverride =
	       (Flags & GEANIMCTRL_CDISPOVERRIDE ? TRUE : FALSE);
	     P->AnimCtrl.CZupdisp =
	       (Flags & GEANIMCTRL_CZUPDISP ? TRUE : FALSE);
	     if (P->AnimCtrl.CZupdisp) geSeg0->AnimCtrl.CZupdisp = TRUE;
	     /*
	      * Dynamic animation state flags
	      */
	     P->AnimCtrl.Animated      = P->AnimCtrl.Animate;
	     P->AnimCtrl.Blocked       = P->AnimCtrl.Block;
	     P->AnimCtrl.CurDirForward = P->AnimCtrl.StartForward;
	     P->AnimCtrl.NumFramesRemaining = P->AnimCtrl.NumFrames;
	     /*
	      * Read in the animation frames, if any.
	      */
	     if (P->AnimCtrl.NumPackets <= 0)
	       P->AnimCtrl.Frames = NULL;
	     else
	       {/*
		 * Allocate the space for the frames.
		 */
		 P->AnimCtrl.Frames = (struct  GE_ANIM_FRAME *)
		   geMalloc(P->AnimCtrl.NumPackets *
			    (sizeof(struct GE_ANIM_FRAME)));

		 /*
		  * Read in the packets
		  */
		 for (i = 0; i < P->AnimCtrl.NumPackets; i++)
		   {/*
		     * Read the frame data and flags
		     */
		     geFscanf5(geFdI, "%d %d %d %d %d",
			       &P->AnimCtrl.Frames[i].Repeat,
			       &P->AnimCtrl.Frames[i].DrawDelay,
			       &P->AnimCtrl.Frames[i].EraseDelay,
			       &P->AnimCtrl.Frames[i].Dx,
			       &P->AnimCtrl.Frames[i].Dy);
		     
		     geFscanff3(geFdI, " %f %f %f",
			       &P->AnimCtrl.Frames[i].MagFX,
			       &P->AnimCtrl.Frames[i].MagFY,
			       &P->AnimCtrl.Frames[i].Rot);
		     
		     geFscanf1(geFdI, " %d",
			       &Flags);
		     
		     if (ResScaleFactor)
		       {/*
			 * The drawing being read in was produced on a system
			 * with a display resolution different from the one
			 * on which it is now being displayed or edited, so
			 * have to scale Dx,Dy to take into account this reso-
			 * lution differential.
			 */
			 ft = (float)P->AnimCtrl.Frames[i].Dx * ResScaleFactor;
			 GEINT(ft, P->AnimCtrl.Frames[i].Dx);
			 ft = (float)P->AnimCtrl.Frames[i].Dy * ResScaleFactor;
			 GEINT(ft, P->AnimCtrl.Frames[i].Dy);
		       }
		     c = geFgetc(geFdI);       /* Read past new line         */
		     if (geFeof(geFdI)) 
		       {geErrorReport(GERRBADMETA, FALSE);
			return(GERRBADMETA);
		       }
		     /*
		      * Unpack the flags
		      */
		     P->AnimCtrl.Frames[i].Erase =
		       (Flags & GEANIMCTRL_FRAME_ERASE ? TRUE : FALSE);
		     P->AnimCtrl.Frames[i].Redraw =
		       (Flags & GEANIMCTRL_FRAME_REDRAW ? TRUE : FALSE);
		   }				/* End of reading packets    */
	       }				/* End of reading anim frames*/
	     break;				/* End of version 8	     */

	   case 7:
	     /*
	      * Diff between 7 and prev:
	      *		- This is the first time for this data
	      *
	      * Read the control structure data and flags
	      */

	     geFscanf5(geFdI, "%d %d %d %d %d",
		       &P->AnimCtrl.FirstFrame,
		       &P->AnimCtrl.NumFrames,
		       &P->AnimCtrl.LastFrame,
		       &Flags,
		       &P->AnimCtrl.NumPackets);

	     c = geFgetc(geFdI);                /* Read past new line        */
	     if (geFeof(geFdI)) 
	       {geErrorReport(GERRBADMETA, FALSE);
		return(GERRBADMETA);
	       }
	     /*
	      * Unpack the control structure flags
	      */
	     P->AnimCtrl.Animate = (Flags & GEANIMCTRL_ANIMATE ? TRUE : FALSE);
	     P->AnimCtrl.Interrupt =
	       (Flags & GEANIMCTRL_INTERRUPT ? TRUE : FALSE);
	     P->AnimCtrl.Block =
	       (Flags & GEANIMCTRL_BLOCK ? TRUE : FALSE);
	     P->AnimCtrl.StartForward =
	       (Flags & GEANIMCTRL_STARTFORWARD  ? TRUE : FALSE);
	     P->AnimCtrl.ReverseAtEnd =
	       (Flags & GEANIMCTRL_REVERSEATEND ? TRUE : FALSE);
	     P->AnimCtrl.CdispOverride =
	       (Flags & GEANIMCTRL_CDISPOVERRIDE ? TRUE : FALSE);
	     P->AnimCtrl.CZupdisp =
	       (Flags & GEANIMCTRL_CZUPDISP ? TRUE : FALSE);
	     /*
	      * Dynamic animation state flags
	      */
	     P->AnimCtrl.Animated      = P->AnimCtrl.Animate;
	     P->AnimCtrl.Blocked       = P->AnimCtrl.Block;
	     P->AnimCtrl.CurDirForward = P->AnimCtrl.StartForward;
	     P->AnimCtrl.NumFramesRemaining = P->AnimCtrl.NumFrames;
	     /*
	      * Read in the animation frames, if any.
	      */
	     if (P->AnimCtrl.NumPackets <= 0)
	       P->AnimCtrl.Frames = NULL;
	     else
	       {/*
		 * Allocate the space for the frames.
		 */
		 P->AnimCtrl.Frames = (struct  GE_ANIM_FRAME *)
		   geMalloc(P->AnimCtrl.NumPackets *
			    (sizeof(struct GE_ANIM_FRAME)));

		 /*
		  * Read in the packets
		  */
		 for (i = 0; i < P->AnimCtrl.NumPackets; i++)
		   {/*
		     * Read the frame data and flags
		     */
		     geFscanf6(geFdI, "%d %d %d %d %d %d",
			       &P->AnimCtrl.Frames[i].Repeat,
			       &P->AnimCtrl.Frames[i].DrawDelay,
			       &P->AnimCtrl.Frames[i].EraseDelay,
			       &P->AnimCtrl.Frames[i].Dx,
			       &P->AnimCtrl.Frames[i].Dy,
			       &Flags);
		     
		     P->AnimCtrl.Frames[i].MagFX = 100.0;
		     P->AnimCtrl.Frames[i].MagFY = 100.0;
		     P->AnimCtrl.Frames[i].Rot   = 0.0;
		     c = geFgetc(geFdI);       /* Read past new line         */
		     if (geFeof(geFdI)) 
		       {geErrorReport(GERRBADMETA, FALSE);
			return(GERRBADMETA);
		       }
		     /*
		      * Unpack the flags
		      */
		     P->AnimCtrl.Frames[i].Erase =
		       (Flags & GEANIMCTRL_FRAME_ERASE ? TRUE : FALSE);
		     P->AnimCtrl.Frames[i].Redraw =
		       (Flags & GEANIMCTRL_FRAME_REDRAW ? TRUE : FALSE);
		   }				/* End of reading packets    */
	       }				/* End of reading anim frames*/
	     break;				/* End of version 7	     */

	   default:
	     break;
	  }					/* End of version switch     */
	break;

    default:
	break;
   }

return(0);
}
