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
 *	@(#)$RCSfile: cma_queue.h,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/08/18 14:50:04 $
 */
/*
 *  FACILITY:
 *
 *	CMA services
 *
 *  ABSTRACT:
 *
 *	Header file for generic queuing functions operating on circular
 *	double-linked lists.
 *
 *  AUTHORS:
 *
 *	Dave Butenhof
 *
 *  CREATION DATE:
 *
 *	24 August 1989
 *
 *  MODIFICATION HISTORY:
 *
 *	001	Dave Butenhof	18 October 1989
 *		Change queue header operations to operate on "cma__t_queue *"
 *		rather than generic "void *".  Change "void *" to
 *		"cma_t_address" (which is same if compiler supports it).
 *		Convert some routines to macros for compilers which don't
 *		support #inline pragma.
 *	002	Dave Butenhof	19 October 1989
 *		All queue operations should accept and return "cma__t_queue"
 *		type objects.  Require caller to use explicit casts where
 *		necessary, to support C compilers which don't correctly
 *		handle ANSI C's "void *" data type.
 *	003	Webb Scales	20 October 1989
 *		Move cma__queue_dequeue into cma_queue.c:  on MIPS, when the
 *		routine definition is included multiple time, we get competing
 *		definitions at link time.
 *	004	Dave Butenhof	23 October 1989
 *		Make cma__queue_dequeue prototype "extern" instead of
 *		"static", since definition was moved to .c...
 *	005	Webb Scales	15 November 1989
 *		Added assertions to queue operations
 *	006	Webb Scales	21 June 1990
 *		Added macro to insert after an element or at the head of queue.
 *	007	Dave Butenhof	11 June 1991
 *		Add trace array capability for queue operations; too
 *		frequent for cma__trace, but it's sometimes useful to be able
 *		to follow the sequences.
 *	008	Webb Scales	 8 May 1992
 *		Added queue element consistency checks (to augment existing
 *		queue header consistency checks); improved bugcheck messages;
 *		added a new macro which zero's an element's queue links.
 *	009	Dave Butenhof	25 August 1992
 *		Improve queue_dequeue by adding macro version
 *		(queue_dequeuei) that takes output parameter rather than
 *		trying to pretend it's a function.
 *	048	Dave Butenhof	23 September 1992
 *		Not for the first time, I "cleaned up" some cma__dequeuei()
 *		macro calls to put the arguments on separate lines (avoiding
 *		lines > 80 columns). And, not for the first time, I
 *		discovered that this DOESN'T COMPILE on VAX or MIPS ULTRIX.
 *		The newline and whitespace between "," or "(" and each
 *		argument is included in the argument replacement, and breaks
 *		assert macros that cause the arguments to be replaced within
 *		a string. Rename the arguments to avoid this replacement
 *		(although it's nice to have the assert message report the
 *		actual queue, it ain't worth the aggravation).
 *	049	Dave Butenhof	13 April 1993
 *		Get rid of cma__queue_dequeue() routine, and take over the
 *		name for cma__queue_dequeuei() macro. Change
 *		cma__queue_remove to also take output parameter, allowing use
 *		of VAX queue instructions.
 *	050	Brian Keane	20 April 1993
 *		Tweaks to make 049 work on non-VMS platforms.
 *	051	Dave Butenhof	23 June 1993
 *		To allow CV wait to unconditionally dequeue the awakened
 *		thread, wake initializes removed queue elements as "queue
 *		headers", rather than clearing them. This is really as good
 *		as "empty" elements for consistency checking, so modify the
 *		queue_zero macro and the various assertions to accept that.
 */


#ifndef CMA_QUEUE
#define CMA_QUEUE

/*
 *  INCLUDE FILES
 */

#if _CMA_PLATFORM_ == _CMA__VAX_VMS
# pragma builtins
#endif

/*
 * CONSTANTS AND MACROS
 */

/*
 * Remove the first entry from a queue
 * 
 *	_hd_	Address of queue header
 *
 *	_e_	Variable to receive address of element (type _t_*)
 *
 *	_t_	Type of *_e_
 */
#if _CMA_PLATFORM_ == _CMA__VAX_VMS
# define cma__queue_dequeue(_dhd_,_de_,_dt_) { \
    cma__t_queue    *_dh_ = (_dhd_); \
    cma__assert_fail ( \
	    _dh_ != (cma__t_queue *)cma_c_null_ptr, \
	    "queue_dequeue: dequeue using null queue head"); \
    cma__assert_fail ( \
	    ((_dh_->flink != (cma__t_queue *)cma_c_null_ptr) \
	    && (_dh_->blink != (cma__t_queue *)cma_c_null_ptr)), \
	    "queue_dequeue: dequeue on uninitialized head"); \
    cma__assert_fail ( \
	    _dh_->blink->flink == _dh_, \
	    "queue_dequeue: corruption -- head->blink->flink != head"); \
    cma__assert_fail ( \
	    _dh_->flink->blink == _dh_, \
	    "queue_dequeue: corruption -- head->flink->blink != head"); \
    if (_REMQUE (_dh_->flink, (void **)&(_de_)) == 2) \
	_de_ = (_dt_ *)cma_c_null_ptr; \
    else cma__queue_zero((cma__t_queue *)(_de_));}
