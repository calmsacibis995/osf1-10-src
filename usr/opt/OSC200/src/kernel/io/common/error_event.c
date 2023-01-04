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
static char *rcsid = "@(#)$RCSfile: error_event.c,v $ $Revision: 1.1.2.4 $ (DEC) $Date: 1993/11/23 21:56:32 $";
#endif

/*
 *
 *  MODULE NAME:	error_event.c
 *
 *  AUTHOR:		Maria Vella
 *
 *  VERSION:		1.0                    DATE: 05/03/93
 *
 *  DESCRIPTION:
 *	This module contains the kernel error event functions used by 
 *	drivers to register for notifcation and be notified of an
 *	asynchronous event on a particular device.
 *
 */

/************************* Include Files ********************************/

#include <sys/errno.h>
#include <sys/types.h>
#include <io/common/iotypes.h>
#include <kern/lock.h>
#include "error_event.h"

/************************* Local Defines ********************************/

#define Dprintf 
#define Eprintf printf

/*
 * Checks whether the event control structure has been initialized and if not
 * performs initialization.
 */
#define EVENT_INIT()						\
    if(ev_inited == FALSE)  					\
    {								\
	EVENT_INIT_LOCK();    /* init the event lock */ 	\
	event_cntrl->ec_flink = event_cntrl->ec_blink = 	\
		(EVENT_ENTRY *)&event_cntrl->ec_flink;		\
	ev_inited = TRUE;					\
    }

void event_notify();
void find_event_entry();

/****************** Initialized and Unitialized Data ********************/

static ev_inited = FALSE;
EVENT_CNTRL event_cntrl_struct;  	/* Anchor for event registrations */
EVENT_CNTRL *event_cntrl = &event_cntrl_struct;

struct lockinfo *event_cntrl_li;
lock_data_t lk_event_cntrl;


/*
 *  ROUTINE NAME: find_event_entry()
 *
 *  FUNCTIONAL DESCRIPTION:
 * 	Searches from start_ptr to find an event entry for dev.
 *	If a match is found for dev, then the event_ptr is set to the
 *	event entry. If no match is found, event_ptr is set to NULL.
 *
 *  FORMAL PARAMETERS:
 *	dev 	  - The dev_t for the device event entry.
 *	start_ptr - The starting location in the event list to search from.
 *	event_ptr - Contains the pointer to the event entry found or NULL
 *		    if a match is not found for dev.
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:	None.
 *
 *  ADDITIONAL INFORMATION:
 *
 */

void
find_event_entry(dev, start_ptr, event_ptr)
dev_t dev;
EVENT_ENTRY *start_ptr;
EVENT_ENTRY **event_ptr;
{
	EVENT_ENTRY *cur_ptr;
	int found = 0;
	int s;

	Dprintf("find_event_entry(): enter dev=%x start=%lx\n", dev, start_ptr);

	*event_ptr = (EVENT_ENTRY *)NULL;

	cur_ptr = start_ptr;

	EVENT_LOCK(s);

	/*
	 * Walk the event list looking for a match.
	 */
	do   
	{
		/*
		 * Check for end of list.
		 */
		if( cur_ptr == (EVENT_ENTRY *)&event_cntrl->ec_flink)  
		{
			Dprintf("find_major_entry() No match found\n");
			break;
		}

		/*
		 * Check for a match on the dev - only compare the unit 
		 * number portion of the dev_t. Device specific bits 
		 * are not needed.
		 */
		if(GETUNIT(cur_ptr->ee_dev) == GETUNIT(dev))  
		{
			Dprintf("find_event_entry(): found match\n");
			found = 1;
			break;
		}

		cur_ptr = cur_ptr->flink;

	}  while( (cur_ptr != (EVENT_ENTRY *)&event_cntrl->ec_flink) || !(found));

	EVENT_UNLOCK(s);

	if(found)
		*event_ptr = cur_ptr;
	else
		*event_ptr = NULL;
	
}

