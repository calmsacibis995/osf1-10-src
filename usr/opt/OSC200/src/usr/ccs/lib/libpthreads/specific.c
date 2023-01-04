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
static char	*sccsid = "@(#)$RCSfile: specific.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 04:13:03 $";
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
 * File: specific.c
 *
 * This file contains all the functions associated with the management of
 * thread specific data. Thread specific data is held in a page of memory
 * beyond the red zone of the stack.
 */

#include <pthread.h>
#include <errno.h>
#include "internal.h"

/*
 * Local Definitions
 */
#define	MAXKEYS		(vm_page_size / sizeof(specific_data_t))
#define KEYTABLE_SIZE	(sizeof(specific_key_t) * MAXKEYS)

/*
 * Local Variables
 */
private	volatile int	specific_lock;
private	unsigned	next_key;
private specific_key_t	*key_table;

/*
 * Function:
 *	specific_data_startup
 *
 * Description:
 *	This function is called from pthread_init to initialize all the
 *	specific data global data and state.
 */
void
specific_data_startup()
{
	/*
	 * allocate a key table for the process. This defines whether a
	 * key is valid or not. Mark the next free key to be the start
	 * of this table and initialize the lock that protects all this.
	 */
	key_table = (specific_key_t *)malloc(KEYTABLE_SIZE);
	bzero(key_table, KEYTABLE_SIZE);
	next_key = 0;
	specific_lock = SPIN_LOCK_UNLOCKED;
}

/*
 * Function:
 *	pthread_keycreate
 *
 * Parameters:
 *	key_ptr	- pointer to a place to store the newly created key
 *	destructor - function to call for any data when the thread dies.
 *
 * Return value:
 *	0	Success, key_ptr is set with the key id
 *	-1	if the pointer passed is an invalid pointer (EINVAL)
 *		if the key table is full (ENOMEM)
 *
 * Description:
 *	Allocate the next entry out of the key table. Mark it
 *	as allocated and move the next key pointer on. Key ids
 *	are small integers which are used as an index into the key array.
 */
int
pthread_keycreate(pthread_key_t *key_ptr, void (*destructor)(void *))
{

	/*
	 * Check we have somewhere to store the new key id
	 */
	if (key_ptr == NULL) {
		set_errno(EINVAL);
		return(-1);
	}

	/*
	 * try to allocate the next empty key slot.
	 */
	spin_lock(&specific_lock);
	if (next_key == MAXKEYS) {
		spin_unlock(&specific_lock);
		set_errno(ENOMEM);
		return(-1);
	}

	*key_ptr = next_key++;

	/*
	 * We have got the new key id. Mark the slot as allocated and
	 * remember the destructor function (which may be NULL).
	 */
	key_table[*key_ptr].flags = KEY_ALLOCATED;
	key_table[*key_ptr].destructor = destructor;
	spin_unlock(&specific_lock);
	return(0);
}

/*
 * Function:
 *	pthread_setspecific
 *
 * Parameters:
 *	key	- the id of the specific data to set
 *	value	- the new data to associate with that key
 *
 * Return value:
 *	0	Success, the data is set in the per thread area
 *	-1	If the key is outside the key table (EINVAL)
 *		The key is not yet allocated (EINVAL)
 *
 * Description:
 *	Check that the key lies within the table and that is has been
 *	allocated. If so then put the data passed into the thread specific
 *	data pointed to by the thread structure.
 */
int
pthread_setspecific(pthread_key_t key, any_t value)
{
	pthread_t	self;
	specific_data_t	*data;
	specific_key_t	*key_status;

	/*
	 * Check the key is within the key table
	 */
	if (key >= MAXKEYS) {
		set_errno(EINVAL);
		return(-1);
	}

	/*
	 * get the key descriptor and check that this key has been allocated.
	 */
	key_status = &key_table[key];

	if (!(key_status->flags & KEY_ALLOCATED)) {
		set_errno(EINVAL);
		return(-1);
	}

	/*
	 * the key is find, now set the data. Mark the data as being valid.
	 */
	self = pthread_self();
	data = &self->specific_data[key];
	data->value = value;
	data->flags |= SPECIFIC_DATA_SET;
	return(0);
}

