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
#ifndef lint
static char *rcsid = "@(#)$RCSfile: topology.c,v $ $Revision: 1.1.3.5 $ (DEC) $Date: 1992/08/17 14:23:42 $";
#endif
*/

/*
 * This module contains routines to walk the hardware topology tree initially
 * created by static configuration.
 */

#include <sys/errno.h>
#include <sys/types.h>

#include <io/common/devdriver.h>
#include <io/common/devdriver_loadable.h>

/* Define all global pointers */

struct bus 			*system_bus	/* Pointer to nexus bus structure */
				   = NULL;
struct config_entry		*ldbl_devlist	/* Pointer to list of config devs */
				   = NULL;
int				ldbl_debug	/* True if debug messages should  */
				   = FALSE;	/* ...be printed.		  */

/* Define all globally declared routines */

struct bus *			ldbl_find_bus();
struct bus *			ldbl_search_bus_local();
struct bus *			ldbl_get_next_bus();
struct controller *		ldbl_find_ctrl();
int				ldbl_enqueue_devlist();
int				ldbl_find_devlist_entry();
void				ldbl_cleanup_devlist();

void				ldbl_deallocate_bus();
void				ldbl_deallocate_ctlr();
void				ldbl_deallocate_dev();

#ifdef LDBL_DEBUG
void				print_hardware_topology();
void				indent_level();
void				print_bus_entry();
void				print_ctlr_entry();
void				print_dev_entry();
#endif /* LDBL_DEBUG */

/******************************************************************************
 *
 * ldbl_find_bus()
 *
 *	This routine will scan the hardware topology tree starting at the
 *	specified bus structure for a match of bus_name (LDBL_WILDNAME for
 *	any) and/or bus_num (LDBL_WILDNUM for any).  The searching method
 *	is depth then width.  
 *
 *	One of the state flags is used to indicate whether the search should
 *	include the bus structure specified.  This is used when the routine 
 *	is called multiple times to restart a search from the last found bus.
 *	Specifying LDBL_STATE_NOINCLUDE prevents you from getting into an 
 *	infinite loop if the search criteria is the same (the current bus 
 *	structure would match and be returned - again).
 *
 *	Because this routine must be able to continue searching from any
 *	given location (both up and down the tree) without saving any state,
 *	it is not written recursively and hence is a bit ugly.
 *
 * Return Values:
 *	Null	- No match found
 *	Address	- Pointer to matching bus structure
 */

struct bus *
ldbl_find_bus (bus_ptr, state, bus_name, bus_num)
   struct bus	*bus_ptr;		/* Starting bus structure	*/
   int		state;			/* Misc. state flags		*/
   char		*bus_name;		/* Name of bus to search for	*/
   int		bus_num;		/* Bus number to search for	*/
{
   /*
    * Check to see if we should include the current bus structure in our
    * search or not.  If not, we need to decide what to do next.
    */
   if ((bus_ptr != NULL) && (state & LDBL_SEARCH_NOINCLUDE)) {
      bus_ptr = ldbl_get_next_bus (bus_ptr);
   }
   /*
    * Check to see if the current bus matches our search criteria.  If not
    * go decide which bus structure to check next.
    */
   while (bus_ptr != NULL) {
      if ((strcmp_EQL (bus_name, LDBL_WILDNAME) || strcmp_EQL (bus_ptr->bus_name, bus_name)) &&
         ((bus_num == LDBL_WILDNUM) || (bus_ptr->bus_num == bus_num))) {
            /*
	     *  We found a match, give the address to the caller.
	     */
            break;
      }
      bus_ptr = ldbl_get_next_bus (bus_ptr);
   }      
   return (bus_ptr);
}

