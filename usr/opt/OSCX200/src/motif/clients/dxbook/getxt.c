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
static char SccsId[] = "@(#)getxt.c	1.23\t10/16/89";
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
**	GETXT	             	       Text object handler
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
**	GNE 02/17/87 Created
**
**--
**/

#include "geGks.h"
#include <X11/keysym.h>
#include <X11/Xutil.h>


struct  GECURSE
 {int x;
  int y;
  int ox;
  int oy;
  int w;
  int h;
  int dx;
 };

extern unsigned long 	geAllocColor();
extern XFontStruct * geLoadFont();

extern char          	*geMalloc();
extern Region		geXPolygonRegion();

static  unsigned char buf[1000], KillBuf[1000] = {'\0'};
static  short         CurI, MaxI;
static  int           Slen, ascent, descent, dir, PrivFont, PrivX, PrivY,
  		      NumFrames;
static	long	      StartX, StartY, Temp, CurXoff;
struct GECURSE        Curse;

static XCharStruct    overall;


geTxt(cmd, ASegP)
long     	cmd;
struct GE_SEG	*ASegP;
{                       
char                    Import[1000];
short           	width, height;
int			x, y;
long			lx, ly;

struct GE_TXT   	*priv, *privnu;

Region			TRegion;

if (ASegP)
  {if (!ASegP->Live && cmd != GESCRUB   && cmd != GEKILL && cmd != GERELPRIV &&
                       cmd != GEKILPRIV && cmd != GEGETSEG_COL &&
		       cmd != GEXDEL)
     return;
   priv = (struct GE_TXT *)ASegP->Private;
  }

switch(cmd)
   {case GEDISP:
        if (ASegP && ASegP->Talk &&  priv && priv->Str &&
	    geState.Window && (Slen = strlen(priv->Str)) &&
	    geState.AnimPreview &&
	    !geState.HaltRequested)
	  {/*
	    * Invoke DECtalk with text buff
	    */
	    geDECtalk(priv->Str, Slen);
	  }
        if (ASegP && ASegP->Visible && ASegP->InFrame && priv &&
	    priv->Str && (Slen = strlen(priv->Str)) &&
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

	       if (ASegP->FillStyle != GETRANSPARENT)
		 {GE_SET_BG_DISP_ATTR;
		  GEFILSTYLE_FOS(geGC3, geDispAttr.BgStyle,
				 geDispAttr.BgPixel);
		  GEFORG(geGC3, geDispAttr.BgPixel);
		  GEMODE(geGC3, ASegP->FillWritingMode);
		  GESTIPPLE(geGC3, geDispAttr.BgHT);
		 }

	       GE_SET_FG_DISP_ATTR;

	       if (priv->WritingMode == GXxor &&
		   priv->Col.RGB.pixel == geBlack.pixel)
		 {GEF_FG_M_PM(geGC4, geFontUser[priv->Font]->fid,
			      GEXORPIXEL, priv->WritingMode,
			      geXWPixel ^ geXBPixel);
		 }
	       else
		 {GEF_FG_M_PM(geGC4, geFontUser[priv->Font]->fid,
			      geDispAttr.FgPixel, priv->WritingMode,
			      AllPlanes);
		 }

	       if (geDispAttr.FgHT == 100)
		 {GEFILSTYLE(geGC4, FillSolid);}
	       else
		 {GEFILSTYLE(geGC4, geDispAttr.FgStyle);
		  GESTIPPLE(geGC4, geDispAttr.FgHT);
		 }

	       if (ASegP->FillStyle != GETRANSPARENT)
		 XFillRectangle(geDispDev, geState.Drawable, geGC3,
				GETPIX(ASegP->Co.x1), GETPIX(ASegP->Co.y1),
				GETPIX(ASegP->Co.x2) - GETPIX(ASegP->Co.x1),
				GETPIX(ASegP->Co.y2) - GETPIX(ASegP->Co.y1));

	       XDrawString(geDispDev, geState.Drawable, geGC4,
			   GETPIX(priv->Pt.x), GETPIX(priv->Pt.y),
			   (char *)priv->Str, Slen);

	       if (ASegP->AnimCtrl.Animated && NumFrames > 0)
		 NumFrames = geSleepDelay(ASegP, NumFrames);
	       
	      } while (--NumFrames > 0 && !geState.HaltRequested);
	   }
	break;

