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
static char *rcsid = "@(#)$RCSfile: ldbl_controller_support.c,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/04/23 14:57:37 $";
#endif

/*
 * This module contains the routines necessary to provide support for
 * loadable controllers.
 */

#include <sys/errno.h>
#include <sys/types.h>

#include <io/common/devdriver.h>
#include <io/common/devdriver_loadable.h>

/* Define all globally declared routines */

int			ldbl_ctlr_configure();
int			ldbl_ctlr_unconfigure();

/* Define all of the local routines */

int			remove_device();


/******************************************************************************
 *
 * ldbl_ctlr_configure()
 *
 *	
 *	Scan the current hardware topology tree for controller structures with
 *	the specified driver structure address that have been configured by
 *	the stanza resolver (ALV_PRES & ALV_LOADABLE is set).
 *
 *	For each controller found, call the bus specific configure routine to
 *	finish the configuration process.
 *	
 *
 * Return Values:
 *	ESUCCESS	- Controller(s) successfully configured
 *	Status values from bus specified controller configure routine
 */

int 
ldbl_ctlr_configure(bus_name, bus_num, driver_name, driver_struct, flags)
   char			*bus_name;		/* Name of bus to configure	    */
   int			bus_num;		/* Bus number to configure	    */
   char			*driver_name;		/* Name of controlling driver	    */
   struct driver	*driver_struct;		/* Pointer to calling driver struct */
   int			flags;			/* Misc. flags			    */
{
   int			status = ESUCCESS;	/* Return status 		    */
   struct bus		*bus_ptr;		/* Pointer to current bus	    */
   struct controller	*ctlr_ptr;		/* Pointer to current ctlr	    */
   /*
    * Lock the hardware topology tree.
    */
   LOCK_TOPOLOGY_TREE;

   /*
    * Search for all buses with the specified name and number (wildcards are permitted).
    * For each bus found, check for controller structures that have the correct driver
    * structure address and call the bus specific configuration routine.
    */
   bus_ptr = ldbl_find_bus (system_bus, LDBL_SEARCH_ALL, bus_name, bus_num);
   while ((bus_ptr != NULL) && (!status)) {
      ctlr_ptr = ldbl_find_ctlr (bus_ptr->ctlr_list, LDBL_SEARCH_ALL, 
				 LDBL_WILDNAME, LDBL_WILDNUM, LDBL_WILDNUM);
      while ((ctlr_ptr != NULL) && (!status)) {
         /*
          * Since the search criteria (driver structure address) is not a valid search
	  * item in the ldbl_find_ctlr routine.  Use it to walk the ctlr_list and
	  * check each item by hand.
	  */
	 if ((ctlr_ptr->driver == driver_struct) && (ctlr_ptr->alive & ALV_LOADABLE) &&
	     (!(ctlr_ptr->alive & (ALV_ALIVE|ALV_NOCNFG)))) {
	    /*
	     *  We found a valid controller, call the bus specific controller configure routine
	     *  to do its initialization of this controller.  This will include calling the
	     *  probe, controller attach, slave and attach routines.
	     */
	    if (bus_ptr->framework != NULL) {
	       if (bus_ptr->framework->ctlr_configure != NULL) {
	          if (status = (*bus_ptr->framework->ctlr_configure)(driver_name, bus_ptr, ctlr_ptr, flags)) {
			/*
			 * If the bus rejected the controller because it isn't there,
			 * simply delete the controller structure.  Don't report an error.
			 */
			if (status == ENODEV) {
			   Dprintf ("LDBL: Unable to configure controller %s%d on bus %s%d, slot %d - controller doesn't exist.\n",
				     ctlr_ptr->ctlr_name, ctlr_ptr->ctlr_num, ctlr_ptr->bus_name, 
				     ctlr_ptr->bus_num, ctlr_ptr->slot);
			   ldbl_deallocate_ctlr (ctlr_ptr);
			   status = ESUCCESS;
			}
			else {
	          	   ctlr_ptr->alive &= ~ALV_LOADABLE;
	          	   printf ("LDBL: Bus %s%d failed to configure controller %s%d, status = %d\n",
			            bus_ptr->bus_name, bus_ptr->bus_num,
		 	            ctlr_ptr->ctlr_name, ctlr_ptr->ctlr_num, status);
			}
	          }
	          else {
	          	/*
			 * Since the bus configuration went okay, mark the controller alive.
			 */
	          	ctlr_ptr->alive = (ctlr_ptr->alive & ~ALV_PRES) | ALV_ALIVE;
	          }
	       }
	       else {
	          printf ("LDBL: Unable to configure controller %s%d for bus %s%d, no bus ctlr_configure routine\n.",
		           ctlr_ptr->ctlr_name, ctlr_ptr->ctlr_num, bus_ptr->bus_name, bus_ptr->bus_num);
	       }
            }
	    else {
	       printf ("LDBL: Unable to configure controller %s%d, bus %s%d has no loadable support\n",
		        ctlr_ptr->ctlr_name, ctlr_ptr->ctlr_num, bus_ptr->bus_name, bus_ptr->bus_num);
	    }
         }
	 /*
	  * Go check the next controller
	  */
	 if (!status) {
	    ctlr_ptr = ldbl_find_ctlr (ctlr_ptr, LDBL_SEARCH_NOINCLUDE, LDBL_WILDNAME, LDBL_WILDNUM, LDBL_WILDNUM);
	 }
      }
      if (!status) {
         bus_ptr = ldbl_find_bus (bus_ptr, LDBL_SEARCH_NOINCLUDE, bus_name, bus_num);
      }
   }
   /*
    * Before we leave, remove all entries for this driver that are left on the ldbl_devlist.
    * Notice that this is done while we still have the topology tree lock.  While this is
    * kind of sleezy, it does permit us to be MP compliant without need of another lock.
    * This is because all usage of the ldbl_devlist is done inside of routines that are
    * already protected by the topology tree lock.
    */
   ldbl_cleanup_devlist (driver_name);
   UNLOCK_TOPOLOGY_TREE;

   return (status);
}