/******************************************************************************
 *
 * ldbl_get_next_bus()
 *
 *	This routine decides what is the next bus structure that should be
 *	checked when walking the hardware topology tree.  Since walking of
 *	the tree is not done recursively, we need to be careful about what
 *	the next bus structure we select will be.  Otherwise we can easily
 *	place the system into an infinite loop by selecting something we
 *	already checked or miss part of the configuration.
 *
 *	The rules for walking the tree are as follows:
 *		1)  Attempt to go down the tree (bus_ptr->bus_list)
 *		2)  If that fails, try to go right (bus_ptr->nxt_bus)
 *		3)  If that fails, go up until you can go right or
 *		    you reach the system bus (bus_hd = NEXUS_NUMBER).
 *
 * Return Values:
 *	Null	- No match found
 *	Address	- Pointer to appropriate bus structure
 */

static struct bus *
ldbl_get_next_bus (bus_ptr)
   struct bus	*bus_ptr;		/* Starting bus structure	*/
{
   int		found = FALSE;		/* True when we find the struct */

   /*
    *  Check to see if we can go down from here.
    */
   if ((bus_ptr != NULL) && (bus_ptr->bus_list != NULL)) {
      bus_ptr = bus_ptr->bus_list;
   }
   else {
      /*
       *  Well, we couldn't go down, so let's start trying to go right
       *  and then up-right.  Watch out for the case where we reach the
       *  system bus - we can't go any further up.
       */
      while ((bus_ptr != NULL) && !found) {
         if (bus_ptr->nxt_bus != NULL) {
            bus_ptr = bus_ptr->nxt_bus;
	    found = TRUE;
         }
         else {
            bus_ptr = bus_ptr->bus_hd;
	    if (bus_ptr == (void *) NEXUS_NUMBER) { /* Don't walk off the top	*/
	       bus_ptr = NULL;
            }
         }
      }
   }
   return (bus_ptr);
}

/******************************************************************************
 *
 * ldbl_search_bus_local()
 *
 *	This routine, similiar to the ldbl_find_bus() routine, will scan
 *	one level of buses starting at the specified bus structure for a
 *	match of bus_name (LDBL_WILDNAME for any), bus_num (LDBL_WILDNUM
 *	for any) and slot (LDBL_WILDNUM for any).  Unlike ldbl_find_bus(),
 *	this routine will only attempt to scan right from the current
 *	bus structure.
 *
 *	One of the state flags is used to indicate whether the search should
 *	include the bus structure specified.  This is used when the routine 
 *	is called multiple times to restart a search from the last found bus.
 *	Specifying LDBL_STATE_NOINCLUDE prevents you from getting into an 
 *	infinite loop if the search criteria is the same (the current bus 
 *	structure would match and be returned - again).
 *
 * Return Values:
 *	Null	- No match found
 *	Address	- Pointer to matching bus structure
 */

struct bus *
ldbl_search_bus_local (bus_ptr, state, bus_name, bus_num, slot)
   struct bus	*bus_ptr;		/* Starting bus structure	*/
   int		state;			/* Misc. state flags		*/
   char		*bus_name;		/* Name of bus to search for	*/
   int		bus_num;		/* Bus number to search for	*/
   int		slot;			/* Slot number to search for	*/
{
   struct bus	*match1	= NULL,
		*match2	= NULL;
   /*
    * Check to see if we should include the current bus structure in our
    * search or not.  If not, we need to decide what to do next.
    */
   if ((bus_ptr != NULL) && (state & LDBL_SEARCH_NOINCLUDE)) {
      bus_ptr = bus_ptr->nxt_bus;
   }
   /*
    *  Now scan the (rest of the) bus lists for a match.  This is done
    *  by looking for three types of matches.
    *
    *		match1	- This is the primary match in which all three search
    *			  criteria are met.
    *		match2	- This is a match of the bus_name and slot number using
    *			  a wildcarded bus_num.  This allows us to find bus adapters
    *			  with a different bus_num that are in the desired slot.
    */
   while (bus_ptr != NULL) {
      if ((strcmp_EQL (bus_name, LDBL_WILDNAME) || strcmp_EQL (bus_ptr->bus_name, bus_name)) &&
         ((slot == LDBL_WILDNUM) || (bus_ptr->slot == slot)) &&
         ((bus_num == LDBL_WILDNUM) || (bus_ptr->bus_num == bus_num))) {
		match1 = bus_ptr;
		break;
      }
      else if (strcmp_EQL (bus_ptr->bus_name, bus_name)) {
         if (bus_ptr->slot == slot) {
		if (match2 == NULL) match2 = bus_ptr;
         }
      }
      bus_ptr = bus_ptr->nxt_bus;
   }
   /*
    * Determine if we found a match and use the appropriate one.  If nothing
    * was found, match2 should be null.
    */
   if (match1 != NULL) return (match1);
   return (match2);
}

