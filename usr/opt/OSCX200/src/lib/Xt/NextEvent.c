
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
/* $XConsortium: NextEvent.c,v 1.110 93/02/10 15:47:39 converse Exp $ */

/***********************************************************
Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

#ifdef VMS
#define NOFILE	32
#endif /* VMS */

#include "IntrinsicI.h"
#include <stdio.h>
#include <errno.h>

#ifndef VMS
extern int errno;
#endif /* VMS */

#ifdef DEC_BUG_FIX
static TimerEventRec* freeTimerRecs = NULL;
static WorkProcRec* freeWorkRecs = NULL;
#else
static TimerEventRec* freeTimerRecs;
static WorkProcRec* freeWorkRecs;
#endif

#ifdef VMS
#include <iodef.h>
#include <ssdef.h>
#include <libdef.h>
#define  TIMER_EVENT_FLAG        13
#define  TIMER_EVENT_MASK	1 << TIMER_EVENT_FLAG  
#endif /* VMS */

/* Some systems running NTP daemons are known to return strange usec
 * values from gettimeofday.  At present (3/90) this has only been
 * reported on SunOS...
 */

#ifndef NEEDS_NTPD_FIXUP
#ifdef DEC_BUG_FIX
# if defined(sun) || defined(MOTOROLA) || (defined(OSF1) && defined(__alpha))
#  define NEEDS_NTPD_FIXUP 1
# else
#  define NEEDS_NTPD_FIXUP 0
# endif
#else
# if defined(sun) || defined(MOTOROLA)
#  define NEEDS_NTPD_FIXUP 1
# else
#  define NEEDS_NTPD_FIXUP 0
# endif
#endif /* DEC_BUG_FIX */
#endif

#if NEEDS_NTPD_FIXUP
#define FIXUP_TIMEVAL(t) { \
	while ((t).tv_usec >= 1000000) { \
	    (t).tv_usec -= 1000000; \
	    (t).tv_sec++; \
	} \
	while ((t).tv_usec < 0) { \
	    if ((t).tv_sec > 0) { \
		(t).tv_usec += 1000000; \
		(t).tv_sec--; \
	    } else { \
		(t).tv_usec = 0; \
		break; \
	    } \
	}}
#else
#define FIXUP_TIMEVAL(t)
#endif /*NEEDS_NTPD_FIXUP*/



/*
 * Private routines
 */
#ifdef VMS
#define ADD_TIME(dest, src1, src2) { \
	lib$addx (&src1.low, &src2.low, &dest.low ); }

#define IS_AFTER(t1,t2) (((t2).high > (t1).high) \
       ||(((t2).high == (t1).high)&& ((t2).low > (t1).low)))

#define IS_AT_OR_AFTER(t1,t2) (((t2).high > (t1).high) \
       ||(((t2).high == (t1).high)&& ((t2).low >= (t1).low)))

#define GET_TIME(t) { sys$gettim(&t); }

#else
#define ADD_TIME(dest, src1, src2) { \
	if(((dest).tv_usec = (src1).tv_usec + (src2).tv_usec) >= 1000000) {\
	      (dest).tv_usec -= 1000000;\
	      (dest).tv_sec = (src1).tv_sec + (src2).tv_sec + 1 ; \
	} else { (dest).tv_sec = (src1).tv_sec + (src2).tv_sec ; \
	   if(((dest).tv_sec >= 1) && (((dest).tv_usec <0))) { \
	    (dest).tv_sec --;(dest).tv_usec += 1000000; } } }


#define TIMEDELTA(dest, src1, src2) { \
	if(((dest).tv_usec = (src1).tv_usec - (src2).tv_usec) < 0) {\
	      (dest).tv_usec += 1000000;\
	      (dest).tv_sec = (src1).tv_sec - (src2).tv_sec - 1;\
	} else 	(dest).tv_sec = (src1).tv_sec - (src2).tv_sec;  }

#define IS_AFTER(t1, t2) (((t2).tv_sec > (t1).tv_sec) \
	|| (((t2).tv_sec == (t1).tv_sec)&& ((t2).tv_usec > (t1).tv_usec)))

#define IS_AT_OR_AFTER(t1, t2) (((t2).tv_sec > (t1).tv_sec) \
	|| (((t2).tv_sec == (t1).tv_sec)&& ((t2).tv_usec >= (t1).tv_usec)))
#endif /* VMS */

static void QueueTimerEvent(app, ptr)
    XtAppContext app;
    TimerEventRec *ptr;
{
        TimerEventRec *t,**tt;
        tt = &app->timerQueue;
        t  = *tt;
        while (t != NULL &&
                IS_AFTER(t->te_timer_value, ptr->te_timer_value)) {
          tt = &t->te_next;
          t  = *tt;
         }
         ptr->te_next = t;
         *tt = ptr;
}

