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
 *	@(#)$RCSfile: sched_mon.h,v $ $Revision: 4.3 $ (DEC) $Date: 1992/01/15 02:12:13 $
 */ 
/*
 */

/*
 * OSF/1 Release 1.0
 */

/*
 *  This file contains definitions for a scheduler monitoring facility.
 *  The facility is turned on by defining RT_SCHED_MON in the build options
 *  file.
 *
 *  This is a part of the POSIX RT sceduling effort.
 */

#ifndef _SYS_SCHED_MON_H_
#define _SYS_SCHED_MON_H_

#if KERNEL
#include <rt_sched_mon.h>
#else /* KERNEL */
#ifndef RT_SCHED_MON
#define RT_SCHED_MON 1
#endif /* RT_SCHED_MON */
#endif /* KERNEL */

#include <sys/syscall.h>
#include <mach/policy.h>

/* This header only needed when defining kernel internal data structures. */
#if KERNEL
#include <kern/lock.h>
#endif

/* 
 * Coverage Monitoring
 *
 * The following code is only compiled in if RT_SCHED_MON is enabled.
 *
 * The scheduler monitor consists of an event buffer, an array of event
 * counters, and a set of macros which, when invoked from kernel code,
 * record events in the event buffer.  Syscalls allow user programs to
 * read the recent scheduler history and counters.  There are also syscalls
 * which control how much information is recorded in the history buffer.
 *
 * This monitor is intended to be useful for debugging, coverage analysis,
 * and scheduler-related crashe analysis (hopefully this last purpose
 * will never be needed).
 *
 * To define a new event, do the following:
 *   - Define an event constant of the form RTS_mumble for the event.  Use
 *     a unique number.
 *   - If the event is to have a counter associated with it, the event
 *     constant should be one higher than the value of RT_SCHED_HIST_EVTS.
 *     Increase RT_SCHED_HIST_EVTS to include the new event.
 *   - Insert a call to RT_SCHED_HIST{|_NL|SPL} where the event is to be
 *     recorded.
 *   - Modify hist.c and counts.c to understand the events.
 *
 * The event macro has the form:
 *
 *   RT_SCHED_HIST(event,thread,policy,d1,d2,d3)
 *
 *     event: Number of the event to record (use an event constant).
 *     thread: Thread the event relates to.
 *     policy: thread->policy flags, or bitwise logical and/or of multiple
 *             policy flags.  This is used to control recording of events.
 *     d1,d2: Arbitrary longword data which is relevant to the event.
 *     d3: Arbitrary unsigned byte which is relevant to the event.
 *
 * The RT_SCHED_HIST macro assumes that spl has been raised to splsched() and
 * that the rt_sched_hist lock has not been taken.  If spl has not been raised
 * and the lock has not been taken, use RT_SCHED_HIST_SPL.  If spl has been
 * raised and the lock has been taken, use RT_SCHED_HIST_NL.
 *
 * The following syscalls provide access to the history buffer.
 *
 *   int getschedhist(bufmax,&ctxt,&buf,&lost)
 *
 *      Copies the most recent scheduler events into a user's buffer, pointed
 *      to by _buf_ and with maximum size (in bytes) _bufmax_.  Each entry is
 *      a struct rt_sched_hist_entry (currently four longwords long).  The
 *      number of entries copied is returned.  If something went wrong, a
 *      negative number (not just -1) is returned and errno is set.
 *
 *      The _ctxt_ parameter is a longword containing an event number.  It is
 *      updated to point to the next unread event.  The _ctxt_ parameter should
 *      be initialized to 0 before the first call.  The _lost_ parameter is set
 *      to 0 if no events were lost before the call was made.  If events were
 *      lost, the _lost_ parameter indicates how many were lost.
 *
 *      Note that the user's buffer does not have to be as large as the
 *      internal kernel history buffer.  As long as the caller can call often
 *      enough to keep up with the scheduler, no events will be lost.  However,
 *      each invocation of the user's thread causes scheduling events, so the
 *      larger the buffer, the less likelihood that the monitoring program will
 *      interfere with the system.
 *
 *   int getschedcounts(bufmax,&buf);
 *
 *      Copies scheduler counters into the user's buffer _buf_, which has a
 *      maximum length of _bufmax_.  Each counter is one longword long.  The
 *      number of bytes copied is returned.  If anything goes wrong, a
 *      negative number is returned.
 *
 *      If buf is defined as "long buf[<size>]", then after a successful call,
 *      buf[0] will contain the count of occurrences of event 1, buf[1] will
 *      contain the count of occurrences of event 2, etc.
 *
 *   int getschedctrl(start,len,&ctrl)
 *
 *      Gets the current scheduler control values.  Each event with a value
 *      in the range 1..RT_SCHED_HIST_EVTS has its own separate control byte.
 *      Also, there is a catch-all control byte which is accessed by using
 *      the value 0 (RTS_catchall).
 *
 *      Multiple control bytes may be read at once.  This is accomplished
 *      by specifying the index of the first control byte to be read (start), 
 *      the number of bytes to read (len), and the address of the control 
 *      buffer (ctrl).
 *
 *      The system service returns the number of bytes written, or a negative
 *      number if the call was unsuccessful.
 *
 *   int setschedctrl(start,len,&ctrl)
 *   int andschedctrl(start,len,&ctrl)
 *   int orschedctrl(start,len,&ctrl)
 *
 *      These functions modify the scheduler history control values.  They
 *      operate on a range of values, as getschedctrl does (see above).
 *      The setschedctrl() function sets the controls to the specified values.
 *      The andschedctrl() function performs a bitwise "and" with the existing
 *      values.  The orschedctrl() function performs a bitwise "or."  The
 *      logical operators are provided to allow atomic operations on the
 *      control values.
 */

/*
 * Function codes for schedmon()
 */
#define SCHEDMON_GET_COUNTS	1
#define SCHEDMON_GET_HISTORY	2
#define SCHEDMON_GET_CTRL	3
#define SCHEDMON_SET_CTRL	4
#define SCHEDMON_CLR_CTRL	5

/*
 * Control Type
 *
 * This is the type of a scheduler monitor control flag.
 */
typedef unsigned char rt_sched_ctrl_t;

/*
 * Syscall Declarations
 *
 * For now, these are macros.  Eventually, need to add real syscalls.
 */

#if !KERNEL
#define getschedcounts(bufmax,buf) syscall(SYS_getschedcounts,(bufmax),(buf))
#define getschedhist(bufmax,ctxt,buf,lost) \
		syscall(SYS_getschedhist,(bufmax),(ctxt),(buf),(lost))
#define getschedctrl(start,len,ctrl) \
                syscall(SYS_getschedctrl,(start),(len),(ctrl))
#define setschedctrl(start,len,ctrl) \
                syscall(SYS_setschedctrl,(start),(len),(ctrl))
#define andschedctrl(start,len,ctrl) \
                syscall(SYS_andschedctrl,(start),(len),(ctrl))
#define orschedctrl(start,len,ctrl) \
                syscall(SYS_orschedctrl,(start),(len),(ctrl))
#endif /* !KERNEL */

/*
 * Scheduling Control
 *
 * The control information consists of an unsigned byte whose bits control the
 * behavior of the scheduler monitor as follows:
 *
 *    2^0 Record actions on time sharing policy threads.
 *    2^1 Record actions on Mach fixed priority policy threads.
 *    2^2 Record actions on POSIX FIFO policy threads.
 *    2^3 Record actions on POSIX Round Robin policy threads.
 *    2^4 <unused>
 *    2^5 If two of these events occur in a row, write over the previous event
 *        in the history buffer to save space.
 *    2^6 Record this event in its counter.
 *    2^7 Record this event in the history buffer.
 *
 * The default is 0xEC: Record only activities regarding POSIX FIFO and RR
 * threads, record this information in both the history buffer and counters,
 * and merge similar events.
 */
#define RT_SCHED_CTRL_HISTORY 0x80
#define RT_SCHED_CTRL_COUNTERS 0x40
#define RT_SCHED_CTRL_MERGE 0x20
#define RT_SCHED_CTRL_POLICY_TIMESHARE POLICY_TIMESHARE
#define RT_SCHED_CTRL_POLICY_FIXEDPRI POLICY_FIXEDPRI
#define RT_SCHED_CTRL_POLICY_FIFO POLICY_FIFO
#define RT_SCHED_CTRL_POLICY_RR POLICY_RR
#define RT_SCHED_CTRL_POLICY_RT (POLICY_FIFO|POLICY_RR)
#define RT_SCHED_CTRL_POLICY_FIXED (POLICY_FIXEDPRI|POLICY_FIFO|POLICY_RR)
#define RT_SCHED_CTRL_POLICY_ALL \
             (POLICY_TIMESHARE|POLICY_FIXEDPRI|POLICY_FIFO|POLICY_RR)
#define RT_SCHED_CTRL_DEFAULT \
         (RT_SCHED_CTRL_HISTORY|RT_SCHED_CTRL_COUNTERS| \
          RT_SCHED_CTRL_MERGE|RT_SCHED_CTRL_POLICY_FIXED)

/*
 * Scheduling Events
 *
 * The following constants represent events which are to be recorded in a
 * history ring buffer.  Some of the constants are also indices into an
 * array of counters which accumulate the total number of events.
 *
 * The constant RT_SCHED_HIST_EVTS indicates the highest event index which
 * is stored in the counter array.
 */

/* Catch-all control information (not an event). */
#define RTS_catchall 0

/* bsd/kern_resource.c */

#define RTS_donice 1
#define RTS_setprio 2
#define RTS_getprio 3
#define RTS_setscheduler 4
#define RTS_getscheduler 5
#define RTS_yield 6

/* bsd/kern_sig.c */
#define RTS_kill 7

/* bsd/kern_synch.c */
#define RTS_autonice 8

/* kern/ast.c */
#define RTS_shutdown 9
#define RTS_thast 10
#define RTS_ast 11
#define RTS_astcsw 12
#define RTS_rqempty5 46
#define RTS_tspreempt 13
#define RTS_rrqsave5 47
#define RTS_rqempty4 45
#define RTS_preempt 14
#define RTS_sig 15

/* kern/clock_prim.c */
#define RTS_rrqexp 16

/* kern/ipc_basics.c */
#define RTS_tsswitch 17

/* kern/sched_prim.c */
#define RTS_await 37
#define RTS_wake 38
#define RTS_block 39
#define RTS_rrqsave1 18
#define RTS_rrqrest1 20
#define RTS_rqempty1 21
#define RTS_rrqsave2 22
#define RTS_rrqrest2 24
#define RTS_exec 40
#define RTS_run 42
#define RTS_totail1 43
#define RTS_tohead1 25
#define RTS_totail2 44
#define RTS_tohead2 26
#define RTS_setrun 41
#define RTS_rrqsave3 19
#define RTS_rrqsave4 23
#define RTS_remq 48
#define RTS_rqempty2 27
#define RTS_rqempty3 28
#define RTS_rqlow 29
#define RTS_rrqrest3 30
#define RTS_rqinvalid 36
#define RTS_rqmask 31


/* kern/thread.c */
#define RTS_policy 32
#define RTS_priority 33
#define RTS_max_priority 34

/* vm/vm_unix.c */
#define RTS_fork 35

/* kern/thread_swap.c */
#define RTS_swapin 49
#define RTS_swapout 50

/* not yet implemented */
#define RTS_pth_setprio 60
#define RTS_pth_getprio 61
#define RTS_pth_setscheduler 62
#define RTS_pth_getscheduler 63
#define RTS_pth_yield 64

/* to be folded in at some time */

/* Range of events to count. */
#define RT_SCHED_HIST_EVTS 50

#if RT_SCHED_MON

/* Array of counters for scheduler events. */
#define RTS_HIST_SIZE 4*RT_SCHED_HIST_EVTS
typedef int *rt_sched_hist_counts_t[RT_SCHED_HIST_EVTS];

/* Kernel counters. */
#if KERNEL
extern int rt_sched_hist_counts[RT_SCHED_HIST_EVTS+1];
#endif /* KERNEL */

/* Structure to hold a single scheduler event.  Used internally and by
   the system call to read history information. */
struct rt_sched_hist_entry {
  unsigned char event;        /* Event code. */
  unsigned char policy;       /* Policy mask passed to macro. */ 
  unsigned char duplicates;   /* Number of duplicates of this event. */
  unsigned char d3;           /* Third data value (only one byte). */
  int th;                     /* Thread id of subject. */
  int d1, d2;                 /* First two data values. */
};
typedef struct rt_sched_hist_entry *rt_sched_hist_entry_t;

/* Kernel buffer to hold history information. */
#if KERNEL
#define RT_SCHED_HIST_LEN 4095
struct rt_sched_hist {		/* History buffer for debugging scheduler */
  unsigned count;               /* Count of events recorded. */
  short next;			/* Next cell to write in buffer. */
  short last;                   /* Last cell written in buffer. */
  int ctrl_len;                 /* Length of history control structure. */
  int fill2;  
  struct rt_sched_hist_entry buf[RT_SCHED_HIST_LEN];
  unsigned char ctrl[RT_SCHED_HIST_EVTS+1];
};
extern struct rt_sched_hist rt_sched_hist;
#endif /* KERNEL */

/* Locking macros for history information. */
#if KERNEL
decl_simple_lock_data(extern,rt_sched_hist_lock)
#define rt_sched_hist_lock()    simple_lock(&rt_sched_hist_lock)
#define rt_sched_hist_unlock()  simple_unlock(&rt_sched_hist_lock)
#endif /* KERNEL */

/* Record a scheduling history event.
 *
 * The counters are bumped only when a new event is started.  This is to
 * reduce the likelihood of a counter rollover.
 *
 *  RT_SCHED_HIST_NL  records an event with no locking or spl change.
 *  RT_SCHED_HIST     locks before recording the event.
 *  RT_SCHED_HIST_SPL sets spl and locks before recording the event.
 *
 * The history buffer should only be accessed when at splsched() with the
 * history lock taken.
 */

#if KERNEL
#define RT_SCHED_HIST_NL(_evt,_th,_policy,_d1,_d2,_d3)			\
{									\
  register short _rts_hist_index;					\
  register struct rt_sched_hist_entry *_rts_hist_entry;			\
  register unsigned char _rts_hist_ctrl;				\
  register unsigned char _rts_hist_evt = (unsigned char)(_evt);		\
  register unsigned char _rts_policy = (unsigned char)(_policy);	\
									\
  LASSERT(rt_sched_hist_lock.lock_data != 0);				\
  if (((_rts_hist_evt) <= 0) || 					\
      ((_rts_hist_evt) >= rt_sched_hist.ctrl_len)) {			\
    _rts_hist_evt = RTS_catchall;					\
  }									\
  _rts_hist_ctrl = rt_sched_hist.ctrl[_rts_hist_evt];			\
									\
  if (_rts_policy & _rts_hist_ctrl) {					\
    if (RT_SCHED_CTRL_HISTORY & _rts_hist_ctrl) {			\
      _rts_hist_index = rt_sched_hist.last;				\
      _rts_hist_entry = &rt_sched_hist.buf[_rts_hist_index];		\
      if ((_rts_hist_ctrl & RT_SCHED_CTRL_MERGE)			\
          && ((_evt) == _rts_hist_entry->event)				\
          && (_rts_hist_entry->duplicates < 255)) {			\
        _rts_hist_entry->duplicates++;					\
      }									\
      else {								\
        rt_sched_hist.count++;						\
        _rts_hist_index = rt_sched_hist.next;				\
        _rts_hist_entry = &rt_sched_hist.buf[_rts_hist_index];		\
        rt_sched_hist.last = _rts_hist_index;				\
        _rts_hist_index++;						\
        if (_rts_hist_index >= RT_SCHED_HIST_LEN) _rts_hist_index = 0;	\
        rt_sched_hist.next = _rts_hist_index;				\
        _rts_hist_entry->event = (unsigned char)(_evt);			\
        _rts_hist_entry->duplicates = 0;				\
      }									\
      _rts_hist_entry->th = (int)(_th);					\
      _rts_hist_entry->policy = _rts_policy;				\
      _rts_hist_entry->d1 = (int)(_d1);					\
      _rts_hist_entry->d2 = (int)(_d2);					\
      _rts_hist_entry->d3 = (unsigned char)(_d3);			\
    }									\
    if (RT_SCHED_CTRL_COUNTERS & _rts_hist_ctrl) {			\
      rt_sched_hist_counts[_rts_hist_evt]++;	 			\
    }									\
  }									\
}

#define RT_SCHED_HIST(evt,th,policy,d1,d2,d3) 				\
{									\
  ASSERT(issplsched());							\
  rt_sched_hist_lock();							\
  RT_SCHED_HIST_NL(evt,th,policy,d1,d2,d3);				\
  rt_sched_hist_unlock();						\
}

#define RT_SCHED_HIST_SPL(evt,th,policy,d1,d2,d3)			\
{									\
  register int s;							\
									\
  s = splsched();							\
  RT_SCHED_HIST(evt,th,policy,d1,d2,d3);				\
  splx(s);								\
}
#endif /* KERNEL */

#else /* RT_SCHED_MON */
#if KERNEL
#define RT_SCHED_HIST_NL(evt,th,policy,d1,d2,d3)
#define RT_SCHED_HIST(evt,th,policy,d1,d2,d3)
#define RT_SCHED_HIST_SPL(evt,th,policy,d1,d2,d3)
#endif /* KERNEL */
#endif /* RT_SCHED_MON */

#endif /* _SYS_SCHED_MON_H_ */
