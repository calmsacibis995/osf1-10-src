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
static char *rcsid = "@(#)$RCSfile: handler.c,v $ $Revision: 1.1.3.4 $ (DEC) $Date: 1992/06/03 16:27:31 $";
#endif

#include <io/common/handler.h>
#include <kern/lock.h>
#include <io/common/devdriver.h>
#include <sys/table.h>

#if NCPUS > 1
	extern volatile unsigned long  system_intr_cnts_level[NCPUS][INTR_MAX_LEVEL];
	extern volatile unsigned long  system_intr_cnts_type[NCPUS][INTR_TYPE_SIZE];
#else	/* NCPUS */
	extern volatile unsigned long  system_intr_cnts_level[INTR_MAX_LEVEL];
	extern volatile unsigned long  system_intr_cnts_type[INTR_TYPE_SIZE];
#endif	/* NCPUS */
	extern volatile int  *system_intr_cnts_type_transl;

/*
 * This file contains the implementation of the handler_{add, del, enable,
 * disable} framework routines.  These routines are used to register interrupt
 * service routines (handlers).  The model followed here is that the
 * dispatching of interrupts is a BUS specific function.  Each bus type is
 * likely to have a different form of interrupt dispatching mechansim.  Based
 * on this the implementation of the handler_* routines consists of dispatching
 * the request to the appropriate bus specific implementation of the function.
 * As a result this file consists of dispatching routines, NOT the routines
 * which do the actual work.
 */

/*
 * MP locks to insure serial access to the linked list of handler keys
 * and to insure that the bus specific handler_{del, enable, disable} calls
 * are serialized.
 */
lock_data_t			handler_id_lock;

#define LOCK_ID_LIST 		lock_write(&handler_id_lock)
#define UNLOCK_ID_LIST 		lock_done (&handler_id_lock)
#define INIT_ID_LOCK 		lock_init (&handler_id_lock, TRUE)
#define LOCK_HANDLER_KEY(key)  	lock_write(&(key->lock))
#define UNLOCK_HANDLER_KEY(key) lock_done(&(key->lock))
#define INIT_HANDLER_KEY(key)	lock_init (&(key->lock), TRUE)

/*
 * This is a list header for a global linked list of handler key structures.
 * Initialized to null to specify that the list is empty.
 */
struct handler_key *handler_id_list_head = NULL;

/*
 * NAME:	handler_init()
 * 
 * FUNCTION:	Initialize data structures used by handler functions.
 * 
 * EXEC ENV: 	Called from system startup sequence prior to performing any
 *		other handler functions.
 *
 * RETURNS:	void	
 */
void
handler_init()
{
	INIT_ID_LOCK;
	handler_id_list_head = NULL;
}

/*
 * NAME:	handler_id_unique()
 * 
 * FUNCTION:	Sets a key which will uniquely identify a handler structure
 *		in a linked list of structures.  The id will be setup to be
 *		the address of the handler structure.  This is an easy way to 
 *		insure that the id is unique; rather than bumping a counter
 *		and using that to produce a unique value.
 * 
 * EXEC ENV: 	Called as a result of a handler add operation to obtain a new
 *		unique key which will be used by subsequent handler_ functions
 *		to identify the specific handler or set of handlers.
 *
 * RETURNS:	0 - success = a new unique id has been generated.
 */
handler_id_unique(handler_key_st, id)
	struct handler_key *handler_key_st;
	ihandler_id_t *id;
{
	*id = (ihandler_id_t)handler_key_st;
	return(0);
}

/*
 * NAME:	handler_id_verify()
 * 
 * FUNCTION:	This routine is called to verify that the handler being 
 *		referenced is on the linked list of valid handlers.  This
 *		check is performed to help prevent against a stale id being 
 *		used.
 * 
 * EXEC ENV: 	Called from handler_{enable, disable, del}
 *
 * RETURNS:	0 - success, a matching id is present on the global linked
 *		list of id's.  
 *		-1 - error, the id is invalid.
 */