#ifndef VMS
/* 
 * Routine to block in the toolkit.  This should be the only call to select.
 *
 * This routine returns when there is something to be done.
 *
 * Before calling this with ignoreInputs==False, app->outstandingQueue should
 * be checked; this routine will not verify that an alternate input source
 * has not already been enqueued.
 *
 *
 * _XtwaitForSomething( ignoreTimers, ignoreInputs, ignoreEvents,
 *			block, howlong, appContext)
 * Boolean ignoreTimers;     (Don't return if a timer would fire
 *				Also implies forget timers exist)
 *
 * Boolean ignoreInputs;     (Ditto for input callbacks )
 *
 * Boolean ignoreEvents;     (Ditto for X events)
 *
 * Boolean block;	     (Okay to block)
 * TimeVal howlong;	     (howlong to wait for if blocking and not
 *				doing Timers... Null mean forever.
 *				Maybe should mean shortest of both)
 * XtAppContext app;	     (Displays to check wait on)
 * Returns display for which input is available, if any
 * and if ignoreEvents==False, else returns -1
 *
 * if ignoring everything && block=True && howlong=NULL, you'll have
 * lots of time for coffee; better not try it!  In fact, it probably
 * makes little sense to do this regardless of the value of howlong
 * (bottom line is, we don't bother checking here).
 */
#if NeedFunctionPrototypes
int _XtwaitForSomething(
	_XtBoolean ignoreTimers,
	_XtBoolean ignoreInputs,
	_XtBoolean ignoreEvents,
	_XtBoolean block,
	unsigned long *howlong,
	XtAppContext app
        )
#else
int _XtwaitForSomething(ignoreTimers, ignoreInputs, ignoreEvents,
			block, howlong, app)
	Boolean ignoreTimers;
	Boolean ignoreInputs;
	Boolean ignoreEvents;
	Boolean block;
	unsigned long *howlong;
	XtAppContext app;
