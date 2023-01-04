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
/* 
 * (c) Copyright 1989, 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.2
*/ 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$RCSfile: UniqueEvnt.c,v $ $Revision: 1.1.6.4 $ $Date: 1993/08/03 22:02:05 $"
#endif
#endif
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#include <Xm/XmP.h>
#include <limits.h>

#define XmCHECK_UNIQUENESS 1
#define XmRECORD_EVENT     2


/********    Static Function Declarations    ********/
#ifdef _NO_PROTO

static Time ExtractTime() ;
static Boolean ManipulateEvent() ;

#else

static Time ExtractTime( 
                        XEvent *event) ;
static Boolean ManipulateEvent( 
                        XEvent *event,
                        int action) ;

#endif /* _NO_PROTO */
/********    End Static Function Declarations    ********/


/************************************************************************
 *
 *  _XwExtractTime
 *     Extract the time field from the event structure.
 *
 ************************************************************************/
static Time 
#ifdef _NO_PROTO
ExtractTime( event )
        XEvent *event ;
#else
ExtractTime(
        XEvent *event )
#endif /* _NO_PROTO */
{
   if ((event->type == ButtonPress) || (event->type == ButtonRelease))
      return (event->xbutton.time);

   if ((event->type == KeyPress) || (event->type == KeyRelease))
      return (event->xkey.time);

   return ((Time) 0);
}

static Boolean
#ifdef _NO_PROTO
Later(recorded, new_l)
     unsigned long recorded, new_l;
#else
Later(unsigned long recorded,
      unsigned long new_l)
#endif /* _NO_PROTO */
{
  long normalizedNew;

  /* The pathogenic cases for this calculation involve numbers
     very close to 0 or ULONG_MAX.  

     So the way we do it is by normalizing to 0 (signed).  That
     way the differences are +/- in the appropriate way.
     
     These numbers are defined as a unsigned long.  Please 
     remember that when changing this code.
     */

  normalizedNew = new_l - recorded;

  return (normalizedNew > 0);
}

static Boolean 
#ifdef _NO_PROTO
ManipulateEvent( event, action )
        XEvent *event ;
        int action ;
#else
ManipulateEvent(
        XEvent *event,
        int action )
#endif /* _NO_PROTO */
{
   static unsigned long serial = 0;
   static Time time = 0;
   static int type = 0;
#ifdef DEC_MOTIF_BUG_FIX
   static Display *display = NULL;
#endif

   switch (action)
   {
      case XmCHECK_UNIQUENESS:
      {
	/*
	 * Ignore duplicate events, caused by an event being dispatched
	 * to both the focus widget and the spring-loaded widget, where
	 * these map to the same widget (menus).
	 * Also, ignore an event which has already been processed by
	 * another menu component.
	 * 
	 * Changed D.Rand 6/26/92 Discussion:
	 *
	 * This used to be done by making an exact comparison with
	 * a recorded event.  But there are many times when we can
	 * get an event,  but not the original event.  This cuts
	 * down on some distributed processing in the menu
	 * system.  
	 *
	 * So now we compare the serial number of the 
	 * examined event against the recorded event.  This needs
	 * to be done carefully to include the case where the
	 * serial number wraps
	 *
	 * 7/23/92 added discussion:
	 *
	 * The other case we must be careful of is when the serial
	 * number are the same.  This can only realistically occur
	 * if the user is very fast and therefore clicks while no
	 * protocol request occurs.  XSentEvent would cause an
	 * increment,  so we needn't worry over synthetic events
	 * causing problems.  
	 *
	 * So if the serial numbers match,  we use the timestamps
	 *
	 */

#ifdef DEC_MOTIF_BUG_FIX
	/*
	** The serial field of the event is different for each display connection.
	** Therefore should not compare the serial of the event with the last event
	** if the displays are different.  It may be better to keep the serial, time
	** and type information for each display.  But this fix is simple and it seems
	** to work.  If the displays are not the same then consider the event unique.
	**
	** When this is fixed by OSF, there is a DEC_MOTIF_BUG_FIX in
	** /Xm/MenuShell.c PostMenuShell that sets the event.xbutton.display field before
	** calling _XmRecordEvent.  That fix can probably be removed at the same time
	** as this one.
	** CS, 3-Aug-1993
	*/
	if (display && (display != event->xany.display))
	  return (TRUE);
	else
#endif
	if (Later(serial, event->xany.serial) 
	    || (serial == event->xany.serial &&
		Later(time, event->xbutton.time)))
	  return (TRUE);
	else
	  return (FALSE);
      }

      case XmRECORD_EVENT:
      {
         /* Save the fingerprints for the new event */
         type = event->type;
         serial = event->xany.serial;
         time = ExtractTime(event);
#ifdef DEC_MOTIF_BUG_FIX
	 display = event->xany.display;
#endif

         return (TRUE);
      }

      default:
      {
         return (FALSE);
      }
   }
}


/*
 * Check to see if this event has already been processed.
 */
Boolean 
#ifdef _NO_PROTO
_XmIsEventUnique( event )
        XEvent *event ;
#else
_XmIsEventUnique(
        XEvent *event )
#endif /* _NO_PROTO */
{
   return (ManipulateEvent (event, XmCHECK_UNIQUENESS));
}



/*
 * Record the specified event, so that it will not be reprocessed.
 */
void 
#ifdef _NO_PROTO
_XmRecordEvent( event )
        XEvent *event ;
#else
_XmRecordEvent(
        XEvent *event )
#endif /* _NO_PROTO */
{
   ManipulateEvent (event, XmRECORD_EVENT);
}
