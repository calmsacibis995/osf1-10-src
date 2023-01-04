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
static char *rcsid = "@(#)$RCSfile: stanza_resolver.c,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/04/23 12:54:33 $";
#endif
*/
/*
 * This module contains the routines necessary to resolve the data structures
 * created by cfgmgr (BUS, CONTROLLER and DEVICE structures) against the
 * current hardware topology in memory (initially created by static
 * configuration.
 */

#include <sys/errno.h>
#include <sys/types.h>

#include <io/common/devdriver.h>
#include <io/common/devdriver_loadable.h>

/* Define all globally declared routines */

int			ldbl_stanza_resolver();

/* Define all of the local routines */

int			locate_and_merge_bus();
int			merge_bus_structures();
void			merge_bus_error();
int			locate_and_merge_ctlr();
int			merge_ctlr_structures();
int			seek_ctlr_conflict();
void			merge_ctlr_error();
int			find_next_bus_num();
int			find_next_ctlr_num();
struct config_entry *	transfer_bus_module_list();
struct config_entry *	transfer_ctlr_module_list();

/******************************************************************************
 *
 * ldbl_stanza_resolver()
 *
 *	This routine is used to merge the configuration data supplied in a
 *	stanza entry (and processed by the cfgmgr daemon) into the hardware
 *	topology tree created at static configuration time.  It will also
 *	call a bus specific resolver routine that can be used to finish
 *	autoconfiguration.
 *
 * Return Values:
 *	ESUCCESS	- Successful
 *	ENOMEM		- Unable to allocate memory
 *	LDBL_ENOBUS	- Parent bus does not exist
 */

int
ldbl_stanza_resolver (driver_name, bus_name, driver_struct, bus_param)
   char			*driver_name;	/* Name of driver from cfgmgr	    */
   char			*bus_name;	/* Name of bus(es) to connect to    */
   struct driver 	*driver_struct;	/* Driver's driver struct	    */
   caddr_t	 	*bus_param;	/* Bus Specific Parameter	    */
{
   int			status;		/* Return status from routines      */
   struct config_entry	*config_bp,	/* Bus structures from module_list  */
			*config_cp,	/* Ctlr structures from module_list */
			*config_dp,	/* Dev. structures from module_list */
			*entry_to_delete;
   struct bus		*bus_ptr;	/* Bus from hardware topology tree  */
   /*
    * Lock the hardware topology tree.
    */
   LOCK_TOPOLOGY_TREE;

   /*
    * The first thing we need to do is call the bus specific config_resolver()
    * routine.  This routine is responsible for handling any autoconfiguration
    * issues that might exist.  For example, non-active slots on the bus could
    * be re-probed to ensure the configuration.  In addition, there must be
    * controller structures present in the hardware topology tree in order for
    * autoconfiguration to work.  The bus specific config_resolver() routine
    * is responsible for doing this.
    *
    * Notice that the config_resolver() routine will be called once for every
    * bus found with the desired name.
    */
   bus_ptr = ldbl_find_bus (system_bus, LDBL_SEARCH_ALL, bus_name, LDBL_WILDNUM);
   if (bus_ptr == NULL) {
      UNLOCK_TOPOLOGY_TREE;
      return (LDBL_ENOBUS);
   }
   while (bus_ptr != NULL) {
      if (bus_ptr->framework != NULL) {
         if (bus_ptr->alive & ALV_ALIVE) {
            if (bus_ptr->framework->config_resolver != NULL) {
	       if (status = (bus_ptr->framework->config_resolver)(bus_ptr, bus_param)) {
	          printf ("LDBL: Failure to config resolve bus %s%d, status = %d\n",
		           bus_ptr->bus_name, bus_ptr->bus_num, status);
		  UNLOCK_TOPOLOGY_TREE;
                  return status;
	       }
	    }
	    else {
	       Dprintf ("LDBL: Warning - Bus %s%d does not have a config resolver routine.\n",
			 bus_ptr->bus_name, bus_ptr->bus_num);
	    }
         }
      }
      else {
	 printf ("LDBL: Cannot resolve bus %s%d, bus has no loadable support.\n",
		  bus_ptr->bus_name, bus_ptr->bus_num);
      }
      bus_ptr = ldbl_find_bus (bus_ptr, LDBL_SEARCH_NOINCLUDE, bus_name, LDBL_WILDNUM);
   }
   /*
    * Okay, we are now going to process all of the bus structures on the
    * ldbl_module_list.  To handle wildcard problems, we need to sort the
    * entries based upon which fields are specified and which are wildcarded.
    * This way, if we need to create any bus structures that don't have a
    * bus number defined (it's wildcarded), we can search the hardware tree
    * to determine which number to use.
    *
    * Once the structures are sorted, we will start merging them into the
    * hardware topology tree.  This is done by searching the tree for matches
    * (either explicit or via wildcarding).  For each match found, and there can
    * be multiple if wildcarded, the information present in the one from the 
    * ldbl_module_list will be copied to the one(s) in the topology tree.  If
    * there is no match and the structure does not use wildcarding for the
    * connection bus name and bus number, a bus structure will be allocated, 
    * hooked into the tree and the data copied.
    *
    * When all searching is completed, the bus structure from the 
    * ldbl_module_list will be deallocated.
    */
   config_bp = transfer_bus_module_list (driver_name);
   while (config_bp != NULL) {
      Dprint_bus ("LDBL: Processing", &config_bp->e_str.e_bus);
      status = locate_and_merge_bus (config_bp);
      if ((status != ESUCCESS) && (status != LDBL_ECONFLICT)) {
         /*
          * Since an error occurred, we need to deallocate the bus
	  * structures we moved from ldbl_module_list.
	  */
	 while (config_bp != NULL) {
            entry_to_delete = config_bp;
	    config_bp = config_bp->e_next;
	    kfree (entry_to_delete, sizeof (struct config_entry));
         };
	 UNLOCK_TOPOLOGY_TREE;
         return (status);
      }
      else {
         entry_to_delete = config_bp;
         config_bp = config_bp->e_next;
         kfree (entry_to_delete, sizeof (struct config_entry));
      }
   }
   /*
    * Now process all of the controller structures on the ldbl_module_list. 
    * Again, to handle wildcard problems, we need to sort the entries based
    * upon which fields are specified and which are wildcarded.  Everything
    * is done essentially the same as for buses.
    */
   config_cp = transfer_ctlr_module_list (driver_name);
   while (config_cp != NULL) {
      Dprint_ctlr("LDBL: Processing", &config_cp->e_str.e_controller);
      status = locate_and_merge_ctlr (config_cp, driver_struct);
      if ((status != ESUCCESS) && (status != LDBL_ECONFLICT)) {
         /*
          * Since an error occurred, we need to deallocate the controller
	  * structures we moved from ldbl_module_list.
	  */
	 while (config_cp != NULL) {
            entry_to_delete = config_cp;
	    config_cp = config_cp->e_next;
	    kfree (entry_to_delete, sizeof (struct config_entry));
         };
	 UNLOCK_TOPOLOGY_TREE;
         return (status);
      }
      else {
         entry_to_delete = config_cp;
         config_cp = config_cp->e_next;
         kfree (entry_to_delete, sizeof (struct config_entry));
      }
   }
   /*
    * Lastly, take all of the device structures that exist and move them to
    * the ldbl_devlist.  This contains only devices and is parsed by the
    * controller configuration code.
    */
   while ((config_dp = get_config_entry (driver_name, DEVICE_TYPE)) != NULL) {
      ldbl_enqueue_devlist (config_dp);
   }
   /*
    * Everything is done.  Any errors would get trapped up above so we can
    * simply exit with success.
    */
   UNLOCK_TOPOLOGY_TREE;
   return (ESUCCESS);
}

