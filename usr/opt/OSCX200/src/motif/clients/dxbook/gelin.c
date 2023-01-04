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
static char SccsId[] = "@(#)gelin.c	1.16\t10/16/89";
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
**	GELIN	             	       Line object handler
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

extern unsigned long geAllocColor();
extern char          *geMalloc();
extern double	     atof();	

geLin(cmd, ASegP)
long     	cmd;
struct GE_SEG	*ASegP;
{                       
#define GEOBJLIS  4

float		FList[GEOBJLIS], FCon, ftemp, fdx, fdy, flensui,
                FOCon, FICon, fx, fy;
char            *error_string, *string, buff[15];
short		EditPtFlip;
int		Hlw, PixLinW, temp, XSav, YSav, NumFrames;
long            x, y, dx, dy, len;
struct GE_LIN   *priv, *privnu;
struct GE_PT    Pta;
Boolean		horz, vert;

if (ASegP)
  {if (!ASegP->Live && cmd != GEKILL    && cmd != GERELPRIV &&
                       cmd != GEKILPRIV && cmd != GEGETSEG_COL &&
		       cmd != GEXDEL)
     return;
   priv = (struct GE_LIN *)ASegP->Private;
  }

switch(cmd)
   {case GEDISP:
        if (ASegP && ASegP->Visible && ASegP->Visible && priv &&
	    (priv->LineStyle != GETRANSPARENT || ASegP->WhiteOut) &&
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

	     GE_SET_FG_DISP_ATTR;

	     if (priv->WritingMode == GXxor && priv->Col.RGB.pixel ==
		 geBlack.pixel)
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
	     
	     if (geDispAttr.FgStyle != GETRANSPARENT)
	       {if (geDispAttr.FgHT == 100)
		  {GEFILSTYLE(geGC2, FillSolid);}
	       else
		 {GEFILSTYLE(geGC2, geDispAttr.FgStyle);
		  GESTIPPLE(geGC2, geDispAttr.FgHT);
	         }
	       }

	     XDrawLine(geDispDev, geState.Drawable, geGC2,
		       GETPIX(priv->Co.x1), GETPIX(priv->Co.y1),
		       GETPIX(priv->Co.x2), GETPIX(priv->Co.y2));

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
	  {GEL_FG_P_M(geGC2, GETPIX(priv->LinW), priv->Pat.LinP,
			    GEXORPIXEL, GXxor);
	   if (priv->Pat.LinP != LineSolid)
		XSetDashes(geDispDev, geGC2, 0, priv->Pat.DashList,
			   priv->Pat.DashLen);
	   GECAPSTYLE(geGC2, priv->CapStyle);
	   GEJOINSTYLE(geGC2, priv->JoinStyle);
	   if (priv->LineStyle != GETRANSPARENT)
	     {if (priv->LineHT == 100)
		{GEFILSTYLE(geGC2, FillSolid);}
	      else
		{GEFILSTYLE(geGC2, priv->LineStyle);
		 GESTIPPLE(geGC2, priv->LineHT);
	        }
	     }
	   XDrawLine(geDispDev, geState.Drawable, geGC2,
		     GETPIX(priv->Co.x1), GETPIX(priv->Co.y1),
		     GETPIX(priv->Co.x2), GETPIX(priv->Co.y2));
	  }
	 else
	   XDrawLine(geDispDev, geState.Drawable, geGC5,
	   	     GETPIX(priv->Co.x1), GETPIX(priv->Co.y1),
		     GETPIX(priv->Co.x2), GETPIX(priv->Co.y2));
	   if (geAnimMin && (abs(priv->Co.x2 - priv->Co.x1) > geAnimMin ||
			     abs(priv->Co.y2 - priv->Co.y1) > geAnimMin))
	     {XDrawLine(geDispDev, geState.Drawable, geGC5,
			GETPIX((priv->Co.x1 + priv->Co.x2) >> 1) - 3,
		        GETPIX((priv->Co.y1 + priv->Co.y2) >> 1),
		        GETPIX((priv->Co.x1 + priv->Co.x2) >> 1) + 3,
		        GETPIX((priv->Co.y1 + priv->Co.y2) >> 1));
	      XDrawLine(geDispDev, geState.Drawable, geGC5,
			GETPIX((priv->Co.x1 + priv->Co.x2) >> 1),
		        GETPIX((priv->Co.y1 + priv->Co.y2) >> 1) - 3,
		        GETPIX((priv->Co.x1 + priv->Co.x2) >> 1),
		        GETPIX((priv->Co.y1 + priv->Co.y2) >> 1) + 3);
	     }
	  }
	break;

    case GEHDISP:
    case GEHXDISP:
        if (priv && ASegP && ASegP->Visible && ASegP->InFrame)
	   {if (geState.EditPts)
		{if (priv->EditPt1) geGenBoxEP(cmd, priv->Co.x1, priv->Co.y1);
		 if (priv->EditPt2) geGenBoxEP(cmd, priv->Co.x2, priv->Co.y2);
		 break;
		}

	    if (geState.HilightMode == GEHILIGHT_BOX)
		{GEVEC(GEEXTENT, ASegP);    	/* EXTENT RECTANGLE          */
		 XDrawRectangle(geDispDev, geState.Drawable, geGC5,
                        	GETPIX(geClip.x1), GETPIX(geClip.y1),
                        	GETPIX(geClip.x2 - geClip.x1),
				GETPIX(geClip.y2 - geClip.y1));
		}
	    else
	  	{PixLinW = GETPIX(priv->LinW);
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
	  	 XDrawLine(geDispDev, geState.Drawable, geGC2,
		     	   GETPIX(priv->Co.x1), GETPIX(priv->Co.y1),
		     	   GETPIX(priv->Co.x2), GETPIX(priv->Co.y2));
		}
	  }
	break;

    case GECREATE:
    case GECREATESTAT:
    case GECREATE_NOSNAP1:
    case GECREATESTAT_NOSNAP1:
	if (cmd == GECREATESTAT || cmd == GECREATESTAT_NOSNAP1)
	  {geState.Mouse.x = geGenBx.x1;
	   geState.Mouse.y = geGenBx.y1;
	  }
	geSegCr();
	geState.ASegP->Private    = geMalloc(sizeof(struct GE_LIN));
	priv 		          = (struct GE_LIN *)geState.ASegP->Private;
	geState.ASegP->Handle[0]  = 'L';
	geState.ASegP->Handle[1]  = 'I';
	geState.ASegP->Handle[2]  = 'N';
	geState.ASegP->Handle[3]  = '\0';
	geState.ASegP->Handler    = geLin;
	
	if(cmd == GECREATE || cmd == GECREATESTAT &&
	   (geState.Grav.Align ||
	    geState.APagP->Grid.XAlign || geState.APagP->Grid.YAlign))
	  {geState.Dx = geState.Dy   = 0;
	   geState.Grav.OPt.x = geState.Mouse.x;
	   geState.Grav.OPt.y = geState.Mouse.y;
	   geGenAln(geState.Mouse.x, geState.Mouse.y);
	   geState.Grav.OPt.x = geState.Mouse.x += geState.Dx;
	   geState.Grav.OPt.y = geState.Mouse.y += geState.Dy;
	  }
	priv->Co.x1               = geU.XR = geState.Mouse.x;
	priv->Co.y1               = geU.YR = geState.Mouse.y;
	if (cmd == GECREATE || cmd == GECREATE_NOSNAP1)
	  geMouseXY(geState.Window);
	else
	  {geState.Mouse.x = geGenBx.x2;
	   geState.Mouse.y = geGenBx.y2;
	  }

	if (!geState.MouLockCr)
	  {priv->Co.x2            = geState.Mouse.x;
	   priv->Co.y2            = geState.Mouse.y;
	  }
	else
	  {if (geState.MouLockCr == GEMOU_X)
	     {priv->Co.x2            = geState.Mouse.x;
	      priv->Co.y2            = priv->Co.y1;
	      geState.Mouse.y	     = priv->Co.y1;
	     }
	   else
	     {priv->Co.x2            = priv->Co.x1;
	      geState.Mouse.x	     = priv->Co.x1;
	      priv->Co.y2            = geState.Mouse.y;
	     }
	  }

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
	priv->EditPt1             = priv->EditPt2 = FALSE;
	GEVEC(GEBOUNDS, geState.ASegP);
	if (cmd == GECREATE || cmd == GECREATE_NOSNAP1)
	  {geMnStat(GEDISP, 45);
	   geAnim(GECREATE, geState.ASegP);
	  }
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
	    (priv = (struct GE_LIN *)ASegP->Private))
	  {GE_SET_ATTR_ERASE;
	  }
	break;

