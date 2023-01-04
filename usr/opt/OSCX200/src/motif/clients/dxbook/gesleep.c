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
static char SccsId[] = "@(#)gesleep.c	1.3\t1/17/89";
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
** 	GESLEEP	             	       Sleeper
**
**  ABSTRACT:
**
**	This routine will idle until either the specified time has ellapsed
**	or, if the interrupt flag is set, till some event occurs other than:
**		EnterNotify
**		LeaveNotify
**		MotionNotify
**		
**
**  ENVIRONMENT:
**
**	VAX/VMS, ULTRIX Operating Systems
**
**  MODIFICATION HISTORY:
**
** 	GNE 02/09/87 Created
**
**--                
**/
#include "geGks.h"
#include <math.h>
#ifdef __osf__
#include <sys/time.h>
#include <sys/timeb.h>
#else
#include <timeb.h>
#endif

extern	              		geGrp(), gePol();
extern struct GE_ANIM_FRAME	*geGetFrameP();

/*
#include <sys/time.h>
#include <signal.h>
*/

unsigned int
geSleep(ms, interrupt)
unsigned long ms;
int interrupt;
{                       
unsigned long    deltat;
struct timeb     stimep, ctimep;
/*
struct itimerval v, ov;
struct sigvec    vec;

vec.sv_handler = SIG_IGN;

getitimer(ITIMER_REAL, &ov);
getitimer(ITIMER_REAL, &v);
v.it_value.tv_usec += ms * 100;
setitimer(ITIMER_REAL, &v, &ov);
sigvec(SIGALRM, &vec, 0);
sigpause(sigblock(SIGALRM));
*/

ftime(&stimep);
deltat = 0;

while (deltat < ms)
   {if (interrupt)
      {while (XPending(geDispDev))
	 {XNextEvent(geDispDev, &geState.Event);
	  if (geState.Event.type == GE_Mx || geState.Event.type == GE_We ||
	      geState.Event.type == GE_Wx)
	    continue;
	  else
	    {XPutBackEvent(geDispDev, &geState.Event);
	     return(1);
	    }
         }
      }

    ftime(&ctimep);
    deltat = (ctimep.time - stimep.time) * 1000 +
             ctimep.millitm - stimep.millitm;
   }

return(XPending(geDispDev));
}

unsigned int
geSleepInterrupt(ASegP, ms)
struct GE_SEG *ASegP;
unsigned long ms;
{                       
unsigned long   deltat;
struct timeb    stimep, ctimep;

if(ms <= 0 && geInterrupt(ASegP, NULL))
  return(1);


ftime(&stimep);
deltat = 0;

do
  {if(geInterrupt(ASegP, NULL))
     return (((ms - deltat) > 0 ? (ms - deltat) : 1));

   ftime(&ctimep);
   deltat = (ctimep.time - stimep.time) * 1000 +
     ctimep.millitm - stimep.millitm;
   } while (deltat < ms);


return(0);
}

