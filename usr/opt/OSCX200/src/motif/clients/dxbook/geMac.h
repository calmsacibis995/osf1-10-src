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
/* DEC/CMS REPLACEMENT HISTORY, Element GEMAC.H*/
/* *5    30-APR-1992 22:23:39 GOSSELIN "updating with RAGS animation fixes"*/
/* *4    20-MAR-1992 13:33:34 GOSSELIN "finished updating"*/
/* *3    19-MAR-1992 13:46:42 GOSSELIN "added new RAGS support"*/
/* *2     3-MAR-1992 17:24:42 KARDON "UCXed"*/
/* *1    16-SEP-1991 12:49:29 PARMENTER "Rags"*/
/* DEC/CMS REPLACEMENT HISTORY, Element GEMAC.H*/
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
**	GEMAC.H         	  	RAGS macros
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
**	GNE 03/17/86 Created
**
**--
**/

/* static char GEMacSid[] = "@(#)geMac.h	1.5 10/17/89";                                       */

#include <Xm/Protocols.h>

/*
 * Command Vector
 */
#define GEVEC(mp_cmd, mp_p) \
{if(mp_p && mp_p->Handler) (*mp_p->Handler)(mp_cmd, mp_p); \
 }
/*
 * Object creation command vector
 */
#define GEVEC_CR \
{if(Handler) (*Handler)(GECREATE, 0); \
 }
/*
 * Pt inside selection box test for point edit command
 */
#define GESELTST(mp_x, mp_y) \
(geSel.x1 <= mp_x && geSel.x2 >= mp_x && geSel.y1 <= mp_y && geSel.y2 >= mp_y)

/*
 * Allocate buffer for a message string
 */
#define	GEMSGALLOC(mp_p, mp_str) \
   {mp_p = geMalloc(strlen(mp_str) + 1); \
    strcpy(mp_p, mp_str); \
   }
/*
 * Fetch integer resource
 */
#define	GEGET_RMI(mp_name, mp_struct) \
   {if (XrmGetResource (geRM.SysDb, "mp_name", NULL, &b, &v)) \
    mp_struct = atoi(v.addr); \
   }
/*
 * Fetch Float resource
 */
#define	GEGET_RMF(mp_name, mp_struct) \
   {if (XrmGetResource (geRM.SysDb, "mp_name", NULL, &b, &v)) \
    mp_struct = (float)atof(v.addr); \
   }
/*
 * Put integer resource
 */
#define	GEPUT_RMI(mp_name, mp_struct) \
   {sprintf(geUtilBuf, "%d", (mp_struct)); \
    XrmPutStringResource (&geRM.SysDb, "mp_name", geUtilBuf); \
   }
/*
 * Put float resource
 */
#define	GEPUT_RMF(mp_name, mp_struct) \
   {sprintf(geUtilBuf, "%f", (mp_struct)); \
    XrmPutStringResource (&geRM.SysDb, "mp_name", geUtilBuf); \
   }
/*
 * Fetch geometry resource
 */
#define	GEGET_RMGEO(mp_name, mp_struct) \
   {if (XrmGetResource (geRM.SysDb, "mp_name", NULL, &b, &v)) \
    geFT = ((float)atof(v.addr) * (float)PixWidth) / (float)MMWidth; \
    GEINT(geFT, mp_struct); \
   }
/*
 * Put geometry resource
 */
#define	GEPUT_RMGEO(mp_name, mp_struct) \
   {geFT = (float)((mp_struct) * MMWidth) / PixWidth; \
    sprintf(geUtilBuf, "%f", geFT); \
    XrmPutStringResource (&geRM.SysDb, "mp_name", geUtilBuf); \
   }
/*
 * Fetch KSym resource
 */
#define	GEGET_RMK(mp_name, mp_struct) \
   {if (XrmGetResource (geRM.SysDb, "mp_name", NULL, &b, &v)) \
    mp_struct = *v.addr; \
    mp_struct = (char)tolower((mp_struct)); \
   }