/*
 *  ROUTINE NAME: event_register()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function will register an event callback function for a 
 *	specific dev. First we make sure that the event list is initialized.
 *	If an entry already exists for this device with the same error
 *	callback function, then we are done so return ESUCCESS. Otherwise
 *	allocate an event entry structure, fill in the dev,error event
 *	callback function and driver registered type, and insert it 
 *	into the error event list.
 *
 *  FORMAL PARAMETERS:
 *	dev - Device to be notified on.
 *	error_func - Function to call when event occurs.
 *	reg_type - Registration type (ie EV_AVAIL_MANAGER for the  
 *		   Avaialability Manager driver used for ASE.
 *
 *  IMPLICIT INPUTS:
 *	Event list.
 *
 *  IMPLICIT OUTPUTS:
 *	Modified event list.
 *
 *  RETURN VALUE:
 *	ESUCCESS
 *	ENOMEM - Failed to allocate event entry structure.
 *
 *  ADDITIONAL INFORMATION:
 */

event_register(dev, error_func, reg_type)
dev_t dev;
void (*error_func)();
U32 reg_type;
{
	EVENT_ENTRY *ep = (EVENT_ENTRY *)NULL;
	int s;

	Dprintf("In event_register() dev=%x \n", dev);

	/*
	 * Initialize on the first call to this function.
	 */
	EVENT_INIT();

	/*
	 * Get the event pointer for this device if one exists.
	 */
	FIND_EVENT_ENTRY(dev, event_cntrl->ec_flink, &ep);

	/*
	 * Check whether we are registering the same function for the device.
	 * If so there is nothing to do.
	 */
	if( ep != (EVENT_ENTRY *)NULL) 
	{
		if(ep->ee_error_func == error_func)
			Eprintf("event_register() - re-reigister for dev %x\n",
				dev);
			return(ESUCCESS);
	}
		
	/*
	 * Allocate and fill in an event entry.
	 */
	Dprintf("event_register() - allocate event entry\n");
	if( (ep = (EVENT_ENTRY *)kalloc(sizeof(EVENT_ENTRY))) 
	    == (EVENT_ENTRY *)NULL) 
	{
		Eprintf("event_register() - kalloc failed\n");
		return(ENOMEM);
	}
	bzero(ep, sizeof(EVENT_ENTRY)); 

	ep->ee_dev = dev;
	ep->ee_reg_type = reg_type;
	ep->ee_error_func = error_func;

	Dprintf("event_register() - Adding event entry\n");

	EVENT_LOCK(s);
	ADD_EVENT_ENTRY(ep, &event_cntrl->ec_flink);
	EVENT_UNLOCK(s);

	Dprintf("event_register() - exit\n");

	return(ESUCCESS);
}

/*
 *
 *  ROUTINE NAME: event_deregister()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function will deregister an event or a device.
 *
 *  FORMAL PARAMETERS:
 *	dev - Device to be notified on.
 *	error_func - Function to call when event occurrs.
 *	reg_type - Registration type (ie AM driver).
 *
 *  IMPLICIT INPUTS:
 *	Event list.
 *
 *  IMPLICIT OUTPUTS:
 *	Modified event list.
 *
 *  RETURN VALUE:
 *
 *  ADDITIONAL INFORMATION:
 *
 */

event_deregister(dev, error_func, reg_type)
dev_t dev;
void (*error_func)();
U32 reg_type;
{
	EVENT_ENTRY *prev;
	EVENT_ENTRY *ep = (EVENT_ENTRY *)NULL;
	int s;

	/*
	 * Initiialize on the first call to this function.
	 */
	EVENT_INIT();

	Dprintf("In event_deregister() dev=%x \n", dev);

	/*
	 * Get the event pointer for this device if one exists.
	 */
	FIND_EVENT_ENTRY(dev, event_cntrl->ec_flink, &ep);

	EVENT_LOCK(s);

	/*
	 * Find the device event entry.
	 */
	while( ep != (EVENT_ENTRY *)NULL)  
	{
	   prev = ep->flink;
	   if( (ep->ee_dev == dev)
	    	&& (ep->ee_error_func == error_func)
	    	&& (ep->ee_reg_type == reg_type) ) 
		{
		        REMOVE_EVENT_ENTRY(ep);
		        kfree(ep, sizeof(EVENT_ENTRY));
		}
		FIND_EVENT_ENTRY(dev, prev, &ep);
	}

	EVENT_UNLOCK(s);
}

