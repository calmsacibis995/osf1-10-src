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
static char SccsId[] = "@(#)gegrp.c	1.17\t10/16/89";
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
**	GEGRP	             	       Group object handler
**
**  ABSTRACT:
**
** Note:  I feel that an explanation is in order regarding the apparent
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
**        And the somewhat subtle distinction of CDISP:
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
**	GNE 02/04/87 Created
**
**--
**/

#include "geGks.h"


extern	              	geLin(), geArc(), geTxt(), gePol();
extern unsigned long  	geAllocColor();
extern char           	*geMalloc();
extern  struct GE_SEG 	*geGenSegL(), *geGenSelPt();
extern Region		geXPolygonRegion();

geGrp(cmd, ASegP)
long     	cmd;
struct GE_SEG	*ASegP;
{                       
char             p[10];
short            ChkPtSav, LiveSav, DamageRepairSav, Live;
int              i, c, GravAlignSav, GridXAlignSav, GridYAlignSav, ix, iy,
		 NumFrames;
long		 editcmd, AnimMinSav, x, y, LVSegs, InqRes;

struct GE_SEG    *GSegP, *SSegP, *NSegP, *PSegP, *ASegPSav, *TSegP, *LSegP;

Region		 TRegion;
char  	       	 *error_string;

if (ASegP &&
    (!ASegP->Live && cmd != GEINQIMGREF && cmd != GEKILL &&
                     cmd != GESCRUB     && cmd != GEGETSEG_COL &&
		     cmd != GEXDEL)) return;
/*
 * Check to see if the command should be applied to the group members instead
 * of the group to itself - case in point - user generated SEPARATE command
 * must be applied to the group members not to the temp group.
 */
if (ASegP && ASegP == geGrpSel && geBreakIn &&
    (cmd == GEXDISP  || cmd == GEALNCH  || cmd == GEALNCV ||
     cmd == GESEP    || cmd == GEXVISIBLE || cmd == GEALN))
  {GSegP = SSegP = (struct GE_SEG *)ASegP->Private;
   while (SSegP)
     {NSegP = SSegP->Next; GEVEC(cmd, SSegP); SSegP = NSegP;}
   return;
  }

switch(cmd)
   {case GEWARP1:
    case GEWARP2:
    case GEWARPX:
    case GEGETLINE:
    case GEGETTXTFG:
    case GEGETFONT:
    case GEPUTLINE:
    case GEPUTTXTFG:
    case GEPUTFONT:
    case GEROTX:
    case GEROTFIXED:
    case GEFLIP:
    case GEMAG:                            	/* Magnify by specified %    */
    case GEMAGRESD:                            	/* Magnify by specified %    */
    case GEIMGMAG:
    case GEGRAV:
    case GECOMP:
    case GEINQIMGREF:
    case GEINQPSREF:
    case GEROUND:
    case GEINQLVTXT:                            /* HowManyLIVETXTsegsIn grp */
    case GETESTPT:
    case GETESTPTCLR:
    case GEPSADJUST:
    case GEADDVERT:
    case GEAUTODIG:
    case GEXAUTODIG:
        if (ASegP && ASegP->Visible &&
	    (SSegP = (struct GE_SEG *)ASegP->Private))
	  {if (cmd == GEGRAV &&
	    (geState.Grav.Pt.x < (ASegP->Co.x1 - geState.Grav.Rad) ||
	     geState.Grav.Pt.y < (ASegP->Co.y1 - geState.Grav.Rad) ||
	     geState.Grav.Pt.x > (ASegP->Co.x2 + geState.Grav.Rad) ||
	     geState.Grav.Pt.y > (ASegP->Co.y2 + geState.Grav.Rad)))
	     break;

	   if (cmd == GEMAG && !geMagGrp)
		{geMagGrp = TRUE;
		 geMagCx  = (ASegP->Co.x1 + ASegP->Co.x2) >> 1;
		 geMagCy  = (ASegP->Co.y1 + ASegP->Co.y2) >> 1;
		}
	   while (SSegP) {GEVEC(cmd, SSegP); SSegP = SSegP->Next;}
	  }
        if (cmd == GECOMP)
	  {ASegP->Col.RGB.red   = GEMAXUSHORT - ASegP->Col.RGB.red;
	   ASegP->Col.RGB.green = GEMAXUSHORT - ASegP->Col.RGB.green;
	   ASegP->Col.RGB.blue  = GEMAXUSHORT - ASegP->Col.RGB.blue;
	   ASegP->Col.RGB.pixel = geAllocColor(ASegP->Col.RGB.red,
					       ASegP->Col.RGB.green,
					       ASegP->Col.RGB.blue, NULL);
	   geGenRgbToCmyk(&(ASegP->Col.RGB), &(ASegP->Col.CMYK));
	   geGenNameRgb(&(ASegP->Col));
          }
	break;

    case GEXDISP:
    case GEGDISP:
        if (ASegP && ASegP->Visible && ASegP->Private)
	 {if (cmd == GEXDISP && geState.AnimDispMode == GEANIM_BOX)
	   {GEVEC(GEBOUNDS, ASegP);
	    XDrawRectangle(geDispDev, geState.Drawable, geGC5,
			   GETPIX(ASegP->Co.x1), GETPIX(ASegP->Co.y1),
			   GETPIX(ASegP->Co.x2 - ASegP->Co.x1),
			   GETPIX(ASegP->Co.y2 - ASegP->Co.y1));
	    break;
	   }

	  AnimMinSav = geAnimMin;
	  geAnimMin  = 0;
	  SSegP = (struct GE_SEG *)ASegP->Private;
	   while (SSegP) {GEVEC(cmd, SSegP); SSegP = SSegP->Next;}
	  geAnimMin  = AnimMinSav;
	  }
	break;

    case GEDISP:
        if (ASegP && ASegP->Visible && ASegP->InFrame && ASegP->Private &&
	    geState.Window && (!ASegP->ConLine || geRun.RagsCalling) &&
	    !geState.HaltRequested)
	 {/*
	   * Display it, taking into consideration possible animation
	   * controls.  If the display request is coming about as a
	   * consequence of some OTHER object currently animating itself,
	   * then simply display the object normally ONCE and
	   * ignore all of the animation controls.  This is accomplished
	   * by setting the NumFrames factor to 0.
	   */
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
	    }
	  
	  do
	      {/*
		* See if Group needs to be filled - if so then get segments to
		* add themselves to the vertex list and display the interior
		* prior to displaying the edges.
		*/
		if (ASegP->FillStyle != GETRANSPARENT)
		  {geVn = 0;
		   SSegP = (struct GE_SEG *)ASegP->Private;
		   while (SSegP)
		     {if (SSegP->Handler == geLin || SSegP->Handler == geArc)
			{GEVEC(GEADDVERT, SSegP);}
		      SSegP = SSegP->Next;
		     }
		   if (geVn > 2)
		     {GE_SET_BG_DISP_ATTR;
		      GE_FG_M(geGC3, geDispAttr.BgPixel,
			      ASegP->FillWritingMode);
		      GEFILSTYLE_FOS(geGC3, geDispAttr.BgStyle,
				     geDispAttr.BgPixel);
		      GESTIPPLE(geGC3, geDispAttr.BgHT);
		      XFillPolygon(geDispDev, geState.Drawable, geGC3, geVert,
				   geVn,
				   Complex, CoordModeOrigin);
		     }
		  }
		/*
		 * Now send the command to the members
		 */
		SSegP = (struct GE_SEG *)ASegP->Private;
		while (SSegP && !geState.HaltRequested)
		  {if (SSegP != geState.ASegP)
		     {GEVEC(cmd, SSegP);
		      if (SSegP->AnimCtrl.Animate && geState.AnimPreview &&
			  geState.State != GEANIMPREVIEW &&
			  SSegP->AnimCtrl.Block && SSegP->AnimCtrl.Blocked)
			break;
		     }
		   SSegP = SSegP->Next;
		  }

		if (ASegP->AnimCtrl.Animated && NumFrames > 0)
		  NumFrames = geSleepDelay(ASegP, NumFrames);
	       
	    } while (--NumFrames > 0 && !geState.HaltRequested);
	   }
	break;