#endif
{
	struct timeval  cur_time;
	struct timeval  start_time;
	struct timeval  wait_time;
	struct timeval  new_time;
	struct timeval  time_spent;
	struct timeval	max_wait_time;
	static struct timeval  zero_time = { 0 , 0};
	register struct timeval *wait_time_ptr;
	Fd_set rmaskfd, wmaskfd, emaskfd;
	static Fd_set zero_fd = { 0 };
	int nfound, i, d;
	
 	if (block) {
		(void) gettimeofday (&cur_time, NULL);
		FIXUP_TIMEVAL(cur_time);
		start_time = cur_time;
		if(howlong == NULL) { /* special case for ever */
			wait_time_ptr = 0;
		} else { /* block until at most */
			max_wait_time.tv_sec = *howlong/1000;
			max_wait_time.tv_usec = (*howlong %1000)*1000;
			wait_time_ptr = &max_wait_time;
		}
	} else {  /* don't block */
		max_wait_time = zero_time;
		wait_time_ptr = &max_wait_time;
	}

      WaitLoop:
	while (1) {
		if (app->timerQueue != NULL && !ignoreTimers && block) {
		    if(IS_AFTER(cur_time, app->timerQueue->te_timer_value)) {
			TIMEDELTA (wait_time, app->timerQueue->te_timer_value, 
				   cur_time);
			if(howlong==NULL || IS_AFTER(wait_time,max_wait_time)){
				wait_time_ptr = &wait_time;
			} else {
				wait_time_ptr = &max_wait_time;
			}
		    } else wait_time_ptr = &zero_time;
		} 
		if( !ignoreInputs ) {
			rmaskfd = app->fds.rmask;
			wmaskfd = app->fds.wmask;
			emaskfd = app->fds.emask;
		} else {
			rmaskfd = zero_fd;
			wmaskfd = zero_fd;
			emaskfd = zero_fd;
		}
		if (!ignoreEvents) {
		    for (d = 0; d < app->count; d++) {
			FD_SET (ConnectionNumber(app->list[d]), &rmaskfd);
		    }
		}
#if defined(OSF1) && defined(__alpha)
		/* Need to evaluate this some more, CS */
		if (wait_time_ptr != (struct timeval *)NULL) 
		  FIXUP_TIMEVAL(*wait_time_ptr) ;
#endif
#ifdef DEC_BUG_FIX
		nfound = select (app->fds.nfds, (fd_set *) &rmaskfd,
			(fd_set *) &wmaskfd, (fd_set *) &emaskfd, wait_time_ptr);
#else
		nfound = select (app->fds.nfds, (int *) &rmaskfd,
			(int *) &wmaskfd, (int *) &emaskfd, wait_time_ptr);
#endif
		if (nfound == -1) {
			/*
			 *  interrupt occured recalculate time value and select
			 *  again.
			 */
			if (errno == EINTR) {
			    errno = 0;  /* errno is not self reseting */
			    if (block) {
				if (wait_time_ptr == NULL) /*howlong == NULL*/
				    continue;
				(void)gettimeofday (&new_time, NULL);
				FIXUP_TIMEVAL(new_time);
				TIMEDELTA(time_spent, new_time, cur_time);
				cur_time = new_time;
				if(IS_AFTER(time_spent, *wait_time_ptr)) {
					TIMEDELTA(wait_time, *wait_time_ptr,
						  time_spent);
					wait_time_ptr = &wait_time;
					continue;
				} else {
					/* time is up anyway */
					nfound = 0;
				}
			    }
			} else {
			    char Errno[12];
			    String param = Errno;
			    Cardinal param_count = 1;

			    if (!ignoreEvents) {
				/* get Xlib to detect a bad connection */
				for (d = 0; d < app->count; d++) {
				    if (XEventsQueued(app->list[d],
						      QueuedAfterReading))
					return d;
				}
			    }
			    sprintf( Errno, "%d", errno);
			    XtAppWarningMsg(app, "communicationError","select",
			       XtCXtToolkitError,"Select failed; error code %s",
			       &param, &param_count);
			    continue;
			}
		} /* timed out or input available */
		break;
	}
	
	if (nfound == 0) {
		if(howlong) *howlong = (unsigned long)0;  /* Timed out */
		return -1;
	}
	if(block && howlong != NULL) { /* adjust howlong */
	    (void) gettimeofday (&new_time, NULL);
	    FIXUP_TIMEVAL(new_time);
	    TIMEDELTA(time_spent, new_time, start_time);
	    if(*howlong <= (time_spent.tv_sec*1000+time_spent.tv_usec/1000))
		*howlong = (unsigned long)0;  /* Timed out */
	    else
		*howlong -= (time_spent.tv_sec*1000+time_spent.tv_usec/1000);
	}
	if(ignoreInputs) {
	    if (ignoreEvents) return -1; /* then only doing timers */
	    for (d = 0; d < app->count; d++) {
		if (FD_ISSET(ConnectionNumber(app->list[d]), &rmaskfd)) {
		    if (XEventsQueued( app->list[d], QueuedAfterReading ))
			return d;
		    /*
		     * An error event could have arrived
		     * without any real events, or events
		     * could have been swallowed by Xlib,
		     * or the connection may be broken.
		     * We can't tell the difference, so
		     * ssume Xlib will eventually discover
		     * a broken connection.
		     */
		}
	    }
	    goto WaitLoop;	/* must have been only error events */
        }
	{
	int ret = -1;
	Boolean found_input = False;

	for (i = 0; i < app->fds.nfds && nfound > 0; i++) {
	    XtInputMask condition = 0;
	    if (FD_ISSET (i, &rmaskfd)) {
		nfound--;
		if (!ignoreEvents) {
		    for (d = 0; d < app->count; d++) {
			if (i == ConnectionNumber(app->list[d])) {
			    if (ret == -1) {
				if (XEventsQueued( app->list[d],
						   QueuedAfterReading ))
				    ret = d;
				/*
				 * An error event could have arrived
				 * without any real events, or events
				 * could have been swallowed by Xlib,
				 * or the connection may be broken.
				 * We can't tell the difference, so
				 * assume Xlib will eventually discover
				 * a broken connection.
				 */
			    }
			    goto ENDILOOP;
			}
		    }
		}
		condition = XtInputReadMask;
	    }
	    if (FD_ISSET (i, &wmaskfd)) {
		condition |= XtInputWriteMask;
		nfound--;
	    }
	    if (FD_ISSET (i, &emaskfd)) {
		condition |= XtInputExceptMask;
		nfound--;
	    }
	    if (condition) {
		InputEvent *ep;
		for (ep = app->input_list[i]; ep; ep = ep->ie_next) {
		    if (condition & ep->ie_condition) {
			ep->ie_oq = app->outstandingQueue;
			app->outstandingQueue = ep;
		    }
		}
		found_input = True;
	    }
ENDILOOP:   ;
	} /* endfor */
	if (ret >= 0 || found_input)
	    return ret;
	goto WaitLoop;		/* must have been only error events */
	}
}
#endif /* VMS */

#define IeCallProc(ptr) \
    (*ptr->ie_proc) (ptr->ie_closure, &ptr->ie_source, (XtInputId*)&ptr);

#define TeCallProc(ptr) \
    (*ptr->te_proc) (ptr->te_closure, (XtIntervalId*)&ptr);

/*
 * Public Routines
 */

XtIntervalId XtAddTimeOut(interval, proc, closure)
	unsigned long interval;
	XtTimerCallbackProc proc;
	XtPointer closure;
{
	return XtAppAddTimeOut(_XtDefaultAppContext(), 
		interval, proc, closure); 
}


XtIntervalId XtAppAddTimeOut(app, interval, proc, closure)
	XtAppContext app;
	unsigned long interval;
	XtTimerCallbackProc proc;
	XtPointer closure;
{
	TimerEventRec *tptr;
#ifdef VMS
        vms_time current_time;
#else
        struct timeval current_time;
#endif

	if (freeTimerRecs) {
	    tptr = freeTimerRecs;
	    freeTimerRecs = tptr->te_next;
	}
	else tptr = XtNew(TimerEventRec);

	tptr->te_next = NULL;
	tptr->te_closure = closure;
	tptr->te_proc = proc;
	tptr->app = app;
#ifdef VMS
	lib$emul( &(10000), &interval, &(0), &tptr->te_timer_value.low);
	GET_TIME(current_time);
#else
	tptr->te_timer_value.tv_sec = interval/1000;
	tptr->te_timer_value.tv_usec = (interval%1000)*1000;
        (void) gettimeofday(&current_time, NULL);
#endif /* VMS */
	FIXUP_TIMEVAL(current_time);
        ADD_TIME(tptr->te_timer_value,tptr->te_timer_value,current_time);
	QueueTimerEvent(app, tptr);
	return( (XtIntervalId) tptr);
}

