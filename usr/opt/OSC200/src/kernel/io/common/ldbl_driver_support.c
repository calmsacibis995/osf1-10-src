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
static char *rcsid = "@(#)$RCSfile: ldbl_driver_support.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/04/14 16:07:28 $";
#endif

#include <kern/lock.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/lock_types.h>
#include <sys/sysconfig.h>

#include <io/common/devdriver.h>

/*
 * Macros used for MP locking of loadable module list.
 */
#define LOADABLES_LOCK_INIT  lock_init(&ldbl_module_list_lock,TRUE)
#define LOADABLES_LOCK	     lock_write(&ldbl_module_list_lock)
#define LOADABLES_UNLOCK     lock_done(&ldbl_module_list_lock)
#define DEVMETHOD_STATE_LOCK_INIT  lock_init(&dev_method_state_lock,TRUE)
#define DEVMETHOD_STATE_LOCK	   lock_write(&dev_method_state_lock)
#define DEVMETHOD_STATE_UNLOCK     lock_done(&dev_method_state_lock)
/*
 * Initialization macro for MP locking of hardware topology tree.
 * The actual lock and unlock macros live in devdriver.h.  This macro
 * is here because we don't want someone to just blindly init the
 * lock on us.
 */
#define TOPOLOGY_TREE_LOCK_INIT	   lock_init(&topology_tree_lock,TRUE)

struct config_entry *ldbl_module_list = NULL;	/* Global list of configs */
dev_mod_t *dev_method_state_list = NULL;        /* driver method state */
/*
 * MP locking mechansims.
 */
lock_data_t	ldbl_module_list_lock;		/* MP lock for module list */
lock_data_t	dev_method_state_lock;		/* Lock for dev method state */
lock_data_t	topology_tree_lock;		/* MP Lock for topology tree */

/*
 * NAME:	loadable_driver_init()
 * 
 * FUNCTION:	Initializes the loadable driver framework.
 * 
 * EXEC ENV: 	Called out of startup sequence.
 *
 * RETURNS:	
 */
int
loadable_driver_init()
{
	/*
	 * Initialize the global list header of config data structures to be
	 * NULL.  Next initialize the MP lock used to protect this data.
 	 */
	ldbl_module_list = NULL;
	LOADABLES_LOCK_INIT;

	dev_method_state_list = NULL;
	DEVMETHOD_STATE_LOCK_INIT;
	/*
	 * Initialize the MP lock used to protect the hardware topology tree.
	 */
	TOPOLOGY_TREE_LOCK_INIT;
}

/*
 * NAME:	get_config_entry()
 * 
 * FUNCTION:	This routine searches the global list of config data
 *		structures and returns a pointer to the first one in the
 *		list of the matching name and type.  The name field is the
 *		driver name as listed in the stanza entry.  The type field
 *		will be either BUS, CONTROLLER, or DEVICE.
 *		The return value is a pointer
 *		to the matching structure.  It is expected that the caller of
 *		this routine will later free the structure.  The structure
 *		itself is unlinked from the list.
 * 
 * EXEC ENV: 	Called during the loading process of a loadable device driver.
 *
 * RETURNS:	A pointer to the matching config_entry data structure.
 *		NULL if no matching structure is located.
 */
struct config_entry *
get_config_entry(name, type)
	char *name;
	int  type;
{
	struct config_entry *entry_ptr, *prev_entry;
	struct config_entry *retval = NULL;


	entry_ptr = ldbl_module_list;
	prev_entry = ldbl_module_list;

	LOADABLES_LOCK;
	while (entry_ptr != NULL) {
		if ((strcmp(name, entry_ptr->e_name) == 0) &&
		    (entry_ptr->e_type == type)) {
			retval = entry_ptr;
			if (prev_entry == ldbl_module_list) {
				ldbl_module_list = entry_ptr->e_next;
			}
			else {
				prev_entry->e_next = entry_ptr->e_next;
			}
			break;
		}
		else {
			prev_entry = entry_ptr;
			entry_ptr = entry_ptr->e_next;
		}
	}
	LOADABLES_UNLOCK;
	/*
	 * Prior to handing over the configuration data structure, zap out
	 * the next pointer field just in case the consumer gets any funny
	 * ideas.
	 */
	if (retval != NULL) {
		retval->e_next = NULL;
	}
	return(retval);
}

