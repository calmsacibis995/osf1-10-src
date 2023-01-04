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
static char SccsId[] = "@(#)gegrf.c	1.19\t10/16/89";
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
**	GEGRF	             	       Graph object handler
**
**  ABSTRACT:
**
**        I feel that an explanation is in order regarding the apparent
**        redundancy of the three routines "geGrf, geGrp, and gePol"
**        which are very similiar, but NOT identical.  There are
**        both subtle and obvious differences in what needs to be
**        performed for certain commands that I feel justifies the
**        seperate existence of the three.  For example on the more obvious
**        side consider CREATE -
**                            Grf - simply wants to make a blank segment
**                            Grp - wants to collect already existing segments
**                                  under a common parent
**                            Pol - wants to create new segments and collect
**                                  them under a common parent
**        And the somewhat subtle distinction of CDISP
**                            Grf - dispatch the command to all but the ACTIVE
**                                  segment within the graph
**                            Grp - dispatch the command to
**                        and Pol   ALL sub-segments
**
**  ENVIRONMENT:
**
**	VAX/VMS, ULTRIX Operating Systems
**
**  MODIFICATION HISTORY:
**
**	GNE 03/05/87 Created
**
**--
**/

#include "geGks.h"

extern char           	    *geMalloc();
extern	              	    geGrp(), gePol(), geLin(), geTxt();
extern  struct GE_SEG 	    *geGenSegL(), *geGenSelPt();
extern struct GE_ANIM_FRAME *geGetFrameP();

