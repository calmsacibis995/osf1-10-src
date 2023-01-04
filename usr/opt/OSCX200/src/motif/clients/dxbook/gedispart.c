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
static char SccsId[] = "@(#)gedispart.c	1.2\t11/6/89";
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
**      GEDISPART             		Put up drawing into given window
**
**  ABSTRACT:
**
**	Main entry point for VOILA and VOYER.
**
**  ENVIRONMENT:
**
**	VAX/VMS, ULTRIX Operating Systems
**
**  MODIFICATION HISTORY:
**
**      GNE 10/21/87 Created
**
**--
**/

#include "geGks.h"

extern               geImg();
extern char          *geMalloc();
extern XImage        *geImgCr();

static char          geFile[GE_MAX_MSG_CHAR];

geDispArt(cmd, DispDev, Win, Buf, Len, Seg0, xoff, yoff, xclip, yclip,
	  wclip, hclip, Type, Event)
long                    cmd;			/* One of GEINIT, GEREAD,    */
						/* GEREADM, GECDISP, GEKILL  */
						/* GETERM, GEWEVENT          */
Display			*DispDev;		/* Display connection ID     */
Window			Win;			/* Window ID                 */
char			*Buf;			/* FileName or Buffer Pointer*/
int                     Len;			/* NULL     or Buff Len      */
struct GE_SEG           **Seg0;			/* Address of Segment pointer*/
int                     xoff, yoff,		/* Offset from ULC of window */
						/* to ULC of drawing         */
						/* (positive y axis pointing */
						/* DOWN)                     */
			xclip, yclip,		/* Clip offset and extent    */
			wclip, hclip;		/* into graph                */
						/* (wclip, hclip =0=display  */
						/* WHOLE drawing)            */
int    			Type;			/* What kind of file is it   */
XButtonEvent		*Event;			/* The event - maybe         */