/******************************************************************************
 *
 * locate_and_merge_bus()
 *
 *	This routine takes a given ldbl_module_list bus structure and merges
 *	the contents into the hardware topology tree.  For discussion, a bus
 *	structure found in the ldbl_module_list is referred to as an LBS.
 *
 *	Since each LBS may map onto multiple bus structures in the hardware
 *	topology tree (via wildcarding), the entire tree will be searched to
 *	find all matches.  The criteria used for matching will be taken
 *	directly out of the LBS (bus name/number, connecting bus name/number).
 *	If a match occurs, the information in the LBS is copied to the one
 *	found from the hardware topology.  If no match occurs and the LBS
 *	explicitly defines a connecting bus name, number and slot then a
 *	bus structure will be allocated and connected to the configuration.
 *
 *	For each structure found (or created) and used, the ALV_LOADABLE flag
 *	will be set to indicate that the resolver has processed it.  This 
 *	prevents the resolver from finding it again should a more generic 
 *	wildcard search occur.
 *
 *	There are also instances when a bus number needs to be assigned.  
 *	This can occur when an LBS does not explicitly specify a bus number
 *	and the bus structure from the hardware topology has a wildcarded bus
 *	number (always true when the structure needed to be created or is 
 *	created by bus probing).
 *
 * Return Values:
 *	ESUCCESS	- Successfull
 *	ENOMEM		- No memory to complete operation
 */
static int
locate_and_merge_bus (config_bp)
   struct config_entry	*config_bp;	/* Bus entry to scan		    */
{
   int			explicit_bus;
   int			found_match = FALSE;
   int			status = ESUCCESS;
   struct bus		*bus_ptr,
			*parent_bus;
   /*
    * The first thing we need to do is check to see if the LBS has a
    * defined parent bus name.  If so, check the rest of the configuration 
    * for any name conflicts.  This can happen when an adapter driver is 
    * unloaded and the stanza entry is changed.
    *
    * Assumption: By definition the bus_name cannot be wildcarded in
    *	   the LBS since we would not have any clue as to what to look
    *	   for.
    */
   if (strcmp_NEQ (config_bp->e_str.e_bus.connect_bus, LDBL_WILDNAME)) {
      if (status = seek_bus_conflict (&config_bp->e_str.e_bus))
         return (status);
   }
   /*
    * Okay, we now need to see if there is/are (an) existing bus 
    * structure(s) that match the LBS.  If so, we can merge the LBS
    * into the structure(s) that we find.
    *
    * Note: Since the LBS may contain wildcards, we need to loop until
    *	    we don't find any more structures.
    */
   explicit_bus = (strcmp_NEQ (config_bp->e_str.e_bus.connect_bus, LDBL_WILDNAME) &&
		  (config_bp->e_str.e_bus.connect_num != LDBL_WILDNUM) &&
		  (config_bp->e_str.e_bus.slot != LDBL_WILDNUM));

   parent_bus = ldbl_find_bus (system_bus, LDBL_SEARCH_ALL,
			       config_bp->e_str.e_bus.connect_bus, 
			       config_bp->e_str.e_bus.connect_num);
   if (parent_bus == NULL) {
      merge_bus_error (&config_bp->e_str.e_bus, MERGE_BUS_NOPARENT);
      return (ESUCCESS);
   }

   while (parent_bus != NULL) {
      /*
       * Okay, we found a bus that matches against what config told us.  Look
       * to see if we can find any local buses that match against the LBS.
       * Remember, ldbl_search_bus_local() will first try to find an exact
       * match.  If that fails and the slot number is specified (not wildcarded),
       * it will try again using a wildcarded bus_num.  This allows it to
       * find a bus structure that has a wildcarded bus_num when we have
       * explicitly specified a bus_num.
       */
      Dprintf ("LDBL: Checking against bus %s%d\n", parent_bus->bus_name, parent_bus->bus_num);
      bus_ptr = ldbl_search_bus_local (parent_bus, LDBL_SEARCH_ALL, 
				       config_bp->e_str.e_bus.bus_name,
				       config_bp->e_str.e_bus.bus_num,
				       config_bp->e_str.e_bus.slot);
      /*
       * If we didn't find a match and the connection information is explicitly
       * defined, check to see if there is any adapter in the slot.  This allows
       * us to handle the case where the bus name has changed but a structure for
       * that slot exists.  If we don't do this we could end up creating duplicate
       * structures for the slot.
       *
       * Note: We only need do this at the beginning since there can only be
       *       one structure with the specified slot number.
       */
      if ((bus_ptr == NULL) && explicit_bus) {
	 bus_ptr = ldbl_search_bus_local (parent_bus, LDBL_SEARCH_ALL, 
				          LDBL_WILDNAME, LDBL_WILDNUM, 
				          config_bp->e_str.e_bus.slot);
      }
      /*
       * If we still didn't find anything, try one last gasp effort.  If the
       * bus_num is NOT wildcarded and the slot number IS wildcarded, look 
       * for a bus adapter that has its bus_num explicitly set to LDBL_WILDNUM.
       * This allows us to handle the case where the user specified the
       * bus_num but not where it is connected.  Since the bus routines may
       * leave the bus_num equal to LDBL_WILDNUM for structures they create
       * we would not find them.
       *
       * Note:  We only want to do this once since there can only be one
       *        possible match.
       */
      if ((bus_ptr == NULL) && 
         (config_bp->e_str.e_bus.bus_num != LDBL_WILDNUM) &&
	 (config_bp->e_str.e_bus.slot == LDBL_WILDNUM)) {
		bus_ptr = ldbl_search_bus_local (bus_ptr->bus_list, LDBL_SEARCH_ALL,
					         config_bp->e_str.e_bus.bus_name,
					         LDBL_WILDNUM, LDBL_WILDNUM);
		while (bus_ptr != NULL) {
		   if (bus_ptr->bus_num == LDBL_WILDNUM)
		      break;
		   bus_ptr = ldbl_search_bus_local (bus_ptr, LDBL_SEARCH_NOINCLUDE,
					            config_bp->e_str.e_bus.bus_name,
					            LDBL_WILDNUM, LDBL_WILDNUM);
		}
      }
      while (bus_ptr != NULL) {
	 /*
	  * This bus fully matches the one we are searching for.  If it is
	  * not active AND has not been previously processed by the resolver,
	  * then we can fillin the structure from the LBS.
	  */
	 Dprintf ("LDBL:    Found bus %s%d, slot %d\n", bus_ptr->bus_name, bus_ptr->bus_num, bus_ptr->slot);
	 if ((bus_ptr->alive & ALV_PRES) && (!(bus_ptr->alive & ALV_LOADABLE))) {
	    /*
	     * One last check to see if the parent bus actually has support
	     * for loadable drivers.
	     */
            if (parent_bus->framework != NULL) {
	       /*
		* Merge the structures and then set the ALV_LOADABLE bit so
		* we won't try and reconfigure this bus at a future time.  Also,
		* indicate that we found a match during the search.
		*
		* Note: We don't touch the connect_bus and connect_num since they
		*	are presumeably correct.
		*/
	       if (status = merge_bus_structures (config_bp, bus_ptr)) {
		  return (status);
	       }
	       bus_ptr->alive |= ALV_LOADABLE | ALV_NOSIZER;
	       found_match = TRUE;
            }
	    else {
	       /*
		* We can't add the requested bus as loadable since its parent bus
		* does not contain loadable support.
		*/
	       merge_bus_error (bus_ptr, MERGE_BUS_NOSUPPORT);
	    }
         }
         else {
            /*
             * A match was found but it was already configured.  If we got here
	     * because the entry was wildcarded, ignore it.  Otherwise output a 
	     * warning and continue - this is not a fatal error.
	     */
	    if (explicit_bus) {
	       merge_bus_error (bus_ptr, MERGE_BUS_INUSE);
	       return (ESUCCESS);
	    }
         }
	 /*
	  * See if there are any other adapters.  Note that the initial entry
	  * into this loop uses three attempts to find a bus adapter.  This next
	  * call is only useful if the first one found a bus structure.  If either
	  * of the other two calls were used to find a bus structure then this call
	  * should not find anything.
	  */
         bus_ptr = ldbl_search_bus_local (bus_ptr, LDBL_SEARCH_NOINCLUDE,
                                          config_bp->e_str.e_bus.bus_name,
                                          config_bp->e_str.e_bus.bus_num,
                                          config_bp->e_str.e_bus.slot);
      }
      parent_bus = ldbl_find_bus (parent_bus, LDBL_SEARCH_NOINCLUDE,
			          config_bp->e_str.e_bus.connect_bus, 
			          config_bp->e_str.e_bus.connect_num);
   }
   /*
    * If we didn't find any matches (found_match is false), check to see if
    * this was an explicit declaration.  If it was then we need to create the
    * bus structure and hook it into the configuration.
    */
   if (found_match == FALSE) {
      Dprintf ("LDBL:	No entry found...");
      if (explicit_bus) {
	 Dprintf ("creating one\n");
	 parent_bus = ldbl_find_bus (system_bus, LDBL_SEARCH_ALL,
				     config_bp->e_str.e_bus.connect_bus,
				     config_bp->e_str.e_bus.connect_num);
         if ((bus_ptr = (struct bus *) kalloc (sizeof (struct bus))) == NULL) {
	    status = ENOMEM;
	 }
	 else {
            bzero (bus_ptr, sizeof (struct bus));
	    if (!(status = merge_bus_structures (config_bp, bus_ptr))) {
	       bus_ptr->connect_bus = parent_bus->bus_name;
	       bus_ptr->connect_num = parent_bus->bus_num;
	       bus_ptr->alive = ALV_LOADABLE | ALV_NOSIZER;
	       conn_bus (parent_bus, bus_ptr);
            }
	 }
      }
      else {
         Dprintf ("and not explicitly defined... skipping\n");
      }
   }
   return (status);
}