    case GESETATTRERASE_WHITE:
	if (ASegP && ASegP->Visible && ASegP->WhiteOut && !geState.PrintPgBg &&
	    (priv = (struct GE_LIN *)ASegP->Private))
	  {GE_SET_ATTR_ERASE_WHITE;
	  }
	break;

    case GEOBJCREATE:
	if (!ASegP || !ASegP->Visible) break;

	if (geU.DMap != -1) FCon = geU.Con[geU.DMap];
	else                FCon = 1.0;
	FOCon      = FCon / GEFSUIPP;
	FICon      = GEFSUIPP / FCon;

	/* Set up default values for line object and fetch the create
	 * to size modal box.  Only use default values if in create 
	 * mode, otherwise use the current values (editing).
	 */
 	if (geState.InCreateMode)
	  {priv->Co.x1 = geState.Mouse.x;
	   priv->Co.y1 = geState.Mouse.y;
	   priv->Co.x2 = priv->Co.x1 + GESUI1 * 20;
	   priv->Co.y2 = priv->Co.y1;
	  }

	fdx         = (float)(priv->Co.x2 - priv->Co.x1);
	fdy         = (float)(priv->Co.y2 - priv->Co.y1);
	FList[1]    = (float)sqrt((double)(fdx * fdx) +
				     (double)(fdy * fdy)) * FOCon;
	FList[2]    = 0.;
	FList[3]    = (float)priv->LinW  * FOCon;