/******************************************************************************
 *
 * ldbl_ctlr_unconfigure()
 *
 *	Find all buses with the specified name and number (wildcards are 
 *	permitted).  For each bus found, check to see if the specified 
 *	controller is found (searching by controller name and number - again 
 *	wildcards are permitted).  Also ensure that the controller was 
 *	configured by loadable code (ALV_LOADABLE is set) and is ALV_ALIVE.
 *
 *	For each controller found, scan its device list calling the driver's 
 *	dev_unattach routine for each device found.  As each device structure 
 *	is processed, it will be deallocated to prevent subsequent configuration 
 *	(driver loading) from seeing devices that may no longer be in the
 *	configuration.
 *
 *	After the devices are processed, the driver's ctlr_unattach() routine
 *	will be called to allow the driver to cleanup after the controller.
 *	Once the ctlr_unattach routine completes, the controller structure will
 *	be marked ALV_PRES and the ALV_LOADABLE flag will be cleared.
 *
 * Return Values:
 *	ESUCCESS	- Successful
 */
int 
ldbl_ctlr_unconfigure(bus_name, bus_num, driver_struct, ctlr_name, ctlr_num)
   char			*bus_name;		/* Bus name to search for	     */
   int			bus_num;		/* Bus number to search for	     */
   struct driver	*driver_struct;		/* Pointer to calling driver struct  */
   char			*ctlr_name;		/* Controller name to search for     */
   int			ctlr_num;		/* Controller number to search for   */
{
   int			status = ESUCCESS;
   struct bus		*bus_ptr;
   struct device	*device, 
			*device_to_delete;
   struct controller	*ctlr_ptr;
   /*
    * Lock the hardware topology tree.
    */
   LOCK_TOPOLOGY_TREE;

   /*
    * Search the current hardware topology tree for the specified bus name and number.
    */
   bus_ptr = ldbl_find_bus (system_bus, LDBL_SEARCH_ALL, bus_name, bus_num);
   while ((bus_ptr != NULL) && (!status)) {
      /*
       * See if there are any matching controllers on this bus.
       */
      ctlr_ptr = ldbl_find_ctlr (bus_ptr->ctlr_list, LDBL_SEARCH_ALL, ctlr_name, ctlr_num, LDBL_WILDNUM);
      while ((ctlr_ptr != NULL) && (!status)) {
         /*
	  * Okay we found a matching controller structure.  Verify that it is ours by
	  * checking the driver structure address and that it is alive and was created
	  * by loadable code.
	  */
	 if (ctlr_ptr->driver == driver_struct) {
	    if (ctlr_ptr->alive & (ALV_ALIVE|ALV_LOADABLE)) {
	       /*
	        *  We found a valid controller.  Call the driver's device unattach,
	        *  controller unattach and then the bus specific controller unconfigure
	        *  routine.  As we unattach the devices, deallocate them.
	        */
	       Dprintf ("LDBL: Unconfiguring controller %s%d on %s%d\n",
		        ctlr_ptr->ctlr_name, ctlr_ptr->ctlr_num, 
		        ctlr_ptr->bus_hd->bus_name, ctlr_ptr->bus_hd->bus_num);
	       status = remove_devices (ctlr_ptr);
               /*
	        * If all of the devices were removed then call the driver's controller 
	        * unattach routine and the buses ctlr_unconfigure routine.  Then reset the 
	        * controller structure and proceed to the next controller.
	        */
	       if (!status) {
		  status = reset_ctlr (ctlr_ptr, driver_struct);
	       }
	    }
	    else {
	       /*
	        * We found the controller but it is still active or was not marked
		* as having been created by a loadable driver (which could mean a
		* data structure problem).
		*/
	       if (!(ctlr_ptr->alive & ALV_LOADABLE)) {
		  Dprintf ("LDBL: Controller %s%d at %s%d was not created by a loadable driver even though it is marked as such.\n",
			  ctlr_ptr->ctlr_name, ctlr_ptr->ctlr_num,
			  ctlr_ptr->bus_name, ctlr_ptr->bus_num);
	       }
	       else {
		  /*
		   * Entry was marked as being controlled by the current driver but is not
		   * alive.  This is not a fatal error.
		   */
		  printf ("LDBL: Spurious controller %s%d entry found in hardware topology tree.\n",
			  ctlr_ptr->ctlr_name, ctlr_ptr->ctlr_num,
			  ctlr_ptr->bus_name, ctlr_ptr->bus_num);
	       }
	    }
         }
	 else {
	    /*
	     * The controller is not controlled by this driver.  This can be caused by the
	     * caller wildcarding the controller name to search for.  Ignore the entry.
	     */
	 }
         if (!status) {
	    ctlr_ptr = ldbl_find_ctlr (ctlr_ptr, LDBL_SEARCH_NOINCLUDE, ctlr_name, ctlr_num, LDBL_WILDNUM);
         }
      }
      if (!status) {
         bus_ptr = ldbl_find_bus (bus_ptr, LDBL_SEARCH_NOINCLUDE, bus_name, bus_num);
      }
   }
   UNLOCK_TOPOLOGY_TREE;
   return (status);
}