    case GECDISP:
	if (ASegP && ASegP->Visible && geGenInClip(ASegP))
	  {/*
	    * If the Clipped Refresh request is arising as a consequence
	    * of an object in animation AND this object's "CdispOverride"
	    * flag is OFF then the request is to be IGNORED if either:
	    *	1) if the Z order of this object is greater than the Z order
	    *	   of the object currently animating;
	    *	2) if this object is supposed to be ERASED during its own
	    *	   animation sequence and it is NOT to be finally REDRAWN
	    */
	   if ((geState.State == GEANIMPREVIEW &&
		geState.ASegP && ASegP->Z >= geState.ASegP->Z) ||
	       (geState.State != GEANIMPREVIEW &&
		ASegP->AnimCtrl.CdispOverride))
	     break;

	   GEVEC(GEDISP, ASegP);
	  }
	break;

    case GECZUPDISP:
	if (ASegP && ASegP->Visible && ASegP->AnimCtrl.CZupdisp &&
	    geGenInClip(ASegP))
	  {GEVEC(GEDISP, ASegP);}
	break;

    case GECZDOWNDISP:
	if (ASegP && ASegP->Visible && geGenInClip(ASegP))
	  {GEVEC(GEDISP, ASegP);}
	break;

    case GEINTERRUPT:
	if (ASegP && ASegP->Visible)
	  {if (ASegP->AnimCtrl.Interrupt)
	     {/*
	       * If the interrupt flag is set for the group itself, then
	       * allow it operate on the group.
	       */
	       geRespondToInterrupt(ASegP);
	     }
	   else
	     {/*
	       * Otherwise, just send the command to the group members.
	       */
	       SSegP = (struct GE_SEG *)ASegP->Private;
	       while (SSegP) {GEVEC(cmd, SSegP); SSegP = SSegP->Next;}
	     }
	  }
	break;

    case GEERASE:
	if (ASegP && ASegP->Visible && ASegP->InFrame)
	   {GEVEC(GEEXTENT, ASegP);
	    geClearArea(GETPIX(geClip.x1), GETPIX(geClip.y1),
			GETPIX(geClip.x2 - geClip.x1),
			GETPIX(geClip.y2 - geClip.y1));
	   }
	break;

    case GEHDISP:
    case GEHXDISP:
        if (ASegP && ASegP->Visible && ASegP->InFrame && ASegP->Private)
	  {/*
	    * See if Group needs to be filled - if so then get segments to
	    *add themselves to the vertex list and display the interior
	    *prior to displaying the edges.
	    */
	   if (ASegP->FillStyle != GETRANSPARENT &&
	       geState.HilightMode == GEHILIGHT_BLINK)
	     {geVn = 0;
	      SSegP = (struct GE_SEG *)ASegP->Private;
	      while (SSegP) {GEVEC(GEADDVERT, SSegP); SSegP = SSegP->Next;}
	      if (geVn > 2)
		{if (geVert[geVn].x != geVert[0].x ||
		     geVert[geVn].y != geVert[0].y)
			{geVert[geVn] = geVert[0]; geVn++;}
		 if (cmd == GEHDISP)
	     	   {GE_FG_M(geGC3, ASegP->Col.RGB.pixel, GXxor);}
		 else
		   {GE_FG_M(geGC3, ASegP->Col.RGB.pixel,
			    ASegP->FillWritingMode);}
    	    	 GEFILSTYLE_FOS(geGC3, ASegP->FillStyle, ASegP->Col.RGB.pixel);
		 GESTIPPLE(geGC3, ASegP->FillHT);
		 XFillPolygon(geDispDev, geState.Drawable, geGC3, geVert, geVn,
			      Complex, CoordModeOrigin);
	        }
	     }
	   /*
	    * Now send the command to the members
	    */
	   SSegP = (struct GE_SEG *)ASegP->Private;
	   while (SSegP)
		{if (SSegP != geState.ASegP)
		   {GEVEC(cmd, SSegP);}
		 SSegP = SSegP->Next;
		}
	  }
	break;

    case GEANIMRESET:
	if (ASegP && ASegP->Visible)
	  geAnimReset(ASegP);
	break;

    case GEMOVE:
    case GEGRAVTST:
    case GESCALE:
    case GESCRL:
    case GESCRR:
    case GESCRU:
    case GESCRD:
        if ((cmd == GEGRAVTST || cmd == GEMOVE) &&
	    (!ASegP || !ASegP->Visible)) break;