	if (!geUI.WidgetArray[GEWCREATE_LIN] || 
	    !XtIsManaged(geUI.WidgetArray[GEWCREATE_LIN]))
	  {if (MrmFetchWidget(geUI.Id, GECREATE_LIN_POPUP,
			      XtParent(geState.APagP->WidgetId),
			      &geUI.WidgetArray[GEWCREATE_LIN], 
			      &geUI.Class) != MrmSUCCESS)
	     {error_string = (char *) geFetchLiteral("GE_ERR_NOWIDGET",
						     MrmRtypeChar8);
	      if (error_string != NULL) 
	        {sprintf(geErrBuf, error_string, GECREATE_LIN_POPUP);
	         geError(geErrBuf, TRUE);
	         XtFree(error_string);
	        }
	     }
	  }
        /* Set all the fields before managing box. 
 	 */
	sprintf(buff, "%.3f", FList[1]);		/* Length */
 	XmTextSetString(geUI.WidgetArray[GEWCREATE_LEN], buff);
	sprintf(buff, "%.3f", FList[2]);		/* DegRot */
 	XmTextSetString(geUI.WidgetArray[GEWCREATE_DEGROT], buff);
	sprintf(buff, "%.3f", FList[3]);		/* LinW */
 	XmTextSetString(geUI.WidgetArray[GEWCREATE_LINW], buff);
	
	XtManageChild(geUI.WidgetArray[GEWCREATE_LIN]);

	if (geState.InCreateMode)
  	  { GEVEC(GEXDISP, ASegP); } 			/* XOR the object */
        break;

