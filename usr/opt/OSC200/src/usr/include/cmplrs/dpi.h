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
/*   "@(#)$RCSfile: dpi.h,v $ $Revision: 1.1.13.3 $ (DEC) $Date: 1993/12/15 22:13:14 $";
*/
/*
 * $Header: /usr/sde/osf1/rcs/os/src/usr/include/cmplrs/dpi.h,v 1.1.13.3 1993/12/15 22:13:14 Thomas_Peterson Exp $
 */
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

#ifndef DPI_H
#define DPI_H
/* this file contains externally visible declarations for the 
 *	debug process interface (dpi) library.
 */

/* KNOWN PREFIXES:
 *
 *	DPI_BLK_	block access size for layer
 *	DPI_D_		debug flags
 *	DPI_C_		constants
 *	DPI_L_		layers
 *	DPI_Q_		query strings
 *	DPI_S_		states
 *	DPI_V_		version
 *	DPI_		results
 */

#define DPI_V_VERSION	4	/* hopefully for full backward compat */

/* well know results */
#define DPI_FAIL	0
#define DPI_SUCCESS	1

/* block sizes allowed through the different layers 
 *	layer use masks of these to make known their
 *	abilities. Specific requests for reads and write use single
 *	values of these for hints where applicable.
 */

#define DPI_BLK_FAIL	DPI_FAIL/* illegal value */
#define DPI_BLK_CHAR	1	/* can read a byte block */
#define DPI_BLK_BYTE	1	/* can read a byte block */
#define DPI_BLK_SHORT	2	/* can read a short block */
#define DPI_BLK_LONG	4	/* can read a long block */
#define DPI_BLK_DOUBLE	8	/* can read a double block */
#define DPI_BLK_ANY	0x80000000	/* can read any size block */

/* test to see if bit is set in flag */
#define DPI_BLK_TEST(mask,block_size) ((mask&block_size) || (mask&DPI_BLK_ANY))

/* debug flags */
#define DPI_D_NONE		0
#define DPI_D_LAYER		1	/* debug the layer to whatever */
#define DPI_D_COMM		2	/* debug low level comm */
#define DPI_D_PROTO		4	/* debug high level protocol */
#define DPI_D_INIT		8	/* debugged something during this run */
#define DPI_D_DPI		0x10	/* debug dpi interface */

/* query constant, specific to layers but must be supported
 *	by clients which wish to use those dpi facilities
 *	when dpi calls the query routine (which is the argument for
 *	dpi_create) with one of these constants, it either expects back DPI_FAIL
 *	or some address or address-sized constant which is meaning to the
 *	layer. See the README for detailed comments for each constant
 *
 *
 *      define constant     	constant	layer		return
 *	_______________	    	________	---------	type
 *      			 		         	------
 */
#define DPI_Q_GUEST	    	0		/* DPI_L_GUEST	init_routine */
#define DPI_Q_CORE_NAME	    	1		/* DPI_L_CORE	dpi_string */
#define DPI_Q_PID	    	2		/* DPI_L_KERNEL	dpi_pid */
#define DPI_Q_IMAGE_FILE    	3		/* DPI_L_IMAGE	dpi_string */
#define DPI_Q_OBJ_READ		4		/* DPI_L_OBJ  dpi_unsigned_32 */
#define DPI_Q_PATCH		5		/* DPI_L_OBJ  dpi_unsigned_32 */
#define DPI_Q_AFTER_EXEC	6		/* _dpi_unix  routine(pid) */
#define DPI_Q_PROGRAM		7		/* SABLE	dpi_string */
#define DPI_Q_USE_SOCKETS	8		/* SABLE	dpi_flag */
#define DPI_Q_KDEBUG_HOST	9		/* KDEBUG	dpi_string */
#define DPI_Q_KDEBUG_LINE	10		/* KDEBUG	dpi_string */
#define DPI_Q_KDEBUG_DBGTTY	11		/* KDEBUG	dpi_string */
#define DPI_Q_UNRESERVED	8096	/* from here, guest queries can ovrlp */

/* miscellanious constants */
#define DPI_C_NO_DPI	0	/* bogus dpi */

