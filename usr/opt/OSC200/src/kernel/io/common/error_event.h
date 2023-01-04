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
 * @(#)$RCSfile: error_event.h,v $ $Revision: 1.1.2.5 $ (DEC) $Date: 1993/11/23 21:56:35 $
 */
/*
 *
 *  FILE:	error_event.h
 *
 *  DESCRIPTION:
 * 	Contains the defines and data structure definitions 
 *	used by the kernel error event functions.
 *
 */

#ifndef _ERROR_EVENT_H_
#define _ERROR_EVENT_H_

/*
 * Event entry created when registering a device and fucntion to be called
 * whenever the event_notify() routine indicates an error occurred for
 * the registered device.
 */
typedef struct event_entry {
	struct event_entry *flink;		/* forward link to next entry */
	struct event_entry *blink;		/* back link to next entry */
	dev_t 		   ee_dev;		/* dev_t for device to watch */
						/* only uses UNIT number */
	U32 		   ee_event_flags;	/* curently not used */
	void 		   (*ee_error_func)();	/* function to be called when */
						/* an error occurs on dev */
	U32		   ee_reg_type;		/* registered type, see below */
						/* allows for deregistration */
						/* based on regsitered type. */
} EVENT_ENTRY;

/*
 * Event control structure - acts as anchor for event entries.
 */
typedef struct event_cntrl  {
	EVENT_ENTRY 	*ec_flink;	/* forward link to event entry */
	EVENT_ENTRY 	*ec_blink;	/* back link to event entry */
	lock_data_t     ec_lk_event;	/* lock structure */
} EVENT_CNTRL;

/*
 * The following is a list of the possible registered types.
 */
#define EV_AVAIL_MANAGER	0x01	/* Availability Manager driver */
					/* used for ASE */

/*
 * The following lists the possible return values from the error event
 * functions.
 */
#define EV_FAILURE 	0
#define EV_SUCCESS	1

/*
 * FIND_EVENT_ENTRY - searches from start_ptr to find an event entry for dev.
 */
#define FIND_EVENT_ENTRY(dev, start_ptr, event_ptr)	\
{							\
	find_event_entry(dev, start_ptr, event_ptr);	\
}

/*
 * ADD_EVENT_ENTRY - adds the event_ptr event entry at location event_loc.
 */
#define ADD_EVENT_ENTRY(event_ptr, event_loc)		\
{							\
	insque(event_ptr, event_loc);			\
}

/*
 * REMOVE_EVENT_ENTRY - removes the links to the event_ptr event entry.
 */
#define REMOVE_EVENT_ENTRY(event_ptr)			\
{							\
	remque(event_ptr);				\
}

/*
 * The following are locking macros for the error event control structure.
 *
 * NOTE: GOLD Merge needs to remove SMP ifdef.
 */

#ifdef SMP

#define LOCK_IT(lk, flags)  			\
{						\
    extern shutting_down; 			\
    if( !shutting_down ) ulock_write((lk)); 	\
}

#define UNLOCK_IT(lk)				\
{						\
    extern shutting_down; 			\
    if( !shutting_down) ulock_done((lk)); 	\
}						\

#define LOCK_INIT() 							\
{									\
	ulock_setup((lock_t)&lk_event_cntrl, event_cntrl_li, TRUE); 	\
}		

#else

#define LOCK_IT(lk, flags)

#define UNLOCK_IT(lk)

#define LOCK_INIT() 					\
{                                                       \
    lock_init(&lk_event_cntrl, TRUE);                  	\
}

#endif /* SMP */

#define EVENT_INIT_LOCK()				\
{							\
    LOCK_INIT();					\
}

#define EVENT_LOCK(saveipl)                		\
{                                                       \
    saveipl = splbio();                                 \
    LOCK_IT(&lk_event_cntrl, LK_RETRY);         	\
}

#define EVENT_UNLOCK(saveipl)                       	\
{                                                       \
    UNLOCK_IT(&lk_event_cntrl);                     	\
    (void)splx(saveipl);                                \
}

#endif   /* _ERROR_EVENT_H_ */
