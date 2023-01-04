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
 *	@(#)$RCSfile: cma_ux.h,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/04/13 21:35:11 $
 */
/*
 *  FACILITY:
 *
 *	CMA services
 *
 *  ABSTRACT:
 *
 *	Header file for unix system call wrapper routines
 *
 *  AUTHORS:
 *
 *	Hans Oser
 *
 *  CREATION DATE:
 *
 *	19 September 1989
 *
 *  MODIFIED BY:
 *
 *	Dave Butenhof
 *	Paul Curtin
 *	Webb Scales
 */


#ifndef CMA_UX
#define CMA_UX

/*
 *  INCLUDE FILES
 */

/*
 * CONSTANTS AND MACROS
 */

#if !_CMA_THREAD_SYNC_IO_
# if !defined(_CMA_NOWRAPPERS_) && _CMA_UNIPROCESSOR_
/*
 * U*ix I/O System Call Wrappers
 */
#  define accept	cma_accept
#  define close		cma_close
#  define connect	cma_connect
#  define creat		cma_creat
#  define dup		cma_dup
#  define dup2		cma_dup2
#  define fcntl		cma_fcntl
#  define open		cma_open
#  define pipe		cma_pipe
#  define recv		cma_recv
#  define recvmsg	cma_recvmsg
#  define recvfrom	cma_recvfrom
#  define read		cma_read
#  define readv		cma_readv
#  define select	cma_select
#  define send		cma_send
#  define sendmsg	cma_sendmsg
#  define sendto	cma_sendto
#  define socket	cma_socket
#  define socketpair	cma_socketpair
#  define write		cma_write
#  define writev	cma_writev
# endif
#endif

#if !_CMA_UNIPROCESSOR_ && !defined(_CMA_NOWRAPPERS_)
# define cma_import_fd(fd)
# define cma_unimport_fd(fd)
#endif

/*
 * U*ix process control wrappers
 */
#if !defined(_CMA_NOWRAPPERS_) && (_CMA_HARDWARE_ != _CMA__ALPHA || _CMA_OSIMPL_ != _CMA__OS_OSF)
# define fork           cma_fork
#endif

#if !defined(_CMA_NOWRAPPERS_)
# define atfork         cma_atfork
#endif

/*
 * TYPEDEFS
 */

typedef void (*cma_t_fork_rtn) _CMA_PROTOTYPE_ ((
        cma_t_address   arg));

/*
 *  GLOBAL DATA
 */

/*
 * INTERFACES
 */

# if _CMA_HARDWARE_ != _CMA__HPPA
extern void
cma_atfork _CMA_PROTOTYPE_ ((
        cma_t_address user_state,
        cma_t_fork_rtn pre_fork,
        cma_t_fork_rtn parent_fork,
        cma_t_fork_rtn child_fork));
# else
extern void
atfork _CMA_PROTOTYPE_ ((
        cma_t_address user_state,
        cma_t_fork_rtn pre_fork,
        cma_t_fork_rtn parent_fork,
        cma_t_fork_rtn child_fork));
# endif

# if _CMA_UNIPROCESSOR_
extern void
cma_import_fd _CMA_PROTOTYPE_ ((
	int		fd));

extern void
cma_unimport_fd _CMA_PROTOTYPE_ ((
	int		fd));
# endif

#endif
