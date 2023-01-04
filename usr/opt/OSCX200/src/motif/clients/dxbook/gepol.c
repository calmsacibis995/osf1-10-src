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
static char SccsId[] = "@(#)gepol.c	1.19\t10/16/89";
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
**	GEPOL	             	       Polygon object handler
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
**        And the somewhat subtle distinction of CDISP -
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

extern               	geLin();
extern               	geGrp();
extern unsigned long 	geAllocColor();
extern char          	*geMalloc();
extern struct GE_SEG 	*geGenSegL(), *geGenSelPt();
extern Region		geXPolygonRegion();

gePol(cmd, ASegP)
long     	cmd;
struct GE_SEG	*ASegP;
{                       
#define		 GEMOTION_THRES	2
#define		 GEDELETE_THRES	7

char             p[10];
short            xp1, yp1, stop, ChkPtSav, ObjEdCrSav, LiveSav, FillStyleSav,
		 dxp, dyp, AutoTrack, HintsAllowed, Live;
int              i, c, GravAlignSav, GravAlignSav2,
                 GridXAlignSav, GridYAlignSav, ix, iy, NumFrames;
long             AnimMinSav, GravRadSav2, x, y, NumPts, LVSegs, CrCmd;
unsigned long	 Pfil;

struct GE_SEG    *PSegP, *SSegP, *SSegX, *NSegP, *TSegP, *LSegP;

Window           winsave;
XEvent           Event;
Region		 TRegion;
char		 *error_string;

if (ASegP &&
    (!ASegP->Live && cmd != GEINQIMGREF && cmd != GEKILL &&
                     cmd != GESCRUB     && cmd != GEGETSEG_COL &&
		     cmd != GEXDEL))
    return;

switch(cmd)
   {case GEWARP1:
    case GEWARP2:
    case GEWARPX:
    case GEGETLINE:
    case GEGETTXTFG:
    case GEPUTLINE:
    case GEPUTTXTFG:
    case GEROTX:
    case GEROTFIXED:
    case GEFLIP:
    case GEMAG:                            	/* Magnify by specified %    */
    case GEMAGRESD:                            	/* Magnify by specified %    */
    case GEIMGMAG:
    case GEGRAV:
    case GECOMP:
    case GEINQIMGREF:
    case GEROUND:
    case GETESTPT:
    case GETESTPTCLR:
    case GEPSADJUST:
    case GEADDVERT:
        if (ASegP && ASegP->Visible)
	  {if (cmd == GEGRAV &&
	    (geState.Grav.Pt.x < (ASegP->Co.x1 - geState.Grav.Rad) ||
	     geState.Grav.Pt.y < (ASegP->Co.y1 - geState.Grav.Rad) ||
	     geState.Grav.Pt.x > (ASegP->Co.x2 + geState.Grav.Rad) ||
	     geState.Grav.Pt.y > (ASegP->Co.y2 + geState.Grav.Rad)))
	     break;

	   SSegP = (struct GE_SEG *)ASegP->Private;
	   if (cmd == GEMAG && !geMagGrp)
		{geMagGrp = TRUE;
		 geMagCx  = (ASegP->Co.x1 + ASegP->Co.x2) >> 1;
		 geMagCy  = (ASegP->Co.y1 + ASegP->Co.y2) >> 1;
		}
	   /*
	    * Send the command to all of the members
	    */
	   while (SSegP) {GEVEC(cmd, SSegP); SSegP = SSegP->Next;}

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
	  }
	break;

    case GEDISP:
        if (ASegP && ASegP->Visible && ASegP->InFrame &&
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
	      * See if Poly needs to be filled - if so then get segments to
	      *add themselves to the vertex list and display the interior
	      *prior to displaying the edges.
	      */
	      if (ASegP->WhiteOut)
		{GEVEC(GESETATTRERASE, ASegP);}

	      if (ASegP->FillStyle != GETRANSPARENT)
		{geVn = 0;
		 SSegP = (struct GE_SEG *)ASegP->Private;
		 while (SSegP) {GEVEC(GEADDVERT, SSegP); SSegP = SSegP->Next;}
		 if (geVn > 2)
		   {GE_SET_BG_DISP_ATTR;
		    GE_FG_M(geGC3, geDispAttr.BgPixel, ASegP->FillWritingMode);
		    GEFILSTYLE_FOS(geGC3, geDispAttr.BgStyle,
				   geDispAttr.BgPixel);
		    GESTIPPLE(geGC3, geDispAttr.BgHT);
		    XFillPolygon(geDispDev, geState.Drawable, geGC3, geVert,
				 geVn, Complex, CoordModeOrigin);
		   }
	        }
	      /*
	       * Now get the edges to display themselves
	       */
	      SSegP = (struct GE_SEG *)ASegP->Private;
	      while (SSegP && !geState.HaltRequested)
		{if (SSegP != geState.ASegP) GEVEC(cmd, SSegP);
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
	  geRespondToInterrupt(ASegP);
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
        if (ASegP && ASegP->Visible && ASegP->InFrame)
	   {if (geState.HilightMode == GEHILIGHT_BOX && !geState.EditPts &&
		!geState.GrpEdit)
		{GEVEC(GEEXTENT, ASegP);    	/* EXTENT RECTANGLE          */
		 XDrawRectangle(geDispDev, geState.Drawable, geGC5,
                        	GETPIX(geClip.x1), GETPIX(geClip.y1),
                        	GETPIX(geClip.x2 - geClip.x1),
				GETPIX(geClip.y2 - geClip.y1));
		}
	    else
	   	{/*
	    	  * Let edges highlight themselves
	    	  */
	  	 SSegP = (struct GE_SEG *)ASegP->Private;
	  	 while (SSegP)
		    {if (SSegP != geState.ASegP) GEVEC(cmd, SSegP);
		     SSegP = SSegP->Next;
		    }
		}
	  }
	break;

    case GEXDISP:
    case GEGDISP:
        if (ASegP && ASegP->Visible)
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
	  }
	 geAnimMin  = AnimMinSav;
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
        if ((cmd == GESCALE || cmd == GEMOVE) &&
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
	    	geSampleAttr.FillWritingMode = ASegP->FillWritingMode;
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
	    	ASegP->FillWritingMode = geSampleAttr.FillWritingMode;
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
	    	geSampleAttr.FillWritingMode = ASegP->FillWritingMode;
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
	    	ASegP->FillWritingMode = geSampleAttr.FillWritingMode;
	   }
	break;

    case GEEQUATE:
    case GEEQUATE_NOIMG:
        if(ASegP && geState.ASegP && ASegP->Handler == geState.ASegP->Handler)
	  {geState.ASegP->Co = ASegP->Co;
	   geGenCopyAnim(geState.ASegP, ASegP);
	   geGenCopyDesc(geState.ASegP, ASegP);
	   SSegP         = (struct GE_SEG *)ASegP->Private;
	   PSegP         = geState.ASegP;
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
	   geState.ASegP = PSegP;
          }
	break;

    case GECREATE:
	if (!gePHandler) break;
	geSegCr();
	geState.ASegP->Handle[0]  = 'P';
	geState.ASegP->Handle[1]  = 'O';
	geState.ASegP->Handle[2]  = 'L';
	geState.ASegP->Handle[3]  = '\0';
	geState.ASegP->Handler    = gePol;
	PSegP                     = geState.ASegP;
	geMnStat(GEDISP, 54);			/* Put up Poly Cr msg once   */
	/*
	 * Must dissable "fill", and "create obj to size" for duration
	 *of poly creation - otherwise
	 *get somewhat bizzare results.  Also must disable check-pointing
	 *because the partial completed polygon causes problems.
	 */
	FillStyleSav            = geAttr.FillStyle;
	Pfil                    = geAttr.FilCol.RGB.pixel;
	geAttr.FilCol.RGB.pixel = 0;
	geAttr.FillStyle        = GETRANSPARENT;
	HintsAllowed   		= geState.HintsAllowed;
	ChkPtSav                = geState.ChkPt.On;
	ObjEdCrSav              = geState.ObjEdCr;
	geState.ChkPt.On 	= FALSE;
	geState.ObjEdCr         = FALSE;
        geState.HintsAllowed 	= FALSE;

        /*
	 * Establish initial point of poly - adjust for grid or grav snap
	 * maybe
	 */
	stop    	        = FALSE;
	geState.Mouse.x         = GETSUI(geState.Event.xbutton.x);
	geState.Mouse.y         = GETSUI(geState.Event.xbutton.y);
	geMouseSnap();				/* Adj mouse grid or grav snap*/
	gePolXStart             = geState.Mouse.x;
	gePolYStart             = geState.Mouse.y;
	AutoTrack		= FALSE;
	xp1 			= GETPIX(gePolXStart);
	yp1 			= GETPIX(gePolYStart);
	NumPts			= 0;
	CrCmd			= GECREATE_NOSNAP1;

	for(;;)
	  {geSync(1);
	   if (!stop)
		{/*
		  * Wait for next event to determine what to do.  If it's a
		  * button up event, make a single new segment.  If it's a
		  * mouse motion event, determine if the displacement is
		  * sufficient from the last known point to enter freehand
		  * drawing mode.  If it's neither of these, dispatch the
		  * event to the toolkit and continue;
		  */
		 do
		    {XWindowEvent(geDispDev, geState.Window,
				  GE_MBu|GE_MMx, &Event);

		     if (Event.type == GE_Mx)
			{if (gePHandler != geLin)
			   {AutoTrack = FALSE;
			    break;
			   }
			 /*
			  * See if mouse has moved a minimum amount
			  */
			 geState.Dx = geState.Dy = 0;
	   	     	 if (geState.ScrAuto)	/* Need to auto-scrol?       */
		             {TSegP = geState.ASegP;
			      geState.ASegP = NULL;
			      geGenScrAuto(cmd, &x, &y);
			      geState.ASegP = TSegP;
			     }

			 dxp = abs(Event.xmotion.x - xp1);
			 dyp = abs(Event.xmotion.y - yp1);

			 if (dxp > GEMOTION_THRES || dyp > GEMOTION_THRES)
			    {if(!AutoTrack && XCheckWindowEvent(geDispDev,
			     			geState.APagP->Surf.self,
						GE_MBu, &Event))
				break;

			     AutoTrack = TRUE;
			     xp1 = Event.xmotion.x;
		  	     yp1 = Event.xmotion.y;
			     geState.Event.type = GE_Bd;
		 	     XPutBackEvent(geDispDev, &geState.Event);
			     break;
			    }
			}
		    }while (Event.type != GE_Bu);

		  if (Event.type == GE_Bu)
			 AutoTrack = FALSE;
		
		  if ((NumPts += 2) >= GEMAX_PTS)
			{stop = TRUE;
			 continue;
			}
		  if (AutoTrack)
		  	{GravAlignSav2      	    = geState.Grav.Align;
	       		 GridXAlignSav              = geState.APagP->Grid.XAlign;
	       		 GridYAlignSav              = geState.APagP->Grid.YAlign;
	       		 geState.Grav.Align         = FALSE;
	       		 geState.APagP->Grid.XAlign = FALSE;
	      		 geState.APagP->Grid.YAlign = FALSE;
			 geLin(GECREATE, 0); 	/* Create a LINE segment     */
		  	 geState.Grav.Align 	    = GravAlignSav2;
	      		 geState.APagP->Grid.XAlign = GridXAlignSav;
	      		 geState.APagP->Grid.YAlign = GridYAlignSav;
			 CrCmd			    = GECREATE_NOSNAP1;
			}
		  else
		  	{(*gePHandler)(CrCmd, 0);	/* Create another seg*/
			 CrCmd = GECREATE_NOSNAP1;
			 xp1   = geState.Event.xbutton.x;
		  	 yp1   = geState.Event.xbutton.y;
			}
		 }
	   else
		  geState.Event.xbutton.button = GEBUT_L;

	   if ((geState.Event.xbutton.window == geState.Window &&
	       (geState.Event.type == GE_Bd || AutoTrack)) || stop)
	      switch (geState.Event.xbutton.button)
		{case GEBUT_L:
		  /*
		   * Wait for button release
		   */
		  do
		    {XNextEvent(geDispDev, &Event);
		    }while (Event.type != GE_Bu);
		  /*
		   * See if user wants to AUTO-CLOSE - wait a short time,
		   * then if there are events in the que,
		   * see if there is a button down event; if so => autoclose
		   */
		  if (PSegP->Next)		/* Any Poly segs?	     */
			{XDefineCursor(geDispDev, geState.APagP->Surf.self,
				       geMouseCursor); 
			 geSleep(750, TRUE);
		       	 if (XPending(geDispDev))
			    {do
		       	     	{XNextEvent(geDispDev, &Event);
		       	     	}while (XPending(geDispDev) &&
				     	Event.type != GE_Bd);
		    	    }

		  	 if (Event.type == GE_Bd)
		    	    {/*
		      	      * Find last defined point
		      	      */
		     	     LSegP = geGenSegL();
			     GEVEC(GEINQPTS, LSegP);
			     geGenBx.x1 = geClip.x2;
			     geGenBx.y1 = geClip.y2;
		     	     geGenBx.x2 = gePolXStart;
		     	     geGenBx.y2 = gePolYStart;
		    	     geState.MouLockCr = FALSE;
		    	     geLin(GECREATESTAT_NOSNAP1, NULL);
			    }

		    	 PSegP->Next->Prev   = NULL;
		    	 PSegP->Private      = (char *)PSegP->Next;
		    	 PSegP->Next         = NULL;
		    	 geState.ASegP       = PSegP;
			 if (geState.WhiteOut)
			    {GEVEC(GESETATTRERASE, geState.ASegP);}
		    	 geState.Mode  	     = -1;
		    	 GEVEC(GEBOUNDS, PSegP);
		    	 geAttr.FilCol.RGB.pixel = Pfil;
		    	 geAttr.FillStyle    = FillStyleSav;
		     	 if (PSegP->FillStyle != GETRANSPARENT)
		       	    {GEVEC(GEDISP, PSegP);}
		    	 geState.ChkPt.On    = ChkPtSav;
		     	 geState.ObjEdCr     = ObjEdCrSav;
			 geState.HintsAllowed = HintsAllowed;
			 geMnStat(GEDISP, GEQUIETMSG);
		    	}			/* End NORMAL Poly TERM	     */
		    else			/* EMPTY Poly - kill it	     */
		      	{geSegKil(&PSegP);
		      	 geState.ASegP = NULL;
		      	}
		  /*
		   * This is the only way out of CREATE
		   */
	    	  XUndefineCursor(geDispDev, geState.APagP->Surf.self);
		  geState.Mode        = -1;
		  geState.ChkPt.On    = ChkPtSav;
		  geState.ObjEdCr     = ObjEdCrSav;
		  geAttr.FilCol.RGB.pixel = Pfil;
		  geAttr.FillStyle    = FillStyleSav;
		  geState.HintsAllowed = HintsAllowed;
		  geMnStat(GEDISP, GEQUIETMSG);
	   	  XBell(geDispDev, GEBEEP_DONE);
		  return;

		case GEBUT_R:
		  AutoTrack = TRUE;
		  winsave = geState.Event.xbutton.window;
		  XDefineCursor(geDispDev, geState.APagP->Surf.self,
				geMouseCursor); 
		  for (;;)
		    {LSegP = geGenSegL();
		     if (LSegP == geSeg0 || LSegP == PSegP)
			{stop = TRUE;
			 break;
			}

		     XNextEvent(geDispDev, &geState.Event);
		     if (geState.Event.type == GE_Bd &&
			 geState.Event.xbutton.window == winsave)
		       {if (geState.Event.xbutton.button == GEBUT_L)
			   {stop      = TRUE;
			    AutoTrack = FALSE;
			    break;
			   }
			else
		        if (geState.Event.xbutton.button == GEBUT_R)
			   {AutoTrack = TRUE;
			    geState.ASegP = LSegP;
			    geAnim(GEDELNOUNDO,  LSegP);
			    GEVEC(GEKILL, LSegP);
			    if (NumPts >= 2) NumPts -= 2;
			    continue;
			   }
			else
			   {AutoTrack = FALSE;
			    xp1	      = geState.Event.xbutton.x;
			    yp1       = geState.Event.xbutton.y;
			    break;
			   }
		       }
		     else
		        if (AutoTrack && geState.Event.type == GE_Mx)
	   	     	   {geSync(1);
			    if (geState.ScrAuto)	/* Need to auto-scrol?       */
		             {TSegP = geState.ASegP;
			      geState.ASegP = NULL;
			      geMouseXY(geState.APagP->Surf.self);
			      geGenScrAuto(cmd, &x, &y);
			      geState.ASegP = TSegP;
			     }
			    
		    	    /*
		      	     * Find last defined point
		      	     */
			     GEVEC(GEINQPTS, LSegP);
			     geClip.x2 = GETPIX(geClip.x2);
			     geClip.y2 = GETPIX(geClip.y2);
			     dxp = abs(geState.Event.xmotion.x - geClip.x2);
			     dyp = abs(geState.Event.xmotion.y - geClip.y2);
			     if (dxp > GEDELETE_THRES || dyp > GEDELETE_THRES)
				continue;

			    geState.ASegP = LSegP;
			    geAnim(GEDELNOUNDO,  LSegP);
			    GEVEC(GEKILL, LSegP);
			    if (NumPts >= 2) NumPts -= 2;
			    continue;
			   }
		     else
		       {AutoTrack = FALSE;
			if (!gePolNuH())
   			  {XtDispatchEvent(&geState.Event);  
			   geDispatch();
			  }
		       }
		    }
	    	  XUndefineCursor(geDispDev, geState.APagP->Surf.self);
		  geState.Mouse.x = GETSUI(geState.Event.xbutton.x);
		  geState.Mouse.y = GETSUI(geState.Event.xbutton.y);
		  /*
		   * Grav align the point so that poly connection may be
		   * more easily established.
		   */
		  GravAlignSav2      = geState.Grav.Align;
		  GravRadSav2        = geState.Grav.Rad;
		  geState.Grav.Align = TRUE;
		  geState.Grav.Rad   = GETSUI(7);
		  geMouseSnap();
		  geState.Grav.Align = GravAlignSav2;
		  geState.Grav.Rad   = GravRadSav2;
		  CrCmd		     = GECREATE_NOSNAP1;
		  break;
	        }				/* END of inner switch	     */
	 }					/* End of creation for loop  */

	break;

    case GESETATTRERASE:
	if (ASegP && ASegP->Visible && ASegP->WhiteOut)
	  {ASegP->Col            	= geAttr.PagCol;
	   ASegP->FillHT	      	= 100;
	   ASegP->FillStyle      	= FillSolid;
	   ASegP->FillWritingMode	= GXcopy;
	  }
	break;

    case GESETATTRERASE_WHITE:
	if (ASegP && ASegP->Visible && ASegP->WhiteOut && !geState.PrintPgBg)
	  {ASegP->Col.RGB       = geWhite;
	   ASegP->Col.Name      = "White";
	   geGenRgbToCmyk(&(ASegP->Col.RGB), &(ASegP->Col.CMYK));
	  }
	break;

    case GEZRELINK:
	if (!ASegP || !ASegP->Visible) break;

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
	if (!ASegP || !ASegP->Visible) break;

        SSegP = (struct GE_SEG *)ASegP->Private;
	SSegP->Z = ASegP->Z + 1;

        while ((SSegP = SSegP->Next))
	  {SSegP->Z = SSegP->Prev->Z + 1;
	   GEVEC(cmd, SSegP);
	  }
	break;

    case GECOPY:
    case GECOPYFLP:
	if (!ASegP || !ASegP->Visible) break;
	geSegCr();
	geGenCopySeg(geState.ASegP, ASegP);
	PSegP                          = geState.ASegP;

	i = 0;
        SSegP = (struct GE_SEG *)ASegP->Private;
	while (SSegP)
	  {LiveSav     = SSegP->Live;
	   SSegP->Live = TRUE;
	   GEVEC(cmd, SSegP);
	   if (!(SSegP->Live = LiveSav))
	     geState.ASegP->Live = FALSE;
	   if (!i)
	     {PSegP->Private = (char *)geState.ASegP;
	      geState.ASegP->Prev = NULL;
	      i = 1;
	     }
	   SSegP = SSegP->Next;
	  }

	PSegP->Next = NULL;                     /* This makes poly-segs priv-*/
	geState.ASegP = PSegP;                  /*ate to polygon alone       */
	GEVEC(GEBOUNDS, PSegP);
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
	  {PSegP = ASegP;
	   PSegP->Co.x1 = PSegP->Co.y1 =  GEMAXLONG;
	   PSegP->Co.x2 = PSegP->Co.y2 = -GEMAXLONG;
	   while (SSegP)
	    {if (SSegP->Live)
	       {GEVEC(cmd, SSegP);
		if (SSegP->Co.x1 !=  GEMAXLONG || SSegP->Co.y1 !=  GEMAXLONG ||
		    SSegP->Co.x2 != -GEMAXLONG || SSegP->Co.y2 != -GEMAXLONG)
		   {PSegP->Co.x1 = min(PSegP->Co.x1,
				   min(SSegP->Co.x1, SSegP->Co.x2));
		    PSegP->Co.y1 = min(PSegP->Co.y1,
				   min(SSegP->Co.y1, SSegP->Co.y2));
		    PSegP->Co.x2 = max(PSegP->Co.x2,
				   max(SSegP->Co.x1, SSegP->Co.x2));
		    PSegP->Co.y2 = max(PSegP->Co.y2,
				   max(SSegP->Co.y1, SSegP->Co.y2));
		   }
	       }
	     SSegP = SSegP->Next;
	    }
	   if (PSegP->Co.x2 < 0 || PSegP->Co.y2 < 0 ||
	       GETPIX(PSegP->Co.x1) > geState.APagP->Surf.width ||
	       GETPIX(PSegP->Co.y1) > geState.APagP->Surf.height)
	     PSegP->InFrame = FALSE;
	   else
	     PSegP->InFrame = TRUE;
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
	if (ASegP && ASegP->Visible && ASegP->FillStyle != GETRANSPARENT &&
	    !geState.GrpEdit && !geState.EditPts)
	  {if (ASegP->Co.x1 > geSel.x2 || ASegP->Co.y1 > geSel.y2 ||
	       ASegP->Co.x2 < geSel.x1 || ASegP->Co.y2 < geSel.y1)
		 break;
	   if (!(SSegP = (struct GE_SEG *)ASegP->Private)) break;

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
	    * If the selection point is INSIDE of this object, then
	    * compute the proximity - the closest object will be
	    * PICKED.
	    */
	   geVn = 0;
	   GEVEC(GEADDVERT, ASegP); 
	   geVert[geVn++] = geVert[0];
	   if ((TRegion = geXPolygonRegion(geVert, geVn, EvenOddRule)))
		{x  = (geSel.x1 + geSel.x2) >> 1;
	  	 y  = (geSel.y1 + geSel.y2) >> 1;
	  	 ix = GETPIX(x);
	  	 iy = GETPIX(y);
	  	 if (XPointInRegion(TRegion, ix, iy))	/* This IS a candidate	  */
	   	    {geGenPtSelVert(0, cmd);
		     geSegSel = geState.PGrpP = ASegP;
		    }
	  	 XDestroyRegion(TRegion);
	  	}
	   else
	   	 geState.SelRadC = GEMAXSHORT;
	  }
	break;

    case GESELBX:
	if (ASegP && ASegP->Visible)
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

    case GEINQLINCOL:                           /* FIRST segment in list     */
        if (ASegP && (SSegP = (struct GE_SEG *)ASegP->Private))
	   GEVEC(cmd, SSegP);
	break;

    case GEINQCOL:
    case GEINQFILCOL:
        if (ASegP) geAttr.Col = ASegP->Col;
	break;

    case GEINQLIVE:                             /* How many LIVE segs in poly*/
	if (ASegP && ASegP->Live && ASegP->Private)
	   {SSegP            = (struct GE_SEG *)ASegP->Private;
	    LVSegs           = geState.LiveSegs;
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
	    LVSegs 		= geState.VisibleSegs;
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

    case GEGPDELEMPTY:
	geState.LiveSegs    = 0;
	GEVEC(GEINQLIVE, ASegP);
	if (!geState.LiveSegs) ASegP->Live = FALSE;
        SSegP            = (struct GE_SEG *)ASegP->Private;
	while (SSegP)
	  {GEVEC(cmd, SSegP); SSegP = SSegP->Next;}

	geState.LiveSegs    = 0;
	GEVEC(GEINQLIVE, ASegP);
	if (!geState.LiveSegs) ASegP->Live = FALSE;
	break;

    case GEGPLIVE:
	ASegP->Live = TRUE;
        SSegP            = (struct GE_SEG *)ASegP->Private;

	while (SSegP)
	  {GEVEC(cmd, SSegP); SSegP = SSegP->Next;}
	break;

    case GEVISIBLE:
	if (ASegP) ASegP->Visible     = TRUE;
	break;

    case GEXVISIBLE:
	if (ASegP) ASegP->Visible     = FALSE;
	break;

    case GEPURGEHIDDEN:
	if (ASegP && !ASegP->Visible)
	  {GEVEC(GEKILL, ASegP);}		/* KILL it now		     */
	break;

    case GEDEL:
    case GEXDEL:
	if (ASegP)
	   {/*
	     * Dispatch cmd to sub-segments
	     */
	    SSegP = (struct GE_SEG *)ASegP->Private;
	    while (SSegP)
		{GEVEC(cmd, SSegP);
	     	 SSegP = SSegP->Next;
	     	}

	    if (cmd == GEDEL) ASegP->Live = FALSE;
	    else	      ASegP->Live = TRUE;
	   }
	break;

    case GEPGRELINK:
	if (ASegP->Private == (char *)geState.ASegP)
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
        if (ASegP && ASegP->Col.RGB.pixel == geGenCol.pixel)
	  geState.ASegP = ASegP;
        else
	  {SSegP = (struct GE_SEG *)ASegP->Private;
	   while (SSegP && !geState.ASegP)
	     {GEVEC(cmd, SSegP);
	      SSegP = SSegP->Next;
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
	  {if (!ASegP->Private)
             geSegKil(&ASegP);
           else
             {SSegP = (struct GE_SEG *)ASegP->Private;
              if (!SSegP->Next) geSegReplace(&ASegP, &SSegP);
             }
	  }
	break;

    case GEWRITE:
    case GEREAD:
	gePolIO(cmd, ASegP);
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

/*
 * The following is a layer routine for XPolygonRegion which checks the
 * vertex list for a minimal number of points (3) and uniqueness of at least
 * two of these.  It's required because XPolygonRegion blows up when handed
 * a degenerate polygon.
 */
Region
geXPolygonRegion(Vert, Vn, Rule)
XPoint	Vert[];
int	Vn, Rule;
{
int i, x, y;

if (Vn < 3) return(NULL);

x = Vert[0].x;
y = Vert[0].y;
i = 1;

while (i < Vn && (Vert[i].x == x || Vert[i].y == y)) i++;

if (i < Vn) return(XPolygonRegion(Vert, Vn, Rule));
else	    return(NULL);
}