handler_id_verify(id)
	ihandler_id_t id;
{
	struct handler_key *key;
	int retval = -1;

	if (handler_id_list_head == NULL) {
		return(-1);
	}
	if (id == NULL) {
		return(-1);
	}
	LOCK_ID_LIST;
	key = handler_id_list_head;
	while (key != NULL) {
		if (key->key == id) {
			retval = 0;
			break;
		}
		key = key->next;
	}
	UNLOCK_ID_LIST;
	return(retval);
}

/*
 * NAME:	handler_alloc_key()
 * 
 * FUNCTION:	Allocates a handler key data structure.
 * 
 * EXEC ENV: 	Called during handler_add processing.
 *
 * RETURNS:	Returns a pointer to the new key on success,  returns NULL
 *		if unable to allocate memory.
 */
struct handler_key *
handler_alloc_key()
{
	struct handler_key *key;

	key = (struct handler_key *)(kalloc(sizeof(struct handler_key)));
	if ((char *)key <= (char *)0) {
		key = NULL;
	}
	INIT_HANDLER_KEY(key);
	return(key);
}

/*
 * NAME:	handler_alloc_it_t()
 * 
 * FUNCTION:	Allocates a handler id data structure.
 * 
 * EXEC ENV: 	Called during handler_add processing.
 *
 * RETURNS:	Returns a pointer to the new id struct on success,  returns NULL
 *		if unable to allocate memory.
 */
ihandler_id_t *
handler_alloc_it_t()
{
	ihandler_id_t *id;

	id = (ihandler_id_t *)(kalloc(sizeof(ihandler_id_t)));
	if ((char *)id <= (char *)0) {
		id = NULL;
	}
	return(id);
}

/*
 * NAME:	handler_free_key()
 * 
 * FUNCTION:	Dealocates the memory associated with a handler key structure.
 *		Bzero the structure to protect against any dangling references.
 * 
 * EXEC ENV: 	Called during handler_del processing.
 *
 * RETURNS:	0 - success = memory released, -1 - error = unable to allocate
 *		the memory.
 */
handler_free_key(key)
	struct handler_key *key;
{

	/*
	 * Paranoia check.
	 */
	if (key == NULL) {
		return(-1);
	}
	bzero((char *)key, sizeof(struct handler_key));
	kfree(key, sizeof(struct handler_key));
	return(0);
}

/*
 * NAME:	handler_free_id_t()
 * 
 * FUNCTION:	Dealocates the memory associated with a handler id structure.
 *		Bzero the structure to protect against any dangling references.
 * 
 * EXEC ENV: 	Called during handler_del processing.
 *
 * RETURNS:	0 - success = memory released, -1 - error = unable to allocate
 *		the memory.
 */
handler_free_id_t(id)
	ihandler_id_t *id;
{

	/*
	 * Paranoia check.
	 */
	if (id == NULL) {
		return(-1);
	}
	bzero((char *)id, sizeof(ihandler_id_t));
	kfree(id, sizeof(ihandler_id_t));
	return(0);
}

/*
 * NAME:	handler_link_key()
 * 
 * FUNCTION:	Link a handler key sturcture onto the global linked list.
 *		This is done by placing the new entry at the head of the
 *		list.  There is no need for list ordering.  The handler keys
 *		are placed on a global linked list so that when an "id" is
 *		passed to handler_{del, enable, disable} we can verify that the
 *		id is valid.
 * 
 * EXEC ENV: 	Called during handler_add processing to add the key to the
 *		global linked list of keys.
 *
 * RETURNS:	0 - success = was able to add key to global list.
 *		-1 - error  = the key appears to be invalid.  In this case
 *			      do not add it to the list.
 */
handler_link_key(handler_key_st)
	struct handler_key *handler_key_st;
{
	/*
	 * Paranoia check.
	 */
	if (handler_key_st == NULL) {
		return(-1);
	}
	if (handler_key_st->key != (ihandler_id_t)handler_key_st) {
		return(-1);
	}
	LOCK_ID_LIST;
	if (handler_id_list_head != NULL) {
           handler_id_list_head->prev = handler_key_st;
        }
	handler_key_st->next = handler_id_list_head;
	handler_key_st->prev = NULL;
	handler_id_list_head = handler_key_st;
	UNLOCK_ID_LIST;
	return(0);
}

