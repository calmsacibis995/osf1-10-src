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
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: device.c,v $ $Revision: 4.4.3.2 $ (DEC) $Date: 1993/01/08 17:54:00 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * Copyright (c) 1988 Carnegie-Mellon University
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Device.c
 *
 * Modification History:
 *
 * 07-sep-91    Brian Harrigan
 *      Removed 30-sep changes, as they were moved to a .h file
 *
 * 30-AUG-91    Brian Harrigan
 *      Changed FUNNEL defs for RT_PREEMPT for the RT MPK
 *	(EFT) This Hack should be fixed in the .h files ...
*/
/*
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */
/*
 * File:	device.c
 * Purpose:
 *	Kernel device i/o interface
 */

#include <sys/secdefines.h>
#include <mach/kern_return.h>
#include <mach/mach_types.h>
#include <mach/device_types.h>

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/user.h>
#include <sys/buf.h>
#include <sys/conf.h>
#include <kern/lock.h>
#include <kern/task.h>
#include <mach/port.h>
#include <kern/kern_port.h>
#include <kern/parallel.h>
#define DEV_IO_BAD_DEVICE -2
#define DEV_IO_ERROR	  -3
#include <mach/error.h>

#include <vm/vm_kern.h>
#include <vm/vm_map.h>
#include <mach/vm_prot.h>
#if	SEC_BASE
#include <sys/security.h>
#endif

extern vm_map_t ipc_soft_map;
extern vm_map_t kernel_map;
#define kern_device_io_map	kernel_map	/* XXX allocate submap */
#ifndef	private
#define private static
#endif	

/*
 *	state variables
 */

private task_t		device_task;
private kern_port_t	device_server_port;
private port_set_name_t	device_enabled_set;
decl_simple_lock_data(private,dev_lock)

#ifdef	block_write
typedef struct dev_q {
	queue_chain_t	qc;
	struct buf q_bp;
} * dev_q_t;

private dev_q_t	dev_block_q;

zone_t	dev_io_zone;
#endif	/* block_write */

#ifdef	block_write
int block_write_done( bp )
struct buf * bp;
{
	thread_wakeup((vm_offset_t)&dev_block_q);
}
#endif	/* block_write */

kern_return_t block_write( port, device, block_number, block_size, block )
	port_t		port;
	dev_t		device;
	int		block_number;
	int		block_size;
	io_buf_t	block;
{
#ifdef	lint
	port++; device++; block_number++; block++; block_size++;
#endif	
	return( KERN_SUCCESS );
}


kern_return_t block_write_async( port, device, block_number, block_size, block )
	port_t		port;
	dev_t		device;
	int		block_number;
	int		block_size;
	io_buf_t	block;
{
#ifdef	block_write
	struct buf 		* bp;
	kern_return_t		result;
	int err;
	static struct buf	bufinit = {
		B_WRITE,
	};
#endif	

#ifdef	lint
	port++; device++; block_number++; block++; block_size++;
#endif	

#ifdef	block_write

	buf = bufinit;

	if( major(device) > nblkdev )
		return( DEV_IO_BAD_DEVICE );

	/*
	 *  wire down our copy of user space buffer
	 */
	if( vm_map_pageable(kern_device_io_map, block, block+block_size,
	    VM_PROT_READ|VM_PROT_WRITE) != KERN_SUCCESS)
		panic("block_read: can't wire buffer\n");

	/* drag a new buf header out of the buffer queue */
	q = zalloc( dev_io_zone );
	q->bp = bufinit;

	/* save the old block in a queue to be reused */
	simple_lock( &dev_lock );
	queue_enter(&dev_block_q, q, dev_q_t, links);

	bp = &(q->q_bp);
	bp->b_flags = B_WRITE;

	bp->b_bcount = bp->b_bufsize = block_size;
	bp->b_dev = device;
	bp->b_blkno = block_number;
	BDEVSW_STRATEGY(major(device),bp, err);	/* XXX */


	if ( bp->b_error != 0 ) {
		result = unix_err(bp->b_error);
		thread_wakeup((vm_offset_t)&dev_block_q);/* wake up the buf reaper */
	} else
		result = KERN_SUCCESS;

	simple_unlock( &dev_lock );

	return( result );
#else	/* block_write */
	return( KERN_SUCCESS );
#endif	/* block_write */
}


