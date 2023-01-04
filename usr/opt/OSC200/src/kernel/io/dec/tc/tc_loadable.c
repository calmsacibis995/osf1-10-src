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
static char *rcsid = "@(#)$RCSfile: tc_loadable.c,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/04/23 13:03:20 $";
#endif

#include <sys/errno.h>
#include <sys/map.h>
#include <io/dec/tc/tc.h>
#include <io/common/devdriver.h>
#include <io/common/devdriver_loadable.h>

/*
 * Define all routines vectored via the loadable framework
 * extension.
 */

int		tc_ctlr_configure();
int		tc_ctlr_unconfigure();
int		tc_adp_unattach();
int		tc_config_resolver();
ihandler_id_t	tc_handler_add();
int		tc_handler_del();
int		tc_handler_enable();
int		tc_handler_disable();

struct bus_framework TC_loadable_framework = {
	tc_ctlr_configure, tc_ctlr_unconfigure, 
	tc_adp_unattach, tc_config_resolver, 
	tc_handler_add, tc_handler_del,
	tc_handler_enable, tc_handler_disable,
	0, 0, 0, 0, 0,		0, 0, 0, 0, 0 };

/* Define all local routines to this module */

struct device *	clone_device();
int		slave_device();
int		scan_tc_slot();
int		create_tc_bus_entry();
int		create_tc_ctlr_entry();

static int
tc_ctlr_configure(d_name, bus, ctlr, flags)
   char			*d_name;
   struct bus		*bus;
   struct controller	*ctlr;
   int			flags;
{
   register struct driver *drp;
   struct device 	*device,
			*real_dev;
   int 			slot, status, index;
   char 		*nxv;
   char 		*nxp;
   char 		*devname;
   caddr_t 		context;

   /*
    * Check to see if this controller is not currently present in the
    * configuration.  If not, then this is an error.  Signal it.
    *
    * Note:  This functionality is normally used by buses that cannot
    *	     autoconfigure such as VME.  In that case we need to be
    *	     able to add new controllers (or buses) via the stanza
    *	     entries.  For TURBOchannel, this is not necessary since
    *	     the bus is self-describing.
    */
   if ((!(ctlr->alive & ALV_PRES)) & (!(ctlr->alive & (ALV_ALIVE|ALV_NOCNFG)))) {
      return (ENODEV);
   }

   index = (int)ctlr->tcindx;
   slot = tc_slot[index].slot;
   nxv = (char *)PHYS_TO_K1(tc_slot[index].physaddr);
   nxp = (char *)tc_slot[index].physaddr;
   /*
    * Check to see if the tc_slot table needs to be updated.  This can happen
    * when the name of the controller is changed by the user.
    */
   if (strcmp_NEQ (ctlr->ctlr_name, tc_slot[index].devname)) {
      strcpy (tc_slot[index].devname, ctlr->ctlr_name);
   }
   devname = tc_slot[index].devname;
   /*
    * Probe the slot to see if a controller really exists there.  If
    * it does, fill in the tc_slot table's pointer to the controller.
    */
   if ((status = tc_cont_probe(nxv, slot, ctlr)) != ESUCCESS) {
      return (status);
   }
   tc_slot[index].dev_str = (caddr_t) ctlr;
   tc_slot[index].unit = ctlr->ctlr_num;
   tc_slot[index].flags |= TC_ACTIVE;
   /*
    * We have a real controller.  Okay, find the devices for it and try
    * to slave and attach them.
    */
   drp = ctlr->driver;
   context = NULL;
   for(;;) {
      status = ldbl_find_devlist_entry(d_name, ctlr->ctlr_name, ctlr->ctlr_num, &device, &context);
      switch (status) {
	 case ENODEV :
		return (ESUCCESS);
		break;
	 case ESUCCESS :
		break;
	 default :
		return (status);
      }
      /*
       * Now try to slave the device.  If this is a wildcard device
       * and the driver supports autoconfiguration (flag LDBL_AUTOCONFIG
       * is set), clone it so we can repeatedly call the slave routine 
       * until the device is rejected.  This allows us to handle creation of
       * the actual device structures and placement of those structures in 
       * the topology tree.  Also, if there is no slave routine then we
       * have to assume the device is no good.
       *
       * A word of caution:  the driver's slave routine has every reason
       *	to expect the device structure to point to its parent's
       *	controller structure.  Therefore we intentionally connect
       *	the two before calling the slave routine.  If the slave
       *	routine rejects it, the ldbl_deallocate_dev() routine will
       *	disconnect it as part of its cleanup work.
       */
      device->ctlr_hd = NULL;
      device->ctlr_num = ctlr->ctlr_num;
      device->ctlr_name = ctlr->ctlr_name;
      real_dev = NULL;

      if (drp->slave != NULL) {
         if ((device->logunit == LDBL_WILDNUM) && 
            ((device->ctlr_num == LDBL_WILDNUM) || (device->unit == LDBL_WILDNUM))) {
            if (flags & LDBL_AUTOCONFIG) {
	       status = ESUCCESS;
	       real_dev = device;
               while (status == ESUCCESS) {
                  device = clone_device (real_dev);
	          if (device != NULL) {
	             /*
	              * Okay, we have a cloned device structure.  Connect it to
	              * the controller and send it to the driver's slave routine.
	              */
	             conn_device (ctlr, device);
	             status = slave_device (drp, nxv, device);
	          }
	          else {
		     status = ENOMEM;
	          }
               }
            }
            else {
               /*
		* The driver does not support autoconfiguration.
		*/
               status = ENODEV;
            }
         }
         else {
            /*
             * The device is not wildcarded.
	     */
            conn_device (ctlr, device);
	    status = slave_device (drp, nxv, device);
         }
      }
      else {
	 status = ENODEV;
      }
      /*
       * Now check to see if it was successful.  If not we need to remove
       * the device structure we enqueued.  However, it is never an error
       * for a device structure to be rejected (otherwise the controller
       * would be rejected too).
       */
      if (status != ESUCCESS) {
         ldbl_deallocate_dev(device);
	 if (real_dev != NULL) ldbl_deallocate_dev(real_dev);
         status = ESUCCESS;
      }
   }
   /*
    * Actually, we should never get here.  The normal exit is from the
    * switch statement up at the first 3rd of the routine.
    */
   Dprintf ("LDBL: tc_ctlr_configure: Exiting by unconventional means.\n");
   return(status);
}

