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
static char *rcsid = "@(#)$RCSfile: fbus_loadable.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1993/07/22 18:07:12 $";
#endif

#include <sys/errno.h>
#include <sys/map.h>
#include <io/dec/fbus/fbus_loadable.h>
#include <io/common/devdriver.h>

extern stray();
extern kern_return_t allocvec();
extern u_short vecoffset();
extern void intrsetvec();

#define Fbus_Dprintf 

/* Note that locking is handled in handler.c, before these functions
 * are called and so there is no need to do SMP locking here.
 */

	/* Add fbus interrupt handler. */
	/* Return bus specific key. */
ihandler_id_t
fbus_handler_add(handler)
ihandler_t *handler;
{
	struct fbus_intr_info *iptr;
	struct bus *bus_ptr;
	vm_offset_t vec_addr;
	struct fbus_handler_info *saved_info;

	Fbus_Dprintf("fbus_handler_add entered\n");
	if(allocvec(1,&vec_addr) != KERN_SUCCESS) {
		return((ihandler_id_t)-1);
	}
	saved_info = (struct fbus_handler_info *)kalloc(sizeof(struct fbus_handler_info));
	if(saved_info == (struct fbus_handler_info *)0) {
		return((ihandler_id_t)-1);
	}
	iptr = (struct fbus_intr_info *)handler->ih_bus_info;
	saved_info->vec_addr = vec_addr;
	saved_info->vec_offset = vecoffset((vm_offset_t *)vec_addr);
	saved_info->intr = iptr->intr;
	saved_info->param = iptr->param;
	saved_info->self_reference = saved_info;
	iptr->vector_offset = saved_info->vec_offset; /* return offset for device to use */
	Fbus_Dprintf(" vec_addr=%x offset=%x intr=%x param=%d saved_info=%lx\n",
		saved_info->vec_addr, saved_info->vec_offset, saved_info->intr, saved_info->param,
		saved_info->self_reference);
	return((ihandler_id_t)saved_info);
}

int
fbus_handler_del(id)
ihandler_id_t id;
{
	struct fbus_handler_info *saved_info = (struct fbus_handler_info *)id;

	Fbus_Dprintf("fbus_handler_del entered - id = %x\n", (int)id);
		/* Sanity check */
	if(!saved_info || saved_info->self_reference != saved_info) {
		return(-1);
	}
	intrsetvec(saved_info->vec_offset, &stray, saved_info->vec_offset);
	bzero((char *)saved_info, sizeof(struct fbus_handler_info));
	kfree(saved_info, sizeof(struct fbus_handler_info));
	return(0);
}

		/* Note: The device must do device specific actions.  */
int
fbus_handler_enable(id)
ihandler_id_t id;
{
	struct fbus_handler_info *saved_info = (struct fbus_handler_info *)id;

	Fbus_Dprintf("fbus_handler_enable entered - id = %x\n", (int)id);
		/* Sanity check */
	if(!saved_info || saved_info->self_reference != saved_info) {
		return(-1);
	}
	intrsetvec(saved_info->vec_offset, saved_info->intr, saved_info->param);
#ifdef FBUS_DEBUG_PRINTVEC
	Fbus_Dprintf("\n",fbus_handler_print_scb(saved_info->vec_addr));
#endif
	return(0);
}

int
fbus_handler_disable(id)
ihandler_id_t id;
{
	struct fbus_handler_info *saved_info = (struct fbus_handler_info *)id;

	Fbus_Dprintf("fbus_handler_disable entered - id = %x\n", (int)id);
		/* Sanity check */
	if(!saved_info || saved_info->self_reference != saved_info) {
		return(-1);
	}
	intrsetvec(saved_info->vec_offset, &stray, saved_info->vec_offset );
#ifdef FBUS_DEBUG_PRINTVEC
	Fbus_Dprintf("\n",fbus_handler_print_scb(saved_info->vec_addr));
#endif
	return(0);
}

#ifdef FBUS_DEBUG_PRINTVEC
fbus_handler_print_scb(vec_addr)
vm_offset_t *vec_addr;
{
	struct fbus_hp {u_long a;u_long b;} *v = (struct fbus_hp *)vec_addr;

	printf("intr function= %x, param = %x\n", v->a, v->b);
}
#endif