{
char		*error_string;
short           Locked;

unsigned long   pixel;
unsigned int    width, height;
struct GE_IMG   *priv;
struct GE_SEG	*ASegP;
struct GE_PAG	*APagP;
struct GE_STATE	State;

XImage          *ImgPtr = NULL;

XWindowAttributes	watts;

State = geState;

if (DispDev) geDispDev     = DispDev;
if (Win)     geRgRoot.self = Win;
/*
 * For all commands except INIT, TERM, READ and READM have to locate the
 * page to which the segment or window belongs.
 */
if (cmd != GEINIT && cmd != GETERM && cmd != GEREAD && cmd != GEREADM)
  {if (!gePag0) return(-1);			/* Should never happen ...  */

   APagP = NULL;				/* Don't have it yet	     */

   if (Seg0 && (ASegP = *Seg0))
     {/*
       * A seg id was passed in - try to locate page based on the seg id.
       *
       * First try the currently active page, which is what it's
       * going to be most often.
       */
      if (geState.APagP && geState.APagP->Seg0 == ASegP)
	APagP = geState.APagP;
      else
	{APagP = gePag0->Next;
	 while (APagP && APagP->Seg0 != ASegP) APagP = APagP->Next;
        }
     }
   else if (Win || Event && (Win = Event->window))
     {/*
       * Don't have a seg id but a window id was passed in - try to locate page
       * based on the window id.
       *
       * First try the currently active page, which is what it's
       * going to be most often.
       */
      if (geState.APagP && geState.APagP->Surf.self == Win)
	APagP = geState.APagP;
      else
	{APagP = gePag0->Next;
	 while (APagP && APagP->Surf.self != Win) APagP = APagP->Next;
        }
     }

   if (APagP)					/* Got it?		     */
     {/*
       * Yep, FOUND IT
       */
      geState.APagP  = APagP;
      geSeg0         = *Seg0 = APagP->Seg0;
      if (Win) geState.APagP->Surf.self = Win;
      geState.Window = geState.APagP->Surf.self;
      geState.Pixmap = geState.APagP->Pixmap;
      if (geState.Pixmap)
	geState.Drawable = geState.Pixmap;
      else
	geState.Drawable = geState.Window;
      if (geState.APagP->Surf.self) geRgRoot.self = geState.APagP->Surf.self;
     }
   else return(-1);				/* Couldn't find the page    */
  }

switch(cmd)
  {case GEINIT:
    if (gePag0) break;                          /* Already done once         */

    if (!Win) return(-1);                       /* Can't do it without a wind*/

    geRgRoot.self = Win;
#ifndef GEVOYER
    geGlobin1(); 				/* ALL globals               */
#endif
    geLoadSup();
    geGlobin2(); 				/* RAGS globals              */
    geInitHT();					/* Create stipples for HT    */
    gePagCr("Root Page");			/* Create root page          */
    geState.APagP = NULL;			/* Still no active page      */
    break;

  case GEREAD:
  case GEREADM:
    if (!Buf || !gePag0) return(-1);		/* Something drasticaly wrong*/

    gePagCr(Buf);                            	/* Create user page          */
    if (Win) geState.APagP->Surf.self = Win;
    Locked 	          = geState.Locked;
    geState.Locked	  = TRUE;
    geState.HaltRequested = FALSE;
    geState.HaltCmd	  = 0;
    if (cmd == GEREADM)
      {geMemIO.InMem     = TRUE;
       geMemIO.PtrS      = geMemIO.PtrC = Buf;
       geMemIO.NumBytes  = Len;
       geMemIO.PtrE      = geMemIO.PtrS + Len;
       Buf               = NULL;
      }
    else
      geMemIO.InMem = FALSE;

    geInFile = Buf;				/* Ptr to file name	     */
    
    geReadWriteType = GE_RW_ASCII;
    if (Type == GEIMGTYPE_NONE)
      {/*
	* Caller has NOT indicated the file type
	*
	* Try to read it first as a metafile.  If:
	*     1) File is NOT a metafile => try to read it as one of the IMPORT
	*        types.
	*     2) For all other types of errors => report it and return -1.
	*/
	geStat = geRead(geSeg0);
	if (geStat == GERRNOTMETA)		/* Try to figure out what    */
	  {if (geObjCrStat(GESHIFT_UL, GEIMGTYPE_NONE)) /* kind of file it   */
	     {if (geState.HaltRequested && geState.HaltCmd) 
		{geState.Locked = FALSE;	/* might be 		     */
		 geDispArt(geState.HaltCmd, DispDev, Win, Buf, Len, Seg0,
			   xoff, yoff, xclip, yclip, 0, 0, Type, Event);
		 geState.HaltRequested = FALSE;
		 geState.HaltCmd       = 0;
	        }
	      geState.Locked = Locked;		/* might be  		     */
	      return(-1);			/* ERROR		     */
	     }
	   strcpy(geFile, geUtilBuf);
	  }
	else if (geStat)                        /* ALL other errors get      */
	  {geErrorReport(geStat, FALSE);        /* reported and STOP         */
	   if (geState.HaltRequested && geState.HaltCmd) 
	     {geState.Locked = FALSE;	/* might be 		     */
	      geDispArt(geState.HaltCmd, DispDev, Win, Buf, Len, Seg0,
			xoff, yoff, xclip, yclip, 0, 0, Type, Event);
	      geState.HaltRequested = FALSE;
	      geState.HaltCmd       = 0;
	      geState.Locked = Locked;		/* might be  		     */
	     }
	   return(-1);
	  }
	else                                    /* OK - it's a metafile      */
	  strcpy(geFile, "Metafile");
      }
    else
      {/*
	* Caller has specified the file type.  It must match exactly or
	* produce an error.
	*/
       if (Type == GEIMGTYPE_RAGS) geStat = geRead(geSeg0);
       else 			   geStat = geObjCrStat(GESHIFT_UL, Type);

       if (geStat)                           	/* Report error and	     */
	 {geErrorReport(geStat, FALSE);      	/* STOP         	     */
	  if (geState.HaltRequested && geState.HaltCmd) 
	    {geState.Locked = FALSE;	/* might be 		     */
	     geDispArt(geState.HaltCmd, DispDev, Win, Buf, Len, Seg0,
		       xoff, yoff, xclip, yclip, 0, 0, Type, Event);
	     geState.HaltRequested = FALSE;
	     geState.HaltCmd       = 0;
	     geState.Locked = Locked;		/* might be  		     */
	    }
	  return(-1);
	 }
       if (Type == GEIMGTYPE_RAGS)
	 strcpy(geFile, "Metafile");
      }

    GEVEC(GEBOUNDS, geSeg0);
    geState.PrintPgBg = geState.PrintPgBgNu;

    if (xoff != -1 || yoff != -1)
      {geState.Dx = - geState.APagP->Crop.x1;
       geState.Dy = - geState.APagP->Crop.y1;
       geGrf(GEMOVE, geSeg0);			/* Adj. pos. for crop offst. */
       geGrf(GEBOUNDS, geSeg0);

       geState.APagP->Crop.x2 -= geState.APagP->Crop.x1;
       geState.APagP->Crop.y2 -= geState.APagP->Crop.y1;
       geState.APagP->Crop.x1  = geState.APagP->Crop.y1 = 0;
      }
    *Seg0 = geSeg0;
    strcpy(geUtilBuf, geFile);

    if (geState.HaltRequested && geState.HaltCmd) 
      {geState.Locked = FALSE;	/* might be 		     */
       geDispArt(geState.HaltCmd, DispDev, Win, Buf, Len, Seg0,
		 xoff, yoff, xclip, yclip, 0, 0, Type, Event);
       geState.HaltRequested = FALSE;
       geState.HaltCmd       = 0;
       geState.Locked        = Locked;		/* might be  		     */
     }
    geState.Locked = Locked;
    
    break;

  case GECDISP:
    if (!Win || !gePag0 || !gePag0->Next) return(-1);

    if (strlen(geUtilBuf))
      strcpy(geFile, geUtilBuf);
    else
      geFile[0] ='\0';

    XGetWindowAttributes(geDispDev, Win, &watts);
    XSync(geDispDev, FALSE);/* Make sure if ERR, it gets reprtd*/
    XFlush(geDispDev);

    if (geErr || (watts.map_state == IsUnviewable ||
		  watts.map_state == IsUnmapped))
      return(-1);				/* Can't do it		     */

    geState.APagP->Surf.width  = watts.width;
    geState.APagP->Surf.height = watts.height;
    /*
     * If animation preview is allowed and not already in it and want to use
     * Pixmaps to perform the animation and (whew!) haven't yet created the
     * Pixmap, then set it up now.
     */
    if (geState.APagP->Seg0->AnimCtrl.Animated && geState.AnimPreview &&
	geState.State != GEANIMPREVIEW && geState.APagP->PixmapAnim &&
	!geState.APagP->Pixmap)
      {geClipAnim.x1 = geClipAnim.y1 = geClipAnim.x2 = geClipAnim.y2 = -1;
       wclip = abs(GETPIX(geState.APagP->Crop.x2 - geState.APagP->Crop.x1));
       hclip = abs(GETPIX(geState.APagP->Crop.y2 - geState.APagP->Crop.y1));

       if (wclip && hclip)
	 {geErr = 0;
	  geState.APagP->Pixmap =
	    XCreatePixmap(DispDev, geState.APagP->Surf.self,
			  wclip, hclip, watts.depth);
			  
	  XSync(geDispDev, FALSE);/* Make sure if ERR, it gets reprtd*/
	  if (geErr || !geState.APagP->Pixmap)
	    {/*
	      * Pixmap alloc has failed, so do the best we
	      * can and display the animation without using a Pixmap
	      */
	     geState.APagP->Pixmap     = NULL;
	     geState.APagP->PixmapAnim = FALSE;
	    }
	  else
	    {/*
	      * Provide the Pixmap with a background.  If possible use the
	      * window's contents, if not, just white it out.
	      */
	     if (watts.map_state == IsUnviewable ||
		 watts.map_state == IsUnmapped ||
		 geState.APagP->Surf.width  - xoff < wclip ||
		 geState.APagP->Surf.height - yoff < hclip)
	       {GEFILSTYLE_FOS(geGC3, FillSolid, geWhite.pixel);
		GE_FG_M(geGC3, geWhite.pixel, GXcopy);
		XFillRectangle(geDispDev, geState.APagP->Pixmap, geGC3, 0, 0,
			       wclip, hclip);
	       }
	     else
	       {XCopyArea(geDispDev, Win, geState.APagP->Pixmap, geGC0,
			  xoff, yoff, wclip, hclip, 0, 0);

		ImgPtr  = XGetImage(geDispDev, Win, 0, 0, 1, 1, XAllPlanes(),
			  ZPixmap);
		pixel = XGetPixel(ImgPtr, 0, 0);
		geXDestImg(ImgPtr);
		geSeg0->FillStyle = FillSolid;
		geSeg0->FillWritingMode = GXcopy;
		geSeg0->Col.RGB.pixel = pixel;
	       }
	    }
	     
	 }
       else
	 {if (geState.HaltRequested && geState.HaltCmd) 
	    {geState.Locked = FALSE;
	     geDispArt(geState.HaltCmd, DispDev, Win, Buf, Len, Seg0,
		       xoff, yoff, xclip, yclip, 0, 0, Type, Event);
	     geState.HaltRequested = FALSE;
	     geState.HaltCmd       = 0;
	    }
	  geState = State;
	  if (geState.APagP) geSeg0 = geState.APagP->Seg0;
	  return(-1);			/* If Pg width or height=0,forget it */
	 }
      }
    /*
     * Set up window, drawable and pixmap pointers to point at this drawing.
     */
    geState.Window = geState.APagP->Surf.self = Win;
    geState.Pixmap = geState.APagP->Pixmap;
    /*
     * Save off user requested offsets in the drawing window
     */
    geState.APagP->ViewPortXoff = xoff;
    geState.APagP->ViewPortYoff = yoff;

    if (geState.Pixmap)
      geState.Drawable = geState.Pixmap;
    else
      {geState.Drawable = geState.Window;
       /*
	* Adjust position of drawing for user requested offset in window
	*/
       if (geState.APagP->ViewPortXoff || geState.APagP->ViewPortYoff)
	 {geState.Dx = GETSUI(geState.APagP->ViewPortXoff);
	  geState.Dy = GETSUI(geState.APagP->ViewPortYoff);
	  geGrf(GEMOVE, geSeg0);		/* Adj. pos. for crop offst. */
	 }
      }

    geGrf(GEBOUNDS, geSeg0);
    if (wclip > 0 && hclip > 0)
      {geClip.x1 = GETSUI(xclip);
       geClip.y1 = GETSUI(yclip);
       if (geState.Pixmap)
	 {geClip.x1 -= GETSUI(geState.APagP->ViewPortXoff);
	  geClip.y1 -= GETSUI(geState.APagP->ViewPortYoff);
	 }
       geClip.x2 = geClip.x1 + GETSUI(wclip);
       geClip.y2 = geClip.y1 + GETSUI(hclip);
       geState.ASegP = NULL;
       geGrf(GECDISP, geSeg0);                   /* Display the drawing      */
      }
    else
      {geClip.x1 = geClip.y1 = 0;
       geClip.x2 = geClip.y2 = GETSUI(1024);
       geGrf(GEDISP, geSeg0);                    /* Display the drawing      */
      }

    /*
     * If the drawing is being constructed in a pixmap and there are
     * elements on top of animated segments which themselves are not
     * animated in any way, post the pixmap now to flush them to the window.
     */
    if (geState.Pixmap)
      {geClipAnim.x1 = geClipAnim.y1 = geClipAnim.x2 = geClipAnim.y2 = -1;
       if (geState.NeedToPostPixmap)
	 gePostIt();
      }
    else
      {if (geState.APagP->ViewPortXoff || geState.APagP->ViewPortYoff)
	 {geState.Dx = -GETSUI(geState.APagP->ViewPortXoff);
	  geState.Dy = -GETSUI(geState.APagP->ViewPortYoff);
	  geGrf(GEMOVE, geSeg0);	         /* Reset position           */
	  geGrf(GEBOUNDS, geSeg0);
	 }
      }

    if (geState.HaltRequested && geState.HaltCmd) 
      {geState.Locked = FALSE;
       geDispArt(geState.HaltCmd, DispDev, Win, Buf, Len, Seg0,
		 xoff, yoff, xclip, yclip, 0, 0, Type, Event);
       geState.HaltRequested = FALSE;
       geState.HaltCmd       = 0;
      }

    geState = State;
    if (geState.APagP) geSeg0 = geState.APagP->Seg0;
    break;

  case GEKILL:
    if (geState.Locked)				/* The drawing is currently  */
      {geState.HaltRequested = TRUE;		/* locked, so cannot im-     */
       geState.HaltCmd = cmd;			/* mediately execute the com-*/
       break;					/* mand; so request HALT and */
      }						/* stack the cmd for later ex*/

    gePagKil(geState.APagP);
    geState.APagP = NULL;
    geSeg0 	  = NULL;
    break;

  case GETERM:
    if (!gePag0) return(-1);

    if (geState.Locked)				/* The drawing is currently  */
      {geState.HaltRequested = TRUE;		/* locked, so cannot im-     */
       geState.HaltCmd = cmd;			/* mediately execute the com-*/
       break;					/* mand; so request HALT and */
      }						/* stack the cmd for later ex*/

    geTerm();					/* Clean up		     */
    break;

  case GEWEVENT:
    if (!Win || !gePag0 || !gePag0->Next || !Event) return(-1);
    
    if (Win = Event->window)
      {if (Event->button == GEBUT_L)
	 {/*
	   * It's MB1 press in the right window.
	   *
	   * See if somebody's block flag is being toggled
	   */
	   geInterrupt(geSeg0, Event);
	   if (geSeg0->AnimCtrl.Interrupted)
	     {/*
	       * Somebody's block flag was toggled - redisplay the whole
	       * drawing
	       */
	       geGrf(GEERASE, geSeg0);
	       geDispArt(GECDISP, DispDev, Win, Buf, Len, Seg0,
			 geState.APagP->ViewPortXoff,
			 geState.APagP->ViewPortYoff,
			 0, 0, 0, 0, Type, Event);
	     }
	 }
       else if (Event->button == GEBUT_R)
	 {/*
	   * It's an MB3 - ABORT.
	   */
	   geDispArt(GETERM, DispDev, Win, Buf, Len, Seg0, xoff, yoff,
		     xclip, yclip, 0, 0, Type, Event);
	 }
      }
    break;

  default:
    geGrf(cmd, geSeg0);                         /* Pass cmd on to grf handler*/
    break;
  }

return(0);
}