/*
 *
 *  ROUTINE NAME: event_deregister_all()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This function will deregister all entries with the reigstration
 *	type of the reg_type.
 *
 *  FORMAL PARAMETERS:
 *	reg_type - Registration type (ie AM driver).
 *
 *  IMPLICIT INPUTS:
 *	Event list.
 *
 *  IMPLICIT OUTPUTS:
 *	Modified event list.
 *
 *  RETURN VALUE:
 *
 *  ADDITIONAL INFORMATION:
 *
 */

event_deregister_all(reg_type)
U32 reg_type;
{
	EVENT_ENTRY *ep, *next;
	int s;

	/*
	 * Initiialize on the first call to this function.
	 */
	EVENT_INIT();
	Dprintf("In event_deregister_all(): type = %x \n", reg_type);

	/*
	 * Check whether we were never initialized.
	 */
	if( (ep = event_cntrl->ec_flink) == (EVENT_ENTRY *)NULL)
	{
	   return;
	}

	EVENT_LOCK(s);

	while( ep != (EVENT_ENTRY *)&event_cntrl->ec_flink)  
	{
	   next = ep->flink;
	   if (ep->ee_reg_type == reg_type) 
	   {
		Dprintf("event_deregister_all: Remove entry %lx for dev %x\n",
			 ep, ep->ee_dev);
		REMOVE_EVENT_ENTRY(ep);
		kfree(ep, sizeof(EVENT_ENTRY));
	   }

	   ep = next;
	   
	}
	EVENT_UNLOCK(s);
}



/*
 *
 *  ROUTINE NAME: event_notify()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This fucntion is called from drivers to indicate an asynchronous
 *	error event for a device.  If an entry exists for the device
 *	with the error, then the registered error event callback
 *	fucntion is called.
 *
 *  FORMAL PARAMETERS:
 *	dev - Device to be notified on.
 *	error_func - Function to call when event occurrs.
 *	reg_type - Registration type (ie AM driver).
 *
 *  IMPLICIT INPUTS:
 *	Event list.
 *
 *  IMPLICIT OUTPUTS:
 *	Modified event list.
 *
 *  RETURN VALUE:
 *
 *  ADDITIONAL INFORMATION:
 *
 */

void
event_notify(dev, struct_ptr)
dev_t dev;
void *struct_ptr;
{
	EVENT_ENTRY *ep;
	EVENT_ENTRY *start;
	int s;

	/*
	 * Initiialize on the first call to this function.
	 */
	EVENT_INIT();

	Dprintf("In event_notify() - dev=%x struct_ptr=%lx\n", dev, struct_ptr);

	/*
	 * Find all matching device and event entries and call 
	 * registered error event function.
	 */
	EVENT_LOCK(s);
	start = event_cntrl->ec_flink;

	do    
	{
		FIND_EVENT_ENTRY(dev, start, &ep);
		if( ep == (EVENT_ENTRY *)NULL)  
		{
			Dprintf("event_notify() - no more entries for dev %x\n",
				 dev);
			break;
		}

		start = ep->flink;

		/*
		 * Call the registered error event function.
		 */
		ep->ee_error_func(dev, struct_ptr);

	}  while(ep != (EVENT_ENTRY *)NULL);

	EVENT_UNLOCK(s);

	Dprintf("event_notify() - exit\n");
}