/*
 * Color detection
 */
#define GEMONO		(DefaultVisual(geDispDev, geScreen)->class <= GrayScale)
#define GEFEWCOLS	(geNumCols <= 64)
#define GECOLCON(x)	((long)(65535.0 * (float)((float)(x) / 255.0)))
/*
 * Sin - Cos macros for possibly float angle.  If the angle is not whole
 * number of degrees, perform linear interpolation between closest pre-
 * computed sin-cos values.  Resultant is stored in geTF;
 */
#define GEFSIN(mp_x) \
   {geIT = (int)mp_x; \
    geFT = (float)mp_x - (float)geIT; \
    if (geFT) \
	geFT = geSin[geIT] + geFT * (geSin[geIT + 1] - geSin[geIT]); \
    else \
	geFT = geSin[geIT]; \
   }

#define GEFCOS(mp_x) \
   {geIT = (int)mp_x; \
    geFT = (float)mp_x - (float)geIT; \
    if (geFT) \
	geFT = geCos[geIT] + geFT * (geCos[geIT + 1] - geCos[geIT]); \
    else \
	geFT = geCos[geIT]; \
   }
/*
 * Graphics Context macros
 */
#define GESTIPPLE(mp_gc, mp_x) \
   {if (mp_x >= 0 && mp_x < (100 + GEMAXPAT)) \
	{if (mp_x <= 100) \
	   XSetStipple(geDispDev, mp_gc, geHTStipple[mp_x]); \
         else \
	   XSetStipple(geDispDev, mp_gc, gePATStipple[mp_x - 101]); \
	} \
    else \
	 XSetStipple(geDispDev, mp_gc, geHTStipple[100]); \
   }
#define GETSORIGIN(mp_gc, mp_x, mp_y) \
   {geGCV.clip_x_origin = mp_x; \
    geGCV.clip_y_origin = mp_y; \
    XChangeGC(geDispDev, mp_gc, GCTileStipXOrigin|GCTileStipYOrigin, &geGCV); \
   }
#define GECLIPMASK(mp_gc, mp_x, mp_y, mp_z) \
   {geGCV.clip_mask     = mp_x; \
    geGCV.clip_x_origin = mp_y; \
    geGCV.clip_y_origin = mp_z; \
    XChangeGC(geDispDev, mp_gc, GCClipMask|GCClipXOrigin|GCClipYOrigin, &geGCV); \
   }
#define GEFORG(mp_gc, mp_x) \
   {geGCV.foreground = mp_x; \
    XChangeGC(geDispDev, mp_gc, GCForeground, &geGCV); \
   }
#define GEBACK(mp_gc, mp_x) \
   {geGCV.background = mp_x; \
    XChangeGC(geDispDev, mp_gc, GCBackground, &geGCV); \
   }
#define GELINW(mp_gc, mp_x) \
   {geGCV.line_width = mp_x; \
    XChangeGC(geDispDev, mp_gc, GCLineWidth, &geGCV); \
   }
#define GELINP(mp_gc, mp_x) \
   {geGCV.line_style = mp_x; \
    XChangeGC(geDispDev, mp_gc, GCLineStyle, &geGCV); \
   }
#define GEFILSTYLE(mp_gc, mp_x) \
   {geGCV.fill_style = mp_x; \
    XChangeGC(geDispDev, mp_gc, GCFillStyle, &geGCV); \
   }
/*
 * The next macro is similiar to GEFILSTYLE, except it does special
 * things for the case of FillOpaqueStippled (it sets the BACKGROUND to
 * WHITE), and even more importantly it guards against this mode being used
 * in conjunction with a white pixel
 * value, as the VMS server barfs on this combination.
 */
#define GEFILSTYLE_FOS(mp_gc, mp_x, mp_y) \
   {if (mp_x == FillOpaqueStippled) \
	{if (mp_y == geWhite.pixel) geGCV.fill_style = FillSolid; \
     	 else \
   	    {geGCV.fill_style = mp_x; \
	     GEBACK(mp_gc, geWhite.pixel); \
	    } \
	} \
    else geGCV.fill_style = mp_x; \
    XChangeGC(geDispDev, mp_gc, GCFillStyle, &geGCV); \
   }