/******************************************************************************
 *
 * merge_bus_structures()
 *
 *	This routine does the actual merge between an LBS and a bus structure
 *	found on the hardware topology tree.
 *
 * Return Value:
 *	ESUCCESS	- Successful
 *	ENOMEM		- memory could not be allocated
 */
static int
merge_bus_structures (config_bp, bus_ptr, driver_name)
   struct config_entry	*config_bp;	/* Merge from		      */
   struct bus		*bus_ptr;	/* Merge to		      */
   char			*driver_name;	/* Name of controlling driver */
{
   int			update_subs = FALSE;
   struct bus		*bp;		/* Used to update sub buses   */
   struct controller	*cp;		/* Used to update sub ctlrs   */
   /*
    * Since there are at least two ways for bus structures to be created
    * (autoconfiguration and by the resolver), we need to be check some of the
    * fields to see if we should merge them or not.  For example, if the LBS
    * wildcarded some fields which were filled in in the bus structure we 
    * wouldn't want to try and merge those fields since they are already
    * correct.
    *
    * The following fields are not merged (just zerod):
    *	bus_type	- Filled in by the ldbl_bus_configure() routine
    *   connect_bus	- Filled in by the caller
    *   connect_num	- Filled in by the called
    *	confl1		- Filled in by the ldbl_bus_configure() routine
    *	confl2		- Filled in by the ldbl_bus_configure() routine
    *	intr		- Filled in by bus specific routines
    *	alive		- Calling routines responsibility
    *   framework	- Filled in by the ldbl_bus_configure() routine
    *	private		- Filled in by bus specific routines
    *
    * The following fields are not touched.  They may contain information
    * placed there when the bus structure was created:
    *	conn_priv	- Possibly used by connecting bus specific routines
    *	rsvd		- Not currently used
    *
    * Plus the headers at the top of the structure which are filled in as
    * items are connected to the hardware topology tree.
    */

   /*
    * First make sure the bus name is correct.
    */
   if (strcmp_NEQ (config_bp->e_str.e_bus.bus_name, LDBL_WILDNAME) &&
     ((bus_ptr->bus_name == NULL) ||
       strcmp_NEQ (config_bp->e_str.e_bus.bus_name, bus_ptr->bus_name))) {
         if (bus_ptr->bus_name != NULL) kfree (bus_ptr->bus_name, strlen (bus_ptr->bus_name) + 1);
         bus_ptr->bus_name = (char *) kalloc (strlen (config_bp->e_str.e_bus.bus_name) + 1);
	 if (bus_ptr->bus_name != NULL) {
	    strcpy (bus_ptr->bus_name, config_bp->e_str.e_bus.bus_name);
	    update_subs = TRUE;
	 }
	 else {
	    return (ENOMEM);
	 }
   }
   /*
    * Determine where the bus number is coming from.  If it isn't
    * defined in the LBS or the found bus structure we must search the
    * entire configuration for the current maximum value and use
    * the next one.
    */
   if ((config_bp->e_str.e_bus.bus_num == LDBL_WILDNUM) && (bus_ptr->bus_num == LDBL_WILDNUM)) {
      bus_ptr->bus_num = find_next_bus_num (bus_ptr->bus_name);
      update_subs = TRUE;
   }
   else if (config_bp->e_str.e_bus.bus_num != LDBL_WILDNUM) {
      bus_ptr->bus_num = config_bp->e_str.e_bus.bus_num;
      update_subs = TRUE;
   }

   if ((config_bp->e_str.e_bus.slot != LDBL_WILDNUM) &&
       (config_bp->e_str.e_bus.slot != bus_ptr->slot)) {
         bus_ptr->slot = config_bp->e_str.e_bus.slot;
   }
   /*
    * If the bus name or number was changed then we need to update 
    * any buses and controllers that may be connected to it.
    */
   if (update_subs) {
      bp = ldbl_search_bus_local (bus_ptr, LDBL_SEARCH_ALL, LDBL_WILDNAME, LDBL_WILDNUM, LDBL_WILDNUM);
      while (bp != NULL) {
         bp->connect_bus = bus_ptr->bus_name;
         bp->connect_num = bus_ptr->bus_num;
         bp = ldbl_search_bus_local (bp, LDBL_SEARCH_NOINCLUDE, LDBL_WILDNAME, LDBL_WILDNUM, LDBL_WILDNUM);
      }

      cp = ldbl_find_ctlr (bus_ptr->ctlr_list, LDBL_SEARCH_ALL, LDBL_WILDNAME, LDBL_WILDNUM, LDBL_WILDNUM);
      while (cp != NULL) {
         cp->bus_name = bus_ptr->bus_name;
         cp->bus_num  = bus_ptr->bus_num;
         cp = ldbl_find_ctlr (cp, LDBL_SEARCH_NOINCLUDE, LDBL_WILDNAME, LDBL_WILDNUM, LDBL_WILDNUM);
      }
   }
   /*
    * Fillin the port stuff.
    */
   if (strcmp_NEQ (config_bp->e_str.e_bus.pname, LDBL_WILDNAME) &&
     ((bus_ptr->pname == NULL) ||
       strcmp_NEQ (config_bp->e_str.e_bus.pname, bus_ptr->pname))) {
         if (bus_ptr->pname != NULL) kfree (bus_ptr->pname, strlen (bus_ptr->pname) + 1);
         bus_ptr->pname = (char *) kalloc (strlen (config_bp->e_str.e_bus.pname) + 1);
	 if (bus_ptr->pname != NULL) {
	    strcpy (bus_ptr->pname, config_bp->e_str.e_bus.pname);
	 }
	 else {
	    return (ENOMEM);
	 }
   }

   if ((config_bp->e_str.e_bus.port != NULL) &&
       (config_bp->e_str.e_bus.port != bus_ptr->port)) {
         kfree (bus_ptr->port, sizeof (struct port));
         bus_ptr->port = (struct port *) kalloc (sizeof (struct port));
	 if (bus_ptr->port != NULL) {
            bcopy (config_bp->e_str.e_bus.port, bus_ptr->port, sizeof (struct port));
	 }
	 else {
	    return (ENOMEM);
	 }
   }
   /*
    * Setup the driver name.
    */
   if ((bus_ptr->driver_name == NULL) ||
       strcmp_NEQ (driver_name, bus_ptr->driver_name)) {
         if (bus_ptr->driver_name != NULL) 
	     kfree (bus_ptr->driver_name, strlen (bus_ptr->driver_name) + 1);
         bus_ptr->driver_name = (char *) kalloc (strlen (driver_name) + 1);
	 if (bus_ptr->driver_name != NULL)
	    strcpy (bus_ptr->driver_name, driver_name);
	 else {
	    return (ENOMEM);
	 }
   }
   /*
    * Zero out the remaining fields.
    */
   bus_ptr->confl1	= NULL;
   bus_ptr->confl2	= NULL;
   bus_ptr->intr	= NULL;
   bus_ptr->framework	= NULL;

   bus_ptr->private[0]	= NULL;
   bus_ptr->private[1]	= NULL;
   bus_ptr->private[2]	= NULL;
   bus_ptr->private[3]	= NULL;
   bus_ptr->private[4]	= NULL;

   return (ESUCCESS);
}  

