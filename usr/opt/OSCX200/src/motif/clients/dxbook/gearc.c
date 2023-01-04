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
static char SccsId[] = "@(#)gearc.c	1.5\t12/13/89";
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
**	GEARC	             	       Arc object handler
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
**	GNE 02/04/87 Created
**
**--
**/

#include "geGks.h"
#include <math.h>

extern	float		geGenAngle();
extern unsigned long 	geAllocColor();
extern char          	*geMalloc();
extern Region		geXPolygonRegion();
extern double	        atof();	
extern int 		atoi();	

geArc(cmd, ASegP)
long     	cmd;
struct GE_SEG	*ASegP;
{                       
#define         GEOBJLIS 6

char            *error_string, *string, buff[15];
short           GravAlignSav, EditPtFlip;
register int	i, theta, inc;
int             icx, icy, Hlw, PixLinW, temp, XSav, YSav, ix, iy,
                startangle, endangle, arclen, NumFrames, Rad;
long		x, y, dx, dy;
float           FList[GEOBJLIS], FCon, FLprev, FH, FHprev, ftemp, fdx, fdy,
                FOCon, FICon, fx, fy, fs, falpha, fbeta, fgamma, ft,
		ftemp1;
Region		TRegion;
XRectangle	TRect;

struct GE_ARC   *priv, *privnu;
struct GE_CO    Co;
struct GE_PT    Pt;

if (ASegP)
  {if (!ASegP->Live && cmd != GEKILL    && cmd != GERELPRIV &&
                       cmd != GEKILPRIV && cmd != GEGETSEG_COL && cmd != GEXDEL)
     return;
   priv = (struct GE_ARC *)ASegP->Private;
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

	     if (priv->Pat.LinP != LineSolid)
	       XSetDashes(geDispDev, geGC2, 0, priv->Pat.DashList,
			  priv->Pat.DashLen);
	     XSetArcMode(geDispDev, geGC3, ArcChord);
	     GECAPSTYLE(geGC2, priv->CapStyle);
	     GEJOINSTYLE(geGC2, priv->JoinStyle);
	     PixLinW = GETPIX(priv->LinW);	/* These are for the benefit */
	     x	  = GETPIX(priv->Pta.x);	/*of the ULTRIX c compiler - */
	     y	  = GETPIX(priv->Pta.y);	/*next call is too complex   */
	     GE_SET_BG_DISP_ATTR;
	     GE_SET_FG_DISP_ATTR;
	     geDrawArcNu(geState.Drawable, geGC2, priv->C.x, priv->C.y,
			 priv->Rad, priv->Alpha, priv->Beta,
			 PixLinW, priv->Pat.LinP, geDispAttr.FgPixel,
			 geDispAttr.FgStyle, geDispAttr.FgHT,
			 priv->WritingMode, geDispAttr.BgStyle,
			 geDispAttr.BgPixel, geDispAttr.BgHT,
			 ASegP->FillWritingMode,
			 GETPIX(priv->Pta.x), GETPIX(priv->Pta.y),
			 GETPIX(priv->Ptb.x), GETPIX(priv->Ptb.y));

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
	  PixLinW = GETPIX(priv->LinW);
	 if (geState.AnimDispMode == GEANIM_TRUE)
	     geDrawArcNu(geState.Drawable, geGC2, priv->C.x, priv->C.y,
			 priv->Rad, priv->Alpha, priv->Beta,
			 PixLinW, priv->Pat.LinP, GEXORPIXEL,
			 priv->LineStyle, priv->LineHT, GXxor,
			 GETRANSPARENT, 0, 0, 0,
			 GETPIX(priv->Pta.x), GETPIX(priv->Pta.y),
			 GETPIX(priv->Ptb.x), GETPIX(priv->Ptb.y));

	 else
	     geDrawArcNu(geState.Drawable, geGC5,
			 priv->C.x, priv->C.y, priv->Rad,
			 priv->Alpha, priv->Beta,
			 0,0,0,0,0,0,0,0,0,0,
			 GETPIX(priv->Pta.x), GETPIX(priv->Pta.y),
			 GETPIX(priv->Ptb.x), GETPIX(priv->Ptb.y));

	  if (geAnimMin && (abs(priv->Ptb.x - priv->Pta.x) > geAnimMin ||
			    abs(priv->Ptb.y - priv->Pta.y) > geAnimMin))
	     {XDrawLine(geDispDev, geState.Drawable, geGC5,
			GETPIX(priv->Ptm.x) - 3, GETPIX(priv->Ptm.y),
			GETPIX(priv->Ptm.x) + 3, GETPIX(priv->Ptm.y));
	      XDrawLine(geDispDev, geState.Drawable, geGC5,
			GETPIX(priv->Ptm.x), GETPIX(priv->Ptm.y) - 3,	
			GETPIX(priv->Ptm.x), GETPIX(priv->Ptm.y) + 3);
	     }
	 }
	break;

