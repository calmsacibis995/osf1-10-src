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
static char SccsId[] = "@(#)gecir.c	1.13\t10/16/89";
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
**	GECIR	             	       Circle object handler
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
**	GNE 02/03/87 Created
**
**--
**/

#include "geGks.h"
#include <math.h>

extern unsigned long geAllocColor();
extern char          *geMalloc();
extern double	     atof();	

geCir(cmd, ASegP)
long     	cmd;
struct GE_SEG	*ASegP;
{                       
#define         GEOBJLIS 3

float    	FList[GEOBJLIS], FCon, FLprev, ftemp,
                FOCon, FICon, Rad;
char            *error_string, *string, buff[15];
int		Hlw, PixLinW, temp, XSav, YSav, NumFrames;
long		x, y, dx, dy, delta;

struct GE_CIR   *priv, *privnu;
struct GE_PT	Pta;
struct GE_PT    Ptb;
struct GE_FPT	C;

if (ASegP)
  {if (!ASegP->Live && cmd != GEKILL    && cmd != GERELPRIV &&
                       cmd != GEKILPRIV && cmd != GEGETSEG_COL &&
		       cmd != GEXDEL)
     return;
   priv = (struct GE_CIR *)ASegP->Private;
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
	     GE_SET_BG_DISP_ATTR;
	     GE_SET_FG_DISP_ATTR;
	     geDrawCir(geState.Drawable, geGC2,
		       GETPIX(priv->Cx), GETPIX(priv->Cy), GETPIX(priv->Rad),
		       GETPIX(priv->LinW), priv->Pat.LinP, geDispAttr.FgPixel,
		       geDispAttr.FgStyle, geDispAttr.FgHT, priv->WritingMode,
		       geDispAttr.BgStyle, geDispAttr.BgPixel, geDispAttr.BgHT,
		       ASegP->FillWritingMode);

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
	   if (geState.AnimDispMode == GEANIM_TRUE)
	     geDrawCir(geState.Drawable, geGC2,
		       GETPIX(priv->Cx), GETPIX(priv->Cy), GETPIX(priv->Rad),
		       GETPIX(priv->LinW), priv->Pat.LinP,
		       GEXORPIXEL,
		       priv->LineStyle, priv->LineHT, GXxor,
		       GETRANSPARENT, 0, 0, 0);
	   else
	     geDrawCir(geState.Drawable, geGC5,
		       GETPIX(priv->Cx), GETPIX(priv->Cy), GETPIX(priv->Rad),
		       0,0,0,0,0,0,0,0,0,0);
	   if (geAnimMin && priv->Rad > (geAnimMin >> 1))
	    {XDrawLine(geDispDev, geState.Drawable, geGC5, 
		       GETPIX(priv->Cx - priv->Rad), GETPIX(priv->Cy),
		       GETPIX(priv->Cx + priv->Rad), GETPIX(priv->Cy));
	     XDrawLine(geDispDev, geState.Drawable, geGC5,
		       GETPIX(priv->Cx), GETPIX(priv->Cy - priv->Rad),
		       GETPIX(priv->Cx), GETPIX(priv->Cy + priv->Rad));
	    }
	  }
	break;