/******************************************************************************
 *
 * seek_bus_conflict()
 *
 *	This routine checks the configuration for any conflicts that might
 *	exist.  This can happen if a bus was defined (by a previous driver
 *	loading) and is now being reloaded after a stanza entry change.
 *
 *	For example: if bus foo1 on bus tc0 slot 3 previously existed, and
 *	then the stanza entry was changed to say that bus foo1 is now on
 *	bus tc1 slot 1 - we have a name conflict.
 *
 *	Upon reload, the previously existing bus structure on bus tc0 needs
 *	to be located and the bus_num needs to be changed to LDBL_WILDNUM.
 *	This will allow us to use the bus_num on the new bus that will be
 *	created (or redefined).
 *
 * Return Value:
 *	ESUCCESS	- Successful
 *	LDBL_ECONFLICT	- Existing active adapter or controller will prevent 
 *			  merging of explicit bus declaration.
 */
static int
seek_bus_conflict(new_bus)
   struct bus		*new_bus;
{
   struct bus		*bus;
   struct controller	*cp;

   bus = ldbl_find_bus (system_bus, LDBL_SEARCH_ALL, LDBL_WILDNAME, LDBL_WILDNUM);
   while (bus != NULL) {
      /*
       * Okay, we need to determine if this bus conflicts with the LBS in one
       * of four ways:
       *	1) It has the same bus name and number but a different parent bus name
       *	2) It has the same bus name and number but a different parent bus number
       *	3) It has the same bus name and number but a different parent bus slot
       *	4) It has a different bus name, but the same parent bus name, number and slot
       *	5) There is an active controller in the requested slot
       *
       * For the first four cases, if the structure is not ALV_ALIVE then we will
       * simply change the bus number to LDBL_WILDNUM.  If the structure is
       * alive or there is an active controller in the slot, we need to report it 
       * since it will prevent subsequent configuration.
       */
      if (strcmp_EQL (bus->bus_name, new_bus->bus_name)) {
	 if ((new_bus->bus_num != LDBL_WILDNUM) && (bus->bus_num == new_bus->bus_num)) {
	    /*
	     * We have a bus with the same name and number.  Therefore we could have
	     * one of the first three cases.
	     *
	     * A few notes about the logic (possible assumptions):
	     *	    - We can't get here if the new_bus has a wildcarded parent
	     *        bus name (connect_bus) and buses in the configurations
	     *        (bus) must have a valid bus name.
	     *      - An LBS that has an explicit parent bus number always collides
	     *        with a bus structure with a wildcarded parent bus number.
	     *      - If neither the LBS or bus have a wildcarded bus number then
	     *        a collision will occur if the bus numbers are not the same.
	     *      - Buses in the configuration (bus) cannot have a wildcarded
	     *	      slot number.
	     */
	    if (strcmp_NEQ (bus->connect_bus, new_bus->connect_bus) ||
	       ((new_bus->connect_num != LDBL_WILDNUM) && (bus->connect_num == LDBL_WILDNUM)) ||
	       ((bus->connect_num != LDBL_WILDNUM) && (bus->connect_num != new_bus->connect_num)) ||
	       ((new_bus->slot != LDBL_WILDNUM) && (bus->slot != new_bus->slot))) {
		   /*
		    * Yes, we definately have one of the first three cases (it doesn't
		    * matter which).  Check to see that it is not alive then zap the bus_num.
		    */
		   if (!(bus->alive & ALV_ALIVE))
		      bus->bus_num = LDBL_WILDNUM;
		   else {
		      printf ("LDBL: Conflict detected between proposed new bus (%s%d on %s%d) and existing active bus (%s%d on %s%d, slot %d)\n",
			       new_bus->bus_name, new_bus->bus_num, new_bus->connect_bus, new_bus->connect_num,
			       bus->bus_name, bus->bus_num, bus->connect_bus, bus->connect_num, bus->slot);
		   }
            }
         }
      }
      else {
	 /*
	  * We have a different bus name.  Check to see if this is case #4.  If it is,
	  * then as long as it is not alive we are okay since the bus resolver code will
	  * find it and simply change its name.  However, if it is alive we are
	  * hosed since we cannot touch it.
	  */
	 if (strcmp_EQL (bus->connect_bus, new_bus->connect_bus) &&
	    (bus->connect_num != LDBL_WILDNUM) && (bus->connect_num == new_bus->connect_num) &&
	    (bus->slot == new_bus->slot) &&
	    (bus->alive & ALV_ALIVE)) {
		/*
		 * We have case number 4.  A different bus adapter was found active
		 * on the specified bus and slot.
		 */
		printf ("LDBL: Conflict detected between proposed new bus %s%d and existing active bus (%s%d on %s%d, slot %d)\n",
			 new_bus->bus_name, new_bus->bus_num,
			 bus->bus_name, bus->bus_num, bus->connect_bus, bus->connect_num, bus->slot);
		return (LDBL_ECONFLICT);
         }
      }
      /*
       * If this is the correct parent bus, check to see if there is a controller
       * on the bus in the slot that we wanted to use.
       */
      if (strcmp_EQL (bus->bus_name, new_bus->connect_bus) &&
	 (bus->bus_num != LDBL_WILDNUM) && (bus->bus_num == new_bus->connect_num) &&
	 (new_bus->slot != LDBL_WILDNUM)) {
	    cp = ldbl_find_ctlr (bus->ctlr_list, LDBL_SEARCH_ALL, LDBL_WILDNAME, LDBL_WILDNUM, new_bus->slot);
	    if (cp != NULL) {
	       /*
		* We have a controller currently in the slot we want to use for
		* this new bus.  If it is not ALV_ALIVE and was previously created
		* by a loadable driver (ALV_NOSIZER is set) then delete the controller
		* so we can be free to create a bus structure (duplicate structures
		* are not allowed).
		*/
	       if (cp->alive & ALV_ALIVE) {
		  if (cp->alive & ALV_NOSIZER) {
		     Dprintf ("LDBL: Conflicting controller %s%d found on bus %s%d, slot %d being deleted.\n",
			       cp->ctlr_name, cp->ctlr_num, bus->bus_name, bus->bus_num, cp->slot);
		     ldbl_deallocate_ctlr (cp);
		  }
		  else {
		     printf ("LDBL: Conflict detected between proposed new bus %s%d and static controller %s%d at %s%d, slot %d\n",
			      new_bus->bus_name, new_bus->bus_num, cp->ctlr_name, cp->ctlr_num, cp->slot);
		     return (LDBL_ECONFLICT);
		  }
	       }
	       else {
		  printf ("LDBL: Conflict detected between proposed new bus %s%d and active controller %s%d at %s%d, slot %d\n",
			   new_bus->bus_name, new_bus->bus_num, cp->ctlr_name, cp->ctlr_num, cp->slot);
		  return (LDBL_ECONFLICT);
	       }
	    }
      }
      bus = ldbl_find_bus (bus, LDBL_SEARCH_NOINCLUDE, LDBL_WILDNAME, LDBL_WILDNUM);
   }
   return (ESUCCESS);
}

/******************************************************************************
 *
 * merge_bus_error()
 *
 *	This routine reports error messages from the merge_bus_struct routine.
 *
 * Return Value:
 *	-None-
 */
static void
merge_bus_error (bp, err_num)
   struct bus		*bp;		/* Entry that caused error	*/
   int			err_num;	/* Error number			*/
{
   switch (err_num) {
      case MERGE_BUS_NOPARENT :
	 printf ("LDBL: Unable to create bus %s", bp->bus_name);
	 if (bp->bus_num != LDBL_WILDNUM)
	    printf ("%d", bp->bus_num);
         else
            printf ("?");
         printf (" - Parent bus %d", bp->connect_bus);
	 if (bp->connect_num != LDBL_WILDNUM)
	    printf ("%d", bp->connect_num);
	 else
	    printf ("?");
	 printf (" does not exist.\n");
         break;

      case MERGE_BUS_INUSE :
	 printf ("LDBL: Bus %s%d at %s%d was found already in use - skipping\n",
		  bp->bus_name, bp->bus_num, bp->connect_bus, bp->connect_num);
         break;

      case MERGE_BUS_ACTIVE :
	 printf ("LDBL: Bus %s%d was found active at %s%d - WRONG PARENT BUS\n",
		  bp->bus_name, bp->bus_num, bp->connect_bus, bp->connect_num);
	 break;

      case MERGE_BUS_NOSUPPORT :
	 printf ("LDBL: Cannot configure bus %s%d because parent bus %s%d has no loadable support\n",
		  bp->bus_name, bp->bus_num, bp->connect_bus, bp->connect_num);
	 break;

      default : ;
   }
}