        if (ASegP && ASegP->Private)
	  {if (geState.Grav.Align || geState.APagP->Grid.XAlign ||
	       geState.APagP->Grid.YAlign)
	     {geState.Grav.Lock = FALSE;
	      SSegP = (struct GE_SEG *)ASegP->Private;
	      while (SSegP && !geState.Grav.Lock)
		{GEVEC(GEGRAVTST, SSegP); SSegP = SSegP->Next;}
	     }

	   if (cmd != GEGRAVTST)
	     {GravAlignSav               = geState.Grav.Align;
	      GridXAlignSav              = geState.APagP->Grid.XAlign;
	      GridYAlignSav              = geState.APagP->Grid.YAlign;

	      geState.Grav.Align         = FALSE;
	      geState.APagP->Grid.XAlign = FALSE;
	      geState.APagP->Grid.YAlign = FALSE;

	      SSegP = (struct GE_SEG *)ASegP->Private;
	      while (SSegP) {GEVEC(cmd, SSegP); SSegP = SSegP->Next;}
	      geState.Grav.Align         = GravAlignSav;
	      geState.APagP->Grid.XAlign = GridXAlignSav;
	      geState.APagP->Grid.YAlign = GridYAlignSav;
	     }
	  }
	break;

#ifdef GERAGS

    case GEGETFILL:
	if (ASegP && ASegP->Visible)
	   {if (geGetPutFlgs & GEGETPUTCOL)
		geSampleAttr.FilCol = ASegP->Col;
	    if (geGetPutFlgs & GEGETPUTHT)
		geSampleAttr.FillHT = ASegP->FillHT;
	    if (geGetPutFlgs & GEGETPUTSTYLE)
		geSampleAttr.FillStyle = ASegP->FillStyle;
	    if (geGetPutFlgs & GEGETPUTWRITEMODE)
	    	geAttr.WritingMode = ASegP->FillWritingMode;
	    SSegP = (struct GE_SEG *)ASegP->Private;
	    while (SSegP) {GEVEC(cmd, SSegP); SSegP = SSegP->Next;}
	   }
	break;

    case GEPUTFILL:
	if (ASegP && ASegP->Visible)
	   {if (geGetPutFlgs & GEGETPUTCOL)
		ASegP->Col    	       = geSampleAttr.FilCol;
	    if (geGetPutFlgs & GEGETPUTHT)
		ASegP->FillHT 	       = geSampleAttr.FillHT;
	    if (geGetPutFlgs & GEGETPUTSTYLE)
		ASegP->FillStyle       = geSampleAttr.FillStyle;
	    if (geGetPutFlgs & GEGETPUTWRITEMODE)
	    	ASegP->FillWritingMode = geAttr.WritingMode;
	    SSegP = (struct GE_SEG *)ASegP->Private;
	    while (SSegP) {GEVEC(cmd, SSegP); SSegP = SSegP->Next;}
	   }
	break;

    case GEGETTXTBG:
	if (ASegP && ASegP->Visible)
	   {if (geGetPutFlgs & GEGETPUTCOL)
		geSampleAttr.TxtBgCol = ASegP->Col;
	    if (geGetPutFlgs & GEGETPUTHT)
		geSampleAttr.TxtBgHT = ASegP->FillHT;
	    if (geGetPutFlgs & GEGETPUTSTYLE)
		geSampleAttr.TxtBgStyle = ASegP->FillStyle;
	    if (geGetPutFlgs & GEGETPUTWRITEMODE)
	    	geAttr.WritingMode = ASegP->FillWritingMode;
	    SSegP = (struct GE_SEG *)ASegP->Private;
	    while (SSegP) {GEVEC(cmd, SSegP); SSegP = SSegP->Next;}
	   }
	break;

    case GEPUTTXTBG:
	if (ASegP && ASegP->Visible)
	   {if (geGetPutFlgs & GEGETPUTCOL)
		ASegP->Col    	       = geSampleAttr.TxtBgCol;
	    if (geGetPutFlgs & GEGETPUTHT)
		ASegP->FillHT 	       = geSampleAttr.TxtBgHT;
	    if (geGetPutFlgs & GEGETPUTSTYLE)
		ASegP->FillStyle       = geSampleAttr.TxtBgStyle;
	    if (geGetPutFlgs & GEGETPUTWRITEMODE)
	    	ASegP->FillWritingMode = geAttr.WritingMode;
	    SSegP = (struct GE_SEG *)ASegP->Private;
	    while (SSegP) {GEVEC(cmd, SSegP); SSegP = SSegP->Next;}
	   }
	break;

    case GEEQUATE:
    case GEEQUATE_NOIMG:
        if(ASegP && geState.ASegP && ASegP->Private &&
	   geState.ASegP->Private &&ASegP->Handler == geState.ASegP->Handler)
	  {geState.ASegP->Co = ASegP->Co;
	   geGenCopyAnim(geState.ASegP, ASegP);
	   geGenCopyDesc(geState.ASegP, ASegP);
	   SSegP         = (struct GE_SEG *)ASegP->Private;
	   GSegP         = geState.ASegP;
	   geState.ASegP = (struct GE_SEG *)geState.ASegP->Private;
	   while (SSegP && geState.ASegP)
             {LiveSav     = SSegP->Live;
              SSegP->Live = TRUE;
	      GEVEC(cmd, SSegP);
              if (!(SSegP->Live = LiveSav))
                geState.ASegP->Live = FALSE;
	      geState.ASegP = geState.ASegP->Next;
	      SSegP         = SSegP->Next;
	     }
	   geState.ASegP = GSegP;
          }
	break;

    case GEGRPINIT:
	geSegCr();
	geState.ASegP->FillStyle  = GETRANSPARENT;
	geState.ASegP->Handle[0]  = 'G';
	geState.ASegP->Handle[1]  = 'R';
	geState.ASegP->Handle[2]  = 'P';
	geState.ASegP->Handle[3]  = '\0';
	geState.ASegP->Handler    = geGrp;
	geState.GSegP             = geState.ASegP;
	break;

    case GECRGRP:
	editcmd = geState.EditCmd;		/* Save CUR Edit Command     */
	ChkPtSav                = geState.ChkPt.On;
	geState.ChkPt.On = FALSE;
	/*
	 * There must be at least 2 undeleted segments -
	 *otherwise user gets a beep.
	 */
	GEVEC(GEINQLIVE, geSeg0);
	if (geState.LiveSegs < 2)
	  {XBell(geDispDev, GEBEEP_ERR);
	   geState.Mode = -1;
	   break;
	  }

        if (ASegP == geSeg0)
          {geGrp(GEJOINALL, NULL);
           geState.EditCmd = editcmd;           /* Restore OLD edit command  */
           geState.ChkPt.On = ChkPtSav;
           geState.Mode = -1;
           break;
          }
	
	geGrp(GEGRPINIT, NULL);
	GSegP = geState.GSegP;