#define GEMODE(mp_gc, mp_x) \
   {geGCV.function = mp_x; \
    XChangeGC(geDispDev, mp_gc, GCFunction, &geGCV); \
   }
#define GEFONT(mp_gc, mp_x) \
   {geGCV.font = mp_x; \
    XChangeGC(geDispDev, mp_gc, GCFont, &geGCV); \
   }
#define GECAPSTYLE(mp_gc, mp_x) \
   {geGCV.cap_style = mp_x; \
    XChangeGC(geDispDev, mp_gc, GCCapStyle, &geGCV); \
   }
#define GEJOINSTYLE(mp_gc, mp_x) \
   {geGCV.join_style = mp_x; \
    XChangeGC(geDispDev, mp_gc, GCJoinStyle, &geGCV); \
   }
#define GEL_FG_M(mp_gc, mp_x, mp_y, mp_z) \
   {geGCV.line_width = mp_x; \
    geGCV.foreground = mp_y; \
    geGCV.function   = mp_z; \
    XChangeGC(geDispDev, mp_gc, GCLineWidth|GCForeground|GCFunction, &geGCV); \
   }
#define GEL_FG_P_M(mp_gc, mp_x, mp_w, mp_y, mp_z) \
   {geGCV.line_width = mp_x; \
    geGCV.line_style = mp_w; \
    geGCV.foreground = mp_y; \
    geGCV.function   = mp_z; \
    XChangeGC(geDispDev, mp_gc, GCLineWidth|GCLineStyle|GCForeground|GCFunction, &geGCV); \
   }
#define GEL_FG_P_M_PM(mp_gc, mp_x, mp_w, mp_y, mp_z, mp_xx) \
   {geGCV.line_width = mp_x; \
    geGCV.line_style = mp_w; \
    geGCV.foreground = mp_y; \
    geGCV.function   = mp_z; \
    geGCV.plane_mask = mp_xx; \
    XChangeGC(geDispDev, mp_gc, GCLineWidth|GCLineStyle|GCForeground|GCFunction|GCPlaneMask, &geGCV); \
   }
#define GEF_FG_M(mp_gc, mp_x, mp_y, mp_z) \
   {geGCV.font       = mp_x; \
    geGCV.foreground = mp_y; \
    geGCV.function   = mp_z; \
    XChangeGC(geDispDev, mp_gc, GCFont|GCForeground|GCFunction, &geGCV); \
   }
#define GEF_FG_M_PM(mp_gc, mp_x, mp_y, mp_z, mp_xx) \
   {geGCV.font       = mp_x; \
    geGCV.foreground = mp_y; \
    geGCV.function   = mp_z; \
    geGCV.plane_mask = mp_xx; \
    XChangeGC(geDispDev, mp_gc, GCFont|GCForeground|GCFunction|GCPlaneMask, &geGCV); \
   }
#define GE_FG_M(mp_gc, mp_x, mp_y) \
   {geGCV.foreground = mp_x; \
    geGCV.function   = mp_y; \
    XChangeGC(geDispDev, mp_gc, GCForeground|GCFunction, &geGCV); \
   }
/*
 * Convert float (or double) to int, round
 */
#define GEINT(mp_x, mp_i) \
   {mp_i = (int)(mp_x); \
    geTD = (double)(mp_x) - (double)(mp_i); \
    if (geTD >  .5) {(mp_i) += 1;} \
    else \
    if (geTD < -.5) {(mp_i) -= 1;} \
   }

/*
 * Store the given point in geVert at given offset.  But first check that the
 * point does not over-under flow a SHORT - if it does, clamp it at the MAX.
 */