gePostIt()
{
int	     src_x, src_y, dst_x, dst_y;
unsigned int w, h;
struct GE_CO ClipSav;

XWindowAttributes 	watts;

if (!geState.APagP || !geState.APagP->PixmapAnim)
  {XFlush(geDispDev);
   return;
  }

if (!geState.APagP->Surf.self || !geState.APagP->Pixmap)
  return;

if (geClipAnim.x1 == -1 && geClipAnim.y1 == -1 &&
    geClipAnim.x2 == -1 && geClipAnim.y2 == -1)
  {if (geState.APagP->Surf.width == -1 ||
       geState.APagP->Surf.height == -1)  
     {geGrf(GEBOUNDS, geSeg0);
      ClipSav = geClip;
      GEVEC(GEEXTENT, geSeg0);
      geState.APagP->Surf.width  = GETPIX(geClip.x2 - geClip.x1);
      geState.APagP->Surf.height = GETPIX(geClip.y2 - geClip.y1);
      geClipAnim                 = geClip;
      geClip                     = ClipSav;
     }
   else
     {geClipAnim.x1 = geClipAnim.y1 = 0;
      geClipAnim.x2 = GETSUI(geState.APagP->Surf.width);
      geClipAnim.y2 = GETSUI(geState.APagP->Surf.height);
     }
  }

/*
 * Clamp clipping region inside of window
 */
if (geClipAnim.x1 > GETSUI(geState.APagP->Surf.width) ||
    geClipAnim.y1 > GETSUI(geState.APagP->Surf.height) ||
    geClipAnim.x2 < 0 || geClipAnim.y2 < 0)
  return;

if (geClipAnim.x1 < 0) geClipAnim.x1 = 0;
if (geClipAnim.y1 < 0) geClipAnim.y1 = 0;
if (geClipAnim.x2 > GETSUI(geState.APagP->Surf.width))
  geClipAnim.x2 = GETSUI(geState.APagP->Surf.width);
if (geClipAnim.y2 > GETSUI(geState.APagP->Surf.height))
  geClipAnim.y2 = GETSUI(geState.APagP->Surf.height);

src_x = GETPIX(geClipAnim.x1);
src_y = GETPIX(geClipAnim.y1);
w     = GETPIX(geClipAnim.x2 - geClipAnim.x1);
h     = GETPIX(geClipAnim.y2 - geClipAnim.y1);
dst_x = GETPIX(geClipAnim.x1) + geState.APagP->ViewPortXoff;
dst_y = GETPIX(geClipAnim.y1) + geState.APagP->ViewPortYoff;

XCopyArea(geDispDev, geState.APagP->Pixmap, geState.APagP->Surf.self, geGC0,
	  src_x, src_y, w, h, dst_x, dst_y);

XSync(geDispDev, FALSE);
XFlush(geDispDev);

geState.NeedToPostPixmap = FALSE;

}

geTimer(Reset)
int Reset;
{                       
int                     deltat;
static int              initialized = FALSE;
static struct timeb     stimep, ctimep;

if (Reset || !initialized)
  {ftime(&stimep);
   initialized = TRUE;
   return(0);
 }

ftime(&ctimep);
deltat = (int)((ctimep.time - stimep.time) * 1000 +
	       ctimep.millitm - stimep.millitm);

return(deltat);
}

/*
 * This routine processes any animation factors associated with the object.
 */