    case GECDISP:
	if (ASegP && ASegP->Visible && geGenInClip(ASegP))
	  {/*
	    * Outside of Rags, if the Clipped Refresh request is arising
	    * as a consequence of an object in animation AND this object's
	    * "CdispOverride" flag is OFF then the request is to be IGNORED
	    * if either:
	    *	1) if the Z order of this object is greater than the Z order
	    *	   of the object currently animating;
	    *	2) if this object is supposed to be ERASED during its own
	    *	   animation sequence and it is NOT to be finally REDRAWN
	    */
	   if (!geRun.RagsCalling && ((geState.State == GEANIMPREVIEW &&
		geState.ASegP && ASegP->Z >= geState.ASegP->Z) ||
	       (geState.State != GEANIMPREVIEW &&
		ASegP->AnimCtrl.CdispOverride)))
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
	if (geState.AnimDispMode == GEANIM_NONE) break;

	if (ASegP && ASegP->Visible && priv && priv->Str &&
	    (Slen = strlen(priv->Str)))
	  {if (geState.AnimDispMode == GEANIM_BOX)
	     {GEVEC(GEBOUNDS, ASegP);
	      XDrawRectangle(geDispDev, geState.Drawable, geGC5,
			     GETPIX(ASegP->Co.x1), GETPIX(ASegP->Co.y1),
			     GETPIX(ASegP->Co.x2 - ASegP->Co.x1),
			     GETPIX(ASegP->Co.y2 - ASegP->Co.y1));
	      if (geAnimMin && abs(ASegP->Co.x2 - ASegP->Co.x1) >
		  (geAnimMin >> 1))
		{XDrawLine(geDispDev, geState.Drawable, geGC5,
			   GETPIX(ASegP->Co.x1 + ASegP->Co.x2) >> 1,
			   GETPIX(ASegP->Co.y1),
			   GETPIX(ASegP->Co.x1 + ASegP->Co.x2) >> 1,
			   GETPIX(ASegP->Co.y2));
		 XDrawLine(geDispDev, geState.Drawable, geGC5,
			   GETPIX(ASegP->Co.x1),
			   GETPIX(ASegP->Co.y1 + ASegP->Co.y2) >> 1,
			   GETPIX(ASegP->Co.x2),
			   GETPIX(ASegP->Co.y1 + ASegP->Co.y2) >> 1);
	        }
	      break;
	     }

	   GEFONT(geGC5, geFontUser[priv->Font]->fid);
    	   XDrawString(geDispDev, geState.Drawable, geGC5,
		       GETPIX(priv->Pt.x), GETPIX(priv->Pt.y),
		       priv->Str, Slen);
	   }
	break;

    case GEHDISP:
    case GEHXDISP:
	if (ASegP && ASegP->Visible && ASegP->InFrame && priv && priv->Str &&
	    (Slen = strlen(priv->Str)))
	   {if (geState.HilightMode == GEHILIGHT_BOX)
		{GEVEC(GEEXTENT, ASegP);        /* EXTENT RECTANGLE          */
		 XDrawRectangle(geDispDev, geState.Drawable, geGC5,
                        	GETPIX(geClip.x1), GETPIX(geClip.y1),
                        	GETPIX(geClip.x2 - geClip.x1),
				GETPIX(geClip.y2 - geClip.y1));
		}
	    else
	   	{GEFONT(geGC5, geFontUser[priv->Font]->fid);
    	  	 XDrawString(geDispDev, geState.Drawable, geGC5,
		             GETPIX(priv->Pt.x), GETPIX(priv->Pt.y),
		             priv->Str, Slen);
		}
	   }
	break;

    case GECREATE:
	geSegCr();
	geState.ASegP->FillHT     = geAttr.TxtBgHT;
	geState.ASegP->FillStyle  = geAttr.TxtBgStyle;
	geState.ASegP->Col        = geAttr.TxtBgCol;
	geState.ASegP->Private    = geMalloc(sizeof(struct GE_TXT));
	priv 		          = (struct GE_TXT *)geState.ASegP->Private;
	geState.ASegP->Handle[0]  = 'T';
	geState.ASegP->Handle[1]  = 'X';
	geState.ASegP->Handle[2]  = 'T';
	geState.ASegP->Handle[3]  = '\0';
	geState.ASegP->Handler    = geTxt;

	if(geState.Grav.Align || geState.APagP->Grid.XAlign ||
	   geState.APagP->Grid.YAlign)
	  {geState.Dx = geState.Dy   = 0;
	   geState.Grav.OPt.x = geState.Mouse.x;
	   geState.Grav.OPt.y = geState.Mouse.y;
	   geGenAln(geState.Mouse.x, geState.Mouse.y);
	   geState.Grav.OPt.x = geState.Mouse.x += geState.Dx;
	   geState.Grav.OPt.y = geState.Mouse.y += geState.Dy;
	  }
	priv->Pt.x = StartX       = geU.XR = geState.Mouse.x;
	priv->Pt.y = StartY       = geU.YR = geState.Mouse.y;

	priv->Just                = geJustTxt;
	priv->Font                = geAttr.Font;
	priv->Kern                = 0;
	priv->Str                 = NULL;
	if (geState.WhiteOut)
	   {GEVEC(GESETATTRERASE, geState.ASegP);}
	else
	   {priv->Col          	  = geAttr.TxtFgCol;
            priv->LineHT          = geAttr.TxtFgHT;
            priv->LineStyle       = geAttr.TxtFgStyle;
	    priv->WritingMode	  = geAttr.WritingMode;
	   }
	geMnStat(GEDISP,   59);
	if (geState.Event.type == GE_Kd && !geComposeStatus.chars_matched)
	    XPutBackEvent(geDispDev, &geState.Event);

	geEditStr(geState.ASegP);
	if (!priv || !priv->Str || !strlen(priv->Str))
	  geSegKil(&geState.ASegP);             /*Str empty...DUMP Seg !!!   */
	else                                    /* String is NON-empty - keep*/
	 {GEVEC(GEDISP, geState.ASegP);         /*it and display it          */
	  geOldState.APagP = geState.APagP;	/*- for possible RESELECT    */
	  geOldState.ASegP = geState.ASegP;     /* For "Select" keyboard key */
	 }

	geState.MsgNum = GEQUIETMSG;
	geMnStat(GEDISP, geState.MsgNum);
	break;

    case GESETATTRERASE:
	if (ASegP && ASegP->Visible && ASegP->WhiteOut &&
	    (priv = (struct GE_TXT *)ASegP->Private))
	  {priv->Col 	          = geAttr.PagCol;
	   priv->LineHT           = 100;
	   priv->LineStyle        = FillSolid;
	   priv->WritingMode      = GXcopy;
	   
	   ASegP->Col             = geAttr.PagCol;
	   ASegP->FillHT	  = 100;
	   ASegP->FillStyle       = GETRANSPARENT;
	   ASegP->FillWritingMode = GXcopy;
	  }
	break;

    case GESETATTRERASE_WHITE:
	if (ASegP && ASegP->Visible && ASegP->WhiteOut && !geState.PrintPgBg &&
	    (priv = (struct GE_TXT *)ASegP->Private))
	  {GE_SET_ATTR_ERASE_WHITE;
	  }
	break;

    case GEEDIT:
	if (!ASegP || !ASegP->Visible || !priv || !priv->Str) break;
	geMnStat(GEDISP,   59);

	if (priv->Just == GEJUSTTXTL)
	  {StartX  = priv->Pt.x;
	   StartY  = priv->Pt.y;
	  }
	else
	if (priv->Just == GEJUSTTXTC)
	  {Slen = strlen(priv->Str);
	   Temp = XTextWidth(geFontUser[priv->Font], priv->Str, Slen) >> 1;
	   StartX  = priv->Pt.x + GETSUI(Temp);
	   StartY  = priv->Pt.y;
	  }
	else
	  {Slen = strlen(priv->Str);
	   Temp = XTextWidth(geFontUser[priv->Font], priv->Str, Slen);
	   StartX  = priv->Pt.x + GETSUI(Temp);
	   StartY  = priv->Pt.y;
	  }
	geEditStr(ASegP);
	if (!priv->Str || !strlen(priv->Str))   /* String is EMPTY -         */
	  geSegKil(&geState.ASegP);             /*DUMP it !!!                */
	else                                    /* String is NON-empty - keep*/
	  GEVEC(GEDISP, geState.ASegP);         /*it and display it          */
	geState.MsgNum = GEQUIETMSG;
	geMnStat(GEDISP, geState.MsgNum);
	break;

    case GECOPY:
    case GECOPYFLP:
	if (!ASegP || !ASegP->Visible || !priv ||
	    !priv->Str || !(Slen = strlen(priv->Str))) break;
	geSegCr();
	geState.ASegP->Private    = geMalloc(sizeof(struct GE_TXT));
	privnu 		          = (struct GE_TXT *)geState.ASegP->Private;
	geGenCopySeg(geState.ASegP, ASegP);
	*privnu                        = *priv;
	privnu->Str                    = 
	    (unsigned char *)geMalloc(Slen + 1);
	strcpy(privnu->Str, priv->Str);
	GEVEC(GEBOUNDS, geState.ASegP);
	break;

#endif

    case GECREATEIMPORT:
	strcpy(Import, geUtilBuf);              /* Save TXT contents         */
	geSegCr();
	geState.ASegP->FillHT     = geAttr.TxtBgHT;
	geState.ASegP->FillStyle  = geAttr.TxtBgStyle;
	geState.ASegP->Col        = geAttr.TxtBgCol;
	geState.ASegP->Private    = geMalloc(sizeof(struct GE_TXT));
	priv 		          = (struct GE_TXT *)geState.ASegP->Private;
	geState.ASegP->Handle[0]  = 'T';
	geState.ASegP->Handle[1]  = 'X';
	geState.ASegP->Handle[2]  = 'T';
	geState.ASegP->Handle[3]  = '\0';
	geState.ASegP->Handler    = geTxt;
	priv->Col                 = geAttr.TxtFgCol;

	if(geState.Grav.Align || geState.APagP->Grid.XAlign ||
	   geState.APagP->Grid.YAlign)
	  {geState.Dx = geState.Dy   = 0;
	   geGenAln(geState.Mouse.x, geState.Mouse.y);
	   geState.Mouse.x += geState.Dx;
	   geState.Mouse.y += geState.Dy;
	  }
	priv->Pt.x = StartX       = geU.XR = geState.Mouse.x;
	priv->Pt.y = StartY       = geU.YR = geState.Mouse.y;

	if (!geFontUser[geAttr.Font])
	    geFontUser[geAttr.Font] = geLoadFont(geFntXName[geAttr.Font]);
	priv->Just                = geJustTxt;
	priv->Font                = geAttr.Font;
	priv->Kern                = 0;
	priv->Str                 = NULL;
        priv->LineHT              = geAttr.TxtFgHT;
	if (geState.WhiteOut)
	   {GEVEC(GESETATTRERASE, geState.ASegP);}
	else
	   {priv->Col          	  = geAttr.TxtFgCol;
            priv->LineHT          = geAttr.TxtFgHT;
            priv->LineStyle       = geAttr.TxtFgStyle;
	    priv->WritingMode	  = geAttr.WritingMode;
	   }
/*
	if (geAttr.TxtFgStyle != GETRANSPARENT)
	  priv->LineStyle = geAttr.TxtFgStyle;
	else
	  priv->LineStyle = FillSolid;
	priv->WritingMode	  = geAttr.WritingMode;
*/
	priv->Str                 =
	  (unsigned char *)geMalloc(strlen(Import) + 1);
	strcpy(priv->Str, Import);
	GEVEC(GEBOUNDS, geState.ASegP);
	break;

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
	  {struct GE_BX Bx;

	   if(geState.Grav.Align || geState.APagP->Grid.XAlign ||
	      geState.APagP->Grid.YAlign)
	  {geState.Grav.Lock = FALSE;
	   Bx.x1 = Bx.x4 = ASegP->Co.x1;
	   Bx.y1 = Bx.y2 = ASegP->Co.y1;
	   Bx.x2 = Bx.x3 = ASegP->Co.x2;
	   Bx.y3 = Bx.y4 = ASegP->Co.y2;
	   geGenAln(Bx.x1, Bx.y1);
	   if (!geState.Grav.Lock)
	     {geGenAln(Bx.x2, Bx.y2);
	      if (!geState.Grav.Lock)
		{geGenAln(Bx.x3, Bx.y3);
		 if (!geState.Grav.Lock)
		   {geGenAln(Bx.x4, Bx.y4);
		    if (!geState.Grav.Lock)
		       {geGenAln((Bx.x3 + Bx.x1) >> 1,
				  (Bx.y3 + Bx.y1) >> 1);
			if (!geState.Grav.Lock)
			  {geGenAln((Bx.x2 + Bx.x1) >> 1,
				     (Bx.y2 + Bx.y1) >> 1);
			   if (!geState.Grav.Lock)
			     {geGenAln((Bx.x3 + Bx.x2) >> 1,
					(Bx.y3 + Bx.y2) >> 1);
			      if (!geState.Grav.Lock)
				{geGenAln((Bx.x4 + Bx.x3) >> 1,
					   (Bx.y4 + Bx.y3) >> 1);
				 if (!geState.Grav.Lock)
				   geGenAln((Bx.x4 + Bx.x1) >> 1,
					     (Bx.y4 + Bx.y1) >> 1);
			        }
			     }
			  }
		       }
		   } 
	        }
	     }
	  }
	   if (cmd != GEGRAVTST && (geState.Dx || geState.Dy))
	     {priv->Pt.x += geState.Dx;
	      priv->Pt.y += geState.Dy;
	      GEVEC(GEBOUNDS, ASegP);
	     }
	  }
	break;

#ifdef GERAGS

    case GEGRAV:
	if (ASegP && ASegP->Visible && priv && priv->Str &&
	    strlen(priv->Str) &&
	    (geState.Grav.Pt.x >= (ASegP->Co.x1 - geState.Grav.Rad) &&
	     geState.Grav.Pt.y >= (ASegP->Co.y1 - geState.Grav.Rad) &&
	     geState.Grav.Pt.x <= (ASegP->Co.x2 + geState.Grav.Rad) &&
	     geState.Grav.Pt.y <= (ASegP->Co.y2 + geState.Grav.Rad)))
	  {struct GE_BX Bx;

	   Bx.x1 = Bx.x4 = ASegP->Co.x1;
	   Bx.y1 = Bx.y2 = ASegP->Co.y1;
	   Bx.x2 = Bx.x3 = ASegP->Co.x2;
	   Bx.y3 = Bx.y4 = ASegP->Co.y2;
	   geGenGrav(GEGRAVLOCK, Bx.x1, Bx.y1);
	   geGenGrav(GEGRAVLOCK, Bx.x2, Bx.y2);
	   geGenGrav(GEGRAVLOCK, Bx.x3, Bx.y3);
	   geGenGrav(GEGRAVLOCK, Bx.x4, Bx.y4);
	   geGenGrav(GEGRAVLOCK, ((Bx.x3 + Bx.x1) >> 1),
		     ((Bx.y3 + Bx.y1) >> 1));
	   geGenGrav(GEGRAVLOCK, ((Bx.x2 + Bx.x1) >> 1),
		     ((Bx.y2 + Bx.y1) >> 1));
	   geGenGrav(GEGRAVLOCK, ((Bx.x3 + Bx.x2) >> 1),
		     ((Bx.y3 + Bx.y2) >> 1));
	   geGenGrav(GEGRAVLOCK, ((Bx.x3 + Bx.x4) >> 1),
		     ((Bx.y3 + Bx.y4) >> 1));
	   geGenGrav(GEGRAVLOCK, ((Bx.x4 + Bx.x1) >> 1),
		     ((Bx.y4 + Bx.y1) >> 1));
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
        if (ASegP && ASegP->Visible && priv && priv->Str &&
	    (Slen = strlen(priv->Str)))
	  {geClip.x1 = priv->Pt.x;
	   width  =
	     XTextWidth(geFontUser[priv->Font], priv->Str, Slen);
   	   XTextExtents(geFontUser[priv->Font], priv->Str, Slen,
			&dir, &ascent, &descent, &overall); 
	   geClip.x2 = priv->Pt.x + GETSUI(width);
	   geClip.y1 = priv->Pt.y - GETSUI(ascent);
	   geClip.y2 = priv->Pt.y + GETSUI(descent);
	  }
	break;

#endif

    case GEBOUNDS:
        if (ASegP && ASegP->Visible && priv && priv->Str &&
	    (Slen = strlen(priv->Str)))
	  {ASegP->Co.x1 = priv->Pt.x;
	   width  =
	     XTextWidth(geFontUser[priv->Font], (char *)priv->Str, Slen);
   	   XTextExtents(geFontUser[priv->Font], (char *)priv->Str, Slen,
			&dir, &ascent, &descent, &overall); 
	   ASegP->Co.x2 = priv->Pt.x + GETSUI(width);
	   ASegP->Co.y1 = priv->Pt.y - GETSUI(ascent);
	   ASegP->Co.y2 = priv->Pt.y + GETSUI(descent);
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
          {priv->Pt.x = GETSUI(GETPIX(priv->Pt.x));
           priv->Pt.y = GETSUI(GETPIX(priv->Pt.y));
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
	  {if (ASegP->Co.x1 > geSel.x2 || ASegP->Co.y1 > geSel.y2 ||
	       ASegP->Co.x2 < geSel.x1 || ASegP->Co.y2 < geSel.y1)
		 break;

	   /*
	    * If the selection point is INSIDE
	    * of it, then PICK it, set geState.SelRadC = 0.  The test for being
	    * INSIDE is conducted by requesting the object to establish its
	    * vertex list (walk along its perimeter in small increments) and
	    * then call XPointInRegion.
	    */
	   geVn = 0;
	   GEVEC(GEADDVERT, ASegP); 
	   geVert[geVn++] = geVert[0];
	   if ((TRegion = XPolygonRegion(geVert, geVn, EvenOddRule)))
	   	{lx = (geSel.x1 + geSel.x2) >> 1;
	  	 ly = (geSel.y1 + geSel.y2) >> 1;
	  	 x  = GETPIX(lx);
	  	 y  = GETPIX(ly);
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
	   geVert[geVn++] = geVert[0];
	   if (geGenPtSelVert(0, cmd))
	   	GEVEC(GETESTPT, ASegP);
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
	   geVert[geVn++] = geVert[0];
	   if ((TRegion = XPolygonRegion(geVert, geVn, EvenOddRule)))
	   	{lx = (geSel.x1 + geSel.x2) >> 1;
	  	 ly = (geSel.y1 + geSel.y2) >> 1;
	  	 x  = GETPIX(lx);
	  	 y  = GETPIX(ly);
	  	 if (XPointInRegion(TRegion, x, y))
	   	     geGenPtSelVert(0, cmd);
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

    case GEEQUATE:
    case GEEQUATE_NOIMG:
        if(ASegP && geState.ASegP && ASegP->Handler == geState.ASegP->Handler
	   && priv && (privnu = (struct GE_TXT *)geState.ASegP->Private))
	  {geState.ASegP->Co = ASegP->Co;
	   geGenCopyAnim(geState.ASegP, ASegP);
	   geGenCopyDesc(geState.ASegP, ASegP);
	   privnu->Pt        = priv->Pt;
	   geFree(&privnu->Str, 0);
	   if (priv->Str && (Slen = strlen(priv->Str)))
	     {privnu->Str       = 
		(unsigned char *)geMalloc(Slen + 1);
	      strcpy(privnu->Str, priv->Str);
	     }
	   else
	      privnu->Str       =  NULL;
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

    case GEINQANIMATED:                         /* Increment animatedsegs ctr*/
	if (ASegP && ASegP->Live && ASegP->Visible && ASegP->AnimCtrl.Animate) 
	  geState.AnimatedSegs++;
	break;

    case GEINQXVISIBLE:                        /* Are there HIDDEN segs */
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
        if (ASegP && ASegP->Visible && priv) geAttr.Col = priv->Col;
	break;

    case GEINQFILCOL:
        if (ASegP && ASegP->Visible && priv) geAttr.Col = ASegP->Col;
	break;

    case GEINQFONT:
        if (ASegP && ASegP->Visible && priv) geAttr.Font = priv->Font;
	break;

    case GEINQLVTXT:                            /* How manyLIVE&VISBL TEXTs  */
	if (ASegP->Live && ASegP->Visible) geState.LiveSegs++;
        break;

    case GEADDTXT_GRP:                          /* Add this seg to master    */
	if (!ASegP->Visible) break;             /* group of text segments    */

	geGrp(GEJOIN, ASegP);
	break;

    case GEFLIP:
	if (geState.Constraint == GEHORIZONTAL ||
	    geState.Constraint == GENOCONSTRAINT)	
	{if (ASegP && ASegP->Visible && priv && priv->Str &&
	    (Slen = strlen(priv->Str)))
	   {lx = (ASegP->Co.x1 + ASegP->Co.x2) >> 1;
	    ly = (ASegP->Co.y1 + ASegP->Co.y2) >> 1;
	    geGenFlpPt(geState.Constraint, &lx, &ly);
	    x = (XTextWidth(geFontUser[priv->Font], priv->Str, Slen) >> 1);
	    priv->Pt.x = lx - GETSUI(x);
	   }
	}
	else if (geState.Constraint == GEVERTICAL)
        {if (ASegP && ASegP->Visible && priv)
	   {lx = (ASegP->Co.x1 + ASegP->Co.x2) >> 1;
	    ly = (ASegP->Co.y1 + ASegP->Co.y2) >> 1;
	    geGenFlpPt(geState.Constraint, &lx, &ly);
   	    XTextExtents(geFontUser[priv->Font], "X", 1, &dir,
		           &ascent, &descent, &overall); 
	    y = (ascent + descent) >> 1;
	    priv->Pt.y = ly + GETSUI(y);
       	   }
	}
	break;

#endif

    case GESCALE:
	/*
	 * Note that it is the text strings CENTER pt which will be trans-
	 *lated for scale and magnify and then the end pts of the bounding
	 *box computed off of the new center.  This is so that center
	 *alignment of text will be maintained through a scale or magnify
	 *operation.
	 */
        if (ASegP && ASegP->Visible && priv) geGenScalePt(&priv->Pt);
	break;

    case GEMAG:
    case GEMAGRESD:
        if (ASegP && ASegP && priv && geMagGrp) geGenMagPt(&priv->Pt);
	break;

    case GEROTX:
        if (ASegP && ASegP->Visible && priv)
	  geGenRotX(&priv->Pt.x, &priv->Pt.y);
	break;

    case GEROTFIXED:
        if (ASegP && ASegP->Visible && priv)
	  {if (( geRot.Clockwise && geRot.Beta  == 90.) ||
	       (!geRot.Clockwise && geRot.Alpha == 90.))
	     	geGenRot90(&priv->Pt.x, &priv->Pt.y);
	   else geGenRotX (&priv->Pt.x, &priv->Pt.y);
	  }
	break;

    case GEADDVERT:
	/*
	 * Add the coords of this object to the vertex list - used for
	 * selection.
	 */
	if (ASegP && ASegP->Visible && priv && geVn < GEMAX_PTS)
	  {GESTOREVERT(geVn, GETPIX(ASegP->Co.x1), GETPIX(ASegP->Co.y1));
	   if (++geVn >= GEMAX_PTS) break;
	   GESTOREVERT(geVn, GETPIX(ASegP->Co.x2), GETPIX(ASegP->Co.y1));
	   if (++geVn >= GEMAX_PTS) break;
	   GESTOREVERT(geVn, GETPIX(ASegP->Co.x2), GETPIX(ASegP->Co.y2));
	   if (++geVn >= GEMAX_PTS) break;
	   GESTOREVERT(geVn, GETPIX(ASegP->Co.x1), GETPIX(ASegP->Co.y2));
	   geVn++;
	  }
	break;

#ifdef GERAGS

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

    case GEPUTTXTFG:
	if (ASegP && ASegP->Visible && priv)
	   {if (geGetPutFlgs & GEGETPUTCOL)
		priv->Col    	       = geSampleAttr.TxtFgCol;
	    if (geGetPutFlgs & GEGETPUTHT)
		priv->LineHT 	       = geSampleAttr.TxtFgHT;
	    if (geGetPutFlgs & GEGETPUTSTYLE)
		priv->LineStyle       = geSampleAttr.TxtFgStyle;
	    if (geGetPutFlgs & GEGETPUTWRITEMODE)
	    	priv->WritingMode = geSampleAttr.WritingMode;
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
	   }
	break;

    case GEPUTFONT:                             /* Set the font              */
	if (ASegP && ASegP->Visible && priv) 
	  priv->Font = geSampleAttr.Font;
	break;

    case GEGETFONT:                             /* Get the font              */
	if (ASegP && ASegP->Visible && priv) 
	  geSampleAttr.Font = priv->Font;
	break;

#endif

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

    case GEGETSEG_COL:
        if (ASegP && ASegP->Col.RGB.pixel == geGenCol.pixel ||
	    (priv && priv->Col.RGB.pixel == geGenCol.pixel))
	  geState.ASegP = ASegP;
	break;

    case GEKILL:
	if(ASegP) geSegKil(&ASegP);
	break;

    case GERELPRIV:
	if (priv && priv->Str && strlen(priv->Str))
	  geFree(&priv->Str, 0);
	break;

    case GEKILPRIV:
	if (ASegP->Private)
	  geFree(&ASegP->Private, sizeof(struct GE_TXT));
	break;

	break;

    case GESCRUB:
	if (ASegP && priv && priv->Str && !strlen(priv->Str)) geSegKil(&ASegP);
	break;

    case GEWRITE:
    case GEREAD:
     /*
      * See if it's ascii or ddif
      */
     if (geReadWriteType == GE_RW_DDIF)
       geTxtIODDIF(cmd, ASegP);
     else
       geTxtIO(cmd, ASegP);
	break;

    default:
	break;
   }
}

#ifdef GERAGS

/* GEEDITSTR	             	       Text string editing
 *
 * History:
 * GNE 02/18/87 Created
 */
static
geEditStr(ASegP)
struct GE_SEG	*ASegP;
{                       
#define GEREDISP \
  {if (priv->Just == GEJUSTTXTC && priv && priv->Str && \
       (Slen = strlen(priv->Str))) \
     {Temp       = XTextWidth(geFontUser[priv->Font], \
			      priv->Str, Slen) >> 1; \
      priv->Pt.x = PrivX = StartX - GETSUI(Temp); \
     } \
   else \
     if (priv->Just == GEJUSTTXTR && priv && priv->Str && \
	 (Slen = strlen(priv->Str))) \
       {Temp       = XTextWidth(geFontUser[priv->Font], priv->Str, \
				Slen); \
	priv->Pt.x = PrivX = StartX - GETSUI(Temp); \
       }  \
   else \
     if (priv && priv->Str && (Slen = strlen(priv->Str))) \
   Temp            = XTextWidth(geFontUser[priv->Font], \
				priv->Str, Slen); \
   geWarpCur(1); \
   GEVEC(GEXDISP,  ASegP); \
  }

#define GEADDCHAR(mp_c) \
  {if (geState.Insert && CurI < MaxI) \
     {strncpy(WorkBuf, buf, CurI); \
      WorkBuf[CurI]     = mp_c; \
      WorkBuf[CurI + 1] = '\0'; \
      strcat (WorkBuf, &buf[CurI]); \
      strcpy (buf, WorkBuf); \
      CurI++; \
      MaxI++; \
     } \
   else \
     {if (CurI < MaxI) buf[CurI++] = mp_c; \
      else \
	{buf[CurI++] = mp_c; \
         buf[CurI]   = '\0'; \
         MaxI++; \
        } \
     } \
  }

#define GEDELCHAR \
  {if (CurI) \
    {if (geState.Insert && CurI < MaxI) \
       {strcpy(WorkBuf, buf); \
	strcpy (&buf[CurI - 1], &WorkBuf[CurI]); \
        CurI--; \
        MaxI--; \
       } \
     else \
       {if (CurI < MaxI) buf[--CurI] = ' '; \
	else \
	  {buf[--CurI] = '\0'; \
	   MaxI--; \
	  } \
       } \
    } \
  }

char		*p;
unsigned char   Str[2], WorkBuf[1000], cr;
short           Done;
int             j, nchar, c, ix, iy, dxc, dh;
long            XSave, YSave;
struct GE_TXT   *priv;
KeySym		KSym;

Done        = FALSE;
p	    = (char *)buf;
priv        = (struct GE_TXT *)ASegP->Private;
PrivX       = priv->Pt.x;
PrivY       = priv->Pt.y;

if (priv->Str)
  {strcpy(buf, priv->Str);
   geFree(&priv->Str, 0);
  }
else
   buf[0] = '\0';

priv->Str   = buf;
cr          = FALSE;
/*
 * Get some basic font metrics info.  Set-up cursor.  If editing an
 * existing string, locate upper left corner of char closest to user's
 * pick and position the cursor there.
 */
PrivFont = priv->Font;
XTextExtents(geFontUser[priv->Font], "Xy", 2, &dir, &ascent, &descent,
	     &overall); 
Curse.h  = ascent + descent;
Curse.y  = GETPIX(StartY) - ascent;
MaxI	 = 0;

if (*p)
  {/*
    * String is NOT empty
    */
   CurXoff = GETPIX(geState.Mouse.x - priv->Pt.x);
   MaxI    = strlen(p);
   /*
    * Determine index into string based on cursor location at the time of
    * MB2
    */
   CurI    = 0;
   if (CurXoff > 0)
	{while (buf[CurI])
     	   {dxc = XTextWidth(geFontUser[priv->Font], p, (CurI + 1));
     	    if (dxc > CurXoff)
		{if (geState.Insert && CurI && CurI < MaxI)
		   {dh = (XTextWidth(geFontUser[priv->Font], &buf[CurI], 1)) >> 1;
		    if ((dxc - CurXoff) < dh) CurI++;
		   }
		 break;
		}
      	    CurI++;
     	   }
	}
   /*
    * Calculate cursor x position and width based on string offset
    */
   if (MaxI > 0 && CurI > 0)
	{if (CurI < MaxI)			
	   {Curse.x  = GETPIX(priv->Pt.x) +
     		       XTextWidth(geFontUser[priv->Font], buf, CurI);
	    Curse.w  = XTextWidth(geFontUser[PrivFont], &buf[CurI], 1);
	   }
	 else
	   {CurI    = MaxI;
	    Curse.x = GETPIX(priv->Pt.x) +
     		       XTextWidth(geFontUser[priv->Font], buf, CurI);
	    Curse.w = geFontUser[PrivFont]->ascent +
		      geFontUser[PrivFont]->descent;
	   }
	}
   else
	{CurI     = 0;
	 Curse.x  = GETPIX(priv->Pt.x);
   	 Curse.w  = XTextWidth(geFontUser[priv->Font], buf, 1);
	}
  }
else
  {/*
    * String IS empty
    */
   CurI     = MaxI = 0;
   Curse.w  = geFontUser[priv->Font]->ascent + geFontUser[priv->Font]->descent;
   Curse.x  = GETPIX(StartX);
  }
Curse.ox  = Curse.x;
Curse.oy  = Curse.y;
Curse.dx  = Curse.h >> 3;
/*
 * Display initial cursor
 */
geWarpCur(0);
/*
 * Wait for a keyboard-button event
 *
 * Note:  There are basically 3 ways out of this loop -
 *        1 - button press
 *        2 - carriage return
 *        3 - exceeding the max number of chars (GE_MAX_MSG_CHAR)
 */

while (!Done)
  {geMaskEvent(GE_MKd|GE_MBd|GE_MWu|GE_MWr, &geState.Event);

   if (geState.Event.type == GE_Bd)
     {geState.Mouse.x = StartX;
      if (geState.Event.xbutton.button == GEBUT_R)
	{GEVEC(GEXDISP,  ASegP);
	 *p  = '\0';                            /* R = CANCEL string         */
        }
      break;                                    /* Any mouse button = TERM   */
     }
   if (geState.Event.type == GE_Wu || geState.Event.type == GE_Wr)
     {geDispatch();
      continue;
     }
   /*
    * Some key has been pressed - if compose status is on, let the server
    * figure out the resultant character, so just continue till that's done
    */
   nchar = XLookupString(&geState.Event, Str, 1, &KSym, &geComposeStatus);
   if (geComposeStatus.chars_matched) continue;

   if (p) MaxI = strlen(p);
   else   MaxI = 0;
   if (MaxI >= GE_MAX_MSG_CHAR)                    break;
   c = (int)*(Str);

   switch (KSym)
     {case XK_Return: 			        /* CARRIAGE RETURN	     */
      case XK_KP_Enter:			        /* ENTER		     */
       /*
	* Snap to grid
	*/
       if (geState.APagP->Grid.XAlign || geState.APagP->Grid.YAlign)
       	   {geState.Dx = StartX - geState.Mouse.x;
	    geState.Dy = (priv->Pt.y + GETSUI(ascent + descent)) - 
			 geState.Mouse.y + (geState.APagP->Grid.MinorY >> 1);
       	    geGenGridAln(geState.Mouse.x, geState.Mouse.y);
       	    geState.Mouse.x += geState.Dx;
       	    geState.Mouse.y += geState.Dy;
	    geOldState.Mouse = geState.Mouse;
	   }
       else
           {geOldState.Mouse.y = geState.Mouse.y = 
	 	priv->Pt.y + GETSUI(ascent + descent);
            geOldState.Mouse.x = geState.Mouse.x = StartX;
	   }
       ix = GETPIX(geState.Mouse.x);
       iy = GETPIX(geState.Mouse.y);
       XWarpPointer(geDispDev, None, geState.Window, 0, 0, 0, 0, ix, iy);

       Done = TRUE;
       continue;

      case XK_Delete:			        /* Delete a char from end    */
       if (MaxI)
	 {GEVEC(GEXDISP, ASegP);
	  GEDELCHAR;
	 }
       else   continue;
       break;

      case XK_Tab:
       if (MaxI)
	 GEVEC(GEXDISP, ASegP);
       for (j = geState.Tab; j > 0; j--)
	 GEADDCHAR(' ');
       break;

      case XK_Left:
       if (CurI)
	 {CurI--;
	  geWarpCur(1);
	 }
       continue;
       break;

      case XK_Right:
       if (CurI < MaxI)
	 {CurI++;
	  geWarpCur(1);
	 }
       continue;
       break;

      case XK_Control_L:
      case XK_Control_R:
       /*
	* Wait for next key to determine what to do next (loop until
	* control key is released).
	* The following keys are acceptable:
	*         a  Toggles Insert-Overstrike mode
	*         c  CANCEL line
	*         e  Go to end       of line
	*         h  Go to beginning of line
	*         z  Drop line (like CR)
	*/
       geMaskEvent(GE_MKd|GE_MBd, &geState.Event);
       /*
	* ? key mask has a value of 6 when CapsLock is ON and CTRL key is
	* held down - well, it works
	*/
       while (!Done && geState.Event.type == GE_Kd &&
	      (geState.Event.xkey.state == ControlMask ||
	       geState.Event.xkey.state == 6))
	 {/*
	   * Control key functions
	   */
	  nchar = XLookupString(&geState.Event, Str, 1, &KSym, &geComposeStatus);

	  switch (KSym)
	    {case XK_a:                         /* Toggle between inser-ostrk*/
	     case XK_A:
	      geWarpCur(0);
	      geState.Insert = geState.Insert ? FALSE : TRUE;
	      geWarpCur(0);
	      break;
	    case XK_c:                          /* CANCEL text entry         */
	    case XK_C:
	      GEVEC(GEXDISP,  ASegP);
	      *p  = '\0';
	      Done = TRUE;
	      break;
	    case XK_e:                          /* GOTO END of line          */
	    case XK_E:
	      CurI = MaxI;
	      geWarpCur(1);
	      break;
	    case XK_h:                          /* GOTO BEGINNING of line    */
	    case XK_H:
	      CurI = 0;
	      geWarpCur(1);
	      break;
	    case XK_k:                          /* KILL to end of line       */
	    case XK_K:
	      GEVEC(GEXDISP,  ASegP);
	      strcpy(KillBuf, &buf[CurI]);
	      buf[CurI] = '\0';
	      MaxI      = CurI;
	      GEREDISP;
	      geWarpCur(1);
	      break;
	    case XK_y:                          /* YANK back from kill buff  */
	    case XK_Y:
	      if (*KillBuf)
		{GEVEC(GEXDISP,  ASegP);
		 strcpy(&buf[CurI], KillBuf);
		 CurI += strlen(KillBuf);
		 MaxI  = CurI;
		 GEREDISP;
		 geWarpCur(1);
	        }
	      break;
	    case XK_z:                          /* DROP text in place        */
	    case XK_Z:
	      geOldState.Mouse.y = geState.Mouse.y = 
		priv->Pt.y + GETSUI(ascent + descent);
	      geOldState.Mouse.x = geState.Mouse.x = StartX;
	      ix = GETPIX(geState.Mouse.x);
	      iy = GETPIX(geState.Mouse.y);
	      Done = TRUE;
	      break;
	    case XK_Up:                         /* MOVE text object on page  */
	    case XK_Down:
	    case XK_Left:
	    case XK_Right:
	      XSave = geOldState.Mouse.x = geState.Mouse.x;
	      YSave = geOldState.Mouse.y = geState.Mouse.y;
	      KSym = geArrowTst(ASegP);
	      StartX += (geState.Mouse.x - XSave);
	      StartY += (geState.Mouse.y - YSave);
	      PrivX  += (geState.Mouse.x - XSave);
	      PrivY  += (geState.Mouse.y - YSave);
	      geWarpCur(1);
	      break;
    
	    default:
	      XBell(geDispDev, GEBEEP_ERR);
	      break;
	     }
	  if (!Done)
	    geMaskEvent(GE_MKd|GE_Bd, &geState.Event);
	 }
       if (!Done)
	 XPutBackEvent(geDispDev, &geState.Event);
       continue;

      default:                                /* Add a char to end of str  */
       if ((GEIsKeypadKey(KSym)) || (GEIsAltKeypadKey(KSym)))
	 {if (MaxI) GEVEC(GEXDISP, ASegP);
	  GEADDCHAR((unsigned char)c);
	 }
       else continue;
       break;
      }					      /* SWITCH		             */

   GEREDISP;
  }						/* Next event		     */

if (p && *p)
  {priv->Str = (unsigned char *)geMalloc(strlen(p) + 1);
   strcpy(priv->Str, p);
   GEVEC(GEBOUNDS, ASegP);
   GEVEC(GEXDISP,  ASegP);
  }
else
   priv->Str = NULL;
/*
 * Clean up cursor
 */
geWarpCur(0);

}

/* WARPCUR	             	       Manage the text cursor
 *
 * History:
 * GNE 11/17/888 Created
 */
static
geWarpCur(Update)
int Update;
{                       
int delx, bufoff;

if (MaxI) bufoff = min(CurI, (MaxI - 1));
else	  bufoff = 0;

if (geState.Insert)
  {/*
    * Xor old one
    */
    XDrawLine(geDispDev, geState.Drawable, geGC5,
	      Curse.ox - Curse.dx, Curse.oy,
	      Curse.ox + Curse.dx, Curse.oy);
    XDrawLine(geDispDev, geState.Drawable, geGC5,
	      Curse.ox, Curse.oy,
	      Curse.ox, Curse.oy + Curse.h);
    XDrawLine(geDispDev, geState.Drawable, geGC5,
	      Curse.ox - Curse.dx, Curse.oy + Curse.h,
	      Curse.ox + Curse.dx, Curse.oy + Curse.h);

   if (Update)
     {/*
       * Xor new one
       */
       if (MaxI > 0 && CurI > 0)
	{if (CurI < MaxI)
	    {CurXoff = XTextWidth(geFontUser[PrivFont], buf, CurI);
	     Curse.w = XTextWidth(geFontUser[PrivFont], &buf[bufoff], 1);
	    }
	 else
	    {CurI    = MaxI;
	     CurXoff = XTextWidth(geFontUser[PrivFont], buf, CurI);
	     Curse.w = geFontUser[PrivFont]->ascent +
		       geFontUser[PrivFont]->descent;
	    }
	}
       else
	{CurI     = 0;
	 CurXoff  = 0;
   	 if (buf[0])
	    Curse.w  = XTextWidth(geFontUser[PrivFont], buf, 1);
	 else
	    Curse.w  = geFontUser[PrivFont]->ascent +
		       geFontUser[PrivFont]->descent;
	}

       Curse.x  = Curse.ox = GETPIX(PrivX) + CurXoff;
       Curse.y  = Curse.oy = GETPIX(PrivY) - ascent;
       XDrawLine(geDispDev, geState.Drawable, geGC5,
		 Curse.ox - Curse.dx, Curse.oy,
		 Curse.ox + Curse.dx, Curse.oy);
       XDrawLine(geDispDev, geState.Drawable, geGC5,
		 Curse.ox, Curse.oy,
		 Curse.ox, Curse.oy + Curse.h);
       XDrawLine(geDispDev, geState.Drawable, geGC5,
		 Curse.ox - Curse.dx, Curse.oy + Curse.h,
		 Curse.ox + Curse.dx, Curse.oy + Curse.h);
     }
  }
else
  {/*
    * Xor old one
    */
   XDrawRectangle(geDispDev, geState.Drawable, geGC5,
		  Curse.ox, Curse.oy, Curse.w, Curse.h);
   if (Update)
     {/*
       * Xor new one
       */
       if (MaxI > 0 && CurI > 0)
	{if (CurI < MaxI)
	    {CurXoff = XTextWidth(geFontUser[PrivFont], buf, CurI);
	     Curse.w = XTextWidth(geFontUser[PrivFont], &buf[bufoff], 1);
	    }
	 else
	    {CurI    = MaxI;
	     CurXoff = XTextWidth(geFontUser[PrivFont], buf, CurI);
	     Curse.w = geFontUser[PrivFont]->ascent +
		       geFontUser[PrivFont]->descent;	
	    }
	}
       else
	{CurI     = 0;
	 CurXoff  = 0;
   	 if (buf[0])
	    Curse.w  = XTextWidth(geFontUser[PrivFont], buf, 1);
	 else
	    Curse.w  = geFontUser[PrivFont]->ascent +
		       geFontUser[PrivFont]->descent;	
	}

       Curse.x  = Curse.ox = GETPIX(PrivX) + CurXoff;
       Curse.y  = Curse.oy = GETPIX(PrivY) - ascent;
       XDrawRectangle(geDispDev, geState.Drawable, geGC5,
		      Curse.ox, Curse.oy, Curse.w, Curse.h);
      }
  }
}

#endif

#ifdef VMS
#include <unixio.h>
#endif
#ifdef __osf__
#include <sys/file.h>
#else
#include <file.h>
#endif

extern char *getenv();

geDECtalk(str, len)
    char	*str;
    int		 len;
    
{
    int		file_desc;
    char	new_str[1024], *dectalk_device;

#ifdef VMS
    dectalk_device = getenv("GRA$DECTALK_DEVICE");
#else
    dectalk_device = getenv("GRA_DECTALK_DEVICE");
#endif

    if (!dectalk_device) return;


    file_desc = open(dectalk_device, O_WRONLY, 0);

    if (file_desc == -1) {
	fprintf(stderr, "Can't open channel to DECtalk!\n");
	return;
    }

    strncpy(new_str, str, len);
    new_str[len] = '\n';
    new_str[len+1] = '\0';

    write(file_desc, new_str, strlen(new_str));

    close(file_desc);
}