void  XtRemoveTimeOut(id)
    XtIntervalId id;
{
   TimerEventRec *t, *last, *tid = (TimerEventRec *) id;

#ifdef DEC_EXTENSION
   if ( tid == NULL ) return;		/* sanity */
#endif

   /* find it */

   for(t = tid->app->timerQueue, last = NULL;
	   t != NULL && t != tid;
	   t = t->te_next) last = t;

   if (t == NULL) return; /* couldn't find it */
   if(last == NULL) { /* first one on the list */
       t->app->timerQueue = t->te_next;
   } else last->te_next = t->te_next;

   t->te_next = freeTimerRecs;
   freeTimerRecs = t;
   return;
}

XtWorkProcId XtAddWorkProc(proc, closure)
	XtWorkProc proc;
	XtPointer closure;
{
	return XtAppAddWorkProc(_XtDefaultAppContext(), proc, closure);
}

XtWorkProcId XtAppAddWorkProc(app, proc, closure)
	XtAppContext app;
	XtWorkProc proc;
	XtPointer closure;
{
	WorkProcRec *wptr;

	if (freeWorkRecs) {
	    wptr = freeWorkRecs;
	    freeWorkRecs = wptr->next;
	} else wptr = XtNew(WorkProcRec);

	wptr->next = app->workQueue;
	wptr->closure = closure;
	wptr->proc = proc;
	wptr->app = app;
	app->workQueue = wptr;

	return (XtWorkProcId) wptr;
}

void  XtRemoveWorkProc(id)
	XtWorkProcId id;
{
	WorkProcRec *wid= (WorkProcRec *) id, *w, *last;

	/* find it */
	for(w = wid->app->workQueue, last = NULL; w != NULL && w != wid; w = w->next) last = w;

	if (w == NULL) return; /* couldn't find it */

	if(last == NULL) wid->app->workQueue = w->next;
	else last->next = w->next;

	w->next = freeWorkRecs;
	freeWorkRecs = w;
}

XtInputId XtAddInput( source, Condition, proc, closure)
	int source;
	XtPointer Condition;
	XtInputCallbackProc proc;
	XtPointer closure;
{
	return XtAppAddInput(_XtDefaultAppContext(),
		source, Condition, proc, closure);
}

XtInputId XtAppAddInput(app, source, Condition, proc, closure)
	XtAppContext app;
	int source;
	XtPointer Condition;
	XtInputCallbackProc proc;
	XtPointer closure;
{
	InputEvent* sptr;
#ifdef VMS
	struct _iosb     *p_iosb = (struct _iosb *)Condition;  
			   /* a pointer to an io status block or NULL if
                                   the user doesn't want to provide one. */ 
	int event_flag = source;/* the event flag to watch. Normally allocated
                                   with LIB$GET_EF or LIB$RESERVE_EF */
#else
	XtInputMask condition = (XtInputMask) Condition;
#endif /* VMS */
	
#ifndef VMS
	if (!condition ||
	    condition & ~(XtInputReadMask|XtInputWriteMask|XtInputExceptMask))
	    XtAppErrorMsg(app,"invalidParameter","xtAddInput",XtCXtToolkitError,
			  "invalid condition passed to XtAppAddInput",
			  (String *)NULL, (Cardinal *)NULL);
#endif /* VMS */

	if (app->input_max <= source) {
	    Cardinal n = source + 1;
	    app->input_list = (InputEvent**)XtRealloc((char*) app->input_list,
						      n * sizeof(InputEvent*));
	    bzero((char *) &app->input_list[app->input_max],
		  (unsigned) (n - app->input_max) * sizeof(InputEvent*));
	    app->input_max = n;
	}
	sptr = XtNew(InputEvent);
	sptr->ie_proc = proc;
	sptr->ie_closure = closure;
	sptr->app = app;
#ifdef VMS
	sptr->ie_source = event_flag;
        sptr->ie_iosb = p_iosb;
        sptr->ie_next = NULL;  /* NOT USED; */
        sptr->ie_oq = NULL;    /* NOT USED; */
        sptr->ie_condition = NULL;    /* NOT USED; */

        /* OR the new event flag into the current mask */
        app->Input_EF_Mask |= (1 << event_flag);

        /* place the new event flag record in the list */
	app->input_list[event_flag] = sptr;
#else
	sptr->ie_oq = NULL;
	sptr->ie_source = source;
	sptr->ie_condition = condition;
	sptr->ie_next = app->input_list[source];
	app->input_list[source] = sptr;

	if (condition & XtInputReadMask)   FD_SET(source, &app->fds.rmask);
	if (condition & XtInputWriteMask)  FD_SET(source, &app->fds.wmask);
	if (condition & XtInputExceptMask) FD_SET(source, &app->fds.emask);

	if (app->fds.nfds < (source+1)) app->fds.nfds = source+1;
	app->fds.count++;
#endif /* VMS */

	return((XtInputId)sptr);

}

