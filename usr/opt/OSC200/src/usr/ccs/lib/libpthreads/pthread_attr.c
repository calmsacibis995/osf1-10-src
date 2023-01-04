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
static char	*sccsid = "@(#)$RCSfile: pthread_attr.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 04:13:00 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

#ifndef	lint
#endif	not lint

/*
 * File: pthread_attr.c
 *
 * Support for thread attribute objects. Currently only the stack size
 * attribute is supported. The remaining attibutes are all scheduling.
 */

#include <pthread.h>
#include <errno.h>
#include "internal.h"

/*
 * Imported Data
 */
extern int	pthread_default_stack_size;

/*
 * Global Data
 */
pthread_attr_t		pthread_attr_default;

/*
 * Function:
 *	pthread_attr_init
 *
 * Description:
 *      This function is called from pthread_init() but after the
 *	call to stack_init() and is used to initialize the default
 *	attribute structure.
 *	
 */
void
pthread_attr_startup()
{
	pthread_attr_default = (pthread_attr_t)malloc(sizeof(struct pthread_attr));
	pthread_attr_default->stacksize = pthread_default_stack_size;
	pthread_attr_default->flags = ATTRIBUTE_VALID;
}

/*
 * Function:
 *	pthread_attr_create
 *
 * Parameters:
 *	attr - a pointer to the attribute structure to be created
 *
 * Return value:
 *	0	Success
 *	-1	the pointer to the attribute was invalid (EINVAL)
 *
 * Description:
 *	The structure is created by copying the default attribute structure
 *	into the new attribute.
 *
 */
int
pthread_attr_create(pthread_attr_t *attr)
{
	if (attr == NULL) {
		set_errno(EINVAL);
		return(-1);
	}
	*attr = (pthread_attr_t)malloc(sizeof(struct pthread_attr));
	**attr = *pthread_attr_default;
	return(0);
}

/*
 * Function:
 *	pthread_attr_delete
 *
 * Parameters:
 *	attr - a pointer to the attribute structure to be deleted
 *
 * Return value:
 *	0	Success
 *	-1	the pointer to the attribute was invalid (EINVAL)
 *		the attribute was invalid (EINVAL)
 *		the attribute was the default attribute (EINVAL)
 *
 * Description:
 *	attributes are deleted by marking them as invalid. No storage
 *	needs to be reclaimed.
 */
int
pthread_attr_delete(pthread_attr_t *attr)
{
	if ((attr == NULL) || (*attr == NO_ATTRIBUTE) ||
	    (*attr == pthread_attr_default) ||
	    !((*attr)->flags&ATTRIBUTE_VALID)) {
		set_errno(EINVAL);
		return(-1);
	}
	(*attr)->flags &= ~ATTRIBUTE_VALID;;
	free(*attr);
	*attr = NO_ATTRIBUTE;
	return(0);
}

/*
 * Function:
 *	pthread_attr_setstacksize
 *
 * Parameters:
 *	attr - a pointer to the attribute to be altered
 *	newsize - the size of the stack to be created when using this attribute
 *
 * Return value:
 *	0	Success
 *	-1	the pointer to the attribute was invalid (EINVAL)
 *		the attribute was invalid (EINVAL)
 *		the attribute was the default attribute (EINVAL)
 *
 * Description:
 *	The stacksize is rounded up to the nearest page so we don't
 *	get silly numbers. The value is a minimum value so when the
 *	stack is allocated the thread may get more than this.
 */
int
pthread_attr_setstacksize(pthread_attr_t *attr, long newsize)
{
	if ((attr == NULL) || (*attr == NO_ATTRIBUTE) ||
	    (*attr == pthread_attr_default) ||
	    !((*attr)->flags&ATTRIBUTE_VALID)) {
		set_errno(EINVAL);
		return(-1);
	}
	/*
	 * round up to the nearest page
	 */
	newsize = (newsize + vm_page_size - 1) & ~(vm_page_size - 1);
	/*
	 * if we have gone negative then the requested size was too big
	 * and so we return an error. Also check that we have enough
	 * to add the size of the red zone without overflowing.
	 */
	if ((newsize + RED_ZONE_SIZE) < 0) {
		set_errno(EINVAL);
		return(-1);
	}
	(*attr)->stacksize = newsize;
	return(0);
}

/*
 * Function:
 *	pthread_attr_getstacksize
 *
 * Parameters:
 *	attr - a pointer to the attribute
 *
 * Return value:
 *	-1	the pointer to the attribute was invalid (EINVAL)
 *		the attribute was invalid (EINVAL)
 *	otherwise the size of stack to be created with this attribute
 *
 * Description:
 *	After checking that we have a real attribute we return whatever
 *	is in the stacksize element.
 */
long
pthread_attr_getstacksize(pthread_attr_t attr)
{
	if ((attr == NO_ATTRIBUTE) || !(attr->flags&ATTRIBUTE_VALID)) {
		set_errno(EINVAL);
		return(-1);
	}
	return(attr->stacksize);
}

/*
 * Function:
 *	pthread_attr_setprio
 *
 * Parameters:
 *	attr - a pointer to the attribute to be altered
 *	priority - the priority a thread is created with using this attribute
 *
 * Return value:
 *	-1	This function is not supported (ENOSYS)
 *
 * Description:
 *	This function is not supported.
 */
int
pthread_attr_setprio(pthread_attr_t *attr, int priority)
{
	set_errno(ENOSYS);
	return(-1);
}

/*
 * Function:
 *	pthread_attr_getprio
 *
 * Parameters:
 *	attr - a pointer to the attribute
 *
 * Return value:
 *	-1	This function is not supported (ENOSYS)
 *
 * Description:
 *	This function is not supported.
 */
int
pthread_attr_getprio(pthread_attr_t attr)
{
	set_errno(ENOSYS);
	return(-1);
}

/*
 * Function:
 *	pthread_attr_setsched
 *
 * Parameters:
 *	attr - a pointer to the attribute to be altered
 *	scheduler - the scheduler policy the thread is created with
 *
 * Return value:
 *	-1	This function is not supported (ENOSYS)
 *
 * Description:
 *	This function is not supported.
 */
int
pthread_attr_setsched(pthread_attr_t *attr, int scheduler)
{
	set_errno(ENOSYS);
	return(-1);
}

/*
 * Function:
 *	pthread_attr_getsched
 *
 * Parameters:
 *	attr - a pointer to the attribute
 *
 * Return value:
 *	-1	This function is not supported (ENOSYS)
 *
 * Description:
 *	This function is not supported.
 */
int
pthread_attr_getsched(pthread_attr_t attr)
{
	set_errno(ENOSYS);
	return(-1);
}

/*
 * Function:
 *	pthread_attr_setinheritsched
 *
 * Parameters:
 *	attr - a pointer to the attribute to be altered
 *	inherit - flag indicating inheritence of scheduling policy
 *
 * Return value:
 *	-1	This function is not supported (ENOSYS)
 *
 * Description:
 *	This function is not supported.
 */
int
pthread_attr_setinheritsched(pthread_attr_t *attr, int inherit)
{
	set_errno(ENOSYS);
	return(-1);
}

/*
 * Function:
 *	pthread_attr_getinheritsched
 *
 * Parameters:
 *	attr - a pointer to the attribute
 *
 * Return value:
 *	-1	This function is not supported (ENOSYS)
 *
 * Description:
 *	This function is not supported.
 */
int
pthread_attr_getinheritsched(pthread_attr_t attr)
{
	set_errno(ENOSYS);
	return(-1);
}