/******************************************************************************
 *
 * ldbl_find_ctlr()
 *
 *	This routine will scan the specified controller list for a controller
 *	structure that matches the ctlr_name (LDBL_WILDNAME for any),
 *	ctlr_num (LDBL_WILDNUM for any) and slot (LDBL_WILDNUM for any).
 *
 * Return Values:
 *	Null	- No match found
 *	Address	- Pointer to matching controller structure
 */

struct controller *
ldbl_find_ctlr (ctlr_ptr, state, ctlr_name, ctlr_num, slot)
   struct controller 	*ctlr_ptr;	/* Starting controller struct	*/
   int			state;		/* Misc. state flags		*/
   char			*ctlr_name;	/* Name of ctlr to search for	*/
   int			ctlr_num;	/* Ctlr number to search for	*/
   int			slot;		/* Slot number to search for	*/
{
   struct controller	*match1	= NULL,
			*match2 = NULL;
   /*
    * Check to see if we should include the current controller structure
    * in our search or not.
    */
   if ((ctlr_ptr != NULL) && (state & LDBL_SEARCH_NOINCLUDE)) {
      ctlr_ptr = ctlr_ptr->nxt_ctlr;
   }
   /*
    *  Now scan the (rest of the) controller lists for a match.  This is done
    *  by looking for three types of matches.
    *
    *		match1	- This is the primary match in which all three search
    *			  criteria are met.
    *		match2	- This is a match of the ctlr_name and slot number using
    *			  a wildcarded ctlr_num.  This allows us to find controllers
    *			  with a different ctlr_num that are in the desired slot.
    */
   while (ctlr_ptr != NULL) {
      if ((strcmp_EQL (ctlr_name, LDBL_WILDNAME) || strcmp_EQL (ctlr_ptr->ctlr_name, ctlr_name)) && 
          ((slot == LDBL_WILDNUM) || (ctlr_ptr->slot == slot)) &&
          ((ctlr_num == LDBL_WILDNUM) || (ctlr_ptr->ctlr_num == ctlr_num))) {
		match1 = ctlr_ptr;
		break;
      }
      else if (strcmp_EQL (ctlr_ptr->ctlr_name, ctlr_name)) {
         if (ctlr_ptr->slot == slot) {
		if (match2 == NULL) match2 = ctlr_ptr;
         }
      }
      ctlr_ptr = ctlr_ptr->nxt_ctlr;
   }
   /*
    * Determine if we found a match and use the appropriate one.  If nothing
    * was found, match2 should be null.
    */
   if (match1 != NULL) return (match1);
   return (match2);
}

/******************************************************************************
 *
 * ldbl_enqueue_devlist()
 *
 *	This routine takes a config entry and places it on the ldbl_devlist
 *	list.  This contains all of the devices that are not currently active
 *	but previously defined.
 *
 *	The enqueuing mechanism requires that explicitly defined devices (those
 *	with a ctlr_name and ctlr_num not wildcarded) be placed on the list first.
 *	After that should come structures with a wildcarded ctlr_num.  Device
 *	structures that have both the ctlr_name and ctlr_num wildcarded should
 *	be placed at the end of the list.
 *
 *	Note:  To provide MP synchronization, we expect to be called with the
 *	       topology tree lock.  While the devlist is technically not part
 *	       of the topology tree, all current callers of this and the other
 *	       devlist routines already have this lock taken.  Therefore we will
 *	       just sleeze our way to MP compliance by assuming it has already
 *	       been taken by our caller.
 *
 * Return Values:
 *	ESUCCESS	- Structure successfully enqueued to the ldbl_devlist.
 */