/******************************************************************************
 *
 * remove_devices()
 *
 *	This routine takes the specified controller and calls the driver's
 *	dev_unattach routines for each entry in the device list.  As each 
 *	entry is process, the device structure is deallocated.
 *
 * Return Values:
 *	Success/Failure status
 *
 */
static int
remove_devices(ctlr_ptr)
   struct controller	*ctlr_ptr;		/* Pointer to controller	*/
{
   int			status = ESUCCESS;	/* Return status		*/
   int			(*dev_unattach)();	/* Cached pointer to rtn	*/
   struct device	*device,		/* Current device structure	*/
			*dev_to_delete;		/* Used to delete device struct	*/

   device = ctlr_ptr->dev_list;
   dev_unattach = ctlr_ptr->driver->dev_unattach;
   while ((device != NULL) && (!status)) {
      if (dev_unattach != NULL) {
	 Dprintf ("LDBL:    calling dev_unattach(0x%x, 0x%x) for device %s%d\n",
		   ctlr_ptr, device, device->dev_name, device->logunit);
         if (status = (*dev_unattach)(ctlr_ptr, device)) {
            printf ("LDBL: Unable to remove device %s%d from controller %s%d, status = %d\n",
		    device->dev_name, device->logunit,
		    ctlr_ptr->ctlr_name, ctlr_ptr->ctlr_num, status);
         }
      }
      dev_to_delete = device;
      device = ctlr_ptr->dev_list = device->nxt_dev;
      kfree (dev_to_delete, sizeof (struct device));
   }
   return (status);
}