static int
tc_cont_probe(nxv, slot, ctlr)
   char *nxv;
   int slot;
   struct controller 	*ctlr;
{
   struct driver 	*drp;
   int 			status;
   /*
    * If the controller is already alive, simply indicate
    * that everything is okay.
    */
   if (ctlr->alive & ALV_ALIVE)
      return(ESUCCESS);
   /*
    * Try to probe the controller.  If there is no probe routine,
    * simply assume that everything is okay.
    */
   drp = ctlr->driver;
   if ((drp->probe) != NULL) {
      Dprintf ("LDBL:       calling probe(0x%x, 0x%x) for controller %s%d\n", 
		nxv, ctlr, ctlr->ctlr_name, ctlr->ctlr_num);
      if ((*drp->probe)(nxv, ctlr) == 0) {
         return(ENODEV);
      }
   }
   else {
      status = ESUCCESS;
   }

   ctlr->alive = (ctlr->alive & ~ALV_PRES) | ALV_ALIVE;
   ctlr->addr = (char *)nxv;
   (void)svatophys(ctlr->addr, &ctlr->physaddr);
   if (drp->ctlr_list != NULL) {
      drp->ctlr_list[ctlr->ctlr_num] = ctlr;
   }
   config_fillin(ctlr);
   if (drp->cattach) {
      (*drp->cattach)(ctlr);
      printf (" - Attached\n");
   }
   else {
      printf ("\n");
   }

   return(ESUCCESS);
}

static int
slave_device (drp, nxv, device)
   struct driver	*drp;
   char			*nxv;
   struct device	*device;
{
   int			status = ESUCCESS;
   struct controller	*ctlr;