int
ldbl_enqueue_devlist(dev_ce)
   struct config_entry		*dev_ce;
{
   struct config_entry		*entry_ptr;

   entry_ptr = ldbl_devlist;
   if (entry_ptr == NULL) {
      ldbl_devlist = dev_ce;
      dev_ce->e_next = NULL;
   }
   else {
      if (strcmp_EQL (dev_ce->e_str.e_device.ctlr_name, LDBL_WILDNAME)) {
         /*
          * This entry has a wildcarded ctlr_name.  Therefore we need to 
	  * insert it at the end of the list.
          */
         while (entry_ptr->e_next != NULL)
            entry_ptr = entry_ptr->e_next;
         entry_ptr->e_next = dev_ce;
	 dev_ce->e_next = NULL;
      }
      else if (dev_ce->e_str.e_device.ctlr_num == LDBL_WILDNUM) {
         /*
          * This entry has a wildcarded ctlr_num.  Therfore we need to
	  * insert it as the point where the list turns into entries with 
          * wildcarded ctlr_names and insert this entry before that
	  * point.
          */
         while ((entry_ptr->e_next != NULL) && 
                 strcmp_NEQ (entry_ptr->e_next->e_str.e_device.ctlr_name, LDBL_WILDNAME)) {
			entry_ptr = entry_ptr->e_next;
         }
	 dev_ce->e_next = entry_ptr->e_next;
	 entry_ptr->e_next = dev_ce;
      }
      else {
         /*
          * This is an explicitly defined entry.  Find the point where
	  * the list turns into entries with wildcarded ctlr_nums and
	  * insert this entry before that point.
	  */
         while ((entry_ptr->e_next != NULL) && 
		(entry_ptr->e_next->e_str.e_device.ctlr_num != LDBL_WILDNUM)) {
			entry_ptr = entry_ptr->e_next;
         }
	 dev_ce->e_next = entry_ptr->e_next;
	 entry_ptr->e_next = dev_ce;
      }
   }
   return (ESUCCESS);
}

/******************************************************************************
 *
 * ldbl_find_devlist_entry()
 *
 *	This routine searches the ldbl_devlist for entries with the specified
 *	driver_name.  For each entry found, a check is made to ensure that it is
 *	a device (e_type) and the controller name and number is compared.  If
 *	everything matches, a device structure is kalloc'd and the entry is 
 *	copied to it.  The resulting device structure address is returned.
 *
 *	Note:  This routine expects the ldbl_devlist to be ordered such that
 *	   explicitly defined devices come first, followed by those with
 *	   wildcarded controller numbers, followed by those with wildcarded
 *	   controller numbers and names.
 *
 *	Note Also:  The discussion about MP locking in the ldbl_enqueue_devlist
 *	   routine.
 *
 * Return Values:
 *	ESUCCESS	- Entry found
 *	ENODEV		- No matching device list entry
 *	ENOMEM		- Not enough memory to complete operation
 */

