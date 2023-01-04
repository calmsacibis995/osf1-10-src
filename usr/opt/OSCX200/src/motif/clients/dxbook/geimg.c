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
static char SccsId[] = "@(#)geimg.c	1.12\t4/20/89";
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
**	GEIMG	             	       Image object handler
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
**	GNE 02/02/88 Created
**
**--
**/

#include "geGks.h"

#ifdef GEISL

#ifdef VMS
#include <img$def.h>
#else
#include <img/img_def.h>
#endif

#endif

extern unsigned long 	geAllocColor();
extern char          	*geMalloc();
extern XImage        	*geImgCr(), *geXCrImg(), *geXImgToSing(),
  			*geXImgToSingOrdDith();
extern Region		geXPolygonRegion();

static char		*error_string;

geImg(cmd, ASegP)
long     	cmd;
struct GE_SEG	*ASegP;
{                       
#define GECHUNK 512

static int	ttag = GEOBJEDIT;

char            *ptr, *Private;   
int 		XSav, YSav, temp, xoff, yoff, x, y, xt,
                wt, xtoff, i, numbytes, (*Handler)(), NumFrames;
long		StateSav;

unsigned char	*ptru;
unsigned int    width, height, num, widtho, heighto, widthcropo, heightcropo;
unsigned long   islfid, nu_islfid, lx, ly;
float           xscale, yscale, rotangle, Mag;

struct GE_IMG   *priv, *privnu;
struct GE_SEG   *Seg0, *SegP, *ASegPSav, *XSegP;
struct GE_PAG   *PagP;
struct GE_PT    Pta;
struct GE_BX	Bxo;
struct GE_CO	Bxsav;

XPoint	        v[5];
XImage          *ImgPtr;
Pixmap          ClipMask;
GC		TempGC;
Region		TRegion;

if (ASegP)
  {if (!ASegP->Live && cmd != GEINQIMGREF  && cmd != GEKILL    &&
                       cmd != GERELPRIV    && cmd != GEKILPRIV &&
                       cmd != GEGETSEG_COL && cmd != GEXDEL)
     return;
   priv = (struct GE_IMG *)ASegP->Private;
  }

switch(cmd)
  {case GEDISP:
        if (priv && priv->ImgPtr && ASegP && ASegP->Visible &&
	    geState.Window && ASegP->InFrame &&
	    (!ASegP->ConLine || geRun.RagsCalling) &&
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
	     {xoff = GETPIX(priv->Bx.x1);
	      yoff = GETPIX(priv->Bx.y1),
	      x =      GETPIX(priv->Pt.x) + GETPIX(priv->Bx.x1);
	      y =      GETPIX(priv->Pt.y) + GETPIX(priv->Bx.y1);
	      width =  GETPIX(priv->Bx.x3 - priv->Bx.x1);
	      height = GETPIX(priv->Bx.y3 - priv->Bx.y1);
	      width  = abs(width);
	      height = abs(height);
	      /*
	       * Make some final visibility checks
	       */
	      if ((width <= 0 || height <= 0) ||	/* CORRUPT	     */
		  (x > geState.APagP->Surf.width ||	/* To the RIGHT	     */
		   y > geState.APagP->Surf.height) ||	/* BELOW	     */
		  ((x + width) < 0 || (y + height) < 0))/* LEFT or ABOVE     */
		{ASegP->InFrame = FALSE;
		 break;
	        }
	      /*
	       * Calculate "visible" region of image and que only that much to
	       * server.  This is probably a bit more efficient than having the
	       * server figure it out - but more importantly it avoids the
	       * problem the "color" server has with very large images.
	       */
	      if (x < 0)
		{xoff   += abs(x);
		 width  -= abs(x);
		 x       = 0;
	        }
	      if (y < 0)
		{yoff   += abs(y);
		 height -= abs(y);
		 y       = 0;
	        }
	      if ((x +  width) > geState.APagP->Surf.width)
		width  = geState.APagP->Surf.width  - x;
	      if ((y + height) > geState.APagP->Surf.height)
		height = geState.APagP->Surf.height - y;

	      GEMODE(geGCImg, priv->WritingMode);
	      GEFORG(geGCImg, priv->ColForeGround.RGB.pixel);
	      GEBACK(geGCImg, priv->ColBackGround.RGB.pixel);

	      if (ASegP->FillStyle != FillOpaqueStippled &&
		  priv->ImgPtr->depth == 1)
		{/*
		  * Want a TRANSLUCENT image, i.e. only want to write the
		  * foreground bits, so going to have to create pixmap mask
		  * with the background set to 0 and the foreground set to 1
		  */

		  geErr = 0;	      
		  ClipMask = XCreatePixmap(geDispDev, geRgRoot.self,
					   width, height, 1);
		  XSync(geDispDev, FALSE);/* Make sure if ERR, it gets reprtd*/
		  if (geErr == BadAlloc)
		    {/*
		      * Pixmap alloc for ClipMask has failed, so do the best we
		      * can and display the image as Opaque
		      */
		      XSetTSOrigin(geDispDev, geGCImg, 0, 0);
		      XPutImage(geDispDev, geState.Drawable, geGCImg,
				priv->ImgPtr, xoff, yoff, x, y, width, height);
		      break;
		     }

		  geGCV.foreground  = 1;
		  geGCV.background  = 0;

		  /*
		   * For compatability with V5 image mangling
		   */
		  if ((priv->Flags & GEV5IMG))
		    {temp     = (priv->ColForeGround.RGB.red +
				 priv->ColForeGround.RGB.green +
				 priv->ColForeGround.RGB.blue) / 3;
		     if (temp < (GEMAXUSHORT >> 1))
		       {geGCV.foreground  = 1;
			geGCV.background  = 0;
			/*
			 * The image has been modified so it is no longer
			 * a V5 image.
			 */
			priv->Flags ^= GEV5IMG;
		       }
		     else
		       {geGCV.foreground  = 0;
			geGCV.background  = 1;
			GEBACK(geGCImg, priv->ColForeGround.RGB.pixel);
			GEFORG(geGCImg, priv->ColBackGround.RGB.pixel);
		       }
		    }

		  TempGC   = XCreateGC(geDispDev, ClipMask,
				       GCForeground|GCBackground,
				       &geGCV);

		  xt    = 0;
		  wt    = width;
		  xtoff = xoff;
		  XPutImage(geDispDev, ClipMask, TempGC, priv->ImgPtr,
			    xtoff, yoff, xt, 0, wt, height);
		  GEFILSTYLE(geGCImg, FillStippled);
		  XSetStipple(geDispDev, geGCImg, ClipMask);
		  XSetTSOrigin(geDispDev, geGCImg, x, y);

		  XFillRectangle(geDispDev, geState.Drawable, geGCImg, x, y,
				 width, height);

		  XFreePixmap(geDispDev, ClipMask);
		  XFreeGC(geDispDev, TempGC);
		  ClipMask = 0;
		 }
	      else
		{if (priv->Flags & GEV5IMG)
		   /*
		    * The image has been modified so it is no longer
		    * a V5 image.
		    */
		   priv->Flags ^= GEV5IMG;
		 XSetTSOrigin(geDispDev, geGCImg, 0, 0);

		 XPutImage(geDispDev, geState.Drawable, geGCImg, priv->ImgPtr,
			   xoff, yoff, x, y, width, height);
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

#ifdef GERAGS

    case GEXDISP:
	if (geState.AnimDispMode != GEANIM_NONE && priv && priv->ImgPtr &&
	    ASegP && ASegP->Visible)
	 {width =  GETPIX(priv->Bx.x3 - priv->Bx.x1);
	  height = GETPIX(priv->Bx.y3 - priv->Bx.y1);
	  width  = abs(width);
	  height = abs(height);
	  if (!width || !height) break;

	  if (geState.AnimDispMode != GEANIM_TRUE)
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

	  GEFORG(geGCImg, GEXORPIXEL);
	  GEBACK(geGCImg, geBlack.pixel);
	  GEMODE(geGCImg, GXxor);
	  xoff = GETPIX(priv->Bx.x1);
	  yoff = GETPIX(priv->Bx.y1),
	  x =      GETPIX(priv->Pt.x) + GETPIX(priv->Bx.x1);
	  y =      GETPIX(priv->Pt.y) + GETPIX(priv->Bx.y1);
	  /*
	   * Calculate "visible" region of image and que only that much to
	   * server.  This is probably a bit more efficient than having the
	   * server figure it out - but more importantly it avoids the
	   * problem the "color" server has with very large images.
	   */
	  if (x < 0)
	    {xoff   += abs(x);
	     width  -= abs(x);
	     x       = 0;
	    }
	  if (y < 0)
	    {yoff   += abs(y);
	     height -= abs(y);
	     y       = 0;
	    }
	  if ((x +  width) > geState.APagP->Surf.width)
	    width  = geState.APagP->Surf.width  - x;
	  if ((y + height) > geState.APagP->Surf.height)
	    height = geState.APagP->Surf.height - y;

	  XPutImage(geDispDev, geState.Drawable, geGCImg, priv->ImgPtr,
		    xoff, yoff, x, y, width, height);
	 }
	break;

    case GEHDISP:
    case GEHXDISP:
	if (ASegP && ASegP->Visible && ASegP->InFrame)
	   {GEVEC(GEEXTENT, ASegP);             /* EXTENT RECTANGLE          */
	    XDrawRectangle(geDispDev, geState.Drawable, geGC5,
                           GETPIX(geClip.x1), GETPIX(geClip.y1),
                           GETPIX(geClip.x2 - geClip.x1),
			   GETPIX(geClip.y2 - geClip.y1));
	   }
	break;

    case GECREATE:
	geSegCr();
	geState.ASegP->Private    = geMalloc(sizeof(struct GE_IMG));
	priv 		          = (struct GE_IMG *)geState.ASegP->Private;
	geState.ASegP->Handle[0]  = 'I';
	geState.ASegP->Handle[1]  = 'M';
	geState.ASegP->Handle[2]  = 'G';
	geState.ASegP->Handle[3]  = '\0';
	geState.ASegP->Handler    = geImg;
	ASegP                     = geState.ASegP;
	ASegP->FillStyle  	  = FillSolid;

	switch (geImgType)
	  {case GEIMGTYPE_X:
	    geGenFileExt(geUtilBuf, geDefProofX, FALSE);
	    break;
	  case GEIMGTYPE_FSE:
	    geGenFileExt(geUtilBuf, geDefProofFSE, FALSE);
	    break;
	  case GEIMGTYPE_SIX:
	    geGenFileExt(geUtilBuf, geDefProofSix, FALSE);
	    break;
	  case GEIMGTYPE_DDIF:
	    geGenFileExt(geUtilBuf, geDefProofDDIF, FALSE);
	    break;
	  }

	geGenFileDir(geUtilBuf, geDefDirInImg);
 	priv->FileName = (unsigned char *)geMalloc(strlen(geUtilBuf) + 1);
	strcpy(priv->FileName, geUtilBuf);

	geMnStat(GEDISP, 86);		        /* Put up message that       */
	geSetWaitCurs(TRUE);
	XFlush(geDispDev);			/*we're doing it             */
	priv->ImgPtr = geImgCr(priv->FileName, geImgType);
	geSetWaitCurs(FALSE);

	if (priv->ImgPtr == NULL)
	  {geFree(&priv->FileName, 0);
	   geSegKil(&ASegP);
	   break;
	  }
	/*
	 * Request location
	 */
	geMnStat(GEDISP, 38);
	geMouseXY(geState.APagP->Surf.self);
	geGenBx.x1                = GETPIX(geState.Mouse.x);
	geGenBx.y1                = GETPIX(geState.Mouse.y);
	geGenBx.x2                = geGenBx.x1 + priv->ImgPtr->width;
	geGenBx.y2                = geGenBx.y1 + priv->ImgPtr->height;
	geState.ASegP->Live       = FALSE;
	geGenBoxMove(geState.APagP->Surf.self, FALSE);
	geState.ASegP->Live       = TRUE;

	geState.Mouse.x           = GETSUI(geGenBx.x1);
	geState.Mouse.y           = GETSUI(geGenBx.y1);

	if(geState.Grav.Align || geState.APagP->Grid.XAlign ||
	   geState.APagP->Grid.YAlign)
	  {geState.Dx = geState.Dy   = 0;
	   geGenAln(geState.Mouse.x, geState.Mouse.y);
	   geState.Mouse.x += geState.Dx;
	   geState.Mouse.y += geState.Dy;
	   geState.Grav.OPt.x = geState.Mouse.x += geState.Dx;
	   geState.Grav.OPt.y = geState.Mouse.y += geState.Dy;
	  }
	priv->Pt.x                = geU.XR = geState.Mouse.x;
	priv->Pt.y                = geU.YR = geState.Mouse.y;
	priv->Bx.x1               = 0;
	priv->Bx.y1               = 0;
	priv->Bx.x3               = priv->Bx.x1 + GETSUI(priv->ImgPtr->width);
	priv->Bx.y3               = priv->Bx.y1 + GETSUI(priv->ImgPtr->height);
	priv->Bx.x2               = priv->Bx.x3;
	priv->Bx.y2               = priv->Bx.y1;
	priv->Bx.x4               = priv->Bx.x1;
	priv->Bx.y4               = priv->Bx.y3;

	priv->Type                = geImgType;
	priv->WritingMode         = geAttr.WritingMode;
	priv->ColForeGround    	  = geAttr.ImgFgCol;
	priv->ColBackGround    	  = geAttr.ImgBgCol;
	priv->Mirrored            = priv->RotAngle = 0;

	priv->Flags               = 0;
	/*
	 * Opacity needs to be continued to be tracked in Flags until no longer
	 * writing pre version 7 metafiles.
	 */
	if (ASegP->FillStyle == GETRANSPARENT ||
	    ASegP->FillStyle == FillStippled)
	  priv->Flags |= GEIMGOPACITY_CLR;

	GEVEC(GEBOUNDS, geState.ASegP);
	GEVEC(GEDISP,   geState.ASegP);
	break;

    case GEOBJEDIT:
    case GEEDIT:
  	geMnU(NULL, &ttag, NULL); 
	GEVEC(GEXDISP,   geState.ASegP);
	geMnStat(GEDISP, 35);			/* Request action	     */
	Bxsav.x1      = GETPIX(priv->Pt.x) + GETPIX(priv->Bx.x1);
	Bxsav.y1      = GETPIX(priv->Pt.y) + GETPIX(priv->Bx.y1);
	Bxsav.x2      = GETPIX(priv->Pt.x) + GETPIX(priv->Bx.x3);
	Bxsav.y2      = GETPIX(priv->Pt.y) + GETPIX(priv->Bx.y3);
	geGenBx.x1    = -1;
	geGenBx.y1    = -1;
	geGenBx.x2    = -1;
	geGenBx.y2    = -1;
	GEVEC(GECROP,   geState.ASegP);
	GEVEC(GEBOUNDS, geState.ASegP);
	GEVEC(GEEXTENT, geState.ASegP);
	GEVEC(GEDISP,   geState.ASegP);
	ASegPSav      = geState.ASegP;        	/* Null out ASegP, so that it*/
	geState.ASegP = NULL;                   /* will be included in CDISP */
	/*
	 * Wait for start point - MB2 will be taken at this time to mean
	 * RESET crop box to MAX.
	 */
  	geMaskEvent(GE_MBd|GE_MKd, &geState.Event);
   	if (geState.Event.xbutton.button != GEBUT_M)
	    {XPutBackEvent(geDispDev, &geState.Event);
	     geGenBox(geState.APagP->Surf.self, FALSE, FALSE);
	                                        /* Track user's box        */
	    }
	geState.ASegP = ASegPSav;
	if (geState.Event.type == GE_Bd &&
	    geState.Event.xbutton.button == GEBUT_R)
	    {geMnStat(GEDISP, 74);
	     GEVEC(GEERASE,  geState.ASegP);
 	     break;
	    }
		
	GEVEC(GEERASE,  geState.ASegP);
	GEVEC(GECROP,   geState.ASegP);
	GEVEC(GEBOUNDS, geState.ASegP);
	break;	break;

    case GECOPY:
    case GECOPYFLP:
	if (!ASegP || !ASegP->Visible) break;
	geSegCr();
	geState.ASegP->Private    = geMalloc(sizeof(struct GE_IMG));
	privnu 		          = (struct GE_IMG *)geState.ASegP->Private;
	geGenCopySeg(geState.ASegP, ASegP);
	*privnu                        = *priv;
	privnu->ImgPtr                 = geXCrImg(geDispDev,
		      XDefaultVisual(geDispDev, geScreen),
		      priv->ImgPtr->depth, priv->ImgPtr->format,
		      priv->ImgPtr->xoffset, priv->ImgPtr->data,
		      priv->ImgPtr->width, priv->ImgPtr->height,
		      priv->ImgPtr->bitmap_pad, priv->ImgPtr->bytes_per_line);
	GEVEC(GEBOUNDS, geState.ASegP);
	break;

#endif

    case GEXAUTODIG:				/* Obj returns from being img*/
	if (ASegP && (XSegP = ASegP->Xref))
	  {/*
	    * Swap the 'object primitive' specific contents of the two segments
	    */
	   StateSav       = geState.State;
	   geState.State  = GEXAUTODIG;

	   strcpy(geUtilBuf, ASegP->Handle);
	   Handler        = ASegP->Handler;
           Private        = ASegP->Private;
	   strcpy(ASegP->Handle, XSegP->Handle);
	   ASegP->Handler = XSegP->Handler;
	   ASegP->Private = XSegP->Private;
	   strcpy(XSegP->Handle, geUtilBuf);
	   XSegP->Handler = Handler;
	   XSegP->Private = Private;
	   XSegP->Xref    = NULL;
	   ASegP->Xref    = NULL;
	   GEVEC(GEKILL, XSegP);		/* Kill dummy - done with it */
	   geState.State  = StateSav;
	  }
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
	 {if(geState.Grav.Align || geState.APagP->Grid.XAlign ||
	     geState.APagP->Grid.YAlign)
	  {geState.Grav.Lock = FALSE;
	   geGenAln(ASegP->Co.x1, ASegP->Co.y1);
	   if (!geState.Grav.Lock)
	     {geGenAln(ASegP->Co.x2, ASegP->Co.y1);
	      if (!geState.Grav.Lock)
		{geGenAln(ASegP->Co.x2, ASegP->Co.y2);
		 if (!geState.Grav.Lock)
		   {geGenAln(ASegP->Co.x1, ASegP->Co.y2);
		    if (!geState.Grav.Lock)
		       {geGenAln((ASegP->Co.x2 + ASegP->Co.x1) >> 1,
				  (ASegP->Co.y2 + ASegP->Co.y1) >> 1);
			if (!geState.Grav.Lock)
			  {geGenAln((ASegP->Co.x2 + ASegP->Co.x1) >> 1,
				      ASegP->Co.y1);
			   if (!geState.Grav.Lock)
			     {geGenAln(ASegP->Co.x2,
					(ASegP->Co.y1 + ASegP->Co.y2) >> 1);
			      if (!geState.Grav.Lock)
				{geGenAln((ASegP->Co.x2 + ASegP->Co.x1) >> 1,
					    ASegP->Co.y2);
				 if (!geState.Grav.Lock)
				   geGenAln(ASegP->Co.x1,
					     (ASegP->Co.y2+ASegP->Co.y1) >> 1);
			        }
			     }
			  }
		       }
		   } 
	        }
	     }
	  }
	   if (cmd != GEGRAVTST && (geState.Dx || geState.Dy))
	     {priv->Pt.x  += geState.Dx;
	      priv->Pt.y  += geState.Dy;
	     }
	}
	break;

#ifdef GEISL

    case GEMAG:
    case GEMAGRESD:
    case GEIMGMAG:
        if (ASegP && ASegP->Visible && priv && priv->ImgPtr)
	  {
#ifdef GERAGS
	   if (geMagFX > 100. || geMagFY > 100.)
		{/*
	   	  * See if there is enough memory to perform the magnification
	   	  */
	  	 GEVEC(GEINQMEMIMG, ASegP);
	  	 num = (int)((float)geInqRes * (geMagFX / 100.) *
			     (geMagFY / 100.));
		 num <<= 1;
		 if (num > 0 && (ptr = (char *)malloc(num)))
		    free(ptr);
		 else
		    {geFT = geMagFX;
	      	     error_string = (char *) geFetchLiteral("GE_ERR_MAGMEM", MrmRtypeChar8);
		     if (error_string != NULL) 
		       {sprintf(geErrBuf, error_string, geFT);
			geError(geErrBuf, FALSE);
		        XtFree(error_string);
		       }
		     return;
		    }
		}
	   geSetWaitCurs(TRUE);
      	   XFlush(geDispDev);   

#endif

	   if (!geMagGrp)
	     {geMagGrp = TRUE;
	      geMagCx  = priv->Pt.x + ((priv->Bx.x1 + priv->Bx.x3) >> 1);
	      geMagCy  = priv->Pt.y + ((priv->Bx.y1 + priv->Bx.y3) >> 1);
	     }
	   ImgPtr = priv->ImgPtr;
	   geXimageToFid(ImgPtr, &islfid, ImgPtr->width, ImgPtr->height);
	   xscale = yscale = 1.;
	   if (geMagMode == GENOCONSTRAINT || geMagMode == GEHORIZONTAL)
	     xscale = geMagFX / 100.;
	   if (geMagMode == GENOCONSTRAINT || geMagMode == GEVERTICAL)
	     yscale = geMagFY / 100.;
	   nu_islfid = ImgScale(islfid, &xscale, &yscale, 0,
			      IMG$M_SAVE_VERTICAL|IMG$M_SAVE_HORIZONTAL, 0);
	   geFidToXimage(&ImgPtr, nu_islfid);
	   ImgDeleteFrame(islfid);
	   ImgDeleteFrame(nu_islfid);
	   /*
	    * Release old priv->ImgPtr, if there are no other refs to it
	    */
	   GEVEC(GERELPRIV, ASegP);
	   priv->ImgPtr = ImgPtr;

	   priv->Bx.x1 += priv->Pt.x;
	   priv->Bx.y1 += priv->Pt.y;
	   priv->Bx.x2 += priv->Pt.x;
	   priv->Bx.y2 += priv->Pt.y;
	   priv->Bx.x3 += priv->Pt.x;
	   priv->Bx.y3 += priv->Pt.y;
	   priv->Bx.x4 += priv->Pt.x;
	   priv->Bx.y4 += priv->Pt.y;
	   geGenMagPt(&priv->Pt);
	   geGenMagBx(&priv->Bx);
	   priv->Bx.x1 -= priv->Pt.x;
	   priv->Bx.y1 -= priv->Pt.y;
	   priv->Bx.x2 -= priv->Pt.x;
	   priv->Bx.y2 -= priv->Pt.y;
	   priv->Bx.x3 -= priv->Pt.x;
	   priv->Bx.y3 -= priv->Pt.y;
	   priv->Bx.x4 -= priv->Pt.x;
	   priv->Bx.y4 -= priv->Pt.y;

#ifdef GERAGS
	   geSetWaitCurs(FALSE);
#endif

	  }
	break;

    case GESCALE:
        if (!ASegP || !ASegP->Visible) break;

        if (priv)
	 {if(geState.Grav.Align || geState.APagP->Grid.XAlign ||
	     geState.APagP->Grid.YAlign)
	  {geGenAln(ASegP->Co.x1, ASegP->Co.y1);
	   if (!geState.Grav.Lock)
	     {geGenAln(ASegP->Co.x2, ASegP->Co.y1);
	      if (!geState.Grav.Lock)
		{geGenAln(ASegP->Co.x2, ASegP->Co.y2);
		 if (!geState.Grav.Lock)
		   {geGenAln(ASegP->Co.x1, ASegP->Co.y2);
		    if (!geState.Grav.Lock)
		       {geGenAln((ASegP->Co.x2 + ASegP->Co.x1) >> 1,
				  (ASegP->Co.y2 + ASegP->Co.y1) >> 1);
			if (!geState.Grav.Lock)
			  {geGenAln((ASegP->Co.x2 + ASegP->Co.x1) >> 1,
				      ASegP->Co.y1);
			   if (!geState.Grav.Lock)
			     {geGenAln(ASegP->Co.x2,
					(ASegP->Co.y1 + ASegP->Co.y2) >> 1);
			      if (!geState.Grav.Lock)
				{geGenAln((ASegP->Co.x2 + ASegP->Co.x1) >> 1,
					    ASegP->Co.y2);
				 if (!geState.Grav.Lock)
				   geGenAln(ASegP->Co.x1,
					     (ASegP->Co.y2+ASegP->Co.y1) >> 1);
			        }
			     }
			  }
		       }
		   } 
	        }
	     }
	  }
	  priv->Bx.x1 += priv->Pt.x;
	  priv->Bx.y1 += priv->Pt.y;
	  priv->Bx.x2 += priv->Pt.x;
	  priv->Bx.y2 += priv->Pt.y;
	  priv->Bx.x3 += priv->Pt.x;
	  priv->Bx.y3 += priv->Pt.y;
	  priv->Bx.x4 += priv->Pt.x;
	  priv->Bx.y4 += priv->Pt.y;
	  geGenScalePt(&priv->Pt);
	  geGenScaleBx(&priv->Bx);
	  priv->Bx.x1 -= priv->Pt.x;
	  priv->Bx.y1 -= priv->Pt.y;
	  priv->Bx.x2 -= priv->Pt.x;
	  priv->Bx.y2 -= priv->Pt.y;
	  priv->Bx.x3 -= priv->Pt.x;
	  priv->Bx.y3 -= priv->Pt.y;
	  priv->Bx.x4 -= priv->Pt.x;
	  priv->Bx.y4 -= priv->Pt.y;
	}
	break;

    case GEROTX:
        if (ASegP && ASegP->Visible && priv)
	  {geGenRotX(&priv->Bx.x1, &priv->Bx.y1);
	   geGenRotX(&priv->Bx.x2, &priv->Bx.y2);
	   geGenRotX(&priv->Bx.x3, &priv->Bx.y3);
	   geGenRotX(&priv->Bx.x4, &priv->Bx.y4);
	  }
	break;

    case GEROTFIXED:
        if (ASegP && ASegP->Visible && priv)
	  {if (geRot.Clockwise) rotangle =  geRot.Beta;
	   else                 rotangle = -geRot.Alpha;

#ifdef GERAGS

	   geSetWaitCurs(TRUE);
      	   XFlush(geDispDev);   
#endif

	   ImgPtr  = priv->ImgPtr;
	   widtho  = GETSUI(ImgPtr->width);
	   heighto = GETSUI(ImgPtr->height);
	   geXimageToFid(ImgPtr, &islfid, ImgPtr->width, ImgPtr->height);
	   nu_islfid = ImgRotate(islfid, &rotangle, 0, 0, 0);
	   geFidToXimage(&ImgPtr, nu_islfid);
	   if (islfid != nu_islfid) ImgDeleteFrame(nu_islfid);
	   ImgDeleteFrame(islfid);
	   /*
	    * Release old priv->ImgPtr, if there are no other refs to it
	    */
	   GEVEC(GERELPRIV, ASegP);
	   priv->ImgPtr = ImgPtr;
	   if (!geRot.Clockwise && geRot.Alpha == 90.)
	     {Bxo         = priv->Bx;
	      geGenRot90(&priv->Pt.x,  &priv->Pt.y);
	      priv->Pt.y -= widtho;
	      widthcropo  = abs(Bxo.x3 - Bxo.x1);
	      heightcropo = abs(Bxo.y3 - Bxo.y1);
	      priv->Bx.x1 = priv->Bx.x4 = Bxo.y1;
	      priv->Bx.y1 = priv->Bx.y2 = widtho - Bxo.x3;
	      priv->Bx.x2 = priv->Bx.x3 = priv->Bx.x1 + heightcropo;
	      priv->Bx.y3 = priv->Bx.y4 = priv->Bx.y1 + widthcropo;
	     }
	   else
	   if (geRot.Clockwise && geRot.Beta  == 90.)
	     {Bxo         = priv->Bx;
	      geGenRot90(&priv->Pt.x,  &priv->Pt.y);
	      priv->Pt.x -= heighto;
	      widthcropo  = abs(Bxo.x3 - Bxo.x1);
	      heightcropo = abs(Bxo.y3 - Bxo.y1);
	      priv->Bx.x1 = priv->Bx.x4 = heighto - Bxo.y3;
	      priv->Bx.y1 = priv->Bx.y2 = Bxo.x1;
	      priv->Bx.x2 = priv->Bx.x3 = priv->Bx.x1 + heightcropo;
	      priv->Bx.y3 = priv->Bx.y4 = priv->Bx.y1 + widthcropo;
	     }
	   else
	     {priv->Bx.x1 = 0;
	      priv->Bx.y1 = 0;
	      priv->Bx.x3 = priv->Bx.x1 + GETSUI(priv->ImgPtr->width);
	      priv->Bx.y3 = priv->Bx.x1 + GETSUI(priv->ImgPtr->height);
	      priv->Bx.x2 = priv->Bx.x3;
	      priv->Bx.y2 = priv->Bx.y1;
	      priv->Bx.x4 = priv->Bx.x1;
	      priv->Bx.y4 = priv->Bx.y3;
	     }
	  }
#ifdef GERAGS
	   geSetWaitCurs(FALSE);
#endif
	break;

#endif

#ifdef GERAGS

    case GEGRAV:
        if (ASegP && ASegP->Visible && priv &&
	    (geState.Grav.Pt.x >= (ASegP->Co.x1 - geState.Grav.Rad) &&
	     geState.Grav.Pt.y >= (ASegP->Co.y1 - geState.Grav.Rad) &&
	     geState.Grav.Pt.x <= (ASegP->Co.x2 + geState.Grav.Rad) &&
	     geState.Grav.Pt.y <= (ASegP->Co.y2 + geState.Grav.Rad)))
	  {geGenGrav(GEGRAVLOCK, ASegP->Co.x1, ASegP->Co.y1);
	   geGenGrav(GEGRAVLOCK, ASegP->Co.x2, ASegP->Co.y1);
	   geGenGrav(GEGRAVLOCK, ASegP->Co.x2, ASegP->Co.y2);
	   geGenGrav(GEGRAVLOCK, ASegP->Co.x1, ASegP->Co.y2);
	   geGenGrav(GEGRAVLOCK, ((ASegP->Co.x2 + ASegP->Co.x1) >> 1),
		     ASegP->Co.y1);
	   geGenGrav(GEGRAVLOCK, ASegP->Co.x2,
		     ((ASegP->Co.y1 + ASegP->Co.y2) >> 1));
	   geGenGrav(GEGRAVLOCK, ((ASegP->Co.x1 + ASegP->Co.x2) >> 1),
		     ASegP->Co.y2);
	   geGenGrav(GEGRAVLOCK, ASegP->Co.x1,
		     ((ASegP->Co.y1 + ASegP->Co.y2) >> 1));
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

	   geClip.x1 = priv->Pt.x + t.x1;
	   geClip.y1 = priv->Pt.y + t.y1;
	   geClip.x2 = geClip.x1 + (t.x3 - t.x1);
	   geClip.y2 = geClip.y1 + (t.y3 - t.y1);

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

	   ASegP->Co.x1 = priv->Pt.x + t.x1;
	   ASegP->Co.y1 = priv->Pt.y + t.y1;
	   ASegP->Co.x2 = ASegP->Co.x1 + (t.x3 - t.x1);
	   ASegP->Co.y2 = ASegP->Co.y1 + (t.y3 - t.y1);

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
           {geClip.x1 = ASegP->Co.x1;
            geClip.y1 = ASegP->Co.y1;
            geClip.x2 = ASegP->Co.x2;
            geClip.y2 = ASegP->Co.y2;
           }
        break;

#ifdef GERAGS

    case GECROP:
        if (ASegP && ASegP->Visible)
	  {if (geGenBx.x1 == -1 && geGenBx.y1 == -1 &&
	       geGenBx.x2 == -1 && geGenBx.y2 == -1)
	     {priv->Bx.x1 = priv->Bx.y1 = priv->Bx.x4 = priv->Bx.y2 = 0;
	      priv->Bx.x2 = priv->Bx.x3 = GETSUI(priv->ImgPtr->width);
	      priv->Bx.y3 = priv->Bx.y4 = GETSUI(priv->ImgPtr->height);
	     }
	  else
	     {/*
	       * Do not allow box size of 0
	       */
	      if (geGenBx.x2 <= geGenBx.x1) geGenBx.x2 = geGenBx.x1 + 1;
	      if (geGenBx.y2 <= geGenBx.y1) geGenBx.y2 = geGenBx.y1 + 1;
	      priv->Bx.x1 = GETSUI(geGenBx.x1) - priv->Pt.x;
	      priv->Bx.y1 = GETSUI(geGenBx.y1) - priv->Pt.y;
	      priv->Bx.x3 = GETSUI(geGenBx.x2) - priv->Pt.x;
	      priv->Bx.y3 = GETSUI(geGenBx.y2) - priv->Pt.y;

	      if (priv->Bx.x1 < 0) priv->Bx.x1 = 0;
	      if (priv->Bx.y1 < 0) priv->Bx.y1 = 0;
	      if (priv->Bx.x3 > GETSUI(priv->ImgPtr->width))
		  priv->Bx.x3 = GETSUI(priv->ImgPtr->width);
	      if (priv->Bx.y3 > GETSUI(priv->ImgPtr->height))
		  priv->Bx.y3 = GETSUI(priv->ImgPtr->height);

	      priv->Bx.x4 = priv->Bx.x1;
	      priv->Bx.y2 = priv->Bx.y1;
	      priv->Bx.x2 = priv->Bx.x3;
	      priv->Bx.y4 = priv->Bx.y3;
	     }
	   GEVEC(GEBOUNDS, ASegP);
	  }
	break;

    case GEFLIP:
	if (geState.Constraint == GEHORIZONTAL ||
	    geState.Constraint == GENOCONSTRAINT)	
	{if (ASegP && ASegP->Visible && priv && priv->ImgPtr)
	  {XSav        = priv->Bx.x1;
	   x           = priv->Pt.x + priv->Bx.x1;
	   width       = priv->Bx.x3 - priv->Bx.x1;
	   xoff        = GETSUI(priv->ImgPtr->width) - priv->Bx.x3;
	   priv->Bx.x1 = priv->Bx.x4 = xoff;
	   priv->Bx.x2 = priv->Bx.x3 = priv->Bx.x1 + width;
	   geGenFlpPt(geState.Constraint, &x, &priv->Pt.y);
	   priv->Pt.x  = x - priv->Bx.x3;
	   geImgFlipX(&(priv->ImgPtr));
	  }
	}
	else if (geState.Constraint == GEVERTICAL ||
  	         geState.Constraint == GENOCONSTRAINT)	
        {if (ASegP && ASegP->Visible && priv && priv->ImgPtr)
	  {YSav        = priv->Bx.y1;
	   y           = priv->Pt.y + priv->Bx.y1;
	   height      = priv->Bx.y3 - priv->Bx.y1;
	   yoff        = GETSUI(priv->ImgPtr->height) - priv->Bx.y3;
	   priv->Bx.y1 = priv->Bx.y2 = yoff;
	   priv->Bx.y3 = priv->Bx.y4 = priv->Bx.y1 + height;
	   geGenFlpPt(geState.Constraint, &priv->Pt.x, &y);
	   priv->Pt.y  = y - priv->Bx.y3;
	   geImgFlipY(priv->ImgPtr);
	  }
	}
	break;

    case GEROUND:
        if (ASegP && ASegP->Visible && priv)
          {priv->Pt.x   = GETSUI(GETPIX(priv->Pt.x));
           priv->Pt.y   = GETSUI(GETPIX(priv->Pt.y));
           priv->Bx.x1  = GETSUI(GETPIX(priv->Bx.x1));
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
	   if ((TRegion = geXPolygonRegion(geVert, geVn, EvenOddRule)))
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

#endif

    case GEEQUATE:
        if(ASegP && geState.ASegP && ASegP->Handler == geState.ASegP->Handler
	   && priv && (privnu = (struct GE_IMG *)geState.ASegP->Private))
	  {geState.ASegP->Co = ASegP->Co;
	   geGenCopyAnim(geState.ASegP, ASegP);
	   geGenCopyDesc(geState.ASegP, ASegP);
	   *privnu           = *priv;
	  }
	break;

    case GEADDVERT:
	/*
	 * Add the coords of this object to the vertex list- used for
	 * selection.
	 */
	if (ASegP && ASegP->Visible && priv && geVn < GEMAX_PTS)
	  {GESTOREVERT(geVn, GETPIX(priv->Pt.x) + GETPIX(priv->Bx.x1),
			     GETPIX(priv->Pt.y) + GETPIX(priv->Bx.y1));
	   if (++geVn >= GEMAX_PTS) break;
	   GESTOREVERT(geVn, GETPIX(priv->Pt.x) + GETPIX(priv->Bx.x2),
			     GETPIX(priv->Pt.y) + GETPIX(priv->Bx.y2));
	   if (++geVn >= GEMAX_PTS) break;
	   GESTOREVERT(geVn, GETPIX(priv->Pt.x) + GETPIX(priv->Bx.x3),
			     GETPIX(priv->Pt.y) + GETPIX(priv->Bx.y3));
	   if (++geVn >= GEMAX_PTS) break;
	   GESTOREVERT(geVn, GETPIX(priv->Pt.x) + GETPIX(priv->Bx.x4),
			     GETPIX(priv->Pt.y) + GETPIX(priv->Bx.y4));
	   geVn++;
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

    case GERELPRIV:
	if (priv && priv->ImgPtr)
	  {/*
	    * See if there are any other references to this image on
	    * any pages
	    */
	   if (gePag0) PagP = gePag0->Next;
	   else        PagP = NULL;

	   geImgRef.Num = 0;
	   geImgRef.Ptr = priv->ImgPtr;
	   while (PagP && geImgRef.Num <= 1)
	     {GEVEC(GEINQIMGREF, PagP->Seg0);
	      PagP = PagP->Next;
	     }

	   if (geImgRef.Num <= 1)
	     {geXDestImg(priv->ImgPtr);
	      priv->ImgPtr   = NULL;
	      geFree(&priv->FileName, 0);
	     }
	  }
	break;

    case GEKILPRIV:
	if (ASegP && ASegP->Private)
	  geFree(&ASegP->Private, sizeof(struct GE_IMG));
	break;

#ifdef GERAGS

    case GEGETLINE:
	if (ASegP && ASegP->Visible && priv)
	   {if (geGetPutFlgs & GEGETPUTCOL)
	       {geIT 		    	      = geSampleAttr.LinCol.OverPrint;
		geSampleAttr.LinCol 	      = priv->ColForeGround;
		geSampleAttr.LinCol.OverPrint = geIT;
	       }
	    if (geGetPutFlgs & GEGETPUTOPRINT)
		geSampleAttr.LinCol.OverPrint = priv->ColForeGround.OverPrint;
	    if (geGetPutFlgs & GEGETPUTWRITEMODE)
	    	geSampleAttr.WritingMode = priv->WritingMode;
	   }
	break;

    case GEPUTLINE:
	if (ASegP && ASegP->Visible && priv)
	   {if (geGetPutFlgs & GEGETPUTCOL)
	       {geIT 		    	      = priv->ColForeGround.OverPrint;
		priv->ColForeGround 	      = geSampleAttr.LinCol;
		priv->ColForeGround.OverPrint = geIT;
	       }
	    if (geGetPutFlgs & GEGETPUTOPRINT)
		priv->ColForeGround.OverPrint = geSampleAttr.LinCol.OverPrint;
	    if (geGetPutFlgs & GEGETPUTWRITEMODE)
	    	priv->WritingMode = geSampleAttr.WritingMode;
	   }
	break;

    case GEGETFILL:
	if (ASegP && ASegP->Visible && priv)
	   {if (geGetPutFlgs & GEGETPUTCOL)
	       {geIT 		    	      = geSampleAttr.FilCol.OverPrint;
		geSampleAttr.FilCol 	      = priv->ColBackGround;
		geSampleAttr.FilCol.OverPrint = geIT;
	       }
	    if (geGetPutFlgs & GEGETPUTOPRINT)
		geSampleAttr.FilCol.OverPrint = priv->ColBackGround.OverPrint;
	    if (geGetPutFlgs & GEGETPUTHT)
		geSampleAttr.FillHT = ASegP->FillHT;
	    if (geGetPutFlgs & GEGETPUTSTYLE)
		geSampleAttr.FillStyle = ASegP->FillStyle;
	    if (geGetPutFlgs & GEGETPUTWRITEMODE)
	    	geSampleAttr.WritingMode = ASegP->FillWritingMode;
	   }
	break;

    case GEPUTFILL:
	if (ASegP && ASegP->Visible && priv)
	   {if (geGetPutFlgs & GEGETPUTCOL)
	       {geIT 		     	      = priv->ColBackGround.OverPrint;
		priv->ColBackGround  	      = geSampleAttr.FilCol;
		priv->ColBackGround.OverPrint = geIT;
	       }
	    if (geGetPutFlgs & GEGETPUTOPRINT)
		priv->ColBackGround.OverPrint = geSampleAttr.FilCol.OverPrint;
	    if (geGetPutFlgs & GEGETPUTHT)
		ASegP->FillHT 	       = geSampleAttr.FillHT;
	    if (geGetPutFlgs & GEGETPUTSTYLE)
		ASegP->FillStyle       = geSampleAttr.FillStyle;
	    if (geGetPutFlgs & GEGETPUTWRITEMODE)
	    	ASegP->FillWritingMode = geSampleAttr.WritingMode;

	    /*
	     * Opacity needs to be continued to be tracked in Flags until no
	     * longer writing pre version 7 metafiles.
	     */
	    if (ASegP->FillStyle == FillOpaqueStippled)
	      priv->Flags         ^= GEIMGOPACITY_CLR;
	    else
	      priv->Flags         |= GEIMGOPACITY_CLR;
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
	    	geSampleAttr.WritingMode = ASegP->FillWritingMode;
	   }
	break;

    case GEGETTXTFG:
	if (ASegP && ASegP->Visible && priv)
	   {if (geGetPutFlgs & GEGETPUTCOL)
		geSampleAttr.TxtFgCol    = priv->ColForeGround;
	    if (geGetPutFlgs & GEGETPUTWRITEMODE)
	    	geSampleAttr.WritingMode = priv->WritingMode;
	   }
	break;

    case GECOMP:
        if (ASegP)
	  {priv->ColForeGround.RGB.red   = GEMAXUSHORT -
	     priv->ColForeGround.RGB.red;
	   priv->ColForeGround.RGB.green = GEMAXUSHORT -
	     priv->ColForeGround.RGB.green;
	   priv->ColForeGround.RGB.blue  = GEMAXUSHORT -
	     priv->ColForeGround.RGB.blue;
	   priv->ColForeGround.RGB.pixel =
	     geAllocColor(priv->ColForeGround.RGB.red,
			  priv->ColForeGround.RGB.green,
			  priv->ColForeGround.RGB.blue, NULL);
	   priv->ColBackGround.RGB.red   = GEMAXUSHORT -
	     priv->ColBackGround.RGB.red;
	   priv->ColBackGround.RGB.green = GEMAXUSHORT -
	     priv->ColBackGround.RGB.green;
	   priv->ColBackGround.RGB.blue  = GEMAXUSHORT -
	     priv->ColBackGround.RGB.blue;
	   priv->ColBackGround.RGB.pixel =
	     geAllocColor(priv->ColBackGround.RGB.red,
			  priv->ColBackGround.RGB.green,
			  priv->ColBackGround.RGB.blue, NULL);
	   geGenRgbToCmyk(&(priv->ColForeGround.RGB),
			  &(priv->ColForeGround.CMYK));
	   geGenRgbToCmyk(&(priv->ColBackGround.RGB),
			  &(priv->ColBackGround.CMYK));
	   geGenNameRgb(&(priv->ColForeGround));/* Try to give it a name     */
	   geGenNameRgb(&(priv->ColBackGround));/* Try to give it a name     */
          }
	break;

    case GEINQLIVE:                             /* Increment livesegs counter*/
	if (ASegP && ASegP->Live) geState.LiveSegs++;
	break;

    case GEINQMEMIMG:                         	/* Total mem needed by images*/
	geInqRes = 0;
	if (ASegP && ASegP->Live && ASegP->Visible && priv && priv->ImgPtr)
	    geInqRes = (long)(priv->ImgPtr->bytes_per_line *
			      priv->ImgPtr->height);
	break;

    case GEINQVISIBLE:                          /* Increment visiblesegs ctr */
	if (ASegP && ASegP->Live && ASegP->Visible) geState.VisibleSegs++;
	break;

    case GEINQXVISIBLE:                        /* Are there HIDDEN segs */
	if (ASegP && ASegP->Live && !ASegP->Visible)
	   geState.Hidden = TRUE; 
	break;

#endif

    case GEINQIMGREF:
        if (ASegP && priv && priv->ImgPtr && geImgRef.Ptr)
	  if (priv->ImgPtr == geImgRef.Ptr ||
	      priv->ImgPtr->data == geImgRef.Ptr->data)
	    geImgRef.Num++;
	break;

    case GEGETSEG_COL:
        if (ASegP && priv && priv->ImgPtr && priv->ImgPtr->data)
	   {if (ASegP->Col.RGB.pixel == geGenCol.pixel ||
	    	priv->ColForeGround.RGB.pixel == geGenCol.pixel ||
		priv->ColBackGround.RGB.pixel == geGenCol.pixel)
		{geState.ASegP = ASegP;
		 break;
		}
		
	    if (priv->ImgPtr->depth != 1)
		{ptru 	  = (unsigned char *)(priv->ImgPtr->data);
		 numbytes = priv->ImgPtr->width * priv->ImgPtr->height;
		 for (i = 0; i < numbytes; i++)
		    {if ((unsigned long)(*ptru++) == geGenCol.pixel)
			{geState.ASegP = ASegP;
			 break;
			}
		    }
		}
	   }
	break;

    case GEKILL:
	if(ASegP) geSegKil(&ASegP);
	break;

    case GESCRUB:
	if (ASegP)
	   {GEVEC(GEEXTENT, ASegP);
	    if ((geClip.x2 - geClip.x1) <= 0 || (geClip.y2 - geClip.y1) <= 0)
		geSegKil(&ASegP);
	   }
	break;

    case GEWRITE:
    case GEREAD:
        geImgIO(cmd, ASegP);
	break;

    default:
	break;
   }
}

geImgFlipX(image)
XImage **image;
{
    struct GE_PAG   *PagP;

    XImage *timage;
    int     fromx,tox,x,y,pntr,frompow,topow;
    unsigned char c;
    char   *frompntr,*topntr, *ptr;

/*    geImgAlign(image, 8);                       /* Insure img is byte aligned*/
    timage = geXCrImg(geDispDev, XDefaultVisual(geDispDev, geScreen),
		      (*image)->depth, (*image)->format, 0, NULL,
		      (*image)->width, (*image)->height, 8, 0);

    timage->data = geMalloc((*image)->bytes_per_line * (*image)->height);

    memset(timage->data, 0, (*image)->bytes_per_line * (*image)->height);

    if ((*image)->bits_per_pixel == 1) {

    for (y = 1; y <= (*image)->height; y++) {
        frompntr = (*image)->data + (*image)->bytes_per_line * y - 1;
        topntr   = timage->data + timage->bytes_per_line * (y-1);
        topow    = 1;
        frompow  = pow(2, (*image)->width % 8);
        if (frompow == 0) frompow = 128;

        fromx = (*image)->width;
        tox   = 1;

        while (tox <= (*image)->width) {

              /* get the from bit and put it down to 1st bit */
              c = (*frompntr & frompow) / frompow;

              /* put the bit into the to pntr */
              *topntr = *topntr | (c * topow);
              
              topow = topow * 2;
              if (topow > 128) {
                 topow = 1;
                 topntr++;
              }
              
              frompow = frompow / 2;
              if (frompow < 1) {
                 frompow = 128;
                 frompntr--;
              }
              tox++; fromx--;
        }
    } /* for y */

    } /* bits == 1 */

    else

    if ((*image)->bits_per_pixel == 8) {

       for (y = 1; y <= (*image)->height; y++) {
           frompntr = (*image)->data + (*image)->bytes_per_line * y - 1;
           topntr   = timage->data + timage->bytes_per_line * (y-1);

           for (x = 1; x <= (*image)->width; x++) {
               *topntr = *frompntr;
                topntr++;
                frompntr--;
           }
       }

    } /* bits == 8 */

    /*
     * See if there are any other refs. to the old img data ptr.  If
     * so, don't release it.
     */
    if (gePag0) PagP = gePag0->Next;
    else        PagP = NULL;

    geImgRef.Num = 0;
    geImgRef.Ptr = *image;
    while (PagP && !geImgRef.Num)
      {GEVEC(GEINQIMGREF, PagP->Seg0);
       PagP = PagP->Next;
      }
    
    ptr          = (*image)->data;
    (*image)->data = timage->data;                /* In any case get new one   */
    if (geImgRef.Num)
      timage->data = NULL;                      /* This will prevent release */
    else                                        /* by geXDestImg             */
      timage->data = ptr;                       /* This will cause release   */
    geXDestImg(timage);                         /* by geXDestImg             */

}


geImgFlipY(image)
XImage *image;
{
    struct GE_PAG   *PagP;

    XImage *timage;
    int    y;
    char   *frompntr, *topntr, *ptr;

    timage = geXCrImg(geDispDev, XDefaultVisual(geDispDev, geScreen),
		      image->depth,image->format,0,NULL,image->width,
		      image->height,8,0);

    timage->data = geMalloc(image->bytes_per_line * image->height);

    for (y = 0; y < image->height; y++) {

        frompntr = image->data + image->bytes_per_line * y;
        topntr   = timage->data + image->bytes_per_line * (timage->height - y) ;

        memcpy(topntr,frompntr,image->bytes_per_line);
    }

    /*
     * See if there are any other refs. to this img data ptr.  If
     * so, don't release it.
     */
    if(gePag0) PagP = gePag0->Next;
    else       PagP = NULL;

    geImgRef.Num = 0;
    geImgRef.Ptr = image;
    while (PagP && !geImgRef.Num)
      {GEVEC(GEINQIMGREF, PagP->Seg0);
       PagP = PagP->Next;
      }
    
    ptr          = image->data;
    image->data  = timage->data;                /* In any case, get new one  */
    if (geImgRef.Num)
      timage->data = NULL;                      /* This will prevent release */
    else                                        /* by geXDestImg             */
      timage->data = ptr;                       /* This will cause release   */
    geXDestImg(timage);                         /* by geXDestImg             */

}


geAutoDig(OSegP, WritingMode, FgCol)
struct GE_SEG		*OSegP;
int			WritingMode;
struct GE_COL_OBJ	*FgCol;
{
unsigned char   *Buf;
short		GravAlignSav, GridXAlignSav, GridYAlignSav, EditPtsSav;
int		i, forceHTSav;
unsigned	WSav, HSav, width, height, pixmapw;
long		DxSav, DySav;

struct GE_IMG   *priv;
struct GE_SEG   *ASegP;

Drawable	DrawableSav;
Visual		*vis;
XImage          *ImgPtr, *TImgPtr;
Pixmap		TPixmap;

int		numcols;
XColor		colortable[256];

/*
 * See if capable of producing images on this platform
 */
vis = XDefaultVisual(geDispDev, geScreen);
i   = XBitmapBitOrder(geDispDev);
if (XDefaultDepth(geDispDev, geScreen) > 8 || i != LSBFirst ||
    vis->class == TrueColor || vis->class == DirectColor)
  {geErrorReport(GERRUNAVAILABLE, FALSE);
   return(-1);
  }
/*
 * Save Current drawing window ptr
 */
TPixmap  = NULL;
DrawableSav = geState.Drawable;
WSav     = geState.APagP->Surf.width;
HSav     = geState.APagP->Surf.height;
width    = OSegP->Co.x2 - OSegP->Co.x1;
height   = OSegP->Co.y2 - OSegP->Co.y1;
width    = GETPIX(width);
height   = GETPIX(height);
pixmapw  = ((width + 7) >> 3) << 3;	/* Pixmap width has to byte aligned */
/*
 * Allocate Pixmap
 */
geErr	 = 0;
geState.Drawable = TPixmap =
  XCreatePixmap(geDispDev, geRgRoot.parent, pixmapw, height, geDispChar.Depth);

XSync(geDispDev, FALSE);		/* Make sure if ERR, it gets reported*/
if (geErr == BadAlloc) 
  {geErrorReport(GERRPIXMAPALLOC, FALSE);
   geState.Drawable = DrawableSav;
   geState.APagP->Surf.width  = WSav;
   geState.APagP->Surf.height = HSav;
   return(-1);
  }	
/*
 * Temporarily turn off Grid and Grav align and EditPts
 */
GravAlignSav        = geState.Grav.Align;
GridXAlignSav       = geState.Grid.XAlign;
GridYAlignSav       = geState.Grid.YAlign;
EditPtsSav          = geState.EditPts;
geState.Grav.Align  = FALSE;
geState.Grid.XAlign = FALSE;
geState.Grid.YAlign = FALSE;
geState.EditPts     = FALSE;
/*
 * Fill pixmap with WHITE background, in case object is Translucent
 */
GEFILSTYLE_FOS(geGC3, FillSolid, geWhite.pixel);
GE_FG_M(geGC3, geWhite.pixel, FillSolid);
GEMODE(geGC3, GXcopy);
XFillRectangle(geDispDev, geState.Drawable, geGC3, 0, 0, pixmapw, height);
/*
 * Make a copy of the object, then position the copy at the upper left of
 * pixmap and get it to display itself.
 */
DxSav = OSegP->Co.x1;
DySav = OSegP->Co.y1;
GEVEC(GECOPY,   OSegP);
OSegP      = geState.ASegP;
geState.Dx = -DxSav;
geState.Dy = -DySav;
forceHTSav = geState.ForceHTDisp;
geState.ForceHTDisp = 1;
GEVEC(GEMOVE,   OSegP);
GEVEC(GEBOUNDS, OSegP);
GEVEC(GEDISP,   OSegP);
geState.ForceHTDisp = forceHTSav;
/*
 * Create an image from the pixmap
 */
if (ImgPtr = XGetImage(geDispDev, geState.Drawable, 0, 0, pixmapw, height,
		       XAllPlanes(), ZPixmap)) {
    if (ImgPtr->depth > 1 || ImgPtr->bits_per_pixel != 1) {
	numcols = 1 << geDispChar.Depth;
	for (i = 0; i < numcols; i++)
	    colortable[i].pixel = i;
	XQueryColors(geDispDev, geCmap, colortable,
	    min(numcols, geNumCols));
	TImgPtr = geXImgToSingOrdDith (ImgPtr, colortable,
				       min((1 << geDispChar.Depth), 256));
	if (ImgPtr) {
	    geXDestImg(ImgPtr);
	    ImgPtr  = TImgPtr;
	    TImgPtr = NULL;
	}
    }
}


if (ImgPtr)
  {/*
    * Create a blank image object
    */
    geSegCr();
    geState.ASegP->Private    = geMalloc(sizeof(struct GE_IMG));
    priv 		          = (struct GE_IMG *)geState.ASegP->Private;
    geState.ASegP->Handle[0]  = 'I';
    geState.ASegP->Handle[1]  = 'M';
    geState.ASegP->Handle[2]  = 'G';
    geState.ASegP->Handle[3]  = '\0';
    geState.ASegP->Handler    = geImg;
    ASegP                     = geState.ASegP;
    ASegP->Col  	      = OSegP->Col;  
    ASegP->FillHT  	      = OSegP->FillHT;
    ASegP->FillStyle  	      = OSegP->FillStyle;
    ASegP->FillWritingMode    = OSegP->FillWritingMode;
    ASegP->WhiteOut           = OSegP->WhiteOut;
    ASegP->ConLine            = OSegP->ConLine;
    priv->FileName            = NULL;
    priv->ImgPtr      	      = ImgPtr;
    priv->Pt.x                = DxSav;
    priv->Pt.y         	      = DySav;
    priv->Bx.x1        	      = 0;
    priv->Bx.y1        	      = 0;
    priv->Bx.x3        	      = priv->Bx.x1 + GETSUI(width);
    priv->Bx.y3        	      = priv->Bx.y1 + GETSUI(height);
    priv->Bx.x2        	      = priv->Bx.x3;
    priv->Bx.y2        	      = priv->Bx.y1;
    priv->Bx.x4        	      = priv->Bx.x1;
    priv->Bx.y4        	      = priv->Bx.y3;

    priv->Type                = 0;
    priv->WritingMode         = WritingMode;
    priv->ColForeGround       = *FgCol;
    priv->ColBackGround       = ASegP->Col;
    priv->Mirrored            = priv->RotAngle = 0;
    priv->Flags               = 0;
/* Kludge - take this out when begin writing version 6
 * till then have to track translucent images in Flags
 */
    if (ASegP->FillStyle == GETRANSPARENT ||
	ASegP->FillStyle == FillStippled)
      priv->Flags |= GEIMGOPACITY_CLR;

    /*
     * Have to invert image for some reason
     */
    i = 0;
    Buf = (unsigned char *)priv->ImgPtr->data;
    while (i++ < priv->ImgPtr->bytes_per_line *
	   priv->ImgPtr->height)
      {*Buf = ~*Buf;
       Buf++;
      }

    GEVEC(GEBOUNDS, ASegP);
  }


/*
 * Restore Grid and Grav align and EditPts
 */
geState.Grav.Align  = GravAlignSav;
geState.Grid.XAlign = GridXAlignSav;
geState.Grid.YAlign = GridYAlignSav;
geState.EditPts     = EditPtsSav;

/*
 * Restore Current drawing window info
 */
geState.Drawable = DrawableSav;
geState.APagP->Surf.width  = WSav;
geState.APagP->Surf.height = HSav;

/*
 * Clean up, TERMINATE
 *
 * Free Pixmap and release copy.
 */
if (TPixmap)
  {XFreePixmap(geDispDev, TPixmap);
   TPixmap = NULL;
  }

geSegKilQuick(&OSegP);

return(0);
}