/*
 * NAME:	del_config_entry()
 * 
 * FUNCTION:	This routine is called to delete driver configuration
 *		information.  Normally these structures will be removed
 *		when processed by the stanza resolver routine.  However for 
 *		the case where the load of the driver has failed it is
 *		necessary to cleanup any residual data structs because
 *		stanza_resolver will not get called.
 *
 *		This function is performed by calling the get_config_entry
 *		routine to remove all entries of the specified name.  This is
 *		done for each supported configuration information type.
 * 
 * EXEC ENV: 	Called out of setsysinfo if the loading process of a loadable
 *		driver fails.  Called by device driver method.
 *
 * RETURNS:	
 */
int
del_config_entry(ep)
	struct config_entry *ep;
{
	struct config_entry *new_entry;

	/*
	 * Delete bus entries for this driver.
	 */
	while ((new_entry = (struct config_entry *)
			get_config_entry(ep->e_nm_1, BUS_TYPE)) != NULL) {
		kfree(new_entry, sizeof(struct config_entry));
	}
	/*
	 * Delete controller entries for this driver.
	 */
	while ((new_entry = (struct config_entry *)
			get_config_entry(ep->e_nm_1, CONTROLLER_TYPE)) != NULL){
		kfree(new_entry, sizeof(struct config_entry));
	}
	/*
	 * Delete device entries for this driver.
	 */
	while ((new_entry = (struct config_entry *)
			get_config_entry(ep->e_nm_1, DEVICE_TYPE)) != NULL) {
		kfree(new_entry, sizeof(struct config_entry));
	}
}

/*
 * NAME:	add_config_entry()
 * 
 * FUNCTION:	This routine is used to take device configuration
 *		information used for loadable drivers and chain the
 *		information onto a global linked list of such structures.
 *		The structures are removed from the list when they
 *		are accessed by name and type to configure into the 
 *		current hardware topology.
 * 
 * EXEC ENV: 	Called during the process of loading a dynamic driver.
 *		The cfgmgr daemon makes setsysinfo calls containing the
 *		driver configuration information.  Setsysinfo calls this
 *		routine to add the comfiguration entry.
 *
 * RETURNS:	0 - success = was able to allocate the memory and chain in
 *		the new configuration data structure.
 *		nonzero - error = unable to chain new entry.
 */
int
add_config_entry(ep)
	struct config_entry *ep;
{
	struct config_entry *new_entry;
	struct config_entry *entry_ptr;
	struct bus *bp;
	struct controller *cp;
	struct device *dp;

	/*
	 * Allocate memory for a new structure.  Setup the string pointers
	 * to point to the "copy" of the string contained in the entry
	 * structure.  This way we don't have to allocate separate memory to
	 * do a copyinstr to.  By doing this all the memory associated with
	 * the entry can be deallocated in one call; rather than keeping 
	 * the structs separate from the char arrays.
	 */
	if ((new_entry = (struct config_entry *)
		kalloc(sizeof(struct config_entry))) == NULL) {
		return(ENOMEM);
	}
	bcopy(ep,new_entry, sizeof(struct config_entry));

	switch (new_entry->e_type) {
		case BUS_TYPE:
				bp = (struct bus *)&new_entry->e_str;
				bp->bus_name = new_entry->e_nm_1;
				bp->connect_bus = new_entry->e_nm_2;
				bp->pname = new_entry->e_nm_3;
				break;
		case CONTROLLER_TYPE:	
				cp = (struct controller *)&new_entry->e_str;
				cp->ctlr_name = new_entry->e_nm_1;
				cp->bus_name = new_entry->e_nm_2;
				cp->pname = new_entry->e_nm_3;
				break;
		case DEVICE_TYPE:	
				dp = (struct device *)&new_entry->e_str;
				dp->dev_type = new_entry->e_nm_1;
				dp->dev_name = new_entry->e_nm_2;
				dp->ctlr_name = new_entry->e_nm_3;
				break;
		default:
			return (EINVAL);
			
	}

	/*
	 * Add the structure to the end of the module list.
	 */
	LOADABLES_LOCK;
	entry_ptr = ldbl_module_list;
	if (entry_ptr == NULL) {
		ldbl_module_list = new_entry;
	}
	else {
		while (entry_ptr->e_next != NULL) {
			entry_ptr = entry_ptr->e_next;
		}
		entry_ptr->e_next = new_entry;
	}
	new_entry->e_next = NULL;
	LOADABLES_UNLOCK;
	return(0);
}