	/*
	 * Note:  Select will wind its way back to this routine (via Anim)
	 *to cmd = JOIN - it may seem circuitous but if you think about it,
	 *it makes sense - I think.
	 */
	geState.EditCmd = GEJOIN;
	geState.EditMsg = 25;
	geState.ASegP = ASegP;
	geGrp(GEJOIN, geState.ASegP);

	while (geState.GSegP)		        /* Get ALL members for GROUP */
	  {geState.LiveSegs    = 0;
	   GEVEC(GEINQLIVE, geSeg0);            /* If less than two free     */
	   if (geState.LiveSegs < 2)            /* segs remain in grf =>     */
	     {geState.Mode = -1;                /* auto terminate the        */
	      XBell(geDispDev, GEBEEP_DONE);    /* grouping process          */
	      geState.ChkPt.On = ChkPtSav;
	      break;
	     }

	   geMnStat(GEDISP, 25);
           XDefineCursor(geDispDev, geState.APagP->Surf.self, geMouseCursor); 
	   XNextEvent(geDispDev, &geState.Event);
	   if (geState.Event.type == GE_Bd)
	     {if (geState.Event.xbutton.window ==
		  geState.APagP->Surf.self &&
		  geState.Event.xbutton.button == GEBUT_L)
		geSelect(NULL);
	      else
		{/*
		  * TERMINATION = RIGHT button or ANY button outside of
		  * DRAWING area (Let the command associated with the
		  * event, if any, be handled later on).
		  */
		  if (geState.Event.xbutton.button == GEBUT_R ||
		      geState.Event.xany.window != geState.Window)
		    {XPutBackEvent(geDispDev, &geState.Event);
		     geState.Mode = -1;
		     XBell(geDispDev, GEBEEP_DONE);
		     geState.ChkPt.On = ChkPtSav;
		     XUndefineCursor(geDispDev, geState.APagP->Surf.self);
   	      	     break;
		    }
		}
	     }
	  }
		
	if (GSegP->Private)                     /* Anybody in there?         */
	  {GSegP->Next   = NULL;                /* Grp segs, priv to grp     */
	   GEVEC(GEBOUNDS, GSegP);              /* Establish group bounds    */
	   GEVEC(GEEXTENT, GSegP);              /* Set clip to group bounds  */
	   if (!geState.StackOnTop)             /* Push it to the bottom if  */
	     geAnim(GEZB, GSegP);               /* stack flag indicates so   */
	   GEVEC(GEERASE,  GSegP);              /* Erase everyone intersectng*/
	   geState.ASegP = NULL;
	   GEVEC(GECDISP,  geSeg0);             /* Redraw everyone inersectng*/
	   GEVEC(GEZRENUM, geSeg0);             /* Renumber all Zs in grf    */
	   geState.ASegP = GSegP;
	  }
	else
	  {GEVEC(GEKILL, GSegP);
	   geState.ASegP = NULL;
	  }

	geState.EditCmd = editcmd;		/* Restore OLD edit command  */
	geState.Mode = -1;
	geState.GSegP = NULL;
	break;

    case GEZRELINK:
	if (!ASegP || !ASegP->Visible || !ASegP->Private) break;