/******************************************************************************
 *
 * locate_and_merge_ctlr()
 *
 *	This routine takes a given ldbl_module_list controller structure and
 *	merges the contents into the hardware topology tree.  For discussion, a
 *	controller structure taken from the ldbl_module_list is referred to as
 *	an LCS.
 *
 *	Since each LCS may map onto multiple controller structures in the
 *	hardware topology tree (via wildcarding), the entire tree will be
 *	searched to find all matches.  The criteria used for matching will be
 *	taken directly out of the LCS (controller name/number, bus name/number).
 *	If a match occurs, the information in the LCS is copied to the one
 *	found from the hardware topology.  Note that the bus name/number will
 *	not be modified since it is already correct (it maybe wildcarded in the
 *	LCS).  If no match occurs and the LCS explicitly defines the connecting
 *	bus name, number and slot then a controller structure will be allocated
 *	and connected to the appropriate bus.
 *
 *	For each structure found (or created) and used, the ALV_LOADABLE flag
 *	will be set to indicate that the resolver has processed it.  This
 *	prevents the resolver from finding it again should a more generic
 *	wilcard search occur.
 *
 *	There are also instances when a controller number needs to be assigned. 
 *	This can occur when an LCS does not explicitly specify a controller
 *	number and the controller structure from the hardware topology tree 
 *	has a wildcarded controller number (always true when the structure 
 *	needed to be created or is created by bus probing).
 *
 * Return Values:
 *	ESUCCESS	- Successfull
 *	ENOMEM		- No memory to complete operation
 */
static int
locate_and_merge_ctlr (config_cp, driver_struct)
   struct config_entry	*config_cp;	/* Controller to scan for	   */
   struct driver	*driver_struct;	/* Ptr to driver's driver struct   */
{
   int			explicit_bus;
   int			found_match = FALSE;
   int			status = 0;
   struct bus		*bus_ptr;
   struct controller	*ctlr_ptr;
   /*
    * The first thing we need to do is check to see if the LCS has a
    * defined bus name.  If so, check the rest of the configuration for 
    * any name conflicts.  This can happen when a driver is unloaded 
    * and the stanza entry is changed.
    *
    * Assumption: By definition the ctlr_name cannot be wildcarded in the LCS
    *	   since we would not have any clue as to what to look for.
    */
   if (strcmp_NEQ (config_cp->e_str.e_controller.bus_name, LDBL_WILDNAME)) {
      if (status = seek_ctlr_conflict (&config_cp->e_str.e_controller))
         return (status);
   }
   /*
    * Okay, we now need to see if there is/are (an) existing controller 
    * structure(s) that match the LCS.  If so, we can merge the LCS
    * into the structure(s) that we find.
    *
    * Note: Since the LCS may contain wildcards, we need to loop until
    *       we don't find any more structures.
    */
   explicit_bus = (strcmp_NEQ (config_cp->e_str.e_controller.bus_name, LDBL_WILDNAME) &&
                  (config_cp->e_str.e_controller.bus_num != LDBL_WILDNUM) &&
                  (config_cp->e_str.e_controller.slot != LDBL_WILDNUM));

   bus_ptr = ldbl_find_bus (system_bus, LDBL_SEARCH_ALL, 
			    config_cp->e_str.e_controller.bus_name,
			    config_cp->e_str.e_controller.bus_num);
   if (bus_ptr == NULL) {
      merge_ctlr_error (&config_cp->e_str.e_controller, MERGE_CTLR_NOPARENT);
      return (ESUCCESS);
   }

   while (bus_ptr != NULL) {
      /*
       * We have a bus, check to see if there is a controller hanging off of
       * it that matches the one specified.  Remember, the ldbl_find_ctlr()
       * routine will first try to find an exact match.  If that fails and
       * the slot number is specified (not wildcarded), it will try again
       * using a wildcarded ctlr_num.  This allows it to find a controller
       * structure that has a wildcarded ctlr_num when we have explictly
       * specified a ctlr_num.
       */
      Dprintf ("LDBL: Checking against bus %s%d\n", bus_ptr->bus_name, bus_ptr->bus_num);
      ctlr_ptr = ldbl_find_ctlr (bus_ptr->ctlr_list, LDBL_SEARCH_ALL,
				 config_cp->e_str.e_controller.ctlr_name,
				 config_cp->e_str.e_controller.ctlr_num,
				 config_cp->e_str.e_controller.slot);
      /*
       * If we didn't find any controller using the full specification above 
       * and the controller is explicitly defined (bus name, number and slot),
       * then search for a controller that is in the desired slot.  This allows
       * us to handle the case where the controller name has changed but a 
       * structure for that slot exists.  If we don't do this we could end up 
       * creating duplicate structures for the slot.
       *
       * Note: We only need do this at the beginning since there can only be
       *       one structure with the specified slot number.
       */
      if ((ctlr_ptr == NULL) && explicit_bus) {
	 ctlr_ptr = ldbl_find_ctlr (bus_ptr->ctlr_list, LDBL_SEARCH_ALL,
				    LDBL_WILDNAME, LDBL_WILDNUM,
                                    config_cp->e_str.e_controller.slot);
      }
      /*
       * If we still didn't find anything, try one last gasp effort.  If the
       * ctlr_num is NOT wildcarded and the slot number IS wildcarded, look 
       * for a controller that has its ctlr_num explicitly set to LDBL_WILDNUM.
       * This allows us to handle the case where the user specified the
       * ctlr_num but not where it is connected.  Since the bus routines may
       * leave the ctlr_num equal to LDBL_WILDNUM for structures they create
       * we would not find them.
       *
       * Note:  We only want to do this once since there can only be one
       *        possible match.
       */
      if ((ctlr_ptr == NULL) && 
         (config_cp->e_str.e_controller.ctlr_num != LDBL_WILDNUM) &&
	 (config_cp->e_str.e_controller.slot == LDBL_WILDNUM)) {
		ctlr_ptr = ldbl_find_ctlr (bus_ptr->ctlr_list, LDBL_SEARCH_ALL,
					   config_cp->e_str.e_controller.ctlr_name,
					   LDBL_WILDNUM, LDBL_WILDNUM);
		while (ctlr_ptr != NULL) {
		   if (ctlr_ptr->ctlr_num == LDBL_WILDNUM)
		      break;
		   ctlr_ptr = ldbl_find_ctlr (ctlr_ptr, LDBL_SEARCH_NOINCLUDE,
					      config_cp->e_str.e_controller.ctlr_name,
					      LDBL_WILDNUM, LDBL_WILDNUM);
		}
      }
      while (ctlr_ptr != NULL) {
	 /*
          * This controller fully matches the one we are searching for.
	  * If it is not active AND has not been previously processed by
	  * the resolver, then we can fillin the structure from the LCS.
          */
         if ((ctlr_ptr->alive & ALV_PRES) && (!(ctlr_ptr->alive & ALV_LOADABLE))) {
	    Dprintf ("LDBL:    Found controller %s%d, slot %d\n", ctlr_ptr->ctlr_name, ctlr_ptr->ctlr_num, ctlr_ptr->slot);
	    /*
	     * One last check to see if the parent bus actually has support for
	     * loadable drivers.  
	     */
	    if (bus_ptr->framework != NULL) {
		/*
		 * Merge the structures and then set the ALV_LOADABLE bit so 
		 * we won't try and reconfigure this controller at a future time.
		 * Also, indicate that we found a match during the search.
		 *
		 * Note: We don't touch the bus_name and number since they are
		 *       presumeably correct.
		 */
		if (status = merge_ctlr_structures (config_cp, ctlr_ptr, driver_struct)) {
		   return (status);
		}
		ctlr_ptr->alive |= ALV_LOADABLE | ALV_NOSIZER;
		found_match = TRUE;
	    }
	    else {
		/*
		 * We can't add the requested controller as loadable since its 
		 * parent bus does not contain loadable support.
		 */
		merge_ctlr_error (ctlr_ptr, MERGE_CTLR_NOSUPPORT);
	    }
	 }
	 else {
	    /*
	     * A match was found but it was already configured.  If we got here
	     * because the entry was wildcarded, ignore it.  Otherwise output a
	     * warning and continue - this is not a fatal error.
	     */
	    if (explicit_bus) {
	       merge_ctlr_error (ctlr_ptr, MERGE_CTLR_INUSE);
	    }
         }
	 /*
	  * See if there are any other controllers.  Note that the initial entry
	  * into this loop uses three attempts to find a controller.  This next
	  * call is only useful if the first one found a controller.  If either
	  * of the other two calls were used to find a controller then this call
	  * should not find anything.
	  */
         ctlr_ptr = ldbl_find_ctlr (ctlr_ptr, LDBL_SEARCH_NOINCLUDE,
				    config_cp->e_str.e_controller.ctlr_name,
				    config_cp->e_str.e_controller.ctlr_num,
				    config_cp->e_str.e_controller.slot);
      }
      bus_ptr = ldbl_find_bus (bus_ptr, LDBL_SEARCH_NOINCLUDE,
			       LDBL_WILDNAME, LDBL_WILDNUM);
   }
   /*
    * If we didn't find any matches (found_match is false), check to see if
    * this was an explicit declaration.  If it was then we need to create the
    * controller structure and hook it into the configuration.
    */
   if (found_match == FALSE) {
      Dprintf ("LDBL:	No entry found...");
      if (explicit_bus) {
	 Dprintf ("creating one\n");
	 bus_ptr = ldbl_find_bus (system_bus, LDBL_SEARCH_ALL,
				  config_cp->e_str.e_controller.bus_name,
				  config_cp->e_str.e_controller.bus_num);
	 if (bus_ptr == NULL) {
	    merge_ctlr_error (&config_cp->e_str.e_controller, MERGE_CTLR_NOPARENT);
         }
         else {
            if ((ctlr_ptr = (struct controller *) kalloc (sizeof (struct controller))) == NULL) {
	       status = ENOMEM;
	    }
	    else {
               bzero (ctlr_ptr, sizeof (struct controller));
	       if (!(status = merge_ctlr_structures (config_cp, ctlr_ptr, driver_struct))) {
		   ctlr_ptr->bus_name = bus_ptr->bus_name;
		   ctlr_ptr->bus_num  = bus_ptr->bus_num;
		   ctlr_ptr->alive = ALV_LOADABLE | ALV_NOSIZER;
		   conn_ctlr (bus_ptr, ctlr_ptr);
	       }
               else {
		   ldbl_deallocate_ctlr (ctlr_ptr);
               }
	    }
         }
      }
      else {
         Dprintf ("and not explicitly defined... skipping\n");
      }
   }
   return (status);
}

