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
static char	*sccsid = "@(#)$RCSfile: handler.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:18:08 $";
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
#include <i386/handler.h>
#include <i386/dispatcher.h>



#if	NCPUS > 1
/*
 * Interrupt handler function synchronization locking.
 *
 * The lock stops race conditions dead in their tracks, allowing the resolver 
 * function to remap handler target interrupt vector/level placement.
 */
#include <kern/lock.h>
lock_data_t			handler_lock;
#define HANDLER_WRITE_LOCK()	lock_write( &handler_lock )
#define HANDLER_WRITE_UNLOCK()	lock_done ( &handler_lock )
#define HANDLER_LOCK_INIT()	lock_init ( &handler_lock, TRUE )
#else
#define HANDLER_LOCK()
#define HANDLER_UNLOCK()
#define HANDLER_WRITE_LOCK()
#define HANDLER_WRITE_UNLOCK()
#define HANDLER_LOCK_INIT()
#endif	NCPUS 


/*
 * NAME:        itable_init
 *
 * FUNCTION:    Initialize interrupt dispatch table
 *
 * EXEC ENV:    This routine must get called to initialize the interupt
 *              dispatch table before the dispatcher is accessed or interrupt
 *              handlers are registered.
 *
 * RETURNS:     void
 *
 */
itable_t itable[ITABLE_SIZE];
void
itable_init( )
{
        register int    i;

        for(i=0; i <= ITABLE_SIZE; i++)
	{
                ITABLE_LOCKINIT(i);
	}

	HANDLER_LOCK_INIT();
}



/*
 * NAME:	handler_override()
 * 
 * FUNCTION:	Swap an interrupt chain passed to this function with
 *		the already existing one as specified by the level argument
 * 
 * EXEC ENV: 	This routine is used by the i386 mouse device driver to
 *		substitute the currently existing interrupt routine for 
 *		a particular level with a new one.
 *		We chose this approach because of the difficulity of
 *		of determining which interrupt routine(s) to remove 
 *		based soley upon the interrupt level. This is considered
 *		somewhat of a hack but it works and for reasons of expediants
 *		we lived with this compromise.
 *
 * RETURNS:	Returns a pointer to current chain of handlers for a specified
 *		level.	NULL isn't an error since it is a valid returned value.
 * 
 */