    case GEHDISP:
    case GEHXDISP:
	if (priv && ASegP && ASegP->Visible && ASegP->InFrame)
	   {if (geState.HilightMode == GEHILIGHT_BOX)
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
	 	 geDrawCir(geState.Drawable, geGC2,
		    GETPIX(priv->Cx), GETPIX(priv->Cy), GETPIX(priv->Rad),
		    GETPIX(priv->LinW), priv->Pat.LinP,
		    GEXORPIXEL,
		    FillSolid, priv->LineHT, GXxor,
		    GETRANSPARENT, 0, 0, 0);
		}
	 }
	break;

    case GECREATE:
	geSegCr();
	geState.ASegP->Private    = geMalloc(sizeof(struct GE_CIR));
	priv 		          = (struct GE_CIR *)geState.ASegP->Private;
	geState.ASegP->Handle[0]  = 'C';
	geState.ASegP->Handle[1]  = 'I';
	geState.ASegP->Handle[2]  = 'R';
	geState.ASegP->Handle[3]  = '\0';
	geState.ASegP->Handler    = geCir;
	
	if(geState.Grav.Align || geState.APagP->Grid.XAlign ||
	   geState.APagP->Grid.YAlign)
	  {geState.Dx = geState.Dy   = 0;
	   geState.Grav.OPt.x = geState.Mouse.x;
	   geState.Grav.OPt.y = geState.Mouse.y;
	   geGenAln(geState.Mouse.x, geState.Mouse.y);
	   geState.Mouse.x += geState.Dx;
	   geState.Mouse.y += geState.Dy;
	   geState.Grav.OPt.x = geState.Mouse.x;
	   geState.Grav.OPt.y = geState.Mouse.y;
	  }
	priv->Cx                  = geU.XR = geState.Mouse.x;
	priv->Cy                  = geU.YR = geState.Mouse.y;
	geMouseXY(geState.Window);
	priv->Rad		  = max(abs(priv->Cx - geState.Mouse.x),
					abs(priv->Cy - geState.Mouse.y));
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
	GEVEC(GEBOUNDS, geState.ASegP);
	geMnStat(GEDISP, 47);
	geAnim(cmd, geState.ASegP);
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
	    (priv = (struct GE_CIR *)ASegP->Private))
	  {GE_SET_ATTR_ERASE;
	  }
	break;

    case GESETATTRERASE_WHITE:
	if (ASegP && ASegP->Visible && ASegP->WhiteOut && !geState.PrintPgBg &&
	    (priv = (struct GE_CIR *)ASegP->Private))
	  {GE_SET_ATTR_ERASE_WHITE;
	  }
	break;

    case GEOBJCREATE:
	if (!ASegP || !ASegP->Visible) break;

	if (geU.DMap != -1) FCon = geU.Con[geU.DMap];
	else                FCon = 1.0;
	FOCon      = FCon / GEFSUIPP;
	FICon      = GEFSUIPP / FCon;

	/* Set up default values for circle object and fetch the create
	 * to size modal box.  Only use default values if in create 
	 * mode, otherwise use the current values (editing).
	 */
 	if (geState.InCreateMode)
	   priv->Rad  = GESUI1 * 12;

	FList[1]   = (float)priv->Rad  * FOCon * 2.;
	FList[2]   = (float)priv->LinW * FOCon;

	if (!geUI.WidgetArray[GEWCREATE_CIR] || 
	    !XtIsManaged(geUI.WidgetArray[GEWCREATE_CIR]))
	  {if (MrmFetchWidget(geUI.Id, GECREATE_CIR_POPUP,
			      XtParent(geState.APagP->WidgetId),
			      &geUI.WidgetArray[GEWCREATE_CIR], 
			      &geUI.Class) != MrmSUCCESS)
	     {error_string = (char *) geFetchLiteral("GE_ERR_NOWIDGET", MrmRtypeChar8);
	      if (error_string != NULL) 
	        {sprintf(geErrBuf, error_string, GECREATE_CIR_POPUP);
	         geError(geErrBuf, TRUE);
	         XtFree(error_string);
	        }
	     }
	  }
        /* Set all the fields before managing box. 
 	 */
	sprintf(buff, "%.3f", FList[1]);		/* Diameter */
 	XmTextSetString(geUI.WidgetArray[GEWCREATE_RAD], buff);
	sprintf(buff, "%.3f", FList[2]);		/* Linw */
 	XmTextSetString(geUI.WidgetArray[GEWCREATE_LINW], buff);

	XtManageChild(geUI.WidgetArray[GEWCREATE_CIR]);

	if (geState.InCreateMode)
  	  { GEVEC(GEXDISP, ASegP); } 			/* XOR the object */
        break;

    case GEOBJEDIT:
    case GEEDIT:
	if (!ASegP || !ASegP->Visible) break;

	if (!geUI.WidgetArray[GEWCREATE_CIR])
	  { GEVEC(GEOBJCREATE, ASegP);
	    break;
	  }

	if (geU.DMap != -1) FCon = geU.Con[geU.DMap];
	else                FCon = 1.0;
	FOCon      = FCon / GEFSUIPP;
	FICon      = GEFSUIPP / FCon;

	/* Get all the fields before displaying the new object.
 	 */                                               /* linw */
 	string = XmTextGetString(geUI.WidgetArray[GEWCREATE_LINW]);
	ftemp = (float)atof(string);
	if (!geState.LinWPoints && geU.DMap >= 0)
	  ftemp /= geU.Con[geU.DMap];      
	ftemp *= GEFSUIPP;
	GEINT(ftemp, priv->LinW); 
	XtFree(string);
                                                         /* Diameter */
 	string = XmTextGetString(geUI.WidgetArray[GEWCREATE_RAD]);
	ftemp = (float)atof(string) * FICon / 2.;
	GEINT(ftemp, priv->Rad); 
	XtFree(string);
        break;
	
    case GECOPY:
    case GECOPYFLP:
	if (!ASegP || !ASegP->Visible) break;
	geSegCr();
	geState.ASegP->Private    = geMalloc(sizeof(struct GE_CIR));
	privnu 		          = (struct GE_CIR *)geState.ASegP->Private;
	geGenCopySeg(geState.ASegP, ASegP);
	*privnu                        = *priv;
	GEVEC(GEBOUNDS, geState.ASegP);
	break;

    case GEWARP2:
        if (ASegP && ASegP->Visible && priv)
	  {dx        = geState.Mouse.x - priv->Cx;
	   dy        = geState.Mouse.y - priv->Cy;
	   priv->Rad = (long)sqrt((double)dx * (double)dx +
				  (double)dy * (double)dy);
	  }
	break;

    case GECONSTRAIN:
        if (ASegP && ASegP->Visible && priv)
	  {if (XmToggleButtonGetState(geUI.WidgetArray[GEWHORZCONSTR]) &&
	       XmToggleButtonGetState(geUI.WidgetArray[GEWVERTCONSTR]))
		{if (priv->LinW)
		   {if (geState.ZoomLineThick)
			priv->Rad = priv->LinW >> 1;
		    else
			priv->Rad = (priv->LinW >> 1) *
				    geState.APagP->Zoom.ZoomF / 100.;
		   }
		 else
		    priv->Rad = GEFSUIPP * geState.APagP->Zoom.ZoomF / 200.;

		 if (ASegP->FillStyle == GETRANSPARENT)
		    {ASegP->FillStyle = FillSolid;
		     ASegP->FillHT    = 100;
		     ASegP->Col.RGB.pixel = geBlack.pixel;
		    }
		 priv->LineStyle = GETRANSPARENT;
	  	 geState.Event.type = GE_Bd;
	  	 geState.Event.xbutton.button = GEBUT_L;
	  	 XPutBackEvent(geDispDev, &geState.Event);
		}
	   else
		 GEVEC(GEWARP2, ASegP);
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
	      geGenAln(priv->Cx, priv->Cy);
	      if (!geState.Grav.Lock)
		{geGenAln(priv->Cx - priv->Rad, priv->Cy);
		 if (!geState.Grav.Lock)
		   {geGenAln(priv->Cx, priv->Cy - priv->Rad);
		    if (!geState.Grav.Lock)
		      {geGenAln(priv->Cx + priv->Rad, priv->Cy);
		       if (!geState.Grav.Lock)
			 geGenAln(priv->Cx, priv->Cy + priv->Rad);
		      }
		   }
	        }
	     }
	   if (cmd != GEGRAVTST && (geState.Dx || geState.Dy))
	     {priv->Cx += geState.Dx;
	      priv->Cy += geState.Dy;
	     }
	  }
	break;