/******************************************************************************
 *
 * reset_ctlr()
 *
 *	This routine takes the specified controller and calls the driver's
 *	ctlr_unattach() routine to enable the driver to remove any data structure
 *	entries it might have as well as disable/delete any interrupt handlers it
 *	might have enabled.  It then calls the bus specific ctlr_unconfigure() 
 *	routine to allow the bus to cleanup anything it might have done.
 *
 * Return Values:
 *	Success/Failure status
 *
 */
static int
reset_ctlr(ctlr_ptr, driver_ptr)
   struct controller	*ctlr_ptr;
   struct driver	*driver_ptr;
{
   int			status = NULL;
   /*
    * It is permitted for the driver writer to not have a ctlr_unattach()
    * routine.
    */
   if (driver_ptr->ctlr_unattach != NULL) {
      Dprintf ("LDBL:    calling ctlr_unattach(0x%x, 0x%x) for controller %s%d\n",
		ctlr_ptr->bus_hd, ctlr_ptr, ctlr_ptr->ctlr_name, ctlr_ptr->ctlr_num);
      if (status = (*driver_ptr->ctlr_unattach)(ctlr_ptr->bus_hd, ctlr_ptr)) {
         printf ("LDBL: Failed to unattach controller %s%d, status = %d\n",
		  ctlr_ptr->ctlr_name, ctlr_ptr->ctlr_num, status);
      }
   }
   /*
    * If we didn't get an error, or there was no controller unattach routine,
    * continue with the bus specific controller unconfigure routine.
    */
   if (status == NULL) {
      ctlr_ptr->driver = NULL;
      ctlr_ptr->alive = (ctlr_ptr->alive & ~(ALV_ALIVE|ALV_LOADABLE)) | ALV_PRES;
      if (ctlr_ptr->bus_hd->framework != NULL) {
         if ((ctlr_ptr->bus_hd->framework->ctlr_unconfigure != NULL) &&
             (status = (*ctlr_ptr->bus_hd->framework->ctlr_unconfigure) (ctlr_ptr->bus_hd, driver_ptr, ctlr_ptr))) {
		printf ("LDBL: Failure of bus %s%d to unattach controller %s%d, status = %d\n",
		         ctlr_ptr->bus_name, ctlr_ptr->bus_num,
		         ctlr_ptr->ctlr_name, ctlr_ptr->ctlr_num, status);
	 }
      }
      else {
         /*
	  * We have some sort of data structure inconsistency.  The controller
	  * structure we found indicated that it is controlled by the current
	  * driver.  However, the bus doesn't support loadable drivers!  How
	  * did the controller get loaded in the first place?
	  */
	 panic ("Loaded controller found on bus without loadable support!");
      }
   }
   return (status);
}