/******************************************************************************
 *
 * merge_ctlr_structures()
 *
 *	This routine does the actual merge between an LCS and a controller
 *	structure found on the hardware topology tree.
 *
 * Return Value:
 *	ESUCCESS	- Successful
 *	ENOMEM		- Unable to allocate needed memory
 */
static int
merge_ctlr_structures (config_cp, ctlr_ptr, driver_struct)
   struct config_entry	*config_cp;	/* Merge from		       */
   struct controller	*ctlr_ptr;	/* Merge to		       */
   struct driver	*driver_struct;	/* Address of driver Structure */
{
   /*
    * Since there are at least two ways for controller structures to be created
    * (autoconfiguration and by the resolver), we need to be check some of the
    * fields to see if we should merge them or not.  For example, if the LCS
    * wildcarded some fields which were filled in in the controller structure we 
    * wouldn't want to try and merge those fields since they are already
    * correct.
    *
    * The following fields are not merged (just zerod):
    *	ctlr_type	- Filled in by the ldbl_ctlr_configure() routine
    *   bus_name	- Filled in by the caller
    *   bus_num		- Filled in by the caller
    *	intr		- Filled in by bus specific routines
    *	alive		- Calling routines responsibility
    *	private		- Filled in by controller specific routines
    *
    * The following fields are not touched.  They may contain information
    * placed there when the controller structure was created:
    *	conn_priv	- Possibly used by connecting bus specific routines
    *	rsvd		- Not currently used
    *
    * Plus the headers at the top of the structure which are filled in as
    * items are connected to the hardware topology tree.
    */

   ctlr_ptr->driver = driver_struct;

   if (strcmp_NEQ (config_cp->e_str.e_controller.ctlr_name, LDBL_WILDNAME) &&
     ((ctlr_ptr->ctlr_name == NULL) ||
       strcmp_NEQ (config_cp->e_str.e_controller.ctlr_name, ctlr_ptr->ctlr_name))) {
         if (ctlr_ptr->ctlr_name != NULL) kfree (ctlr_ptr->ctlr_name, strlen (ctlr_ptr->ctlr_name) + 1);
         ctlr_ptr->ctlr_name = (char *) kalloc (strlen (config_cp->e_str.e_controller.ctlr_name) + 1);
	 if (ctlr_ptr->ctlr_name != NULL) {
	    strcpy (ctlr_ptr->ctlr_name, config_cp->e_str.e_controller.ctlr_name);
	 }
	 else {
	    return (ENOMEM);
	 }
   }
   /*
    * Determine where the ctlr number is coming from.  If it isn't
    * defined in the LCS or the found ctlr structure we must search
    * the entire configuration for the current maximum value and use
    * the next free one.
    */
   if ((config_cp->e_str.e_controller.ctlr_num == LDBL_WILDNUM) && (ctlr_ptr->ctlr_num == LDBL_WILDNUM)) {
      ctlr_ptr->ctlr_num = find_next_ctlr_num (ctlr_ptr->ctlr_name);
   }
   else if (config_cp->e_str.e_controller.ctlr_num != LDBL_WILDNUM) {
      ctlr_ptr->ctlr_num = config_cp->e_str.e_controller.ctlr_num;
   }

   if ((config_cp->e_str.e_controller.slot != LDBL_WILDNUM) &&
       (config_cp->e_str.e_controller.slot != ctlr_ptr->slot)) {
         ctlr_ptr->slot = config_cp->e_str.e_controller.slot;
   }
   /*
    * Fillin the port stuff.
    */
   if (strcmp_NEQ (config_cp->e_str.e_controller.pname, LDBL_WILDNAME) &&
     ((ctlr_ptr->pname == NULL) ||
       strcmp_NEQ (config_cp->e_str.e_controller.pname, ctlr_ptr->pname))) {
         if (ctlr_ptr->pname != NULL) kfree (ctlr_ptr->pname, strlen(ctlr_ptr->pname) + 1);
         ctlr_ptr->pname = (char *) kalloc (strlen (config_cp->e_str.e_controller.pname) + 1);
	 if (ctlr_ptr->pname != NULL) {
	    strcpy (ctlr_ptr->pname, config_cp->e_str.e_controller.pname);
	 }
	 else {
	    return (ENOMEM);
	 }
   }

   if ((config_cp->e_str.e_controller.port != NULL) &&
       (config_cp->e_str.e_controller.port != ctlr_ptr->port)) {
         kfree (ctlr_ptr->port, sizeof (struct port));
         ctlr_ptr->port = (struct port *) kalloc (sizeof (struct port));
	 if (ctlr_ptr->port != NULL) {
            bcopy (config_cp->e_str.e_controller.port, ctlr_ptr->port, sizeof (struct port));
	 }
	 else {
	    return (ENOMEM);
	 }
   }
   /*
    * Copy or zero out the remaining fields.
    */
   ctlr_ptr->rctlr	   = config_cp->e_str.e_controller.rctlr;
   ctlr_ptr->addr	   = config_cp->e_str.e_controller.addr;
   ctlr_ptr->addr2	   = config_cp->e_str.e_controller.addr2;
   ctlr_ptr->flags	   = config_cp->e_str.e_controller.flags;
   ctlr_ptr->bus_priority  = config_cp->e_str.e_controller.bus_priority;
   ctlr_ptr->ivnum	   = config_cp->e_str.e_controller.ivnum;
   ctlr_ptr->priority	   = config_cp->e_str.e_controller.priority;
   ctlr_ptr->cmd	   = config_cp->e_str.e_controller.cmd;
   ctlr_ptr->physaddr	   = config_cp->e_str.e_controller.physaddr;
   ctlr_ptr->physaddr2	   = config_cp->e_str.e_controller.physaddr2;

   ctlr_ptr->private[0]    = NULL;
   ctlr_ptr->private[1]    = NULL;
   ctlr_ptr->private[2]    = NULL;
   ctlr_ptr->private[3]    = NULL;
   ctlr_ptr->private[4]    = NULL;