geGrf(cmd, ASegP)
long     	cmd;
struct GE_SEG	*ASegP;
{                       

char             	p[10];
short			GravAlignSav, GridXAlignSav, GridYAlignSav;
int              	i, c, PixWidth, MMWidth, NumFrames;
long             	AnimMinSav, LVSegs, InqRes;
struct GE_SEG    	*GSegP, *SSegP, *TSegP, *NSegP, *LSegP, *BSeg0, *BSeg1,
                 	*GSegPSav, *ASegPSav, *Selection, *CurSel;
struct GE_XDO    	*XDoP, *PDoP;
struct GE_ANIM_FRAME    *AFrameP;
struct GE_CO		Clip;

XRectangle	 	xrect[1];

static int		SearchDir = GEDOWN;
static struct GE_SEG	*SearchSegS = NULL, *SearchSegE = NULL;

/*
 * Turn off interest in mouse motion for all but selected few commands
 */
#ifdef GERAGS
if (cmd != GEGRAV) geMxOnOff(FALSE);
#endif

switch(cmd)
   {case GEMOVE:
    case GECOPY:
    case GECOPYFLP:
    case GESCALE:
    case GEROTX:
    case GEROTFIXED:
    case GEFLIP:
    case GEMAG:                            	/* Magnify by specified %    */
    case GEMAGRESD:                            	/* Magnify by specified %    */
    case GEXVISIBLE:
    case GEALNTOP:
    case GEALNBOT:
    case GEALNLFT:
    case GEALNRT:
    case GEALNCHV:
    case GEALNCH:
    case GEALNCV:
    case GECOMP:
    case GEGPDELEMPTY:
    case GEGPLIVE:
    case GEINQIMGREF:
    case GEINQPSREF:
    case GESCRL:
    case GESCRR:
    case GESCRU:
    case GESCRD:
    case GEROUND:
    case GESCRUB:
    case GEHDISP:
    case GEHXDISP:
    case GETESTPT:
    case GETESTPTCLR:
    case GEGETFILL:
    case GEGETTXTBG:
    case GEGETLINE:
    case GEGETTXTFG:
    case GEGETFONT:
    case GEPUTFILL:
    case GEPUTTXTBG:
    case GEPUTLINE:
    case GEPUTTXTFG:
    case GEPUTFONT:
    case GEPSADJUST:
    case GESETATTRERASE_WHITE:
    case GEAUTODIG:
    case GEXAUTODIG:
    case GEPURGEHIDDEN:
    case GEINTERRUPT:
     if (cmd == GEHDISP && !ASegP) break;
     /*
      * Grid and gravity alignment must be turned off for duration of "ALL"
      *operations which in any way want to modify the coords of objects,
      *otherwise objects will tend to wander about and separate.
      */
     GravAlignSav               = geState.Grav.Align;
     GridXAlignSav              = geState.APagP->Grid.XAlign;
     GridYAlignSav              = geState.APagP->Grid.YAlign;
     geState.Grav.Align         = FALSE;
     geState.APagP->Grid.XAlign = FALSE;
     geState.APagP->Grid.YAlign = FALSE;
     
     if ((cmd == GEMAG || cmd == GEMAGRESD) && !geMagGrp)
       {geMagGrp = TRUE;
	geMagCx  = (ASegP->Co.x1 + ASegP->Co.x2) >> 1;
	geMagCy  = (ASegP->Co.y1 + ASegP->Co.y2) >> 1;
       }

     if (ASegP && (SSegP = ASegP->Next))
       {do
	  {NSegP = SSegP->Next;
	   GEVEC(cmd, SSegP);
	  }while (SSegP = NSegP);
       }

     if (cmd == GEMAGRESD)
       geGenMag(&geState.APagP->Crop);

     geState.Grav.Align         = GravAlignSav;
     geState.APagP->Grid.XAlign = GridXAlignSav;
     geState.APagP->Grid.YAlign = GridYAlignSav;

     break;

    case GEANIMRESET:			/* Reset animation		     */
        if (SSegP = ASegP)
	  {if (SSegP == geSeg0)		/* Reset animation parameters for the*/
	     geAnimReset(SSegP);	/* whole drawing.		     */
	   /*
	    * Now dispatch command to all of the objects in the drawing
	    */
	   while ((SSegP = SSegP->Next))
	     {GEVEC(cmd, SSegP);}
	  }
	break;

    case GEPAN:
     if (ASegP && (ASegP->Next))
       {/*
	 * Ignore gravity alignment but if grid align is on move the drawing
 	 * by a fixed amount which will bring it to a grid point.
	 */
	GravAlignSav       	   = geState.Grav.Align;
	geState.Grav.Align 	   = FALSE;

	GEVEC(GEGRAVTST, ASegP);

	GridXAlignSav              = geState.APagP->Grid.XAlign;
	GridYAlignSav              = geState.APagP->Grid.YAlign;

	geState.APagP->Grid.XAlign = FALSE;
	geState.APagP->Grid.YAlign = FALSE;

     	SSegP = ASegP->Next;
	do				/* Move the whole drawing	     */
	  {NSegP = SSegP->Next;
	   GEVEC(GEMOVE, SSegP);
	  }while (SSegP = NSegP);

	geState.Grav.Align 	   = GravAlignSav;
	geState.APagP->Grid.XAlign = GridXAlignSav;
	geState.APagP->Grid.YAlign = GridYAlignSav;
       }
     break;

    case GEGRAVTST:
     if (ASegP && (ASegP->Next))
	{if (geState.Grav.Align ||
	     geState.APagP->Grid.XAlign || geState.APagP->Grid.YAlign)
	     {geState.Grav.Lock = FALSE;
     	      SSegP = ASegP->Next;
	      while (SSegP && !geState.Grav.Lock)
		{GEVEC(GEGRAVTST, SSegP); SSegP = SSegP->Next;}
	     }
	}
     break;

    case GEKILL:
     /*
      * Release Name and Description buffers and Pixmap, if have 'em
      */
     if (!geState.APagP) break;

     if (geState.APagP->Name)    geFree(&(geState.APagP->Name),    0);
     if (geState.APagP->Descrip) geFree(&(geState.APagP->Descrip), 0);
     if (geState.APagP->Pixmap)
      {XFreePixmap(geDispDev, geState.APagP->Pixmap);
       if (geState.Pixmap == geState.APagP->Pixmap) geState.Pixmap = NULL;
       geState.APagP->Pixmap = NULL;
      }
     /*
      * Release the undo chain, if it exists
      */
     if (ASegP == geState.APagP->Seg0 && (XDoP = geState.APagP->XDoP))
       {do
	  {PDoP = XDoP->Prev;
	   geFree (&XDoP, sizeof(struct GE_XDO));
	  }while ((XDoP = PDoP));
        geState.APagP->XDoP = NULL;       
       }

     if (ASegP && (SSegP = ASegP->Next))
       {do
	  {NSegP = SSegP->Next;
	   GEVEC(GEKILL, SSegP);
	  }while (SSegP = NSegP);
       }
       
     geSegKil(&ASegP);
     geState.APagP->Seg0 = NULL;
     break;

#ifdef GERAGS

    case GEPURGE:				/* KILL "Deleted" objects    */
     /*
      * Release the undo chain, if it exists
      */
     if (ASegP == geState.APagP->Seg0 && (XDoP = geState.APagP->XDoP))
       {do
	  {PDoP = XDoP->Prev;
	   geFree (&XDoP, sizeof(struct GE_XDO));
	  }while ((XDoP = PDoP));
        geState.APagP->XDoP = NULL;       
       }

     if (ASegP && (SSegP = ASegP->Next))
       {do
	  {NSegP = SSegP->Next;
	   if (!SSegP->Live)			/* If seg has been deleted   */
	   	{GEVEC(GEKILL, SSegP);}		/* KILL it now		     */
	  }while (SSegP = NSegP);
       }

     break;

    case GEZRELINK:
	if (!ASegP || !ASegP->Visible) break;

	for(;;)
	  {i = 0;                              /* Number of switches         */
	   SSegP = ASegP->Next;

	   while (SSegP && (NSegP = SSegP->Next))
	     {if (NSegP->Z < SSegP->Z)
		{TSegP = SSegP->Prev;
		 if (ASegP->Next == SSegP) ASegP->Next = NSegP;
		 if (SSegP->Prev) SSegP->Prev->Next = SSegP->Next;
		 if (NSegP->Next) NSegP->Next->Prev = NSegP->Prev;
		 SSegP->Prev = NSegP;
		 SSegP->Next = NSegP->Next;
		 NSegP->Prev = TSegP;
		 NSegP->Next = SSegP;
		 i++;
	        }
	      else
		SSegP = SSegP->Next;
	     }	      
	   if (!i) break;                    /* No switches on a pass = done */
	  }
	break;

    case GEZRENUM:
	if (!ASegP || !ASegP->Visible) break;

        if (SSegP = ASegP->Next)             /* Start the ball rolling       */
	  {SSegP->Z = 0;
	   GEVEC(cmd, SSegP);
	  }
        else
	  break;                             /* No other segs in Grf         */

        while ((SSegP = SSegP->Next))
	  {SSegP->Z = SSegP->Prev->Z + 1;
	   GEVEC(cmd, SSegP);
	  }
	break;
                                             
    case GEVISIBLE:                          /* Make the graph visible */
        if (SSegP = ASegP)
	  {while (SSegP = SSegP->Next)
	     {if (!SSegP->Visible)
		{GEVEC(cmd, SSegP);}
	     }
	   geMnPag(GEDISP);		     /* Redraw the graph */
	  }
	break;

    case GEJOINALLTXT:
        if (NSegP = ASegP->Next)
	  {LSegP = geGenSegL();
	   do
	     {SSegP = NSegP;
	      NSegP = SSegP->Next;
	      GEVEC(GEISOTXT_GRP, SSegP);
	    }while(NSegP && SSegP != LSegP);

	   geGrp(GEJOINALLTXT, NULL);
	   geMnPag(GEDISP);
	  }
	break;

#endif

    case GEDISP:
     if (!ASegP || !geState.Window || geState.HaltRequested) break;
     
#ifdef GERAGS

     if (ASegP == geSeg0)
	   geClearArea(0,0, geState.APagP->Surf.width,
		      geState.APagP->Surf.height);

     if (geState.APagP->Grid.On && !geState.APagP->Grid.Top)
       geGenGridDraw(0);

#endif

     if ((SSegP = ASegP) && ASegP->Visible)
	  while ((SSegP = SSegP->Next) && !geState.HaltRequested)
	    {GEVEC(cmd, SSegP);}

#ifdef GERAGS
     /*
      * Redraw grid points
      */
     if (geState.APagP->Grid.On && geState.APagP->Grid.Top)
       geGenGridDraw(0);

     /*
      * Redraw autoscroll indicators
      */
      if (geState.ScrAuto)
	geGenDrawAutoScroll();
     /*
      * Redraw crop rectangle, if in show crop mode
      */
     if (geState.State == GEDISPCROP) geReDrawCropBox();
#endif

     break;

    case GEXDISP:
     if (!ASegP) break;

     if (geState.AnimDispMode == GEANIM_LIN ||
	 geState.AnimDispMode == GEANIM_BOX)
       {GEVEC(GEBOUNDS, ASegP);
	XDrawRectangle(geDispDev, geState.Drawable, geGC5,
		       GETPIX(ASegP->Co.x1), GETPIX(ASegP->Co.y1),
		       GETPIX(ASegP->Co.x2 - ASegP->Co.x1),
		       GETPIX(ASegP->Co.y2 - ASegP->Co.y1));
       }
     else if (geState.AnimDispMode == GEANIM_TRUE)
       {if (ASegP == geSeg0 && ASegP->FillStyle != GETRANSPARENT)
	  {geClearArea(0, 0, geState.APagP->Surf.width,
		       geState.APagP->Surf.height);
	  }

        AnimMinSav = geAnimMin;
        geAnimMin  = 0;

        if ((SSegP = ASegP) && ASegP->Visible)
	  while (SSegP = SSegP->Next)
	    {GEVEC(cmd, SSegP);}

        geAnimMin  = AnimMinSav;
       }

     break;

    case GECDISP:                               /* Dis subsegs insect clp bx */
    case GECZUPDISP:                            /* Dis objs above given obj  */
    case GECZDOWNDISP:                          /* Dis objs below given obj  */
     if (geState.ASegP == geSeg0) break;
     
     ASegPSav  = geState.ASegP;

     if (geSeg0->AnimCtrl.Animated && ASegP->AnimCtrl.Animated &&
	 geState.AnimPreview && geState.State != GEANIMPREVIEW)
       NumFrames = ASegP->AnimCtrl.NumFramesRemaining;
     else
       {if (geState.AnimPreview && ASegP->AnimCtrl.Animate &&
	    !ASegP->AnimCtrl.Animated &&
	    ASegP->AnimCtrl.Frames && !ASegP->AnimCtrl.Frames->Redraw)
	  break;

        NumFrames = 0;
	if (geState.Pixmap) geState.NeedToPostPixmap = TRUE;
	/*
	 * If there are anymore exposures coming, fetch 'em all now,
	 * put them together and process them all at once.
	 */
	while(XCheckWindowEvent(geDispDev, geState.APagP->Surf.self,
				GE_MWu, &geState.Event))
	  {Clip.x1 = GETSUI(geState.Event.xexpose.x);
	   Clip.y1 = GETSUI(geState.Event.xexpose.y);
	   Clip.x2 = Clip.x1 + GETSUI(geState.Event.xexpose.width);
	   Clip.y2 = Clip.y1 + GETSUI(geState.Event.xexpose.height);
	   geGenClipMerge(&geClip, &Clip);
	  }
       }

     Clip      = geClip;
     do
       {if (geSeg0->AnimCtrl.Animated && geState.AnimPreview &&
	    geState.State != GEANIMPREVIEW)
	  {/*
	    * If animation preview is allowed and haven't yet begun it, call
	    * each object handler to possibly reset the position of the object
	    * prior to starting (restarting) the animation.  Then display all
	    * objects which have the Zupdisp flag set, meaning they are "in
	    * front" of animated objects.
	    */
	    if (ASegP->AnimCtrl.NumFrames)
	      {geGrf(GEANIMRESET, geSeg0);
	       geGrf(GEBOUNDS,    geSeg0);
	      }

	    if ((SSegP = geSeg0) && geSeg0->Visible && cmd != GECZUPDISP)
	      {while ((SSegP = SSegP->Next) && !geState.HaltRequested)
		 {GEVEC(GECZUPDISP, SSegP);}
	      }

	  }

        /*
	 * Note:  geState.ASegP is "excluded" from the display, if the
	 *whole graph is desired to be refreshed, geState.ASegP must be NULLED
	 * About the GRID -
	 * If the grid is VISIBLE and it is on the TOP, then going to have to
	 * ERASE the DROP area and REFRESH the whole graph in the region.
	 */

	xrect[0].x      = xrect[0].y 	= 0;
	xrect[0].width  = GETPIX(Clip.x2 - Clip.x1);
	xrect[0].height = GETPIX(Clip.y2 - Clip.y1);
#ifdef GERAGS
	XSetClipRectangles(geDispDev, geGC1,
			   GETPIX(Clip.x1), GETPIX(Clip.y1),
			   xrect, 1, Unsorted);
#endif
	XSetClipRectangles(geDispDev, geGC2,
			   GETPIX(Clip.x1), GETPIX(Clip.y1),
			   xrect, 1, Unsorted);
   	XSetClipRectangles(geDispDev, geGC3,
			   GETPIX(Clip.x1), GETPIX(Clip.y1),
			   xrect, 1, Unsorted);
	XSetClipRectangles(geDispDev, geGC4,
			   GETPIX(Clip.x1), GETPIX(Clip.y1),
			   xrect, 1, Unsorted);
	XSetClipRectangles(geDispDev, geGC5,
			   GETPIX(Clip.x1), GETPIX(Clip.y1),
			   xrect, 1, Unsorted);
	XSetClipRectangles(geDispDev, geGCImg,
			   GETPIX(Clip.x1), GETPIX(Clip.y1),
			   xrect, 1, Unsorted);
	XSetClipRectangles(geDispDev, geGCPs,
			   GETPIX(Clip.x1), GETPIX(Clip.y1),
			   xrect, 1, Unsorted);
	XSetClipRectangles(geDispDev, geGC0,
			   GETPIX(Clip.x1), GETPIX(Clip.y1),
			   xrect, 1, Unsorted);


#ifdef GERAGS
        if (geState.State == GEDISPCROP) geReDrawCropBox();

        if (geState.APagP->Grid.On && geState.APagP->Grid.Top &&
	    ASegP != geSeg0)
	  {GEVEC(GEERASE,  ASegP);             /* ERASE everyone in region  */
	   ASegP = geSeg0;
	   geState.ASegP = NULL;
	  }
#endif

        if (ASegP == geSeg0 && ASegP->FillStyle != GETRANSPARENT)
	     {geClearArea(0, 0, geState.APagP->Surf.width,
			  geState.APagP->Surf.height);
	     }

#ifdef GERAGS

        if (geState.APagP->Grid.On && !geState.APagP->Grid.Top &&
	    ASegP == geSeg0)
	  geGenGridDraw(1);
#endif

        if (ASegP && ASegP->Visible)
	   {SSegP = ASegP->Next;
	    if (cmd == GECZDOWNDISP) SSegP = geSeg0->Next;
	    while (SSegP && !geState.HaltRequested)
	      {if (SSegP != geState.ASegP)
		 {if (SSegP->AnimCtrl.Animated && geState.AnimPreview &&
		      geState.State != GEANIMPREVIEW)
		    geClipAnim.x1 = geClipAnim.y1 = geClipAnim.x2 =
		      geClipAnim.y2 = -1;
		    
		  GEVEC(cmd, SSegP);

		  if (SSegP->AnimCtrl.Animated && geState.AnimPreview &&
		      geState.State != GEANIMPREVIEW &&
		      SSegP->AnimCtrl.Block && SSegP->AnimCtrl.Blocked)
		    break;
		 }
	       else if (cmd == GECZDOWNDISP) break;
	       SSegP = SSegP->Next;
	      }
	   }

#ifdef GERAGS
        if (geState.APagP->Grid.On && geState.APagP->Grid.Top &&
	    ASegP == geSeg0)
	  geGenGridDraw(1);

        XSetClipOrigin(geDispDev, geGC1,   0, 0);
        /*
         * Autoscroll regions
         */
        if (geState.ScrAuto)
	  geGenDrawAutoScroll();
#endif

	XSetClipOrigin(geDispDev, geGC2,   0, 0);
	XSetClipOrigin(geDispDev, geGC3,   0, 0);
	XSetClipOrigin(geDispDev, geGC4,   0, 0);
	XSetClipOrigin(geDispDev, geGC5,   0, 0);
	XSetClipOrigin(geDispDev, geGCImg, 0, 0);
	XSetClipOrigin(geDispDev, geGCPs,  0, 0);
	XSetClipOrigin(geDispDev, geGC0,   0, 0);

#ifdef GERAGS
	XSetClipMask  (geDispDev, geGC1,   None);
#endif

	XSetClipMask  (geDispDev, geGC2,   None);
	XSetClipMask  (geDispDev, geGC3,   None);
	XSetClipMask  (geDispDev, geGC4,   None);
	XSetClipMask  (geDispDev, geGC5,   None);
	XSetClipMask  (geDispDev, geGCImg, None);
	XSetClipMask  (geDispDev, geGCPs,  None);
	XSetClipMask  (geDispDev, geGC0,   None);
	/*
	 * Skip animation delays if either animation is not allowed at this
	 * time or already in animation preview state.
	 */
	if (!ASegP->AnimCtrl.Animated || !geState.AnimPreview ||
	    geState.State == GEANIMPREVIEW) break;
	/*
	 * Any animation controls?
	 */
	if (ASegP->AnimCtrl.Animated && NumFrames > 0)
	  NumFrames = geSleepDelay(ASegP, NumFrames);
	else
	  break;

      } while (--NumFrames > 0);

     geState.ASegP = ASegPSav;
     break;

    case GEERASE:
     geClearArea(0, 0, -1, -1);
     break;

#ifdef GERAGS

    case GEGRAV:                               /* Snap pt to obj?            */
        if (SSegP = ASegP)
	  {while ((SSegP = SSegP->Next))
	     if (SSegP != geState.ASegP)
	       {GEVEC(cmd, SSegP);}
	  }
	break;

    case GEDEL:
        if (ASegP && (SSegP = ASegP->Next))
	  {/*
	    * Find the first live segment
	    */
	    while (!(SSegP->Live) && (SSegP = SSegP->Next));

	    if (SSegP)                         /* Got at least 1 live seg?   */
	      {XDoP         = (struct GE_XDO *)geMalloc(sizeof(struct GE_XDO));
	       XDoP->Prev   = geState.APagP->XDoP;
	       XDoP->Next   = NULL;
	       XDoP->Seg    = SSegP;
	       geState.APagP->XDoP = XDoP;
	       /*
		* Delete all segments from here down to end
		*/
	       for (;;)
		 {GEVEC(cmd, SSegP);
		  if (!SSegP->Next) break;
		  SSegP       = SSegP->Next;
	        }
	       XDoP->LSeg   = SSegP;           /* Last seg cleared in batch*/
	      }
	   }
     break;

    case GEXDEL:                                /* Recover a graph         */
        if (SSegP = geState.APagP->XDoP->Seg)
	  {do
	     {GEVEC(GEXDEL, SSegP);
	      if (SSegP == geState.APagP->XDoP->LSeg) break;
	     }while (SSegP = SSegP->Next);
           /*
            * ReKill those segs which were trashed BEFORE the graph was killed
            */
           if ((XDoP = geState.APagP->XDoP->Prev) && XDoP->Seg)
             {do
		{if (XDoP->LSeg)
		   {SSegP = XDoP->Seg;         /* ReKill a graph          */
		    do                         /*- from the first segment */
		      {GEVEC(GEDEL, SSegP);    /*in undo struct to last   */
		      }while (SSegP != XDoP->LSeg && (SSegP = SSegP->Next));
		   }
		 else                         /* ReKill a single segment */
		   {GEVEC(GEDEL, XDoP->Seg);}
	        }while ((XDoP = XDoP->Prev) && XDoP->Seg);
             }
	  }
	break;

    case GESELPT:
	/*
	 * The selection proceeds from the TOP down.  It is ENTIRELY possible
	 * that NO ONE will respond to the selection request.  Calling routine
	 * must make provision for this eventuality.
	 *
	 * First take care of special cases:
	 *	1. Graph root is NULL - should NEVER happen
	 *	   	return with no selection
	 *	2. Graph is EMPTY
	 *		return with no selection
	 *	3. Graph has only 1 segment and it't already selected
	 *		return with no selection
	 *
	 */
	if (!geSeg0 || !geSeg0->Next ||
	    (!geSeg0->Next->Next && !ASegP))
	   {geState.ASegP = NULL;
	    break;
	   }
	/*
	 * Here's how it works:
	 * The search is conducted either UP or DOWN in the display list.
	 * The direction may be controlled by the caller or allowed to
	 * be automatically determined inside here.  If the caller specifies
	 * a segment to start the search and that segment is:
	 *	 a) the root - the search is conducted from the bottom UP
	 *	 b) the last seg in the graph - then the search is conducted
	 *	    from the top DOWN
	 *	 c) if a segment is passed in and it's neither the root nor the
	 *	    last segment, the search is conducted DOWN from (including)
	 *	    that segment
	 *	 d) if a NULL is passed in, the search is conducted from a
	 *	    starting point which is tracked within here.
	 *
	 * The selection criteria is defined by the following 3 rules
	 * (corresponding to up to 3 passes through the objects).
	 *
	 *	1 PICK the first FILLED object encountered if the selection
	 *	  point is INSIDE of this object.
	 *	2 Compute the distance from the selection point to the CLOSEST
	 *	  object.  If this distance is <= the minimally acceptable,
	 *	  PICK the object.
	 *	3 Considering ONLY those objects which the selection is INSIDE
	 *	  of (regardless of fill), choose the one which is CLOSEST.
	 * If no object meets any of these 3 conditions, then the selection
	 * process is deemed to have failed.
	 *
	 * Note:  If the request is to "search again", the currently selected
 	 * segment (geGrpSel), is EXCLUDED from the search; this guarantees
	 * that either a NEW segment is picked or NO segment is picked.
	 *
	 * Decide on the search direction and starting and ending segments
	 */
	if (ASegP == geGenSegL())		/* Request=search top DOWN        */
	   {SearchDir  = GEDOWN;
	    SearchSegS = geGenSegL();
	    SearchSegE = geSeg0;
	   }
	else
	if (ASegP == geSeg0)			/* Request=search bottom UP       */
	   {SearchDir  = GEUP;
	    SearchSegS = geSeg0->Next;
	    SearchSegE = geGenSegL();
	   }
	else					/* Have to figure it out          */
	   {if (SearchSegS == NULL)		/* Have to init search parms?     */
	   	{SearchDir  = GEDOWN;		/* Start by looking DOWN          */
		 if (ASegP) SearchSegS = ASegP;
	   	 else 	    SearchSegS = geGenSegL();
	   	 SearchSegE = geSeg0;		/* End search when hit bottom     */
	   	}
	    else
		{if (SearchDir == GEUP)		/* Looking UP?			  */
		    {SearchSegE = geGenSegL();	/* Top seg may have changed	  */
		    }
		 else
		    {SearchSegE = geSeg0;	/* Top seg may have changed	  */
		    }
		 if (SearchSegS == SearchSegE)	/* Turn search around?	     	  */
	   	    {if (SearchDir == GEDOWN)	/* Was looking down, turn it      */
			{SearchDir  = GEUP;	/* around to look UP now	  */
		    	 if (ASegP) SearchSegS = ASegP;
		    	 else       SearchSegS = geSeg0->Next;
		   	 SearchSegE = geGenSegL();
		   	}
	    	     else			/* Was looking up, turn it        */
		   	{SearchDir  = GEDOWN;	/* around to look DOWN now	  */
		   	 if (ASegP) SearchSegS = ASegP;
		   	 else       SearchSegS = geGenSegL();
		   	 SearchSegE = geSeg0;
		   	}
		    }
	   	}
	   }

	if (!ASegP || (geState.Event.xbutton.state != 1  &&
	               geState.Event.xbutton.state != 3))
		CurSel = geGrpSel;
	else	CurSel = NULL;

	if ((Selection = geGenSelPt(NULL, SearchSegS, SearchSegE, CurSel, SearchDir)))
	   {if (SearchDir == GEDOWN) SearchSegS = Selection->Prev;
	    else 		     SearchSegS = Selection->Next;
	   }

	else
	    geState.ASegP = NULL;

	break;

    case GESELBX:
	/*
	 * The selection proceeds from the TOP down.  It is ENTIRELY possible
	 *that NO ONE will respond to the selection request.  Calling routine
	 *must make provision for this eventuality.
	 *
	 * If there at least TWO more segments COMPLETELY contained by the
         * bounding box, then a temp group, called geGrpSel, is formed
	 *
	 * Try to find at least two segments wholely contained within the box.
	 * If there aren't at least two segments for selection, then the
	 * procedure is identical to the simple case of point select.
	 */
        if (SSegP = ASegP)
	  {BSeg0 = BSeg1 = NULL;

	   while (SSegP && SSegP != geSeg0)
	     {geState.ASegP = NULL;
	      if (SSegP != geState.GSegP)
		{GEVEC(cmd, SSegP);}
	      if (geState.ASegP)
		{if (!BSeg0)
		   BSeg0 = geState.ASegP;
		 else       
		   {BSeg1 = geState.ASegP;
		    break;
		   }
	        }
	      SSegP = SSegP->Prev;
	     }
	   /*
	    * Get this now, because JOIN will destroy Prev pointer
	    */
	   SSegP = SSegP->Prev;

	   /*
	    * There are at least two segments in the selection box.  Going to
	    * create a temporary group out of all of the segments completely
	    * contained within the selection box.
	    */
	   if (BSeg1)
	     {GSegPSav = geState.GSegP;

	      geGrp(GEGRPINIT, NULL);
	      GSegP = geGrpSel = geState.GSegP;

	      geGrp(GEJOIN, BSeg0);
	      geGrp(GEJOIN, BSeg1);
	      /*
	       * Now get the rest of segments falling within the selection
	       * box
	       */
	      while (SSegP && SSegP != geSeg0)
		{geState.ASegP = NULL;
		 if (SSegP != geState.GSegP)
		   {GEVEC(cmd, SSegP);}
		 SSegP = SSegP->Prev;
		 if (geState.ASegP)
		    geGrp(GEJOIN, geState.ASegP);
	        }

	      /*
	       * Finalize the group
	       */
	      GSegP->Next   = NULL;             /* Grp segs, priv to grp     */
	      GEVEC(GEBOUNDS, GSegP);           /* Establish group bounds    */
	      geState.ASegP = GSegP;
	      geState.GSegP = GSegPSav;
	     }
	   else geState.ASegP = BSeg0;
	  }
	break;

    case GEEXTENT0:
        if (ASegP && (SSegP = ASegP->Next))
	  {struct GE_CO Co;

	   Co.x1 = Co.y1 =  GEMAXLONG;
	   Co.x2 = Co.y2 = -GEMAXLONG;
	   while (SSegP)
	    {if (SSegP->Live)
	       {GEVEC(cmd, SSegP);
		if (geClip.x1 !=  GEMAXLONG || geClip.y1 !=  GEMAXLONG ||
		    geClip.x2 != -GEMAXLONG || geClip.y2 != -GEMAXLONG)
		   {Co.x1 = min(Co.x1, min(geClip.x1, geClip.x2));
		    Co.y1 = min(Co.y1, min(geClip.y1, geClip.y2));
		    Co.x2 = max(Co.x2, max(geClip.x1, geClip.x2));
		    Co.y2 = max(Co.y2, max(geClip.y1, geClip.y2));
		   }
	       }
	     SSegP = SSegP->Next;
	    }
	   geClip = Co;
	  }
	break;

#endif

    case GECREATE:
	geSeg0			  = NULL;
	geSegCr();
	geState.ASegP->Handle[0]  = 'G';
	geState.ASegP->Handle[1]  = 'R';
	geState.ASegP->Handle[2]  = 'F';
	geState.ASegP->Handle[3]  = '\0';
	geState.ASegP->Handler    = geGrf;
	geState.ASegP->Col        = geAttr.PagCol;
/*
 * Kludge - till figure out a better way.
 */
        geState.ASegP->FillStyle  = GETRANSPARENT;
	break;

    case GEBOUNDS:
        if (SSegP = ASegP)
	  {GSegP = ASegP;
	   GSegP->Co.x1 =  GEMAXLONG;	GSegP->Co.y1 =  GEMAXLONG;
	   GSegP->Co.x2 = -GEMAXLONG;	GSegP->Co.y2 = -GEMAXLONG;
	   while (SSegP = SSegP->Next)
	     {if (SSegP->Live &&  SSegP->Visible)
		{GEVEC(cmd, SSegP);
		 if (SSegP->Co.x1 !=  GEMAXLONG || SSegP->Co.y1 != GEMAXLONG ||
		     SSegP->Co.x2 != -GEMAXLONG || SSegP->Co.y2 != -GEMAXLONG)
		   {GSegP->Co.x1 = min(GSegP->Co.x1,
				       min(SSegP->Co.x1, SSegP->Co.x2));
		    GSegP->Co.y1 = min(GSegP->Co.y1,
				       min(SSegP->Co.y1, SSegP->Co.y2));
		    GSegP->Co.x2 = max(GSegP->Co.x2,
				       max(SSegP->Co.x1, SSegP->Co.x2));
		    GSegP->Co.y2 = max(GSegP->Co.y2,
				       max(SSegP->Co.y1, SSegP->Co.y2));
		   }
	        }
	     }
	   if (GSegP->Co.x2 < 0 || GSegP->Co.y2 < 0 ||
	       GETPIX(GSegP->Co.x1) > geState.APagP->Surf.width ||
	       GETPIX(GSegP->Co.y1) > geState.APagP->Surf.height)
	     GSegP->InFrame = FALSE;
	   else
	     GSegP->InFrame = TRUE;
	   } 
	break;

    case GEEXTENT:
        if (SSegP = ASegP)
	  {GSegP = ASegP;
	   GSegP->Co.x1 = geClip.x1 =  GEMAXLONG;
	   GSegP->Co.y1 = geClip.y1 =  GEMAXLONG;
	   GSegP->Co.x2 = geClip.x2 = -GEMAXLONG;
	   GSegP->Co.y2 = geClip.y2 = -GEMAXLONG;
	   while (SSegP = SSegP->Next)
	     {if (SSegP->Live &&  SSegP->Visible)
		{GEVEC(cmd, SSegP);
		 GSegP->Co.x1 = min(GSegP->Co.x1, geClip.x1);
		 GSegP->Co.y1 = min(GSegP->Co.y1, geClip.y1);
		 GSegP->Co.x2 = max(GSegP->Co.x2, geClip.x2);
		 GSegP->Co.y2 = max(GSegP->Co.y2, geClip.y2);
	        }
	     }
	   geClip.x1 = GSegP->Co.x1;
	   geClip.y1 = GSegP->Co.y1;
	   geClip.x2 = GSegP->Co.x2;
	   geClip.y2 = GSegP->Co.y2;
	   } 
	break;

#ifdef GERAGS

    case GEALNREF:
        if (ASegP)
	   {GEVEC(GEEXTENT0, ASegP);
	    geAln.Co    = geClip;
	    geAln.Pt.x  = (geAln.Co.x1 + geAln.Co.x2) >> 1;
	    geAln.Pt.y  = (geAln.Co.y1 + geAln.Co.y2) >> 1;
	   }
	break;

    case GEINQLIVE:                             /* How many LIVE segs?      */
	geState.LiveSegs = 0;
        if ((SSegP = ASegP) && SSegP->Live)
	  {while (SSegP = SSegP->Next)
	     {if (SSegP == geGrpSel)
		{LVSegs = geState.LiveSegs;	/* Number of segments	    */
		 geState.LiveSegs = 0;
		 GEVEC(cmd, SSegP);		/* inside the temp select   */
		 geState.LiveSegs += LVSegs;	/* group is added to total  */
		}
	      else
		 GEVEC(cmd, SSegP);		/* Just send cmd to object  */
	     }
	  }
	break;

    case GEINQVISIBLE:                         /* How manyVISIBLEsegs in grf*/
	geState.VisibleSegs = 0;
        if ((SSegP = ASegP) && SSegP->Live && SSegP->Visible)
	  {while (SSegP = SSegP->Next)
		{GEVEC(cmd, SSegP);}	       /* Just send cmd to object  */
	  }
	break;

    case GEINQXVISIBLE:                        /* Are there HIDDEN segs in grf*/
	geState.Hidden = FALSE;
        if ((SSegP = ASegP) && SSegP->Live)
	   {if (!SSegP->Visible)
		{geState.Hidden = TRUE; 
		 break; 
		}
	    while (SSegP = SSegP->Next)		/* Just send cmd to object  */
	      {GEVEC(cmd, SSegP);
	       if (geState.Hidden)
		  break;
	      }
 	   }
	break;

    case GEINQMEMIMG:                         	/* Total mem needed by images*/
	geInqRes = InqRes = 0;
        if ((SSegP = ASegP) && SSegP->Live && SSegP->Visible)
	  {while (SSegP = SSegP->Next)
		{GEVEC(cmd, SSegP);		/* Just send cmd to object  */
		 InqRes += geInqRes;
		}
	   geInqRes = InqRes;
	  }
	break;

    case GEINQLVTXT:                            /* How manyLIVE&VISBL TEXTs  */
        if (SSegP = ASegP)
	  {geState.LiveSegs = 0;
	   while (SSegP = SSegP->Next)
		{GEVEC(cmd, SSegP);}
	  }
	break;

    case GEINQLINCOL:                           /* FIRST segment in list     */
    case GEINQFILCOL:                           /*controls these             */
    case GEINQCOL:
        if (ASegP && (SSegP = ASegP->Next))
	   {GEVEC(cmd, SSegP);}
	break;

    case GEPGRELINK:
     if (geState.ASegP && !geState.ASegP->Prev && (SSegP = ASegP))
       while ((SSegP = SSegP->Next) && geState.ASegP) GEVEC(cmd, SSegP);
     break;

#endif

    case GEGETSEG_COL:
        if (geSeg0->Col.RGB.pixel == geGenCol.pixel) geState.ASegP = geSeg0;
        else
	  {if ((SSegP = ASegP))
	     {while ((SSegP = SSegP->Next) && !geState.ASegP)
		{GEVEC(cmd, SSegP);}
	     }
	  }
	break;

    case GEWRITE:
    case GEREAD:
     /*
      * See if it's ascii or ddif
      */
     if (geReadWriteType == GE_RW_DDIF)
        geGrfIODDIF(cmd, ASegP);
     else
       geGrfIO(cmd, ASegP);
     GEVEC(GEZRENUM, geSeg0);
     break;


    default:
	break;
   }
/*
 * Turn on interest in mouse motion
 */
#ifdef GERAGS
if (cmd != GEGRAV) geMxOnOff(TRUE);
#endif
}