int
ldbl_find_devlist_entry(driver_name, ctlr_name, ctlr_num, dev_entry, context)
   char			*driver_name;	/* Name of controlling driver	*/
   char			*ctlr_name;	/* Controller to search for	*/
   int			ctlr_num;	/* Controller num to search for	*/
   struct device	**dev_entry;	/* Found device structure       */
   caddr_t		*context;	/* Search context holder	*/
{
   int			status		= ENODEV;
   struct config_entry	*entry		= NULL;
   struct device	*dev_ptr	= NULL;
   /*
    * Validate the output parameters.
    */
   if (dev_entry == NULL) panic("Parameter \"dev_entry\" to routine ldbl_find_devlist_entry is NULL.\n");
   if (context   == NULL) panic("Parameter \"context\" to routine ldbl_find_devlist_entry is NULL.\n");
   /*
    * Check the search context holder to see if we should start at the
    * beginning or from some point in the ldbl_devlist.
    */
   if (*context == NULL) {
      entry = ldbl_devlist;
   }
   else {
      entry = (struct config_entry *) *context;
      entry = entry->e_next;
   }
   while ((entry != NULL) && (status != ESUCCESS)) {
      if ((entry->e_type == DEVICE_TYPE) && strcmp_EQL (entry->e_name, driver_name)) {
	 /*
	  * Now search for a device config entry that matches as follows:
	  *	1) The controller name and number match
	  *	2) The controller name matches and the config entry has a wildcarded ctlr_num
	  *	3) The config entry has a wildcarded ctlr_name and ctlr_num
	  */
         if ((strcmp_EQL (entry->e_str.e_device.ctlr_name, ctlr_name) && 
	     	((ctlr_num == entry->e_str.e_device.ctlr_num) || 
	      	 (entry->e_str.e_device.ctlr_num == LDBL_WILDNUM))) ||
             (strcmp_EQL (entry->e_str.e_device.ctlr_name, LDBL_WILDNAME) &&
	     	(entry->e_str.e_device.ctlr_num == LDBL_WILDNUM))) {
		/*
		 * We found an entry.  Create a real device structure and
		 * copy the information to it.
		 */
		dev_ptr = (struct device *) kalloc (sizeof (struct device));
		if (dev_ptr == NULL) return (ENOMEM);

		bcopy (&entry->e_str.e_device, dev_ptr, sizeof (struct device));
		dev_ptr->nxt_dev = NULL;
		dev_ptr->ctlr_hd = NULL;
		dev_ptr->alive   = ALV_LOADABLE  | ALV_NOSIZER;
		
		dev_ptr->dev_name = (char *) kalloc (strlen(entry->e_str.e_device.dev_name) + 1);
		if (dev_ptr->dev_name != NULL) {
		   strcpy (dev_ptr->dev_name, entry->e_str.e_device.dev_name);
		}
		else {
		   ldbl_deallocate_dev (dev_ptr);
		   return (ENOMEM);			/* FIX: Not a really good return value */
		}
		      
		dev_ptr->ctlr_name = (char *) kalloc (strlen(entry->e_str.e_device.ctlr_name) + 1);
		if (dev_ptr->dev_name != NULL) {
		   strcpy (dev_ptr->ctlr_name, entry->e_str.e_device.ctlr_name);
		}
		else {
		   ldbl_deallocate_dev (dev_ptr);
		   return (ENOMEM);			/* FIX: Not a really good return value */
		}
		/*
		 * Return the new device structure to the caller and signal success.
		 */
		*dev_entry = dev_ptr;
		*context = (caddr_t) entry;
		status = ESUCCESS;
         }
      }
      entry = entry->e_next;
   }
   return (status);
}

/******************************************************************************
 *
 * ldbl_cleanup_devlist()
 *
 *	This routine scans the ldbl_devlist structure for entries created for
 *	the specified driver.  Any entry found will be deleted.
 *
 *	Note:  The discussion about MP locking in the ldbl_enqueue_devlist
 *	   routine.
 * 
 * Return Values:
 */

void
ldbl_cleanup_devlist(driver_name)
   char				*driver_name;
{
   struct config_entry		*prev, *entry,
				*entry_to_delete;

   if (driver_name == NULL) return;
   prev = NULL;
   entry = ldbl_devlist;
   while (entry != NULL) {
      if (strcmp_EQL (driver_name, entry->e_name)) {
         /*
          * We found a match.  Remove it from the queue and delete it.
          */
         if (prev != NULL)
            prev->e_next = entry->e_next;
	 else
	    ldbl_devlist = entry->e_next;

	 entry_to_delete = entry;
	 entry = entry->e_next;
         kfree (entry_to_delete, sizeof (struct config_entry));
      }
      else {
	 prev = entry;
         entry = entry->e_next;
      }
   }
}