kern_return_t block_read( port, device, block_number, block_size, block, blockCnt )
	port_t		port;
	dev_t		device;
	daddr_t		block_number;
	int		block_size;
	io_buf_ptr_t	block;
	int		* blockCnt;
{
	struct buf		local_buf;

	caddr_t			addr;
	vm_address_t		_addr;
	kern_return_t		err;
	int err1;
	static struct buf	bufinit = {
		B_READ,
	};

#ifdef	lint
	port++; device++; block_number++; block++; block_size++; blockCnt++;
#endif	

	if( major(device) > nblkdev )
		return( DEV_IO_BAD_DEVICE );

	local_buf = bufinit;

	/*
	 *  allocate and wire down a buffer
	 */
	if ( (err = vm_allocate(
			kern_device_io_map,
			&_addr,
			(vm_size_t) block_size,
			TRUE))
	    != KERN_SUCCESS) {
		printf("block_read: cannot allocate a buffer!");
		return( err );
	}
	addr = (caddr_t) _addr;

	if( (err = vm_map_pageable(kern_device_io_map, _addr,
	    _addr+block_size, VM_PROT_READ|VM_PROT_WRITE)) != KERN_SUCCESS) {
		printf("block_read: can't wire buffer\n");
		return( err );
	}

		
	local_buf.b_un.b_addr = addr;

	local_buf.b_bcount = local_buf.b_bufsize = block_size;
	local_buf.b_dev = device;
	local_buf.b_blkno = block_number;
	BDEVSW_STRATEGY(major(device), &local_buf, err1);
	if( local_buf.b_error != 0 )
		return( unix_err(local_buf.b_error) );

	biowait(&local_buf);

	if ( local_buf.b_error != 0 )
		return( unix_err(local_buf.b_error) );

	*blockCnt = local_buf.b_bcount - local_buf.b_resid;

	/* 
	 * move it into the ipc soft map so it will get magically moved
	 * back into the user's map on exit from the message.
	 *
	 * XXX this is entirely temporary
	 */
	(void) vm_map_copyin(
			kern_device_io_map,
			_addr,
			(vm_size_t) *blockCnt,
			TRUE,
			(vm_map_copy_t *) block);

	return( KERN_SUCCESS );
}


kern_return_t block_read_ahead( port, device, block_number, 
			       block_size, ra_block_number, ra_block_size,
			       block )
	port_t		port;
	dev_t		device;
	daddr_t		block_number;
	io_buf_ptr_t	block;
	int		block_size;
	daddr_t		ra_block_number;
	daddr_t		ra_block_size;
{
	struct buf		local_buf;
	struct buf *		rabp;
	caddr_t			addr;
	vm_address_t		_addr;
	int err;
	static struct buf	bufinit = {
		B_READ,
	};

#ifdef	lint
	port++;
#endif	

	local_buf = bufinit;

	/*
	 *  allocate and wire some data in user space
	 */

#ifdef	block_write
	/* keep a cache of blocks allocated by block_write */
#endif	/* block_write */

	if (vm_allocate(
		kern_device_io_map,
		&_addr,
		(vm_size_t) block_size,
		TRUE)
	    != KERN_SUCCESS)
		panic("block_read_ahead: cannot allocate a buffer!");
	if( vm_map_pageable(kern_device_io_map, _addr, _addr+block_size,
	    VM_PROT_READ|VM_PROT_WRITE) != KERN_SUCCESS)
		panic("block_read_ahead: can't wire buffer\n");
	addr = (caddr_t) _addr;

	*block = addr;
	local_buf.b_un.b_addr = (io_buf_t)addr;
	local_buf.b_bcount = local_buf.b_bufsize = block_size;
	local_buf.b_dev = device;
	local_buf.b_blkno = block_number;
	BDEVSW_STRATEGY(major(device), &local_buf, err);
	if( local_buf.b_error != 0 )
		return( DEV_IO_ERROR );

	/*
	 * If there's a read-ahead block, start i/o
	 * on it also (as above).
	 */
	if (ra_block_number && !incore(device, ra_block_number)) {
		rabp = getblk(device, ra_block_number, ra_block_size);
		if (event_posted(&rabp->b_iocomplete))
			brelse(rabp);
		else {
			rabp->b_flags |= B_READ|B_ASYNC;
			if (rabp->b_bcount > rabp->b_bufsize)
				panic("breadrabp");
			BDEVSW_STRATEGY(major(device), rabp, err);
		}
	}
	
	biowait(&local_buf);
	if( local_buf.b_error != 0 )
		return( DEV_IO_ERROR );

	return( KERN_SUCCESS );

}