#else
# define cma__queue_dequeue(_dhd_,_de_,_dt_) { \
    cma__t_queue    *_dh_ = (_dhd_); \
    cma__assert_fail ( \
	    _dh_ != (cma__t_queue *)cma_c_null_ptr, \
	    "queue_dequeue: dequeue using null queue head"); \
    cma__assert_fail ( \
	    ((_dh_->flink != (cma__t_queue *)cma_c_null_ptr) \
	    && (_dh_->blink != (cma__t_queue *)cma_c_null_ptr)), \
	    "queue_dequeue: dequeue on uninitialized head"); \
    cma__assert_fail ( \
	    _dh_->blink->flink == _dh_, \
	    "queue_dequeue: corruption -- head->blink->flink != head"); \
    cma__assert_fail ( \
	    _dh_->flink->blink == _dh_, \
	    "queue_dequeue: corruption -- head->flink->blink != head"); \
    if (_dh_ == _dh_->flink) \
	_de_ = (_dt_ *)cma_c_null_ptr; \
    else \
	cma__queue_remove (_dh_->flink, _de_, _dt_);}
#endif

/*
 * Test whether a queue is empty.  Return cma_c_true if so, else
 * cma_c_false.
 */
#define cma__queue_empty(_ehd_)	(			\
    cma__assert_fail (					\
	    (_ehd_)->blink->flink == (_ehd_),		\
	    "queue_empty: queue corruption -- head->blink->flink != head"), \
    cma__assert_fail (					\
	    (_ehd_)->flink->blink == (_ehd_),		\
	    "queue_empty: queue corruption -- head->flink->blink != head"), \
    (_ehd_)->flink == (_ehd_))

/*
 * Initialize a queue header to empty.  (Note that the comma operator is used
 * merely to avoid the necessity for a block, not because a return value is
 * actually useful).
 */
#define cma__queue_init(_ihd_)	((_ihd_)->flink = (_ihd_)->blink = (_ihd_))

/*
 * Insert an element in a queue preceding the specified item (or at end of
 * queue if _qp_ is the queue head).
 */
#if _CMA_PLATFORM_ == _CMA__VAX_VMS
# define cma__queue_insert(_ie_,_iqp_)    (		\
    cma__assert_fail (					\
	    (_ie_)->flink == (_ie_), \
	    "queue_insert: queue corruption -- element flink not self"), \
    cma__assert_fail (					\
	    (_ie_)->blink == (_ie_),			\
	    "queue_insert: queue corruption -- element blink not self"), \
    cma__assert_fail (					\
	    (_iqp_)->blink->flink == (_iqp_),		\
	    "queue_insert: queue corruption -- q_ptr->blink->flink != q_ptr"), \
    cma__assert_fail (					\
	    (_iqp_)->flink->blink == (_iqp_),		\
	    "queue_insert: queue corruption -- q_ptr->flink->blink != q_ptr"), \
    _INSQUE (_ie_, (_iqp_)->blink))
