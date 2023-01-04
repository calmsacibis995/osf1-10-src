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
static char SccsId[] = "@(#)gePs.c	1.12\t4/20/89";
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
**	GEPS	             	       PostScript object handler
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
**	MAS 08/23/91 Created
**
**--
**/

#include "geGks.h"
#include <math.h>

extern unsigned long    geAllocColor ();
extern char *geMalloc ();
extern unsigned char   *gePsCr ();
extern  Region geXPolygonRegion ();

#ifdef GEDPS_ON
#include <DPS/dpsXclient.h>
extern  void cupdps_error_proc ();
extern  DPSContext cupdps_alloc_ctx ();
#endif GEDPS_ON

static char *error_string;



/*
 * Multiplies a transformation matrix by scalars.
 */
void GE_CONCAT_SCALAR (m1, ma, mb, mc, md, mtx, mty)
struct GE_TM   *m1;
float   ma,
        mb,
        mc,
        md,
        mtx,
        mty;
{
    struct GE_TM    m;

    m = *m1;

    m.a = m1->a * ma + m1->b * mc;
    m.b = m1->a * mb + m1->b * md;
    m.c = m1->c * ma + m1->d * mc;
    m.d = m1->c * mb + m1->d * md;
    m.tx = m1->tx * ma + m1->ty * mc + mtx;
    m.ty = m1->tx * mb + m1->ty * md + mty;

    *m1 = m;
}


gePs (cmd, ASegP)
long    cmd;
struct GE_SEG  *ASegP;
{
#define GECHUNK 512

    static int  ttag = GEOBJEDIT;

    char   *ptr,
           *Private;
    int     XSav,
            YSav,
            temp,
            xoff,
            yoff,
            x,
            y,
            xt,
            wt,
            i,
            NumFrames,
            numbytes,
            llx,
            lly,
            urx,
            ury,
            Slen,
            (*Handler) (),
            forceHT;

    unsigned char  *ptru;
    unsigned int    width,
                    height,
                    num,
                    widtho,
                    heighto,
                    widthcropo,
                    heightcropo;
    unsigned long   lx,
                    ly;
    float   xscale,
            yscale,
            rotangle,
            tempf,
            Mag;

    struct GE_PS   *priv,
                   *privnu;
    struct GE_SEG  *Seg0,
                   *SegP,
                   *ASegPSav,
                   *XSegP;
    struct GE_PAG  *PagP;
    struct GE_PT    Pta;
    struct GE_CO    Bxsav;
    struct GE_BX    Bx;
    struct GE_COL_OBJ   FgCol;

    XPoint v[5];
    Pixmap ClipMask;
    GC TempGC;
    Region TRegion;

#ifdef GEDPS_ON
    DPSContext ctx;
#endif GEDPS_ON

    XRectangle rect[1];

    XFontStruct * PSTextFont;
    GContext g0Context;

    long    StateSav;


    if (ASegP) {
	if (!ASegP->Live && cmd != GEINQPSREF && cmd != GEKILL &&
		cmd != GERELPRIV && cmd != GEKILPRIV &&
		cmd != GEGETSEG_COL && cmd != GEXDEL)
	    return;
	priv = (struct GE_PS   *) ASegP->Private;
    }