/*
 * Function:
 *	pthread_getspecific
 *
 * Parameters:
 *	key	- the id of the specific data to get
 *	value	- the place to store the data associated with that key
 *
 * Return value:
 *	0	success, the data si stored in 'value'
 *	-1	The key is out of the key table (EINVAL)
 *		The pointer 'value' is not a valid pointer (EINVAL)
 *		The key has within the table but unallocated (EINVAL)
 *
 * Description:
 *	Ensure the key and place to store the data are valid. If they are
 *	the specific data is stored in value if a set_specific has already
 *	happened. If the data has not already been set then NULL is stored
 *	in value but the call returns successfully.
 */
int
pthread_getspecific(pthread_key_t key, any_t *value)
{
	pthread_t	self;
	specific_data_t	*data;
	specific_key_t	*key_status;

	/*
	 * check the key is within the table and the pointer we have
 	 * been given is OK
	 */
	if ((key >= MAXKEYS) || (value == NULL)) {
		set_errno(EINVAL);
		return(-1);
	}

	/*
	 * get the key descriptor and check that this key has been allocated.
	 */
	key_status = &key_table[key];

	if (!(key_status->flags & KEY_ALLOCATED)) {
		set_errno(EINVAL);
		return(-1);
	}

	/*
	 * Find the data area. Return NULL if the data has not been previously
	 * set. The call is still considered a success.
	 */
	self = pthread_self();
	data = &self->specific_data[key];

	if (data->flags & SPECIFIC_DATA_SET)
		*value = data->value;
	else
		*value = NULL;
	return(0);
}

/*
 * Function:
 *	specific_data_setup
 *
 * Parameters:
 *	thread	- The new thread that is being created
 *
 * Description:
 *	create the specific data area for a newly created thread. Find the
 *	specific data area beyond the red zone of the vps stack and zero
 *	it. We only really need to zero the flags but that would probably
 *	take longer. Save the pointer in thre thread structure.
 */
void
specific_data_setup(pthread_t thread)
{
	/*
	 * At the base of every vp's stack is a red zone.
	 * The red zone consists of two pages (RED_ZONE_SIZE) below the
	 * stack base. One of these pages is protected to try and catch
	 * stack overflow. The other is used for thread specific data.
	 * set the pointer in the pthread structure to point at this
	 * area.
	 */
	thread->specific_data = (specific_data_t *)(thread->vp->stackbase -
					RED_ZONE_SIZE);
	bzero(thread->specific_data, vm_page_size);
}

/*
 * Function:
 *	specific_data_cleanup
 *
 * Parameters:
 *	thread	- the thread that is about to exit
 *
 * Description:
 *	For every valid key, call the destructor function on every piece
 *	of set, non-null data.
 */
void
specific_data_cleanup(pthread_t thread)
{
	pthread_key_t	key;
	pthread_key_t	last_key;
	specific_data_t	*data;
	specific_key_t	*key_status;

	/*
	 * Look at all allocated keys.
	 */
	spin_lock(&specific_lock);
	last_key = next_key;
	spin_unlock(&specific_lock);

	for (key = 0; key < last_key; key++) {
		data = &thread->specific_data[key];
		key_status = &key_table[key];

		/*
		 * Only call the destructor if there is one, the thread had
		 * set some data and that data was not NULL
		 */
		if ((data->flags & SPECIFIC_DATA_SET) &&
		    (key_status->destructor != NULL) &&
		    (data->value != NULL))
			(*(key_status->destructor))(data->value);
	}
}

/*
 * Function:
 *	specific_fork_prepare
 *
 * Description:
 *	quiesce the specific data subsystem. Do not allow any key creation
 *	during a fork.
 */
void
specific_fork_prepare()
{
	/*
	 * Grab the key table lock
	 */
	spin_lock(&specific_lock);
}

/*
 * Function:
 *	specific_fork_parent
 *
 * Description:
 *	Allow more key creation in the parent after a fork is complete.
 */
void
specific_fork_parent()
{
	spin_unlock(&specific_lock);
}

/*
 * Function:
 *	specific_fork_child
 *
 * Description:
 *	Set up the thread specific area for the child process. The table is
 *	still valid from the parent but we invalidate all the keys. Note that
 *	this means that any keys the forking thread has are now also invalid.
 */
void
specific_fork_child()
{
	/*
	 * Clear the table and reset the next free pointer.
	 */
	bzero(key_table, KEYTABLE_SIZE);
	next_key = 0;
	spin_unlock(&specific_lock);
}