   ctlr = device->ctlr_hd;
   if (drp->slave != NULL) {
      Dprintf ("LDBL:       calling slave(0x%x, 0x%x) for device %s%d\n",
		device, nxv, device->dev_name, device->logunit);
      if ((*drp->slave)(device, nxv) != 0) {
         if ((device->logunit != LDBL_WILDNUM) && (device->unit != LDBL_WILDNUM)) {
            device->alive = (device->alive & ~ALV_PRES) | ALV_ALIVE;
	    if (drp->dev_list != NULL) {
               drp->dev_list[device->logunit] = device;
	    }	       
            if (device->unit >= 0) {
               printf("Device %s%d configured on %s%d, unit %d", 
		       device->dev_name, device->logunit,
		       drp->ctlr_name, ctlr->ctlr_num, device->unit);
            }
            else {
	       printf("Device %s%d configured on %s%d", 
		       device->dev_name, device->logunit,
		       drp->ctlr_name, ctlr->ctlr_num);
            }
            if (drp->dattach) {
               (*drp->dattach)(device);
	       printf(" - Attached\n");
            }
            else {
	       printf ("\n");
            }
         }
         else {
	    printf ("LDBL: TURBOchannel: Driver %s returned an invalid unit or logunit for device %s on %s%d\n",
		     drp->dev_name, device->dev_name, device->ctlr_name, device->ctlr_num);
	    status = ENODEV;
         }
      }
      else {
         status = ENODEV;
      }
   }
   return (status);
}

static struct device *
clone_device (real_dev)
   struct device	*real_dev;
{
   struct device	*new_dev;
   /*
    * First thing is allocate the space for the device structure.
    * Then we can copy the contents of the old device structure and
    * fixup the dev_name field (it needs to have its own copy since
    * that is how deallocation expects things to be setup).
    */
   new_dev = (struct device *) kalloc (sizeof (struct device));
   if (new_dev != NULL) {
      bcopy (real_dev, new_dev, sizeof (struct device));
      new_dev->nxt_dev = NULL;
      new_dev->dev_name = (char *) kalloc (strlen(real_dev->dev_name) + 1);
      if (new_dev->dev_name != NULL) {
         strcpy (new_dev->dev_name, real_dev->dev_name);
      }
      else {
         kfree (new_dev, sizeof (struct device));
         new_dev = NULL;
      }
   }
   return (new_dev);      
}

static int
tc_ctlr_unconfigure(bus, driver, ctlr)
   struct bus		*bus;
   struct driver	*driver;
   struct controller	*ctlr;
{
   int			index;

   /*
    * Since the controller has been successfully unconfigured, clear the active
    * flag in the tc_slot table..  That way, if the driver is reloaded with a
    * different name and/or parameters they will be reset.
    */
   index = (int) ctlr->tcindx;
   tc_slot[index].flags &= ~TC_ACTIVE;

   return (ESUCCESS);
}

static int
tc_adp_unattach()
{
   Dprintf ("LDBL: In TURBOchannel adapter unattach routine.\n");
   return (ENODEV);
}

static int
tc_config_resolver(bus, tc_option_snippet)
   struct bus		*bus;
   struct tc_option	*tc_option_snippet;
{
   int			entry;
   int			index;
   int 			status = ESUCCESS;

   if (tc_option_snippet == NULL)
      return (ESUCCESS);
   /*
    * Walk the tc_option_snippet table looking in the tc_slot table for
    * a module name match.  If one is found, check to see if the tc_slot
    * entry is not yet active (flag TC_ACTIVE is not set).  If not, then
    * we need to fill the tc_slot table in and search for any bus or 
    * controller structure that may exist.  If none exists, then we will 
    * create the appropriate one.
    */
   for (entry = 0; tc_option_snippet[entry].modname[0] != NULL; entry++) {
      index = -1;
      Dprintf ("LDBL: Searching for module %s...", tc_option_snippet[entry].modname);
      do {
         index = scan_tc_slot (index, tc_option_snippet[entry].modname);
         if ((index >= 0) && (!(tc_slot[index].flags & TC_ACTIVE))) {
            /*
	     * We found a module with the correct module name that has
	     * not been configured.  Fill in the rest of the tc_slot
	     * table.
	     */
	    strcpy(tc_slot[index].devname, tc_option_snippet[entry].confname);
	    tc_slot[index].intr_b4_probe = tc_option_snippet[entry].intr_b4_probe;
	    tc_slot[index].intr_aft_attach = tc_option_snippet[entry].intr_aft_attach;
            switch (tc_option_snippet[entry].type) {
		case 'A' : {
	           tc_slot[index].adpt_config = tc_option_snippet[entry].adpt_config;
	           tc_slot[index].class = TC_ADPT;
	           status = create_tc_bus_entry (bus, &tc_slot[index], index);
		   break;
	        }
		case 'C' : {
		   tc_slot[index].class = TC_CTLR;
		   status = create_tc_ctlr_entry (bus, &tc_slot[index], index);
		   break;
		}
		case 'D' : {
		   /*
		    * We don't currently handle devices connected directly
		    * to a TURBOchannel bus.  This is in keeping with the
		    * bus, controler, device model.
		    */
		   printf ("LDBL: Attempt to install device directly on TURBOchannel failed.\n");
		   break;
		}
		default : {
		   printf ("LDBL: Unknown type \"%d\" in entry %s of driver's TURBOchannel option table\n",
			    tc_option_snippet[entry].type, tc_option_snippet[entry].modname);
		   break;
		}
            }
         }
	 else if ((index >= 0) && (!(tc_slot[index].flags & TC_ACTIVE))) {
	    printf ("\nLDBL: Module %s found already active in bus %s%d, slot %d\n",
		     tc_option_snippet[entry].modname, bus->bus_name, 
		     bus->bus_num, tc_slot[index].slot);
         }
      }
      while ((index >= 0) && (!status));
   }
   Dprintf ("LDBL:\n");
   return (status);
}