/* layers */
typedef enum dpi_layer {
	DPI_L_DEFAULT,		/* when used, we'll use preset value */
	DPI_L_PTRACE,		/* ptrace layer */
	DPI_L_SPROC,		/* /proc */
	DPI_L_CORE,		/* debug core file */
	DPI_L_IMAGE,		/* image out on disk, needs a another layer */
	DPI_L_KERNELCORE,	/* debug kernel core files */
	DPI_L_SABLE,		/* sable simulator startup */
	DPI_L_KDEBUG,		/* kdebug startup */
	DPI_L_GUEST,		/* guest layer, must supply init by query */
	DPI_L_PIXIE,		/* pixie profiler map layer */
	DPI_L_PIO,		/* partial io layer, size/align issues */
	DPI_L_OBJ,		/* go to object when you can */
	DPI_L_BREAK,		/* breakpoint manager */
	DPI_L_MCACHE,		/* Memory cache */
	DPI_L_RCACHE,		/* register cache */
	DPI_L_TLIB,		/* thread library */
	DPI_L_FILE,		/* file access on read */
	DPI_L_MACHINE,		/* machine specific layer */
	DPI_L_KERNEL,		/* vtop mappings, and registers from pcbs */
	DPI_L_CROSS,		/* cross byte sex debugging */
	DPI_L_DEBUG,		/* debug layer */
	DPI_L_UNSUPPORTED	/* bottom layer */
} dpi_layer;


/* states */
enum dpi_state {
	DPI_S_NOTHERE=DPI_FAIL,	/* not registered */
	DPI_S_INIT,		/* registered */
	DPI_S_STOPPED,		/* known to not be running */
	DPI_S_EXITED,		/* known to have exited */
	DPI_S_RUNNING		/* known to be running */
};

/* argument/return types */


/* base types */

typedef unsigned long		dpi_unsigned_long;
typedef long			dpi_signed_long;
typedef unsigned int		dpi_unsigned_int;
typedef int			dpi_signed_int;
typedef signed char		dpi_signed_8;
typedef unsigned char		dpi_unsigned_8;

typedef int			dpi_fd;		/* file descriptors */
typedef dpi_signed_8		*dpi_data;	/* data buffers */
typedef dpi_signed_8		*dpi_string;	/* string arguments */
typedef dpi_signed_int		dpi_signal;	/* signals to pend and clear */
typedef dpi_signed_int		dpi_size;	/* bytes in dpi_data */
typedef dpi_signed_int		dpi_pid;	/* process id */
typedef dpi_signed_long		dpi_tid;	/* thread id */
typedef dpi_signed_int		dpi_count;	/* count of tids */
typedef dpi_tid			*dpi_tid_list;	/* thread id list */
typedef dpi_unsigned_long	dpi_address;	/* client addresses */
typedef dpi_unsigned_long	dpi_register_no;/* register identifier */
typedef unsigned int		dpi_flag;	/* binary or of various bits */
typedef dpi_unsigned_long	dpi_result;	/* many routines return this */
typedef dpi_unsigned_long	dpi_query_constant;/* query callback arg */
typedef dpi_signed_int		dpi_int;	/* Misc integer */

typedef dpi_string		*dpi_str_vec;	/* vector of string arguments */
typedef enum dpi_state		dpi_state;	/* state of this process */
typedef struct dpi_info		*dpi_info;	/* general info for process */
typedef struct dpi		*dpi;		/* function vector */
typedef struct obj_list		*dpi_objlist;	/* libmld dpi */
typedef dpi_result		(*dpi_query)	/* callback for layer
						 * specific info
						 */
		(dpi_query_constant	query_constant,
		 dpi_data		data);

/* dpi and functions were not always the same, and could change again
 *	so I'll maintain both.
 */

/* add new dpi to the end */

struct dpi {

    dpi	_next;		/* links layers must be first */

    dpi	_prev;		/* links layers */

    dpi_layer		_layer;		/* layer tag */

    dpi_info		_info;		/* points to base structure */

    dpi_data		_data;		/* layer specific data */

    dpi_query		_query;		/* private callback */

    dpi_string		_string;	/* informational string for the layer */

    dpi_result	(*catch_signal)(	/* catch signal for process */
		    dpi			dpi,
		    dpi_signal		sig);

    dpi_result	(*deliver_signal)(	/* deliver current signal */
		    dpi			dpi,
		    dpi_signal		sig);

    dpi_result	(*destroy) (		/* destructor for layer */
		    dpi			dpi);

    dpi_flag	(*get_block_access)(	/* return layer supported block size */
		    dpi			dpi);

    dpi_int	(*get_exitval)(		/* return process exit value */
		    dpi			dpi);

    dpi_result	(*get_fd)(		/* return file desc assoc with proc */
		    dpi			dpi,
		    dpi_fd		*fd);