void XtRemoveInput( id )
	register XtInputId  id;
{
#ifdef VMS
        register event_flag;
        XtAppContext app;

	if (id == NULL) return;		/* Sanity */

        event_flag = ((InputEvent *)id)->ie_source;
        app = ((InputEvent *)id)->app;

        /* remove the event flag from the mask */
        app->Input_EF_Mask &= ~(1 << event_flag);
        /* remove the InputEvent record from the list */
        app->input_list[event_flag] = 0;

        XtFree ((char *) id);
#else
  	register InputEvent *sptr, *lptr;
	XtAppContext app = ((InputEvent *)id)->app;
	register int source = ((InputEvent *)id)->ie_source;
	Boolean found = False;

	sptr = app->outstandingQueue;
	lptr = NULL;
	for (; sptr != NULL; sptr = sptr->ie_oq) {
	    if (sptr == (InputEvent *)id) {
		if (lptr == NULL) app->outstandingQueue = sptr->ie_oq;
		else lptr->ie_oq = sptr->ie_oq;
	    }
	    lptr = sptr;
	}

	if(app->input_list && (sptr = app->input_list[source]) != NULL) {
		for( lptr = NULL ; sptr; sptr = sptr->ie_next ){
			if(sptr == (InputEvent *) id) {
				XtInputMask condition = 0;
				if(lptr == NULL) {
				    app->input_list[source] = sptr->ie_next;
				} else {
				    lptr->ie_next = sptr->ie_next;
				}
				for (lptr = app->input_list[source];
				     lptr; lptr = lptr->ie_next)
				    condition |= lptr->ie_condition;
				if ((sptr->ie_condition & XtInputReadMask) &&
				    !(condition & XtInputReadMask))
				   FD_CLR(source, &app->fds.rmask);
				if ((sptr->ie_condition & XtInputWriteMask) &&
				    !(condition & XtInputWriteMask))
				   FD_CLR(source, &app->fds.wmask);
				if ((sptr->ie_condition & XtInputExceptMask) &&
				    !(condition & XtInputExceptMask))
				   FD_CLR(source, &app->fds.emask);
				XtFree((char *) sptr);
				found = True;
				break;
			}
			lptr = sptr;	      
		}
	}

    if (found)
	app->fds.count--;
    else
	XtAppWarningMsg(app, "invalidProcedure","inputHandler",XtCXtToolkitError,
                   "XtRemoveInput: Input handler not found",
		   (String *)NULL, (Cardinal *)NULL);
#endif /* VMS */
}

#ifdef VMS
static Boolean DoVMSInput(app, onlyOne, call)
	XtAppContext app;
	Boolean onlyOne;	/* return value only meaningful if this set */
	Boolean call;		/* If FALSE, don't call, just check */
{
        int status, i;
        unsigned long efnMask;

	if (app->Input_EF_Mask == 0) return FALSE;

	/* get the state of the event flags */
	status = SYS$READEF (1,&efnMask);

	/* proceed only if status successful */
	if ((status & 1) != 1) return FALSE;

	/* mask out non-XtAddInput efns, efnMask should now
	hold those EFNs registered with XtAddInput and set */
	efnMask &= app->Input_EF_Mask;

	/* if none are set then quit */
	if (efnMask == 0) return FALSE;

	/* go through the EFNs to see which are set */
	for (i=1; i < app->input_max; i++) {
	    /* If the event flag is set... */
	    if (((1<<i) & efnMask) !=0) {
		/* ... and there was no IOSB */
		if (app->input_list[i]->ie_iosb == 0) {
		    if (call) IeCallProc(app->input_list[i]);
		    if (onlyOne) return TRUE;
		} else { /* ...and the IOSB filled in */
		    if (app->input_list[i]->ie_iosb->cond_val != 0) {
			if (call) IeCallProc(app->input_list[i]);
			if (onlyOne) return TRUE;
		    }
		}
	    }
 	    /* mask out non-XtAddInput efns, efnMask should now
	    hold those EFNs registered with XtAddInput and set */
	    efnMask &= app->Input_EF_Mask;

	    /* if none are set then quit */
	    if (efnMask == 0) return FALSE;
	}
	return FALSE;
}
#endif /* VMS */


void _XtRemoveAllInputs(app)
    XtAppContext app;
{
    int i;
    for (i = 0; i < app->input_max; i++) {
	InputEvent* ep = app->input_list[i];
	while (ep) {
	    InputEvent *next = ep->ie_next;
	    XtFree( (char*)ep );
	    ep = next;
	}
    }
    XtFree((char *) app->input_list);
}