geSleepDelay(ASegP, RemainingFrames)
struct GE_SEG  *ASegP;
int		RemainingFrames;
{
#define GE_LOC_START	TRUE
#define GE_LOC_ELAPSED	FALSE

int			Delay, RemainingDelay, DrawDelay, EraseDelay;
long			StateSav, x, y;
struct GE_SEG 		*ASegPSav;
struct GE_CO 		Clip, ClipSav;
struct GE_ANIM_FRAME	*AFrameP;

if (!geState.AnimPreview || !ASegP->AnimCtrl.Animated)
  return(0);

geTimer(GE_LOC_START);

RemainingDelay = 0;

ASegPSav       = geState.ASegP;
StateSav       = geState.State;
ClipSav        = geClip;

geState.ASegP  = ASegP;
geState.State  = GEANIMPREVIEW;

/*
 * If current frame number has not yet been set, set it now.
 */
if (ASegP->AnimCtrl.CurFrame == -1)
  geSetNextFrame(ASegP);			/* Calculate next frame num  */
/*
 * Get pointer to frame packet
 */
if (!(AFrameP = geGetFrameP(ASegP))) return(0);

 if (geSeg0->AnimCtrl.CZupdisp && ASegP != geSeg0) geGrf(GECZUPDISP, ASegP);

gePostIt();
if (!geState.APagP || !geState.APagP->Surf.self)
  {geState.ASegP = ASegPSav;
   geState.State = StateSav;
   return(0);					/* Window GONE - TERMINATE! */
  }

if (ASegP->AnimCtrl.FastPlay)
  {DrawDelay = EraseDelay = 0;}
else
  {DrawDelay  = AFrameP->DrawDelay;
   EraseDelay = AFrameP->EraseDelay;
  }
/*
 * Pause after display or at least check for an interrupt.
 */
Delay = abs(DrawDelay) - geTimer(GE_LOC_ELAPSED);
if (Delay < 0) Delay = 0;
geSleepInterrupt(ASegP, Delay);

if (geState.HaltRequested) return(0);

/*
 * If received an interrupt and it was to PAUSE the animation, then set the
 * RemainingFrames to 1 to indicate that the animation of this object should
 * terminate gracefully now; otherwise, if don't know why the interrupt
 * occurred, just recompute the reamining number of frames.
 */
if (ASegP->AnimCtrl.Interrupted)
  {if (ASegP->AnimCtrl.Paused)
     RemainingFrames = 1;
   else
     RemainingFrames = ASegP->AnimCtrl.NumFramesRemaining;
  }

/*
 * Erase it?
 */
if (AFrameP->Erase && geState.APagP && geState.APagP->Surf.self)
  {GEVEC(GEERASE, ASegP);       		/* ERASE everyone in region  */
   geGrf(GECZDOWNDISP, ASegP);			/* Redraw everyone below  obj*/
   /*
    * If there is supposed to be a delay following the erase, make sure
    * the pixmap with the erasure is posted.
    */
   if (EraseDelay)
     {geTimer(GE_LOC_START);
      Delay = abs(EraseDelay);
      gePostIt();
      if (!geState.APagP || !geState.APagP->Surf.self)
	{geState.ASegP = ASegPSav;
	 geState.State = StateSav;
	 geClip        = ClipSav;
	 return(0);				/* Window GONE - TERMINATE! */
        }
     }
  }						/* End of ERASE  case        */

/*
 * Move, Scale, or Rotate it?
 */
if (!ASegP->AnimCtrl.Interrupted) geAnimMoveIt(ASegP, AFrameP);
 
/*
 * If there's been no interrupt up to this point and there was an erase
 * check to see if there is a delay associated with it.
 */
if (!ASegP->AnimCtrl.Interrupted)
  {if (AFrameP->Erase && EraseDelay > 0)
     Delay -= geTimer(GE_LOC_ELAPSED);
   if (Delay < 0) Delay = 0;
   /*
    * Pause after erase or again check for an interrupt.
    */
   geSleepInterrupt(ASegP, Delay);

   if (geState.HaltRequested) return(0);

   /*
    * If received an interrupt and it was to PAUSE the animation, then set the
    * RemainingFrames to 1 to indicate that the animation of this object should
    * terminate gracefully now; otherwise, if don't know why the interrupt
    * occurred, just recompute the remaining number of frames.
    */
   if (ASegP->AnimCtrl.Interrupted)
     {if (ASegP->AnimCtrl.Paused)
	RemainingFrames = 1;
     else
       RemainingFrames = ASegP->AnimCtrl.NumFramesRemaining;
     }
  }

/*
 * If the object was erased and it is to be REDRAWN at the end of
 * animation and this is the last cycle then force a redisplay now.
 */
if (AFrameP->Erase && AFrameP->Redraw && RemainingFrames <= 1)
  {GEVEC(GEDISP,    ASegP);			/* DRAW obj                  */
   geGenClipMerge(&geClipAnim, &geClip);
   gePostIt();
   if (!geState.APagP || !geState.APagP->Surf.self)
     {geState.ASegP = ASegPSav;
      geState.State = StateSav;
      geClip        = ClipSav;
      return(0);		/* Window GONE - TERMINATE! */
     }
 }						/* End of forced REDISPLAY   */

geSetNextFrame(ASegP);				/* Calculate next frame num  */

geState.ASegP = ASegPSav;
geState.State = StateSav;
geClip        = ClipSav;

return(RemainingFrames);
}

/*
 * Given a pointer to the segment structure, this routine locates and
 * returns the pointer to the packet containing the frame control information.
 */
struct GE_ANIM_FRAME *
geGetFrameP(ASegP)
struct GE_SEG	*ASegP;
{

int			CurFrame, i, Count;
struct GE_ANIM_FRAME	*Packet, *AFrameP;

/*
 * Locate current frame packet
 */
Packet   = ASegP->AnimCtrl.Frames;
AFrameP  = Packet;
Count    = 0;
i	 = 0;

for (i = 0; i < ASegP->AnimCtrl.NumPackets; i++)
  {Count += Packet->Repeat;
   if (Count > ASegP->AnimCtrl.CurFrame)
     {AFrameP = Packet;
      break;
     }
   Packet++;
  }

return(AFrameP);

}