#define GESTOREVERT(mp_i, mp_x, mp_y) \
   {if (mp_i < GEMAX_PTS) \
	{geLT1 = mp_x; \
	 geLT2 = mp_y; \
	 if (geLT1 < -GEMAXVERT) geLT1 = -GEMAXVERT; \
	 if (geLT1 >  GEMAXVERT) geLT1 =  GEMAXVERT; \
	 if (geLT2 < -GEMAXVERT) geLT2 = -GEMAXVERT; \
	 if (geLT2 >  GEMAXVERT) geLT2 =  GEMAXVERT; \
	 geVert[mp_i].x = (short)geLT1; \
	 geVert[mp_i].y = (short)geLT2; \
	} \
   }

/* Atom macros for ICCCM
 */
#if defined (GERAGS) || defined (GEVOYER)
#define WM_PROTOCOLS_ATOM       \
    	    XmInternAtom(geDispDev, "WM_PROTOCOLS", FALSE)
#define WM_DELETE_WINDOW_ATOM    \
    	    XmInternAtom(geDispDev, "WM_DELETE_WINDOW", FALSE)
#endif
/*
 *  Initialize an XColor data structure
 */
#define  GESET_XCOLOR( c,p,r,g,b,f )  \
    {c.pixel = p; 	    	      \
     c.red = r;  	    	      \
     c.green = g;	    	      \
     c.blue = b; 	    	      \
     c.flags = f; 	    	      \
    }

/*
 * RAISE_WINDOW -- Raises a window to the top of the screen and de-iconifies
 *		       the window if need be.  
 *  	    	    NOTE: if the window manager running isn't Motif's
 *  	    	    	Mwm then assume its not ICCCM compliant
 */

#define GERAISE_WINDOW(sh_id_ptr)	    	    	\
   {   	    	    	    	    	    		\
    	Window 	win = XtWindow(sh_id_ptr);	    	\
    	Display *dpy = XtDisplay(sh_id_ptr);	    	\
    	if (win != 0) 	    	    	    		\
    	{   	    	    	    	    	    	\
    	    if (!XmIsMotifWMRunning(sh_id_ptr))		\
    	    {	    	    	    	    	    	\
	    	XWMHints    *wmhints;	    	    	\
    	    	wmhints = XGetWMHints(dpy, win);  	\
	    	wmhints->flags |= StateHint;	    	\
	    	wmhints->initial_state = NormalState;	\
	    	XSetWMHints(dpy, win, wmhints);   	\
    	    	XFree(wmhints);   	    	    	\
    	    }	    	    	    	    	    	\
	    XMapRaised(dpy, win);   	    	    	\
    	}   	    	    	    	    	    	\
   }

/*
 *  ICONIFY_WINDOW - iconifies a window given its shell widget id.
 *  	    	     NOTE: if the window manager running isn't Motif's
 *  	    	    	Mwm then assume its not ICCCM compliant
 */

#define GEICONIFY_WINDOW(sh_id_ptr) 	    	    	\
    {   	    	    	    	    	    	\
    	Window 	    win = XtWindow(sh_id_ptr);		\
    	Display     *dpy = XtDisplay(sh_id_ptr);	\
    	int 	    screen = XDefaultScreen(dpy);	\
    	if (win != 0) 	 	   	    	    	\
    	{   	    	    	    	    	    	\
    	    if (!XmIsMotifWMRunning(sh_id_ptr))		\
    	    {	    	    	    	    	    	\
	    	XWMHints    *wmhints;	    	    	\
    	    	wmhints = XGetWMHints(dpy, win);  	\
	    	wmhints->flags |= StateHint;	    	\
	    	wmhints->initial_state = IconicState;	\
	    	XSetWMHints(dpy, win, wmhints);   	\
    	    	XFree(wmhints);   	    	    	\
    	    }	    	    	    	    	    	\
    	    XIconifyWindow(dpy, win, screen);	    	\
    	}   	    	    	    	    	    	\
    }