#ifdef GERAGS

struct GE_SEG *
geGenSelPt(cmd, SearchSegS, SearchSegE, SegExclude, SearchDir)
long		cmd;
struct GE_SEG   *SearchSegS, *SearchSegE, *SegExclude;
int		SearchDir;
{
short		SelRadP;
struct GE_SEG 	*SSegP, *SegSel, *PGrpP, *ProxSel, *ProxPGrpP;

SegSel  = ProxSel = ProxPGrpP = NULL;

/*
 * See if can find any objects within the selection radius.  If so, select it;
 * otherwise keep looking
 */
if (cmd == GESELPT || !cmd)
   {SegSel  = PGrpP = NULL;
    SelRadP = GEMAXSHORT;
    SSegP 	 = SearchSegS;

    do
	{if (SSegP != geState.GSegP && SSegP != SegExclude)
	    {geState.SelRadC = GEMAXSHORT;
    	     GEVEC(GESELPT, SSegP);
	     if (geState.SelRadC < SelRadP)
		{SelRadP  = geState.SelRadC;
	     	 SegSel   = geSegSel;
		 if (SSegP->Handler == gePol || SSegP->Handler == geGrp)
		     PGrpP   = geState.PGrpP;
		 else
		     PGrpP   = NULL;
		}
	    }

	 if (SearchDir == GEDOWN) SSegP = SSegP->Prev;
	 else		     	  SSegP = SSegP->Next;
	}while (SSegP && SSegP != SearchSegE);
   }

if (SegSel && SelRadP <= GESELRAD)
   {ProxSel   = SegSel;
    ProxPGrpP = PGrpP;
   }
/*
 * See if inside of any FILLED objects.  If so, pick the first one encountered;
 * otherwise keep looking.
 */
if (cmd == GESELPTINFILL || !cmd)
   {SegSel  = PGrpP = NULL;
    SelRadP = GEMAXSHORT;
    SSegP   = SearchSegS;

    do
	{if (SSegP != geState.GSegP && SSegP != SegExclude)
	    {geState.SelRadC = GEMAXSHORT;
	     GEVEC(GESELPTINFILL, SSegP);
	     if (geState.SelRadC == 0)
		{SegSel = geSegSel;
		 if (SSegP->Handler == gePol || SSegP->Handler == geGrp)
		     PGrpP   = geState.PGrpP;
		 else
		     PGrpP   = NULL;
		 break;
		}
	    }
	 if (SearchDir == GEDOWN) SSegP = SSegP->Prev;
	 else		      	  SSegP = SSegP->Next;
	}while (SSegP && SSegP != SearchSegE);
   }	
/*
 * If a "close" object was found in the earlier test then switch the selection
 * to point to that object if either NO "filled" object was detected in the
 * previous test or the "close" object is on TOP of the filled object.
 */
if (ProxSel && (!SegSel || (ProxSel->Z > SegSel->Z)))
   {SegSel = geSegSel = ProxSel;
    PGrpP  = geState.PGrpP = ProxPGrpP;
   }

/*
 * See if INSIDE of any object at all.  If so, pick the closest one; otherwise
 * select has FAILED.
 */
else
if (!SegSel && (cmd == GESELPTIN || !cmd))
   {SegSel  = PGrpP = NULL;
    SelRadP = GEMAXSHORT;
    SSegP   = SearchSegS;
    do
	{if (SSegP != geState.GSegP && SSegP != SegExclude)
	    {geState.SelRadC = GEMAXSHORT;
	     GEVEC(GESELPTIN, SSegP);
	     if (geState.SelRadC < SelRadP)
		    {SelRadP = geState.SelRadC;
		     SegSel  = geSegSel;
		     if (SSegP->Handler == gePol || SSegP->Handler == geGrp)
		     	PGrpP   = geState.PGrpP;
		     else
			PGrpP   = NULL;
		    }
	    }
	 if (SearchDir == GEDOWN) SSegP = SSegP->Prev;
	 else		     	  SSegP = SSegP->Next;
   	}while (SSegP && SSegP != SearchSegE);

   }

geState.SelRadC = SelRadP;
/*
 * If the selected object is part of a group or polygon, then point geState.ASegP
 * at the group or polygon UNLESS the GrpPoly Edit flag is turned on, then point it
 * at the primitive member of the group or polygon.
 */
geState.PGrpP = PGrpP;
geSegSel      = SegSel;
if (geState.GrpEdit || !PGrpP)
    geState.ASegP = SegSel;
else
    geState.ASegP = PGrpP;

return (geState.ASegP);

}