/*
 * Given a pointer to the segment structure, this routine calulates the
 * next frame number in the sequence, taking into consideration current
 * direction of travel and possible looping at ends.
 */
geSetNextFrame(ASegP)
struct GE_SEG	*ASegP;
{

int			NextFrame, EndFrame;

if (ASegP->AnimCtrl.CurFrame == -1)
  {/*
    * Needs to be initialized or reinitialized
    */
   if (ASegP->AnimCtrl.StartForward)
     ASegP->AnimCtrl.CurFrame = ASegP->AnimCtrl.FirstFrame;
   else
     ASegP->AnimCtrl.CurFrame = ASegP->AnimCtrl.LastFrame;

   ASegP->AnimCtrl.CurDirForward = ASegP->AnimCtrl.StartForward;
   ASegP->AnimCtrl.NumFramesRemaining =  ASegP->AnimCtrl.NumFrames;
   return;
  }

if (ASegP->AnimCtrl.CurDirForward)
  {/*
    * Running in FORWARD direction.
    */
   NextFrame = ASegP->AnimCtrl.CurFrame + 1;
   EndFrame  = ASegP->AnimCtrl.LastFrame;
   if (NextFrame > EndFrame)
     {if (ASegP->AnimCtrl.ReverseAtEnd)
	{NextFrame = EndFrame;
	 ASegP->AnimCtrl.CurDirForward = FALSE;
        }
      else
	NextFrame = ASegP->AnimCtrl.FirstFrame;
     }
  }
else
  {/*
    * Running in BACKWARD direction.
    */
   NextFrame = ASegP->AnimCtrl.CurFrame - 1;
   EndFrame  = ASegP->AnimCtrl.FirstFrame;
   if (NextFrame < EndFrame)
     {if (ASegP->AnimCtrl.ReverseAtEnd)
	{NextFrame = EndFrame;
	 ASegP->AnimCtrl.CurDirForward = TRUE;
        }
      else
	NextFrame = ASegP->AnimCtrl.LastFrame;
     }
  }

ASegP->AnimCtrl.CurFrame = NextFrame;
if (!(--ASegP->AnimCtrl.NumFramesRemaining)) ASegP->AnimCtrl.Animated = FALSE;
}

/*
 * Reposition (maybe) the object to its location at the start of the animation
 * sequence and reset the current animation frame as unknown.
 */
geAnimReset(ASegP)
struct GE_SEG	*ASegP;
{
/*
 * Rotate, scale and\or move it back?
 */
if (ASegP->AnimCtrl.SumDx || ASegP->AnimCtrl.SumDy || ASegP->AnimCtrl.SumRot ||
    (ASegP->AnimCtrl.SumMagFX && ASegP->AnimCtrl.SumMagFX != 100.0) ||
    (ASegP->AnimCtrl.SumMagFY && ASegP->AnimCtrl.SumMagFY != 100.0))
  {geEditBox.Pt.x  = geMagCx = (ASegP->Co.x1 + ASegP->Co.x2) >> 1;
   geEditBox.Pt.y  = geMagCy = (ASegP->Co.y1 + ASegP->Co.y2) >> 1;
   /*
    * Restore to original size?
    */
   if ((ASegP->AnimCtrl.SumMagFX && ASegP->AnimCtrl.SumMagFX != 100.0) ||
       (ASegP->AnimCtrl.SumMagFY && ASegP->AnimCtrl.SumMagFY != 100.0))
     {geMagGrp   = TRUE;
      geMagFX = (100.0 / ASegP->AnimCtrl.SumMagFX) * 100.;
      geMagFY = (100.0 / ASegP->AnimCtrl.SumMagFY) * 100.;
      geMagMode = GENOCONSTRAINT;
      GEVEC(GEMAG, ASegP);
     }						/* End of scaling	     */
   /*
    * Rotate it back?
    */
   if (ASegP->AnimCtrl.SumRot)
     {if (ASegP->AnimCtrl.SumRot > 0) geRot.Clockwise = TRUE;
      else                            geRot.Clockwise = FALSE;

      geRot.Alpha = geRot.Beta = fabs(ASegP->AnimCtrl.SumRot);
      GEVEC(GEROTFIXED, ASegP);
     }						/* End of ROTATE	     */
   /*
    * Reposition it?
    */
   if (ASegP->AnimCtrl.SumDx || ASegP->AnimCtrl.SumDy)
     {geState.Dx = -ASegP->AnimCtrl.SumDx;
      geState.Dy = -ASegP->AnimCtrl.SumDy;
      
      GEVEC(GEMOVE, ASegP);
     }						/* End of MOVE               */

   GEVEC(GEBOUNDS, ASegP);
   ASegP->AnimCtrl.SumDx    = 0;
   ASegP->AnimCtrl.SumDy    = 0;
   ASegP->AnimCtrl.SumMagFX = 0.;
   ASegP->AnimCtrl.SumMagFY = 0.;
   ASegP->AnimCtrl.SumRot   = 0;

 }						/* End of ALL GEOMETRIC reset*/

/*
 * Reset all the animation parameters which may have been changed during
 * the animation sequence.
 */
ASegP->AnimCtrl.CurFrame           = -1;
ASegP->AnimCtrl.NumFramesRemaining = ASegP->AnimCtrl.NumFrames;
ASegP->AnimCtrl.Animated           = ASegP->AnimCtrl.Animate;
ASegP->AnimCtrl.Blocked            = ASegP->AnimCtrl.Block;
ASegP->AnimCtrl.CurDirForward      = ASegP->AnimCtrl.StartForward;
ASegP->AnimCtrl.Interrupted        = FALSE;
ASegP->AnimCtrl.FastPlay           = FALSE;
ASegP->AnimCtrl.Paused             = FALSE;
geState.HaltRequested 		   = FALSE;
geState.HaltCmd       		   = 0;
geState.State       		   = 0;

}

