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
static char SccsId[] = "@(#)gebox.c	1.5\t10/27/89";
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
**	GEBOX	             	       Box object handler
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
#include <math.h>

extern unsigned long 	geAllocColor();
extern char          	*geMalloc();
extern Region		geXPolygonRegion();
extern double	        atof();	

geBox(cmd, ASegP)
long     	cmd;
struct GE_SEG	*ASegP;
{                       
#define         GEOBJLIS 5

char            *error_string, *string, buff[15];
short		EditPtFlip;
int		Hlw, PixLinW, temp, XSav, YSav, ix, iy, NumFrames, RotSavC;
long            x, y, dx, dy, ds, wid, hgt;
float           fx, fy, fdx, fdy, fx1, fy1, fx2, fy2, fx3, fy3, fx4, fy4,
		m, m1, FList[GEOBJLIS], FCon, ftemp, ftemp1, FOCon,
		FICon, RotSavA, RotSavB;
XPoint	        v[5];
Region		TRegion;

static float    FWprev = 0., FWcur = 0., FHprev = 0., FHcur = 0.;

struct GE_BOX   *priv, *privnu;
struct GE_PT    Pta;

if (ASegP)
  {if (!ASegP->Live && cmd != GEKILL    && cmd != GERELPRIV &&
                       cmd != GEKILPRIV && cmd != GEGETSEG_COL &&
		       cmd != GEXDEL)
     return;
   priv = (struct GE_BOX *)ASegP->Private;
  }

switch(cmd)
   {case GEDISP:
        if (priv && ASegP && ASegP->Visible && ASegP->InFrame &&
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
	    {if (ASegP->WhiteOut)
	       {GEVEC(GESETATTRERASE, ASegP);}

	     v[0].x = GETPIX(priv->Bx.x1); v[0].y = GETPIX(priv->Bx.y1);
	     v[1].x = GETPIX(priv->Bx.x2); v[1].y = GETPIX(priv->Bx.y2);
	     v[2].x = GETPIX(priv->Bx.x3); v[2].y = GETPIX(priv->Bx.y3);
	     v[3].x = GETPIX(priv->Bx.x4); v[3].y = GETPIX(priv->Bx.y4);
	     v[4]   = v[0];
	     /*
	      * Interior first
	      */
	     if (ASegP->FillStyle != GETRANSPARENT)
	       {GE_SET_BG_DISP_ATTR;
		GEFILSTYLE_FOS(geGC3, geDispAttr.BgStyle, geDispAttr.BgPixel);
		GE_FG_M(geGC3, geDispAttr.BgPixel, ASegP->FillWritingMode);
		GESTIPPLE(geGC3, geDispAttr.BgHT);
	       }
	     /*
	      * Outline next
	      */
	     if (priv->LineStyle != GETRANSPARENT)
	       {GE_SET_FG_DISP_ATTR;
	     
		if (priv->WritingMode == GXxor &&
		    priv->Col.RGB.pixel == geBlack.pixel)
		  {GEL_FG_P_M_PM(geGC2, GETPIX(priv->LinW), priv->Pat.LinP,
				 GEXORPIXEL, priv->WritingMode,
				 geXWPixel ^ geXBPixel);
		  }
		else 
		  {GEL_FG_P_M_PM(geGC2, GETPIX(priv->LinW), priv->Pat.LinP,
				 geDispAttr.FgPixel, priv->WritingMode,
				 AllPlanes);
		  }
		if (priv->Pat.LinP != LineSolid)
		  XSetDashes(geDispDev, geGC2, 0, priv->Pat.DashList,
			     priv->Pat.DashLen);
		GECAPSTYLE(geGC2, priv->CapStyle);
		GEJOINSTYLE(geGC2, priv->JoinStyle);
		if (geDispAttr.FgHT == 100)
		  {GEFILSTYLE(geGC2, FillSolid);}
		else
		  {GEFILSTYLE(geGC2, geDispAttr.FgStyle);
		   GESTIPPLE(geGC2, geDispAttr.FgHT);
		  }
	       }

	     if (v[0].x == v[1].x && v[0].x == v[2].x && v[0].x == v[3].x &&
		 v[0].y == v[1].y && v[0].y == v[2].y && v[0].y == v[3].y)
	       {XDrawPoint(geDispDev, geState.Drawable, geGC2,
			   v[0].x, v[0].y);
		continue;
	      }
	     if (ASegP->FillStyle != GETRANSPARENT)
	       XFillPolygon(geDispDev, geState.Drawable, geGC3, v, 5, Complex,
			    CoordModeOrigin);
	     if (priv->LineStyle != GETRANSPARENT)
	       XDrawLines(geDispDev, geState.Drawable, geGC2, v, 5,
			  CoordModeOrigin);
	     
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

#ifdef GERAGS

    case GEXDISP:
	if (geState.AnimDispMode != GEANIM_NONE && priv && ASegP &&
	    ASegP->Visible)
	 {if (geState.AnimDispMode == GEANIM_BOX)
	   {GEVEC(GEBOUNDS, ASegP);
	    XDrawRectangle(geDispDev, geState.Drawable, geGC5,
			   GETPIX(ASegP->Co.x1), GETPIX(ASegP->Co.y1),
			   GETPIX(ASegP->Co.x2 - ASegP->Co.x1),
			   GETPIX(ASegP->Co.y2 - ASegP->Co.y1));
	    break;
	   }
	  v[0].x = GETPIX(priv->Bx.x1); v[0].y = GETPIX(priv->Bx.y1);
	  v[1].x = GETPIX(priv->Bx.x2); v[1].y = GETPIX(priv->Bx.y2);
	  v[2].x = GETPIX(priv->Bx.x3); v[2].y = GETPIX(priv->Bx.y3);
	  v[3].x = GETPIX(priv->Bx.x4); v[3].y = GETPIX(priv->Bx.y4);
	  v[4]   = v[0];
	  if (geState.AnimDispMode == GEANIM_TRUE)
	    {GEL_FG_P_M(geGC2, GETPIX(priv->LinW), priv->Pat.LinP,
			GEXORPIXEL, GXxor);
	     if (priv->Pat.LinP != LineSolid)
	       XSetDashes(geDispDev, geGC2, 0, priv->Pat.DashList,
			  priv->Pat.DashLen);
	     GECAPSTYLE(geGC2, priv->CapStyle);
	     GEJOINSTYLE(geGC2, priv->JoinStyle);
	     if (priv->LineHT == 100)
	       {GEFILSTYLE(geGC2, FillSolid);}
             else
               {GEFILSTYLE(geGC2, priv->LineStyle);
	        GESTIPPLE(geGC2, priv->LineHT);
	       } 
	     if (v[0].x == v[1].x && v[0].x == v[2].x && v[0].x == v[3].x &&
		 v[0].y == v[1].y && v[0].y == v[2].y && v[0].y == v[3].y)
	       {XDrawPoint(geDispDev, geState.Drawable, geGC2, v[0].x, v[0].y);
		break;
	       }
	     XDrawLines(geDispDev, geState.Drawable, geGC2, v, 5,
			CoordModeOrigin);
	    }
	  else
	    {if (v[0].x == v[1].x && v[0].x == v[2].x && v[0].x == v[3].x &&
		 v[0].y == v[1].y && v[0].y == v[2].y && v[0].y == v[3].y)
	       {XDrawPoint(geDispDev, geState.Drawable, geGC5, v[0].x, v[0].y);
		break;
	       }
	     XDrawLines(geDispDev, geState.Drawable, geGC5, v, 5,
			CoordModeOrigin);
	    }
	  if (geAnimMin && (abs(priv->Bx.x2 - priv->Bx.x1) >
			    (geAnimMin >> 1) ||
			    abs(priv->Bx.y4 - priv->Bx.y1) >
			    (geAnimMin >> 1)))
	   {XDrawLine(geDispDev, geState.Drawable, geGC5,
		      GETPIX(priv->Bx.x1 + priv->Bx.x4) >> 1,
		      GETPIX(priv->Bx.y1 + priv->Bx.y4) >> 1,
		      GETPIX(priv->Bx.x2 + priv->Bx.x3) >> 1,
		      GETPIX(priv->Bx.y2 + priv->Bx.y3) >> 1);
	    XDrawLine(geDispDev, geState.Drawable, geGC5,
		      GETPIX(priv->Bx.x1 + priv->Bx.x2) >> 1,
		      GETPIX(priv->Bx.y1 + priv->Bx.y2) >> 1,
		      GETPIX(priv->Bx.x3 + priv->Bx.x4) >> 1,
		      GETPIX(priv->Bx.y3 + priv->Bx.y4) >> 1);
	   }
	 }
	break;

    case GEHDISP:
    case GEHXDISP:
        if (priv && ASegP && ASegP->Visible && ASegP->InFrame)
	   {if (geState.EditPts)
		{if (priv->EditPt1) geGenBoxEP(cmd, priv->Bx.x1, priv->Bx.y1);
		 if (priv->EditPt2) geGenBoxEP(cmd, priv->Bx.x2, priv->Bx.y2);
		 if (priv->EditPt3) geGenBoxEP(cmd, priv->Bx.x3, priv->Bx.y3);
		 if (priv->EditPt4) geGenBoxEP(cmd, priv->Bx.x4, priv->Bx.y4);
		 break;
		}

	    if (geState.HilightMode == GEHILIGHT_BOX)
		{GEVEC(GEEXTENT, ASegP);                /* EXTENT RECTANGLE          */
		 XDrawRectangle(geDispDev, geState.Drawable, geGC5,
                        	GETPIX(geClip.x1), GETPIX(geClip.y1),
                        	GETPIX(geClip.x2 - geClip.x1),
				GETPIX(geClip.y2 - geClip.y1));
		}
	    else
	 	{v[0].x = GETPIX(priv->Bx.x1); v[0].y = GETPIX(priv->Bx.y1);
	  	 v[1].x = GETPIX(priv->Bx.x2); v[1].y = GETPIX(priv->Bx.y2);
	 	 v[2].x = GETPIX(priv->Bx.x3); v[2].y = GETPIX(priv->Bx.y3);
	 	 v[3].x = GETPIX(priv->Bx.x4); v[3].y = GETPIX(priv->Bx.y4);
	 	 v[4]   = v[0];

	 	 PixLinW = GETPIX(priv->LinW);
	 	 GEL_FG_P_M_PM(geGC2, PixLinW, priv->Pat.LinP,
			       GEXORPIXEL, GXxor, geXWPixel ^ geXBPixel);
	  	 if (priv->Pat.LinP != LineSolid)
		    XSetDashes(geDispDev, geGC2, 0, priv->Pat.DashList,
			       priv->Pat.DashLen);
	 	 GECAPSTYLE(geGC2, priv->CapStyle);
	 	 GEJOINSTYLE(geGC2, priv->JoinStyle);
	 	 if (priv->LineHT == 100)
	    	    {GEFILSTYLE(geGC2, FillSolid);}
	  	 else
	     	    {GEFILSTYLE(geGC2, priv->LineStyle);}
	 	 GESTIPPLE(geGC2, 0);
	 	 if (v[0].x == v[1].x && v[0].x == v[2].x && v[0].x == v[3].x &&
	             v[0].y == v[1].y && v[0].y == v[2].y && v[0].y == v[3].y)
	   	    {XDrawPoint(geDispDev, geState.Drawable, geGC2, v[0].x, v[0].y);
	             break;
	    	    }
	 	 XDrawLines(geDispDev, geState.Drawable, geGC2, v, 5, CoordModeOrigin);
	 	}
	   }
	break;

    case GECREATE:
	geSegCr();
	geState.ASegP->Private    = geMalloc(sizeof(struct GE_BOX));
	priv 		          = (struct GE_BOX *)geState.ASegP->Private;
	geState.ASegP->Handle[0]  = 'B';
	geState.ASegP->Handle[1]  = 'O';
	geState.ASegP->Handle[2]  = 'X';
	geState.ASegP->Handle[3]  = '\0';
	geState.ASegP->Handler    = geBox;
	ASegP                     = geState.ASegP;

	if(geState.Grav.Align || geState.APagP->Grid.XAlign ||
	   geState.APagP->Grid.YAlign)
	  {geState.Dx = geState.Dy   = 0;
	   geState.Grav.OPt.x = geState.Mouse.x;
	   geState.Grav.OPt.y = geState.Mouse.y;
	   geGenAln(geState.Mouse.x, geState.Mouse.y);
	   geState.Grav.OPt.x = geState.Mouse.x += geState.Dx;
	   geState.Grav.OPt.y = geState.Mouse.y += geState.Dy;
	  }
	priv->Bx.x1               = geU.XR = ASegP->Co.x1 = geState.Mouse.x;
	priv->Bx.y1               = geU.YR = ASegP->Co.y1 = geState.Mouse.y;
	geMouseXY(geState.Window);
/*
geState.Mouse.x += 50;
geState.Mouse.y += 50;
*/
	priv->Bx.x3               = ASegP->Co.x2 = geState.Mouse.x;
	priv->Bx.y3               = ASegP->Co.y2 = geState.Mouse.y;
	priv->Bx.x2               = priv->Bx.x3;
	priv->Bx.y2               = priv->Bx.y1;
	priv->Bx.x4               = priv->Bx.x1;
	priv->Bx.y4               = priv->Bx.y3;

	priv->LinW             	  = geAttr.LinW;
	if (geState.ZoomLineThick)
	      priv->LinW = ((priv->LinW ? priv->LinW : GESUI1) *
			     geState.APagP->Zoom.ZoomF) /100;
	if (geState.WhiteOut)
	   {GEVEC(GESETATTRERASE, geState.ASegP);}
	else
	   {priv->Col          	  = geAttr.LinCol;
	    priv->Pat.LinP        = geAttr.LinP;
	    priv->Pat.DashList[0] = geDashList[geAttr.DashIndx][0];
	    priv->Pat.DashList[1] = geDashList[geAttr.DashIndx][1];
	    priv->Pat.DashList[2] = geDashList[geAttr.DashIndx][2];
	    priv->Pat.DashList[3] = geDashList[geAttr.DashIndx][3];
	    priv->Pat.DashLen	  = geAttr.DashLen;
	    priv->Pat.DashIndx	  = geAttr.DashIndx;
            priv->LineHT          = geAttr.LineHT;
            priv->LineStyle       = geAttr.LineStyle;
	    priv->WritingMode	  = geAttr.WritingMode;
	   }
	priv->CapStyle		  = CapButt;
	priv->JoinStyle		  = geAttr.JoinStyle;
	priv->EditPt1 = priv->EditPt2 = priv->EditPt3 = priv->EditPt4 = FALSE;
	geMnStat(GEDISP, 56);
/* creation at an angle test
geRot.Clockwise = FALSE;
GEVEC(GEROTX, ASegP);
*/
	geAnim  (cmd,    ASegP);
	if (!geState.ObjEdCr)
	  {/*
	    * This has to be "geState.ASegP" rather than "ASegP" because the
	    * obj may have been cancelled in Anim.
	    */
	    GEVEC(GEDISP, geState.ASegP);
	  }
	break;

    case GESETATTRERASE:
	if (ASegP && ASegP->Visible && ASegP->WhiteOut &&
	    (priv = (struct GE_BOX *)ASegP->Private))
	  {GE_SET_ATTR_ERASE;
	  }
	break;

    case GESETATTRERASE_WHITE:
	if (ASegP && ASegP->Visible && ASegP->WhiteOut && !geState.PrintPgBg &&
	    (priv = (struct GE_BOX *)ASegP->Private))
	  {GE_SET_ATTR_ERASE_WHITE;
	  }
	break;

    case GEOBJCREATE:
	if (!ASegP || !ASegP->Visible) break;

	if (geU.DMap != -1) FCon = geU.Con[geU.DMap];
	else                FCon = 1.0;
	FOCon       = FCon / GEFSUIPP;
	FICon       = GEFSUIPP / FCon;

	/* Set up default values for rectangle object and fetch the create
	 * to size modal box.  Only use default values if in create 
	 * mode, otherwise use the current values (editing).
	 */
 	if (geState.InCreateMode)
	  {priv->Bx.x1 = geState.Mouse.x;
	   priv->Bx.y1 = geState.Mouse.y;
	   FWprev = wid = GESUI1 * 20;
	   priv->Bx.x3 = priv->Bx.x2 = priv->Bx.x1 + wid;	
	   priv->Bx.x4 = priv->Bx.x1;
	   FList[1]    = (float)(wid * FOCon);
           FHprev = hgt = GESUI1 * 10;
           priv->Bx.y2 = priv->Bx.y1;
	   priv->Bx.y4 = priv->Bx.y3 = priv->Bx.y1 + hgt;
	   FList[2]    = (float)(hgt * FOCon);
           FList[3]    = 0.;
           FList[4]    = (float)priv->LinW  * FOCon;
          }      
	else
	  {fdx        = (float)(priv->Bx.x2 - priv->Bx.x1);
	   fdy        = (float)(priv->Bx.y2 - priv->Bx.y1);
	   FList[1]   = (float)sqrt((double)(fdx * fdx) +
					  (double)(fdy * fdy)) * FOCon;
	   fdx        = (float)(priv->Bx.x4 - priv->Bx.x1);
	   fdy        = (float)(priv->Bx.y4 - priv->Bx.y1);
	   FList[2]   = (float)sqrt((double)(fdx * fdx) +
					  (double)(fdy * fdy)) * FOCon;
	   FList[3]   = 0.;
	   FList[4]   = (float)priv->LinW  * FOCon;
	  }

	if (!geUI.WidgetArray[GEWCREATE_REC] || 
	    !XtIsManaged(geUI.WidgetArray[GEWCREATE_REC]))
	  {if (MrmFetchWidget(geUI.Id, GECREATE_REC_POPUP,
			      XtParent(geState.APagP->WidgetId),
			      &geUI.WidgetArray[GEWCREATE_REC], 
			      &geUI.Class) != MrmSUCCESS)
	     {error_string = (char *) geFetchLiteral("GE_ERR_NOWIDGET", MrmRtypeChar8);
	      if (error_string != NULL) 
	        {sprintf(geErrBuf, error_string, GECREATE_REC_POPUP);
	         geError(geErrBuf, TRUE);
	         XtFree(error_string);
	        }
	     }
	  }
        /* Set all the fields before managing box. 
 	 */
	sprintf(buff, "%.3f", FList[1]);		/* Width */
 	XmTextSetString(geUI.WidgetArray[GEWCREATE_WIDTH], buff);
	sprintf(buff, "%.3f", FList[2]);		/* Height */
 	XmTextSetString(geUI.WidgetArray[GEWCREATE_HEIGHT], buff);
	sprintf(buff, "%.3f", FList[3]);		/* DegRot */
 	XmTextSetString(geUI.WidgetArray[GEWCREATE_DEGROT], buff);
	sprintf(buff, "%.3f", FList[4]);		/* LinW */
 	XmTextSetString(geUI.WidgetArray[GEWCREATE_LINW], buff);

	XtManageChild(geUI.WidgetArray[GEWCREATE_REC]);

	if (geState.InCreateMode)
  	  { GEVEC(GEXDISP, ASegP); } 			/* XOR the object */
	break;

    case GEOBJEDIT:
    case GEEDIT:
	if (!ASegP || !ASegP->Visible) break;

	if (!geUI.WidgetArray[GEWCREATE_REC])
	  { GEVEC(GEOBJCREATE, ASegP);
	    break;
	  }

	if (geU.DMap != -1) FCon = geU.Con[geU.DMap];
	else               FCon = 1.0;
	FOCon      = FCon / GEFSUIPP;
	FICon      = GEFSUIPP / FCon;

	/* Get all the fields before displaying the new object.
 	 */  						  /* Width */
 	string = XmTextGetString(geUI.WidgetArray[GEWCREATE_WIDTH]);
	FWcur  = (float)atof(string) * FICon;
	FWprev = (float)sqrt((double)(priv->Bx.x2 - priv->Bx.x1) *
			     (double)(priv->Bx.x2 - priv->Bx.x1) +
			     (double)(priv->Bx.y2 - priv->Bx.y1) *
			     (double)(priv->Bx.y2 - priv->Bx.y1));

	if (FWprev == 0.0) FWprev = 1.0;
	if (FWprev != FWcur)
	  {/*
	    * Adjust x2
	    */
	    ftemp = FWcur * (float)fabs((double)(priv->Bx.x2 - priv->Bx.x1)) /
	      FWprev;
	    if (priv->Bx.x2 >= priv->Bx.x1)
	      ftemp1 = (double)priv->Bx.x1 + ftemp;
	    else
	      ftemp1 = (double)priv->Bx.x1 - ftemp;
	    GEINT(ftemp1, priv->Bx.x2);
	   /*
	    * Adjust x3
	    */
	    ftemp = FWcur * (float)fabs((double)(priv->Bx.x3 - priv->Bx.x4)) /
	      FWprev;
	    if (priv->Bx.x3 >= priv->Bx.x4)
	      ftemp1 = (double)priv->Bx.x4 + ftemp;
	    else
	      ftemp1 = (double)priv->Bx.x4 - ftemp;
	    GEINT(ftemp1, priv->Bx.x3);
	    /*
	     * Adjust y2
	     */
	    ftemp = FWcur * (float)fabs((double)(priv->Bx.y2 - priv->Bx.y1)) /
	      FWprev;
	    if (priv->Bx.y2 >= priv->Bx.y1)
	      ftemp1 = (double)priv->Bx.y1 + ftemp;
	    else
	      ftemp1 = (double)priv->Bx.y1 - ftemp;
	    GEINT(ftemp1, priv->Bx.y2);
	    /*
	     * Adjust y3
	     */
	    ftemp = FWcur * (float)fabs((double)(priv->Bx.y3 - priv->Bx.y4)) /
	      FWprev;
	    if (priv->Bx.y3 >= priv->Bx.y4)
	      ftemp1 = (double)priv->Bx.y4 + ftemp;
	    else
	      ftemp1 = (double)priv->Bx.y4 - ftemp;
	    GEINT(ftemp1, priv->Bx.y3);
	  }
	  
	XtFree(string);
							/* Height */
 	string = XmTextGetString(geUI.WidgetArray[GEWCREATE_HEIGHT]);
	FHcur  = (float)atof(string) * FICon;
	FHprev = (float)sqrt((double)(priv->Bx.x4 - priv->Bx.x1) *
			     (double)(priv->Bx.x4 - priv->Bx.x1) +
			     (double)(priv->Bx.y4 - priv->Bx.y1) *
			     (double)(priv->Bx.y4 - priv->Bx.y1));

	if (FHprev == 0.0) FHprev = 1.0;
	if (FHprev != FHcur)
	  {/*
	    * Adjust x4
	    */
	    ftemp = FHcur * (float)fabs((double)(priv->Bx.x4 - priv->Bx.x1)) /
	      FHprev;
	    if (priv->Bx.x4 >= priv->Bx.x1)
	      ftemp1 = (double)priv->Bx.x1 + ftemp;
	    else
	      ftemp1 = (double)priv->Bx.x1 - ftemp;
	    GEINT(ftemp1, priv->Bx.x4);
	   /*
	    * Adjust x3
	    */
	    ftemp = FHcur * (float)fabs((double)(priv->Bx.x3 - priv->Bx.x2)) /
	      FHprev;
	    if (priv->Bx.x3 >= priv->Bx.x2)
	      ftemp1 = (double)priv->Bx.x2 + ftemp;
	    else
	      ftemp1 = (double)priv->Bx.x2 - ftemp;
	    GEINT(ftemp1, priv->Bx.x3);
	    /*
	     * Adjust y4
	     */
	    ftemp = FHcur * (float)fabs((double)(priv->Bx.y4 - priv->Bx.y1)) /
	      FHprev;
	    if (priv->Bx.y4 >= priv->Bx.y1)
	      ftemp1 = (double)priv->Bx.y1 + ftemp;
	    else
	      ftemp1 = (double)priv->Bx.y1 - ftemp;
	    GEINT(ftemp1, priv->Bx.y4);
	    /*
	     * Adjust y3
	     */
	    ftemp = FHcur * (float)fabs((double)(priv->Bx.y3 - priv->Bx.y2)) /
	      FHprev;
	    if (priv->Bx.y3 >= priv->Bx.y2)
	      ftemp1 = (double)priv->Bx.y2 + ftemp;
	    else
	      ftemp1 = (double)priv->Bx.y2 - ftemp;
	    GEINT(ftemp1, priv->Bx.y3);
	  }
	  
	XtFree(string);
							 /* DegRot */
	if (XmToggleButtonGetState(geUI.WidgetArray[GEWCREATE_ROTATE]))
 	  {string = XmTextGetString(geUI.WidgetArray[GEWCREATE_DEGROT]);
	   if (ftemp = (float)atof(string))
	     {/*
	       * Rotate the object about its center
	       */
	       geEditBox.Pt.x  = (priv->Bx.x1 + priv->Bx.x3) >> 1;
	       geEditBox.Pt.y  = (priv->Bx.y1 + priv->Bx.y3) >> 1;
	       geGenRotObj(ASegP, ftemp);
	     }
	   XtFree(string);
	  }
  	                                                /* LinW */
 	string = XmTextGetString(geUI.WidgetArray[GEWCREATE_LINW]);
	ftemp = (float)atof(string);
	if (!geState.LinWPoints && geU.DMap >= 0)
	  ftemp /= geU.Con[geU.DMap];      
	ftemp *= GEFSUIPP;
	GEINT(ftemp, priv->LinW); 
	XtFree(string);
	break;

    case GECOPY:
    case GECOPYFLP:
	if (!ASegP || !ASegP->Visible) break;
	geSegCr();
	geState.ASegP->Private    = geMalloc(sizeof(struct GE_BOX));
	privnu 		          = (struct GE_BOX *)geState.ASegP->Private;
	*privnu                        = *priv;
	geGenCopySeg(geState.ASegP, ASegP);
 	GEVEC(GEBOUNDS, geState.ASegP);
	break;

    case GEWARP2:
        if (ASegP && ASegP->Visible && priv)
	  {/*
	    * If it's orthogonal, then it's a bit simpler to figure out
	    */
	   if (priv->Bx.x1 == priv->Bx.x2 || priv->Bx.x1 == priv->Bx.x3 ||
	       priv->Bx.x1 == priv->Bx.x4)
	     {priv->Bx.x2 = priv->Bx.x3 = geState.Mouse.x;
	      priv->Bx.y3 = priv->Bx.y4 = geState.Mouse.y;
	     }
	   else
	     {fx1 = (float)priv->Bx.x1; fy1 = (float)priv->Bx.y1;
	      fx2 = (float)priv->Bx.x2; fy2 = (float)priv->Bx.y2;
	      fx3 = (float)priv->Bx.x3; fy3 = (float)priv->Bx.y3;
	      fx4 = (float)priv->Bx.x4; fy4 = (float)priv->Bx.y4;
	      m   = (fy2 - fy1) / (fx2 - fx1);
	      m1  = -1.0 / m;
	      fx  = (m * fx1 - fy1 - m1 * (float)geState.Mouse.x +
		     (float)geState.Mouse.y) / (m - m1);
	      fy  = m * fx - m * fx1 + fy1;
	      GEINT(fx, priv->Bx.x2);
	      GEINT(fy, priv->Bx.y2);
	      m   = (fy4 - fy1) / (fx4 - fx1);
	      m1  = -1.0 / m;
	      fx  = (m * fx1 - fy1 - m1 * (float)geState.Mouse.x +
		     (float)geState.Mouse.y) / (m - m1);
	      fy  = m * fx - m * fx1 + fy1;
	      GEINT(fx, priv->Bx.x4);
	      GEINT(fy, priv->Bx.y4);
	      priv->Bx.x3 = geState.Mouse.x;
	      priv->Bx.y3 = geState.Mouse.y;
	     }
	  }
	break;

#endif
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
        if ((cmd == GEGRAVTST || cmd == GEMOVE || cmd == GESCALE) &&
	    (!ASegP || !ASegP->Visible)) break;

        if (priv)
	 {if(geState.Grav.Align || geState.APagP->Grid.XAlign ||
	     geState.APagP->Grid.YAlign)
	  {geState.Grav.Lock = FALSE;
	   if (!geState.EditPts || priv->EditPt1)
	     geGenAln(priv->Bx.x1, priv->Bx.y1);
	   if (!geState.Grav.Lock)
	     {if (!geState.EditPts || priv->EditPt2)
		geGenAln(priv->Bx.x2, priv->Bx.y2);
	      if (!geState.Grav.Lock)
		{if (!geState.EditPts || priv->EditPt3)
		   geGenAln(priv->Bx.x3, priv->Bx.y3);
		 if (!geState.Grav.Lock)
		   {if (!geState.EditPts || priv->EditPt4)
		      geGenAln(priv->Bx.x4, priv->Bx.y4);
		    if (!geState.EditPts && !geState.Grav.Lock)
		       {geGenAln((priv->Bx.x3 + priv->Bx.x1) >> 1,
				  (priv->Bx.y3 + priv->Bx.y1) >> 1);
			if (!geState.Grav.Lock)
			  {geGenAln((priv->Bx.x2 + priv->Bx.x1) >> 1,
				     (priv->Bx.y2 + priv->Bx.y1) >> 1);
			   if (!geState.Grav.Lock)
			     {geGenAln((priv->Bx.x3 + priv->Bx.x2) >> 1,
					(priv->Bx.y3 + priv->Bx.y2) >> 1);
			      if (!geState.Grav.Lock)
				{geGenAln((priv->Bx.x4 + priv->Bx.x3) >> 1,
					   (priv->Bx.y4 + priv->Bx.y3) >> 1);
				 if (!geState.Grav.Lock)
				   geGenAln((priv->Bx.x4 + priv->Bx.x1) >> 1,
					     (priv->Bx.y4 + priv->Bx.y1) >> 1);
			        }
			     }
			  }
		       }
		   } 
	        }
	     }
	  }
	   if (cmd == GEMOVE || cmd == GESCRL || cmd == GESCRR ||
	       cmd == GESCRU || cmd == GESCRD && (geState.Dx || geState.Dy))
	     {if (!geState.EditPts || priv->EditPt1)
		{priv->Bx.x1 += geState.Dx;
		 priv->Bx.y1 += geState.Dy;
	        }
	      if (!geState.EditPts || priv->EditPt2)
		{priv->Bx.x2 += geState.Dx;
		 priv->Bx.y2 += geState.Dy;
	        }
	      if (!geState.EditPts || priv->EditPt3)
		{priv->Bx.x3 += geState.Dx;
		 priv->Bx.y3 += geState.Dy;
	        }
	      if (!geState.EditPts || priv->EditPt4)
		{priv->Bx.x4 += geState.Dx;
		 priv->Bx.y4 += geState.Dy;
	        }
	     }
	  else
	  if (cmd == GESCALE)
	    {if (!geState.EditPts || (priv->EditPt1 && priv->EditPt2 &&
				      priv->EditPt3 && priv->EditPt4))
	       geGenScaleBx(&priv->Bx);
	     else
	       {if (priv->EditPt1)
		  {Pta.x       = priv->Bx.x1;
		   Pta.y       = priv->Bx.y1;
		   geGenScalePt(&Pta);
		   priv->Bx.x1 = Pta.x;
		   priv->Bx.y1 = Pta.y;
	          }
	        else if (priv->EditPt2)
		  {Pta.x       = priv->Bx.x2;
		   Pta.y       = priv->Bx.y2;
		   geGenScalePt(&Pta);
		   priv->Bx.x2 = Pta.x;
		   priv->Bx.y2 = Pta.y;
	          }
	        else if (priv->EditPt3)
		  {Pta.x       = priv->Bx.x3;
		   Pta.y       = priv->Bx.y3;
		   geGenScalePt(&Pta);
		   priv->Bx.x3 = Pta.x;
		   priv->Bx.y3 = Pta.y;
	          }
	        else if (priv->EditPt4)
		  {Pta.x       = priv->Bx.x4;
		   Pta.y       = priv->Bx.y4;
		   geGenScalePt(&Pta);
		   priv->Bx.x4 = Pta.x;
		   priv->Bx.y4 = Pta.y;
	          }
	       }
	    } 
	 }
	break;

#ifdef GERAGS

    case GETESTPT:
        if (!geState.EditPts || !ASegP || !ASegP->Visible) break;

	EditPtFlip = FALSE;
	if GESELTST(priv->Bx.x1, priv->Bx.y1)
	  {priv->EditPt1 = priv->EditPt1 ? FALSE : TRUE;
	   EditPtFlip = TRUE;
	  }
	if GESELTST(priv->Bx.x2, priv->Bx.y2)
	  {priv->EditPt2 = priv->EditPt2 ? FALSE : TRUE;
	   EditPtFlip = TRUE;
	  }
	if GESELTST(priv->Bx.x3, priv->Bx.y3)
	  {priv->EditPt3 = priv->EditPt3 ? FALSE : TRUE;
	   EditPtFlip = TRUE;
	  }
	if GESELTST(priv->Bx.x4, priv->Bx.y4)
	  {priv->EditPt4 = priv->EditPt4 ? FALSE : TRUE;
	   EditPtFlip = TRUE;
	  }

	if (!EditPtFlip)
	  {priv->EditPt1 = priv->EditPt1 ? FALSE : TRUE;
	   priv->EditPt2 = priv->EditPt2 ? FALSE : TRUE;
	   priv->EditPt3 = priv->EditPt3 ? FALSE : TRUE;
	   priv->EditPt4 = priv->EditPt4 ? FALSE : TRUE;
	  }

	break;

    case GETESTPTCLR:
        if (!ASegP || !ASegP->Visible) break;

	priv->EditPt1 = FALSE;
	priv->EditPt2 = FALSE;
	priv->EditPt3 = FALSE;
	priv->EditPt4 = FALSE;
	break;

    case GEGRAV:
        if (ASegP && ASegP->Visible && priv &&
	    (geState.Grav.Pt.x >= (ASegP->Co.x1 - geState.Grav.Rad) &&
	     geState.Grav.Pt.y >= (ASegP->Co.y1 - geState.Grav.Rad) &&
	     geState.Grav.Pt.x <= (ASegP->Co.x2 + geState.Grav.Rad) &&
	     geState.Grav.Pt.y <= (ASegP->Co.y2 + geState.Grav.Rad)))
	  {geGenGrav(GEGRAVLOCK, priv->Bx.x1, priv->Bx.y1);
	   geGenGrav(GEGRAVLOCK, priv->Bx.x2, priv->Bx.y2);
	   geGenGrav(GEGRAVLOCK, priv->Bx.x3, priv->Bx.y3);
	   geGenGrav(GEGRAVLOCK, priv->Bx.x4, priv->Bx.y4);
	   geGenGrav(GEGRAVLOCK, ((priv->Bx.x3 + priv->Bx.x1) >> 1),
		     ((priv->Bx.y3 + priv->Bx.y1) >> 1));
	   geGenGrav(GEGRAVLOCK, ((priv->Bx.x2 + priv->Bx.x1) >> 1),
		     ((priv->Bx.y2 + priv->Bx.y1) >> 1));
	   geGenGrav(GEGRAVLOCK, ((priv->Bx.x3 + priv->Bx.x2) >> 1),
		     ((priv->Bx.y3 + priv->Bx.y2) >> 1));
	   geGenGrav(GEGRAVLOCK, ((priv->Bx.x3 + priv->Bx.x4) >> 1),
		     ((priv->Bx.y3 + priv->Bx.y4) >> 1));
	   geGenGrav(GEGRAVLOCK, ((priv->Bx.x4 + priv->Bx.x1) >> 1),
		     ((priv->Bx.y4 + priv->Bx.y1) >> 1));
	  }
	break;

    case GECONTROL:
        if (ASegP && ASegP->Visible)
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
        if (ASegP && ASegP->Visible && priv)
	  {struct GE_BX t;
	   /*
	    * If it's orthogonal, then it's a bit simpler to figure out
	    */
	   if (priv->Bx.x1 == priv->Bx.x2 || priv->Bx.x1 == priv->Bx.x3 ||
	       priv->Bx.x1 == priv->Bx.x4)
	     {t.x1         = min(min(min(priv->Bx.x1,  priv->Bx.x2),
				         priv->Bx.x3), priv->Bx.x4);
	      t.y1         = min(min(min(priv->Bx.y1,  priv->Bx.y2),
				         priv->Bx.y3), priv->Bx.y4);
	      t.x3         = max(max(max(priv->Bx.x1,  priv->Bx.x2),
				         priv->Bx.x3), priv->Bx.x4);
	      t.y3         = max(max(max(priv->Bx.y1,  priv->Bx.y2),
				         priv->Bx.y3), priv->Bx.y4);
	      t.x2 = t.x3; t.y2 = t.y1; t.x4 = t.x1; t.y4 = t.y3;
	     }
	   else
	     {t.x1         = min(min(min(priv->Bx.x1,  priv->Bx.x2),
				         priv->Bx.x3), priv->Bx.x4);
	      t.y1         = (t.x1 == priv->Bx.x1 ? priv->Bx.y1 :
			      t.x1 == priv->Bx.x2 ? priv->Bx.y2 :
			      t.x1 == priv->Bx.x3 ? priv->Bx.y3 : priv->Bx.y4);
	      t.y2         = min(min(min(priv->Bx.y1,  priv->Bx.y2),
				         priv->Bx.y3), priv->Bx.y4);
	      t.x2         = (t.y2 == priv->Bx.y1 ? priv->Bx.x1 :
			      t.y2 == priv->Bx.y2 ? priv->Bx.x2 :
			      t.y2 == priv->Bx.y3 ? priv->Bx.x3 : priv->Bx.x4);
	      t.x3         = max(max(max(priv->Bx.x1,  priv->Bx.x2),
				         priv->Bx.x3), priv->Bx.x4);
	      t.y3         = (t.x3 == priv->Bx.x1 ? priv->Bx.y1 :
			      t.x3 == priv->Bx.x2 ? priv->Bx.y2 :
			      t.x3 == priv->Bx.x3 ? priv->Bx.y3 : priv->Bx.y4);
	      t.y4         = max(max(max(priv->Bx.y1,  priv->Bx.y2),
				         priv->Bx.y3), priv->Bx.y4);
	      t.x4         = (t.y4 == priv->Bx.y1 ? priv->Bx.x1 :
			      t.y4 == priv->Bx.y2 ? priv->Bx.x2 :
			      t.y4 == priv->Bx.y3 ? priv->Bx.x3 : priv->Bx.x4);
	     }

	   geClip.x1 = t.x1;
	   geClip.y1 = t.y2;
	   geClip.x2 = t.x3;
	   geClip.y2 = t.y4;
	  }
	break;

#endif

    case GEBOUNDS:
        if (ASegP && ASegP->Visible && priv)
	  {struct GE_BX t;
	   /*
	    * If it's orthogonal, then it's a bit simpler to figure out
	    */
	   if (priv->Bx.x1 == priv->Bx.x2 || priv->Bx.x1 == priv->Bx.x3 ||
	       priv->Bx.x1 == priv->Bx.x4)
	     {t.x1         = min(min(min(priv->Bx.x1,  priv->Bx.x2),
				         priv->Bx.x3), priv->Bx.x4);
	      t.y1         = min(min(min(priv->Bx.y1,  priv->Bx.y2),
				         priv->Bx.y3), priv->Bx.y4);
	      t.x3         = max(max(max(priv->Bx.x1,  priv->Bx.x2),
				         priv->Bx.x3), priv->Bx.x4);
	      t.y3         = max(max(max(priv->Bx.y1,  priv->Bx.y2),
				         priv->Bx.y3), priv->Bx.y4);
	      t.x2 = t.x3; t.y2 = t.y1; t.x4 = t.x1; t.y4 = t.y3;
	     }
	   else
	     {t.x1         = min(min(min(priv->Bx.x1,  priv->Bx.x2),
				         priv->Bx.x3), priv->Bx.x4);
	      t.y1         = (t.x1 == priv->Bx.x1 ? priv->Bx.y1 :
			      t.x1 == priv->Bx.x2 ? priv->Bx.y2 :
			      t.x1 == priv->Bx.x3 ? priv->Bx.y3 : priv->Bx.y4);
	      t.y2         = min(min(min(priv->Bx.y1,  priv->Bx.y2),
				         priv->Bx.y3), priv->Bx.y4);
	      t.x2         = (t.y2 == priv->Bx.y1 ? priv->Bx.x1 :
			      t.y2 == priv->Bx.y2 ? priv->Bx.x2 :
			      t.y2 == priv->Bx.y3 ? priv->Bx.x3 : priv->Bx.x4);
	      t.x3         = max(max(max(priv->Bx.x1,  priv->Bx.x2),
				         priv->Bx.x3), priv->Bx.x4);
	      t.y3         = (t.x3 == priv->Bx.x1 ? priv->Bx.y1 :
			      t.x3 == priv->Bx.x2 ? priv->Bx.y2 :
			      t.x3 == priv->Bx.x3 ? priv->Bx.y3 : priv->Bx.y4);
	      t.y4         = max(max(max(priv->Bx.y1,  priv->Bx.y2),
				         priv->Bx.y3), priv->Bx.y4);
	      t.x4         = (t.y4 == priv->Bx.y1 ? priv->Bx.x1 :
			      t.y4 == priv->Bx.y2 ? priv->Bx.x2 :
			      t.y4 == priv->Bx.y3 ? priv->Bx.x3 : priv->Bx.x4);
	     }

	   ASegP->Co.x1 = t.x1 - (Hlw = priv->LinW >> 1);
	   ASegP->Co.y1 = t.y2 - Hlw;
	   if (Hlw)
	     {Hlw = (priv->LinW - GESUI1) >> 1;}
	   else	    Hlw = 0;
	   ASegP->Co.x2 = t.x3 + Hlw;
	   ASegP->Co.y2 = t.y4 + Hlw;
	   if (ASegP->Co.x2 < 0 || ASegP->Co.y2 < 0 ||
	       GETPIX(ASegP->Co.x1) > geState.APagP->Surf.width ||
	       GETPIX(ASegP->Co.y1) > geState.APagP->Surf.height)
	     ASegP->InFrame = FALSE;
	   else
	     ASegP->InFrame = TRUE;
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

    case GEROUND:
        if (ASegP && ASegP->Visible && priv)
          {priv->Bx.x1  = GETSUI(GETPIX(priv->Bx.x1));
           priv->Bx.y1  = GETSUI(GETPIX(priv->Bx.y1));
           priv->Bx.x2  = GETSUI(GETPIX(priv->Bx.x2));
           priv->Bx.y2  = GETSUI(GETPIX(priv->Bx.y2));
           priv->Bx.x3  = GETSUI(GETPIX(priv->Bx.x3));
           priv->Bx.y3  = GETSUI(GETPIX(priv->Bx.y3));
           priv->Bx.x4  = GETSUI(GETPIX(priv->Bx.x4));
           priv->Bx.y4  = GETSUI(GETPIX(priv->Bx.y4));
          }
        break;

    case GEALN:
	geGenAlign(ASegP);
	break;

    case GEALNREF:
        if (ASegP && ASegP->Visible)
 	   {GEVEC(GEEXTENT0, ASegP);
	    geAln.Co    = geClip;
	    geAln.Pt.x  = (geAln.Co.x1 + geAln.Co.x2) >> 1;
	    geAln.Pt.y  = (geAln.Co.y1 + geAln.Co.y2) >> 1;
	   }
	break;

    case GEALNCH:
    case GEALNCV:
	if (ASegP && ASegP->Visible && priv)
 	   {geState.Dx = geState.Dy = 0;
	    if (cmd == GEALNCV)
		geState.Dx = geAln.Pt.x - ((ASegP->Co.x1 + ASegP->Co.x2) >> 1);
	    if (cmd == GEALNCH)
		geState.Dy = geAln.Pt.y - ((ASegP->Co.y1 + ASegP->Co.y2) >> 1);
	   }
	break;

    case GESELPTINFILL:
	geSegSel = ASegP;
	if (ASegP && ASegP->Visible)
	  {if (ASegP->FillStyle == GETRANSPARENT ||
	       (ASegP->Co.x1 > geSel.x2 || ASegP->Co.y1 > geSel.y2 ||
	        ASegP->Co.x2 < geSel.x1 || ASegP->Co.y2 < geSel.y1)) break;

	   /*
	    * This must be a filled object, so if the selection point is INSIDE
	    * of it, then PICK it, set geState.SelRadC = 0.  The test for being
	    * INSIDE is conducted by requesting the object to establish its
	    * vertex list and then call XPointInRegion.
	    */
	   v[0].x = GETPIX(priv->Bx.x1); v[0].y = GETPIX(priv->Bx.y1);
	   v[1].x = GETPIX(priv->Bx.x2); v[1].y = GETPIX(priv->Bx.y2);
	   v[2].x = GETPIX(priv->Bx.x3); v[2].y = GETPIX(priv->Bx.y3);
	   v[3].x = GETPIX(priv->Bx.x4); v[3].y = GETPIX(priv->Bx.y4);
/*
	   v[4]   = v[0];
	   if ((TRegion = geXPolygonRegion(v, 5, EvenOddRule)))
*/
	   if ((TRegion = geXPolygonRegion(v, 4, EvenOddRule)))
		{x  = (geSel.x1 + geSel.x2) >> 1;
	  	 y  = (geSel.y1 + geSel.y2) >> 1;
	  	 ix = GETPIX(x);
	  	 iy = GETPIX(y);
	  	 if (XPointInRegion(TRegion, ix, iy))
	   	    {geState.SelRadC = 0;	/* This is IT		     */
		     GEVEC(GETESTPT, ASegP);
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
	geSegSel = ASegP;
	if (ASegP && ASegP->Visible)
	  {if (ASegP->Co.x1 > geSel.x2 || ASegP->Co.y1 > geSel.y2 ||
	       ASegP->Co.x2 < geSel.x1 || ASegP->Co.y2 < geSel.y1) break;

	   if (geGenPtSelBox(priv))
	   	 {GEVEC(GETESTPT, ASegP);}
	  }	
	break;

    case GESELPTIN:
	geSegSel = ASegP;
	if (ASegP && ASegP->Visible)
	  {if (ASegP->Co.x1 > geSel.x2 || ASegP->Co.y1 > geSel.y2 ||
	       ASegP->Co.x2 < geSel.x1 || ASegP->Co.y2 < geSel.y1) break;

	   /*
	    * If the selection point is INSIDE of this object, then
	    * compute the proximity - the closest object will be
	    * PICKED.
	    */
	   v[0].x = GETPIX(priv->Bx.x1); v[0].y = GETPIX(priv->Bx.y1);
	   v[1].x = GETPIX(priv->Bx.x2); v[1].y = GETPIX(priv->Bx.y2);
	   v[2].x = GETPIX(priv->Bx.x3); v[2].y = GETPIX(priv->Bx.y3);
	   v[3].x = GETPIX(priv->Bx.x4); v[3].y = GETPIX(priv->Bx.y4);
	   v[4]   = v[0];
	   if ((TRegion = geXPolygonRegion(v, 5, EvenOddRule)))
	   	{x  = (geSel.x1 + geSel.x2) >> 1;
	  	 y  = (geSel.y1 + geSel.y2) >> 1;
	  	 ix = GETPIX(x);
	  	 iy = GETPIX(y);
	  	 if (XPointInRegion(TRegion, ix, iy))
	       	    {geGenPtSelBox(priv);
		     GEVEC(GETESTPT, ASegP);
		    }
	  	 else
		     geState.SelRadC = GEMAXSHORT;
	  	 XDestroyRegion(TRegion);
		}
	   else
		 geState.SelRadC = GEMAXSHORT;
	  }
	
	break;

    case GESELBX:
	if (ASegP && ASegP->Visible)
	  {if (geState.EditPts)
	     {Hlw = priv->LinW >> 1;
	      geSel.x1 += Hlw;
	      geSel.y1 += Hlw;
	      geSel.x2 -= Hlw;
	      geSel.y2 -= Hlw;
	      if ((priv->Bx.x1 > geSel.x1 && priv->Bx.y1 > geSel.y1 &&
		   priv->Bx.x1 < geSel.x2 && priv->Bx.y1 < geSel.y2) ||
		  (priv->Bx.x2 > geSel.x1 && priv->Bx.y2 > geSel.y1 &&
		   priv->Bx.x2 < geSel.x2 && priv->Bx.y2 < geSel.y2) ||
		  (priv->Bx.x3 > geSel.x1 && priv->Bx.y3 > geSel.y1 &&
		   priv->Bx.x3 < geSel.x2 && priv->Bx.y3 < geSel.y2) ||
		  (priv->Bx.x4 > geSel.x1 && priv->Bx.y4 > geSel.y1 &&
		   priv->Bx.x4 < geSel.x2 && priv->Bx.y4 < geSel.y2))
		{geState.ASegP = ASegP;
		 GEVEC(GETESTPT, ASegP);
		}
	      geSel.x1 -= Hlw;
	      geSel.y1 -= Hlw;
	      geSel.x2 += Hlw;
	      geSel.y2 += Hlw;
	     }
	   else
	     if (ASegP->Co.x1 > geSel.x1 && ASegP->Co.y1 > geSel.y1 &&
		 ASegP->Co.x2 < geSel.x2 && ASegP->Co.y2 < geSel.y2)
	       geState.ASegP = ASegP;
	  }	
	break;

    case GEGETFILL:
	if (ASegP && ASegP->Visible)
	   {if (geGetPutFlgs & GEGETPUTCOL)
	       {geIT 		    	      = geSampleAttr.FilCol.OverPrint;
		geSampleAttr.FilCol 	      = ASegP->Col;
		geSampleAttr.FilCol.OverPrint = geIT;
	       }
	    if (geGetPutFlgs & GEGETPUTOPRINT)
		geSampleAttr.FilCol.OverPrint = ASegP->Col.OverPrint;
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
	       {geIT 		     = ASegP->Col.OverPrint;
		ASegP->Col 	     = geSampleAttr.FilCol;
		ASegP->Col.OverPrint = geIT;
	       }
	    if (geGetPutFlgs & GEGETPUTOPRINT)
		ASegP->Col.OverPrint = geSampleAttr.FilCol.OverPrint;
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

    case GEGETTXTFG:
	if (ASegP && ASegP->Visible && priv)
	   {if (geGetPutFlgs & GEGETPUTCOL)
		geSampleAttr.TxtFgCol = priv->Col;
	    if (geGetPutFlgs & GEGETPUTHT)
		geSampleAttr.TxtFgHT = priv->LineHT;
	    if (geGetPutFlgs & GEGETPUTSTYLE)
		geSampleAttr.TxtFgStyle = priv->LineStyle;
	    if (geGetPutFlgs & GEGETPUTWRITEMODE)
	    	geSampleAttr.WritingMode = priv->WritingMode;
	   }
	break;

    case GEGETLINE:
	if (ASegP && ASegP->Visible && priv)
	   {if (geGetPutFlgs & GEGETPUTCOL)
	       {geIT 		    	      = geSampleAttr.LinCol.OverPrint;
		geSampleAttr.LinCol 	      = priv->Col;
		geSampleAttr.LinCol.OverPrint = geIT;
	       }
	    if (geGetPutFlgs & GEGETPUTOPRINT)
		geSampleAttr.LinCol.OverPrint = priv->Col.OverPrint;
	    if (geGetPutFlgs & GEGETPUTHT)
		geSampleAttr.LineHT = priv->LineHT;
	    if (geGetPutFlgs & GEGETPUTSTYLE)
		geSampleAttr.LineStyle = priv->LineStyle;
	    if (geGetPutFlgs & GEGETPUTWRITEMODE)
	    	geSampleAttr.WritingMode = priv->WritingMode;
	    if (geGetPutFlgs & GEGETPUTLINW)
	    	geSampleAttr.LinW = priv->LinW;
	    if (geGetPutFlgs & GEGETPUTJOIN)
	    	geSampleAttr.JoinStyle = priv->JoinStyle;
	    if (geGetPutFlgs & GEGETPUTLINP)
	        {geSampleAttr.LinP 	= priv->Pat.LinP;
	    	 geSampleAttr.DashLen	= priv->Pat.DashLen;
	    	 geSampleAttr.DashIndx	= priv->Pat.DashIndx;
	    	 geDashList[3][0] = priv->Pat.DashList[0];    /* user defined */
	    	 geDashList[3][1] = priv->Pat.DashList[1];
	    	 geDashList[3][2] = priv->Pat.DashList[2];
	    	 geDashList[3][3] = priv->Pat.DashList[3];
		}
	   }
	break;

    case GEPUTLINE:
	if (ASegP && ASegP->Visible && priv)
	   {if (geGetPutFlgs & GEGETPUTCOL)
	       {geIT 		    = priv->Col.OverPrint;
		priv->Col 	    = geSampleAttr.LinCol;
		priv->Col.OverPrint = geIT;
	       }
	    if (geGetPutFlgs & GEGETPUTOPRINT)
		priv->Col.OverPrint = geSampleAttr.LinCol.OverPrint;
	    if (geGetPutFlgs & GEGETPUTHT)
		priv->LineHT = geSampleAttr.LineHT;
	    if (geGetPutFlgs & GEGETPUTSTYLE)
		priv->LineStyle = geSampleAttr.LineStyle;
	    if (geGetPutFlgs & GEGETPUTWRITEMODE)
	    	priv->WritingMode = geSampleAttr.WritingMode;
	    if (geGetPutFlgs & GEGETPUTLINW)
	    	priv->LinW = geSampleAttr.LinW;
	    if (geGetPutFlgs & GEGETPUTJOIN)
	    	priv->JoinStyle = geSampleAttr.JoinStyle;
	    if (geGetPutFlgs & GEGETPUTLINP)
	        {priv->Pat.LinP        = geSampleAttr.LinP;
	    	 priv->Pat.DashLen     = geSampleAttr.DashLen;
	    	 priv->Pat.DashIndx    = geSampleAttr.DashIndx;
	    	 priv->Pat.DashList[0] = geDashList[3][0];    /* user defined */
	    	 priv->Pat.DashList[1] = geDashList[3][1];
	    	 priv->Pat.DashList[2] = geDashList[3][2];
	    	 priv->Pat.DashList[3] = geDashList[3][3];
		}
	   }
	break;

    case GECONSTRAIN:
        if (ASegP && ASegP->Visible && priv)
	  {/*
	    * If it's orthogonal, then it's a bit simpler to figure out
	    */
	   if (priv->Bx.x1 == priv->Bx.x2 || priv->Bx.x1 == priv->Bx.x3 ||
	       priv->Bx.x1 == priv->Bx.x4)
	     {dx = geState.Mouse.x - priv->Bx.x1;
	      dy = geState.Mouse.y - priv->Bx.y1;
	      if (abs(dx) > abs(dy)) ds = abs(dx);
	      else                   ds = abs(dy);

	      if (dx >= 0) priv->Bx.x2 = priv->Bx.x3 = priv->Bx.x1 + ds;
	      else         priv->Bx.x2 = priv->Bx.x3 = priv->Bx.x1 - ds;

	      if (dy >= 0) priv->Bx.y3 = priv->Bx.y4 = priv->Bx.y1 + ds;
	      else         priv->Bx.y3 = priv->Bx.y4 = priv->Bx.y1 - ds;

	      geState.Mouse.x = geOldState.Mouse.x = priv->Bx.x3;
	      geState.Mouse.y = geOldState.Mouse.y = priv->Bx.y3;
	      geState.MouLockCr = GEMOU_X;
	     }
	  }
	break;

    case GECOMP:
        if (ASegP && ASegP->Visible && priv)
	  {priv->Col.RGB.red   = GEMAXUSHORT - priv->Col.RGB.red;
	   priv->Col.RGB.green = GEMAXUSHORT - priv->Col.RGB.green;
	   priv->Col.RGB.blue  = GEMAXUSHORT - priv->Col.RGB.blue;
	   priv->Col.RGB.pixel = geAllocColor(priv->Col.RGB.red,
					      priv->Col.RGB.green,
					      priv->Col.RGB.blue, NULL);
	   ASegP->Col.RGB.red   = GEMAXUSHORT - ASegP->Col.RGB.red;
	   ASegP->Col.RGB.green = GEMAXUSHORT - ASegP->Col.RGB.green;
	   ASegP->Col.RGB.blue  = GEMAXUSHORT - ASegP->Col.RGB.blue;
	   ASegP->Col.RGB.pixel = geAllocColor(ASegP->Col.RGB.red,
					       ASegP->Col.RGB.green,
					       ASegP->Col.RGB.blue, NULL);
	   geGenRgbToCmyk(&(priv->Col.RGB), &(priv->Col.CMYK));
	   geGenRgbToCmyk(&(ASegP->Col.RGB), &(ASegP->Col.CMYK));
	   geGenNameRgb(&(priv->Col));
	   geGenNameRgb(&(ASegP->Col));
          }
	break;

    case GEINQLIVE:                             /* Increment livesegs counter*/
	if (ASegP && ASegP->Live) geState.LiveSegs++;
	break;

    case GEINQVISIBLE:                          /* Increment visiblesegs ctr */
	if (ASegP && ASegP->Live && ASegP->Visible) geState.VisibleSegs++;
	break;

    case GEINQXVISIBLE:                        /* Are there HIDDEN segs */
	if (ASegP && ASegP->Live && !ASegP->Visible)
	   geState.Hidden = TRUE; 
	break;

    case GEFLIP:
	if (ASegP && ASegP->Visible && priv) 
	    geGenFlpBx(geState.Constraint, &priv->Bx);
	break;

    case GEADDVERT:
	/*
	 * Add the coords of this object to the vertex list  - used
	 * for selection
	 */
	if (ASegP && ASegP->Visible && priv && geVn < GEMAX_PTS)
	   {GESTOREVERT(geVn, GETPIX(priv->Bx.x1), GETPIX(priv->Bx.y1));
	    if (++geVn >= GEMAX_PTS) break;
	    GESTOREVERT(geVn, GETPIX(priv->Bx.x2), GETPIX(priv->Bx.y2));
	    if (++geVn >= GEMAX_PTS) break;
	    GESTOREVERT(geVn, GETPIX(priv->Bx.x3), GETPIX(priv->Bx.y3));
	    if (++geVn >= GEMAX_PTS) break;
	    GESTOREVERT(geVn, GETPIX(priv->Bx.x4), GETPIX(priv->Bx.y4));
	    geVn++;
	   }
	break;

#endif

    case GEGETSEG_COL:
        if (ASegP && ASegP->Col.RGB.pixel == geGenCol.pixel ||
	    (priv && priv->Col.RGB.pixel == geGenCol.pixel))
	  geState.ASegP = ASegP;
	break;

    case GEMAG:
    case GEMAGRESD:
        if (ASegP && ASegP->Visible && priv)
	   {if (!geMagGrp)
		{geMagGrp = TRUE;
		 geMagCx  = (ASegP->Co.x1 + ASegP->Co.x2) >> 1;
		 geMagCy  = (ASegP->Co.y1 + ASegP->Co.y2) >> 1;
		}
	    if (!geState.EditPts || (priv->EditPt1 && priv->EditPt2))
	      geGenMagBx(&priv->Bx);
	    else
	      {if (priv->EditPt1)
		 {Pta.x       = priv->Bx.x1;
		  Pta.y       = priv->Bx.y1;
		  geGenMagPt(&Pta);
		  priv->Bx.x1 = Pta.x;
		  priv->Bx.y1 = Pta.y;
	         }
	       else if (priv->EditPt2)
		 {Pta.x       = priv->Bx.x2;
		  Pta.y       = priv->Bx.y2;
		  geGenMagPt(&Pta);
		  priv->Bx.x2 = Pta.x;
		  priv->Bx.y2 = Pta.y;
	         }
	       else if (priv->EditPt3)
		 {Pta.x       = priv->Bx.x3;
		  Pta.y       = priv->Bx.y3;
		  geGenMagPt(&Pta);
		  priv->Bx.x3 = Pta.x;
		  priv->Bx.y3 = Pta.y;
	         }
	       else if (priv->EditPt4)
		 {Pta.x       = priv->Bx.x4;
		  Pta.y       = priv->Bx.y4;
		  geGenMagPt(&Pta);
		  priv->Bx.x4 = Pta.x;
		  priv->Bx.y4 = Pta.y;
	         }
	      }
            if (geState.ZoomLineThick)
              priv->LinW = ((priv->LinW ? priv->LinW : GESUI1) * geMagFX) / 100;
	   }
	break;

    case GEROTX:
        if (ASegP && ASegP->Visible && priv)
	  {if (!geState.EditPts || priv->EditPt1)
	     geGenRotX(&priv->Bx.x1, &priv->Bx.y1);
	   if (!geState.EditPts || priv->EditPt2)
	     geGenRotX(&priv->Bx.x2, &priv->Bx.y2);
	   if (!geState.EditPts || priv->EditPt3)
	     geGenRotX(&priv->Bx.x3, &priv->Bx.y3);
	   if (!geState.EditPts || priv->EditPt4)
	     geGenRotX(&priv->Bx.x4, &priv->Bx.y4);
	  }
	break;

    case GEROTFIXED:
        if (ASegP && ASegP->Visible && priv)
	  {if (( geRot.Clockwise && geRot.Beta  == 90.) ||
	       (!geRot.Clockwise && geRot.Alpha == 90.))
	     {geGenRot90(&priv->Bx.x1, &priv->Bx.y1);
	      geGenRot90(&priv->Bx.x2, &priv->Bx.y2);
	      geGenRot90(&priv->Bx.x3, &priv->Bx.y3);
	      geGenRot90(&priv->Bx.x4, &priv->Bx.y4);
	     }
	   else
	     {geGenRotX (&priv->Bx.x1, &priv->Bx.y1);
	      geGenRotX (&priv->Bx.x2, &priv->Bx.y2);
	      geGenRotX (&priv->Bx.x3, &priv->Bx.y3);
	      geGenRotX (&priv->Bx.x4, &priv->Bx.y4);
	     }
	  }
	break;

    case GEEQUATE:
    case GEEQUATE_NOIMG:
        if(ASegP && geState.ASegP && ASegP->Handler == geState.ASegP->Handler
	   && priv && (privnu = (struct GE_BOX *)geState.ASegP->Private))
	  {geState.ASegP->Co = ASegP->Co;
	   geGenCopyAnim(geState.ASegP, ASegP);
	   geGenCopyDesc(geState.ASegP, ASegP);
	   privnu->Bx        = priv->Bx;
	   privnu->LinW      = priv->LinW;
	  }
	break;

    case GEVISIBLE:
	if (ASegP) ASegP->Visible     = TRUE;
	break;

    case GEXVISIBLE:
	if (ASegP) ASegP->Visible     = FALSE;
	break;

    case GEDEL:
	if (ASegP) ASegP->Live = FALSE;         /* Delete the segment        */
	break;

    case GEPURGEHIDDEN:
	if (ASegP && !ASegP->Visible)
	  {GEVEC(GEKILL, ASegP);}		/* KILL it now		     */
	break;

    case GEXDEL:
	if (ASegP) ASegP->Live = TRUE;          /* UnDelete the segment      */
	break;

    case GEPSADJUST:
	/*
	 * This adjustment is required for producing the PostScript rendition
	 * of the graphic.  The pixel "anchor" in X is in the "upper left
	 * corner" of the pixel; whereas PostScript has it at the "center"
	 * of the pixel.
	 */
	if (ASegP && ASegP->Live && ASegP->Visible && priv)
	   {if (priv->LinW) geState.Dy = (priv->LinW % GESUI2) >> 1;
	    else	    geState.Dy = GESUIPH;
	    if (geState.Dx = geState.Dy)
	      {GEVEC(GEMOVE, ASegP)};
	   }
	break;

    case GEKILL:
	if(ASegP) geSegKil(&ASegP);
	break;

    case GEKILPRIV:
	if (ASegP->Private)
	  geFree(&ASegP->Private, sizeof(struct GE_BOX));
	break;

    case GEWRITE:
    case GEREAD:
        geBoxIO(cmd, ASegP);
	break;

    default:
	break;
   }
}

#ifdef GERAGS

geGenPtSelBox(priv)
struct GE_BOX   *priv;

{
int		mind, select;
long		x, y;
struct GE_CO	Co;

select = TRUE;
mind   = GEMAXSHORT;
Co.x1  = priv->Bx.x1;
Co.y1  = priv->Bx.y1;
Co.x2  = priv->Bx.x2;
Co.y2  = priv->Bx.y2;
x      = (geSel.x1 + geSel.x2) >> 1;
y      = (geSel.y1 + geSel.y2) >> 1;

if (!geGenLinPtSel(&Co, priv->LinW, x, y))
   {if (geState.SelRadC < mind) mind = geState.SelRadC;
    Co.x1 = priv->Bx.x2;
    Co.y1 = priv->Bx.y2;
    Co.x2 = priv->Bx.x3;
    Co.y2 = priv->Bx.y3;
    if (!geGenLinPtSel(&Co, priv->LinW, x, y))
   	{if (geState.SelRadC < mind) mind = geState.SelRadC;
         Co.x1 = priv->Bx.x3;
	 Co.y1 = priv->Bx.y3;
	 Co.x2 = priv->Bx.x4;
	 Co.y2 = priv->Bx.y4;
	 if (!geGenLinPtSel(&Co, priv->LinW, x, y))
   	    {if (geState.SelRadC < mind) mind = geState.SelRadC;
	     Co.x1 = priv->Bx.x4;
	     Co.y1 = priv->Bx.y4;
	     Co.x2 = priv->Bx.x1;
	     Co.y2 = priv->Bx.y1;
	     if (!geGenLinPtSel(&Co, priv->LinW, x, y))
		 select = FALSE;
	    }
	}
   }

if (geState.SelRadC < mind) mind = geState.SelRadC;
return (select);
}

#endif