/*---------------------------------------------------------------------
 * Routines used to save device driver method state in the kernel.
 * NOTE: This approach to saving device method state is likely to change
 * with the 1.1 approach.  That was not available in time for this 
 * implementation.
 *---------------------------------------------------------------------*/

/*
 * NAME:	get_driver_state_entry()
 * 
 * FUNCTION:	This routine is called to get the state information 
 *		associated with the specified driver name.  This information
 *		is used to represent the state of the device driver method
 *		portion of the cfgmgr daemon.
 *
 *		NOTE: This approach to saving device method state is
 *		likely to change with the 1.1 approach.  That was not
 *		available in time for this implementation.
 * 
 * EXEC ENV: 	Called out of setsysinfo if a driver state entry is being
 *		deleted or via getsysinfo if a driver state entry is being
 *		returned back to cfgmgr.
 *
 * RETURNS:	A pointer to the matching dev_mod_t data structure.
 *              NULL if no matching structure is located.
 */
dev_mod_t *
get_driver_state_entry(name)
	char *name;
{
	dev_mod_t *mod_ptr;
	dev_mod_t *retval = NULL;

	if (name[0] == '\0') {
		return(NULL);
	}
	/*
	 * NOTE: prior to calling this routine the lock must already
	 * be taken out on the method state lock.  For this reason
	 * the lock is not taken out here.
	 */
	mod_ptr = dev_method_state_list;
	while (mod_ptr != NULL) {
		if (strcmp(name, mod_ptr->dev_name) == 0) {
			retval = mod_ptr;
			break;
		}
		mod_ptr = mod_ptr->next;
	}
	return(retval);
}

/*
 * NAME:	add_driver_state_entry()
 * 
 * FUNCTION:	This routine is used to add the state of a newly loaded
 *		device driver to a global linked list.  This is done to
 *		save the state for the driver method of cfgmgr.  The info
 *		will be retrieved if cfgmgr is killed off and restarted.  
 *		That enables cfgmgr to know what is currently loaded.
 *
 *		NOTE: This approach to saving device method state is
 *		likely to change with the 1.1 approach.  That was not
 *		available in time for this implementation.
 * 
 * EXEC ENV: 	Called when a device driver is loaded via cfgmgr by using
 *		a call to setsysinfo.
 *
 * RETURNS:	0 - success = was able to allocate the memory and chain in
 *		the new method state structure for the driver.
 *		nonzero - error = unable to chain new entry.
 */
int
add_driver_state_entry(modp)
	dev_mod_t *modp;
{
	dev_mod_t *new_devmod;
	dev_mod_t *mod_ptr;
	dev_mod_t *mod_next;
	dev_mod_t *mod_prev;

	if ((modp == NULL) || (modp->dev_name[0] == '\0')) {
		return(-1);
	}
	/*
	 * Make sure there isn't already a driver entry under this name.
	 */
	DEVMETHOD_STATE_LOCK;
	new_devmod = get_driver_state_entry(modp->dev_name);
	DEVMETHOD_STATE_UNLOCK;
	/*
	 * If there is alredy a state entry for this driver update it
	 * with the appropriate information.
	 *
	 * Normally one would expect that an error should be returned
	 * if this happens, but because kloadsrv is not currently stateless
	 * that is not the case.  As a result this allows the "stale" entry
	 * to be updated with the correct kloadsrv "id" to operate with.
	 *
	 * No need to link in the new entry as the pointers are already
	 * established.  The pointers have to be saved prior to the bcopy
	 * and restored after the bcopy because the user level pointers
	 * refer to the linked list within cfgmgr.
	 */
	if (new_devmod != NULL) {
#ifdef KLOADSRV_STATELESS
		return(EALAREDY);
#else /* KLOADSRV_STATELESS */
		mod_next = new_devmod->next;
		mod_prev = new_devmod->prev;
		bcopy(modp, new_devmod, sizeof(dev_mod_t));
		new_devmod->next = mod_next;
		new_devmod->prev = mod_prev;
		DEVMETHOD_STATE_UNLOCK;
		return(0);
#endif /* KLOADSRV_STATELESS */
	}
	/*
	 * Allocate memory for a new structure.  
	 */
	if ((new_devmod = (dev_mod_t *)
		kalloc(sizeof(dev_mod_t))) == NULL) {
		return(ENOMEM);
	}
	bcopy(modp, new_devmod, sizeof(dev_mod_t));

	/*
	 * Add the structure to the end of the module list.
	 */
	DEVMETHOD_STATE_LOCK;
	mod_ptr = dev_method_state_list;
	if (mod_ptr == NULL) {
		dev_method_state_list = new_devmod;
	}
	else {
		while (mod_ptr->next != NULL) {
			mod_ptr = mod_ptr->next;
		}
		mod_ptr->next = new_devmod;
	}
	new_devmod->next = NULL;
	new_devmod->prev = mod_ptr;
	DEVMETHOD_STATE_UNLOCK;
	return(0);
}