geGenDrawAutoScroll()
{
short            	scrw, scrh;

GEFILSTYLE(geGC3, FillStippled);
GE_FG_M(geGC3, geBlack.pixel, GXcopy);
GESTIPPLE(geGC3, 10);

scrw = geState.APagP->Surf.width  >> 5;
scrh = geState.APagP->Surf.height >> 5;
scrw = scrw < 4 ? 4 : (scrw > 16 ? 16 : scrw);
scrh = scrh < 4 ? 4 : (scrh > 16 ? 16 : scrh);
scrw = min(scrw, scrh);
scrh = scrw;

XFillRectangle(geDispDev, geState.Drawable, geGC3, 0, 0,
	       geState.APagP->Surf.width, scrh);
XFillRectangle(geDispDev, geState.Drawable, geGC3,
	       geState.APagP->Surf.width - scrw, 0,
	       scrw, geState.APagP->Surf.height);
XFillRectangle(geDispDev, geState.Drawable, geGC3, 0,
	       geState.APagP->Surf.height - scrh,
	       geState.APagP->Surf.width, scrh);
XFillRectangle(geDispDev, geState.Drawable, geGC3, 0, 0,
	       scrw, geState.APagP->Surf.height);

}

#endif

/*
 * Copies generic segment information from Seg2 to Seg1
 */