   return (ESUCCESS);
}  

/******************************************************************************
 *
 * seek_ctlr_conflict()
 *
 *	This routine checks the configuration for any conflicts that might
 *	exist.  This can happen if a controller was defined (by a previous
 *	driver loading) and is now being reloaded after a stanza entry change.
 *
 *	For example: if controller foo1 on bus tc0 slot 3 previously existed,
 *	and then the stanza entry was changed to say that controller foo1 is
 *	now on bus tc1 slot 1 - we have a name conflict.
 *
 *	Upon reload, the previously existing controller structure on bus tc0
 *	needs to be located and the ctlr_num needs to be changed to LDBL_WILDNUM.
 *	This will allow us to use the ctlr_num on the new controller that will
 *	be created (or redefined).
 *
 * Return Value:
 *	ESUCCESS	- Successful
 *	LDBL_ECONFLICT	- Existing active controller or adpater will prevent
 *			  merging of explict controller declaration
 */
static int
seek_ctlr_conflict(new_ctlr)
   struct controller	*new_ctlr;
{
   struct bus		*bus, *bp;
   struct controller	*cp;

   bus = ldbl_find_bus (system_bus, LDBL_SEARCH_ALL, LDBL_WILDNAME, LDBL_WILDNUM);
   while (bus != NULL) {
      cp = ldbl_find_ctlr (bus->ctlr_list, LDBL_SEARCH_ALL, LDBL_WILDNAME, LDBL_WILDNUM, LDBL_WILDNUM);
      while (cp != NULL) {
         /*
          * Okay, we need to determine if this controller conflicts with the LCS in one
	  * of five ways:
	  *	1) It has the same ctlr name and number but a different bus name
	  *	2) It has the same ctlr name and number but a different bus number
	  *	3) It has the same ctlr name and number but a different slot
	  *	4) It has a different ctlr name, but the same bus name, number and slot
	  *	5) There is an active adapter (bus) in the requested slot
	  *
	  * For the first four cases, if the structure is not ALV_ALIVE then we will
	  * simply change the controller number to LDBL_WILDNUM.  If the structure is
	  * alive or there is an active adapter in the slot, we need to report it 
          * since it will prevent subsequent configuration.
	  */
	 if (strcmp_EQL (cp->ctlr_name, new_ctlr->ctlr_name)) {
	    if ((new_ctlr->ctlr_num != LDBL_WILDNUM) && (cp->ctlr_num == new_ctlr->ctlr_num)) {
		/*
		 * We have a controller with the same name and number.  Therefore we
		 * could have one of the first three cases.
		 *
		 * A few notes about the logic (possible assumptions):
		 *	- We can't get here if the new_ctlr has a wildcarded bus name
		 *	  and controllers in the config (cp) must have a valid bus name.
		 *	- An LCS that has an explicit bus number always collides with
		 *	  a bus structure with a wildcarded bus number.
		 *	- If neither the LCS or controller have a wildcarded bus
		 *	  number then a collision will occur if the bus numbers are
		 *	  not the same.
		 *	- Controllers in the config (cp) cannot have a wildcarded
		 *	  slot number.
		 */
		if (strcmp_NEQ (cp->bus_name, new_ctlr->bus_name) ||
		   ((new_ctlr->bus_num != LDBL_WILDNUM) && (cp->bus_num == LDBL_WILDNUM)) ||
		   ((cp->bus_num != LDBL_WILDNUM) && (cp->bus_num != new_ctlr->bus_num)) ||
		   ((new_ctlr->slot != LDBL_WILDNUM) && (cp->slot != new_ctlr->slot))) {
			/*
			 * Yes, we definately have one of the first three cases (doesn't matter
			 * which).  Check to see that it is not alive then zap the ctlr_num.
			 */
			if (!(cp->alive & ALV_ALIVE)) {
			   cp->ctlr_num = LDBL_WILDNUM;
			}
			else {
			   printf ("LDBL: Conflict detected between proposed new controller %s%d and existing active ctlr (%s%d on %s%d, slot %d)\n",
				    new_ctlr->ctlr_name, new_ctlr->ctlr_num,
				    cp->ctlr_name, cp->ctlr_num, cp->bus_name, cp->bus_num, cp->slot);
			}
		}
	    }
         }
	 else {
	    /*
	     * We have a different controller name.  Check to see if this is case #4.
	     * As long as it is not alive we are okay since the controller resolver code
	     * will find it and simply change its name.  However, if it is alive we
	     * are hosed since we cannot touch it.
	     */
	    if (strcmp_EQL (cp->bus_name, new_ctlr->bus_name) && 
	       (cp->bus_num != LDBL_WILDNUM) && (cp->bus_num == new_ctlr->bus_num) &&
	       (cp->slot == new_ctlr->slot) &&
	       (cp->alive & ALV_ALIVE)) {
		   /*
		    * We have case number 4.  A different controller was found active
		    * on the specified bus and slot.
		    */
		   printf ("LDBL: Conflict detected between proposed new controller %s%d and existing active ctlr (%s%d on %s%d, slot %d)\n",
			    new_ctlr->ctlr_name, new_ctlr->ctlr_num,
			    cp->ctlr_name, cp->ctlr_num, cp->bus_name, cp->bus_num, cp->slot);
		   return (LDBL_ECONFLICT);
	    }
	 }
         cp = ldbl_find_ctlr (cp, LDBL_SEARCH_NOINCLUDE, LDBL_WILDNAME, LDBL_WILDNUM, LDBL_WILDNUM);
      }
      /*
       * If this is the correct parent bus, check to see if there is an active 
       * adapter (bus) on the bus in the slot that we wanted to use.
       */
      if (strcmp_EQL (bus->bus_name, new_ctlr->bus_name) &&
	 (bus->bus_num != LDBL_WILDNUM) && (bus->bus_num == new_ctlr->bus_num) &&
	 (new_ctlr->slot != LDBL_WILDNUM)) {
	    bp = ldbl_search_bus_local (bus->bus_list, LDBL_SEARCH_ALL, LDBL_WILDNAME, LDBL_WILDNUM, new_ctlr->slot);
	    if (bp != NULL) {
	       /*
		* We have an adapter (bus) currently connected to the slot we want
		* to used for this new controller.  If it is not ALV_ALIVE and was
		* previously created by a loadable driver (ALV_NOSIZER is set), then
		* delete the bus structure so we can be free to create a controller
		* structure (duplicate structures are not allowed).
		*/
	       if (bp->alive & ALV_ALIVE) {
		  if (bp->alive & ALV_NOSIZER) {
		     Dprintf ("LDBL: Conflicting bus %s%d found on bus %s%d, slot %d being deleted.\n",
			       bp->bus_name, bp->bus_num, bus->bus_name, bus->bus_num, cp->slot);
		     ldbl_deallocate_bus (bp);
		  }
		  else {
		     printf ("LDBL: Conflict detected between proposed new controller %s%d and static bus %s%d at %s%d, slot %d\n",
			      new_ctlr->ctlr_name, new_ctlr->ctlr_num, bp->bus_name, bp->bus_num, 
			      bus->bus_name, bus->bus_num, bp->slot);
		     return (LDBL_ECONFLICT);
		  }
	       }
	       else {
		  printf ("LDBL: Conflict detected between proposed new controller %s%d and active bus %s%d at %s%d, slot %d\n",
			   new_ctlr->ctlr_name, new_ctlr->ctlr_num, bp->bus_name, bp->bus_num, 
			   bus->bus_name, bus->bus_num, bp->slot);
		  return (LDBL_ECONFLICT);
	       }
	    }
      }
      bus = ldbl_find_bus (bus, LDBL_SEARCH_NOINCLUDE, LDBL_WILDNAME, LDBL_WILDNUM);
   }						   
   return (ESUCCESS);
}

/******************************************************************************
 *
 * merge_ctlr_error()
 *
 *	This routine reports error messages from the locate_and_merge_ctlr routine.
 *
 * Return Value:
 *	Void
 */