#define GESET_ARG(mp_widg, mp_resource, mp_value) 	\
   {							\
      if (mp_widg)	                                \
        {XtSetArg(al[0], mp_resource, mp_value);	\
         XtSetValues(mp_widg, al, 1);                   \
	}						\
   }                                                    

#define GE_MANAGE(mp_widg)				      \
   {                                                          \
      if (XtIsManaged(mp_widg))				      \
    	XRaiseWindow(geDispDev, XtWindow(XtParent(mp_widg))); \
      else XtManageChild(mp_widg);			      \
   }

/*
 * Set Foreground attributes - for screen display only
 */
#define GE_SET_FG_DISP_ATTR \
   {if ((geState.ForceHTDisp || GEMONO) && priv->LineStyle != GETRANSPARENT) \
	{geDispAttr.FgLumi = (float)priv->Col.RGB.red + \
			     (float)priv->Col.RGB.green + \
			     (float)priv->Col.RGB.blue; \
	 if (geDispAttr.FgLumi > 5000. && geDispAttr.FgLumi < (float)(GEMAXUSHORT3 - 5000.)) \
	    {if (priv->LineStyle == FillSolid || priv->LineHT == 100) \
		geDispAttr.FgStyle = FillOpaqueStippled; \
	     else \
		geDispAttr.FgStyle = priv->LineStyle; \
 	     geDispAttr.FgHT = (int)((float)priv->LineHT * \
		((float)GEMAXUSHORT3 - geDispAttr.FgLumi) / GEMAXUSHORT3); \
	     geDispAttr.FgPixel = geBlack.pixel; \
	    } \
    	 else \
	    {geDispAttr.FgStyle = priv->LineStyle; \
	     geDispAttr.FgHT    = priv->LineHT; \
	     geDispAttr.FgPixel = priv->Col.RGB.pixel; \
	    } \
	} \
    else \
	{geDispAttr.FgStyle = priv->LineStyle; \
	 geDispAttr.FgHT    = priv->LineHT; \
	 geDispAttr.FgPixel = priv->Col.RGB.pixel; \
	} \
   }

/*
 * Set Background attributes - for screen display only
 */
#define GE_SET_BG_DISP_ATTR \
   {if ((geState.ForceHTDisp || GEMONO) && ASegP->FillStyle != GETRANSPARENT) \
	{geDispAttr.BgLumi = (float)ASegP->Col.RGB.red + \
			     (float)ASegP->Col.RGB.green + \
			     (float)ASegP->Col.RGB.blue; \
	 if (geDispAttr.BgLumi > 5000. && geDispAttr.BgLumi < (float)(GEMAXUSHORT3 - 5000.)) \
	    {if (ASegP->FillStyle == FillSolid || ASegP->FillHT == 100) \
		geDispAttr.BgStyle = FillOpaqueStippled; \
	     else \
		geDispAttr.BgStyle = ASegP->FillStyle; \
 	     geDispAttr.BgHT = (int)((float)ASegP->FillHT * \
		((float)GEMAXUSHORT3 - geDispAttr.BgLumi) / GEMAXUSHORT3); \
	     geDispAttr.BgPixel = geBlack.pixel; \
	    } \
    	 else \
	    {geDispAttr.BgStyle = ASegP->FillStyle; \
	     geDispAttr.BgHT    = ASegP->FillHT; \
	     geDispAttr.BgPixel = ASegP->Col.RGB.pixel; \
	    } \
	} \
    else \
	{geDispAttr.BgStyle = ASegP->FillStyle; \
	 geDispAttr.BgHT    = ASegP->FillHT; \
	 geDispAttr.BgPixel = ASegP->Col.RGB.pixel; \
	} \
   }

/*
 * Set Foreground attributes - for screen display only
 */