/******************************************************************************
 *
 * ldbl_deallocate_bus()
 *
 *	This routine takes the supplied bus entry and deallocates it.  This
 *	can sometimes be an involved process since there maybe controllers
 *	and buses connected to it.  If there are, we need to go and delete
 *	all of them also.
 *
 *	A check is also made at the beginning to ensure that the bus
 *	structure is not active and was created by loadable code.  If either
 *	of these cases is not true we will panic the system.  We should
 *	never ever be attempting to delete a bus that is active.  Also, if
 *	the structure was not created by loadable code we should not be
 *	able to get here.  In either case we have a fatal bug.
 *	
 *	Note:  When deleting the character strings, it is assumed that the
 *	connect_name (parent bus name) is not private to this bus structure 
 *	but is, instead, the address copied from the parent's bus structure 
 *	bus_name field - hence it will not be touched.
 *
 * Return Values:
 *	- None -
 */
void
ldbl_deallocate_bus (bus)
   struct bus		*bus;
{
   struct bus		*bp, *curr_bus, *parent_bus;
   struct controller	*cp, *curr_ctlr;

   if (bus->alive & ALV_ALIVE) panic ("Loadable driver attempting to delete bus structure that is currently active!");
   if (!(bus->alive & ALV_NOSIZER)) panic ("Loadable driver attempting to delete status bus structure!");
   /*
    * If there are any controllers connected to this bus, go delete them.
    */
   cp = bus->ctlr_list;
   while (cp != NULL) {
      curr_ctlr = cp;
      cp = cp->nxt_ctlr;
      ldbl_deallocate_ctlr (curr_ctlr);
   }
   /*
    * If there are any buses connected to this bus, go delete them.
    */
   bp = bus->bus_list;
   while (bp != NULL) {
      curr_bus = bp;
      bp = bp->nxt_bus;
      ldbl_deallocate_bus (curr_bus);
   }
   /*
    * Now check to see if the parent bus (bus_hd) is specified.  If
    * so, we need to go and remove this bus from the parent buses bus_list.
    */
   parent_bus = bus->bus_hd;
   if ((parent_bus != NULL) && (parent_bus->bus_list != NULL)) {
      if (parent_bus->bus_list == bus)
         parent_bus->bus_list = bus->nxt_bus;
      else {
         bp = parent_bus->bus_list;
         while (bp != NULL) {
            if (bp->nxt_bus == bus) {
	       bp->nxt_bus = bus->nxt_bus;
	       bp = NULL;
            }
            else
               bp = bp->nxt_bus;
         }
      }
   }
   /*
    * Now delete all of the local character strings.
    */
   if (bus->bus_name != NULL) kfree (bus->bus_name, strlen(bus->bus_name) + 1);
   if (bus->pname    != NULL) kfree (bus->pname,    strlen(bus->pname)    + 1);
   if (bus->port     != NULL) kfree (bus->port,     sizeof(struct port)      );
   kfree (bus, sizeof (struct bus));
}

/******************************************************************************
 *
 * ldbl_deallocate_ctlr()
 *
 *	This routine takes the supplied controller entry and deallocates it.  
 *
 *	Note:  When deleting the character strings, it is assumed that the 
 *	bus_name (parent bus name) is not private to this controller structure 
 *	but is, instead, the address copied from the parent buses bus structure 
 *	bus_name field - hence it will not be touched.
 *
 * Return Values:
 *	- None -
 */
void
ldbl_deallocate_ctlr (ctlr)
   struct controller	*ctlr;
{
   struct bus		*parent_bus;
   struct controller	*cp;

   if (ctlr->alive & ALV_ALIVE) panic ("Loadable driver attempting to delete controller structure that is currently active!");
   if (!(ctlr->alive & ALV_NOSIZER)) panic ("Loadabler driver attempting to delete static controller structure!");
   /*
    * Check to see if the parent bus (bus_hd) is specified.  If so, we
    * need to go and remove this controller from the parent bus' ctlr_list.
    */
   parent_bus = ctlr->bus_hd;
   if ((parent_bus != NULL) && (parent_bus->ctlr_list != NULL)) {
      if (parent_bus->ctlr_list == ctlr)
         parent_bus->ctlr_list = ctlr->nxt_ctlr;
      else {
         cp = parent_bus->ctlr_list;
         while (cp != NULL) {
            if (cp->nxt_ctlr == ctlr) {
               cp->nxt_ctlr = ctlr->nxt_ctlr;
               cp = NULL;
            }
            else
               cp = cp->nxt_ctlr;
         }
      }
   }
   /*
    * Now delete all of the local character strings.
    */
   if (ctlr->ctlr_name != NULL) kfree (ctlr->ctlr_name, strlen(ctlr->ctlr_name) + 1);
   if (ctlr->pname     != NULL) kfree (ctlr->pname,     strlen(ctlr->pname)     + 1);
   if (ctlr->port      != NULL) kfree (ctlr->port,      sizeof(struct port)        );
   kfree (ctlr, sizeof (struct controller));
}