#else
# define cma__queue_insert(_ie_,_iqp_)    (		\
    cma__assert_fail (					\
	    (_ie_)->flink == (_ie_), \
	    "queue_insert: queue corruption -- element flink not self"), \
    cma__assert_fail (					\
	    (_ie_)->blink == (_ie_),			\
	    "queue_insert: queue corruption -- element blink not self"), \
    cma__assert_fail (					\
	    (_iqp_)->blink->flink == (_iqp_),		\
	    "queue_insert: queue corruption -- q_ptr->blink->flink != q_ptr"), \
    cma__assert_fail (					\
	    (_iqp_)->flink->blink == (_iqp_),		\
	    "queue_insert: queue corruption -- q_ptr->flink->blink != q_ptr"), \
    (_ie_)->blink		= (_iqp_)->blink,	\
    (_ie_)->flink		= (_iqp_),		\
    (_ie_)->blink->flink	= (_ie_),		\
    (_iqp_)->blink		= (_ie_))
#endif

/*
 * Insert an element in a queue following the specified item (or at head of
 * queue if _qp_ is the queue head).
 */
#if _CMA_PLATFORM_ == _CMA__VAX_VMS
# define cma__queue_insert_after(_iae_,_iaqp_)    (		\
    cma__assert_fail (					\
	    (_iae_)->flink == (_iae_), \
	    "queue_insert: queue corruption -- element flink not self"), \
    cma__assert_fail (					\
	    (_iae_)->blink == (_iae_),			\
	    "queue_insert: queue corruption -- element blink not self"), \
    cma__assert_fail (						\
	    (_iaqp_)->blink->flink == (_iaqp_),			\
	    "queue_insert_after: queue corruption -- q_ptr->blink->flink != q_ptr"), \
    cma__assert_fail (						\
	    (_iaqp_)->flink->blink == (_iaqp_),			\
	    "queue_insert_after: queue corruption -- q_ptr->flink->blink != q_ptr"), \
    _INSQUE (_iae_, _iaqp_))
#else
# define cma__queue_insert_after(_iae_,_iaqp_)    (		\
    cma__assert_fail (					\
	    (_iae_)->flink == (_iae_), \
	    "queue_insert: queue corruption -- element flink not self"), \
    cma__assert_fail (					\
	    (_iae_)->blink == (_iae_),			\
	    "queue_insert: queue corruption -- element blink not self"), \
    cma__assert_fail (						\
	    (_iaqp_)->blink->flink == (_iaqp_),			\
	    "queue_insert_after: queue corruption -- q_ptr->blink->flink != q_ptr"), \
    cma__assert_fail (						\
	    (_iaqp_)->flink->blink == (_iaqp_),			\
	    "queue_insert_after: queue corruption -- q_ptr->flink->blink != q_ptr"), \
    (_iae_)->flink		= (_iaqp_)->flink,		\
    (_iae_)->blink		= (_iaqp_),			\
    (_iaqp_)->flink->blink	= (_iae_),			\
    (_iaqp_)->flink		= (_iae_))
#endif

/*
 * Return the next item in a queue (or the first, if the address is of the
 * queue header)
 */
#define cma__queue_next(_ne_)    (			\
    cma__assert_fail (					\
	    (_ne_)->flink != 0,			\
	    "queue_next: queue corruption -- element flink is zero"), \
    cma__assert_fail (					\
	    (_ne_)->blink != 0,			\
	    "queue_next: queue corruption -- element blink is zero"), \
    cma__assert_fail (					\
	    (_ne_)->blink->flink == (_ne_),	\
	    "queue_next: queue corruption -- element->blink->flink != element"), \
    cma__assert_fail (					\
	    (_ne_)->flink->blink == (_ne_),	\
	    "queue_next: queue corruption -- element->flink->blink != element"), \
    (_ne_)->flink)

/*
 * Return the previous item in a queue (or the last, if the address is of the
 * queue header)
 */
#define cma__queue_previous(_pe_)    (		\
    cma__assert_fail (					\
	    (_pe_)->flink != 0,			\
	    "queue_previous: queue corruption -- element flink is zero"), \
    cma__assert_fail (					\
	    (_pe_)->blink != 0,			\
	    "queue_previous: queue corruption -- element blink is zero"), \
    cma__assert_fail (					\
	    (_pe_)->blink->flink == (_pe_),	\
	    "queue_previous: queue corruption -- element->blink->flink != element"), \
    cma__assert_fail (					\
	    (_pe_)->flink->blink == (_pe_),	\
	    "queue_previous: queue corruption -- element->flink->blink != element"), \
    (_pe_)->blink)