#define GE_SET_FG_SAMPLE \
   {if ((geState.ForceHTDisp || GEMONO) && geSampleAttr.LineStyle != GETRANSPARENT) \
	{geDispAttr.FgLumi = (float)geSampleAttr.LinCol.RGB.red + \
			     (float)geSampleAttr.LinCol.RGB.green + \
			     (float)geSampleAttr.LinCol.RGB.blue; \
	 if (geDispAttr.FgLumi > 5000. && geDispAttr.FgLumi < (float)(GEMAXUSHORT3 - 5000.)) \
	    {if (geSampleAttr.LineStyle == FillSolid || geSampleAttr.LineHT == 100) \
		geDispAttr.FgStyle = FillOpaqueStippled; \
	     else \
		geDispAttr.FgStyle = geSampleAttr.LineStyle; \
 	     geDispAttr.FgHT = (int)((float)geSampleAttr.LineHT * \
		((float)GEMAXUSHORT3 - geDispAttr.FgLumi) / GEMAXUSHORT3); \
	     geDispAttr.FgPixel = geBlack.pixel; \
	    } \
    	 else \
	    {geDispAttr.FgStyle = geSampleAttr.LineStyle; \
	     geDispAttr.FgHT    = geSampleAttr.LineHT; \
	     geDispAttr.FgPixel = geSampleAttr.LinCol.RGB.pixel; \
	    } \
	} \
    else \
	{geDispAttr.FgStyle = geSampleAttr.LineStyle; \
	 geDispAttr.FgHT    = geSampleAttr.LineHT; \
	 geDispAttr.FgPixel = geSampleAttr.LinCol.RGB.pixel; \
	} \
   }

/*
 * Set Background attributes - for screen display only
 */
#define GE_SET_BG_SAMPLE \
   {if ((geState.ForceHTDisp || GEMONO) && geSampleAttr.FillStyle != GETRANSPARENT) \
	{geDispAttr.BgLumi = (float)geSampleAttr.FilCol.RGB.red + \
			     (float)geSampleAttr.FilCol.RGB.green + \
			     (float)geSampleAttr.FilCol.RGB.blue; \
	 if (geDispAttr.BgLumi > 5000. && geDispAttr.BgLumi < (float)(GEMAXUSHORT3 - 5000.)) \
	    {if (geSampleAttr.FillStyle == FillSolid || geSampleAttr.FillHT == 100) \
		geDispAttr.BgStyle = FillOpaqueStippled; \
	     else \
		geDispAttr.BgStyle = geSampleAttr.FillStyle; \
 	     geDispAttr.BgHT = (int)((float)geSampleAttr.FillHT * \
		((float)GEMAXUSHORT3 - geDispAttr.BgLumi) / GEMAXUSHORT3); \
	     geDispAttr.BgPixel = geBlack.pixel; \
	    } \
    	 else \
	    {geDispAttr.BgStyle = geSampleAttr.FillStyle; \
	     geDispAttr.BgHT    = geSampleAttr.FillHT; \
	     geDispAttr.BgPixel = geSampleAttr.FilCol.RGB.pixel; \
	    } \
	} \
    else \
	{geDispAttr.BgStyle = geSampleAttr.FillStyle; \
	 geDispAttr.BgHT    = geSampleAttr.FillHT; \
	 geDispAttr.BgPixel = geSampleAttr.FilCol.RGB.pixel; \
	} \
   }


/*
 * Set Foreground attributes for TEXT - for screen display only
 */