/******************************************************************************
 *
 * ldbl_deallocate_dev()
 *
 *	This routine takes the supplied device entry and deallocates it.  This
 *	is done normally when a controller is unloaded or a specific device is
 *	taken offline.
 *
 *	As in the other two deallocate routines, the ctlr_name will not be
 *	touched since it is the address from the parent controller's ctlr_name
 *	field.
 *
 * Return Values:
 *	- None -
 */
void
ldbl_deallocate_dev (dev)
   struct device	*dev;
{
   struct controller	*parent_ctlr;
   struct device	*dp;

   if (dev->alive & ALV_ALIVE) panic ("Loadable driver attempting to delete device structure that is currently active!");
   if (!(dev->alive & ALV_NOSIZER)) panic ("Loadable driver is attempting to delete a static device structure!");
   /*
    * Check to see if the parent controller (ctlr_hd) is specified.  If so,
    * we need to go and remove this device from the parent controller's
    * dev_list.
    */
   parent_ctlr = dev->ctlr_hd;
   if ((parent_ctlr != NULL) && (parent_ctlr->dev_list != NULL)) {
      if (parent_ctlr->dev_list == dev)
         parent_ctlr->dev_list = dev->nxt_dev;
      else {
         dp = parent_ctlr->dev_list;
         while (dp != NULL) {
            if (dp->nxt_dev == dev) {
               dp->nxt_dev = dev->nxt_dev;
               dp = NULL;
            }
            else
               dp = dp->nxt_dev;
         }
      }
   }
   /*
    * Now delete any local character strings.
    */
   if (dev->dev_name != NULL) kfree (dev->dev_name, strlen(dev->dev_name) + 1);
   kfree (dev, sizeof (struct device));
}

/************************************************************************
 *
 * print_hardware_topology()
 *
 *	This routine scans the hardware topology pointed to by system_bus
 *	and prints it out.
 */
void
print_hardware_topology()
{
   int			level = 0;
   struct bus		*bus_ptr, *prev_bus;
   struct controller	*ctlr_ptr;
   struct device	*dev_ptr;
   struct config_entry	*device;

   prev_bus = bus_ptr = ldbl_find_bus (system_bus, LDBL_SEARCH_ALL, LDBL_WILDNAME, LDBL_WILDNUM);
   while (bus_ptr != NULL) {
      indent_level (level);
      print_bus_entry (bus_ptr);

      ctlr_ptr = ldbl_find_ctlr (bus_ptr->ctlr_list, LDBL_SEARCH_ALL, LDBL_WILDNAME, LDBL_WILDNUM, LDBL_WILDNUM);
      if (ctlr_ptr != NULL) {
         level += 1;
         while (ctlr_ptr != NULL) {
            indent_level (level);
	    print_ctlr_entry (ctlr_ptr);
	    dev_ptr = ctlr_ptr->dev_list;
	    if (dev_ptr != NULL) {
	       level += 1;
	       while (dev_ptr != NULL) {
	          indent_level (level);
	          print_dev_entry (dev_ptr);
		  dev_ptr = dev_ptr->nxt_dev;
	       }
	       level -= 1;
	    }  
	    ctlr_ptr = ldbl_find_ctlr (ctlr_ptr, LDBL_SEARCH_NOINCLUDE, LDBL_WILDNAME, LDBL_WILDNUM, LDBL_WILDNUM);
         }
	 level-= 1;
      }
      bus_ptr = ldbl_find_bus (bus_ptr, LDBL_SEARCH_NOINCLUDE, LDBL_WILDNAME, LDBL_WILDNUM);	 
   }
   printf ("\nLoadable Device List:\n");
   for (device = ldbl_devlist; device != NULL; device = device->e_next) {
      printf ("	");
      print_dev_entry (&device->e_str.e_device);
   }
}