    dpi_signal	(*get_last_signal)(	/* return last signal recieved */
		    dpi			dpi);

    dpi_string	(*get_string)(		/* return string assoc with layer */
		    dpi			dpi);

    dpi_state	(*get_state)(		/* return process state */
		    dpi			dpi);

    dpi_pid	(*get_pid)(		/* return process id */
		    dpi			dpi);

    dpi_tid	(*get_tid)(		/* return last stopped thread id */
		    dpi			dpi);

    dpi_count	(*get_tid_count)(	/* how many threads in process */
		    dpi			dpi);

    dpi_result	(*get_tid_list)(	/* return list of tids */
		    dpi			dpi,
		    dpi_count		max_tids,	/* get_tid_count */
		    dpi_tid_list	tid_list);	/* size is max_tids */

    dpi_result	(*ignore_signal)(	/* ignore signal for process */
		    dpi			dpi,
		    dpi_signal		sig);

    dpi_result	(*read)(		/* read data from target */
		    dpi			dpi,
		    dpi_address		address,
		    dpi_data		dest,
		    dpi_size		size,
		    dpi_flag		block_access);

    dpi_result	(*set_pid)(		/* set process id */
		    dpi			dpi,
		    dpi_pid		pid);

    dpi_result	(*start)(		/* start target */
		    dpi			dpi,
		    dpi_str_vec		argv,
		    dpi_str_vec		envp,
		    dpi_str_vec		files,
		    dpi_count		nfiles);

    dpi_result	(*stop)(		/* stop process */
		    dpi			dpi);

    dpi_result	(*terminate)(		/* terminate process */
		    dpi			dpi);

    dpi_result	(*thread_catch_signal)(	/* catch signal for thread */
		    dpi			dpi,
		    dpi_count		thread_count,
		    dpi_tid_list	tid_list,
		    dpi_signal		sig);

    dpi_result	(*thread_cont)(		/* continue listed threads */
		    dpi			dpi,
		    dpi_count		thread_count,
		    dpi_tid_list	tid_list);

    dpi_result	(*thread_cont_all_but)(	/* continue all but listed threads */
		    dpi			dpi,
		    dpi_count		thread_count,
		    dpi_tid_list	tid_list);

    dpi_state	(*thread_get_state)(	/* get thread state */
		    dpi			dpi,
		    dpi_tid		tid);

    dpi_result	(*thread_ignore_signal)(/* ignore signal for thread */
		    dpi			dpi,
		    dpi_count		thread_count,
		    dpi_tid_list	tid_list,
		    dpi_signal		sig);

    dpi_result	(*thread_read_reg)(	/* read registers for thread */
		    dpi			dpi,
		    dpi_tid		tid,
		    dpi_register_no	reg,
		    dpi_data		value);

    dpi_result	(*thread_step)(		/* step a thread */
		    dpi			dpi,
		    dpi_tid		tid);

    dpi_result	(*thread_terminate)(	/* terminate a thread */
		    dpi			dpi,
		    dpi_count		count,
		    dpi_tid_list	list);

    dpi_result	(*thread_write_reg)(	/* write registers for thread */
		    dpi			dpi,
		    dpi_tid		tid,
		    dpi_register_no	reg,
		    dpi_data		data);

    dpi_result	(*write)(		/* write data to process */
		    dpi			dpi,
		    dpi_address		address,
		    dpi_data		src,
		    dpi_size		size,
		    dpi_flag		block_access);

    dpi_result	(*wait)(		/* wait for a thread to stop */
		    dpi			dpi);
};

/* externally visible routines */

#ifdef __cplusplus
extern "C" {
#endif
extern dpi	
dpi_add_layer(				/* add a layer to process */
    dpi			dpi,
    dpi_layer		layer,
    dpi_query		query)
;
extern dpi	
dpi_change_main_layer(			/* change the interface for process */
    dpi			dpi,
    dpi_layer		layer,
    dpi_query		query)
;
extern dpi_result
dpi_init()				/* to be called at start of prog */
;

extern dpi
dpi_create(				/* create a process, return dpi */
    dpi_string		name,		/* object file name */
    dpi_layer		base_layer,	/* base layer */
    dpi_objlist		*objlist,	/* libmld objlist pointer */
    dpi_query		query)		/* query for base layer */
;
extern dpi
dpi_debug_set(				/* set debug flags to print stuff */
    dpi			dpi,
    dpi_flag		flag,
    dpi_flag		on_off)
;
#ifdef __cplusplus
}
#endif
#endif /* DPI_H */