static ihandler_id_t
tc_handler_add(handler)
ihandler_t *handler;
{
	int index;
	struct tc_intr_info *iptr;
	struct controller *ctlr;
	struct bus *bus_ptr;

	Dprintf("LDBL:       tc_handler_add entered\n");
	iptr = (struct tc_intr_info *)handler->ih_bus_info;
	switch (iptr->config_type) {
		case TC_ADPT:
			bus_ptr = (struct bus *)iptr->configuration_st;
			index = (int)bus_ptr->tcindx;
			break;
		case TC_CTLR:
			ctlr = (struct controller *)iptr->configuration_st;
			index = (int)ctlr->tcindx;
			break;
		default: /* invalid type, return error */
			printf ("LDBL:    ** TURBOchannel: Unknown dynamic interrupt config type (%d).\n",
				 iptr->config_type);
			return((ihandler_id_t)-1);
			break;
	}
	if((tc_slot[index].flags & TC_INTR_ADD) == 0) {
		tc_slot[index].intr = iptr->intr;
		tc_slot[index].param = iptr->param;
		tc_slot[index].flags |= TC_INTR_ADD;
		return((ihandler_id_t)index);
	}
	else {
		return((ihandler_id_t)-1);
	}
}

static int
tc_handler_del(id)
ihandler_id_t id;
{

	int index;
	struct controller *ctlr;

	Dprintf("LDBL:    tc_handler_del entered - id = %d\n", (int)id);
	index = (int)id;
	if((tc_slot[index].flags & TC_INTR_ENAB) == 0) {
		(*tc_sw.disable_option)(tc_slot[index].slot);
		tc_slot[index].intr = 0;
		tc_slot[index].param = 0;
		tc_slot[index].flags &= ~TC_INTR_ADD;
		return(ESUCCESS);
	}
	else {
		return(-1);
	}
}

static int
tc_handler_enable(id)
ihandler_id_t id;
{

	int index;

	Dprintf("LDBL:       tc_handler_enable entered - id = %d\n", (int)id);
	index = (int)id;
	if((tc_slot[index].flags & TC_INTR_ADD) &&
	   ((tc_slot[index].flags & TC_INTR_ENAB) == 0)) {
		tc_slot[index].flags |= TC_INTR_ENAB;
		(*tc_sw.enable_option)(tc_slot[index].slot);
		return(ESUCCESS);
	}
	else {
		return(-1);
	}

}

static int
tc_handler_disable(id)
ihandler_id_t id;
{

	int index;

	Dprintf("LDBL:    tc_handler_disable entered - id = %d\n", (int)id);
	index = (int)id;
	if(tc_slot[index].flags & TC_INTR_ENAB) {
		(*tc_sw.disable_option)(tc_slot[index].slot);
		tc_slot[index].flags &= ~TC_INTR_ENAB;
		return(ESUCCESS);
	}
	else {
		return(-1);
	}
}

/******************************************************************************
 *
 * scan_tc_slot()
 *
 *	This routine takes the supplied starting slot number (-1 to start at
 *	the beginning) and scans the table looking for a module name that matches
 *	the one supplied.
 *
 * Return Values:
 *	+int	- Slot number match was found in
 *	-1	- No match found
 */