/*
 * NAME:	handler_unlink_key()
 * 
 * FUNCTION:	Remove a handler key from the globabl list of handler keys.
 * 
 * EXEC ENV: 	Called during handler_del processing to remove references to 
 *		the handler.
 *
 * RETURNS:	0 - success = unlinked the specified key.
 *		-1 - error  = the key appears to be invalid.  Fail the unlink.
 * 
 * NOTE:	Interrupt handlers are initially disabled. See handler_enable().
 */
handler_unlink_key(handler_key_st)
	struct handler_key *handler_key_st;
{
	struct handler_key *next_st;
	struct handler_key *prev_st;


	/*
	 * Paranoia check.
	 */
	if (handler_key_st == NULL) {
		return(-1);
	}
	if (handler_key_st->key != (ihandler_id_t)handler_key_st) {
		return(-1);
	}
	LOCK_ID_LIST;
	next_st = handler_key_st->next;
	prev_st = handler_key_st->prev;

	if ((prev_st == NULL) && (next_st == NULL)) {
		handler_id_list_head = NULL;
	}
	else {
		if (prev_st != NULL) 
		   prev_st->next = handler_key_st->next;
                else
		   handler_id_list_head = next_st;
		if (next_st != NULL) next_st->prev = prev_st;
	}
	UNLOCK_ID_LIST;
	return(0);
}

/*
 * NAME:	handler_add()
 * 
 * FUNCTION:	Register an interrupt handler with the interrupt dispatcher
 * 
 * EXEC ENV: 	Static Device Drivers - This routine is called for registering
 *		each static device driver's ISRs. This is typically controlled 
 *		by autoconfig during system initialization.
 *		Dynamic Device Drivers - This routine is called by a 
 *		dynamically loaded device driver's configuration routine to 
 *		register its ISRs.
 *
 * RETURNS:	Returns a pointer to the new ihandler_id_t on success, 
 *		NULL on failure.
 * 
 * NOTE:	Interrupt handlers are initially disabled. See handler_enable().
 */
ihandler_id_t *
handler_add( ih )
	ihandler_t *	ih;
{
	struct handler_key *key_ptr;
	ihandler_id_t *id;

	/*
	 * Sanity check
	 */
	if ((ih == NULL) || (ih->ih_bus == NULL) || 
	    (ih->ih_bus->framework == NULL) || 
	    (ih->ih_bus->framework->adp_handler_add == 
				(ihandler_id_t (*)())NULL)) {
		return(NULL);
	}
	/*
	 * Allocate a handler key for this ISR.  This will be used to
	 * uniquely identify the ISR for future enable/disable/delete
	 * operations.
	 */
	key_ptr = handler_alloc_key();
	if (key_ptr == NULL) {
		return(NULL);
	}
	/*
	 * Allocate the memory to be used as a ihandler_id_t.
	 */
	id = handler_alloc_it_t();
	if (id == NULL) {
		return(NULL);
	}
	/*
	 * Obtain a unique identifier which will be used to reference this
 	 * handler in future handler_{del, enable, disable} calls.
 	 */
	handler_id_unique(key_ptr, id);
	ih->ih_id = *id;
	/*
	 * Call the bus specific handler_add routine.
	 */
	key_ptr->bus = ih->ih_bus;
	key_ptr->key = *id;
	LOCK_HANDLER_KEY(key_ptr);
	key_ptr->bus_id_t = (*ih->ih_bus->framework->adp_handler_add)(ih);
	/*
 	 * Check the return value of the bus specific handler_add routine.
	 * It should return an id of its own form.
 	 */
	if (key_ptr->bus_id_t == (ihandler_id_t)-1) {
		handler_free_id_t(id);
		handler_free_key(key_ptr);
		UNLOCK_HANDLER_KEY(key_ptr);
		return(NULL);
	}
	/*
	 * Disable interrupt handler state.
	 */
	key_ptr->state &=  ~IH_STATE_ENABLED;
	/*
	 * Now that the handler add has completed chain the key structure onto
	 * the system-wide global linked list of handler keys.
	 */
	if (handler_link_key(key_ptr) != 0) {
		handler_free_id_t(id);
	        handler_free_key(key_ptr);
		UNLOCK_HANDLER_KEY(key_ptr);
                return(NULL);
	}
	UNLOCK_HANDLER_KEY(key_ptr);
	return(id);
}