/*
 * Remove the specified item from a queue.
 *
 *	_e_	Address of element to remove
 *
 *	_o_	Variable to receive address of element (_t_ *)(_e_)
 *
 *	_t_	Type of *_o_
 */
#if _CMA_PLATFORM_ == _CMA__VAX_VMS
# define cma__queue_remove(_re_,_ro_,_rt_) (	\
    cma__assert_fail (				\
	    (_re_)->flink != 0,			\
	    "queue_remove: queue corruption -- element flink is zero"), \
    cma__assert_fail (				\
	    (_re_)->blink != 0,			\
	    "queue_remove: queue corruption -- element blink is zero"), \
    cma__assert_fail (				\
	    (_re_)->blink->flink == (_re_),	\
	    "queue_remove: queue corruption -- element->blink->flink != element"), \
    cma__assert_fail (				\
	    (_re_)->flink->blink == (_re_),	\
	    "queue_remove: queue corruption -- element->flink->blink != element"), \
    _REMQUE (_re_, (void **)&(_ro_)),		\
    cma__queue_zero((cma__t_queue *)(_ro_)))
#else
# define cma__queue_remove(_re_,_ro_,_rt_) (	\
    cma__assert_fail (				\
	    (_re_)->flink != 0,			\
	    "queue_remove: queue corruption -- element flink is zero"), \
    cma__assert_fail (				\
	    (_re_)->blink != 0,			\
	    "queue_remove: queue corruption -- element blink is zero"), \
    cma__assert_fail (				\
	    (_re_)->blink->flink == (_re_),	\
	    "queue_remove: queue corruption -- element->blink->flink != element"), \
    cma__assert_fail (				\
	    (_re_)->flink->blink == (_re_),	\
	    "queue_remove: queue corruption -- element->flink->blink != element"), \
    _ro_ = (_rt_ *)(_re_),				\
    (_re_)->flink->blink = (_re_)->blink,		\
    (_re_)->blink->flink = (_re_)->flink,		\
    cma__queue_zero((cma__t_queue *)(_ro_)))
#endif

/*
 * Initialize a queue element to "not-on-queue".  (This is defined so as to
 * be able to be used inside of other macros.)
 */
#ifndef NDEBUG
# define cma__queue_zero(_ze_)    ((_ze_)->flink = (_ze_)->blink = (_ze_))
#else
# define cma__queue_zero(_ze_)    0
#endif

/*
 * TYPEDEFS
 */

typedef struct CMA__T_QUEUE {
    struct CMA__T_QUEUE	*flink;		/* Forward link */
    struct CMA__T_QUEUE	*blink;		/* Backward link */
    } cma__t_queue;

/*
 *  GLOBAL DATA
 */

/*
 * INTERNAL INTERFACES
 */

#endif
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_QUEUE.H */
/*  *11    2-JUL-1993 14:38:11 BUTENHOF "Make empty q elements look like heads" */
/*  *10   20-APR-1993 09:00:37 KEANE "Fix queue_remove macro" */
/*  *9    16-APR-1993 13:05:02 BUTENHOF "Recast queue removal to use VAX builtins" */
/*  *8    23-SEP-1992 12:53:06 BUTENHOF "Fix macro arg names" */
/*  *7     2-SEP-1992 16:25:58 BUTENHOF "Add inline queue_dequeuei" */
/*  *6    15-MAY-1992 15:03:53 SCALES "Add additional consistency checks" */
/*  *5    11-JUN-1991 17:17:17 BUTENHOF "Add tracing" */
/*  *4    10-JUN-1991 19:55:11 SCALES "Convert to stream format for ULTRIX build" */
/*  *3    10-JUN-1991 19:21:20 BUTENHOF "Fix the sccs headers" */
/*  *2    10-JUN-1991 18:23:03 SCALES "Add sccs headers for Ultrix" */
/*  *1    12-DEC-1990 21:49:03 BUTENHOF "Atomic queues" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_QUEUE.H */