/************************************************************************
 *
 * indent_level()
 *
 *	Print spaces to indent a level.
 */
static void
indent_level(level)
{
   int i;

   for (i = 0; i < level; i++)
      printf ("     ");
}

/*
 * Debugging routine to print out some fields of a bus structure.
 */
void
print_bus_entry(bp)
	struct bus *bp;
{
	printf("BUS entry: %s",bp->bus_name);
	if (bp->bus_num == -99)
	   printf ("?, ");
	else
	   printf ("%d, ", bp->bus_num);

	printf("slot ");
	if (bp->slot == -99) {
		printf("? ,");
	}
	else {
		printf("%d, ",bp->slot);
	}
	printf("to bus ");
	if (strlen(bp->connect_bus) > 0) {
		printf("%s",bp->connect_bus);
	}
	else if (bp->connect_num == (int)NEXUS_NUMBER) {
		printf ("nexus ");
	}
	else {
		printf("unknown ");
	}
	if ((bp->connect_num != (int)NEXUS_NUMBER) && (bp->connect_num != -99) &&
	    (strlen(bp->connect_bus) > 0)) {
		printf("%d",bp->connect_num);
	}
	if (bp->alive & ALV_ALIVE) printf (", ALIVE");
	if (bp->alive & ALV_PRES) printf (", PRES");
        if (bp->alive & ALV_LOADABLE) printf (", LOADABLE");
        if (bp->alive & ALV_NOSIZER) printf (", NOSIZER");
	printf("\n");
}
/*
 * Debugging routine to print out some fields of a controller structure.
 */
void
print_ctlr_entry(cp)
	struct controller *cp;
{
	printf("CONTROLLER entry: %s",cp->ctlr_name);
	if (cp->ctlr_num == -99) 
	   printf ("?, ");
	else
	   printf ("%d, ", cp->ctlr_num);

	printf("slot ");
	if (cp->slot == -99) {
		printf("? ,");
	}
	else {
		printf("%d, ",cp->slot);
	}
	printf("to bus ");
	if (strlen(cp->bus_name) > 0) {
		printf("%s",cp->bus_name);
		if (strcmp_NEQ (cp->bus_name, LDBL_WILDNAME)) {
	           if (cp->bus_num == LDBL_WILDNUM)
		      printf ("?");
                   else if (cp->bus_num != (int)NEXUS_NUMBER)
		      printf("%d",cp->bus_num);
                }
	}
	else {
		printf("unknown ");
	}
	if (cp->alive & ALV_ALIVE) printf (", ALIVE");
	if (cp->alive & ALV_PRES) printf (", PRES");
        if (cp->alive & ALV_LOADABLE) printf (", LOADABLE");
        if (cp->alive & ALV_NOSIZER) printf (", NOSIZER");
	printf("\n");
}
/*
 * Debugging routine to print out some fields of a device structure.
 */
void
print_dev_entry(dp)
	struct device *dp;
{
	printf("DEVICE entry: ");
	if (strlen(dp->dev_type) > 0) {
		printf("%s ",dp->dev_type);
	}
	printf("%s logical unit %d, physical unit %d, ",
			dp->dev_name, dp->logunit, dp->unit);
	printf("to ctlr ");
	if (strlen(dp->ctlr_name) > 0) {
		printf("%s",dp->ctlr_name);
		if (strcmp(dp->ctlr_name,"*")) {
			printf("%d",dp->ctlr_num);
		}
	}
	if (dp->alive & ALV_ALIVE) printf (", ALIVE");
	if (dp->alive & ALV_PRES) printf (", PRES");
        if (dp->alive & ALV_LOADABLE) printf (", LOADABLE");
        if (dp->alive & ALV_NOSIZER) printf (", NOSIZER");
	printf("\n");
}