/*
 * NAME:	del_driver_state_entry()
 * 
 * FUNCTION:	This routine is called to delete driver method state
 *		information.  This is done when a driver is unloaded.
 *		In this way the driver method state is current with
 *		the driver method of cfgmgr in the event that cfgmgr is
 *		killed off and then restarted.
 *
 *		NOTE: This approach to saving device method state is
 *		likely to change with the 1.1 approach.  That was not
 *		available in time for this implementation.
 * 
 * EXEC ENV: 	Called out of setsysinfo during the unload of a device driver.
 *
 * RETURNS:	0 - success - the entry existed and has now been deleted.
 *		-1 - error  - no matching entry found to delete.
 */
int
del_driver_state_entry(modp)
	dev_mod_t *modp;
{
	dev_mod_t *devmod_being_deleted;
	dev_mod_t *next_devmod;
	dev_mod_t *prev_devmod;

	if ((modp == NULL) || (modp->dev_name[0] == '\0')) {
		return(-1);
	}
	/*
	 * Delete state entry for this driver. 
	 */
	DEVMETHOD_STATE_LOCK;
	devmod_being_deleted = get_driver_state_entry(modp->dev_name);
	if (devmod_being_deleted == NULL) {
		DEVMETHOD_STATE_UNLOCK;
		return(EINVAL);
	}

	next_devmod = devmod_being_deleted->next;
	prev_devmod = devmod_being_deleted->prev;

	if ((next_devmod == NULL) && (prev_devmod == NULL)) {
		dev_method_state_list = NULL;
	}
	else {
		if (prev_devmod == NULL) {
			dev_method_state_list = next_devmod;
		}
		else {
			prev_devmod->next = next_devmod;
		}
		if (next_devmod != NULL) {
			next_devmod->prev = devmod_being_deleted->prev;
		}
	}
	DEVMETHOD_STATE_UNLOCK;
	kfree(devmod_being_deleted, sizeof(dev_mod_t));
	return(0);
}


/*
 * NAME:	driver_get_state()
 * 
 * FUNCTION:	This routine is called to query the state of what is
 *		currently loaded for device drivers.  The information is
 *		used to reconstruct the state of the device driver method
 *		portion of cfgmgr when it is restarted.
 *
 *		NOTE: This approach to saving device method state is
 *		likely to change with the 1.1 approach.  That was not
 *		available in time for this implementation.
 * 
 * EXEC ENV: 	Called out of getsysinfo when cfgmgr is restarted.
 *
 * RETURNS:	0 if the start pointer represents a vaild dev_mod_t structure.
 *              A nonzero errno if the pointer is invalid or the list is empty
 *		indicating that there are no drivers currently loaded.
 */
int
driver_get_state(modp, buf)
	dev_mod_t **modp;
	dev_mod_t *buf;
{
	dev_mod_t *mod_ptr;
	int modp_verified = 0;

	DEVMETHOD_STATE_LOCK;
	/*
	 * If modp is zero the first entry in the list is being requested.
	 * Otherwise a specific "next" entry of the list is being requested.
	 * In that case the pointer must be validated to insure that it is
	 * really pointing to a dev_mod_t structure currently in the list.
	 */
	if (*modp) {
		mod_ptr = dev_method_state_list;
		while (mod_ptr != NULL) {
			if (mod_ptr == *modp) {
				modp_verified = 1;
				break;
			}
			mod_ptr = mod_ptr->next;
		}
		if (modp_verified == 0) {
			DEVMETHOD_STATE_UNLOCK;
			return(EINVAL);
		}
	}
	else {
		mod_ptr = dev_method_state_list; 
	}

	if (mod_ptr != NULL) {
		*modp = mod_ptr->next;
		bcopy(mod_ptr, buf, sizeof(dev_mod_t));
	}
	else { /* End of list encountered. */
		*modp = NULL;
		bzero(buf, sizeof(dev_mod_t));
	}
	DEVMETHOD_STATE_UNLOCK;
	return(0);
}