    case GEOBJEDIT:
    case GEEDIT:
	if (!ASegP || !ASegP->Visible) break;
	    
	/* If the create to size box isn't already displayed, create it.
	 */
	if (!geUI.WidgetArray[GEWCREATE_LIN])
	  { GEVEC(GEOBJCREATE, ASegP);
	    break;
	  }

	if (geU.DMap != -1) FCon = geU.Con[geU.DMap];
	else                FCon = 1.0;
	FOCon      = FCon / GEFSUIPP;
	FICon      = GEFSUIPP / FCon;

	/* Get all the fields before displaying the new object.
 	 */                                             /* Lenth */
 	string = XmTextGetString(geUI.WidgetArray[GEWCREATE_LEN]);
	ftemp = (float)atof(string) * FICon;		
	GEINT(ftemp, len);
	geGenReLen(&priv->Co, len);
	XtFree(string);

/*	if (priv->Co.y2 >= priv->Co.y1) 
	     fy = (float)priv->Co.y1 + ftemp;           /* Adjust y *
	else fy = (float)priv->Co.y1 - ftemp;
	GEINT(fy, priv->Co.y2);
	XtFree(string);
*/
							 /* DegRot */
	if (XmToggleButtonGetState(geUI.WidgetArray[GEWCREATE_ROTATE]))
 	  {string = XmTextGetString(geUI.WidgetArray[GEWCREATE_DEGROT]);
	   if (ftemp = (float)atof(string))
	     {/*
	       * Rotate the object about its center
	       */
	       geEditBox.Pt.x  = (priv->Co.x1 + priv->Co.x2) >> 1;
	       geEditBox.Pt.y  = (priv->Co.y1 + priv->Co.y2) >> 1;
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
	geState.ASegP->Private    = geMalloc(sizeof(struct GE_LIN));
	privnu 		          = (struct GE_LIN *)geState.ASegP->Private;
	strcpy (geState.ASegP->Handle, ASegP->Handle);
	geState.ASegP->Handler    = ASegP->Handler;
	geState.ASegP->Col        = priv->Col;
	geState.ASegP->Xref       = ASegP->Xref;
	geState.ASegP->WhiteOut   = ASegP->WhiteOut;
	geState.ASegP->ConLine    = ASegP->ConLine;
	geGenCopyAnim(geState.ASegP, ASegP);
	geGenCopyDesc(geState.ASegP, ASegP);
	*privnu                   = *priv;
	GEVEC(GEBOUNDS, geState.ASegP);
	break;

    case GEWARP1:
        if (ASegP && ASegP->Visible && priv)
	  {priv->Co.x1 = geState.Mouse.x;
	   priv->Co.y1 = geState.Mouse.y;
	  }
	break;

    case GEWARP2:
        if (ASegP && ASegP->Visible && priv)
	  {priv->Co.x2 = geState.Mouse.x;
	   priv->Co.y2 = geState.Mouse.y;
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
		geGenAln(priv->Co.x1, priv->Co.y1);
	      if (!geState.Grav.Lock)
		{if (!geState.EditPts || priv->EditPt2)
		   geGenAln(priv->Co.x2, priv->Co.y2);
		 if (!geState.EditPts && !geState.Grav.Lock)
		   geGenAln((priv->Co.x2 + priv->Co.x1) >> 1,
			     (priv->Co.y2 + priv->Co.y1) >> 1);
	        }
	     }
	   if (cmd != GEGRAVTST && (geState.Dx || geState.Dy))
	     {if (!geState.EditPts || priv->EditPt1)
		{priv->Co.x1 += geState.Dx;
		 priv->Co.y1 += geState.Dy;
	        }
	      if (!geState.EditPts || priv->EditPt2)
		{priv->Co.x2 += geState.Dx;
		 priv->Co.y2 += geState.Dy;
	        }
	     }
	  }
	break;

#ifdef GERAGS

    case GETESTPT:
        if (!geState.EditPts || !ASegP || !ASegP->Visible) break;

	EditPtFlip = FALSE;
	if GESELTST(priv->Co.x1, priv->Co.y1)
	  {priv->EditPt1 = priv->EditPt1 ? FALSE : TRUE;
	   EditPtFlip = TRUE;
	  }
	if GESELTST(priv->Co.x2, priv->Co.y2)
	  {priv->EditPt2 = priv->EditPt2 ? FALSE : TRUE;
	   EditPtFlip = TRUE;
	  }

	if (!EditPtFlip)
	  {priv->EditPt1 = priv->EditPt1 ? FALSE : TRUE;
	   priv->EditPt2 = priv->EditPt2 ? FALSE : TRUE;
	  }

	break;

    case GETESTPTCLR:
        if (!ASegP || !ASegP->Visible) break;

	priv->EditPt1 = FALSE;
	priv->EditPt2 = FALSE;
	break;

    case GEGRAV:
        if (ASegP && ASegP->Visible && priv &&
	    (geState.Grav.Pt.x >= (ASegP->Co.x1 - geState.Grav.Rad) &&
	     geState.Grav.Pt.y >= (ASegP->Co.y1 - geState.Grav.Rad) &&
	     geState.Grav.Pt.x <= (ASegP->Co.x2 + geState.Grav.Rad) &&
	     geState.Grav.Pt.y <= (ASegP->Co.y2 + geState.Grav.Rad)))
	  {geGenGrav(GEGRAVLOCK, priv->Co.x1, priv->Co.y1);
	   geGenGrav(GEGRAVLOCK, priv->Co.x2, priv->Co.y2);
	   geGenGrav(GEGRAVLOCK, ((priv->Co.x2 + priv->Co.x1) >> 1),
		     ((priv->Co.y2 + priv->Co.y1) >> 1));
	  }
	break;

    case GECONTROL:
        if (ASegP && ASegP->Visible)
	   {if (abs(geState.Mouse.x - priv->Co.x1) >
		abs(geState.Mouse.x - priv->Co.x2))
		geState.Mouse.x = priv->Co.x1;
	    else
		geState.Mouse.x = priv->Co.x2;
	    if (abs(geState.Mouse.y - priv->Co.y1) >
		abs(geState.Mouse.y - priv->Co.y2))
		geState.Mouse.y = priv->Co.y1;
	    else
		geState.Mouse.y = priv->Co.y2;
	   }
	break;

    case GEEXTENT0:
        if (ASegP && ASegP->Visible && priv)
	  {geClip.x1 = min(priv->Co.x1, priv->Co.x2);
	   geClip.y1 = min(priv->Co.y1, priv->Co.y2);
	   geClip.x2 = max(priv->Co.x1, priv->Co.x2);
	   geClip.y2 = max(priv->Co.y1, priv->Co.y2);
	  }
	break;

#endif

    case GEBOUNDS:
        if (ASegP && ASegP->Visible && priv)
	  {ASegP->Co.x1 = min(priv->Co.x1, priv->Co.x2) -
			  (Hlw = priv->LinW >> 1);
	   ASegP->Co.y1 = min(priv->Co.y1, priv->Co.y2) - Hlw;
	   if (Hlw)
	     {Hlw = (priv->LinW - GESUI1) >> 1;}
	   else	    Hlw = 0;
	   ASegP->Co.x2 = max(priv->Co.x1, priv->Co.x2) + Hlw;
	   ASegP->Co.y2 = max(priv->Co.y1, priv->Co.y2) + Hlw;
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

    case GEINQPTS:
        if (ASegP && ASegP->Visible && priv)
	   {geClip.x1 = priv->Co.x1;
	    geClip.y1 = priv->Co.y1;
	    geClip.x2 = priv->Co.x2;
	    geClip.y2 = priv->Co.y2;
	   }
	break;

#ifdef GERAGS

    case GEROUND:
        if (ASegP && ASegP->Visible && priv)
          {priv->Co.x1  = GETSUI(GETPIX(priv->Co.x1));
           priv->Co.y1  = GETSUI(GETPIX(priv->Co.y1));
           priv->Co.x2  = GETSUI(GETPIX(priv->Co.x2));
           priv->Co.y2  = GETSUI(GETPIX(priv->Co.y2));
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
	   break;
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

    case GESELPT:
	geSegSel = ASegP;
	if (ASegP && ASegP->Visible)
	  {if (ASegP->Co.x1 > geSel.x2 || ASegP->Co.y1 > geSel.y2 ||
	       ASegP->Co.x2 < geSel.x1 || ASegP->Co.y2 < geSel.y1) break;
	   x = (geSel.x1 + geSel.x2) >> 1;
	   y = (geSel.y1 + geSel.y2) >> 1;
	   if (geGenLinPtSel(&priv->Co, priv->LinW, x, y))
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
	      if ((priv->Co.x1 > geSel.x1 && priv->Co.y1 > geSel.y1 &&
		   priv->Co.x1 < geSel.x2 && priv->Co.y1 < geSel.y2) ||
		  (priv->Co.x2 > geSel.x1 && priv->Co.y2 > geSel.y1 &&
		   priv->Co.x2 < geSel.x2 && priv->Co.y2 < geSel.y2))
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
	    if (geGetPutFlgs & GEGETPUTCAP)
	    	priv->CapStyle = geSampleAttr.CapStyle;
	    if (geGetPutFlgs & GEGETPUTLINP)
	        {priv->Pat.LinP     = geSampleAttr.LinP;
	    	 priv->Pat.DashLen  = geSampleAttr.DashLen;
	    	 priv->Pat.DashIndx = geSampleAttr.DashIndx;
	    	 priv->Pat.DashList[0] = geDashList[3][0];   /* user defined */
	    	 priv->Pat.DashList[1] = geDashList[3][1];
	    	 priv->Pat.DashList[2] = geDashList[3][2];
	    	 priv->Pat.DashList[3] = geDashList[3][3];
 		}
	   }
	break;

    case GECONSTRAIN:
        if (ASegP && ASegP->Visible && priv)
          {horz = XmToggleButtonGetState(geUI.WidgetArray[GEWHORZCONSTR]);
           vert = XmToggleButtonGetState(geUI.WidgetArray[GEWVERTCONSTR]);
	   if ((horz && vert) || (!horz && !vert))
	     {if ((dx = abs(geState.Mouse.x - priv->Co.x1)) >
	          (dy = abs(geState.Mouse.y - priv->Co.y1)))
		{if (dy < (dx >> 1))
	           {priv->Co.x2 = geState.Mouse.x;
	            priv->Co.y2 = priv->Co.y1;
	            geState.MouLockCr = GEMOU_X;
	            geState.Mouse.y = geOldState.Mouse.y = priv->Co.y1;
	           }
		 else				/* DIAGONAL		     */
	           {if (geState.Mouse.y > priv->Co.y1)
			 geState.Mouse.y = priv->Co.y1 + dx;
		    else geState.Mouse.y = priv->Co.y1 - dx;
		    priv->Co.x2 = geState.Mouse.x;
	            priv->Co.y2 = geState.Mouse.y;
	            geState.MouLockCr = GEMOU_D;
	           }
		}
	      else
		{if (dx < (dy >> 1))
	           {priv->Co.x2 = priv->Co.x1;
	            priv->Co.y2 = geState.Mouse.y;
	            geState.MouLockCr = GEMOU_Y;
	            geState.Mouse.x = geOldState.Mouse.x = priv->Co.x1;
	           }
		 else				/* DIAGONAL		     */
	           {if (geState.Mouse.x > priv->Co.x1)
			 geState.Mouse.x = priv->Co.x1 + dy;
		    else geState.Mouse.x = priv->Co.x1 - dy;
		    priv->Co.x2 = geState.Mouse.x;
	            priv->Co.y2 = geState.Mouse.y;
	            geState.MouLockCr = GEMOU_D;
	           }
		}
	     }
           else if (horz)
	        {priv->Co.x2 = geState.Mouse.x;
	         priv->Co.y2 = priv->Co.y1;
	         geState.MouLockCr = GEMOU_X;
	         geState.Mouse.y = geOldState.Mouse.y = priv->Co.y1;
	        }
           else if (vert)
	        {priv->Co.x2 = priv->Co.x1;
	         priv->Co.y2 = geState.Mouse.y;
	         geState.MouLockCr = GEMOU_Y;
	         geState.Mouse.x = geOldState.Mouse.x = priv->Co.x1;
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
	   geGenRgbToCmyk(&(priv->Col.RGB), &(priv->Col.CMYK));
	   geGenNameRgb(&(priv->Col));
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
    case GEINQLINCOL:
        if (ASegP && ASegP->Visible && priv) geAttr.Col          = priv->Col;
	break;

    case GEGETSEG_COL:
        if (ASegP && ASegP->Col.RGB.pixel == geGenCol.pixel ||
	    (priv && priv->Col.RGB.pixel == geGenCol.pixel))
	  geState.ASegP = ASegP;
	break;

    case GESCALE:
        if (ASegP && ASegP->Visible && priv)
	  {if (!geState.EditPts || (priv->EditPt1 && priv->EditPt2))
	     geGenScale(&priv->Co);
	   else
	     {if (priv->EditPt1)
		{Pta.x       = priv->Co.x1;
		 Pta.y       = priv->Co.y1;
		 geGenScalePt(&Pta);
		 priv->Co.x1 = Pta.x;
		 priv->Co.y1 = Pta.y;
	        }
	      else
		{Pta.x       = priv->Co.x2;
		 Pta.y       = priv->Co.y2;
		 geGenScalePt(&Pta);
		 priv->Co.x2 = Pta.x;
		 priv->Co.y2 = Pta.y;
	        }
	     }
	  }
	break;

    case GEFLIP:
	if (ASegP && ASegP->Visible && priv)
	  {if (!geState.EditPts || (priv->EditPt1 && priv->EditPt2))
	     geGenFlpCo(geState.Constraint, &priv->Co);
	    else
	      {if (priv->EditPt1)
		 geGenFlpPt(geState.Constraint, &priv->Co.x1, &priv->Co.y1);
	       else
		 geGenFlpPt(geState.Constraint, &priv->Co.x2, &priv->Co.y2);
	      }
	  }
	break;

#endif

    case GEMAG:
    case GEMAGRESD:
        if (ASegP && ASegP->Visible && priv)
	   {if (!geMagGrp)
		{geMagGrp = TRUE;
		 geMagCx  = (ASegP->Co.x1 + ASegP->Co.x2) >> 1;
		 geMagCy  = (ASegP->Co.y1 + ASegP->Co.y2) >> 1;
		}
	    if (!geState.EditPts || (priv->EditPt1 && priv->EditPt2))
	      geGenMag(&priv->Co);
	    else
	      {if (priv->EditPt1)
		 {Pta.x       = priv->Co.x1;
		  Pta.y       = priv->Co.y1;
		  geGenMagPt(&Pta);
		  priv->Co.x1 = Pta.x;
		  priv->Co.y1 = Pta.y;
	         }
	       else
		 {Pta.x       = priv->Co.x2;
		  Pta.y       = priv->Co.y2;
		  geGenMagPt(&Pta);
		  priv->Co.x2 = Pta.x;
		  priv->Co.y2 = Pta.y;
	         }
	      }
	    if (geState.ZoomLineThick)
	      priv->LinW = ((priv->LinW ? priv->LinW : GESUI1) * geMagFX) / 100;
	   }
	break;

    case GEROTX:
        if (ASegP && ASegP->Visible && priv)
	  {if (!geState.EditPts || priv->EditPt1)
	     geGenRotX(&priv->Co.x1, &priv->Co.y1);
	   if (!geState.EditPts || priv->EditPt2)
	     geGenRotX(&priv->Co.x2, &priv->Co.y2);
	  }
	break;

    case GEROTFIXED:
        if (ASegP && ASegP->Visible && priv)
	  {if (( geRot.Clockwise && geRot.Beta  == 90.) ||
	       (!geRot.Clockwise && geRot.Alpha == 90.))
	     {if (!geState.EditPts || priv->EditPt1)
		geGenRot90(&priv->Co.x1, &priv->Co.y1);
	      if (!geState.EditPts || priv->EditPt2)
		geGenRot90(&priv->Co.x2, &priv->Co.y2);
	     }
	   else
	     {if (!geState.EditPts || priv->EditPt1)
		geGenRotX (&priv->Co.x1, &priv->Co.y1);
	      if (!geState.EditPts || priv->EditPt2)
		geGenRotX (&priv->Co.x2, &priv->Co.y2);
	     }
	  }
	break;

    case GEEQUATE:
    case GEEQUATE_NOIMG:
        if(ASegP && geState.ASegP && ASegP->Handler == geState.ASegP->Handler
	   && priv && (privnu = (struct GE_LIN *)geState.ASegP->Private))
	  {geState.ASegP->Co = ASegP->Co;
	   geGenCopyAnim(geState.ASegP, ASegP);
	   geGenCopyDesc(geState.ASegP, ASegP);
	   privnu->Co        = priv->Co;
	   privnu->LinW      = priv->LinW;
	  }
	break;

    case GEADDVERT:
	/*
	 * Add the coords of this object to the vertex list - used for
	 * selection and to display
	 * the filled interior of polygons.  If a coord is the same as the
	 * last coord in the vertex list, then don't add it.
	 */
	if (ASegP && ASegP->Visible && priv && geVn < GEMAX_PTS)
	  {if (!geVn || geVert[geVn - 1].x != GETPIX(priv->Co.x1) ||
	       geVert[geVn - 1].y != GETPIX(priv->Co.y1))
	       {GESTOREVERT(geVn, GETPIX(priv->Co.x1), GETPIX(priv->Co.y1));
		geVn++;
	       }
	   if (!geVn || geVert[geVn - 1].x != GETPIX(priv->Co.x2) ||
	       geVert[geVn - 1].y != GETPIX(priv->Co.y2))
	       {GESTOREVERT(geVn, GETPIX(priv->Co.x2), GETPIX(priv->Co.y2));
		geVn++;
	       }
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
	  geFree(&ASegP->Private, sizeof(struct GE_LIN));
	break;

    case GEWRITE:
    case GEREAD:
     /*
      * See if it's ascii or ddif
      */
     if (geReadWriteType == GE_RW_DDIF)
       geLinIODDIF(cmd, ASegP);
     else
       geLinIO(cmd, ASegP);
     break;

    default:
	break;
   }
}

geGenReLen(Co, NewLen)
struct GE_CO *Co;
long          NewLen;
{
float PrevLen, ftemp, ftemp1, fdx, fdy;

fdx     = (float)(Co->x2 - Co->x1);
fdy     = (float)(Co->y2 - Co->y1);
PrevLen = (float)sqrt((double)(fdx * fdx) +
		      (double)(fdy * fdy));

/*
 * Adjust x2
 */
ftemp = (float)NewLen * (float)fabs((double)(Co->x2 - Co->x1)) / PrevLen;
if (Co->x2 >= Co->x1) ftemp1 = Co->x1 + ftemp;
else                  ftemp1 = Co->x1 - ftemp;
GEINT(ftemp1, Co->x2);
/*
 * Adjust y2
 */
ftemp = (float)NewLen * (float)fabs((double)(Co->y2 - Co->y1)) / PrevLen;
if (Co->y2 >= Co->y1) ftemp1 = Co->y1 + ftemp;
else                  ftemp1 = Co->y1 - ftemp;
GEINT(ftemp1, Co->y2);

}