geGenCopySeg(ASegP1, ASegP2)
struct GE_SEG *ASegP1, *ASegP2;
{
int i;

strcpy (ASegP1->Handle, ASegP2->Handle);
ASegP1->Handler         = ASegP2->Handler;
ASegP1->Col             = ASegP2->Col;
ASegP1->FillHT	        = ASegP2->FillHT;
ASegP1->FillStyle       = ASegP2->FillStyle;
ASegP1->FillWritingMode = ASegP2->FillWritingMode;
ASegP1->Xref            = ASegP2->Xref;
ASegP1->WhiteOut        = ASegP2->WhiteOut;
ASegP1->ConLine         = ASegP2->ConLine;
geGenCopyAnim(ASegP1, ASegP2);
geGenCopyDesc(ASegP1, ASegP2);

}


/*
 * Copies segment descriptor from Seg2 to Seg1
 */
geGenCopyDesc(ASegP1, ASegP2)
struct GE_SEG *ASegP1, *ASegP2;
{
int i;


if (ASegP1->Descrip)
  geFree(&(ASegP1->Descrip), 0);

if (ASegP2->Descrip && (i = strlen(ASegP2->Descrip)))
    {ASegP1->Descrip = (unsigned char *)geMalloc(i + 1);
     strcpy (ASegP1->Descrip, ASegP2->Descrip);
    }
else
  ASegP1->Descrip = NULL;

}

/*
 * Copies animation control information from Seg2 to Seg1
 */
geGenCopyAnim(ASegP1, ASegP2)
struct GE_SEG *ASegP1, *ASegP2;
{
int i;

if (ASegP1->AnimCtrl.Frames)
  geFree(&(ASegP1->AnimCtrl.Frames), 0);

ASegP1->AnimCtrl = ASegP2->AnimCtrl;

if (ASegP2->AnimCtrl.NumPackets > 0)
  {/*
    * Allocate the space for the frames.
    */
    ASegP1->AnimCtrl.Frames = (struct  GE_ANIM_FRAME *)
      geMalloc(ASegP2->AnimCtrl.NumPackets *
	       (sizeof(struct GE_ANIM_FRAME)));

    for (i = 0; i < ASegP2->AnimCtrl.NumPackets; i++)
      {/*
	* Copy the packets
	*/
	ASegP1->AnimCtrl.Frames[i] = ASegP2->AnimCtrl.Frames[i];
      }
  }
}