/* Do alternate input and timer callbacks if there are any */

static void DoOtherSources(app)
	XtAppContext app;
{
	TimerEventRec *te_ptr;
#ifdef VMS
        vms_time cur_time;
#else
	InputEvent *ie_ptr;
	struct timeval  cur_time;
#endif /* VMS */

#ifdef VMS
	(void) DoVMSInput(app, FALSE, TRUE);
#else
#define DrainQueue() \
	for (ie_ptr = app->outstandingQueue; ie_ptr != NULL;) { \
	    app->outstandingQueue = ie_ptr->ie_oq;		\
	    ie_ptr ->ie_oq = NULL;				\
	    IeCallProc(ie_ptr);					\
	    ie_ptr = app->outstandingQueue;			\
	}
/*enddef*/
	DrainQueue();
	if (app->fds.count > 0) {
	    /* Call _XtwaitForSomething to get input queued up */
	    (void) _XtwaitForSomething(TRUE, FALSE, TRUE, FALSE,
		(unsigned long *)NULL, app);
	    DrainQueue();
	}
#endif /* VMS */

	if (app->timerQueue != NULL) {	/* check timeout queue */
#ifdef VMS
	    GET_TIME(cur_time);
#else
	    (void) gettimeofday (&cur_time, NULL);
#endif /* VMS */
	    FIXUP_TIMEVAL(cur_time);
	    while(IS_AT_OR_AFTER (app->timerQueue->te_timer_value, cur_time)) {
		te_ptr = app->timerQueue;
		app->timerQueue = te_ptr->te_next;
		te_ptr->te_next = NULL;
		if (te_ptr->te_proc != NULL)
		    TeCallProc(te_ptr);
		te_ptr->te_next = freeTimerRecs;
		freeTimerRecs = te_ptr;
              if (app->timerQueue == NULL) break;
	    }
	}
#undef DrainQueue
}

/* If there are any work procs, call them.  Return whether we did so */

static Boolean CallWorkProc(app)
	XtAppContext app;
{
	register WorkProcRec *w = app->workQueue;
	Boolean delete;

	if (w == NULL) return FALSE;

	app->workQueue = w->next;

	delete = (*(w->proc)) (w->closure);

	if (delete) {
	    w->next = freeWorkRecs;
	    freeWorkRecs = w;
	}
	else {
	    w->next = app->workQueue;
	    app->workQueue = w;
	}
	return TRUE;
}

/*
 * XtNextEvent()
 * return next event;
 */

void XtNextEvent(event)
	XEvent *event;
{
	XtAppNextEvent(_XtDefaultAppContext(), event);
}

void _XtRefreshMapping(event, dispatch)
    XEvent *event;
    Boolean dispatch;
{
    XtPerDisplay pd = _XtGetPerDisplay(event->xmapping.display);

    if (event->xmapping.request != MappingPointer &&
	pd && pd->keysyms && (event->xmapping.serial >= pd->keysyms_serial))
	_XtBuildKeysymTables( event->xmapping.display, pd );
    XRefreshKeyboardMapping(&event->xmapping);
    if (dispatch && pd && pd->mapping_callbacks)
	XtCallCallbackList((Widget) NULL,
			   (XtCallbackList)pd->mapping_callbacks,
			   (XtPointer)event );
}

#ifdef VMS
static int VMSMultiplexInput(app, peek, ignoreTimers, ignoreInputs)
    XtAppContext app;
    Boolean peek;
    Boolean ignoreTimers;
    Boolean ignoreInputs;
{
    TimerEventRec *te_ptr;
    vms_time cur_time,result_time;
    int status;
    long quotient, remainder;
    long retval;
    int d;

    d = -1;
    do {
	unsigned long EF_Mask;
	if (ignoreInputs)
	    EF_Mask = 0;
	else
	    EF_Mask = app->Input_EF_Mask;
        retval = 0;
        if (app->timerQueue!= NULL && !ignoreTimers) {   /* check timeout queue */
            te_ptr = app->timerQueue;   
            GET_TIME(cur_time); 	
	    /* Jump through hoops to get the time specified in the queue into 
	       milliseconds */
            status = lib$sub_times (&te_ptr->te_timer_value.low, &cur_time, 
					&result_time);

            /*
             * See if this timer has expired.  A timer is considered expired
             * if it's value in the past (the NEGTIM case) or if there is
             * less than one integral milli second before it would go off.
             */

            if (status == LIB$_NEGTIM ||
		(result_time.high == -1 && result_time.low > -10000)) status = 0;
	    else if ((status & 1) == 1) {
                lib$ediv (&(10000), &result_time, &quotient, &remainder);
                quotient *= -1;         /* flip the sign bit */

                status = XMultiplexInput (app->count, &(app->list[0]), 
				EF_Mask, quotient, 0, &retval);
                } else status = -1;
        } else {
            status = XMultiplexInput (app->count, &(app->list[0]), 
				EF_Mask, 0, 0, &retval);
	}

        if (status == -1) {
           XtAppErrorMsg(app, "communicationError","XMultiplexInput",
	   	   XtCXtToolkitError, "Error in XMultiplexInput",NULL,NULL);
	}
	if (peek && status == 0 && app->timerQueue != NULL) return -1;
        if (retval > 0) {
            d = retval - 1;     /* Get to zero based index */
            if (!XPending(app->list[d]))       /* Make sure there is a 'real' 
							event there. */
                d = -1;
	} else if (retval == -1) /* An efn was set */
                status = 0;   /* make sure we break out of the DO-WHILE loop */
        else d = -1;
    } while ((d < 0) && (status == 1));
        /* while the Display has 'error' events */

    return d;
}
#endif /* VMS */