#define GE_SET_FG_SAMPLE_TXT \
   {if ((geState.ForceHTDisp || GEMONO) && geSampleAttr.TxtFgStyle != GETRANSPARENT) \
	{geDispAttr.FgLumi = (float)geSampleAttr.TxtFgCol.RGB.red + \
			     (float)geSampleAttr.TxtFgCol.RGB.green + \
			     (float)geSampleAttr.TxtFgCol.RGB.blue; \
	 if (geDispAttr.FgLumi > 5000. && geDispAttr.FgLumi < (float)(GEMAXUSHORT3 - 5000.)) \
	    {if (geSampleAttr.TxtFgStyle == FillSolid || geSampleAttr.TxtFgHT == 100) \
		geDispAttr.FgStyle = FillOpaqueStippled; \
	     else \
		geDispAttr.FgStyle = geSampleAttr.TxtFgStyle; \
 	     geDispAttr.FgHT = (int)((float)geSampleAttr.TxtFgHT * \
		((float)GEMAXUSHORT3 - geDispAttr.FgLumi) / GEMAXUSHORT3); \
	     geDispAttr.FgPixel = geBlack.pixel; \
	    } \
    	 else \
	    {geDispAttr.FgStyle = geSampleAttr.TxtFgStyle; \
	     geDispAttr.FgHT    = geSampleAttr.TxtFgHT; \
	     geDispAttr.FgPixel = geSampleAttr.TxtFgCol.RGB.pixel; \
	    } \
	} \
    else \
	{geDispAttr.FgStyle = geSampleAttr.TxtFgStyle; \
	 geDispAttr.FgHT    = geSampleAttr.TxtFgHT; \
	 geDispAttr.FgPixel = geSampleAttr.TxtFgCol.RGB.pixel; \
	} \
   }

/*
 * Set Background attributes for TEXT - for screen display only
 */
#define GE_SET_BG_SAMPLE_TXT \
   {if ((geState.ForceHTDisp || GEMONO) && geSampleAttr.TxtBgStyle != GETRANSPARENT) \
	{geDispAttr.BgLumi = (float)geSampleAttr.TxtBgCol.RGB.red + \
			     (float)geSampleAttr.TxtBgCol.RGB.green + \
			     (float)geSampleAttr.TxtBgCol.RGB.blue; \
	 if (geDispAttr.BgLumi > 5000. && geDispAttr.BgLumi < (float)(GEMAXUSHORT3 - 5000.)) \
	    {if (geSampleAttr.TxtBgStyle == FillSolid || geSampleAttr.TxtBgHT == 100) \
		geDispAttr.BgStyle = FillOpaqueStippled; \
	     else \
		geDispAttr.BgStyle = geSampleAttr.TxtBgStyle; \
 	     geDispAttr.BgHT = (int)((float)geSampleAttr.TxtBgHT * \
		((float)GEMAXUSHORT3 - geDispAttr.BgLumi) / GEMAXUSHORT3); \
	     geDispAttr.BgPixel = geBlack.pixel; \
	    } \
    	 else \
	    {geDispAttr.BgStyle = geSampleAttr.TxtBgStyle; \
	     geDispAttr.BgHT    = geSampleAttr.TxtBgHT; \
	     geDispAttr.BgPixel = geSampleAttr.TxtBgCol.RGB.pixel; \
	    } \
	} \
    else \
	{geDispAttr.BgStyle = geSampleAttr.TxtBgStyle; \
	 geDispAttr.BgHT    = geSampleAttr.TxtBgHT; \
	 geDispAttr.BgPixel = geSampleAttr.TxtBgCol.RGB.pixel; \
	} \
   }

/*
 * Set Background attributes for the PAGE - for screen display only
 */
#define GE_SET_BG_DISP_ATTR_PAG \
   {if ((geState.ForceHTDisp || GEMONO) && geSeg0->FillStyle != GETRANSPARENT) \
	{geDispAttr.BgLumi = (float)geSeg0->Col.RGB.red + \
			     (float)geSeg0->Col.RGB.green + \
			     (float)geSeg0->Col.RGB.blue; \
	 if (geDispAttr.BgLumi > 5000. && geDispAttr.BgLumi < (float)(GEMAXUSHORT3 - 5000.)) \
	    {if (geSeg0->FillStyle == FillSolid || geSeg0->FillHT == 100) \
		geDispAttr.BgStyle = FillOpaqueStippled; \
	     else \
		geDispAttr.BgStyle = geSeg0->FillStyle; \
 	     geDispAttr.BgHT = (int)((float)geSeg0->FillHT * \
		((float)GEMAXUSHORT3 - geDispAttr.BgLumi) / GEMAXUSHORT3); \
	     geDispAttr.BgPixel = geBlack.pixel; \
	    } \
    	 else \
	    {geDispAttr.BgStyle = geSeg0->FillStyle; \
	     geDispAttr.BgHT    = geSeg0->FillHT; \
	     geDispAttr.BgPixel = geSeg0->Col.RGB.pixel; \
	    } \
	} \
    else \
	{geDispAttr.BgStyle = geSeg0->FillStyle; \
	 geDispAttr.BgHT    = geSeg0->FillHT; \
	 geDispAttr.BgPixel = geSeg0->Col.RGB.pixel; \
	} \
   }