static int
scan_tc_slot (index, mod_name)
   int		index;
   char		*mod_name;
{
   int		found;
   /*
    * Determine the starting index number for the search.
    */
   if (index < 0) {
      index = 0;
   }
   else {
      index += 1;
   }
   /*
    * Now loop thru the tc_slot table until we either find a match or
    * run out of entries in the table.
    */
   found = FALSE;
   while ((index < TC_IOSLOTS) && (!found)) {
      if (tc_slot[index].modulename[0] != NULL)
         Dprintf ("\nLDBL:    Slot %d has a \"%s\"", tc_slot[index].slot, tc_slot[index].modulename);
      if (!(found = strcmp_EQL (mod_name, tc_slot[index].modulename))) {
         index++;
      }
   }
   if (!found) {
      Dprintf ("\n");
      return (-1);
   }
   else {
      Dprintf (" <--- Found one!");
      return (index);
  } 
}

/******************************************************************************
 *
 * create_tc_bus_entry()
 *
 *	This routine creates a bus structure using the information in the tc_slot
 *	entry supplied.  It will then connect it to the supplied parent bus structure.
 *
 * Return Values:
 *	Success/Failure status
 */
static int
create_tc_bus_entry (bus, tc_slot_entry, index)
   struct bus		*bus;			/* Parent bus structure */
   struct tc_slot	*tc_slot_entry;		/* TC_slot table entry to use */
   int			index;			/* Index into tc_slot table */
{
   struct bus		*new_bus;
   struct controller	*existing_ctlr;

   /*
    * Before we go and actually create the bus, check to see if there
    * is a controller or adapter already in the slot.
    */
   new_bus = ldbl_search_bus_local (bus->bus_list, LDBL_SEARCH_ALL, LDBL_WILDNAME,
				    LDBL_WILDNUM, tc_slot_entry->slot);
   if (new_bus != NULL) {
      /*
       * Since we found an existing bus adapter, check to see that it is 
       * not active.  Also, see if we are just reusing a bus structure
       * already setup correctly (driver was just unloaded and reloaded).
       * If so, we don't need to do anything.
       */
      if (new_bus->alive & ALV_ALIVE) {
         return (LDBL_ECONFLICT);
      }
      else if (strcmp_EQL (new_bus->bus_name, tc_slot_entry->devname)) {
         return (ESUCCESS);
      }
   }
   else {
      existing_ctlr = ldbl_find_ctlr (bus->ctlr_list, LDBL_SEARCH_ALL, LDBL_WILDNAME,
				      LDBL_WILDNUM, tc_slot_entry->slot);
      if (existing_ctlr != NULL) {
         /*
	  * We found an existing controller in the slot we want to use,
	  * if it's not active delete it so a new bus structure can be created.
	  */
	 if (existing_ctlr->alive & ALV_ALIVE) {
	    return (LDBL_ECONFLICT);
         }
	 else {
	    ldbl_deallocate_ctlr (existing_ctlr);
	 }
      }
   }
   /*
    * Okay, either use the bus structure we found or create a new one.
    */
   if (new_bus == NULL) {
      new_bus = (struct bus *) kalloc (sizeof (struct bus));
      if (new_bus == NULL)
         return (ENOMEM);

      bzero (new_bus, sizeof (struct bus));

      new_bus->bus_name = (char *) kalloc (strlen (tc_slot_entry->devname) + 1);
      if (new_bus->bus_name != NULL)
         strcpy (new_bus->bus_name, tc_slot_entry->devname);
      else {
         ldbl_deallocate_bus (new_bus);
         return (ENOMEM);
      }

      new_bus->bus_num     = LDBL_WILDNUM;
      new_bus->alive       = ALV_PRES | ALV_NOSIZER;
      new_bus->slot        = tc_slot_entry->slot;
      new_bus->tcindx	   = (caddr_t) index;
      new_bus->connect_bus = bus->bus_name;
      new_bus->connect_num = bus->bus_num;

      conn_bus (bus, new_bus);
   }
   else {
     char	*old_name;
      /*
       * The structure already exists and is essentially correct, however
       * the bus_name is wrong.  Simply setup the correct bus_name and
       * zap the bus_num.  Any further initialization of the structure
       * will get done by the loadable code.
       */
      old_name = new_bus->bus_name;
      new_bus->bus_name = (char *) kalloc (strlen (tc_slot_entry->devname) + 1);
      if (new_bus->bus_name != NULL) {
         strcpy (new_bus->bus_name, tc_slot_entry->devname);
         kfree (old_name, strlen(old_name) + 1);
      }
      else {
         ldbl_deallocate_bus (new_bus);
         return (ENOMEM);
      }
      new_bus->bus_num     = LDBL_WILDNUM;
      new_bus->alive       = ALV_PRES | ALV_NOSIZER;
   }     
   return (ESUCCESS);
}