void XtAppNextEvent(app, event)
	XtAppContext app;
	XEvent *event;
{
    int i, d;

    for (;;) {
	if (app->count == 0)
	    DoOtherSources(app);
	else {
	    for (i = 1; i <= app->count; i++) {
		d = (i + app->last) % app->count;
		if (d == 0) DoOtherSources(app);
		if (XEventsQueued(app->list[d], QueuedAfterReading))
		    goto GotEvent;
	    }
	    for (i = 1; i <= app->count; i++) {
		d = (i + app->last) % app->count;
		if (XEventsQueued(app->list[d], QueuedAfterFlush))
		    goto GotEvent;
	    }
	}

	/* We're ready to wait...if there is a work proc, call it */
	if (CallWorkProc(app)) continue;

#ifdef VMS
	d = VMSMultiplexInput(app, FALSE, FALSE, FALSE);
#else
	d = _XtwaitForSomething(FALSE, FALSE, FALSE, TRUE,
				(unsigned long *) NULL, app);
#endif /* VMS */

	if (d != -1) {
	  GotEvent:
	    XNextEvent (app->list[d], event);
	    app->last = d;
	    if (event->xany.type == MappingNotify)
		_XtRefreshMapping(event, False);
	    return;
	} 

    } /* for */
}
    
void XtProcessEvent(mask)
	XtInputMask mask;
{
	XtAppProcessEvent(_XtDefaultAppContext(), mask);
}

void XtAppProcessEvent(app, mask)
	XtAppContext app;
	XtInputMask mask;
{
	int i, d;
	XEvent event;
#ifdef VMS
        vms_time cur_time;
#else
	struct timeval cur_time;
#endif /* VMS */

	if (mask == 0) return;

	for (;;) {
	    if (mask & XtIMTimer && app->timerQueue != NULL) {
#ifdef VMS
	        GET_TIME(cur_time);
#else
		(void) gettimeofday (&cur_time, NULL);
#endif /* VMS */
		FIXUP_TIMEVAL(cur_time);
		if (IS_AT_OR_AFTER(app->timerQueue->te_timer_value, cur_time)) {
		    TimerEventRec *te_ptr = app->timerQueue;
		    app->timerQueue = app->timerQueue->te_next;
		    te_ptr->te_next = NULL;
                    if (te_ptr->te_proc != NULL)
		        TeCallProc(te_ptr);
		    te_ptr->te_next = freeTimerRecs;
		    freeTimerRecs = te_ptr;
		    return;
		}
	    }
    
	    if (mask & XtIMAlternateInput) {
#ifdef VMS
		if (DoVMSInput(app, TRUE, TRUE)) return;
#else
		if (app->fds.count > 0 && app->outstandingQueue == NULL) {
		    /* Call _XtwaitForSomething to get input queued up */
		    (void) _XtwaitForSomething(TRUE, FALSE, TRUE, FALSE,
			    (unsigned long *)NULL, app);
		}
		if (app->outstandingQueue != NULL) {
		    InputEvent *ie_ptr = app->outstandingQueue;
		    app->outstandingQueue = ie_ptr->ie_oq;
		    ie_ptr->ie_oq = NULL;
		    IeCallProc(ie_ptr);
		    return;
		}
#endif /* VMS */
	    }
    
	    if (mask & XtIMXEvent) {
		for (i = 1; i <= app->count; i++) {
		    d = (i + app->last) % app->count;
		    if (XEventsQueued(app->list[d], QueuedAfterReading))
			goto GotEvent;
		}
		for (i = 1; i <= app->count; i++) {
		    d = (i + app->last) % app->count;
		    if (XEventsQueued(app->list[d], QueuedAfterFlush))
			goto GotEvent;
		}
	    }

	    /* Nothing to do...wait for something */

	    if (CallWorkProc(app)) continue;

#ifdef VMS
	    d = VMSMultiplexInput(app, FALSE,
				    (mask & XtIMTimer ? FALSE : TRUE),
				    (mask & XtIMAlternateInput ? FALSE : TRUE));
#else
	    d = _XtwaitForSomething(
				    (mask & XtIMTimer ? FALSE : TRUE),
				    (mask & XtIMAlternateInput ? FALSE : TRUE),
				    (mask & XtIMXEvent ? FALSE : TRUE),
				    TRUE,
				    (unsigned long *) NULL, app);
#endif /* VMS */	    

	    if (mask & XtIMXEvent && d != -1) {
	      GotEvent:
		XNextEvent(app->list[d], &event);
		app->last = d;
		if (event.xany.type == MappingNotify) {
		    _XtRefreshMapping(&event, False);
		}
		XtDispatchEvent(&event);
		return;
	    } 
	
	}    
}