/*
 * NAME:	handler_del()
 * 
 * FUNCTION:	Deregister an interrupt handler from the interrupt dispatcher
 * 
 * EXEC ENV: 	Dynamic Device Drivers - This routine is called by a 
 *		dynamically device driver's deconfiguration routine to 
 *		deregister its ISRs.
 *
 * RETURNS:	On success zero is returned, else -1 on failure.
 * 
 * NOTES:	The handler must be currently disabled in order to delete.
 */
int
handler_del( id )
	ihandler_id_t *id;
{
	struct handler_key *key;
	
	/*
	 * Verify that the "id" represents a handler which is currently
	 * in the list.  
	 */
	if (handler_id_verify(*id) != 0) {
		return(-1);
	}
	key = (struct handler_key *)*id;
	/*
	 * Sanity check all the fields first.
	 */
	if ((key == NULL) || (key->bus == NULL) ||
	    (key->bus->framework == NULL) ||
	    (key->bus->framework->adp_handler_del ==
				(int (*)())NULL)) {
		return(-1);
	}
	LOCK_HANDLER_KEY(key);
	/*
	 * The handler must be disabled prior to deletion.
	 */
	if (key->state & IH_STATE_ENABLED) {
		UNLOCK_HANDLER_KEY(key);
		return(-1);
	}
	/*
	 * Call the bus specific handler_del function to do the real work.
	 */
	if ((*key->bus->framework->adp_handler_del)(key->bus_id_t) != 0){
		UNLOCK_HANDLER_KEY(key);
		return(-1);
	}
	/*
	 * Now that the bus has successfully deleted the handler it is safe
	 * to deallocate the handler key and id data structures.
	 */
	if (handler_unlink_key(key) == -1) {
		UNLOCK_HANDLER_KEY(key);
		return(-1);
	}
	if (handler_free_key(key) == -1) {
		UNLOCK_HANDLER_KEY(key);
		return(-1);
	}
	if (handler_free_id_t(id) == -1) {
		UNLOCK_HANDLER_KEY(key);
		return(-1);
	}
	UNLOCK_HANDLER_KEY(key);
	return(0);
}

/* 
 * NAME:	handler_enable()
 *
 * FUNCTION:	Sets a interrupt handler to an enabled state 
 *		This involves various things based upon system architecture
 *		thus this routine uses the cpusw to call the appropriate
 *		routine.
 *
 * EXEC ENV:	This routine must be executed on a interrupt handler before 
 *		the dispatcher will recognize it.
 *
 * RETURNS:	On success 0 is returned, else -1 on failure.
 */
int
handler_enable( id )
	ihandler_id_t *id;
{
	struct handler_key *key;
	
	/*
	 * Verify that the "id" represents a handler which is currently
	 * in the list.  
	 */
	if (handler_id_verify(*id) != 0) {
		return(-1);
	}
	key = (struct handler_key *)*id;
	/*
	 * Sanity check all the fields first.
	 */
	if ((key == NULL) || (key->bus == NULL) ||
	    (key->bus->framework == NULL) ||
	    (key->bus->framework->adp_handler_enable ==
				(int (*)())NULL)) {
		return(-1);
	}
	/*
	 * Assume that it is not an error to call this routine if the
	 * handler has already been enabled.  (ie let the bus specific
	 * routine make that call.)
	 *
	 * Call the bus specific handler_enable function to do the real work.
	 */
	LOCK_HANDLER_KEY(key);
	if ((*key->bus->framework->adp_handler_enable)(key->bus_id_t) 
	     != 0){
		UNLOCK_HANDLER_KEY(key);
		return(-1);
	}
	/*
	 * Mark the state as being enabled.  The only thing this is used for
	 * is in handler_del to make sure that the state is not currently
	 * enabled.
	 */
	key->state |= IH_STATE_ENABLED;
	UNLOCK_HANDLER_KEY(key);
	return(0);
}