/*
 * Set Background attributes for the PAGE - for screen display only
 */
#define GE_SET_BG_SAMPLE_PAG \
   {if ((geState.ForceHTDisp || GEMONO) && geSampleAttr.PagStyle != GETRANSPARENT) \
	{geDispAttr.BgLumi = (float)geSampleAttr.PagCol.RGB.red + \
			     (float)geSampleAttr.PagCol.RGB.green + \
			     (float)geSampleAttr.PagCol.RGB.blue; \
	 if (geDispAttr.BgLumi > 5000. && geDispAttr.BgLumi < (float)(GEMAXUSHORT3 - 5000.)) \
	    {if (geSampleAttr.PagStyle == FillSolid || geSampleAttr.PagHT == 100) \
		geDispAttr.BgStyle = FillOpaqueStippled; \
	     else \
		geDispAttr.BgStyle = geSampleAttr.PagStyle; \
 	     geDispAttr.BgHT = (int)((float)geSampleAttr.PagHT * \
		((float)GEMAXUSHORT3 - geDispAttr.BgLumi) / GEMAXUSHORT3); \
	     geDispAttr.BgPixel = geBlack.pixel; \
	    } \
    	 else \
	    {geDispAttr.BgStyle = geSampleAttr.PagStyle; \
	     geDispAttr.BgHT    = geSampleAttr.PagHT; \
	     geDispAttr.BgPixel = geSampleAttr.PagCol.RGB.pixel; \
	    } \
	} \
    else \
	{geDispAttr.BgStyle = geSampleAttr.PagStyle; \
	 geDispAttr.BgHT    = geSampleAttr.PagHT; \
	 geDispAttr.BgPixel = geSampleAttr.PagCol.RGB.pixel; \
	} \
   }

/*
 * Set object attributes to be the same as the current page background.
 */
#define GE_SET_ATTR_ERASE \
   {priv->Col.RGB          = geSeg0->Col.RGB; \
    priv->Col.CMYK         = geSeg0->Col.CMYK; \
    priv->Col.Name         = geSeg0->Col.Name; \
    priv->LineHT           = 100; \
    priv->LineStyle        = FillSolid; \
    priv->WritingMode      = GXcopy; \
    priv->Pat.LinP         = LineSolid; \
    priv->Pat.DashList[0]  = 2; \
    priv->Pat.DashList[1]  = 2; \
    priv->Pat.DashList[2]  = 2; \
    priv->Pat.DashList[3]  = 2; \
    priv->Pat.DashLen      = 2; \
    priv->Pat.DashIndx     = 0; \
    ASegP->Col.RGB         = geSeg0->Col.RGB; \
    ASegP->Col.CMYK        = geSeg0->Col.CMYK; \
    ASegP->Col.Name        = geSeg0->Col.Name; \
    ASegP->FillHT	  = 100; \
    ASegP->FillStyle       = FillSolid; \
    ASegP->FillWritingMode = GXcopy; \
   }

/*
 * Set object attributes to be White
 */
#define GE_SET_ATTR_ERASE_WHITE \
   {priv->Col.RGB        = geWhite; \
    priv->Col.Name       = "White"; \
    geGenRgbToCmyk(&(priv->Col.RGB), &(priv->Col.CMYK)); \
    ASegP->Col.RGB       = geWhite; \
    ASegP->Col.Name      = "White"; \
    geGenRgbToCmyk(&(ASegP->Col.RGB), &(ASegP->Col.CMYK)); \
   }