	for(;;)
	  {i = 0;                              /* Number of switches         */
	   SSegP = (struct GE_SEG *)ASegP->Private;

	   while (SSegP && (NSegP = SSegP->Next))
	     {if (NSegP->Z < SSegP->Z)
		{TSegP = SSegP->Prev;
		 if ((struct GE_SEG *)ASegP->Private == SSegP)
		   ASegP->Private = (char *)NSegP;
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
	if (!ASegP || !ASegP->Visible ||
	    !(SSegP = (struct GE_SEG *)ASegP->Private)) break;
        
	SSegP->Z = ASegP->Z + 1;

        while ((SSegP = SSegP->Next))
	  {SSegP->Z = SSegP->Prev->Z + 1;
	   GEVEC(cmd, SSegP);
	  }
	break;

    case GECOPY:
    case GECOPYFLP:
	if (!ASegP || !ASegP->Visible || !ASegP->Private) break;
	geSegCr();
	geGenCopySeg(geState.ASegP, ASegP);
	GSegP = geState.ASegP;

	i = 0;
        SSegP = (struct GE_SEG *)ASegP->Private;
	while (SSegP)
	  {geState.ASegP = NULL;
           LiveSav     = SSegP->Live;
           SSegP->Live = TRUE;
	   GEVEC(cmd, SSegP);
           if (!(SSegP->Live = LiveSav) && geState.ASegP)
             geState.ASegP->Live = FALSE;
	   if (geState.ASegP && !i)
	     {GSegP->Private = (char *)geState.ASegP;
	      geState.ASegP->Prev = NULL;
	      i = 1;
	     }
	   SSegP = SSegP->Next;
	  }

	GSegP->Next = NULL;                     /* This makes grp-segs  priv-*/
	geState.ASegP = GSegP;                  /*ate to group               */
	GEVEC(GEBOUNDS, GSegP);
/*	GEVEC(GEZRELINK, GSegP); */
	break;

    case GECONTROL:
        if (ASegP)
	   {if (abs(geState.Mouse.x - ASegP->Co.x1) >
		abs(geState.Mouse.x - ASegP->Co.x2))
		geState.Mouse.x = ASegP->Co.x1;
	    else
		geState.Mouse.x = ASegP->Co.x2;
	    if (abs(geState.Mouse.y - ASegP->Co.y1) >
		abs(geState.Mouse.y - ASegP->Co.y2))
		geState.Mouse.y = ASegP->Co.y1;
	    else
		geState.Mouse.y = ASegP->Co.y2;
	   }
	break;

    case GEEXTENT0:
        if (ASegP && ASegP->Visible &&
	    (SSegP = (struct GE_SEG *)ASegP->Private))
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

    case GEBOUNDS:
        if (ASegP && ASegP->Visible &&
	    (SSegP = (struct GE_SEG *)ASegP->Private))
	  {GSegP = ASegP;
	   GSegP->Co.x1 = GSegP->Co.y1 =  GEMAXLONG;
	   GSegP->Co.x2 = GSegP->Co.y2 = -GEMAXLONG;
	   while (SSegP)
	    {if (SSegP->Live)
	       {GEVEC(cmd, SSegP);
		if (SSegP->Co.x1 !=  GEMAXLONG || SSegP->Co.y1 !=  GEMAXLONG ||
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
	     SSegP = SSegP->Next;
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
        if (ASegP && ASegP->Visible)
           {geClip.x1 = ASegP->Co.x1 - geRun.Sui2;
            geClip.y1 = ASegP->Co.y1 - geRun.Sui2;
            geClip.x2 = ASegP->Co.x2 + geRun.Sui2;
            geClip.y2 = ASegP->Co.y2 + geRun.Sui2;
           }
        break;

#ifdef GERAGS

    case GEALN:
	geGenAlign(ASegP);
	break;

    case GEALNREF:
        if (ASegP)
	   {GEVEC(GEEXTENT0, ASegP);
	    geAln.Co    = geClip;
	    geAln.Pt.x  = (geAln.Co.x1 + geAln.Co.x2) >> 1;
	    geAln.Pt.y  = (geAln.Co.y1 + geAln.Co.y2) >> 1;
	   }
	break;

    case GEALNCH:
    case GEALNCV:
	if (ASegP)
	   {geState.Dx = geState.Dy = 0;
	    if (cmd == GEALNCV)
		geState.Dx = geAln.Pt.x - ((ASegP->Co.x1 + ASegP->Co.x2) >> 1);
	    if (cmd == GEALNCH)
		geState.Dy = geAln.Pt.y - ((ASegP->Co.y1 + ASegP->Co.y2) >> 1);
	   }
	break;

    case GESELPTINFILL:
	geSegSel = geState.PGrpP = NULL;
	if (ASegP && ASegP->Visible && !geState.EditPts)
	  {if (ASegP->Co.x1 > geSel.x2 || ASegP->Co.y1 > geSel.y2 ||
	       ASegP->Co.x2 < geSel.x1 || ASegP->Co.y2 < geSel.y1)
		 break;
	   if (!(SSegP = (struct GE_SEG *)ASegP->Private)) break;

	   /*
	    * Since a group may be composed of individual filled objects,
	    * first see if any of those objects responds by itself to this
	    * selection request.
	    */
	   while (SSegP->Next) SSegP = SSegP->Next;

	   if ((geSegSel = geGenSelPt(cmd, SSegP, NULL, NULL, GEDOWN)))
		{geState.PGrpP = ASegP;
		 geState.SelRadC = 0;
		 break;
		}

	   if (ASegP->FillStyle == GETRANSPARENT) break;

	   SSegP = (struct GE_SEG *)ASegP->Private;
	   /*
	    * This must be a filled object, so if the selection point is INSIDE
	    * of it, then PICK it, set geState.SelRadC = 0.  The test for being
	    * INSIDE is conducted by requesting the object to establish its
	    * vertex list (walk along its perimeter, in small increments along
	    * the curved sections) and then call XPointInRegion.
	    */
	   geVn = 0;
	   GEVEC(GEADDVERT, ASegP); 
	   if ((TRegion = geXPolygonRegion(geVert, geVn, EvenOddRule)))
		{x  = (geSel.x1 + geSel.x2) >> 1;
	  	 y  = (geSel.y1 + geSel.y2) >> 1;
	  	 ix = GETPIX(x);
	  	 iy = GETPIX(y);
	  	 if (XPointInRegion(TRegion, ix, iy))
		    {geSegSel = geState.PGrpP = ASegP;
		     geState.SelRadC = 0;		/* This is IT		  */
		    }
	  	 else
	   	     geState.SelRadC = GEMAXSHORT;
	  	 XDestroyRegion(TRegion);
	  	}
	   else
	   	 geState.SelRadC = GEMAXSHORT;
	  }
	break;

    case GESELPT:
	geSegSel = geState.PGrpP = NULL;
	if (ASegP && ASegP->Visible &&
	    (SSegP = (struct GE_SEG *)ASegP->Private))
	     {if ((geSegSel = geGenSelPt(cmd, SSegP, NULL, NULL, GEUP)))
		geState.PGrpP = ASegP;
	     }
	break;

    case GESELPTIN:
	geSegSel = geState.PGrpP = NULL;
	if (ASegP && ASegP->Visible &&
	    !geState.GrpEdit && !geState.EditPts)
	  {if (ASegP->Co.x1 > geSel.x2 || ASegP->Co.y1 > geSel.y2 ||
	       ASegP->Co.x2 < geSel.x1 || ASegP->Co.y2 < geSel.y1)
		 break;
	   if (!(SSegP = (struct GE_SEG *)ASegP->Private)) break;

	   /*
	    * If the selection point is inside of any of the objects that are
	    * members of this group, select it
	    */

	   if ((geSegSel = geGenSelPt(cmd, SSegP, NULL, NULL, GEUP)))
		{geState.PGrpP = ASegP;
		 break;
		}
	   /*
	    * If the selection point is INSIDE of the group, then
	    * select the group.
	    */
	   geVn = 0;
	   GEVEC(GEADDVERT, ASegP); 
	   if ((TRegion = geXPolygonRegion(geVert, geVn, EvenOddRule)))
		{x  = (geSel.x1 + geSel.x2) >> 1;
	  	 y  = (geSel.y1 + geSel.y2) >> 1;
	  	 ix = GETPIX(x);
	  	 iy = GETPIX(y);
	  	 if (XPointInRegion(TRegion, ix, iy))	/* This IS a candidate	  */
	   	    {GEVEC(GESELPT, ASegP);		/* See how close it is    */
		     geSegSel = geState.PGrpP = ASegP;
		    }
	  	 XDestroyRegion(TRegion);
	  	}
	   else
	   	 geState.SelRadC = GEMAXSHORT;
	  }
	break;

    case GESELBX:
	if (ASegP && ASegP->Visible && ASegP->Private)
	  {if (geState.EditPts && geState.EditCmd != GECRGRP)
	  	{SSegP = (struct GE_SEG *)ASegP->Private;
	   	 geState.ASegP = NULL;
		 while (SSegP)
	     	   {GEVEC(cmd, SSegP); SSegP = SSegP->Next;}
		 if (geState.ASegP) geState.PGrpP = geState.ASegP = ASegP;
		 break;
		}
	   if (geState.GrpEdit)
	  	{SSegP = (struct GE_SEG *)ASegP->Private;
	   	 geState.ASegP = NULL;
		 while (SSegP && !geState.ASegP)
	     	   {GEVEC(cmd, SSegP); SSegP = SSegP->Next;}
		 geState.PGrpP = ASegP;
		 break;
		}
	   if (ASegP->Co.x1 > geSel.x1 && ASegP->Co.y1 > geSel.y1 &&
	       ASegP->Co.x2 < geSel.x2 && ASegP->Co.y2 < geSel.y2)
		geState.ASegP = ASegP;
	  }	
	break;

    case GEJOIN:                                /* Add another seg to group  */
        if (ASegP)
	  {/*
	    * If the incoming segment is the temp grp created through BOX Sel
	    * then just JOIN each member of the temp group into the real group.
	    * Then kill the temp group segment (remember to NULL out its
	    * Private ptr, as it no longer has members).
            */
	    if (ASegP == geGrpSel)
	     {SSegP = (struct GE_SEG *)geGrpSel->Private;
	      geGrpSel->Private = NULL;
	      while (SSegP)
		{NSegP = SSegP->Next;
		 geGrp(GEJOIN,  SSegP);
		 SSegP = NSegP;
	        }
	      GEVEC(GEKILL, geGrpSel);
	      geGrpSel = NULL;
	      break;
	     }
	   /*
	    * Check to see if the segment is ALREADY a member of this group.
	    * If so, then remove it from the group.  If EditPoints is ON,
	    * put segment back in.
	    */
       	    ASegPSav      = geState.ASegP;
       	    geState.ASegP = ASegP;
	    geGrp(GEEXTRACT, ASegP);
	    if (!geState.ASegP)
		{geState.ASegP = ASegPSav;
		 if (!geState.EditPts) return;
		}
	   /*
	    * Adjust segment chain outside of the group.  The chain is
            * COLLAPSED about the incoming segment;  BEFORE DOING THIS, THOUGH,
	    * CHECK TO SEE IF THIS SEGMENT IS THE FIRST SEGMENT IN ANOTHER
	    * GROUP AND IF SO, ADJUST THE PARENT'S POINTER.
	    */
       	    geGrf(GEPGRELINK, geSeg0);
       	    geState.ASegP = ASegPSav;
	    if (ASegP->Prev) ASegP->Prev->Next = ASegP->Next;
	    if (ASegP->Next) ASegP->Next->Prev = ASegP->Prev;
	    /*
	     * If the group already has member(s), search it for a segment
             * with a Z LARGER than that of the incoming segment.  If find
             * one then INSERT the incoming segment BEFORE that segment;
             * otherwise just tack the incoming segment onto the END of the
             * group.
	     * If the group has no members yet, then make the incoming
             * segment the first member of the group (Grp->Private = ASegP).
	     */
	    if ((SSegP = (struct GE_SEG *)geState.GSegP->Private))
	      {while (SSegP->Next)
		 {if (SSegP->Z > ASegP->Z) break;
		  SSegP = SSegP->Next;
		 }
	       if (SSegP->Z > ASegP->Z)
		 {ASegP->Prev = SSegP->Prev;
		  ASegP->Next = SSegP;
		  if (SSegP->Prev)
		    SSegP->Prev->Next = ASegP;
		  else
		    geState.GSegP->Private = (char *)ASegP;
		  SSegP->Prev = ASegP;
		 }
	       else
		 {SSegP->Next = ASegP;
		  ASegP->Prev = SSegP;
		  ASegP->Next = NULL;           /* Makes members priv. to grp*/
		 }
	      }
	    else
	      {geState.GSegP->Private = (char *)ASegP;
	       ASegP->Prev            = NULL;
	       ASegP->Next            = NULL;   /* Makes members priv. to grp*/
	      }
	   }
	break;

    case GESEP:                                 /* Break up the group        */
        if (ASegP && ASegP->Private)
	  {/*
	    * 1) Save Ptr to group                             (GSegP).
	    * 2) Get Ptr to Seg PREceeding group GSegP->Prev   (PSegP).
	    *  (Note: if this NULL -> it indicates that the group to be 
            *   disbanded is the first member of another group - SegKil
            *   will call geGrf with GEPGRELINK, to check for this eventuality
            *   and this command will cause the segment list to be searched
            *   locate the parent group and its Private ptr will be adjust
            *   to point at the first member of the group being dissolved).
	    * 3) Get Ptr to 1st group member                   (SSegP).
	    * 4) Get Ptr to Lst group member                   (LSegP).
	    * 5) Kill the group (no recovery)                  (GSegP).
	    * 6) Insert the members of the group in the spot formerly
	    *    occupied by the group - this spot is NOW pointed to by
	    *    PSegP->Next (might be NULL).
	    *    This is accomplished by doing the following:
	    *    - Set Prev of 1st group member (SSegP->Prev) = ASegP
	    *    - Set Next of Lst group member (NSegP->Next) = ASegP->Next
	    *    - Set Next of ASegP = SSegP
	    *    - If NSegP->Next not NULL then NSegP->Next->Prev = NSegP
	    */
	    if (ASegP->FillStyle != GETRANSPARENT)
	                                        /* If the group had a  fill  */
	      {GEVEC(GEERASE,  ASegP);          /*then erase it and everyone */
	       ASegP->FillStyle = GETRANSPARENT;/* Disbanded group - no inter*/
	       ASegPSav = geState.ASegP;
	       geState.ASegP = NULL;            /* Will want to redraw edges */
	       if (geState.DamageRepair)        /* Of course the disbanded   */
		 {GEVEC(GECDISP,  geSeg0);}     /*grp doesnt have an interior*/
	       geState.ASegP = ASegPSav;
	      }
	    GSegP = ASegP;
	    PSegP = GSegP->Prev;
	    NSegP = GSegP->Next;
	    SSegP = LSegP = (struct GE_SEG *)GSegP->Private;
	    while (LSegP->Next) LSegP = LSegP->Next;
	    geSegKil(&GSegP);
	    SSegP->Prev            = PSegP;
	    if (PSegP) PSegP->Next = SSegP;
	    LSegP->Next            = NSegP;
	    if (NSegP) NSegP->Prev = LSegP;
	    geState.GSegP          = NULL;      /* No longer have active grp */
	   }
	break;

    case GEEXTRACT:                             /* Remove 1 seg from group   */
        if (ASegP && geState.GSegP &&
	    ((SSegP = (struct GE_SEG *)geState.GSegP->Private)))
              {while (SSegP)
                 {if (SSegP == ASegP)		/* Located Seg to be extractd*/
		    {/*
		      * Check to see if this is first seg in Grp, if so, then
		      * readjust Grp->Private to point to next element in
		      * grp (SSegP->Next).
		      */
		     if (SSegP == (struct GE_SEG *)geState.GSegP->Private)
			 geState.GSegP->Private = (char *)SSegP->Next;
		     /*
		      * Remove the segment from the group, adjust the
		      * linkage of the next segment, if there is one,
		      * then attach the segment to the end of the display
		      * list and finally, relink the Grf according to Z order.
		      *
		      * NULL out geState.ASegP to indicate that EXTRACT was
		      * successfull.
		      */
		     if (SSegP->Prev) SSegP->Prev->Next = SSegP->Next;
		     if (SSegP->Next) SSegP->Next->Prev = SSegP->Prev;
		     LSegP         = geGenSegL();
		     LSegP->Next   = SSegP;
		     SSegP->Prev   = LSegP;
		     SSegP->Next   = NULL;
		     geState.ASegP = NULL;
		     GEVEC(GEZRELINK, geSeg0);
		     return;
		    }
                  SSegP = SSegP->Next;
                 }
	      }
	break;

    case GEJOINALL:                             /* Join ALL grf segs into grp*/
	geSegCr();                              /* Newly formed groups are   */
	geState.ASegP->FillStyle  = GETRANSPARENT;
	                                        /*NOT automatically filled   */
	geState.ASegP->Handle[0]  = 'G';
	geState.ASegP->Handle[1]  = 'R';
	geState.ASegP->Handle[2]  = 'P';
	geState.ASegP->Handle[3]  = '\0';
	geState.ASegP->Handler    = geGrp;
	GSegP                     = geState.GSegP = geState.ASegP;
	SSegP                     = geSeg0->Next;
	while(SSegP && (SSegP != GSegP))
	  {NSegP = SSegP->Next;                 /* Get NOW, JOIN nulls Next  */
	   if (SSegP->Live && SSegP->Visible)
	     geGrp(GEJOIN, SSegP);              /* Add seg to group          */
	   SSegP = NSegP;                       /* Next segment in chain     */
	  }
	GEVEC(GEBOUNDS, GSegP);                 /* Establish group bounds    */
	GSegP->Next   = NULL;                   /* Grp segs, priv to grp     */
	geState.GSegP = NULL;                   /* No longer have active grp */
	break;

    case GEISOTXT_GRP:                          /* Isolate TXTsegs into nugrp*/
	if (!ASegP->Visible) break;

	GEVEC(GECOPY, ASegP);                   /* Make a copy               */
	GSegP = geState.ASegP;                  /* Save Ptr to new group     */
	GEVEC(GEKILTXT_GRP, ASegP);             /* Remove txt segs from orig.*/
	GEVEC(GEKILNONTXT_GRP, GSegP);          /* Remove non Txt Segs       */
	GEVEC(GESCRUB, ASegP);
	GEVEC(GESCRUB, GSegP);
	GEVEC(GEBOUNDS, geSeg0);                /* Establish group bounds    */

	geState.GSegP = geState.ASegP = NULL;   /* No longer have active grp */
	break;

    case GEKILTXT_GRP:                          /* Kill txt in grp & subgrps */
    case GEKILNONTXT_GRP:                       /* Kill NONtxt in grp&subgrps*/
	if (!ASegP || !ASegP->Visible || !ASegP->Private) break;

	NSegP = (struct GE_SEG *)ASegP->Private;/* 1st seg in group          */
	/*
	 * Kill the requested type of segs in this group and dispatch the
	 * command to this group's sub-segs
	 */
	do
	  {SSegP = NSegP;
	   PSegP = SSegP->Prev;
	   NSegP = SSegP->Next;
	   if (cmd == GEKILTXT_GRP    && SSegP->Handler == geTxt)
	     geSegKil(&SSegP);
	   else
	   if (cmd == GEKILNONTXT_GRP && SSegP->Handler != geTxt &&
	       SSegP->Handler != geGrp)
	     geSegKil(&SSegP);
	   else
	     {GEVEC(cmd, SSegP); SSegP = NULL;}
	  }while(NSegP);

	break;

    case GEADDTXT_GRP:                          /* Check this group to see if*/
	if (!ASegP || !ASegP->Visible ||
	    !ASegP->Private) break;             /* it contains only TXT and  */
                                                /* group segments.  If so,   */
	                                        /* set geState.ASegP = TXTseg*/
	SSegP = (struct GE_SEG *)ASegP->Private;/* 1st seg in group          */

	do
	  {if (SSegP->Handler == geTxt) {geState.ASegP = SSegP; break;}
	   else
	   if (SSegP->Handler == geGrp)  geGrp(cmd, SSegP);
	   else                          break;
	  }while(!geState.ASegP && (SSegP = SSegP->Next));
	break;

    case GEJOINALLTXT:                          /* Join ALL TXT segs into grp*/
	geSegCr();                              /* Newly formed groups are   */
	geState.ASegP->Col.RGB.pixel  = 0;      /*NOT automatically filled   */
	geState.ASegP->Handle[0]  = 'G';
	geState.ASegP->Handle[1]  = 'R';
	geState.ASegP->Handle[2]  = 'P';
	geState.ASegP->Handle[3]  = '\0';
	geState.ASegP->Handler    = geGrp;
	GSegP                     = geState.GSegP = geState.ASegP;
	SSegP                     = geSeg0->Next;
	while(SSegP && (SSegP != GSegP))
	  {NSegP = SSegP->Next;                 /* Get NOW, JOIN nulls Next  */
	   geState.ASegP = NULL;                /* GRP will set this if needb*/
	   GEVEC(GEADDTXT_GRP, SSegP);          /* Add seg to group          */
	   if(geState.ASegP) geGrp(GEJOIN, SSegP);
	   SSegP = NSegP;                       /* Next segment in chain     */
	  }
	GEVEC(GEBOUNDS, GSegP);                 /* Establish group bounds    */
	GSegP->Next   = NULL;                   /* Grp segs, priv to grp     */
	geState.GSegP = NULL;                   /* No longer have active grp */
	break;

    case GEGPDELEMPTY:
	if (ASegP)
	  {if (ASegP->Private)
	     {geState.LiveSegs    = 0;
	      GEVEC(GEINQLIVE, ASegP);
	      if (!geState.LiveSegs) ASegP->Live = FALSE;
	      SSegP            = (struct GE_SEG *)ASegP->Private;

	      while (SSegP)
		{GEVEC(cmd, SSegP); SSegP = SSegP->Next;}

	      geState.LiveSegs    = 0;
	      GEVEC(GEINQLIVE, ASegP);
	      if (!geState.LiveSegs) ASegP->Live = FALSE;
	     }
	   else
	     ASegP->Live = FALSE;
	  }
	break;

    case GEGPLIVE:
	if (ASegP && ASegP->Private)
	  {ASegP->Live = TRUE;
	   SSegP            = (struct GE_SEG *)ASegP->Private;

	   while (SSegP)
	     {GEVEC(cmd, SSegP); SSegP = SSegP->Next;}
	 }
	break;


    case GEINQLINCOL:                           /* FIRST segment in list     */
    case GEINQFILCOL:                           /*controls these             */
    case GEINQCOL:
        if (ASegP && (SSegP = (struct GE_SEG *)ASegP->Private))
	   {GEVEC(cmd, SSegP);}
	break;

    case GEINQLIVE:                             /* How many LIVE segs in grp */
	if (ASegP && ASegP->Live && ASegP->Private)
	   {SSegP   	     = (struct GE_SEG *)ASegP->Private;
	    LVSegs  	     = geState.LiveSegs;
	    geState.LiveSegs = 0;
	    while (SSegP)
	  	{GEVEC(cmd, SSegP);		/* Just send cmd to object  */
	  	 SSegP = SSegP->Next;
	  	}
	    if (geState.LiveSegs) geState.LiveSegs = LVSegs + 1;
	    else		  geState.LiveSegs = LVSegs;
	   }
	break;

    case GEINQVISIBLE:                             /* How many VISIBLE segs  */
	if (ASegP && ASegP->Live && ASegP->Visible && ASegP->Private)
	   {SSegP            	= (struct GE_SEG *)ASegP->Private;
	    LVSegs              = geState.VisibleSegs;
	    geState.VisibleSegs = 0;
	    while (SSegP)
	  	{GEVEC(cmd, SSegP);		/* Just send cmd to object  */
	  	 SSegP = SSegP->Next;
	  	}
	    if (geState.VisibleSegs) geState.VisibleSegs = LVSegs + 1;
	    else		     geState.VisibleSegs = LVSegs;
	   }
	break;

    case GEINQXVISIBLE:                        /* Are there HIDDEN segs */
	if (ASegP && ASegP->Live && ASegP->Private)
	   {if (!ASegP->Visible)
	      {geState.Hidden = TRUE;
	       break;
	      }
	    SSegP = (struct GE_SEG *)ASegP->Private;
	    while (SSegP)
	  	{GEVEC(cmd, SSegP);		/* Just send cmd to object  */
	         if (geState.Hidden) 
                   break;
	  	 SSegP = SSegP->Next;
	  	}
	   }
	break;

    case GEINQMEMIMG:                       	/* Total mem needed by images*/
	geInqRes = InqRes = 0;
	if (ASegP && ASegP->Live && ASegP->Visible && ASegP->Private)
	   {SSegP            	= (struct GE_SEG *)ASegP->Private;
	    while (SSegP)
	  	{GEVEC(cmd, SSegP);		/* Just send cmd to object  */
		 InqRes += geInqRes;
	  	 SSegP   = SSegP->Next;
	  	}
	    geInqRes = InqRes;
	   }
	break;

    case GEVISIBLE:
	if (ASegP) ASegP->Visible     = TRUE;
	break;

    case GEPURGEHIDDEN:
	if (ASegP && !ASegP->Visible)
	  {GEVEC(GEKILL, ASegP);}		/* KILL it now		     */
	break;

    case GEDEL:
    case GEXDEL:
	if (ASegP)
	  {/*
	    * Pass command to group members.
            */
	   SSegP = (struct GE_SEG *)ASegP->Private;
	   while (SSegP)
		{NSegP = SSegP->Next;
	   	 GEVEC(cmd, SSegP);
		 SSegP = NSegP;
	        }
	    if (cmd == GEDEL)
	    	{ASegP->Live = FALSE;           /* Flag group as non-live    */
	   	 if (ASegP == geGrpSel)	        /* NULL geGrpSel, so geSelect*/
		    geGrpSel = NULL;		/* does not disband it - need*/
	   	}				/* it for UnDelete           */
	    else
	    	 ASegP->Live = TRUE;            /* Flag group as Live again  */
	  }
	break;

    case GEXVISIBLE:
	if (ASegP) ASegP->Visible     = FALSE;
	break;

    case GEPGRELINK:
	if (ASegP && geState.ASegP && ASegP->Private &&
	    (ASegP->Private == (char *)geState.ASegP))
	 {/*
           * Found the parent group.  Now repoint the Private of the
           * parent group to be the Next of the seg being dissolved
           * if that seg is NOT either another group or a polygon.  If
           * it is either of these, then repoint Private of the parent
           * group to the Private of the group or poly which is being
           * dissolved.
           */
	   if (geState.ASegP->Handler == geGrp ||
	       geState.ASegP->Handler == gePol)
	     ASegP->Private = geState.ASegP->Private;
	   else
	     ASegP->Private = (char *)geState.ASegP->Next;
	  geState.ASegP = NULL;
	 }
	else
	  {SSegP = (struct GE_SEG *)ASegP->Private;

	   while (SSegP)
	     {GEVEC(cmd, SSegP); SSegP = SSegP->Next;}
	  }
	break;

#endif

    case GEGETSEG_COL:
        if (ASegP)
	  {if (ASegP->Col.RGB.pixel == geGenCol.pixel)
	     geState.ASegP = ASegP;
	  else
	    {SSegP = (struct GE_SEG *)ASegP->Private;
	     while (SSegP && !geState.ASegP)
	       {GEVEC(cmd, SSegP);
		SSegP = SSegP->Next;
	       }
	    }
	  }
	break;

    case GEKILL:
	if(ASegP)
	  {if ((SSegP = (struct GE_SEG *)ASegP->Private))
	     {while (SSegP->Next) SSegP = SSegP->Next;
	      do
		{PSegP = SSegP->Prev;
		 GEVEC(cmd, SSegP);
	        }while (SSegP = PSegP);
	     }
	   ASegP->Private = NULL;
	   geSegKil(&ASegP);
	  }
	break;

    case GESCRUB:
	if(ASegP)
	  {if ((SSegP = (struct GE_SEG *)ASegP->Private))
	     {do
		{NSegP = SSegP->Next;
		 GEVEC(cmd, SSegP);
	        }while (SSegP = NSegP);
	     }

	   if (!ASegP->Private)
	     geSegKil(&ASegP);
           else
             {SSegP = (struct GE_SEG *)ASegP->Private;
              if (!SSegP->Next) geSegReplace(&ASegP, &SSegP);
	     }
	  }
	break;

    case GEWRITE:
    case GEREAD:
	geGrpIO(cmd, ASegP);
	break;

    case GEEDIT:
    case GEOBJEDIT:
        error_string = (char *) geFetchLiteral("GE_ERR_NOEDIT", MrmRtypeChar8);
        if (error_string != NULL) 
	   {geError(error_string, FALSE);
	    XtFree(error_string);
	   }
	break;

    default:
	break;

   }
}