/*
 * Look for an interrupt - Button1Down - in the drawing window; all other
 * events in the drawing window are discarded.  All mouse motion events
 * anywhere are discarded.  All other events NOT in the drawing window are
 * dispatched to the toolkit.  If the segment responds to the interrupt,
 * it's AnimCtrl->Interrupt flag will be SET and geSeg0->AnimCtrl.Interrupt
 * will also be set.
 */
geInterrupt(ASegP, Event)
struct GE_SEG *ASegP;
XButtonEvent  *Event;
{
Window			DrawWin;
XWindowAttributes 	watts;

if (!geState.APagP || !geState.APagP->Surf.self) return(0);

DrawWin = geState.APagP->Surf.self;

ASegP->AnimCtrl.Interrupted  = FALSE;

if (ASegP != geSeg0 && geSeg0)
  geSeg0->AnimCtrl.Interrupted = FALSE;

if (!Event)
  {/*
    * No event was passed in.  Look ahead in the que for any interesting
    * events.  If find one, process it, otherwise return.
    */
   if(XCheckMaskEvent(geDispDev,
		      GE_MBd|GE_MBu|GE_MWe|GE_MWx|GE_MKd|GE_MKu|GE_MWd|GE_MWu,
		      &geState.Event))
     {if (geState.Event.xany.window == geState.APagP->Surf.self)
	{if (geState.Event.type == GE_Bd)
	   {if (geState.Event.xbutton.button == GEBUT_L)
	      {GEVEC(GEINTERRUPT, ASegP);}
	    else if (geState.Event.xbutton.button == GEBUT_R)
	      {geSeg0->AnimCtrl.Animated = ASegP->AnimCtrl.Animated = FALSE;
	       geState.HaltRequested = TRUE;
	       geState.HaltCmd       = 0;
	      }
	   }
	 else if (geState.Event.type == GE_Wd)
	   {geState.APagP->Surf.self = NULL;
	    geState.HaltRequested    = TRUE;
 	   }
        }
      else if (geState.Event.type == GE_Wu || geRun.VoyerCalling)
	{XtDispatchEvent(&geState.Event);
	 XSync(geDispDev, FALSE);
	 XFlush(geDispDev);

	 if (geState.Event.type == GE_Bd)
	   {/*
	     * Graphics window may have been deleted - check to see if it's
	     * still there.
	     */
	    geErr = 0;
	    XGetWindowAttributes(geDispDev, geState.APagP->Surf.self, &watts);
	    XSync(geDispDev, FALSE);/* Make sure if ERR, it gets reprtd*/

	    if (geErr || watts.map_state == IsUnviewable ||
		watts.map_state == IsUnmapped)
		{/*
		  * Window no longer exists - set halt requested flag
    		  */
		 geState.APagP->Surf.self = NULL;
  		 geState.HaltRequested    = TRUE;
		}
	   }
	}
     }
  }
else if (Event)
  {if (Event->window == geState.APagP->Surf.self)
     {geState.Event.xbutton = *Event;
      GEVEC(GEINTERRUPT, ASegP);
     }
  }

if (!geState.APagP || !geState.APagP->Surf.self)
  {/*
    * Graphics window has been yanked from underneath us
    */
    if (geState.Drawable == DrawWin)
      geState.Drawable = NULL;
    if (geState.APagP && geState.APagP->Surf.self == DrawWin)
      geState.APagP->Surf.self = NULL;
  }

return(ASegP->AnimCtrl.Interrupted);
}