    case GEHDISP:
    case GEHXDISP:
	if (priv && ASegP && ASegP->Visible && ASegP->InFrame)
	   {if (geState.EditPts)
		{if (priv->EditPt1) geGenBoxEP(cmd, priv->Pta.x, priv->Pta.y);
		 if (priv->EditPt2) geGenBoxEP(cmd, priv->Ptm.x, priv->Ptm.y);
		 if (priv->EditPt3) geGenBoxEP(cmd, priv->Ptb.x, priv->Ptb.y);
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
	 	{if (priv->Pat.LinP != LineSolid)
		    XSetDashes(geDispDev, geGC2, 0, priv->Pat.DashList,
			       priv->Pat.DashLen);
	  	 XSetArcMode(geDispDev, geGC3, ArcChord);
	 	 PixLinW = GETPIX(priv->LinW);	/* These are for the benefit */
	 	 x	  = GETPIX(priv->Pta.x);/*of the ULTRIX c compiler - */
	 	 y	  = GETPIX(priv->Pta.y);/*next call is too complex   */
	 	 GECAPSTYLE(geGC2, priv->CapStyle);
	 	 GEJOINSTYLE(geGC2, priv->JoinStyle);
	 	 geDrawArcNu(geState.Drawable, geGC2,
		      	     priv->C.x, priv->C.y, priv->Rad,
		   	     priv->Alpha, priv->Beta,
		     	     PixLinW, priv->Pat.LinP, GEXORPIXEL,
		      	     FillSolid, priv->LineHT, GXxor,
                             GETRANSPARENT, 0,0,0,
 		      	     GETPIX(priv->Pta.x), GETPIX(priv->Pta.y),
		      	     GETPIX(priv->Ptb.x), GETPIX(priv->Ptb.y));
		}
	 }
	break;

    case GECREATE:
    case GECREATE_NOSNAP1:
	geSegCr();
	geState.ASegP->Private    = geMalloc(sizeof(struct GE_ARC));
	priv 		          = (struct GE_ARC *)geState.ASegP->Private;
	geState.ASegP->Handle[0]  = 'A';
	geState.ASegP->Handle[1]  = 'R';
	geState.ASegP->Handle[2]  = 'C';
	geState.ASegP->Handle[3]  = '\0';
	geState.ASegP->Handler    = geArc;
	
	if(cmd != GECREATE_NOSNAP1 && (geState.Grav.Align ||
				       geState.APagP->Grid.XAlign ||
	   			       geState.APagP->Grid.YAlign))
	  {geState.Dx = geState.Dy   = 0;
	   geState.Grav.OPt.x = geState.Mouse.x;
	   geState.Grav.OPt.y = geState.Mouse.y;
	   geGenAln(geState.Mouse.x, geState.Mouse.y);
	   geState.Grav.OPt.x = geState.Mouse.x += geState.Dx;
	   geState.Grav.OPt.y = geState.Mouse.y += geState.Dy;
	  }
	priv->Pta.x               = geU.XR = geState.Mouse.x;
	priv->Pta.y               = geU.YR = geState.Mouse.y;
	geMouseXY(geState.Window);
	priv->Ptb.x		  = geState.Mouse.x + GETSUI(10);
	priv->Ptb.y		  = geState.Mouse.y + GETSUI(10);
	priv->Ptm.x               = ((priv->Pta.x + priv->Ptb.x) >> 1)
	                            + GETSUI(2);
	priv->Ptm.y               = ((priv->Pta.y + priv->Ptb.y) >> 1)
	                            - GETSUI(2);
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
	priv->CapStyle		  = geAttr.CapStyle;
	priv->JoinStyle		  = geAttr.JoinStyle;
	priv->EditPt1 = priv->EditPt2 = priv->EditPt3 = FALSE;
	geGenRC(priv->Pta.x,priv->Pta.y, priv->Ptm.x,priv->Ptm.y,
		priv->Ptb.x,priv->Ptb.y, &priv->C.x,&priv->C.y, &priv->Rad,
		   &priv->Alpha, &priv->Beta);
	GEVEC(GEBOUNDS, geState.ASegP);
	geMnStat(GEDISP, 52);

	if (!geState.ObjEdCr)
	  {geState.Mouse.x		  = priv->Ptb.x;
	   geState.Mouse.y		  = priv->Ptb.y;
	   if(geState.Grav.Align || geState.APagP->Grid.XAlign ||
	      geState.APagP->Grid.YAlign)
	     {geState.Dx = geState.Dy   = 0;
	      geGenAln(geState.Mouse.x, geState.Mouse.y);
	      geState.Mouse.x += geState.Dx;
	      geState.Mouse.y += geState.Dy;
	     }
	   ix = GETPIX(geState.Mouse.x);
	   iy = GETPIX(geState.Mouse.y);
	   XWarpPointer(geDispDev, None, geState.Window, 0, 0, 0,
			0, ix, iy);
	  }
        
	geAnim(GECREATE,     geState.ASegP);
	if (!geState.ObjEdCr)
	  {geMnStat(GEDISP, 53);
	   geState.Mouse.x		  = priv->Ptm.x;
	   geState.Mouse.y		  = priv->Ptm.y;
	   if(geState.Grav.Align || geState.APagP->Grid.XAlign ||
	      geState.APagP->Grid.YAlign)
	     {geState.Dx = geState.Dy   = 0;
	      geGenAln(geState.Mouse.x, geState.Mouse.y);
	      geState.Mouse.x += geState.Dx;
	      geState.Mouse.y += geState.Dy;
	     }
	   ix = GETPIX(geState.Mouse.x);
	   iy = GETPIX(geState.Mouse.y);
	   XWarpPointer(geDispDev, None, geState.Window, 0, 0, 0,
			0, ix, iy);
        
	   GravAlignSav       = geState.Grav.Align;
	   geState.Grav.Align = FALSE;
	   geAnim(GEWARPX, geState.ASegP);
	   geState.Grav.Align = GravAlignSav;
	   GEVEC(GEDISP,   geState.ASegP);

	   if (geState.ASegP)
	     {geState.Mouse.x		  = priv->Ptb.x;
	      geState.Mouse.y		  = priv->Ptb.y;
	      ix = GETPIX(geState.Mouse.x);
	      iy = GETPIX(geState.Mouse.y);
	      XWarpPointer(geDispDev, None, geState.Window, 0, 0, 0,
			   0, ix, iy);
	     }
	  }

	geState.MsgNum = GEQUIETMSG;
	geMnStat(GEDISP, geState.MsgNum);
	break;

    case GESETATTRERASE:
	if (ASegP && ASegP->Visible && ASegP->WhiteOut &&
	    (priv = (struct GE_ARC *)ASegP->Private))
	  {GE_SET_ATTR_ERASE;
	  }
	break;

    case GESETATTRERASE_WHITE:
	if (ASegP && ASegP->Visible && ASegP->WhiteOut && !geState.PrintPgBg &&
	    (priv = (struct GE_ARC *)ASegP->Private))
	  {GE_SET_ATTR_ERASE_WHITE;
	  }
	break;

    case GEOBJCREATE:
	if (!ASegP || !ASegP->Visible) break;

	if (geU.DMap != -1) FCon = geU.Con[geU.DMap];
	else                FCon = 1.0;
	FOCon      = FCon / GEFSUIPP;
	FICon      = GEFSUIPP / FCon;

	/* Set up default values for arc object and fetch the create
	 * to size modal box.  Only use default values if in create 
	 * mode, otherwise use the current values (editing).
	 */
 	if (geState.InCreateMode)
	  {priv->Pta.x = geState.Mouse.x;
	   priv->Pta.y = geState.Mouse.y;
	   priv->Ptm.x = priv->Pta.x +
	     (int)((float)GESUI1 * (20. * .707106781187));
	   priv->Ptm.y = priv->Pta.y +
	     (int)((float)GESUI1 * (20. * (1. - .707106781187)));
	   priv->Ptb.x = priv->Pta.x + GESUI1 * 20;
	   priv->Ptb.y = priv->Pta.y + GESUI1 * 20;
	   geGenRC(priv->Pta.x,priv->Pta.y, priv->Ptm.x,priv->Ptm.y,
		   priv->Ptb.x,priv->Ptb.y, &priv->C.x,&priv->C.y,
		   &priv->Rad, &priv->Alpha, &priv->Beta);
	  }

	FList[1]    = (float)(priv->Rad * FCon);
	FList[2]    = (float)(priv->Alpha >> 6);
	FList[3]    = (float)(priv->Beta >> 6);
	FList[4]    = 0.;
	FList[5]    = (float)priv->LinW  * FOCon;

	if (!geUI.WidgetArray[GEWCREATE_ARC] || 
	    !XtIsManaged(geUI.WidgetArray[GEWCREATE_ARC]))
	  {if (MrmFetchWidget(geUI.Id, GECREATE_ARC_POPUP,
			      XtParent(geState.APagP->WidgetId),
			      &geUI.WidgetArray[GEWCREATE_ARC], 
			      &geUI.Class) != MrmSUCCESS)
	     {error_string = (char *) geFetchLiteral("GE_ERR_NOWIDGET",
						     MrmRtypeChar8);
	      if (error_string != NULL) 
	        {sprintf(geErrBuf, error_string, GECREATE_ARC_POPUP);
	         geError(geErrBuf, TRUE);
	         XtFree(error_string);
	        }
	     }
	  }

        /* Set all the fields before managing box. 
 	 */    
	sprintf(buff, "%.3f", FList[1]);		/* Radius */
 	XmTextSetString(geUI.WidgetArray[GEWCREATE_RAD], buff);
	sprintf(buff, "%.3f", FList[2]);		/* Start Angle */
 	XmTextSetString(geUI.WidgetArray[GEWCREATE_STARTA], buff);
	sprintf(buff, "%.3f", FList[3]);		/* End Angle */
 	XmTextSetString(geUI.WidgetArray[GEWCREATE_ENDA], buff);
	sprintf(buff, "%.3f", FList[4]);		/* DegRot */
 	XmTextSetString(geUI.WidgetArray[GEWCREATE_DEGROT], buff);
	sprintf(buff, "%.3f", FList[5]);		/* LinW */
 	XmTextSetString(geUI.WidgetArray[GEWCREATE_LINW], buff);

	XtManageChild(geUI.WidgetArray[GEWCREATE_ARC]);

	if (geState.InCreateMode)
  	  { GEVEC(GEXDISP, ASegP); } 			/* XOR the object */
	break;

    case GEOBJEDIT:
    case GEEDIT:
	if (!ASegP || !ASegP->Visible) break;

	if (!geUI.WidgetArray[GEWCREATE_ARC])
	  { GEVEC(GEOBJCREATE, ASegP);
	    break;
	  }

	if (geU.DMap != -1) FCon = geU.Con[geU.DMap];
	else                FCon = 1.0;
	FOCon      = FCon / GEFSUIPP;
	FICon      = GEFSUIPP / FCon;

	/* Get all the fields before displaying the new object.
 	 */                                              
							/* Radius */
 	string = XmTextGetString(geUI.WidgetArray[GEWCREATE_RAD]);
	ftemp  = (float)atof(string) / geU.Con[geU.DMap];
	sprintf(buff, "%.3f", ftemp);
	ftemp  = (float)atof(buff);
	/*
	 * To see if the user requested radius is statistically different
	 * from the current value, reduce the current value to 3 significant
	 * digits.
	 */
	sprintf(buff, "%.3f", priv->Rad);
	ftemp1 = (float)atof(buff);
	if (ftemp != ftemp1)
	  {ftemp *= GEFSUIPP;				/* Convert requested */
	   GEINT(ftemp, Rad);				/* radius to suis    */
	   ftemp1 = priv->C.x * GEFSUIPP;
	   GEINT(ftemp1, temp);
	   Co.x1 = temp;
	   ftemp1 = priv->C.y * GEFSUIPP;
	   GEINT(ftemp1, temp);
	   Co.y1 = temp;
	   /*
	    * First end point of arc
	    */
	   Co.x2 = priv->Pta.x;
	   Co.y2 = priv->Pta.y;
	   geGenReLen(&Co, Rad);
	   dx = Co.x2 - priv->Pta.x;
	   dy = Co.y2 - priv->Pta.y;
	   priv->Pta.x = Co.x2;
	   priv->Pta.y = Co.y2;
	   /*
	    * Middle point (or thereabouts) of arc
	    */
	   Co.x2 = priv->Ptm.x;
	   Co.y2 = priv->Ptm.y;
	   geGenReLen(&Co, Rad);
	   priv->Ptm.x = Co.x2;
	   priv->Ptm.y = Co.y2;
	   /*
	    * Second end point of arc
	    */
	   Co.x2 = priv->Ptb.x;
	   Co.y2 = priv->Ptb.y;
	   geGenReLen(&Co, Rad);
	   priv->Ptb.x = Co.x2;
	   priv->Ptb.y = Co.y2;
	   geState.Dx = -dx;
	   geState.Dy = -dy;
	   GEVEC(GEMOVE, ASegP);
	   geGenRC(priv->Pta.x,priv->Pta.y, priv->Ptm.x,priv->Ptm.y,
		   priv->Ptb.x,priv->Ptb.y, &priv->C.x,&priv->C.y, &priv->Rad,
		   &priv->Alpha, &priv->Beta);
	   GEVEC(GEBOUNDS, ASegP);
	   ftemp = (float)(priv->Rad * FCon);
	   sprintf(buff, "%.3f", ftemp);		/* Radius */
	   XmTextSetString(geUI.WidgetArray[GEWCREATE_RAD], buff);
	  }
	XtFree(string);

                                                         /* Extent */
 	string = XmTextGetString(geUI.WidgetArray[GEWCREATE_ENDA]);
	ftemp  = atof(string);
	ftemp1 = ftemp * 64.;
	GEINT(ftemp, arclen);
	if (arclen != priv->Beta)
	  {falpha = (float)priv->Alpha / 64.;
	   fbeta  = ftemp;

	   ftemp  = priv->C.x * GEFSUIPP + priv->Rad * GEFSUIPP *
	     cos(falpha - fbeta);
	   GEINT(ftemp, priv->Ptb.x);
	   ftemp  = priv->C.y * GEFSUIPP -  priv->Rad * GEFSUIPP *
	     sin(falpha - fbeta);
	   GEINT(ftemp, priv->Ptb.y);

	   ftemp  = priv->C.x * GEFSUIPP + priv->Rad * GEFSUIPP *
	     cos(falpha + (falpha - fbeta) / 2.);
	   GEINT(ftemp, priv->Ptm.x);
	   ftemp  = priv->C.y * GEFSUIPP -  priv->Rad * GEFSUIPP *
	     sin(falpha + (falpha - fbeta) / 2.);
	   GEINT(ftemp, priv->Ptm.y);

	   geGenRC(priv->Pta.x,priv->Pta.y, priv->Ptm.x,priv->Ptm.y,
		   priv->Ptb.x,priv->Ptb.y, &priv->C.x,&priv->C.y, &priv->Rad,
		   &priv->Alpha, &priv->Beta);
	   GEVEC(GEBOUNDS, ASegP);
	  }
	XtFree(string);
  
                                                         /* Start Angle */
 	string = XmTextGetString(geUI.WidgetArray[GEWCREATE_STARTA]);
	ftemp  = falpha = atof(string);
	ftemp1 = ftemp * 64.;
	GEINT(ftemp1, startangle);
	if (startangle != priv->Alpha)
	  {/*
	    * Start angle has changed, rotate the arc about the center
	    * of the defining circle.
	    */
	   ftemp = ((float)startangle - (float)priv->Alpha) / 64.;
	   ftemp1  = priv->C.x * 1024.;
	   GEINT(ftemp1,  geEditBox.Pt.x);
	   ftemp1  = priv->C.y * 1024.;
	   GEINT(ftemp1, geEditBox.Pt.y);
	   geGenRotObj(ASegP, ftemp);
	  }
	XtFree(string);
							 /* DegRot */
	if (XmToggleButtonGetState(geUI.WidgetArray[GEWCREATE_ROTATE]))
 	  {string = XmTextGetString(geUI.WidgetArray[GEWCREATE_DEGROT]);
	   if (ftemp = (float)atof(string))
	     {/*
	       * Rotate the arc about its center
	       */
	       ftemp1  = priv->C.x * 1024.;
	       GEINT(ftemp1,  geEditBox.Pt.x);
	       ftemp1  = priv->C.y * 1024.;
	       GEINT(ftemp1, geEditBox.Pt.y);
	       geGenRotObj(ASegP, ftemp);
	       /*
		* Have to recompute the start angle
		*/
	       falpha += ftemp;
	       if (falpha > 360.)
		 while (falpha > 360.) falpha -= 360.;
	       else
	       if (falpha < -360.)
		 while (falpha < -360.) falpha += 360.;
	       FList[2] = falpha;
	       sprintf(buff, "%.3f", FList[2]);		/* Start Angle */
	       XmTextSetString(geUI.WidgetArray[GEWCREATE_STARTA], buff);
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
	geState.ASegP->Private    = geMalloc(sizeof(struct GE_ARC));
	privnu 		          = (struct GE_ARC *)geState.ASegP->Private;
	*privnu                        = *priv;
	geGenCopySeg(geState.ASegP, ASegP);
	GEVEC(GEBOUNDS, geState.ASegP);
	break;

    case GEWARP1:
	if (ASegP && ASegP->Visible && priv)
	  {priv->Pta.x = geState.Mouse.x;
	   priv->Pta.y = geState.Mouse.y;
	   geGenRC(priv->Pta.x,priv->Pta.y, priv->Ptm.x,priv->Ptm.y,
		   priv->Ptb.x,priv->Ptb.y, &priv->C.x,&priv->C.y, &priv->Rad,
		   &priv->Alpha, &priv->Beta);
	  }
	break;

    case GEWARP2:
    case GECONSTRAIN:
        if (ASegP && ASegP->Visible && priv)
	  {priv->Ptb.x = geState.Mouse.x;
	   priv->Ptb.y = geState.Mouse.y;
	   priv->Ptm.x = (priv->Pta.x + priv->Ptb.x) >> 1;
	   priv->Ptm.y = (priv->Pta.y + priv->Ptb.y) >> 1;
	   GEINT(priv->C.x, icx);
	   GEINT(priv->C.y, icy);
	   if (priv->Ptm.x < icx) 	priv->Ptm.x -= GETSUI(6);
	   else				priv->Ptm.x += GETSUI(6);
	   if (priv->Ptm.y < icy) 	priv->Ptm.y -= GETSUI(6);
	   else				priv->Ptm.y += GETSUI(6);
	   geGenRC(priv->Pta.x,priv->Pta.y, priv->Ptm.x,priv->Ptm.y,
		   priv->Ptb.x,priv->Ptb.y, &priv->C.x,&priv->C.y, &priv->Rad,
		   &priv->Alpha, &priv->Beta);
	  }
	break;

    case GEWARPX:
        if (ASegP && ASegP->Visible && priv)
	  {priv->Ptm.x = geState.Mouse.x;
	   priv->Ptm.y = geState.Mouse.y;
	   geGenRC(priv->Pta.x,priv->Pta.y, priv->Ptm.x,priv->Ptm.y,
		   priv->Ptb.x,priv->Ptb.y, &priv->C.x,&priv->C.y, &priv->Rad,
		   &priv->Alpha, &priv->Beta);
	  }
	break;

#endif

    case GEANIMRESET:
	if (ASegP && ASegP->Visible)
	  geAnimReset(ASegP);
 
	break;

    case GEMOVE:
    case GEGRAVTST:
    case GESCRL:
    case GESCRR:
    case GESCRU:
    case GESCRD:
        if ((cmd == GEGRAVTST || cmd == GEMOVE) &&
	    (!ASegP || !ASegP->Visible)) break;

        if (priv)
	  {if(geState.Grav.Align || geState.APagP->Grid.XAlign ||
	      geState.APagP->Grid.YAlign)
	     {geState.Grav.Lock = FALSE;
	      if (!geState.EditPts || priv->EditPt1)
		geGenAln(priv->Pta.x, priv->Pta.y);
	      if (!geState.Grav.Lock)
		{if (!geState.EditPts || priv->EditPt3)
		   geGenAln(priv->Ptb.x, priv->Ptb.y);
		 if (!geState.Grav.Lock && 
		     (!geState.EditPts || priv->EditPt2))
		    geGenAln(priv->Ptm.x, priv->Ptm.y);
	        }
	     }
	   if (cmd != GEGRAVTST && (geState.Dx || geState.Dy))
	     {if (!geState.EditPts || priv->EditPt1)
		{priv->Pta.x += geState.Dx;
		 priv->Pta.y += geState.Dy;
	        }
	      if (!geState.EditPts || priv->EditPt3)
		{priv->Ptb.x += geState.Dx;
		 priv->Ptb.y += geState.Dy;
	        }
	      if (!geState.EditPts || priv->EditPt2)
		{priv->Ptm.x += geState.Dx;
		 priv->Ptm.y += geState.Dy;
	        }
	      geGenRC(priv->Pta.x,priv->Pta.y, priv->Ptm.x,priv->Ptm.y,
		  priv->Ptb.x,priv->Ptb.y, &priv->C.x,&priv->C.y, &priv->Rad,
		   &priv->Alpha, &priv->Beta);
	     }
	  }
	break;

#ifdef GERAGS

    case GETESTPT:
        if (!geState.EditPts || !ASegP || !ASegP->Visible) break;

	EditPtFlip = FALSE;
	if GESELTST(priv->Pta.x, priv->Pta.y)
	  {priv->EditPt1 = priv->EditPt1 ? FALSE : TRUE;
	   EditPtFlip = TRUE;
	  }
	if GESELTST(priv->Ptm.x, priv->Ptm.y)
	  {priv->EditPt2 = priv->EditPt2 ? FALSE : TRUE;
	   EditPtFlip = TRUE;
	  }

	if GESELTST(priv->Ptb.x, priv->Ptb.y)
	  {priv->EditPt3 = priv->EditPt3 ? FALSE : TRUE;
	   EditPtFlip = TRUE;
	  }

	if (!EditPtFlip)
	  {priv->EditPt1 = priv->EditPt1 ? FALSE : TRUE;
	   priv->EditPt2 = priv->EditPt2 ? FALSE : TRUE;
	   priv->EditPt3 = priv->EditPt3 ? FALSE : TRUE;
	  }

	break;

    case GETESTPTCLR:
        if (!ASegP || !ASegP->Visible) break;

	priv->EditPt1 = FALSE;
	priv->EditPt2 = FALSE;
	priv->EditPt3 = FALSE;
	break;

    case GEGRAV:
        if (ASegP && ASegP->Visible && priv &&
	    (geState.Grav.Pt.x >= (ASegP->Co.x1 - geState.Grav.Rad) &&
	     geState.Grav.Pt.y >= (ASegP->Co.y1 - geState.Grav.Rad) &&
	     geState.Grav.Pt.x <= (ASegP->Co.x2 + geState.Grav.Rad) &&
	     geState.Grav.Pt.y <= (ASegP->Co.y2 + geState.Grav.Rad)))
	  {geGenGrav(GEGRAVLOCK, priv->Pta.x, priv->Pta.y);
	   geGenGrav(GEGRAVLOCK, priv->Ptm.x, priv->Ptm.y);
	   geGenGrav(GEGRAVLOCK, priv->Ptb.x, priv->Ptb.y);
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

	  {geVn = 0;
	   GEVEC(GEADDVERT, ASegP); 
	   geVert[geVn++] = geVert[0];
	   if ((TRegion = geXPolygonRegion(geVert, geVn, EvenOddRule)))
		{XClipBox(TRegion, &TRect);
	   	 geClip.x1 = GETSUI(TRect.x);
	   	 geClip.y1 = GETSUI(TRect.y);
	   	 geClip.x2 = ASegP->Co.x1 + GETSUI(TRect.width);
	  	 geClip.y2 = ASegP->Co.y1 + GETSUI(TRect.height);
	  	 XDestroyRegion(TRegion);
		}
	   else
		{geClip.x1 = priv->Pta.x;
	   	 geClip.y1 = priv->Pta.y;
	   	 geClip.x2 = priv->Ptb.x;
	  	 geClip.y2 = priv->Ptb.y;
		}
	  }
	break;

#endif

    case GEBOUNDS:
        if (ASegP && ASegP->Visible && priv)

	  {geVn = 0;
	   GEVEC(GEADDVERT, ASegP); 

	   if (geVn > 2)
	     {geVert[geVn++] = geVert[0];
	      if ((TRegion = geXPolygonRegion(geVert, geVn, EvenOddRule)))
		{XClipBox(TRegion, &TRect);
		 if (TRect.width > 0 && TRect.height > 0)
		   {ASegP->Co.x1 = GETSUI(TRect.x);
		    ASegP->Co.y1 = GETSUI(TRect.y);
		    ASegP->Co.x2 = ASegP->Co.x1 + GETSUI(TRect.width);
		    ASegP->Co.y2 = ASegP->Co.y1 + GETSUI(TRect.height);
		   }
		 else
		   geArcBounds(priv->Pta.x, priv->Pta.y,
			       priv->Ptm.x, priv->Ptm.y,
			       priv->Ptb.x, priv->Ptb.y,
			       priv->C.x, priv->C.y, priv->Rad,
			       priv->LinW, &ASegP->Co.x1, &ASegP->Co.y1,
			       &ASegP->Co.x2, &ASegP->Co.y2);
	  	 XDestroyRegion(TRegion);
		}
	      else
	  	 geArcBounds(priv->Pta.x, priv->Pta.y,
		       priv->Ptm.x, priv->Ptm.y,
		       priv->Ptb.x, priv->Ptb.y,
		       priv->C.x, priv->C.y, priv->Rad,
		       priv->LinW, &ASegP->Co.x1, &ASegP->Co.y1,
		       &ASegP->Co.x2, &ASegP->Co.y2);
	     }
	   else
	     {ASegP->Co.x1 = min(priv->Pta.x, priv->Ptb.x);
	      ASegP->Co.y1 = min(priv->Pta.y, priv->Ptb.y);
	      ASegP->Co.x2 = max(priv->Pta.x, priv->Ptb.x);
	      ASegP->Co.y2 = max(priv->Pta.y, priv->Ptb.y);
	     }

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
	    geFT      = (float)(priv->Beta) / 64.0;
	   }
	break;

    case GEINQPTS:
        if (ASegP && ASegP->Visible && priv)
	   {geClip.x1 = priv->Pta.x;
	    geClip.y1 = priv->Pta.y;
	    geClip.x2 = priv->Ptb.x;
	    geClip.y2 = priv->Ptb.y;
	   }
	break;

#ifdef GERAGS

    case GEROUND:
        if (ASegP && ASegP->Visible && priv)
	  {priv->Pta.x   = GETSUI(GETPIX(priv->Pta.x));
	   priv->Pta.y   = GETSUI(GETPIX(priv->Pta.y));
	   priv->Ptm.x   = GETSUI(GETPIX(priv->Ptm.x));
	   priv->Ptm.y   = GETSUI(GETPIX(priv->Ptm.y));
	   priv->Ptb.x   = GETSUI(GETPIX(priv->Ptb.x));
	   priv->Ptb.y   = GETSUI(GETPIX(priv->Ptb.y));
	   priv->LinW    = GETSUI(GETPIX(priv->LinW));
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
	    if (geGetPutFlgs & GEGETPUTCAP)
	    	geSampleAttr.CapStyle = priv->CapStyle;
	    if (geGetPutFlgs & GEGETPUTJOIN)
	    	geSampleAttr.JoinStyle = priv->JoinStyle;
	    if (geGetPutFlgs & GEGETPUTLINP)
	        {geSampleAttr.LinP 	= priv->Pat.LinP;
	    	 geSampleAttr.DashLen   = priv->Pat.DashLen;
	    	 geSampleAttr.DashIndx  = priv->Pat.DashIndx;
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
	    if (geGetPutFlgs & GEGETPUTCAP)
	    	priv->CapStyle = geSampleAttr.CapStyle;
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

    case GESELPTINFILL:
	geSegSel = ASegP;
	if (ASegP && ASegP->Visible && ASegP->FillStyle != GETRANSPARENT &&
	    !geState.EditPts)
	  {if (ASegP->Co.x1 > geSel.x2 || ASegP->Co.y1 > geSel.y2 ||
	       ASegP->Co.x2 < geSel.x1 || ASegP->Co.y2 < geSel.y1)
		 break;

	   /*
	    * This must be a filled object, so if the selection point is INSIDE
	    * of it, then PICK it, set geState.SelRadC = 0.  The test for being
	    * INSIDE is conducted by requesting the object to establish its
	    * vertex list (walk along its perimeter in small increments) and
	    * then call XPointInRegion.
	    */
	   geVn = 0;
	   GEVEC(GEADDVERT, ASegP); 
	   geVert[geVn++] = geVert[0];
	   if ((TRegion = geXPolygonRegion(geVert, geVn, EvenOddRule)))
		{x  = (geSel.x1 + geSel.x2) >> 1;
	  	 y  = (geSel.y1 + geSel.y2) >> 1;
	  	 ix = GETPIX(x);
	  	 iy = GETPIX(y);
	  	 if (XPointInRegion(TRegion, ix, iy))
		    {geState.SelRadC = 0;	/* This is IT		  */
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

	   if (geGenPtSelArc(priv))
	  	GEVEC(GETESTPT, ASegP);
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
	      if ((priv->Pta.x > geSel.x1 && priv->Pta.y > geSel.y1 &&
		   priv->Pta.x < geSel.x2 && priv->Pta.y < geSel.y2) ||
		  (priv->Ptm.x > geSel.x1 && priv->Ptm.y > geSel.y1 &&
		   priv->Ptm.x < geSel.x2 && priv->Ptm.y < geSel.y2) ||
		  (priv->Ptb.x > geSel.x1 && priv->Ptb.y > geSel.y1 &&
		   priv->Ptb.x < geSel.x2 && priv->Ptb.y < geSel.y2))
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

    case GEINQXVISIBLE:                        /* Are there HIDEN segs in grf*/
	if (ASegP && ASegP->Live && !ASegP->Visible)
	   geState.Hidden = TRUE; 
	break;

    case GEINQCOL:
        if (ASegP && ASegP->Visible)
	  {if (ASegP->Col.RGB.pixel)   geAttr.Col = ASegP->Col;
	   else
	   if (priv)               geAttr.Col = priv->Col;
	  }
	break;

    case GEINQLINCOL:
        if (ASegP && ASegP->Visible && priv)  geAttr.Col = priv->Col;
	break;

    case GEINQFILCOL:
        if (ASegP && ASegP->Visible) geAttr.Col = ASegP->Col;
	break;

    case GEFLIP:
	if (ASegP && ASegP->Visible && priv)
	  {geGenFlpPt(geState.Constraint, &priv->Pta.x, &priv->Pta.y);
	   geGenFlpPt(geState.Constraint, &priv->Ptb.x, &priv->Ptb.y);
	   geGenFlpPt(geState.Constraint, &priv->Ptm.x, &priv->Ptm.y);
	   geGenRC(priv->Pta.x,priv->Pta.y, priv->Ptm.x,priv->Ptm.y,
		   priv->Ptb.x,priv->Ptb.y, &priv->C.x,&priv->C.y, &priv->Rad,
		   &priv->Alpha, &priv->Beta);
	  }
	break;

#endif

    case GEGETSEG_COL:
        if (ASegP && ASegP->Col.RGB.pixel == geGenCol.pixel ||
	    (priv && priv->Col.RGB.pixel == geGenCol.pixel))
	  geState.ASegP = ASegP;
	break;

    case GESCALE:
    case GEMAG:
    case GEMAGRESD:
        if (ASegP && ASegP->Visible && priv)
	  {if(cmd == GESCALE)
		{if (!geState.EditPts || priv->EditPt1)
		   geGenScalePt(&priv->Pta);
		 if (!geState.EditPts || priv->EditPt2)
		   geGenScalePt(&priv->Ptm);
		 if (!geState.EditPts || priv->EditPt3)
		   geGenScalePt(&priv->Ptb);
		}
	   else
		{if (!geMagGrp)
		   {geMagGrp = TRUE;
		    geMagCx  = (priv->Pta.x + priv->Ptb.x) >> 1;
		    geMagCy  = (priv->Pta.y + priv->Ptb.y) >> 1;
		   }
		 if (!geState.EditPts || priv->EditPt1)
		   geGenMagPt(&priv->Pta);
		 if (!geState.EditPts || priv->EditPt2)
		   geGenMagPt(&priv->Ptm);
		 if (!geState.EditPts || priv->EditPt3)
		   geGenMagPt(&priv->Ptb);
		 if (geState.ZoomLineThick)
		   priv->LinW = ((priv->LinW ? priv->LinW : GESUI1) *
				 geMagFX) / 100;
		}
	   geGenRC(priv->Pta.x,priv->Pta.y, priv->Ptm.x,priv->Ptm.y,
		   priv->Ptb.x,priv->Ptb.y, &priv->C.x,&priv->C.y, &priv->Rad,
		   &priv->Alpha, &priv->Beta);
	  }
	break;


    case GEROTX:
        if (ASegP && ASegP->Visible && priv)
	  {if (!geState.EditPts || priv->EditPt1)
	     geGenRotX(&priv->Pta.x, &priv->Pta.y);
	   if (!geState.EditPts || priv->EditPt3)
	     geGenRotX(&priv->Ptb.x, &priv->Ptb.y);
	   if (!geState.EditPts || priv->EditPt2)
	     geGenRotX(&priv->Ptm.x, &priv->Ptm.y);
	   geGenRC(priv->Pta.x,priv->Pta.y, priv->Ptm.x,priv->Ptm.y,
		   priv->Ptb.x,priv->Ptb.y, &priv->C.x,&priv->C.y, &priv->Rad,
		   &priv->Alpha, &priv->Beta);
	  }
	break;

    case GEROTFIXED:
        if (ASegP && ASegP->Visible && priv)
	  {if (( geRot.Clockwise && geRot.Beta  == 90.) ||
	       (!geRot.Clockwise && geRot.Alpha == 90.))
	     {if (!geState.EditPts || priv->EditPt1)
		geGenRot90(&priv->Pta.x, &priv->Pta.y);
	      if (!geState.EditPts || priv->EditPt3)
		geGenRot90(&priv->Ptb.x, &priv->Ptb.y);
	      if (!geState.EditPts || priv->EditPt2)
		geGenRot90(&priv->Ptm.x, &priv->Ptm.y);
	      geGenRC(priv->Pta.x,priv->Pta.y, priv->Ptm.x,priv->Ptm.y,
		      priv->Ptb.x,priv->Ptb.y, &priv->C.x,&priv->C.y,
		      &priv->Rad, &priv->Alpha, &priv->Beta);
	     }
	   else
	     {if (!geState.EditPts || priv->EditPt1)
		geGenRotX(&priv->Pta.x, &priv->Pta.y);
	      if (!geState.EditPts || priv->EditPt3)
		geGenRotX(&priv->Ptb.x, &priv->Ptb.y);
	      if (!geState.EditPts || priv->EditPt2)
		geGenRotX(&priv->Ptm.x, &priv->Ptm.y);
	      geGenRC(priv->Pta.x,priv->Pta.y, priv->Ptm.x,priv->Ptm.y,
		      priv->Ptb.x,priv->Ptb.y, &priv->C.x,&priv->C.y,
		      &priv->Rad, &priv->Alpha, &priv->Beta);
	     }
	  }
	break;

    case GEEQUATE:
    case GEEQUATE_NOIMG:
        if(ASegP && geState.ASegP && ASegP->Handler == geState.ASegP->Handler
	   && priv && (privnu = (struct GE_ARC *)geState.ASegP->Private))
	  {geState.ASegP->Co = ASegP->Co;
	   geGenCopyAnim(geState.ASegP, ASegP);
	   geGenCopyDesc(geState.ASegP, ASegP);
	   privnu->Pta       = priv->Pta;
	   privnu->Ptb       = priv->Ptb;
	   privnu->Ptm       = priv->Ptm;
	   privnu->C         = priv->C;
	   privnu->Rad       = priv->Rad;
	   privnu->LinW      = priv->LinW;
	   privnu->Alpha     = priv->Alpha;
	   privnu->Beta      = priv->Beta;
	  }
	break;

    case GEADDVERT:
	/*
	 * Add the coords of this object to the vertex list - used to display
	 * the filled interior of polygons and for selection.
	 */
	if (ASegP && ASegP->Visible && priv && geVn < GEMAX_PTS)
	  {geGenVertArc(priv->Alpha, priv->Beta, &priv->Pta, &priv->Ptb,
			priv->Rad, &priv->C);
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
	  geFree(&ASegP->Private, sizeof(struct GE_ARC));
	break;

    case GEWRITE:
    case GEREAD:
	geArcIO(cmd, ASegP);
	break;

    default:
	break;
   }
}

geGenPtSelArc(priv)
struct GE_ARC   *priv;

{
int	angle, alpha, beta, omega;
long	x, y;
float	d;

/*
 * If the distance from the center of the circle to the selection point
 * (taking into consideration the thickness of the circle wall) is close
 * to the length of the radius, then the selection point is near the circle
 * outline.
 */
x = (long)(priv->C.x * GEFSUIPP);
y = (long)(priv->C.y * GEFSUIPP);
d = (long)sqrt((double)(x - geSel.x1) *
	       (double)(x - geSel.x1) +
	       (double)(y - geSel.y1) *
	       (double)(y - geSel.y1));

d /= GEFSUIPP;
/* d = fabs(d - priv->Rad) - (float)((GETPIX(priv->LinW)) >> 1); */
d = fabs(d - priv->Rad);
d -= (float)((GETPIX(priv->LinW)) >> 1);
geState.SelRadC = d;
if ((long)d > GESELRAD) return(FALSE);

/*
 * It's been established that the selection point is "near" the circle of
 * which the arc is a section.  Now check to see if selection point is within
 * the arc length
 */

x      = (geSel.x1 + geSel.x2) >> 1;
y      = (geSel.y1 + geSel.y2) >> 1;
angle  = (int)(geGenAngle (x, y, priv->C.x, priv->C.y, priv->Rad) * 64.);
alpha  = priv->Alpha;
beta   = priv->Beta;
if (beta < 0)				/* Adjust start & end angles for ccwise*/
   {alpha = alpha + beta;
    if (alpha < 0) alpha += (360 << 6);
    beta  = -beta;
   }
omega = alpha + beta;

if (angle >= alpha && angle <= omega)
	return(TRUE);
else
   {if (angle <= (omega - (360 << 6)))
	return(TRUE);
    else
	{geState.SelRadC = GEMAXSHORT;
   	 return(FALSE);
   	}
   }
}

geGenVertArc(Alpha, Beta, Pta, Ptb, Rad, C)
int		Alpha, Beta;
struct GE_PT	*Pta, *Ptb;
float		Rad;
struct GE_FPT	*C;
{
register int	i, ix, iy, theta, inc;
int		startangle, endangle, arclen;

float		ftx, fty;

if (!geVn || geVert[geVn - 1].x != GETPIX(Pta->x) ||
    geVert[geVn - 1].y != GETPIX(Pta->y))
   {GESTOREVERT(geVn, GETPIX(Pta->x), GETPIX(Pta->y));
    geVn++;
   }

if (Rad)
   {startangle = Alpha >> 6;
    endangle   = (Alpha + Beta) >> 6;
    arclen     = abs(endangle - startangle);

    if (arclen < 2)
      {if (!geVn || geVert[geVn - 1].x != GETPIX(Ptb->x) ||
	   geVert[geVn - 1].y != GETPIX(Ptb->y))
	 {GESTOREVERT(geVn, GETPIX(Ptb->x), GETPIX(Ptb->y));
	  geVn++;
	 }
       return;
      }

    while (arclen > 360) arclen -= 360;
    if (arclen <= 45)  inc = 1;
    else
    if (arclen <= 90)  inc = 2;
    else
    if (arclen <= 180) inc = 4;
    else               inc = 8;
    theta      = startangle;

    if (endangle > startangle)
	{for (i = startangle + inc; i < endangle; i += inc)
	   {ftx       = C->x + Rad * geCos[theta];
	    GEINT(ftx, ix);
	    fty        = C->y - Rad * geSin[theta];
	    GEINT(fty, iy);
	    theta += inc;
	    if (theta > 360) theta -= 360;
	    if (!geVn || geVert[geVn - 1].x != ix ||
    		 geVert[geVn - 1].y != iy)
   		{GESTOREVERT(geVn, ix, iy);
   		 geVn++;
   		}
	   }
	}
    else
	{for (i = startangle - inc; i > endangle; i -= inc)
	   {ftx       = C->x + Rad * geCos[theta];
	    GEINT(ftx, ix);
	    fty       = C->y - Rad * geSin[theta];
	    GEINT(fty, iy);
	    theta -= inc;
	    if (theta < 0) theta += 360;
	    if (!geVn || geVert[geVn - 1].x != ix ||
    		 geVert[geVn - 1].y != iy)
   		{GESTOREVERT(geVn, ix, iy);
   		 geVn++;
   		}
	   }
	}
   }

/*
 * Make sure end point is hit
 */
if (!geVn || geVert[geVn - 1].x != GETPIX(Ptb->x) ||
    geVert[geVn - 1].y != GETPIX(Ptb->y))
  {GESTOREVERT(geVn, GETPIX(Ptb->x), GETPIX(Ptb->y));
   geVn++;
  }

}