ihandler_t *
handler_override( ih, index )
	ihandler_t *	ih;
	int index;
{
	int		s;
	ihandler_t *	ret_ih;

	HANDLER_LOCK();

	/*
	 * Swap interrupt handler into the interrupt table 
	 */
	s = splhigh();
	ITABLE_WRITE_LOCK( index );
						/* Insert handler into itable */
	if(index < 0 || index > ITABLE_SIZE)
		return( (ihandler_t *) NULL);
	ret_ih = itable[index].it_ih;
	itable[index].it_ih = ih;

	ITABLE_WRITE_UNLOCK( index );
	splx( s );
	HANDLER_UNLOCK();
	return ( ret_ih );
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
	int		s;
	int		index;
	ihandler_t *	p;
	ihandler_id_t	id;

	/*
	 * Sanity check
	 */
	if ( ih == NULL || ih->ih_handler == (int (*)())NULL )
		return( NULL );

	HANDLER_LOCK();
	/*
	 * Call the resolver routine (if provided) to determine if the
	 * new handler can be added.  
	 * Note: The resolver must handle any needed interrupt level changes.
	 */
	if (ih->ih_resolver != (int (*)()) NULL)
	    if ((* ih->ih_resolver)( ih ) < 0) {
		HANDLER_UNLOCK();
		return( NULL );
	}

	/*
	 * Map the interrupt handler to an interrupt table index
	 * Note: The ih_resolver() may have remapped the targetted level
	 */
	id_set( &id, ih, 0 );
	if ( (index = id_index( &id )) < 0 ) {
		HANDLER_UNLOCK();
		return( NULL );
	}

	/*
	 * Add interrupt handler into the interrupt table 
	 */
	s = splhigh();
	ITABLE_WRITE_LOCK( index );
						/* Insert handler into itable */
	ih->ih_next = itable[index].it_ih;
	itable[index].it_ih = ih;

						/* Create the real handle id */
	id_set( &ih->ih_id, ih, id_unique( index ) );

						/* Disable interrupt handler */
	ih->ih_state &=  ~IH_STATE_ENABLED;

	ITABLE_WRITE_UNLOCK( index );
	splx( s );
	HANDLER_UNLOCK();
	return ( &ih->ih_id );
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
 */
int 
handler_del( id )
	ihandler_id_t *	id;
{
	int		s;
	int		index;
	int		err;
	ihandler_t *	p;

	/*
	 * Map the interrupt handler to an interrupt table index
	 */
	if ( (index = id_index( id )) < 0 ) 
		return( -1 );

	/*
	 * Deregister the interrupt handler
	 */
	err = 0;
	HANDLER_LOCK();
	s = splhigh();
	ITABLE_WRITE_LOCK( index );
						/* Unlink handler from itable */
	p = itable[index].it_ih;
	if ( p == NULL ) 
		err = -1;
	else {
		if ( ! id_cmp( &p->ih_id, id )) {	
			itable[index].it_ih = p->ih_next;
		} else {
			while(p->ih_next!=NULL && id_cmp(&p->ih_next->ih_id,id))
				p = p->ih_next;
			if ( p != NULL )
				p->ih_next = p->ih_next->ih_next;
			else
				err = -1;
		}
	}
						/* UNLOCK & RESPL */
	ITABLE_WRITE_UNLOCK( index );
	splx( s );
	HANDLER_UNLOCK();
	return ( err );
}


/* 
 * NAME:	handler_enable()
 *
 * FUNCTION:	Sets a interrupt handler to an enabled state 
 *
 * EXEC ENV:	This routine must be executed on a interrupt handler before 
 *		the dispatcher will recognize it.
 *
 * RETURNS:	On success zero is returned, else -1 on failure.
 */
int
handler_enable( id )
	ihandler_id_t *	id;
{
	return( handler_state( id, IH_STATE_ENABLED, TRUE ));
}


/* 
 * NAME:	handler_disable()
 *
 * FUNCTION:	Sets a interrupt handler to an disabled state 
 *
 * EXEC ENV:	This routine is used on a interrupt handler to tell
 *		the dispatcher not to call this handler.
 *
 * RETURNS:	On success zero is returned, else -1 on failure.
 */
int
handler_disable( id ) 
	ihandler_id_t *	id;
{
	return( handler_state( id, IH_STATE_ENABLED, FALSE ));
}


/*
 * Locate a interrupt handler and set/clear bits in the state flag
 */
int
handler_state( id, state, set )
	ihandler_id_t *	id;
	int		state;
	int		set;			/* True==Set, False==Clear */
{
	int		s;
	int		err;
	int		index;
	ihandler_t *	p;

	/*
	 * Map the interrupt handler to an interrupt table index
	 */
	if ( (index = id_index( id )) < 0 ) 
		return( -1 );

	/*
	 * Find interrupt handler, and set/clear state
	 */
	err = 0;
	HANDLER_LOCK();
	s = splhigh();
	ITABLE_WRITE_LOCK( index );

						/* Unlink handler from itable */
	for(p=itable[index].it_ih; p && id_cmp( &p->ih_id, id ); p=p->ih_next)
	;
	if ( p )
		if ( set == FALSE )
			p->ih_state &= ~state;
		else
			p->ih_state |=  state;
	else
		err = -1;

	ITABLE_WRITE_UNLOCK( index );
	splx( s );
	HANDLER_UNLOCK();
	return ( err );
}


/*
 * Return a unique interrupt handler id (per level)
 */
int
id_unique( index )
	int	index;
{
	return( itable[index].it_elementcnt++ );
}



/*
 * Set id values given interrupt handler and unique id
 */
void
id_set( id, ih, unique )
	ihandler_id_t *	id;
	ihandler_t *	ih;
	int		unique;
{
	id->id_index = ih->ih_level;
	id->id_element = unique;
}


/*
 * Determine from id the interrupt handlers location in itable
 */
int
id_index( id )
	ihandler_id_t *	id;
{
	if ( id->id_index < 0 || id->id_index > ITABLE_SIZE ) 
		return( -1 );
	else
		return( id->id_index );
}


/*
 * Compare two interrupt handler ids
 */
int
id_cmp( id1, id2 )
	ihandler_id_t *	id1;
	ihandler_id_t *	id2;
{
	return( (id1->id_index != id2->id_index || id1->id_element != id2->id_element) );
}