/******************************************************************************
 *
 * create_tc_ctlr_entry()
 *
 *	This routine creates a controller structure using the information in the
 *	tc_slot entry supplied.  It will then connect it to the supplied parent 
 *	bus structure.
 *
 * Return Values:
 *	Success/Failure status
 */
static int
create_tc_ctlr_entry (bus, tc_slot_entry, index)
   struct bus		*bus;			/* Parent bus structure */
   struct tc_slot	*tc_slot_entry;		/* TC_slot table entry to use */
   int			index;			/* Index into TC_slot table */
{
   struct bus		*existing_bus;
   struct controller	*new_ctlr;

   /*
    * Before we go and actually create the controller, check to see if
    * there is a bus or adapter already in the slot.
    */
   new_ctlr = ldbl_find_ctlr (bus->ctlr_list, LDBL_SEARCH_ALL, LDBL_WILDNAME,
			      LDBL_WILDNUM, tc_slot_entry->slot);
   if (new_ctlr != NULL) {
      /*
       * Since we found an existing controller, check to see that it is
       * not active.  Also see if we are just reusing a controller structure 
       * already setup correctly (driver was just unloaded and reloaded).  
       * If so, we don't need to do anything.
       */
      if (new_ctlr->alive & ALV_ALIVE) {
         return (LDBL_ECONFLICT);
      }
      else if (strcmp_EQL (new_ctlr->ctlr_name, tc_slot_entry->devname)) {
         return (ESUCCESS);
      }
   }
   else {
      existing_bus = ldbl_search_bus_local (bus->bus_list, LDBL_SEARCH_ALL, LDBL_WILDNAME,
					    LDBL_WILDNUM, tc_slot_entry->slot);
      if (existing_bus != NULL) {
	 /*
	  * We found an existing bus in the slot we want to use, if it's
	  * not active, delete it so a new controller structure can be created.
	  */
	 if (existing_bus->alive & ALV_ALIVE) {
	    return (LDBL_ECONFLICT);
	 }
	 else {
	    Dprintf ("LDBL: Deleting conflicting bus in TURBOchannel create_tc_ctlr routine.\n");
	    ldbl_deallocate_bus (existing_bus);
         }
      }
   }
   /*
    * Okay, either use the controller structure we found or create a new one.
    */
   if (new_ctlr == NULL) {
      new_ctlr = (struct controller *) kalloc (sizeof (struct controller));
      if (new_ctlr == NULL)
         return (ENOMEM);

      bzero (new_ctlr, sizeof (struct controller));

      new_ctlr->ctlr_name = (char *) kalloc (strlen (tc_slot_entry->devname) + 1);
      if (new_ctlr->ctlr_name != NULL)
         strcpy (new_ctlr->ctlr_name, tc_slot_entry->devname);
      else {
         ldbl_deallocate_ctlr (new_ctlr);
         return (ENOMEM);
      }

      new_ctlr->ctlr_num  = LDBL_WILDNUM;
      new_ctlr->alive     = ALV_PRES | ALV_NOSIZER;
      new_ctlr->slot      = tc_slot_entry->slot;
      new_ctlr->tcindx    = (caddr_t) index;
      new_ctlr->bus_name  = bus->bus_name;
      new_ctlr->bus_num   = bus->bus_num;

      conn_ctlr (bus, new_ctlr);
   }
   else {
      char 	*old_name;
      /*
       * The structure already exists and is essentially correct, however
       * the ctlr_name is wrong.  Simply setup the correct ctlr_name
       * and zap the ctlr_num.  Any further initialization of the 
       * structure will get done by the loadable code.
       */
      old_name = new_ctlr->ctlr_name;
      new_ctlr->ctlr_name = (char *) kalloc (strlen (tc_slot_entry->devname) + 1);
      if (new_ctlr->ctlr_name != NULL) {
         strcpy (new_ctlr->ctlr_name, tc_slot_entry->devname);
         kfree (old_name, strlen(old_name) + 1);
      }
      else {
         return (ENOMEM);
      }
      new_ctlr->ctlr_num  = LDBL_WILDNUM;
      new_ctlr->alive     = ALV_PRES | ALV_NOSIZER;
   }
   return (ESUCCESS);
}