static void
merge_ctlr_error (cp, err_num)
   struct controller	*cp;		/* Entry that caused error	*/
   int			err_num;	/* Error number			*/
{
   switch (err_num) {
      case MERGE_CTLR_NOPARENT :
	 printf ("LDBL: Unable to create controller %s", cp->ctlr_name);
	 if (cp->ctlr_num != LDBL_WILDNUM)
	    printf ("%d", cp->ctlr_num);
         else
            printf ("?");
         printf (" - Parent bus %s", cp->bus_name);
	 if (cp->bus_num != LDBL_WILDNUM)
	    printf ("%d", cp->bus_num);
	 else
	    printf ("?");
	 printf (" does not exist.\n");
         break;

      case MERGE_CTLR_INUSE :
	 printf ("LDBL: Controller %s%d at %s%d was found already in use - skipping\n",
		  cp->ctlr_name, cp->ctlr_num, cp->bus_name, cp->bus_num);
         break;

      case MERGE_CTLR_ACTIVE :
	 printf ("LDBL: Controller %s%d was found active at %s%d - WRONG PARENT BUS\n",
		  cp->ctlr_name, cp->ctlr_num, cp->bus_name, cp->bus_num);
         break;

      case MERGE_CTLR_NOSUPPORT :
	 printf ("LDBL: Cannot configure controller %s%d because parent bus %s%d has no loadable support\n",
		  cp->ctlr_name, cp->ctlr_num, cp->bus_name, cp->bus_num);
	 break;

      default : ;
   }
}

/******************************************************************************
 *
 * transfer_bus_module_list ()
 *
 *	This routines scans the *ldbl_module_list for all bus structures
 *	created by the specified driver.  It will then arrange them in sorted
 *	order on a singly-linked list.  The sorting order is as follows:
 *
 *		1) Explicitly defined bus number entries
 *		2) Bus number wildcarded entries
 *
 * Return Values:
 *	Null	- No entries were found
 *	Address	- List head of e_bus structures
 */
static struct config_entry *
transfer_bus_module_list (driver_name)
   char			*driver_name;	/* Struct tag name to search for    */
{
   struct config_entry	*bus_ptr,	/* Current bus structure	    */
			*bus_hd,	/* List head of bus structures	    */
			*next,		/* Used to search bus list	    */
			*prev;		/* Used to search bus list	    */

   bus_hd = NULL;
   bus_ptr = get_config_entry (driver_name, BUS_TYPE);
   while (bus_ptr != NULL) {
      if (bus_hd == NULL) {
         bus_hd = bus_ptr;
         bus_ptr->e_next = NULL;
      }
      else {
         /*
	  * Try to find where the entry should go by checking the bus
	  * number.  All wildcards should go at the end of the list, all
	  * others should go just before the first wildcard.
	  */
	 if (bus_ptr->e_str.e_bus.bus_num != LDBL_WILDNUM) {
	    /*
	     * The bus number is not wildcarded so find the first entry that
	     * is and insert the bus structure at the appropriate spot.	
	     */
	    if (bus_hd->e_str.e_bus.bus_num == LDBL_WILDNUM) {
	       bus_ptr->e_next = bus_hd;
	       bus_hd = bus_ptr;
            }
	    else {
               prev = bus_hd;
	       for (next = bus_hd; next != NULL; next = next->e_next) {
                  if (next->e_str.e_bus.bus_num == LDBL_WILDNUM)
	             break;
	          else
		     prev = next;
	       }
	       bus_ptr->e_next = next;
	       prev->e_next = bus_ptr;
            }
	 }
	 else {
	    /*
	     * The bus number is wildcarded so find the end of the list and
	     * insert it there.
	     */
            prev = bus_hd;
	    for (next = bus_hd; next != NULL; next = next->e_next) {
	       prev = next;
            }
	    bus_ptr->e_next = next;
	    prev->e_next = bus_ptr;
         }
      }
      bus_ptr = get_config_entry (driver_name, BUS_TYPE);
   }
   return (bus_hd);
}

/******************************************************************************
 *
 * transfer_ctlr_module_list ()
 *
 *	This routines scans the *ldbl_module_list for all controller structures
 *	created by the specified driver.  It will then arrange them in sorted
 *	order on a singly-linked list.  The sorting order is as follows:
 *
 *		1) Explicitly defined controller number entries
 *		2) Controller number wildcarded entries
 *
 * Return Values:
 *	Null	- No entries were found
 *	Address	- List head of e_ctlr structures
 */
static struct config_entry *
transfer_ctlr_module_list (driver_name)
   char			*driver_name;	/* Struct tag name to search for    */
{
   struct config_entry	*ctlr_ptr,	/* Current ctlr structure	    */
			*ctlr_hd,	/* List head of ctlr structures	    */
			*next,		/* Used to search ctlr list	    */
			*prev;		/* Used to search ctlr list	    */

   ctlr_hd = NULL;
   ctlr_ptr = get_config_entry (driver_name, CONTROLLER_TYPE);
   while (ctlr_ptr != NULL) {
      if (ctlr_hd == NULL) {
         ctlr_hd = ctlr_ptr;
         ctlr_ptr->e_next = NULL;
      }
      else {
         /*
	  * Try to find where the entry should go by checking the ctlr
	  * number.  All wildcards should go at the end of the list, all
	  * others should go just before the first wildcard.
	  */
	 if (ctlr_ptr->e_str.e_controller.ctlr_num != LDBL_WILDNUM) {
	    /*
	     * The ctlr number is not wildcarded so find the first entry that
	     * is and insert the ctlr structure at the appropriate spot.	
	     */
	    if (ctlr_hd->e_str.e_controller.ctlr_num == LDBL_WILDNUM) {
	       ctlr_ptr->e_next = ctlr_hd;
	       ctlr_hd = ctlr_ptr;
            }
	    else {
	       prev = ctlr_hd;
	       for (next = ctlr_hd; next != NULL; next = next->e_next) {
                  if (next->e_str.e_controller.ctlr_num == LDBL_WILDNUM)
		     break;
                  else
		     prev = next;
	       }
	       ctlr_ptr->e_next = next;
	       prev->e_next = ctlr_ptr;
            }
	 }
	 else {
	    /*
	     * The ctlr number is wildcarded so find the end of the list and
	     * insert it there.
	     */
            prev = ctlr_hd;
	    for (next = ctlr_hd; next != NULL; next = next->e_next) {
	       prev = next;
            }
	    ctlr_ptr->e_next = NULL;
	    prev->e_next = ctlr_ptr;
	 }
      }
      ctlr_ptr = get_config_entry (driver_name, CONTROLLER_TYPE);
   }
   return (ctlr_hd);
}

/******************************************************************************
 *
 * find_next_bus_num()
 *
 *	This routine searches the entire configuration looking for the
 *	specified bus in order to determine what the next available number
 *	for that bus is.
 *
 * Return Values:
 *	Bus number to use
 */
static int
find_next_bus_num (bus_name)
   char		*bus_name;
{
   int			next_num = 0;
   struct bus		*bp;

   bp = ldbl_find_bus (system_bus, LDBL_SEARCH_ALL, bus_name, LDBL_WILDNUM);
   while (bp != NULL) {
      if (bp->bus_num >= next_num) {
         next_num = bp->bus_num + 1;
      }
      bp = ldbl_find_bus (bp, LDBL_SEARCH_NOINCLUDE, bus_name, LDBL_WILDNUM);
   }
   return (next_num);
}

/******************************************************************************
 *
 * find_next_ctlr_num()
 *
 *	This routine searches the entire configuration looking for the
 *	specified ctlr in order to determine what the next available number
 *	for that controller is.
 *
 * Return Values:
 *	Next availabel controller number
 */
static int
find_next_ctlr_num (ctlr_name)
   char			*ctlr_name;
{
   int			next_num = 0;
   struct bus		*bp;
   struct controller	*cp;

   bp = ldbl_find_bus (system_bus, LDBL_SEARCH_ALL, LDBL_WILDNAME, LDBL_WILDNUM);
   while (bp != NULL) {
      cp = ldbl_find_ctlr (bp->ctlr_list, LDBL_SEARCH_ALL, ctlr_name, LDBL_WILDNUM, LDBL_WILDNUM);
      while (cp != NULL) {
         if (cp->ctlr_num >= next_num) {
	    next_num = cp->ctlr_num + 1;
	 }
         cp = ldbl_find_ctlr (cp, LDBL_SEARCH_NOINCLUDE, ctlr_name, LDBL_WILDNUM, LDBL_WILDNUM);
      }
      bp = ldbl_find_bus (bp, LDBL_SEARCH_NOINCLUDE, LDBL_WILDNAME, LDBL_WILDNUM);
   }
   return (next_num);
}