    switch (cmd) {

	case GEDISP: 
	    if (priv && priv->PsBuf && ASegP && ASegP->Visible &&
		geState.Window && ASegP->InFrame &&
		(!ASegP->ConLine || geRun.RagsCalling) &&
		!geState.HaltRequested) {
	      /*
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
	            !ASegP->AnimCtrl.Animated && ASegP->AnimCtrl.Frames && 
		    !ASegP->AnimCtrl.Frames->Redraw)
		   break;

		 NumFrames = 0;
		 if (geState.Pixmap) geState.NeedToPostPixmap = TRUE;
	        }


	      do {
		  xoff = GETPIX (priv->Pt.x);
		  yoff = GETPIX (priv->Pt.y),
		  x = GETPIX (priv->Pt.x) + GETPIX (priv->Bx.x1);
		  y = GETPIX (priv->Pt.y) + GETPIX (priv->Bx.y1);
		  width = abs (GETPIX (priv->Bx.x3) - GETPIX (priv->Bx.x1));
		  height = abs (GETPIX (priv->Bx.y3) - GETPIX (priv->Bx.y1));
		  v[0].x = (short) GETPIX (priv->Pt.x + priv->Bx.x1);
		  v[0].y = (short) GETPIX (priv->Pt.y + priv->Bx.y1);
		  v[1].x = (short) GETPIX (priv->Pt.x + priv->Bx.x2);
		  v[1].y = (short) GETPIX (priv->Pt.y + priv->Bx.y2);
		  v[2].x = (short) GETPIX (priv->Pt.x + priv->Bx.x3);
		  v[2].y = (short) GETPIX (priv->Pt.y + priv->Bx.y3);
		  v[3].x = (short) GETPIX (priv->Pt.x + priv->Bx.x4);
		  v[3].y = (short) GETPIX (priv->Pt.y + priv->Bx.y4);
		  v[4] = v[0];

		  /* 
		   * Make some final visibility checks
		   */
		  if ((width <= 0 || height <= 0) || /* CORRUPT */
		      (x > geState.APagP->Surf.width ||
		       /* To the RIGHT	 */
		       y > geState.APagP->Surf.height) ||
		      /* BELOW */
		      ((x + width) < 0 || (y + height) < 0)
		      /* LEFT or ABOVE */
		      ) {
		    ASegP->InFrame = FALSE;
		    break;
		  }

#ifdef GERAGS
		  geSetWaitCurs (TRUE);
#endif GERAGS

		  /*
		   *Check to see if DPS is on the server, but check only once 
		   */
#ifdef GEDPS_ON
		  if (geHasDPS == -1) {
		    if (!geDrawPS) {
		      error_string = (char *)
				geFetchLiteral("GE_ERR_DPSOFF", MrmRtypeChar8);
		      if (error_string != NULL) {
			sprintf (geErrBuf, error_string);
			geError (geErrBuf, FALSE);
			XtFree (error_string);
		      }
		    }

		    if (cupdps_check_dps(geDispDev)) {
		      geHasDPS = TRUE;
		    } else {
		      geHasDPS = FALSE;
		      error_string = (char *) geFetchLiteral ("GE_ERR_NODPS",
								MrmRtypeChar8);
		      if (error_string != NULL) {
			sprintf (geErrBuf, error_string);
			geError (geErrBuf, FALSE);
			XtFree (error_string);
		      }
		    }
		  }
#endif GEDPS_ON

		  /* 
		   * Draw PS "page" background first, if there is one
		   */
		  if (ASegP->FillStyle != GETRANSPARENT) {
		    GE_SET_BG_DISP_ATTR;
		    GEFILSTYLE_FOS (geGCPs, geDispAttr.BgStyle,
				    geDispAttr.BgPixel);
		    GE_FG_M (geGCPs, geDispAttr.BgPixel, ASegP->FillWritingMode);
		    GESTIPPLE (geGCPs, geDispAttr.BgHT);
		    if (v[0].x == v[1].x &&
			v[0].x == v[2].x &&
			v[0].x == v[3].x &&
			v[0].y == v[1].y &&
			v[0].y == v[2].y &&
			v[0].y == v[3].y) {
		      XDrawPoint (geDispDev, geState.Drawable, geGCPs,
				  v[0].x, v[0].y);

#ifdef GERAGS
		      geSetWaitCurs (FALSE);
#endif GERAGS
		      break;
		    }
		    XFillPolygon (geDispDev, geState.Drawable, geGCPs, v, 5,
				  Complex, CoordModeOrigin);
		  }


		  /* 
		   * Now draw the PostScript
		   */

		  if (geHasDPS && geDrawPS) {
#ifdef GEDPS_ON
		    /* Allocate a DPS context. Remember that in PS, the */
		    /* origin is in the lower left, not upper left like */
		    /* in X Windows. */
		    /* Set clip rectangle to object's bbox. */
		    /* Note that object's bbox will be bigger than the actual */
		    /* object if it's rotated.  "X" can't clip to a rotated */
		    /* box so this is better than nothing. */
		    forceHT = (geState.ForceHTDisp) ? TRUE : FALSE;
		    if (!(ctx = cupdps_alloc_ctx (geDispDev, geScreen,
						  geState.Drawable, geGCPs,
						  xoff, yoff,
						  GETPIX (ASegP->Co.x1), GETPIX (ASegP->Co.y1),
						  GETPIX (ASegP->Co.x2 - ASegP->Co.x1) + 1,
						  GETPIX (ASegP->Co.y2 - ASegP->Co.y1) + 1,
						  DPSDefaultTextBackstop, cupdps_error_proc,
						  geCmap, geBlack.pixel, geWhite.pixel, forceHT))) {

		      error_string = (char *)
			geFetchLiteral ("GE_ERR_NODPS", MrmRtypeChar8);
		      if (error_string != NULL) {
			sprintf (geErrBuf, error_string);
			geError (geErrBuf, FALSE);
			XtFree (error_string);
		      }
		      goto DrawPSBox;
		    }

		    /* Apply the PS object's matrix to the interpreter's CTM. */
		    sprintf (geUtilBuf,
			     "[%f %f %f %f %f %f] concat\n",
			     priv->tmatrix.a, priv->tmatrix.b, priv->tmatrix.c,
			     priv->tmatrix.d, priv->tmatrix.tx, priv->tmatrix.ty);
		    
		    /* In addition, if the object is complimented, */
		    /* then reverse the current 'transfer' function. */
		    if (priv->Flags & GEPSCOMPLIMENTED) {
		      strcat (geUtilBuf,
			      "/CurXfer currenttransfer def {1 exch sub CurXfer} settransfer\n");
		    }

		    /* Feed the PostScript to the context. */
		    cupdps_draw_ps_buf (geDispDev, geScreen, ctx,
					geUtilBuf, strlen (geUtilBuf),
					priv->PsBuf, priv->PsBufLen);
		    
		    /* Wait until the server is done drawing the picture */
		    cupdps_wait_finish_drawing (ctx);
		    
#ifdef GERAGS
		    geSetWaitCurs (FALSE);
#endif GERAGS
		    
		    /* Free the context and its resources */
		    cupdps_free_ctx (ctx);
#else
		    goto DrawPSBox;
#endif GEDPS_ON
		    
		  } else {
		  DrawPSBox: 
		    /* Prevent anything from drawing outside of object's bbox.*/
		    /* Note that object's bbox will be bigger than the actual */
		    /* object if it's rotated.  "X" can't clip to a rotated */
		    /* box so this is better than nothing. */
/*
 * This causes a problem with expose events.  We need some way to
 * find the intersection of clipping rectangles and use that instead.
 *
 *		    rect[0].x = GETPIX (ASegP->Co.x1);
 *		    rect[0].y = GETPIX (ASegP->Co.y1);
 *		    rect[0].width = GETPIX (ASegP->Co.x2 - ASegP->Co.x1) + 1;
 *		    rect[0].height = GETPIX (ASegP->Co.y2 - ASegP->Co.y1) + 1;
 *		    XSetClipRectangles(geDispDev, geGC0, 0, 0, rect, 1,
 *					Unsorted);
 */
		    
		    XDrawLine (geDispDev, geState.Drawable, geGC0,
			       v[0].x, v[0].y, v[1].x, v[1].y);
		    XDrawLine (geDispDev, geState.Drawable, geGC0,
			       v[1].x, v[1].y, v[2].x, v[2].y);
		    XDrawLine (geDispDev, geState.Drawable, geGC0,
			       v[2].x, v[2].y, v[3].x, v[3].y);
		    XDrawLine (geDispDev, geState.Drawable, geGC0,
			       v[3].x, v[3].y, v[0].x, v[0].y);
		    XDrawLine (geDispDev, geState.Drawable, geGC0,
			       v[0].x, v[0].y, v[2].x, v[2].y);
		    XDrawLine (geDispDev, geState.Drawable, geGC0,
			       v[1].x, v[1].y, v[3].x, v[3].y);
		    
		    /* Extract only the name portion of the filename */
		    /* Throw away directory spec and file extension */
		    if (priv->Filename) {
		      temp = geGenParseF (priv->Filename);
		      ptr = (char *) (priv->Filename) + temp;
		      strcpy (geUtilBuf, ptr);
		      ptr = (char *) strchr (geUtilBuf, '.');
		      if (ptr)
			*ptr = '\0';
		    } else {
		      strcpy (geUtilBuf, "(untitled)");
		    }
		    
		    g0Context = XGContextFromGC (geGC0);
		    PSTextFont = XQueryFont (geDispDev, g0Context);
		    temp = XTextWidth (PSTextFont,
				       geUtilBuf, strlen (geUtilBuf));
		    
		    x = GETPIX (ASegP->Co.x1 + (ASegP->Co.x2 - ASegP->Co.x1) / 2)
		      - temp / 2;
		    y = GETPIX (ASegP->Co.y1 + (ASegP->Co.y2 - ASegP->Co.y1) / 2);
		    
		    XDrawImageString (geDispDev, geState.Drawable, geGC0, x, y,
				      geUtilBuf, strlen (geUtilBuf));
		    
#ifdef GERAGS
		    geSetWaitCurs (FALSE);
#endif GERAGS

		  }

		  /* 
		   * Take into consideration delay factor, if any.
		   */
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

#ifdef GERAGS

	case GEXDISP: 
	    if (geState.AnimDispMode != GEANIM_NONE && priv && priv->PsBuf &&
		    ASegP && ASegP->Visible) {
		width = GETPIX (priv->Bx.x3 - priv->Bx.x1);
		height = GETPIX (priv->Bx.y3 - priv->Bx.y1);
		width = abs (width);
		height = abs (height);
		if (!width || !height)
		    break;
		v[0].x = (short) GETPIX (priv->Pt.x + priv->Bx.x1);
		v[0].y = (short) GETPIX (priv->Pt.y + priv->Bx.y1);
		v[1].x = (short) GETPIX (priv->Pt.x + priv->Bx.x2);
		v[1].y = (short) GETPIX (priv->Pt.y + priv->Bx.y2);
		v[2].x = (short) GETPIX (priv->Pt.x + priv->Bx.x3);
		v[2].y = (short) GETPIX (priv->Pt.y + priv->Bx.y3);
		v[3].x = (short) GETPIX (priv->Pt.x + priv->Bx.x4);
		v[3].y = (short) GETPIX (priv->Pt.y + priv->Bx.y4);
		v[4] = v[0];

		GEVEC (GEBOUNDS, ASegP);
		if (geState.AnimDispMode == GEANIM_BOX) {
		    XDrawRectangle (geDispDev, geState.Drawable, geGC5,
			    GETPIX (ASegP->Co.x1), GETPIX (ASegP->Co.y1),
			    GETPIX (ASegP->Co.x2 - ASegP->Co.x1),
			    GETPIX (ASegP->Co.y2 - ASegP->Co.y1));
		    if (geAnimMin && abs (ASegP->Co.x2 - ASegP->Co.x1) >
			    (geAnimMin >> 1)) {
			XDrawLine (geDispDev, geState.Drawable, geGC5,
				GETPIX (ASegP->Co.x1 + ASegP->Co.x2) >> 1,
				GETPIX (ASegP->Co.y1),
				GETPIX (ASegP->Co.x1 + ASegP->Co.x2) >> 1,
				GETPIX (ASegP->Co.y2));
			XDrawLine (geDispDev, geState.Drawable, geGC5,
				GETPIX (ASegP->Co.x1),
				GETPIX (ASegP->Co.y1 + ASegP->Co.y2) >> 1,
				GETPIX (ASegP->Co.x2),
				GETPIX (ASegP->Co.y1 + ASegP->Co.y2) >> 1);
		    }
		    break;
		} else if (geState.AnimDispMode == GEANIM_LIN) {
			XDrawLine (geDispDev, geState.Drawable, geGC5,
				v[0].x, v[0].y, v[1].x, v[1].y);
			XDrawLine (geDispDev, geState.Drawable, geGC5,
				v[1].x, v[1].y, v[2].x, v[2].y);
			XDrawLine (geDispDev, geState.Drawable, geGC5,
				v[2].x, v[2].y, v[3].x, v[3].y);
			XDrawLine (geDispDev, geState.Drawable, geGC5,
				v[3].x, v[3].y, v[0].x, v[0].y);
			if (geAnimMin && abs (ASegP->Co.x2 - ASegP->Co.x1) >
				(geAnimMin >> 1)) {
			    XDrawLine (geDispDev, geState.Drawable, geGC5,
				    (v[0].x + v[1].x) / 2,
				    (v[0].y + v[1].y) / 2,
				    (v[2].x + v[3].x) / 2,
				    (v[2].y + v[3].y) / 2);
			    XDrawLine (geDispDev, geState.Drawable, geGC5,
				    (v[0].x + v[3].x) / 2,
				    (v[0].y + v[3].y) / 2,
				    (v[1].x + v[2].x) / 2,
				    (v[1].y + v[2].y) / 2);
			}
			break;
		} else if (geState.AnimDispMode == GEANIM_TRUE) {
		    if (priv->Flags & GEPSXORDRAWN) {
			GEVEC  (GEERASE, ASegP);
			GEVEC  (GECDISP, geSeg0);
		    } else {
			GEVEC  (GEDISP,  ASegP);
		    }
		    priv->Flags ^= GEPSXORDRAWN;
		    break;
		}

	    }
	    break;

	case GEHDISP: 
	case GEHXDISP: 
	    if (ASegP && ASegP->Visible && ASegP->InFrame) {
		GEVEC (GEEXTENT, ASegP);/* EXTENT RECTANGLE          */
		XDrawRectangle (geDispDev, geState.Drawable, geGC5,
			GETPIX (geClip.x1), GETPIX (geClip.y1),
			GETPIX (geClip.x2 - geClip.x1),
			GETPIX (geClip.y2 - geClip.y1));
	    }
	    break;

#endif RAGS

	case GEERASE: 
	    if (ASegP && ASegP->Visible && ASegP->InFrame) {
		GEVEC (GEEXTENT, ASegP);
		geClearArea (GETPIX (geClip.x1), GETPIX (geClip.y1),
			GETPIX (geClip.x2 - geClip.x1),
			GETPIX (geClip.y2 - geClip.y1));
	    }
	    break;

#ifdef GERAGS

	case GECREATE: 
	    geSegCr ();
	    geState.ASegP->Private = geMalloc (sizeof (struct GE_PS));
	    priv = (struct GE_PS   *) geState.ASegP->Private;
	    geState.ASegP->Handle[0] = 'E';
	    geState.ASegP->Handle[1] = 'P';
	    geState.ASegP->Handle[2] = 'S';
	    geState.ASegP->Handle[3] = '\0';
	    geState.ASegP->Handler = gePs;
	    ASegP = geState.ASegP;
	/* Initially force a white, transparent background for the EPS */
	/* object.  To follow the current settings for object interior */
	/* would be confusing for the user, I believe. */
	    ASegP->FillStyle = GETRANSPARENT;
	    ASegP->FillHT = 100;
	    ASegP->Col.Name = "White";
	    ASegP->Col.RGB.red = ASegP->Col.RGB.green = ASegP->Col.RGB.blue
		= GEMAXUSHORT;
	    geGenRgbToCmyk (&(ASegP->Col.RGB), &(ASegP->Col.CMYK));

	    geGenFileExt (geUtilBuf, geDefProofPs, FALSE);
	    geGenFileDir (geUtilBuf, geDefDirInPs);
	    priv->Filename = (unsigned char *) geMalloc (strlen (geUtilBuf) + 1);
	    strcpy (priv->Filename, geUtilBuf);

	    geMnStat (GEDISP, 29);/* Put up message that       */

	    geSetWaitCurs (TRUE);

	    XFlush (geDispDev);	/* we're doing it            */
	    priv->PsBuf = gePsCr (priv->Filename,
		    &(priv->w_pix), &(priv->h_pix),
		    &llx, &lly, &urx, &ury,
		    &(priv->PsBufLen));

	    priv->llx_pts = priv->ulx_pts = priv->lft_pts = (float) llx;
	    priv->lly_pts = priv->lry_pts = priv->bot_pts = (float) lly;
	    priv->urx_pts = priv->lrx_pts = priv->rt_pts = (float) urx;
	    priv->ury_pts = priv->uly_pts = priv->top_pts = (float) ury;

	    geSetWaitCurs (FALSE);

	    if (priv->PsBuf == NULL) {
		geFree (&priv->Filename, 0);
		geSegKil (&ASegP);
		break;
	    }

	/* 
	 * Request location
	 */
	    geMnStat (GEDISP, 38);
	    geMouseXY (geState.APagP->Surf.self);
	    geGenBx.x1 = GETPIX (geState.Mouse.x);
	    geGenBx.y1 = GETPIX (geState.Mouse.y);
	    geGenBx.x2 = geGenBx.x1 + priv->w_pix;
	    geGenBx.y2 = geGenBx.y1 + priv->h_pix;
	    geState.ASegP->Live = FALSE;
	    geGenBoxMove (geState.APagP->Surf.self, FALSE);
	    if (geState.Event.type == GE_Bd &&
		    geState.Event.xbutton.button == GEBUT_R) {
		GEVEC (GEKILL, geState.ASegP);
		XBell (geDispDev, GEBEEP_DONE);
		return;
	    }

	    geState.ASegP->Live = TRUE;

	    geState.Mouse.x = GETSUI (geGenBx.x1);
	    geState.Mouse.y = GETSUI (geGenBx.y1);

	    if (geState.Grav.Align || geState.APagP->Grid.XAlign ||
		    geState.APagP->Grid.YAlign) {
		geState.Dx = geState.Dy = 0;
		geGenAln (geState.Mouse.x, geState.Mouse.y);
		geState.Mouse.x += geState.Dx;
		geState.Mouse.y += geState.Dy;
		geState.Grav.OPt.x = geState.Mouse.x += geState.Dx;
		geState.Grav.OPt.y = geState.Mouse.y += geState.Dy;
	    }
	    priv->Pt.x = geU.XR = geState.Mouse.x;
	    priv->Pt.y = geU.YR = geState.Mouse.y;
	/* x1, y1 is upper left corner.  Subsequent numbers go clockwise 
	*/
	    priv->Bx.x1 = 0;
	    priv->Bx.y1 = 0;
	    priv->Bx.x3 = priv->Bx.x1 + GETSUI (priv->w_pix);
	    priv->Bx.y3 = priv->Bx.y1 + GETSUI (priv->h_pix);
	    priv->Bx.x2 = priv->Bx.x3;
	    priv->Bx.y2 = priv->Bx.y1;
	    priv->Bx.x4 = priv->Bx.x1;
	    priv->Bx.y4 = priv->Bx.y3;

	    priv->PenHT = geAttr.LineHT;
	    priv->PenStyle = geAttr.LineStyle;
/* Temporary - till begin using this */
	    priv->PenHT = 100;
	    priv->PenStyle = FillSolid;

	    priv->tmatrix.a = priv->tmatrix.d = 1;
	    priv->tmatrix.b = priv->tmatrix.c = 0;
	    priv->tmatrix.tx = 0;
	    priv->tmatrix.ty = 0;

	/* Move UL of PS image to origin */
	    GE_CONCAT_SCALAR (&(priv->tmatrix), 1.0, 0.0, 0.0, 1.0,
		    -(priv->ulx_pts), -(priv->uly_pts));


	    priv->llx_pts -= priv->lft_pts;
	    priv->ulx_pts -= priv->lft_pts;
	    priv->lly_pts -= priv->top_pts;
	    priv->lry_pts -= priv->top_pts;
	    priv->urx_pts -= priv->lft_pts;
	    priv->lrx_pts -= priv->lft_pts;
	    priv->ury_pts -= priv->top_pts;
	    priv->uly_pts -= priv->top_pts;
	    priv->bot_pts -= priv->top_pts;
	    priv->rt_pts -= priv->lft_pts;
	    priv->lft_pts -= priv->lft_pts;/* do these two last */
	    priv->top_pts -= priv->top_pts;/* do these two last */

	    priv->Flags = 0;

	    GEVEC (GEBOUNDS, geState.ASegP);
	    GEVEC (GEDISP, geState.ASegP);
	    break;

	case GEOBJEDIT: 
	case GEEDIT: 
	    geMnU (NULL, &ttag, NULL);
	    GEVEC (GEXDISP, geState.ASegP);
	    geMnStat (GEDISP, 32);/* Request action	     */
	    Bxsav.x1 = GETPIX (priv->Pt.x) + GETPIX (priv->Bx.x1);
	    Bxsav.y1 = GETPIX (priv->Pt.y) + GETPIX (priv->Bx.y1);
	    Bxsav.x2 = GETPIX (priv->Pt.x) + GETPIX (priv->Bx.x3);
	    Bxsav.y2 = GETPIX (priv->Pt.y) + GETPIX (priv->Bx.y3);
	    geGenBx.x1 = -1;
	    geGenBx.y1 = -1;
	    geGenBx.x2 = -1;
	    geGenBx.y2 = -1;
	    GEVEC (GECROP, geState.ASegP);
	    GEVEC (GEBOUNDS, geState.ASegP);
	    GEVEC (GEEXTENT, geState.ASegP);
	    GEVEC (GEDISP, geState.ASegP);
	    ASegPSav = geState.ASegP;/* Null out ASegP, so that it */
	    geState.ASegP = NULL;/* will be included in CDISP */
	/* 
	 * Wait for start point - MB2 will be taken at this time to mean
	 * RESET crop box to MAX.
	 */
	    geMaskEvent (GE_MBd | GE_MKd, &geState.Event);
	    if (geState.Event.xbutton.button != GEBUT_M) {
		XPutBackEvent (geDispDev, &geState.Event);
		geGenBox (geState.APagP->Surf.self, FALSE, FALSE);
	    /* Track user's box        */
	    }
	    geState.ASegP = ASegPSav;
	    if (geState.Event.type == GE_Bd &&
		    geState.Event.xbutton.button == GEBUT_R) {
		geMnStat (GEDISP, 74);
		GEVEC (GEERASE, geState.ASegP);
		break;
	    }

	    GEVEC (GEERASE, geState.ASegP);
	    GEVEC (GECROP, geState.ASegP);
	    GEVEC (GEBOUNDS, geState.ASegP);
	    break;
	    break;

	case GECOPY: 
	case GECOPYFLP: 
	    if (!ASegP || !ASegP->Visible)
		break;
	    geSegCr ();
	    geState.ASegP->Private = geMalloc (sizeof (struct GE_PS));
	    privnu = (struct GE_PS *) geState.ASegP->Private;
	    if (!privnu)
		break;
	    geGenCopySeg(geState.ASegP, ASegP);
	    *privnu = *priv;
	    GEVEC (GEBOUNDS, geState.ASegP);
	    break;

#endif GERAGS

	case GEAUTODIG: 	/* Obj copies itself into img */
	    if (ASegP && ASegP->Visible && priv && priv->PsBuf && priv->PsBufLen) {
		FgCol.RGB = geBlack;/* Def. for outline = black  */
		FgCol.Name = "Black";
		FgCol.OverPrint = FALSE;
		geGenRgbToCmyk (&(FgCol.RGB), &(FgCol.CMYK));
		StateSav = geState.State;
		geState.State = GEAUTODIG;

		if (geAutoDig (ASegP, GXcopy, &FgCol) == 0 &&
			(XSegP = geState.ASegP)) {
		    ASegP->Xref = XSegP;/* Link the two segments     */
		    XSegP->Xref = ASegP;/* together		     */
		/* 
		 * Swap the 'object primitive' specific contents of the two
		 * segments
		 */
		    strcpy (geUtilBuf, ASegP->Handle);
		    Handler = ASegP->Handler;
		    Private = ASegP->Private;
		    strcpy (ASegP->Handle, XSegP->Handle);
		    ASegP->Handler = XSegP->Handler;
		    ASegP->Private = XSegP->Private;
		    strcpy (XSegP->Handle, geUtilBuf);
		    XSegP->Handler = Handler;
		    XSegP->Private = Private;
		    GEVEC (GEDEL, XSegP);/* Delete the dummy for now  */
		}
	    }
	    geState.State = StateSav;
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
		    (!ASegP || !ASegP->Visible))
		break;

	    if (priv) {
		Bx.x1 = priv->Pt.x + priv->Bx.x1;
		Bx.y1 = priv->Pt.y + priv->Bx.y1;
		Bx.x2 = priv->Pt.x + priv->Bx.x2;
		Bx.y2 = priv->Pt.y + priv->Bx.y2;
		Bx.x3 = priv->Pt.x + priv->Bx.x3;
		Bx.y3 = priv->Pt.y + priv->Bx.y3;
		Bx.x4 = priv->Pt.x + priv->Bx.x4;
		Bx.y4 = priv->Pt.y + priv->Bx.y4;

		if (geState.Grav.Align || geState.APagP->Grid.XAlign ||
			geState.APagP->Grid.YAlign) {
		    geState.Grav.Lock = FALSE;
		    geGenAln (Bx.x1, Bx.y1);
		    if (!geState.Grav.Lock) {
			geGenAln (Bx.x2, Bx.y2);
			if (!geState.Grav.Lock) {
			    geGenAln (Bx.x3, Bx.y3);
			    if (!geState.Grav.Lock) {
				geGenAln (Bx.x4, Bx.y4);
				if (!geState.Grav.Lock) {
				    geGenAln ((Bx.x1 + Bx.x2) / 2,
					    (Bx.y1 + Bx.y2) / 2);
				    if (!geState.Grav.Lock) {
					geGenAln ((Bx.x2 + Bx.x3) / 2,
						(Bx.y2 + Bx.y3) / 2);
					if (!geState.Grav.Lock) {
					    geGenAln ((Bx.x3 + Bx.x4) / 2,
						    (Bx.y3 + Bx.y4) / 2);
					    if (!geState.Grav.Lock) {
						geGenAln ((Bx.x4 + Bx.x1) / 2,
							(Bx.y4 + Bx.y1) / 2);
						if (!geState.Grav.Lock)
						    geGenAln ((Bx.x1 + Bx.x3) / 2,
							    (Bx.y1 + Bx.y3) / 2);
					    }
					}
				    }
				}
			    }
			}
		    }
		}
		if (cmd != GEGRAVTST && (geState.Dx || geState.Dy)) {
		    priv->Pt.x += geState.Dx;
		    priv->Pt.y += geState.Dy;
		}
	    }
	    break;


	case GEMAG: 
	case GEMAGRESD: 
	case GEIMGMAG: 
	    if (ASegP && ASegP->Visible && priv && priv->PsBuf) {

	    /* x = (geEditBox.Pt.x << 1) - x; */

		if (!geMagGrp) {
		    geMagGrp = TRUE;
		    geMagCx = priv->Pt.x + ((priv->Bx.x1 + priv->Bx.x3) / 2);
		    geMagCy = priv->Pt.y + ((priv->Bx.y1 + priv->Bx.y3) / 2);
		}
		xscale = yscale = 1.;
		if (geMagMode == GENOCONSTRAINT || geMagMode == GEHORIZONTAL)
		    xscale = geMagFX / 100.;
		if (geMagMode == GENOCONSTRAINT || geMagMode == GEVERTICAL)
		    yscale = geMagFY / 100.;

	    /* Alter matrix to scale axes */
		GE_CONCAT_SCALAR (&(priv->tmatrix), xscale, 0.0, 0.0, yscale,
			0.0, 0.0);
		priv->Bx.x1 += priv->Pt.x;
		priv->Bx.y1 += priv->Pt.y;
		priv->Bx.x2 += priv->Pt.x;
		priv->Bx.y2 += priv->Pt.y;
		priv->Bx.x3 += priv->Pt.x;
		priv->Bx.y3 += priv->Pt.y;
		priv->Bx.x4 += priv->Pt.x;
		priv->Bx.y4 += priv->Pt.y;
		geGenMagPt (&priv->Pt);
		geGenMagBx (&priv->Bx);
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


	case GESCALE: 
	    if (!ASegP || !ASegP->Visible)
		break;

	    if (priv) {
		if (geState.Grav.Align || geState.APagP->Grid.XAlign ||
			geState.APagP->Grid.YAlign) {
		    geGenAln (ASegP->Co.x1, ASegP->Co.y1);
		    if (!geState.Grav.Lock) {
			geGenAln (ASegP->Co.x2, ASegP->Co.y1);
			if (!geState.Grav.Lock) {
			    geGenAln (ASegP->Co.x2, ASegP->Co.y2);
			    if (!geState.Grav.Lock) {
				geGenAln (ASegP->Co.x1, ASegP->Co.y2);
				if (!geState.Grav.Lock) {
				    geGenAln ((ASegP->Co.x2 + ASegP->Co.x1) >> 1,
					    (ASegP->Co.y2 + ASegP->Co.y1) >> 1);
				    if (!geState.Grav.Lock) {
					geGenAln ((ASegP->Co.x2 + ASegP->Co.x1) >> 1,
						ASegP->Co.y1);
					if (!geState.Grav.Lock) {
					    geGenAln (ASegP->Co.x2,
						    (ASegP->Co.y1 + ASegP->Co.y2) >> 1);
					    if (!geState.Grav.Lock) {
						geGenAln ((ASegP->Co.x2 + ASegP->Co.x1) >> 1,
							ASegP->Co.y2);
						if (!geState.Grav.Lock)
						    geGenAln (ASegP->Co.x1,
							    (ASegP->Co.y2 + ASegP->Co.y1) >> 1);
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
		geGenScalePt (&priv->Pt);
		geGenScaleBx (&priv->Bx);
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
	    if (ASegP && ASegP->Visible && priv) {
		geGenRotX (&priv->Bx.x1, &priv->Bx.y1);
		geGenRotX (&priv->Bx.x2, &priv->Bx.y2);
		geGenRotX (&priv->Bx.x3, &priv->Bx.y3);
		geGenRotX (&priv->Bx.x4, &priv->Bx.y4);
	    }
	    break;


	case GEROTFIXED: 
	    if (ASegP && ASegP->Visible && priv) {

		if (geRot.Clockwise)
		    rotangle = geRot.Beta;
		else
		    rotangle = -geRot.Alpha;

		XFlush (geDispDev);

		GE_CONCAT_SCALAR (&(priv->tmatrix),
			(GECOSD (rotangle)), (-(GESIND (rotangle))),
			(GESIND (rotangle)), (GECOSD (rotangle)),
			0.0, 0.0);


		priv->Bx.x1 += priv->Pt.x;
		priv->Bx.x2 += priv->Pt.x;
		priv->Bx.x3 += priv->Pt.x;
		priv->Bx.x4 += priv->Pt.x;
		priv->Bx.y1 += priv->Pt.y;
		priv->Bx.y2 += priv->Pt.y;
		priv->Bx.y3 += priv->Pt.y;
		priv->Bx.y4 += priv->Pt.y;

		if ((geRot.Clockwise && geRot.Beta == 90.) ||
			(!geRot.Clockwise && geRot.Alpha == 90.)) {
		    geGenRot90 (&priv->Pt.x, &priv->Pt.y);
		    geGenRot90 (&priv->Bx.x1, &priv->Bx.y1);
		    geGenRot90 (&priv->Bx.x2, &priv->Bx.y2);
		    geGenRot90 (&priv->Bx.x3, &priv->Bx.y3);
		    geGenRot90 (&priv->Bx.x4, &priv->Bx.y4);
		} else {
		    geGenRotX (&priv->Pt.x, &priv->Pt.y);
		    geGenRotX (&priv->Bx.x1, &priv->Bx.y1);
		    geGenRotX (&priv->Bx.x2, &priv->Bx.y2);
		    geGenRotX (&priv->Bx.x3, &priv->Bx.y3);
		    geGenRotX (&priv->Bx.x4, &priv->Bx.y4);
		}

		priv->Bx.x1 -= priv->Pt.x;
		priv->Bx.x2 -= priv->Pt.x;
		priv->Bx.x3 -= priv->Pt.x;
		priv->Bx.x4 -= priv->Pt.x;
		priv->Bx.y1 -= priv->Pt.y;
		priv->Bx.y2 -= priv->Pt.y;
		priv->Bx.y3 -= priv->Pt.y;
		priv->Bx.y4 -= priv->Pt.y;
	    }

	    break;

#ifdef GERAGS

	case GEGRAV: 
	    if (ASegP && ASegP->Visible && priv &&
		    (geState.Grav.Pt.x >= (ASegP->Co.x1 - geState.Grav.Rad) &&
			geState.Grav.Pt.y >= (ASegP->Co.y1 - geState.Grav.Rad) &&
			geState.Grav.Pt.x <= (ASegP->Co.x2 + geState.Grav.Rad) &&
			geState.Grav.Pt.y <= (ASegP->Co.y2 + geState.Grav.Rad))) {
		geGenGrav (GEGRAVLOCK, ASegP->Co.x1, ASegP->Co.y1);
		geGenGrav (GEGRAVLOCK, ASegP->Co.x2, ASegP->Co.y1);
		geGenGrav (GEGRAVLOCK, ASegP->Co.x2, ASegP->Co.y2);
		geGenGrav (GEGRAVLOCK, ASegP->Co.x1, ASegP->Co.y2);
		geGenGrav (GEGRAVLOCK, ((ASegP->Co.x2 + ASegP->Co.x1) >> 1),
			ASegP->Co.y1);
		geGenGrav (GEGRAVLOCK, ASegP->Co.x2,
			((ASegP->Co.y1 + ASegP->Co.y2) >> 1));
		geGenGrav (GEGRAVLOCK, ((ASegP->Co.x1 + ASegP->Co.x2) >> 1),
			ASegP->Co.y2);
		geGenGrav (GEGRAVLOCK, ASegP->Co.x1,
			((ASegP->Co.y1 + ASegP->Co.y2) >> 1));
	    }
	    break;


	case GECONTROL: 
	    if (ASegP && ASegP->Visible) {
		if (abs (geState.Mouse.x - ASegP->Co.x1) >
			abs (geState.Mouse.x - ASegP->Co.x2))
		    geState.Mouse.x = ASegP->Co.x1;
		else
		    geState.Mouse.x = ASegP->Co.x2;
		if (abs (geState.Mouse.y - ASegP->Co.y1) >
			abs (geState.Mouse.y - ASegP->Co.y2))
		    geState.Mouse.y = ASegP->Co.y1;
		else
		    geState.Mouse.y = ASegP->Co.y2;
	    }
	    break;

	case GEEXTENT0: 
	    if (ASegP && ASegP->Visible && priv) {
		struct GE_BX    t;
	    /* 
	     * If it's orthogonal, then it's a bit simpler to figure out
	     */
		if (priv->Bx.x1 == priv->Bx.x2 || priv->Bx.x1 == priv->Bx.x3 ||
			priv->Bx.x1 == priv->Bx.x4) {
		    t.x1 = min (min (min (priv->Bx.x1, priv->Bx.x2),
				priv->Bx.x3), priv->Bx.x4);
		    t.y1 = min (min (min (priv->Bx.y1, priv->Bx.y2),
				priv->Bx.y3), priv->Bx.y4);
		    t.x3 = max (max (max (priv->Bx.x1, priv->Bx.x2),
				priv->Bx.x3), priv->Bx.x4);
		    t.y3 = max (max (max (priv->Bx.y1, priv->Bx.y2),
				priv->Bx.y3), priv->Bx.y4);
		    t.x2 = t.x3;
		    t.y2 = t.y1;
		    t.x4 = t.x1;
		    t.y4 = t.y3;
		} else {
		    t.x1 = min (min (min (priv->Bx.x1, priv->Bx.x2),
				priv->Bx.x3), priv->Bx.x4);
		    t.y1 = (t.x1 == priv->Bx.x1 ? priv->Bx.y1 :
			    t.x1 == priv->Bx.x2 ? priv->Bx.y2 :
			    t.x1 == priv->Bx.x3 ? priv->Bx.y3 : priv->Bx.y4);
		    t.y2 = min (min (min (priv->Bx.y1, priv->Bx.y2),
				priv->Bx.y3), priv->Bx.y4);
		    t.x2 = (t.y2 == priv->Bx.y1 ? priv->Bx.x1 :
			    t.y2 == priv->Bx.y2 ? priv->Bx.x2 :
			    t.y2 == priv->Bx.y3 ? priv->Bx.x3 : priv->Bx.x4);
		    t.x3 = max (max (max (priv->Bx.x1, priv->Bx.x2),
				priv->Bx.x3), priv->Bx.x4);
		    t.y3 = (t.x3 == priv->Bx.x1 ? priv->Bx.y1 :
			    t.x3 == priv->Bx.x2 ? priv->Bx.y2 :
			    t.x3 == priv->Bx.x3 ? priv->Bx.y3 : priv->Bx.y4);
		    t.y4 = max (max (max (priv->Bx.y1, priv->Bx.y2),
				priv->Bx.y3), priv->Bx.y4);
		    t.x4 = (t.y4 == priv->Bx.y1 ? priv->Bx.x1 :
			    t.y4 == priv->Bx.y2 ? priv->Bx.x2 :
			    t.y4 == priv->Bx.y3 ? priv->Bx.x3 : priv->Bx.x4);
		}

		geClip.x1 = priv->Pt.x + t.x1;
		geClip.y1 = priv->Pt.y + t.y1;
		geClip.x2 = geClip.x1 + (t.x3 - t.x1);
		geClip.y2 = geClip.y1 + (t.y3 - t.y1);

	    }
	    break;

#endif GERAGS

	case GEBOUNDS: 
	    if (ASegP && ASegP->Visible && priv) {
		struct GE_BX    t;

		t.x1 = min (min (min (priv->Bx.x1, priv->Bx.x2),
			    priv->Bx.x3), priv->Bx.x4);
		t.y1 = min (min (min (priv->Bx.y1, priv->Bx.y2),
			    priv->Bx.y3), priv->Bx.y4);
		t.x3 = max (max (max (priv->Bx.x1, priv->Bx.x2),
			    priv->Bx.x3), priv->Bx.x4);
		t.y3 = max (max (max (priv->Bx.y1, priv->Bx.y2),
			    priv->Bx.y3), priv->Bx.y4);
		t.x2 = t.x3;
		t.y2 = t.y1;
		t.x4 = t.x1;
		t.y4 = t.y3;

		ASegP->Co.x1 = priv->Pt.x + t.x1;
		ASegP->Co.y1 = priv->Pt.y + t.y1;
		ASegP->Co.x2 = ASegP->Co.x1 + (t.x3 - t.x1);
		ASegP->Co.y2 = ASegP->Co.y1 + (t.y3 - t.y1);

		if (ASegP->Co.x2 < 0 || ASegP->Co.y2 < 0 ||
			GETPIX (ASegP->Co.x1) > geState.APagP->Surf.width ||
			GETPIX (ASegP->Co.y1) > geState.APagP->Surf.height)
		    ASegP->InFrame = FALSE;
		else
		    ASegP->InFrame = TRUE;
	    }
	    break;

	case GEEXTENT: 
	    if (ASegP && ASegP->Visible) {
		geClip.x1 = ASegP->Co.x1 - geRun.Sui2;;
		geClip.y1 = ASegP->Co.y1 - geRun.Sui2;
		geClip.x2 = ASegP->Co.x2 + geRun.Sui2;
		geClip.y2 = ASegP->Co.y2 + geRun.Sui2;
	    }
	    break;

	case GECROP: 
	    if (ASegP && ASegP->Visible) {
		if (geGenBx.x1 == -1 && geGenBx.y1 == -1 &&
			geGenBx.x2 == -1 && geGenBx.y2 == -1) {
		    priv->Bx.x1 = priv->Bx.y1 = priv->Bx.x4 = priv->Bx.y2 = 0;
		    priv->Bx.x2 = priv->Bx.x3 = GETSUI (priv->w_pix);
		    priv->Bx.y3 = priv->Bx.y4 = GETSUI (priv->h_pix);
		} else {	/* 
				 * Do not allow box size of 0
				 */
		    if (geGenBx.x2 <= geGenBx.x1)
			geGenBx.x2 = geGenBx.x1 + 1;
		    if (geGenBx.y2 <= geGenBx.y1)
			geGenBx.y2 = geGenBx.y1 + 1;
		    priv->Bx.x1 = GETSUI (geGenBx.x1) - priv->Pt.x;
		    priv->Bx.y1 = GETSUI (geGenBx.y1) - priv->Pt.y;
		    priv->Bx.x3 = GETSUI (geGenBx.x2) - priv->Pt.x;
		    priv->Bx.y3 = GETSUI (geGenBx.y2) - priv->Pt.y;

		    if (priv->Bx.x1 < 0)
			priv->Bx.x1 = 0;
		    if (priv->Bx.y1 < 0)
			priv->Bx.y1 = 0;
		    if (priv->Bx.x3 > GETSUI (priv->w_pix))
			priv->Bx.x3 = GETSUI (priv->w_pix);
		    if (priv->Bx.y3 > GETSUI (priv->h_pix))
			priv->Bx.y3 = GETSUI (priv->h_pix);

		    priv->Bx.x4 = priv->Bx.x1;
		    priv->Bx.y2 = priv->Bx.y1;
		    priv->Bx.x2 = priv->Bx.x3;
		    priv->Bx.y4 = priv->Bx.y3;
		}
		GEVEC (GEBOUNDS, ASegP);
	    }
	    break;


#ifdef GERAGS

	case GEROUND: 
	    if (ASegP && ASegP->Visible && priv) {
		priv->Pt.x = GETSUI (GETPIX (priv->Pt.x));
		priv->Pt.y = GETSUI (GETPIX (priv->Pt.y));
		priv->Bx.x1 = GETSUI (GETPIX (priv->Bx.x1));
		priv->Bx.y1 = GETSUI (GETPIX (priv->Bx.y1));
		priv->Bx.x2 = GETSUI (GETPIX (priv->Bx.x2));
		priv->Bx.y2 = GETSUI (GETPIX (priv->Bx.y2));
		priv->Bx.x3 = GETSUI (GETPIX (priv->Bx.x3));
		priv->Bx.y3 = GETSUI (GETPIX (priv->Bx.y3));
		priv->Bx.x4 = GETSUI (GETPIX (priv->Bx.x4));
		priv->Bx.y4 = GETSUI (GETPIX (priv->Bx.y4));
	    }
	    break;

	case GEALN: 
	    geGenAlign (ASegP);
	    break;

	case GEALNREF: 
	    if (ASegP && ASegP->Visible) {
		GEVEC (GEEXTENT0, ASegP);
		geAln.Co = geClip;
		geAln.Pt.x = (geAln.Co.x1 + geAln.Co.x2) >> 1;
		geAln.Pt.y = (geAln.Co.y1 + geAln.Co.y2) >> 1;
	    }
	    break;

	case GEALNCH: 
	case GEALNCV: 
	    if (ASegP && ASegP->Visible && priv) {
		geState.Dx = geState.Dy = 0;
		if (cmd == GEALNCV)
		    geState.Dx = geAln.Pt.x - ((ASegP->Co.x1 + ASegP->Co.x2) >> 1);
		if (cmd == GEALNCH)
		    geState.Dy = geAln.Pt.y - ((ASegP->Co.y1 + ASegP->Co.y2) >> 1);
	    }
	    break;

	case GESELPTINFILL: 
	    geSegSel = ASegP;
	    if (ASegP && ASegP->Visible) {
		if (ASegP->Co.x1 > geSel.x2 || ASegP->Co.y1 > geSel.y2 ||
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
		GEVEC (GEADDVERT, ASegP);
		geVert[geVn++] = geVert[0];
		if ((TRegion = geXPolygonRegion (geVert, geVn, EvenOddRule))) {
		    lx = (geSel.x1 + geSel.x2) >> 1;
		    ly = (geSel.y1 + geSel.y2) >> 1;
		    x = GETPIX (lx);
		    y = GETPIX (ly);
		    if (XPointInRegion (TRegion, x, y))
			geState.SelRadC = 0;
		/* This is IT		     */
		    else
			geState.SelRadC = GEMAXSHORT;
		    XDestroyRegion (TRegion);
		}
		else
		    geState.SelRadC = GEMAXSHORT;
	    }

	    break;

	case GESELPT: 
	    geSegSel = ASegP;
	    if (ASegP && ASegP->Visible) {
		if (ASegP->Co.x1 > geSel.x2 || ASegP->Co.y1 > geSel.y2 ||
			ASegP->Co.x2 < geSel.x1 || ASegP->Co.y2 < geSel.y1)
		    break;

		geVn = 0;
		GEVEC (GEADDVERT, ASegP);
		geVert[geVn++] = geVert[0];
		if (geGenPtSelVert (0, cmd))
		    GEVEC (GETESTPT, ASegP);
	    }
	    break;

	case GESELPTIN: 
	    geSegSel = ASegP;
	    if (ASegP && ASegP->Visible) {
		if (ASegP->Co.x1 > geSel.x2 || ASegP->Co.y1 > geSel.y2 ||
			ASegP->Co.x2 < geSel.x1 || ASegP->Co.y2 < geSel.y1)
		    break;

	    /* 
	     * If the selection point is INSIDE of this object, then
	     * compute the proximity - the closest object will be
	     * PICKED.
	     */
		geVn = 0;
		GEVEC (GEADDVERT, ASegP);
		geVert[geVn++] = geVert[0];
		if ((TRegion = geXPolygonRegion (geVert, geVn, EvenOddRule))) {
		    lx = (geSel.x1 + geSel.x2) >> 1;
		    ly = (geSel.y1 + geSel.y2) >> 1;
		    x = GETPIX (lx);
		    y = GETPIX (ly);
		    if (XPointInRegion (TRegion, x, y))
			geGenPtSelVert (0, cmd);
		    else
			geState.SelRadC = GEMAXSHORT;
		    XDestroyRegion (TRegion);
		}
		else
		    geState.SelRadC = GEMAXSHORT;
	    }

	    break;

	case GESELBX: 
	    if (ASegP && ASegP->Visible) {
		if (ASegP->Co.x1 > geSel.x1 && ASegP->Co.y1 > geSel.y1 &&
			ASegP->Co.x2 < geSel.x2 && ASegP->Co.y2 < geSel.y2)
		    geState.ASegP = ASegP;
	    }
	    break;

	case GEFLIP: 
	    if (geState.Constraint == GEHORIZONTAL ||
		    geState.Constraint == GENOCONSTRAINT) {
		if (ASegP && ASegP->Visible && priv && priv->PsBuf) {
		    XSav = priv->Bx.x1;
		    x = priv->Pt.x + priv->Bx.x1;
		    width = priv->Bx.x3 - priv->Bx.x1;
		    xoff = GETSUI (priv->w_pix) - priv->Bx.x3;
		    priv->Bx.x1 = priv->Bx.x4 = xoff;
		    priv->Bx.x2 = priv->Bx.x3 = priv->Bx.x1 + width;
		    geGenFlpPt (geState.Constraint, &x, &priv->Pt.y);
		    priv->Pt.x = x - priv->Bx.x3;

		/* Multiply all x coords by -1 */
		    GE_CONCAT_SCALAR (&(priv->tmatrix), -1.0, 0.0, 0.0, 1.0,
			    0.0, 0.0);
		    priv->llx_pts *= -1;
		    priv->lrx_pts *= -1;
		    priv->ulx_pts *= -1;
		    priv->urx_pts *= -1;
		    priv->lft_pts *= -1;
		    priv->rt_pts *= -1;

		/* Move PS image back to origin */
		    GE_CONCAT_SCALAR (&(priv->tmatrix), 1.0, 0.0, 0.0, 1.0,
			    -(priv->rt_pts), 0.0);
		    priv->llx_pts -= priv->rt_pts;
		    priv->lrx_pts -= priv->rt_pts;
		    priv->ulx_pts -= priv->rt_pts;
		    priv->urx_pts -= priv->rt_pts;
		    priv->lft_pts -= priv->rt_pts;
		    priv->rt_pts -= priv->rt_pts;
				/* do this one last */

		/* Swap right and left edge coords */
		    tempf = priv->rt_pts;
		    priv->rt_pts = priv->lft_pts;
		    priv->lft_pts = tempf;
		}
	    }

	    if (geState.Constraint == GEVERTICAL ||
		    geState.Constraint == GENOCONSTRAINT) {
		if (ASegP && ASegP->Visible && priv && priv->PsBuf) {
		    YSav = priv->Bx.y1;
		    y = priv->Pt.y + priv->Bx.y1;
		    height = priv->Bx.y3 - priv->Bx.y1;
		    yoff = GETSUI (priv->h_pix) - priv->Bx.y3;
		    priv->Bx.y1 = priv->Bx.y2 = yoff;
		    priv->Bx.y3 = priv->Bx.y4 = priv->Bx.y1 + height;
		    geGenFlpPt (geState.Constraint, &priv->Pt.x, &y);
		    priv->Pt.y = y - priv->Bx.y3;

		/* Multiply all y coords by -1 */
		    GE_CONCAT_SCALAR (&(priv->tmatrix), 1.0, 0.0, 0.0, -1.0,
			    0.0, 0.0);
		    priv->lly_pts *= -1;
		    priv->lry_pts *= -1;
		    priv->uly_pts *= -1;
		    priv->ury_pts *= -1;
		    priv->top_pts *= -1;
		    priv->bot_pts *= -1;

		/* Move PS image back to origin */
		    GE_CONCAT_SCALAR (&(priv->tmatrix), 1.0, 0.0, 0.0, 1.0,
			    0.0, -(priv->bot_pts));
		    priv->lly_pts -= priv->bot_pts;
		    priv->lry_pts -= priv->bot_pts;
		    priv->uly_pts -= priv->bot_pts;
		    priv->ury_pts -= priv->bot_pts;
		    priv->top_pts -= priv->bot_pts;
		    priv->bot_pts -= priv->bot_pts;
				/* do this one last */

		/* Swap top and bot edge coords */
		    tempf = priv->top_pts;
		    priv->top_pts = priv->bot_pts;
		    priv->bot_pts = tempf;
		}
	    }
	    break;

#endif GERAGS

	case GEEQUATE: 
	    if (ASegP && geState.ASegP && ASegP->Handler == geState.ASegP->Handler
		    && priv && (privnu = (struct GE_PS *) geState.ASegP->Private)) {
		geState.ASegP->Co = ASegP->Co;
		geGenCopyAnim(geState.ASegP, ASegP);
		geGenCopyDesc(geState.ASegP, ASegP);
		*privnu = *priv;
	    }
	                                        break;


	case GEADDVERT: 
	/* 
	 * Add the coords of this object to the vertex list - used for
	 * selection.
	 */
	    if (ASegP && ASegP->Visible && priv && geVn < GEMAX_PTS) {
		GESTOREVERT (geVn, GETPIX (priv->Pt.x) + GETPIX (priv->Bx.x1),
			GETPIX (priv->Pt.y) + GETPIX (priv->Bx.y1));
		if (++geVn >= GEMAX_PTS)
		    break;
		GESTOREVERT (geVn, GETPIX (priv->Pt.x) + GETPIX (priv->Bx.x2),
			GETPIX (priv->Pt.y) + GETPIX (priv->Bx.y2));
		if (++geVn >= GEMAX_PTS)
		    break;
		GESTOREVERT (geVn, GETPIX (priv->Pt.x) + GETPIX (priv->Bx.x3),
			GETPIX (priv->Pt.y) + GETPIX (priv->Bx.y3));
		if (++geVn >= GEMAX_PTS)
		    break;
		GESTOREVERT (geVn, GETPIX (priv->Pt.x) + GETPIX (priv->Bx.x4),
			GETPIX (priv->Pt.y) + GETPIX (priv->Bx.y4));
		geVn++;
	    }
	    break;

	case GEVISIBLE: 
	    if (ASegP)
		ASegP->Visible = TRUE;
	    break;

	case GEXVISIBLE: 
	    if (ASegP)
		ASegP->Visible = FALSE;
	    break;

	case GEDEL: 
	    if (ASegP)
		ASegP->Live = FALSE;/* Delete the segment        */
	    break;

	case GEPURGEHIDDEN:
	    if (ASegP && !ASegP->Visible)
	      {GEVEC(GEKILL, ASegP);}		/* KILL it now		     */
	    break;

	case GEXDEL: 
	    if (ASegP)
		ASegP->Live = TRUE;/* UnDelete the segment      */
	    break;

    case GEPSADJUST:
	/*
	 * This adjustment is required for producing the PostScript rendition
	 * of the graphic.  This case is only executed by MOPS.
	 *
	 * The pixel "anchor" in X is in the "upper left corner" of the
	 * pixel; whereas PostScript defines it at the "center"
	 * of the pixel.  Thus X draws a pixel DOWN and to the RIGHT of the
	 * specified coordinate, while PostScript draws half the pixel
	 * ABOVE and to the LEFT and half BELOW and to the RIGHT of the
	 * coordinate.
	 *
	 * The adjustment performed below compensates for this differential.
	 */
	if (ASegP && ASegP->Live && ASegP->Visible && priv)
	   {geState.Dx = geState.Dy = GESUIPH;
	    GEVEC(GEMOVE, ASegP);
	   }
	break;

	case GERELPRIV: 
	    if (priv && priv->PsBuf) {
	    /* 
	     * See if there are any other references to this image on
	     * any pages
	     */
		if (gePag0)
		    PagP = gePag0->Next;
		else
		    PagP = NULL;

		gePsRef.Num = 0;
		gePsRef.Ptr = priv->PsBuf;
		while (PagP && gePsRef.Num <= 1) {
		    GEVEC (GEINQPSREF, PagP->Seg0);
		    PagP = PagP->Next;
		}

		if (gePsRef.Num <= 1) {
		    geFree (&priv->PsBuf, 0);
		    priv->PsBuf = NULL;
		    geFree (&priv->Filename, 0);
		}
	    }
	    break;

	case GEKILPRIV: 
	    if (ASegP && ASegP->Private)
		geFree (&ASegP->Private, sizeof (struct GE_PS));
	    break;

#ifdef GERAGS

	case GEGETLINE: 
	/* Outline attrib is inactive for PS objects */
	    error_string = (char *) geFetchLiteral ("GE_ERR_NODPSATTRIB", MrmRtypeChar8);
	    if (error_string != NULL) {
		sprintf (geErrBuf, error_string);
		geError (geErrBuf, FALSE);
		XtFree (error_string);
	    }

/*	    if (ASegP && ASegP->Visible && priv) {
 *		if (geGetPutFlgs & GEGETPUTCOL)
 *		    geSampleAttr.LinCol = priv->ColForeGround;
 *		if (geGetPutFlgs & GEGETPUTWRITEMODE)
 *		    geSampleAttr.WritingMode = priv->WritingMode;
 *	    }
 */
	    break;

	case GEPUTLINE: 
	/* Outline attrib is inactive for PS objects */
	    error_string = (char *) geFetchLiteral ("GE_ERR_NODPSATTRIB", MrmRtypeChar8);
	    if (error_string != NULL) {
		sprintf (geErrBuf, error_string);
		geError (geErrBuf, FALSE);
		XtFree (error_string);
	    }


/*	    if (ASegP && ASegP->Visible && priv) {
 *		if (geGetPutFlgs & GEGETPUTCOL)
 *		    priv->ColForeGround = geSampleAttr.LinCol;
 *		if (geGetPutFlgs & GEGETPUTWRITEMODE)
 *		    priv->WritingMode = geSampleAttr.WritingMode;
 *	    }
 */
	    break;

	case GEGETFILL: 
	    if (ASegP && ASegP->Visible && priv) {
		if (geGetPutFlgs & GEGETPUTCOL)
		    geSampleAttr.FilCol = ASegP->Col;
		if (geGetPutFlgs & GEGETPUTHT)
		    geSampleAttr.FillHT = ASegP->FillHT;
		if (geGetPutFlgs & GEGETPUTSTYLE)
		    geSampleAttr.FillStyle = ASegP->FillStyle;
		if (geGetPutFlgs & GEGETPUTWRITEMODE)
		    geSampleAttr.WritingMode = ASegP->FillWritingMode;
	    }
	    break;

	case GEPUTFILL: 
	    if (ASegP && ASegP->Visible && priv) {
		if (geGetPutFlgs & GEGETPUTCOL)
		    ASegP->Col = geSampleAttr.FilCol;
		if (geGetPutFlgs & GEGETPUTHT)
		    ASegP->FillHT = geSampleAttr.FillHT;
		if (geGetPutFlgs & GEGETPUTSTYLE)
		    ASegP->FillStyle = geSampleAttr.FillStyle;
		if (geGetPutFlgs & GEGETPUTWRITEMODE)
		    ASegP->FillWritingMode = geSampleAttr.WritingMode;
		if (ASegP->FillStyle == FillOpaqueStippled)
		    priv->Flags ^= GEPSOPACITY_CLR;
		else
		    priv->Flags |= GEPSOPACITY_CLR;
	    }
	    break;


	case GEGETTXTBG: 
	    if (ASegP && ASegP->Visible) {
		if (geGetPutFlgs & GEGETPUTCOL)
		    geSampleAttr.TxtBgCol = ASegP->Col;
		if (geGetPutFlgs & GEGETPUTHT)
		    geSampleAttr.TxtBgHT = ASegP->FillHT;
		if (geGetPutFlgs & GEGETPUTSTYLE)
		    geSampleAttr.TxtBgStyle = ASegP->FillStyle;
		if (geGetPutFlgs & GEGETPUTWRITEMODE)
		    geSampleAttr.WritingMode = ASegP->FillWritingMode;
	    }
	    break;

	case GEPUTTXTBG: 
	    if (ASegP && ASegP->Visible) {
		if (geGetPutFlgs & GEGETPUTCOL)
		    ASegP->Col = geSampleAttr.TxtBgCol;
		if (geGetPutFlgs & GEGETPUTHT)
		    ASegP->FillHT = geSampleAttr.TxtBgHT;
		if (geGetPutFlgs & GEGETPUTSTYLE)
		    ASegP->FillStyle = geSampleAttr.TxtBgStyle;
		if (geGetPutFlgs & GEGETPUTWRITEMODE)
		    ASegP->FillWritingMode = geSampleAttr.WritingMode;
		if (ASegP->FillStyle == FillOpaqueStippled)
		    priv->Flags ^= GEPSOPACITY_CLR;
		else
		    priv->Flags |= GEPSOPACITY_CLR;
	    }
	    break;


	case GEGETTXTFG: 
	/* Foreground attribs are not settable for PS objects */
/*	    if (ASegP && ASegP->Visible && priv) {
 *		if (geGetPutFlgs & GEGETPUTCOL)
 *		    geSampleAttr.TxtFgCol = priv->ColForeGround;
 *		if (geGetPutFlgs & GEGETPUTWRITEMODE)
 *		    geSampleAttr.WritingMode = priv->WritingMode;
 *	    }
 */
	    break;

	case GEPUTTXTFG: 
	/* Foreground attribs are not settable for PS objects */
/*	    if (ASegP && ASegP->Visible && priv) {
 *		if (geGetPutFlgs & GEGETPUTCOL)
 *		    priv->ColForeGround = geSampleAttr.TxtFgCol;
 *		if (geGetPutFlgs & GEGETPUTWRITEMODE)
 *		    priv->WritingMode = geSampleAttr.WritingMode;
 *	    }
 */
	    break;

	case GECOMP: 
	    if (ASegP && ASegP->Visible) {
		ASegP->Col.RGB.red = GEMAXUSHORT - ASegP->Col.RGB.red;
		ASegP->Col.RGB.green = GEMAXUSHORT - ASegP->Col.RGB.green;
		ASegP->Col.RGB.blue = GEMAXUSHORT - ASegP->Col.RGB.blue;
		ASegP->Col.RGB.pixel = geAllocColor (ASegP->Col.RGB.red,
			ASegP->Col.RGB.green,
			ASegP->Col.RGB.blue, NULL);
		geGenRgbToCmyk (&(ASegP->Col.RGB), &(ASegP->Col.CMYK));
		geGenNameRgb (&(ASegP->Col));

		priv->Flags ^= GEPSCOMPLIMENTED;
	    }
	    break;

	case GEINQLIVE: 	/* Increment livesegs counter */
	    if (ASegP && ASegP->Live)
		geState.LiveSegs++;
	    break;

	case GEINQMEMIMG: 	/* Total mem needed by images */
	    geInqRes = 0;
	    if (ASegP && ASegP->Live && ASegP->Visible && priv && priv->PsBuf)
		geInqRes = (long) (priv->PsBufLen);
	    break;
 
        case GEINQANIMATED:     /* Increment animatedsegs ctr */
	    if (ASegP && ASegP->Live && ASegP->Visible && ASegP->AnimCtrl.Animate) 
	      geState.AnimatedSegs++;
            break;

	case GEINQVISIBLE: 	/* Increment visiblesegs ctr */
	    if (ASegP && ASegP->Live && ASegP->Visible)
		geState.VisibleSegs++;
	    break;

	case GEINQXVISIBLE: 	/* Are there HIDDEN segs */
	    if (ASegP && ASegP->Live && !ASegP->Visible)
		geState.Hidden = TRUE;
	    break;

#endif GERAGS

	case GEINQPSREF: 
	    if (ASegP && priv && priv->PsBuf && gePsRef.Ptr)
		if (priv->PsBuf == gePsRef.Ptr ||
			priv->PsBuf == gePsRef.Ptr)
		    gePsRef.Num++;
	    break;


	case GEGETSEG_COL: 
	    if (ASegP && priv) {
		if (ASegP->Col.RGB.pixel == geGenCol.pixel) {
		    geState.ASegP = ASegP;
		    break;
		}
	    }
	    break;


	case GEKILL: 
	    if (ASegP)
		geSegKil (&ASegP);
	    break;


	case GESCRUB: 
	    if (ASegP) {
		GEVEC (GEEXTENT, ASegP);
		if ((geClip.x2 - geClip.x1) <= 0 || (geClip.y2 - geClip.y1) <= 0)
		    geSegKil (&ASegP);
	    }
	    break;

	case GEWRITE: 
	case GEREAD: 
	    gePSIO (cmd, ASegP);
	    break;

	default: 
	    break;
    }
}