/*
 * Determine what should happen, given that the segment in question has
 * received an interrupt.
 */
geRespondToInterrupt(ASegP)
struct GE_SEG *ASegP;
{
long		     x, y;

struct GE_ANIM_FRAME *AFrameP;

x = GETSUI(geState.Event.xbutton.x - geState.APagP->ViewPortXoff);
y = GETSUI(geState.Event.xbutton.y - geState.APagP->ViewPortYoff);
if (x >= ASegP->Co.x1 && x <= ASegP->Co.x2 &&
    y >= ASegP->Co.y1 && y <= ASegP->Co.y2)
  {switch (ASegP->AnimCtrl.InterruptResponse)
     {case GEANIMCTRL_INTERRUPT_IGNORE:	       	/* Ignore it		     */
      default:
	return;
	break;
	
      case GEANIMCTRL_INTERRUPT_GOTOEND:
	while (ASegP->AnimCtrl.NumFramesRemaining > 1)
	  {AFrameP = geGetFrameP(ASegP);
	   geAnimMoveIt(ASegP, AFrameP);
	   geSetNextFrame(ASegP);
	  }
	break;

      case GEANIMCTRL_INTERRUPT_STOP:	   	/* Stop gracefully           */
	ASegP->AnimCtrl.Animated = FALSE;
	break;

      case GEANIMCTRL_INTERRUPT_FAST:		/* Flip fast play bit        */
	ASegP->AnimCtrl.FastPlay =
	  ASegP->AnimCtrl.FastPlay ? FALSE : TRUE;
	break;

      case GEANIMCTRL_INTERRUPT_REVERSE:	/* Reverse direction	     */
	ASegP->AnimCtrl.CurDirForward =
	  ASegP->AnimCtrl.CurDirForward ? FALSE : TRUE;
	break;

      case GEANIMCTRL_INTERRUPT_RESTART:	/* Restart the animation     */
	geAnimReset(ASegP);
	break;

      case GEANIMCTRL_INTERRUPT_SPEAK:		/* Try to speak the descrip  */
	if (ASegP->Descrip && strlen(ASegP->Descrip))
	    geDECtalk(ASegP->Descrip, strlen(ASegP->Descrip));
	break;

      case GEANIMCTRL_INTERRUPT_PAUSE:		/* Stop or start	     */
	if (ASegP->AnimCtrl.Paused)
	  {ASegP->AnimCtrl.Animated = TRUE;
	   ASegP->AnimCtrl.Paused   = FALSE;
	  }
	else                     
	  {ASegP->AnimCtrl.Animated = FALSE;
	   ASegP->AnimCtrl.Paused   = TRUE;
	  }
	break;

      case GEANIMCTRL_INTERRUPT_SYSCOM:		/* Issue system command      */
	if (ASegP->Descrip && strlen(ASegP->Descrip))
	  system(ASegP->Descrip);
	break;

      case GEANIMCTRL_INTERRUPT_FLPBLOCK:	/* Flip the block flag       */
	if (ASegP->AnimCtrl.Block)
	  ASegP->AnimCtrl.Blocked      =
	    ASegP->AnimCtrl.Blocked ? FALSE : TRUE;
	else
	  return;
	break;

     }
   ASegP->AnimCtrl.Interrupted  = TRUE;
   if (geSeg0) geSeg0->AnimCtrl.Interrupted = TRUE;
  }
}