XtInputMask XtPending()
{
	return XtAppPending(_XtDefaultAppContext());
}

XtInputMask XtAppPending(app)
	XtAppContext app;
{
#ifdef VMS
        vms_time cur_time;
#else
	struct timeval cur_time;
#endif /* VMS */
	int d;
	XtInputMask ret = 0;

/*
 * Check for pending X events
 */
	for (d = 0; d < app->count; d++) {
	    if (XEventsQueued(app->list[d], QueuedAfterReading)) {
		ret = XtIMXEvent;
		break;
	    }
	}
	if (ret == 0) {
	    for (d = 0; d < app->count; d++) {
		if (XEventsQueued(app->list[d], QueuedAfterFlush)) {
		    ret = XtIMXEvent;
		    break;
		}
	    }
	}

/*
 * Check for pending alternate input
 */
	if (app->timerQueue != NULL) {	/* check timeout queue */ 
#ifdef VMS
	    GET_TIME(cur_time);
#else
	    (void) gettimeofday (&cur_time, NULL);
#endif /* VMS */
	    FIXUP_TIMEVAL(cur_time);
	    if ((IS_AT_OR_AFTER(app->timerQueue->te_timer_value, cur_time))  &&
                (app->timerQueue->te_proc != 0)) {
		ret |= XtIMTimer;
	    }
	}

#ifdef VMS
	if (DoVMSInput(app, TRUE, FALSE)) ret |= XtIMAlternateInput;
#else
	if (app->outstandingQueue != NULL) ret |= XtIMAlternateInput;
	else {
	    /* This won't cause a wait, but will enqueue any input */

	    if(_XtwaitForSomething(TRUE, FALSE, FALSE, FALSE, (unsigned long *) NULL,
		    app) != -1) ret |= XtIMXEvent;
	    if (app->outstandingQueue != NULL) ret |= XtIMAlternateInput;
	}
#endif /* VMS */
	return ret;
}

/* Peek at alternate input and timer callbacks if there are any */

static Boolean PeekOtherSources(app)
	XtAppContext app;
{
#ifdef VMS
        vms_time cur_time;
#else
	struct timeval  cur_time;
#endif /* VMS */

#ifdef VMS
	if (DoVMSInput(app, TRUE, FALSE)) return TRUE;
#else
	if (app->outstandingQueue != NULL) return TRUE;

	if (app->fds.count > 0) {
	    /* Call _XtwaitForSomething to get input queued up */
	    (void) _XtwaitForSomething(TRUE, FALSE, TRUE, FALSE,
		    (unsigned long *)NULL, app);
	    if (app->outstandingQueue != NULL) return TRUE;
	}
#endif /* VMS */

	if (app->timerQueue != NULL) {	/* check timeout queue */
#ifdef VMS
	    GET_TIME(cur_time);
#else
	    (void) gettimeofday (&cur_time, NULL);
#endif /* VMS */
	    FIXUP_TIMEVAL(cur_time);
	    if (IS_AT_OR_AFTER (app->timerQueue->te_timer_value, cur_time)) return TRUE;
	}

	return FALSE;
}

Boolean XtPeekEvent(event)
	XEvent *event;
{
	return XtAppPeekEvent(_XtDefaultAppContext(), event);
}

Boolean XtAppPeekEvent(app, event)
	XtAppContext app;
	XEvent *event;
{
	int i, d;
	Boolean foundCall = FALSE;
	
	for (i = 1; i <= app->count; i++) {
	    d = (i + app->last) % app->count;
	    if (d == 0) foundCall = PeekOtherSources(app);
	    if (XEventsQueued(app->list[d], QueuedAfterReading))
		goto GotEvent;
	}
	for (i = 1; i <= app->count; i++) {
	    d = (i + app->last) % app->count;
	    if (XEventsQueued(app->list[d], QueuedAfterFlush))
		goto GotEvent;
	}
	
	if (foundCall) {
	    event->xany.type = 0;
	    event->xany.display = NULL;
	    event->xany.window = 0;
	    return FALSE;
	}
	
#ifdef VMS
          d = VMSMultiplexInput(app, TRUE, FALSE, FALSE);
#else
	d = _XtwaitForSomething(FALSE, FALSE, FALSE, TRUE,
				(unsigned long *) NULL, app);
#endif /* VMS */
	
	if (d != -1) {
	  GotEvent:
	    XPeekEvent(app->list[d], event);
	    app->last = (d == 0 ? app->count : d) - 1;
	    return TRUE;
	}
	event->xany.type = 0;	/* Something else must be ready */
	event->xany.display = NULL;
	event->xany.window = 0;
	return FALSE;
}	