#ifdef GERAGS

    case GEGRAV:
        if (ASegP && ASegP->Visible && priv &&
	    (geState.Grav.Pt.x >= (ASegP->Co.x1 - geState.Grav.Rad) &&
	     geState.Grav.Pt.y >= (ASegP->Co.y1 - geState.Grav.Rad) &&
	     geState.Grav.Pt.x <= (ASegP->Co.x2 + geState.Grav.Rad) &&
	     geState.Grav.Pt.y <= (ASegP->Co.y2 + geState.Grav.Rad)))
	  {geGenGrav(GEGRAVLOCK, priv->Cx, priv->Cy);
	   geGenGrav(GEGRAVLOCK, (priv->Cx - priv->Rad), priv->Cy);
	   geGenGrav(GEGRAVLOCK, (priv->Cx + priv->Rad), priv->Cy);
	   geGenGrav(GEGRAVLOCK, priv->Cx, (priv->Cy - priv->Rad));
	   geGenGrav(GEGRAVLOCK, priv->Cx, (priv->Cy + priv->Rad));
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
	  {geClip.x1 = priv->Cx - priv->Rad;
	   geClip.y1 = priv->Cy - priv->Rad;
	   geClip.x2 = priv->Cx + priv->Rad;
	   geClip.y2 = priv->Cy + priv->Rad;
	  }
	break;

#endif

    case GEBOUNDS:
        if (ASegP && ASegP->Visible && priv)
	  {ASegP->Co.x1 = priv->Cx - priv->Rad - (Hlw = priv->LinW >> 1) -
	                  GESUI1;
	   ASegP->Co.y1 = priv->Cy - priv->Rad - Hlw;
	   if (Hlw)
	     {Hlw = (priv->LinW - GESUI1) >> 1;}
	   else	    Hlw = 0;
	   ASegP->Co.x2 = priv->Cx + priv->Rad + Hlw;
	   ASegP->Co.y2 = priv->Cy + priv->Rad + Hlw;
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
	  {priv->Cx   = GETSUI(GETPIX(priv->Cx));
	   priv->Cy   = GETSUI(GETPIX(priv->Cy));
	   priv->Rad  = GETSUI(GETPIX(priv->Rad));
	   priv->LinW = GETSUI(GETPIX(priv->LinW));
	  }
	break;

    case GEALN:
	geGenAlign(ASegP);
	break;

    case GEALNREF:
        if (ASegP && ASegP->Visible && priv)
	  {GEVEC(GEEXTENT0, ASegP);
	   geAln.Co    = geClip;
	   geAln.Pt.x  = priv->Cx;
	   geAln.Pt.y  = priv->Cy;
	  }
	break;

    case GEALNCH:
    case GEALNCV:
	if (ASegP && ASegP->Visible && priv)
	   {geState.Dx = geState.Dy = 0;
	    if (cmd == GEALNCV)
		geState.Dx = geAln.Pt.x - priv->Cx;
	    if (cmd == GEALNCH)
		geState.Dy = geAln.Pt.y - priv->Cy;
	   }
	break;

    case GESELPTINFILL:
	geSegSel = ASegP;
	if (ASegP && ASegP->Visible)
	  {if (ASegP->FillStyle == GETRANSPARENT ||
	       (ASegP->Co.x1 > geSel.x2 || ASegP->Co.y1 > geSel.y2 ||
	        ASegP->Co.x2 < geSel.x1 || ASegP->Co.y2 < geSel.y1))
		 break;

	   /*
	    * This must be a filled object, so if the selection point is INSIDE
	    * of it, then PICK it, set geState.SelRadC = 0.
	    */
	   delta = (long)sqrt((double)(priv->Cx - geSel.x1) *
	       	   	      (double)(priv->Cx - geSel.x1) +
	       		      (double)(priv->Cy - geSel.y1) *
	       		      (double)(priv->Cy - geSel.y1));

	   if (delta <= (priv->Rad + (priv->LinW >> 1)))
	   	 geState.SelRadC = 0;			/* This is IT	          */
	   else
	   	 geState.SelRadC = GEMAXSHORT;
	  }	
	break;

    case GESELPT:
	geSegSel = ASegP;
	if (ASegP && ASegP->Visible)
	  {if (ASegP->Co.x1 > geSel.x2 || ASegP->Co.y1 > geSel.y2 ||
	       ASegP->Co.x2 < geSel.x1 || ASegP->Co.y2 < geSel.y1) break;

	   geGenPtSelCir(priv);
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
	   delta = (long)sqrt((double)(priv->Cx - geSel.x1) *
	       	   	      (double)(priv->Cx - geSel.x1) +
	       		      (double)(priv->Cy - geSel.y1) *
	       		      (double)(priv->Cy - geSel.y1));

	   if (delta <= (priv->Rad + (priv->LinW >> 1)))
	   	 geGenPtSelCir(priv);
	   else
		 geState.SelRadC = GEMAXSHORT;
	  }	
	break;

    case GESELBX:
	if (ASegP && ASegP->Visible)
	  {if (ASegP->Co.x1 > geSel.x1 && ASegP->Co.y1 > geSel.y1 &&
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
	    if (geGetPutFlgs & GEGETPUTLINP)
	        {geSampleAttr.LinP     = priv->Pat.LinP;
	    	 geSampleAttr.DashLen  = priv->Pat.DashLen;
	    	 geSampleAttr.DashIndx = priv->Pat.DashIndx;
	    	 geDashList[3][0] = priv->Pat.DashList[0];   /* user defined */
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
	    if (geGetPutFlgs & GEGETPUTLINP)
	        {priv->Pat.LinP        = geSampleAttr.LinP;
	    	 priv->Pat.DashLen     = geSampleAttr.DashLen;
	    	 priv->Pat.DashIndx    = geSampleAttr.DashIndx;
	    	 priv->Pat.DashList[0] = geDashList[3][0];   /* user defined */
	    	 priv->Pat.DashList[1] = geDashList[3][1];
	    	 priv->Pat.DashList[2] = geDashList[3][2];
	    	 priv->Pat.DashList[3] = geDashList[3][3];
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

    case GEINQXVISIBLE:                        /* Are there HIDEN segs in grf*/
	if (ASegP && ASegP->Live && !ASegP->Visible)
	   geState.Hidden = TRUE; 
	break;

    case GEINQCOL:
	if (ASegP && ASegP->Visible && priv)
	  {if (ASegP->Col.RGB.pixel)   geAttr.Col = ASegP->Col;
	   else
	   if (priv)               geAttr.Col = priv->Col;
	  }
	break;

    case GEINQLINCOL:
	if (ASegP && ASegP->Visible && priv)
	  geAttr.Col = priv->Col;
	break;

    case GEINQFILCOL:
	if (ASegP && ASegP->Visible && priv)
	  geAttr.Col = ASegP->Col;
	break;

    case GEGETSEG_COL:
        if (ASegP && ASegP->Col.RGB.pixel == geGenCol.pixel ||
	    (priv && priv->Col.RGB.pixel == geGenCol.pixel))
	  geState.ASegP = ASegP;
	break;

    case GESCALE:
	if (ASegP && ASegP->Visible && priv)
	  {struct GE_CO co;
	   float  d, dx, dy;

	   co.x1     = priv->Cx;
	   co.y1     = priv->Cy;
	   dx        = geEditBox.Pt.x - priv->Cx;
	   dy        = geEditBox.Pt.y - priv->Cy;
	   if (dx || dy)
	     {d         = (float)sqrt((double)(dx * dx + dy * dy));
	      co.x2     = priv->Cx + ((float)priv->Rad * dx) / d;
	      co.y2     = priv->Cy + ((float)priv->Rad * dy) / d;
	     }
	   else
	     {co.x2     = priv->Cx + priv->Rad;
	      co.y2     = priv->Cy + priv->Rad;
	     }

	   geGenScale(&co);
	   priv->Cx  = co.x1;
	   priv->Cy  = co.y1;
	   priv->Rad = (long)sqrt((double)(co.x2 - co.x1) *
				  (double)(co.x2 - co.x1) +
				  (double)(co.y2 - co.y1) *
				  (double)(co.y2 - co.y1));
	  }
	break;

    case GEFLIP:
	if (ASegP && ASegP->Visible && priv)
	  geGenFlpPt(geState.Constraint, &priv->Cx, &priv->Cy);
	break;

#endif

    case GEMAG:
    case GEMAGRESD:
	if (ASegP && ASegP->Visible && priv)
	   {priv->Rad = (priv->Rad * (float)geMagFX) / 100.0;
            if (geState.ZoomLineThick)
              priv->LinW = ((priv->LinW ? priv->LinW : GESUI1) * geMagFX) / 100;
	    if (geMagGrp)
		{if (geMagMode == GEHORIZONTAL || geMagMode == GENOCONSTRAINT)
		    {priv->Cx -= geMagCx;
		     priv->Cx  = (priv->Cx * geMagFX) / 100;
		     priv->Cx += geMagCx;
		    }
		 if (geMagMode == GENOCONSTRAINT || geMagMode == GEVERTICAL)
		    {priv->Cy -= geMagCy;
		     priv->Cy  =  (priv->Cy * geMagFY) / 100;
		     priv->Cy += geMagCy;
		    }
		}
	   }
	break;

    case GEROTX:
	if (ASegP && ASegP->Visible && priv)
	  geGenRotX(&priv->Cx, &priv->Cy);
	break;

    case GEROTFIXED:
	if (ASegP && ASegP->Visible && priv)
	  {if (( geRot.Clockwise && geRot.Beta  == 90.) ||
	       (!geRot.Clockwise && geRot.Alpha == 90.))
	  	geGenRot90(&priv->Cx, &priv->Cy);
	   else geGenRotX (&priv->Cx, &priv->Cy);
	  }
	break;

    case GEEQUATE:
    case GEEQUATE_NOIMG:
        if(ASegP && geState.ASegP && ASegP->Handler == geState.ASegP->Handler
	   && priv && (privnu = (struct GE_CIR *)geState.ASegP->Private))
	  {geState.ASegP->Co = ASegP->Co;
	   geGenCopyAnim(geState.ASegP, ASegP);
	   geGenCopyDesc(geState.ASegP, ASegP);
	   privnu->Cx        = priv->Cx;
	   privnu->Cy        = priv->Cy;
	   privnu->Rad       = priv->Rad;
	   privnu->LinW      = priv->LinW;
	  }
	break;

    case GEADDVERT:
	/*
	 * Add the coords of this object to the vertex list - used for
	 * selection.
	 */
	if (ASegP && ASegP->Visible && priv && geVn < GEMAX_PTS)
	   {Pta.x = priv->Cx - priv->Rad;
	    Pta.y = priv->Cy;
	    Ptb   = Pta;
	    C.x   = (float)(GETPIX(priv->Cx));
	    C.y   = (float)(GETPIX(priv->Cy));
	    Rad	  = GETPIX(priv->Rad);
	    geGenVertArc(0, (360 << 6), &Pta, &Ptb, Rad,
			&C);
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
	  geFree(&ASegP->Private, sizeof(struct GE_CIR));
	break;

    case GEREAD:
    case GEWRITE:
	geCirIO(cmd, ASegP);
	break;

    default:
	break;
   }
}

#ifdef GERAGS

geGenPtSelCir(priv)
struct GE_CIR   *priv;

{
long  d;

/*
 * If the distance from the center of the circle to the selection point
 * (taking into consideration the thickness of the circle wall) is close
 * to the length of the radius, then the selection point is near the circle
 * outline.
 */
d = (long)sqrt((double)(priv->Cx - geSel.x1) *
	       (double)(priv->Cx - geSel.x1) +
	       (double)(priv->Cy - geSel.y1) *
	       (double)(priv->Cy - geSel.y1));

d = abs(d - priv->Rad) - (priv->LinW >> 1);
geState.SelRadC = GETPIX(d);
return(TRUE);
/*
if (d <= GETSUI(GESELRAD)) return(TRUE);
else			   return(FALSE);
*/
}

#endif