geAnimMoveIt(ASegP, AFrameP)
struct GE_SEG 	     *ASegP;
struct GE_ANIM_FRAME *AFrameP;
{
/*
 * Move, Scale, or Rotate it?
 */
if (!ASegP || !AFrameP) return;

GEVEC(GEEXTENT, ASegP);
geClipAnim = geClip;

if (AFrameP->Dx || AFrameP->Dy || AFrameP->Rot ||
    (AFrameP->MagFX && AFrameP->MagFX != 100.0) ||
    (AFrameP->MagFY && AFrameP->MagFY != 100.0))
  {geEditBox.Pt.x  = geMagCx = (ASegP->Co.x1 + ASegP->Co.x2) >> 1;
   geEditBox.Pt.y  = geMagCy = (ASegP->Co.y1 + ASegP->Co.y2) >> 1;
   /*
    * Scale it?
    */
   if ((AFrameP->MagFX && AFrameP->MagFX != 100.0) ||
       (AFrameP->MagFY && AFrameP->MagFY != 100.0))
     {geMagGrp   = TRUE;
      geMagFX = AFrameP->MagFX;
      geMagFY = AFrameP->MagFY;
      geMagMode = GENOCONSTRAINT;

      if (ASegP->AnimCtrl.SumMagFX)
	ASegP->AnimCtrl.SumMagFX *= AFrameP->MagFX / 100.0;
      else
	ASegP->AnimCtrl.SumMagFX = AFrameP->MagFX;

      if (ASegP->AnimCtrl.SumMagFY)
	ASegP->AnimCtrl.SumMagFY *= AFrameP->MagFY / 100.0;
      else
	ASegP->AnimCtrl.SumMagFY = AFrameP->MagFY;

      GEVEC(GEMAG, ASegP);
     }						 /* End of scaling	     */
   /*
    * Rotate it?
    */
   if (AFrameP->Rot)
     {if (ASegP->AnimCtrl.CurDirForward)
	{if (AFrameP->Rot > 0) geRot.Clockwise = FALSE;
	 else                  geRot.Clockwise = TRUE;
        }
      else
	{if (AFrameP->Rot > 0) geRot.Clockwise = TRUE;
	 else                  geRot.Clockwise = FALSE;
        }
      geRot.Alpha = geRot.Beta = fabs(AFrameP->Rot);
      ASegP->AnimCtrl.SumRot += AFrameP->Rot;
      /*
       * Normalize the cumulative rotation to between 0-360
       */
      if (ASegP->AnimCtrl.SumRot > 360.)
	{do
	   { ASegP->AnimCtrl.SumRot -= 360.;
	   } while (ASegP->AnimCtrl.SumRot > 360.);
	}
      else if (ASegP->AnimCtrl.SumRot < -360.)
	{do
	   { ASegP->AnimCtrl.SumRot += 360.;
	   } while (ASegP->AnimCtrl.SumRot < -360.);
	}

      GEVEC(GEROTFIXED, ASegP);
     }						 /* End of rotation          */
   /*
    * Move it?
    */
   if (AFrameP->Dx || AFrameP->Dy)
     {if (ASegP->AnimCtrl.CurDirForward)
	{geState.Dx = AFrameP->Dx;
	 geState.Dy = AFrameP->Dy;
        }
      else
       {geState.Dx = -AFrameP->Dx;
	geState.Dy = -AFrameP->Dy;
       }
      ASegP->AnimCtrl.SumDx += geState.Dx;
      ASegP->AnimCtrl.SumDy += geState.Dy;
      GEVEC(GEMOVE, ASegP);
     }						 /* End of motion in frame   */

   GEVEC(GEBOUNDS, ASegP);
   GEVEC(GEEXTENT, ASegP);
   if (AFrameP->Erase) geGenClipMerge(&geClipAnim, &geClip);
   else		       geClipAnim = geClip;
  }						/* End of geometry change    */
 
}
