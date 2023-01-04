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
 * @(#)$RCSfile: handler.h,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/04/14 16:02:42 $
 */

#ifndef _SYS_HANDLER_H_
#define _SYS_HANDLER_H_

#include <kern/lock.h>
#include <sys/types.h>

/* 
 * id type for opaque
 * use with machine dependent id functions:
 * handler_del, handler_enable, handler_disable.
 *
 * This field is used to represent the specific interrupt to be acted upon
 * as a result of handler_{del, enable, disable} calls.  The value of this
 * field is set from the handler_add call.  
 */
typedef char * ihandler_id_t;               

/*
 * The define for ihandler_id_t is needed prior to inclusion of devdriver.h.
 */
#include <io/common/devdriver.h>

/* 
 * Interrupt handler structure.
 *
 * This model of interrupt dispatching is based on the BUS as being the
 * means of interrupt dispatching for all drivers.  For this reason all
 * of the information needed to register an interrupt is considered to be
 * bus specific rather than attempting to represent all possible permutations
 * within the ihandler_t structure itself.  
 *
 * The original fields of the OSF version of this structure have been removed
 * to prevent against their accidental usage.  This was deemed perferable to
 * leaving "stale" fields around to allow a driver to compile but not actually
 * function. 
 */
typedef struct ihandler {
	ihandler_id_t	ih_id;			/* unique id */
        struct bus      *ih_bus;                /* Driver's bus struct */
	char 		*ih_bus_info;		/* Bus registration info */
} ihandler_t;

/*
 * Handler key structure.  
 *
 * This structure is allocated when a handler is
 * added via handler_add.  The purpose of this structure is to contain all
 * the information needed to dispatch off to the bus specific implementations
 * of handler_{del,enable,disable}.  To do this the main pieces of information
 * are the bus pointer which is used to find pointers to the bus specific
 * of handler_{del,enable,disable} routines and the bus_id_t field which 
 * allows the bus to identify which particular interrupt to act upon.
 */
struct handler_key {
	struct handler_key	*next;	   /* Pointer to next handler entry */
	struct handler_key	*prev;	   /* Pointer to prev handler entry */
	struct bus		*bus;	   /* Pointer to bus structure      */
	ihandler_id_t		bus_id_t;  /* Bus specific unique key       */
        unsigned int            state;     /* state info 		    */
	ihandler_id_t		key;	   /* Unique key for this entry     */
	lock_data_t		lock;	   /* MP lock			    */
};

/*
 * Allowable values of the "state" field of the handler_key structure:
 */
#define IH_STATE_ENABLED        0x01                    /* enable/disable */

#endif /* _SYS_HANDLER_H_ */

