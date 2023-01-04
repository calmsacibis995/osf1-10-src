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
static char SccsId[] = "@(#)geelp.c	1.13\t10/16/89";
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
**	GEELP	             	       Ellipse handler
**
**  ABSTRACT:
**
**        The ellipse is defined by a center (Cx,Cy), the magnitudes of
**        the major and minor axes (A, B)
**	  and an angle Gamma, which is measured as a positive angle from
**	  3 o'clock, counter-clockwise to A.  A is defined as the axis which
**	  resides in the first quadrant.
**
**	  The Box coordinates are carried around so that during animation
**	  the major and minor axes do not have to be continually recomputed
**	  (at least not for MOVE).  The box coordinates follow the ellipse
**	  in the fashion indicated below.
**
**				      Bx3
**					 p
**					*
**			   Bx2  p      *
**				  *   *
**				     *
**				    *  *
**				   *     p  Bx4
**				  *
**				 *
**				p
**			     Bx1
**
**  ENVIRONMENT:
**
**	VAX/VMS, ULTRIX Operating Systems
**
**  MODIFICATION HISTORY:
**
**	GNE 02/17/87 Created
**
**--
**/

#include "geGks.h"
#include <math.h>


extern unsigned long 	geAllocColor();
extern char          	*geMalloc();
extern Region		geXPolygonRegion();
extern double	        atof();	

geElp(cmd, ASegP)
long     	cmd;
struct GE_SEG	*ASegP;
{                       
#define ELP_TOPIX \
   {priv->Bx.x1 = GETPIX(priv->Bx.x1); \
    priv->Bx.y1 = GETPIX(priv->Bx.y1); \
    priv->Bx.x2 = GETPIX(priv->Bx.x2); \
    priv->Bx.y2 = GETPIX(priv->Bx.y2); \
    priv->Bx.x3 = GETPIX(priv->Bx.x3); \
    priv->Bx.y3 = GETPIX(priv->Bx.y3); \
    priv->Bx.x4 = GETPIX(priv->Bx.x4); \
    priv->Bx.y4 = GETPIX(priv->Bx.y4); \
    priv->A     = GETPIX(priv->A); \
    priv->B     = GETPIX(priv->B); \
    priv->Cx    = (priv->Bx.x1 + priv->Bx.x3) >> 1; \
    priv->Cy    = (priv->Bx.y1 + priv->Bx.y3) >> 1; \
   }

#define ELP_TOSUI \
   {priv->Bx.x1 = GETSUI(priv->Bx.x1); \
    priv->Bx.y1 = GETSUI(priv->Bx.y1); \
    priv->Bx.x2 = GETSUI(priv->Bx.x2); \
    priv->Bx.y2 = GETSUI(priv->Bx.y2); \
    priv->Bx.x3 = GETSUI(priv->Bx.x3); \
    priv->Bx.y3 = GETSUI(priv->Bx.y3); \
    priv->Bx.x4 = GETSUI(priv->Bx.x4); \
    priv->Bx.y4 = GETSUI(priv->Bx.y4); \
    priv->A     = GETSUI(priv->A); \
    priv->B     = GETSUI(priv->B); \
    priv->Cx    = (priv->Bx.x1 + priv->Bx.x3) >> 1; \
    priv->Cy    = (priv->Bx.y1 + priv->Bx.y3) >> 1; \
   }

#define         GEOBJLIS 5

static long	dx, dy, temp;

float    	FList[GEOBJLIS], FCon, FLprev, ftemp,
                FOCon, FICon, ftsin, ftcos, RotSavA, RotSavB;
float		fx, fy, fdx, fdy, m13, m25, x1,y1, x2, y2, x3, y3, x4, y4,
		fdx1, fdy1, fdx2, fdy2, l13, l24;
char            *error_string, *string, buff[15];
int		Hlw, xa, ya, xb, yb, x, y, PixLinW, XSav, YSav, NumFrames,
		RotSavC;
long		lx, ly;

Region		TRegion;

struct GE_ELP   *priv, *privnu;

if (ASegP)
  {if (!ASegP->Live && cmd != GEKILL    && cmd != GERELPRIV &&
                       cmd != GEKILPRIV && cmd != GEGETSEG_COL &&
		       cmd != GEXDEL)
     return;
   priv = (struct GE_ELP *)ASegP->Private;
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
	     GEMODE(geGC3, GXcopy);
	     GE_SET_BG_DISP_ATTR;
	     GE_SET_FG_DISP_ATTR;
	     geDrawElp(geState.Drawable, geGC2,
		       (float)GETPIX(priv->Cx), (float)GETPIX(priv->Cy),
		       (float)GETPIX(priv->A), (float)GETPIX(priv->B),
		       priv->Gamma,
		       GETPIX(priv->LinW),  priv->Pat.LinP, geDispAttr.FgPixel,
		       geDispAttr.FgStyle, geDispAttr.FgHT, priv->WritingMode,
		       geDispAttr.BgStyle, geDispAttr.BgPixel, geDispAttr.BgHT,
		       ASegP->FillWritingMode, cmd);

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
	     geDrawElp(geState.Drawable, geGC2,
		    (float)GETPIX(priv->Cx), (float)GETPIX(priv->Cy),
		    (float)GETPIX(priv->A), (float)GETPIX(priv->B),
		       priv->Gamma,
		    GETPIX(priv->LinW), priv->Pat.LinP, GEXORPIXEL,
                    priv->LineStyle, priv->LineHT, GXxor,
		    GETRANSPARENT, 0,0,0, cmd);
	 else
	     geDrawElp(geState.Drawable, geGC5,
		       (float)GETPIX(priv->Cx), (float)GETPIX(priv->Cy),
		       (float)GETPIX(priv->A), (float)GETPIX(priv->B),
		       priv->Gamma, 0,0,0,0,0,0,0,0,0,0, cmd);
	  if (geAnimMin && (abs(priv->Bx.x2 - priv->Bx.x1) >
			    (geAnimMin >> 1) ||
			    abs(priv->Bx.y4 - priv->Bx.y1) >
			    (geAnimMin >> 1)))
	    {XDrawLine(geDispDev, geState.Drawable, geGC5,
		       GETPIX(priv->Bx.x1), GETPIX(priv->Bx.y1),
		       GETPIX(priv->Bx.x3), GETPIX(priv->Bx.y3));
	     XDrawLine(geDispDev, geState.Drawable, geGC5,
		       GETPIX(priv->Bx.x2), GETPIX(priv->Bx.y2),
		       GETPIX(priv->Bx.x4), GETPIX(priv->Bx.y4));
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
	 	 PixLinW = GETPIX(priv->LinW);
    	 	 GEMODE(geGC3, GXxor);
	 	 geDrawElp(geState.Drawable, geGC2,
		    (float)GETPIX(priv->Cx), (float)GETPIX(priv->Cy),
		    (float)GETPIX(priv->A), (float)GETPIX(priv->B),
		    priv->Gamma,
		    PixLinW, priv->Pat.LinP, GEXORPIXEL,
                    FillSolid, priv->LineHT, GXxor,
                    GETRANSPARENT, 0, 0, 0, cmd);
		}
	 }
	break;