#ifdef	block_write
private 
void buf_reaper()
{
	dev_q_t q;

	/*
	 *  a separate thread runs through here and deallocates buffers and frees
	 *  the buf structure associated with buffers used for writes
	 */
	while (1) {

		simple_lock( &dev_lock );
		thread_sleep((vm_offset_t)&dev_io_q,
					simple_lock_addr(dev_lock), FALSE );

		simple_lock( &dev_lock );
		if ( ! queue_empty(dev_io_q) ) {
			q = queue_first(dev_io_q);
			do {
				if ( q->q_bp->b_flags & (B_DONE|B_ERROR) ) {
					vm_deallocate( kern_dev_io_map, q->bp->b_un.addr, q->bp->b_bufsize );
					queue_remove( q, q, dev_io_q_t, qc );
					zfree( q );
				}
				q = queue_next(q);
			} while( ! queue_end(q, dev_io_q) );
		}
		simple_unlock( &dev_lock );
	}
}
#endif	/* block_write */

/*
 *	task_get_io_port:  give the device port to user space (no
 *	restrictions as yet)
 */
task_get_io_port( task, port )
	task_t		task;
	kern_port_t	* port;
{
	int notpriv;
#ifdef	lint
	task++;
#endif	

	cr_threadinit(current_thread());
#if	SEC_BASE
	notpriv = (!privileged(SEC_ALLOWDACACCESS, 0));
#else
	notpriv = suser(u.u_cred, &u.u_acflag);
#endif
	if (notpriv)
		return(KERN_FAILURE);
	port_reference( device_server_port );
	*port = device_server_port;
	return( KERN_SUCCESS );
}


void		device_init(task)
	task_t		task;
{
	port_name_t port;

	device_task = task;

	if (port_allocate(device_task, &port) != KERN_SUCCESS)
		panic("device_init: device server port allocate");

	if (port_set_allocate(device_task, &device_enabled_set) != KERN_SUCCESS)
		panic("device_init: device enabled set allocate");

	if (port_set_add(device_task, device_enabled_set, port) != KERN_SUCCESS)
		panic("device_init: device server port enable");

	(void) object_copyin(device_task, port,
			     MSG_TYPE_PORT, FALSE,
			     (kern_obj_t *) &device_server_port);

	simple_lock_init(&dev_lock);
#ifdef	block_write
	dev_io_zone = zinit( sizeof(dev_q_t), DEV_IO_ZMAX, DEV_IO_ZMAX, "dev_io zone" );

	/*
	 * start up the task that will reclaim bufs allocated when writing
	 */
	(void)kernel_thread( task, buf_reaper );
#endif	
		      
}

/*
 *  mach_user_internal.h is needed to redefine server_loop's idea of
 *  how functions should behave for use inside the kernel
 */
#include <mach/mach_user_internal.h>

/*
 *	Deallocate the page structures allocated for a device
 */
void		device_terminate(port)
	port_t		port;
{
#ifdef	lint
	port++;
#endif	
	printf( "device_terminate!\n" );
}


/*
 *  include some canned code to perform our server function
 */

#define SERVER_LOOP		device_server_loop
#define SERVER_NAME		"device_server"
#define SERVER_DISPATCH(in,out)	device_server(in,out)
#define TERMINATE_FUNCTION	device_terminate

#include <kern/server_loop.c>

void device_loop()
{
	extern void task_name();

	task_name("device server");
	SERVER_LOOP(device_enabled_set, TRUE);
}