/* 
 * NAME:	handler_disable()
 *
 * FUNCTION:	Sets a interrupt handler to an disabled state.  This should
 *		prevent interrupts from being dispatched to the handler. 
 *
 * EXEC ENV:	This routine is used on a interrupt handler to tell
 *		the dispatcher not to call this handler.
 *
 * RETURNS:	On success zero is returned, else -1 on failure.
 */
int
handler_disable( id )
	ihandler_id_t *id;
{
	struct handler_key *key;
	
	/*
	 * Verify that the "id" represents a handler which is currently
	 * in the list.  
	 */
	if (handler_id_verify(*id) != 0) {
		return(-1);
	}
	key = (struct handler_key *)*id;
	/*
	 * Sanity check all the fields first.
	 */
	if ((key == NULL) || (key->bus == NULL) ||
	    (key->bus->framework == NULL) ||
	    (key->bus->framework->adp_handler_disable ==
				(int (*)())NULL)) {
		return(-1);
	}
	/*
	 * No check is done to see if the handler is currently enabled. So it
	 * is not an error to disable something that is already disabled.
	 * (ie leave that decision to the bus specific function.)
	 *
	 * Call the bus specific handler_disable function to do the real work.
	 */
	LOCK_HANDLER_KEY(key);
	if ((*key->bus->framework->adp_handler_disable)(key->bus_id_t) 
	     != 0){
		UNLOCK_HANDLER_KEY(key);
		return(-1);
	}
	key->state &= ~IH_STATE_ENABLED;
	UNLOCK_HANDLER_KEY(key);
	return(0);
}
/*
 * NAME:	handler_stats()
 * 
 * FUNCTION:	Returns the number of interrupts of the specified type.
 *		This was formerly in dispatcher.c.
 * 
 * EXEC ENV: 	Called out of the table syscall.
 *
 * RETURNS:	Number of interrupts.
 */
long
handler_stats( type )
	int 	type;
{
	unsigned long count_interrupts_for_type();
	unsigned long total_intr = 0;

	if (type & INTR_HARDCLK)  total_intr += count_interrupts_for_type (INTR_TYPE_HARDCLK);
	if (type & INTR_SOFTCLK)  total_intr += count_interrupts_for_type (INTR_TYPE_SOFTCLK);
	if (type & INTR_DEVICE)   total_intr += count_interrupts_for_type (INTR_TYPE_DEVICE);
	if (type & INTR_OTHER)    total_intr += count_interrupts_for_type (INTR_TYPE_OTHER);
	if (type & INTR_STRAY)    total_intr += count_interrupts_for_type (INTR_TYPE_STRAY);
	if (type & INTR_DISABLED) total_intr += count_interrupts_for_type (INTR_TYPE_DISABLED);

	return((long) total_intr);
}
/*
 * NAME:	count_interrupts_for_type()
 * 
 * FUNCTION:	Returns the number of interrupts of the specified type.
 * 
 * EXEC ENV: 	Called from handler_stats()
 *
 * RETURNS:	Number of interrupts.
 */
static unsigned long
count_interrupts_for_type( type )
	int 	type;
{
   int		 level = 0;
   unsigned long count = 0;

   count += examine_interrupt_cnts_type(type);
   for (level = 0; level < INTR_MAX_LEVEL; level++) {
      if (system_intr_cnts_type_transl[level] == type) {
         count += examine_interrupt_cnts_level(level);
      }
   }
   return (count);
}