    case GECREATE:
	geSegCr();
	geState.ASegP->Private    = geMalloc(sizeof(struct GE_ELP));
	priv 		          = (struct GE_ELP *)geState.ASegP->Private;
	geState.ASegP->Handle[0]  = 'E';
	geState.ASegP->Handle[1]  = 'L';
	geState.ASegP->Handle[2]  = 'P';
	geState.ASegP->Handle[3]  = '\0';
	geState.ASegP->Handler    = geElp;
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
	priv->Cx                  = geU.XR = geState.Mouse.x;
	priv->Cy                  = geU.YR = geState.Mouse.y;
	geMouseXY(geState.Window);
	priv->A		  	  = abs(priv->Cx - geState.Mouse.x);
	priv->B			  = abs(priv->Cy - geState.Mouse.y);
	priv->Gamma          	  = 0.;
/* creation at an angle test
priv->Gamma          	  = geRot.Alpha;
*/
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
	geMnStat(GEDISP, 58);
	geAnim  (cmd,    geState.ASegP);
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
	    (priv = (struct GE_ELP *)ASegP->Private))
	  {GE_SET_ATTR_ERASE;
	  }
	break;

    case GESETATTRERASE_WHITE:
	if (ASegP && ASegP->Visible && ASegP->WhiteOut && !geState.PrintPgBg &&
	    (priv = (struct GE_ELP *)ASegP->Private))
	  {GE_SET_ATTR_ERASE_WHITE;
	  }
	break;

    case GEOBJCREATE:
	if (!ASegP || !ASegP->Visible) break;

	if (geU.Map != -1) FCon = geU.Con[geU.Map];
	else               FCon = 1.0;
	FOCon      = FCon / GEFSUIPP;
	FICon      = GEFSUIPP / FCon;

	/* Set up default values for ellipse object and fetch the create
	 * to size modal box.  Only use default values if in create 
	 * mode, otherwise use the current values (editing).
	 */
 	if (geState.InCreateMode)
	  {priv->A    = GESUI1 * 23;
	   priv->B    = GESUI1 * 10;
	   FList[1]   = (float)priv->A     * FOCon;
	   FList[2]   = (float)priv->B     * FOCon;
           FList[3]   = priv->Gamma;
   	   FList[4]   = (float)priv->LinW  * FOCon;
          }
	else
	  {FList[1]   = (float)priv->A     * FOCon;
	   FList[2]   = (float)priv->B     * FOCon;
           FList[3]   = priv->Gamma;
   	   FList[4]   = (float)priv->LinW  * FOCon;
          }
	
	if (!geUI.WidgetArray[GEWCREATE_ELP] || 
	    !XtIsManaged(geUI.WidgetArray[GEWCREATE_ELP]))
	  {if (MrmFetchWidget(geUI.Id, GECREATE_ELP_POPUP,
			      XtParent(geState.APagP->WidgetId),
			      &geUI.WidgetArray[GEWCREATE_ELP], 
			      &geUI.Class) != MrmSUCCESS)
	     {error_string = (char *) geFetchLiteral("GE_ERR_NOWIDGET", MrmRtypeChar8);
	      if (error_string != NULL) 
	        {sprintf(geErrBuf, error_string, GECREATE_ELP_POPUP);
	         geError(geErrBuf, TRUE);
	         XtFree(error_string);
	        }
	     }
	  }
        /* Set all the fields before managing box. 
 	 */
	sprintf(buff, "%.3f", FList[1]);		/* Width */
 	XmTextSetString(geUI.WidgetArray[GEWCREATE_WIDTH], buff);
	sprintf(buff, "%.3f", FList[2]);		/* Height   */
 	XmTextSetString(geUI.WidgetArray[GEWCREATE_HEIGHT], buff);
	sprintf(buff, "%.3f", FList[3]);		/* DegRot */
 	XmTextSetString(geUI.WidgetArray[GEWCREATE_DEGROT], buff);
	sprintf(buff, "%.3f", FList[4]);		/* LinW */
 	XmTextSetString(geUI.WidgetArray[GEWCREATE_LINW], buff);

	XtManageChild(geUI.WidgetArray[GEWCREATE_ELP]);

	if (geState.InCreateMode)
  	  { GEVEC(GEXDISP, ASegP); } 			/* XOR the object */
	break;

    case GEOBJEDIT:
    case GEEDIT:
	if (!ASegP || !ASegP->Visible) break;

	if (!geUI.WidgetArray[GEWCREATE_ELP])
	  { GEVEC(GEOBJCREATE, ASegP);
	    break;
	  }

	if (geU.Map != -1) FCon = geU.Con[geU.Map];
	else               FCon = 1.0;
	FOCon      = FCon / GEFSUIPP;
	FICon      = GEFSUIPP / FCon;

	/* Get all the fields before displaying the new object.
 	 */                                              /* Width */
 	string = XmTextGetString(geUI.WidgetArray[GEWCREATE_WIDTH]);
	ftemp = (float)atof(string) * FICon;
	GEINT(ftemp, priv->A); 
	XtFree(string);
                                                         /* Height */
 	string = XmTextGetString(geUI.WidgetArray[GEWCREATE_HEIGHT]);
	ftemp = (float)atof(string) * FICon;
	GEINT(ftemp, priv->B); 
	XtFree(string);
							 /* DegRot */
	if (XmToggleButtonGetState(geUI.WidgetArray[GEWCREATE_ROTATE]))
 	  {string = XmTextGetString(geUI.WidgetArray[GEWCREATE_DEGROT]);
	   if (ftemp = (float)atof(string))
	     {/*
	       * Rotate the object about its center
	       */
	       geEditBox.Pt.x  = priv->Cx;
	       geEditBox.Pt.y  = priv->Cy;
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
	geState.ASegP->Private    = geMalloc(sizeof(struct GE_ELP));
	privnu 		          = (struct GE_ELP *)geState.ASegP->Private;
	geGenCopySeg(geState.ASegP, ASegP);
	*privnu                        = *priv;
	GEVEC(GEBOUNDS, geState.ASegP);
	break;

    case GEWARP2:
        if (ASegP && ASegP->Visible && priv)
	  {priv->A = abs(geState.Mouse.x - priv->Cx);
	   priv->B = abs(geState.Mouse.y - priv->Cy);
	   GEVEC(GEBOUNDS, ASegP);
	   if(geState.Grav.Align || geState.APagP->Grid.XAlign ||
	      geState.APagP->Grid.YAlign)
	     {geState.Dx = geState.Dy = 0;
	      geGenAln(priv->Cx, priv->Cy);
	      priv->Cx += geState.Dx;
	      priv->Cy += geState.Dy;
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
		{geGenAln(priv->Bx.x1, priv->Bx.y1);
		 if (!geState.Grav.Lock)
		   {geGenAln(priv->Bx.x2, priv->Bx.y2);
		    if (!geState.Grav.Lock)
		      {geGenAln(priv->Bx.x3, priv->Bx.y3);
		       if (!geState.Grav.Lock)
			 geGenAln(priv->Bx.x4, priv->Bx.y4);
		      }
		   }
	        }
	     }
	   if (cmd != GEGRAVTST && (geState.Dx || geState.Dy))
	     {priv->Cx    += geState.Dx;
	      priv->Cy    += geState.Dy;
	      priv->Bx.x1 += geState.Dx;
	      priv->Bx.y1 += geState.Dy;
	      priv->Bx.x2 += geState.Dx;
	      priv->Bx.y2 += geState.Dy;
	      priv->Bx.x3 += geState.Dx;
	      priv->Bx.y3 += geState.Dy;
	      priv->Bx.x4 += geState.Dx;
	      priv->Bx.y4 += geState.Dy;
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
	   geGenGrav(GEGRAVLOCK, priv->Bx.x1, priv->Bx.y1);
	   geGenGrav(GEGRAVLOCK, priv->Bx.x2, priv->Bx.y2);
	   geGenGrav(GEGRAVLOCK, priv->Bx.x3, priv->Bx.y3);
	   geGenGrav(GEGRAVLOCK, priv->Bx.x4, priv->Bx.y4);
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
	  {/*
	    * If it's orthogonal, then it's a bit simpler to figure out
	    */
	   if (!priv->Gamma)
		{priv->Bx.x1  = priv->Cx - abs(priv->A);
		 priv->Bx.y1  = priv->Bx.y3 = priv->Cy;
		 priv->Bx.x2  = priv->Bx.x4 = priv->Cx;
		 priv->Bx.y2  = priv->Cy - abs(priv->B);
		 priv->Bx.x3  = priv->Cx + abs(priv->A);
		 priv->Bx.y4  = priv->Cy + abs(priv->B);
	   	 geClip.x1 = priv->Bx.x1;
	   	 geClip.y1 = priv->Bx.y2;
	   	 geClip.x2 = priv->Bx.x3;
	   	 geClip.y2 = priv->Bx.y4;
	     	}
	   else
	     	{float asin, acos, bsin, bcos;

		 GEFSIN(priv->Gamma);
    		 ftsin     = geFT;
   		 GEFCOS(priv->Gamma);
   		 ftcos     = geFT;
		 acos = (float)(abs(priv->A)) * ftcos;
		 asin = (float)(abs(priv->A)) * ftsin;
		 bcos = (float)(abs(priv->B)) * ftcos;
		 bsin = (float)(abs(priv->B)) * ftsin;

		 priv->Bx.x1  = priv->Cx - acos;
		 priv->Bx.y1  = priv->Cy + asin;
		 priv->Bx.x2  = priv->Cx - bsin;
		 priv->Bx.y2  = priv->Cy - bcos;
		 priv->Bx.x3  = priv->Cx + acos;
		 priv->Bx.y3  = priv->Cy - asin;
		 priv->Bx.x4  = priv->Cx + bsin;
		 priv->Bx.y4  = priv->Cy + bcos;

	   	 temp         = max(priv->A, priv->B);
	   	 geClip.x1 = priv->Cx - temp;
		 geClip.y1 = priv->Cy - temp;
		 geClip.x2 = priv->Cx + temp;
		 geClip.y2 = priv->Cy + temp;
	     	}
	  }
	break;

#endif

    case GEBOUNDS:
        if (ASegP && ASegP->Visible && priv)
	  {/*
	    * If it's orthogonal, then it's a bit simpler to figure out
	    */
	   if (!priv->Gamma)
		{priv->Bx.x1  = priv->Cx - abs(priv->A);
		 priv->Bx.y1  = priv->Bx.y3 = priv->Cy;
		 priv->Bx.x2  = priv->Bx.x4 = priv->Cx;
		 priv->Bx.y2  = priv->Cy - abs(priv->B);
		 priv->Bx.x3  = priv->Cx + abs(priv->A);
		 priv->Bx.y4  = priv->Cy + abs(priv->B);
	   	 ASegP->Co.x1 = priv->Bx.x1 - (Hlw = priv->LinW >> 1) - GESUI1;
	   	 ASegP->Co.y1 = priv->Bx.y2 - Hlw;
	   	 if (Hlw)
		   {Hlw = (priv->LinW - GESUI1) >> 1;}
	   	 else	    Hlw = 0;
	   	 ASegP->Co.x2 = priv->Bx.x3 + Hlw;
	   	 ASegP->Co.y2 = priv->Bx.y4 + Hlw;
	     	}
	   else
	     	{float asin, acos, bsin, bcos;

		 GEFSIN(priv->Gamma);
    		 ftsin     = geFT;
   		 GEFCOS(priv->Gamma);
   		 ftcos     = geFT;
		 acos = (float)(abs(priv->A)) * ftcos;
		 asin = (float)(abs(priv->A)) * ftsin;
		 bcos = (float)(abs(priv->B)) * ftcos;
		 bsin = (float)(abs(priv->B)) * ftsin;

		 priv->Bx.x1  = priv->Cx - acos;
		 priv->Bx.y1  = priv->Cy + asin;
		 priv->Bx.x2  = priv->Cx - bsin;
		 priv->Bx.y2  = priv->Cy - bcos;
		 priv->Bx.x3  = priv->Cx + acos;
		 priv->Bx.y3  = priv->Cy - asin;
		 priv->Bx.x4  = priv->Cx + bsin;
		 priv->Bx.y4  = priv->Cy + bcos;

	   	 temp         = max(priv->A, priv->B);
	   	 ASegP->Co.x1 = priv->Cx - temp - (Hlw = priv->LinW >> 1) -
		                GESUI1;
		 ASegP->Co.y1 = priv->Cy - temp - Hlw;
		 ASegP->Co.x2 = priv->Cx + temp + Hlw;
		 ASegP->Co.y2 = priv->Cy + temp + Hlw;
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
	   }
        break;

#ifdef GERAGS

    case GEROUND:
        if (ASegP && ASegP->Visible && priv)
          {priv->Cx   = GETSUI(GETPIX(priv->Cx));
           priv->Cy   = GETSUI(GETPIX(priv->Cy));
           priv->A    = GETSUI(GETPIX(priv->A));
           priv->B    = GETSUI(GETPIX(priv->B));
          }
        break;

    case GEALN:
	geGenAlign(ASegP);
	break;

    case GEALNREF:
        if (ASegP && ASegP->Visible)
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
	if (ASegP && ASegP->Visible && ASegP->FillStyle != GETRANSPARENT)
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
	   if ((TRegion = geXPolygonRegion(geVert, geVn, EvenOddRule)))
		{lx  = (geSel.x1 + geSel.x2) >> 1;
	  	 ly  = (geSel.y1 + geSel.y2) >> 1;
	  	 x = GETPIX(lx);
	  	 y = GETPIX(ly);
	  	 if (XPointInRegion(TRegion, x, y))
	   	     geState.SelRadC = 0;	/* This is IT		     */
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

	   geVn = 0;
	   GEVEC(GEADDVERT, ASegP); 
	   if (geVn >= 4) geGenPtSelVert(priv->LinW, cmd);
	   else           geState.SelRadC = GEMAXSHORT;
	  }	
	break;

    case GESELPTIN:
	geSegSel = ASegP;
	if (ASegP && ASegP->Visible)
	  {if (ASegP->Co.x1 > geSel.x2 || ASegP->Co.y1 > geSel.y2 ||
	       ASegP->Co.x2 < geSel.x1 || ASegP->Co.y2 < geSel.y1)
		 break;

	   /*
	    * If the selection point is INSIDE of this object, then
	    * compute the proximity - the closest object will be
	    * PICKED.
	    */
	   geVn = 0;
	   GEVEC(GEADDVERT, ASegP); 
	   if((TRegion = geXPolygonRegion(geVert, geVn, EvenOddRule)))
		{lx  = (geSel.x1 + geSel.x2) >> 1;
	  	 ly  = (geSel.y1 + geSel.y2) >> 1;
	  	 x = GETPIX(lx);
	  	 y = GETPIX(ly);
	  	 if (XPointInRegion(TRegion, x, y))
	   	     geGenPtSelVert(priv->LinW, cmd);
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
	  {priv->A = abs(geState.Mouse.x - priv->Cx);
	   priv->B = abs(geState.Mouse.y - priv->Cy);
	   if (priv->A > priv->B)
	     {priv->B = priv->A;
	      if (geState.Mouse.y > priv->Cy)
		geState.Mouse.y = geOldState.Mouse.y = priv->Cy + priv->B;
	      else
		geState.Mouse.y = geOldState.Mouse.y = priv->Cy - priv->B;
	     }
	   else
	     {priv->A = priv->B;
	      if (geState.Mouse.x > priv->Cx)
		geState.Mouse.x = geOldState.Mouse.x = priv->Cx + priv->A;
	      else
		geState.Mouse.x = geOldState.Mouse.x = priv->Cx - priv->A;
	     }
	   geState.MouLockCr = GEMOU_X;
	   GEVEC(GEBOUNDS, ASegP);
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

    case GEGETSEG_COL:
        if (ASegP && ASegP->Col.RGB.pixel == geGenCol.pixel ||
	    (priv && priv->Col.RGB.pixel == geGenCol.pixel))
	  geState.ASegP = ASegP;
	break;

    case GESCALE:
        if (ASegP && ASegP->Visible && priv)
	  {/*
	    * If you're being subjected to the painful ordeal of having
	    *to navigate through this quagmire, I've obviously not
	    *had the time to clean up this mess.  For what it's worth,
	    *you have my sympathy and apology.
	    *	ge 10/30/87
	    */
	   /*
	    * Standard scaling of the four points which are the end
	    *points of the major and minor axes.
	    */
	   geGenScaleBx(&priv->Bx);
	   /*
	    * To prevent overflow, work in pixels
	    */
	   ELP_TOPIX;
	   /*
	    * See if the results of the scaling have brought (or left)
	    *the ellipse onto an orthogonal footing.  The redundant
	    *checks are because of the possibility of round off errors.
	    */
	   if (priv->Bx.y1 == priv->Cy || priv->Bx.y3 == priv->Cy)
		{ELP_TOSUI;
		 priv->A     = abs(priv->Bx.x3 - priv->Cx);
		 priv->B     = abs(priv->Cy    - priv->Bx.y2);
		 priv->Gamma = 0.;
	   	 GEVEC(GEBOUNDS, ASegP);
		 break;
		}
	    else
	    if (priv->Bx.x1 == priv->Cx || priv->Bx.x3 == priv->Cx)
		{ELP_TOSUI;
		 priv->B     = abs(priv->Cy    - priv->Bx.y3);
		 priv->A     = abs(priv->Bx.x4 - priv->Cx);
		 priv->Gamma = 0.;
	   	 GEVEC(GEBOUNDS, ASegP);
		 break;
		}
	   /*
	    * It's NOT orthogonal - the chances are that the four points of
	    *the box no longer form a diamond (the major and
	    *minor axes are no longer perpendicular to one another).
	    *So, I'm now going to compute the
 	    *vertical distance from the end point of the shorter axis
	    *to the longer axis.  This line segment will then be moved to
	    *the center (really, dx & dy of the segment will be used to
 	    *reconstruct the end points of the shorter axis, which will
	    *again make the shorter axes perpendicular to one another).
	    *I know this is not right, but it will have to do till I have time
	    *to figure out how to construct a better elliptical approximation
	    *to a parallelogram - maybe never.
	    *
	    * The equations for doing this are:
	    *
	    *	y - y1		y - y2		  1
	    *  ------- = m13   ------- = m25 = - ---     which yields
	    *   x - x1		x - x2		 m13
	    *
	    *     m13 * x1 - y1 - m25 * x2 + y2
	    * x = -----------------------------		 and
	    *	  	    m13 - m25
	    *
	    * y = m13 * (x - x1) + y1
	    *
	    *	m13	the slope of the longer  axis
	    *	m25	the slope of the perpendicular to the longer axis
	    *	x1,y1	an end point of the longer  axis
	    *	x2,y2	an end point of the shorter axis
	    *	x,y	point on the longer axis which is the reflection
	    *		of the shorter axis
	    */
	   fdx1 = priv->Bx.x3 - priv->Bx.x1;
	   if (!fdx1) goto done_elp;
	   fdy1 = priv->Bx.y3 - priv->Bx.y1;
	   if (!fdy1) goto done_elp;
	   l13  = (float)fabs(sqrt((double)(fdx1 * fdx1) +
				   (double)(fdy1 * fdy1)));
	   fdx2 = priv->Bx.x4 - priv->Bx.x2;
	   if (!fdx2) goto done_elp;
	   fdy2 = priv->Bx.y4 - priv->Bx.y2;
	   if (!fdy2) goto done_elp;

	   l24  = (float)fabs(sqrt((double)(fdx2 * fdx2) +
				   (double)(fdy2 * fdy2)));
	   if (l13 > l24)
		{m13  = fdy1 / fdx1;
		 x1   = priv->Bx.x1; y1   = priv->Bx.y1;
		 x2   = priv->Bx.x2; y2   = priv->Bx.y2;
		 x3   = priv->Bx.x3; y3   = priv->Bx.y3;
		 x4   = priv->Bx.x4; y4   = priv->Bx.y4;
		}
	   else
		{m13  = fdy2 / fdx2;
		 x1   = priv->Bx.x2; y1   = priv->Bx.y2;
		 x2   = priv->Bx.x3; y2   = priv->Bx.y3;
		 x3   = priv->Bx.x4; y3   = priv->Bx.y4;
		 x4   = priv->Bx.x1; y4   = priv->Bx.y1;
		}
	   m25  = -1.0 / m13;
	   fx   = (m13 * x1 - y1 - m25 * x2 + y2) / (m13 - m25);
	   fy   = m13 * (fx - x1) + y1;
	   fdx  = (float)fabs((double)(x2 - fx));
	   fdy  = (float)fabs((double)(y2 - fy));
	   /*
	    * Reset the end points of the shorter axis so that its
	    *orthogonal to the longer one.
	    */
	   if (l13 > l24)
	   	{if (priv->Bx.x2 < priv->Cx)
		   {priv->Bx.x2 = priv->Cx - (int)fdx;
	   	    priv->Bx.x4 = priv->Cx + (int)fdx;
		   }
		 else
		   {priv->Bx.x2 = priv->Cx + (int)fdx;
	   	    priv->Bx.x4 = priv->Cx - (int)fdx;
		   }
	         if (priv->Bx.y2 < priv->Cy)
		   {priv->Bx.y2 = priv->Cy - (int)fdy;
		    priv->Bx.y4 = priv->Cy + (int)fdy;
		   }
	         else
		   {priv->Bx.y2 = priv->Cy + (int)fdy;
		    priv->Bx.y4 = priv->Cy - (int)fdy;
		   }
		}
	   else
	   	{if (priv->Bx.x1 < priv->Cx)
		   {priv->Bx.x1 = priv->Cx - (int)fdx;
	   	    priv->Bx.x3 = priv->Cx + (int)fdx;
		   }
		 else
		   {priv->Bx.x1 = priv->Cx + (int)fdx;
	   	    priv->Bx.x3 = priv->Cx - (int)fdx;
		   }
	         if (priv->Bx.y1 < priv->Cy)
		   {priv->Bx.y1 = priv->Cy - (int)fdy;
		    priv->Bx.y3 = priv->Cy + (int)fdy;
		   }
	         else
		   {priv->Bx.y1 = priv->Cy + (int)fdy;
		    priv->Bx.y3 = priv->Cy - (int)fdy;
		   }
		}
	   /*
	    * Have to check for orthogonality again.
	    */
	   ELP_TOSUI;
	   if (priv->Bx.y1 == priv->Cy || priv->Bx.y3 == priv->Cy)
		{priv->A     = abs(priv->Bx.x3 - priv->Cx);
		 priv->B     = abs(priv->Cy    - priv->Bx.y2);
		 priv->Gamma = 0.;
	   	 GEVEC(GEBOUNDS, ASegP);
		 break;
		}
	    else
	    if (priv->Bx.x1 == priv->Cx || priv->Bx.x3 == priv->Cx)
		{priv->B     = abs(priv->Cy    - priv->Bx.y3);
		 priv->A     = abs(priv->Bx.x4 - priv->Cx);
		 priv->Gamma = 0.;
	   	 GEVEC(GEBOUNDS, ASegP);
		 break;
		}
	    else
	   if (priv->Bx.y2 == priv->Cy || priv->Bx.y4 == priv->Cy)
		{priv->A     = abs(priv->Bx.x4 - priv->Cx);
		 priv->B     = abs(priv->Cy    - priv->Bx.y1);
		 priv->Gamma = 0.;
	   	 GEVEC(GEBOUNDS, ASegP);
		 break;
		}
	    else
	    if (priv->Bx.x2 == priv->Cx || priv->Bx.x4 == priv->Cx)
		{priv->B     = abs(priv->Cy    - priv->Bx.y2);
		 priv->A     = abs(priv->Bx.x3 - priv->Cx);
		 priv->Gamma = 0.;
	   	 GEVEC(GEBOUNDS, ASegP);
		 break;
		}
	   ELP_TOPIX;
	   /*
	    * Locate the pt in the 1st quadrant (wrt to (Cx,Cy))
	    *which is the end pt. of axis A.  The end pt of
	    *axis B is the point of Bx counter-clockwise from A, e.g.
	    *if A = Bx.x3,y3 then B = Bx.x2,y2, since the points of
	    *Bx are numbered "clockwise".
	    */
	   if (priv->Bx.x3 > priv->Cx && priv->Bx.y3 < priv->Cy)	   
	   	{xa	       = priv->Bx.x3 - priv->Cx;
	   	 ya	       = priv->Cy - priv->Bx.y3;
	   	 xb	       = priv->Cx - priv->Bx.x2;
	   	 yb	       = priv->Bx.y2 - priv->Cy;
		}
	   else
	   if (priv->Bx.x2 > priv->Cx && priv->Bx.y2 < priv->Cy)	   
	   	{xa	       = priv->Bx.x2 - priv->Cx;
	   	 ya	       = priv->Cy - priv->Bx.y2;
	   	 xb	       = priv->Cx - priv->Bx.x1;
	   	 yb	       = priv->Bx.y1 - priv->Cy;
		}
	   else
	   if (priv->Bx.x4 > priv->Cx && priv->Bx.y4 < priv->Cy)	   
	   	{xa	       = priv->Bx.x4 - priv->Cx;
	   	 ya	       = priv->Cy - priv->Bx.y4;
	   	 xb	       = priv->Cx - priv->Bx.x3;
	   	 yb	       = priv->Bx.y3 - priv->Cy;
		}
	   else
	   	{xa	       = priv->Bx.x1 - priv->Cx;
	   	 ya	       = priv->Cy - priv->Bx.y1;
	   	 xb	       = priv->Cx - priv->Bx.x4;
	   	 yb	       = priv->Bx.y4 - priv->Cy;
		}

	   fdx1	       = xa;
	   fdy1	       = ya;
	   fdx2	       = xb;
	   fdy2	       = yb;
	   fdx2	       = (float)sqrt((double)(fdx2 * fdx2) +
				     (double)(fdy2 * fdy2));
	   fdx1        = (float)sqrt((double)(fdx1 * fdx1) +
				     (double)(fdy1 * fdy1));
done_elp:
	   GEINT(fdx2, priv->B);
	   GEINT(fdx1, priv->A);
	   if (priv->A)
	   	{priv->Gamma = (float)(acos((double)xa / (double)fdx1)
			       * GE_RAD_TO_DEG);
		 while (priv->Gamma >= 90.)
		   {temp         = priv->A;
		    priv->A      = priv->B;
		    priv->B      = temp;
		    priv->Gamma -= 90.;
		   }
		}
	   else
		priv->Gamma = 0.;
	   ELP_TOSUI;
	   GEVEC(GEBOUNDS, ASegP);
	  }
	break;

    case GEFLIP:
	if (ASegP && ASegP->Visible && priv)
	   {geGenFlpPt(geState.Constraint, &priv->Cx, &priv->Cy);
	    temp         = priv->A;
	    priv->A      = priv->B;
	    priv->B      = temp;
	    if (priv->Gamma) priv->Gamma  = 90. - priv->Gamma;
	    GEVEC(GEBOUNDS, ASegP);
	   }
	break;

#endif

    case GEMAG:
    case GEMAGRESD:
        if (ASegP && ASegP->Visible && priv)
	   {if (geMagMode == GEHORIZONTAL || geMagMode == GENOCONSTRAINT)
		priv->A = (priv->A * (float)geMagFY) / 100.0;
	    if (geMagMode == GENOCONSTRAINT || geMagMode == GEVERTICAL)
		priv->B = (priv->B * (float)geMagFX) / 100.0;
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
	   GEVEC(GEBOUNDS, ASegP);
	   }
	break;

    case GEROTX:
        if (ASegP && ASegP->Visible && priv)
	  {geGenRotX(&priv->Cx, &priv->Cy);
	   if (geRot.Clockwise) priv->Gamma -= geRot.Beta;
	   else			priv->Gamma += geRot.Alpha;
	   while (priv->Gamma >= 90.)
		{temp         = priv->A;
		 priv->A      = priv->B;
		 priv->B      = temp;
		 priv->Gamma -= 90.;
		}
	   while (priv->Gamma < 0.)
		{temp         = priv->A;
		 priv->A      = priv->B;
		 priv->B      = temp;
		 priv->Gamma += 90.;
		}
	   GEVEC(GEBOUNDS, ASegP);
	  }
	break;

    case GEROTFIXED:
        if (ASegP && ASegP->Visible && priv)
	  {if (( geRot.Clockwise && geRot.Beta  == 90.) ||
	       (!geRot.Clockwise && geRot.Alpha == 90.))
	     {geGenRot90(&priv->Cx, &priv->Cy);
	      temp    = priv->A;
	      priv->A = priv->B;
	      priv->B = temp;
	      GEVEC(GEBOUNDS, ASegP);
	     }
	   else
	      GEVEC(GEROTX, ASegP);
	  }
	break;

    case GEEQUATE:
    case GEEQUATE_NOIMG:
        if(ASegP && geState.ASegP && ASegP->Handler == geState.ASegP->Handler
	   && priv && (privnu = (struct GE_ELP *)geState.ASegP->Private))
	  {geState.ASegP->Co = ASegP->Co;
	   geGenCopyAnim(geState.ASegP, ASegP);
	   geGenCopyDesc(geState.ASegP, ASegP);
	   privnu->Cx	     = priv->Cx;
	   privnu->Cy	     = priv->Cy;
	   privnu->A	     = priv->A;
	   privnu->B	     = priv->B;
	   privnu->Bx        = priv->Bx;
	   privnu->Gamma     = priv->Gamma;
	   privnu->LinW      = priv->LinW;
	  }
	break;

    case GEADDVERT:
	/*
	 * Add the coords of this object to the vertex list - used for
	 * selection.
	 */
	if (ASegP && ASegP->Visible && priv)
	   {geDrawElp(NULL, NULL,
		      (float)GETPIX(priv->Cx), (float)GETPIX(priv->Cy),
		      (float)GETPIX(priv->A), (float)GETPIX(priv->B),
		      priv->Gamma, 0,0,0,0,0,0,0,0,0,0, cmd);
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
	  geFree(&ASegP->Private, sizeof(struct GE_ELP));
	break;

    case GEWRITE:
    case GEREAD:
        geElpIO(cmd, ASegP);
	break;

    default:
	break;
   }
}

#ifdef GERAGS

/*
 * This routine test for the selection point (the center of geSel) being within
 * proximity (GESELRAD) of any line segment of the polygon gescribed by geVert.
 */
geGenPtSelVert(LinW, cmd)
long		LinW, cmd;
{
int		i, mind, select;
long		x, y, lt, minx, miny, maxx, maxy;
struct GE_CO	Co;

select = FALSE;
mind   = GEMAXSHORT;

x      = (geSel.x1 + geSel.x2) >> 1;
y      = (geSel.y1 + geSel.y2) >> 1;

lt     = (GESELRAD << 1);
lt     = (LinW >> 1) + GETSUI(lt);

if (geVn > 0)
   {i = 0;
    do
	{Co.x1 = GETSUI(geVert[i].x);
	 Co.y1 = GETSUI(geVert[i].y);
	 i++;
   	 Co.x2 = GETSUI(geVert[i].x);
   	 Co.y2 = GETSUI(geVert[i].y);
	 i++;
	 if (cmd == GESELPT)
	    {/*
	      * Check for test point being inside of bounding box of line segment
 	      */
	     if (Co.x1 < Co.x2)
	   	{minx = Co.x1 - lt;
	   	 maxx = Co.x2 + lt;
	   	}
	     else
	   	{minx = Co.x2 - lt;
	   	 maxx = Co.x1 + lt;
	   	}
	     if (Co.y1 < Co.y2)
	   	{miny = Co.y1 - lt;
	   	 maxy = Co.y2 + lt;
	   	}
	     else
	   	{miny = Co.y2 - lt;
	   	 maxy = Co.y1 + lt;
	   	}

	     if (x < minx || x > maxx || y < miny || y > maxy) continue;
	    }

	 select = geGenLinPtSel(&Co, LinW, x, y);
   	 if (geState.SelRadC < mind) mind = geState.SelRadC;
	}while (!select && i < geVn);
   }

return (select);

}

#endif

