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
static char *rcsid = "@(#)$RCSfile: method_dev.c,v $ $Revision: 1.1.3.6 $ (DEC) $Date: 1992/09/29 12:05:59 $";
#endif

/*
 * Device driver method
 * Author: Tim Burke 12/91
 *
 * To load a driver this method performs the following steps:
 *
 * - The method maintains a copy of its state in the kernel.  This is done
 *   to make the method stateless so that cfgmgr can be restarted and be
 *   aware of what has already been loaded.  For this reason the first step
 *   is to obtain the state of any previously loaded modules.
 *
 * - Kloadsrv is then called to load the driver object into kernel memory.
 *
 * - The driver's topology information is read in from the stanza file
 *   if anything is specified it will be parsed into bus, controller, and
 *   device lists.  These lists are then passed down into the kernel tagged
 *   with the driver name.  The topology data structs in the kernel will be
 *   used in the driver's probe/slave/attach sequence.
 *
 * - Next the driver is configured.  The major number requirements of the
 *   driver are obtained from the stanza entry.  All other parameters from
 *   the stanza are assembled into a structure passed down to the driver's
 *   configure routine.
 *
 * - Upon return from the driver's configure routine the major number assigned
 *   to the driver is known.  Based on that the device special files as 
 *   specified in the stanza entry are created.
 *
 * - Once the driver has been configured its state is saved in the kernel
 *   in case cfgmgr gets restarted.
 */


#if !defined(lint) && !defined(_NOIDENT)

#endif

#include <errno.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/sysconfig.h>
#include <sys/sysinfo.h>
#include <machine/hal_sysinfo.h>
#include <ctype.h>

#include "cm.h"

/*
 * Name-value pair structure to pass off to setsysinfo.
 * This is used to pass the driver method state down to the kernel 
 * to allow the method to be stateless and support method restart.
 * (NOTE: this will change with 1.1.)
 */
struct driver_ssi {
	int	op;			/* SSI operation type */
	dev_mod_t devmod;		/* Actual data passed */
};

dev_mod_t *devmod_head = NULL;		/* Head of method state list */
int	device_method_state_known = 0;  /* Indicates method state established */

dev_mod_t * DEVICE_alloc_devmod();
dev_mod_t * DEVICE_find_devmod();
int	    DEVICE_add_devmod();
int	    DEVICE_delete_devmod();
int	    DEVICE_free_devmod();
int	    DEVICE_get_state();
int	    DEVICE_set_state();
static void convert_to_upper();
static int  is_a_number();
/*
 * The following are possible values for the dev_load_flags field.
 */
#define	DEV_LOADED	0x01	/* driver loaded 			*/
#define	DEV_CONFIGURED	0x02	/* driver configured 			*/

#define UNUSED_MAJOR	-2	/* Used to represent a bogus major number */
#define MKNOD_CREATE	0	/* Create the device special files	  */
#define MKNOD_DELETE	1	/* Delete the device special files	  */

#define ADD_STATE	1	/* Add method state to kernel */
#define DELETE_STATE	2	/* Delete method state from kernel */

/*
 *
 *	Name:		DEVICE_method()
 *	Description:	Generic OSF/1 DEVICE Configuration Method
 *	Returns:	Zero 		On success.
 *			Non-zero	On failure.
 *
 */
int
DEVICE_method( cm_log_t * logp, ENT_t entry, cm_op_t op, cm_op_t * rop,
	char * opts )
{
	int		rc;
	dev_mod_t *	dev_mod;


	/*
	 * In the event that the cfgmgr daemon is killed off and restarted
	 * there may already be some drivers loaded.  By the use of
	 * getsysinfo determine if there are any drivers currently loaded.
	 * If the drivers are loaded this gives the method the ability to
	 * query and unload the module.
 	 *
	 * NOTE: this will all change with the 1.1 registration table.
	 */
	if (device_method_state_known == 0) {
		/*	
		 * The return value is not looked at.  If unable to 
		 * get the current state just assume that there isn't any
		 * state to be had.
		 */
		DEVICE_get_state( logp );
	}

	rc = 0;
	if (op & CM_OP_LOAD) {
		/*
	 	 * Find the driver's devmod structure if it has already
		 * been loaded (so that an error will be returned).
		 * The expected case is that this will allocate a new
		 * devmod struct for a new entry.
	 	 */
		if (rc=DEVICE_lookup_str_mod(logp, AFentname(entry), &dev_mod)){
			METHOD_LOG(LOG_ERR, SUBSYS_ADD_FAIL);
			return(-1);
		}
		rc = DEVICE_method_load(logp, entry, dev_mod);
		if (rc == 0) {
			*rop = CM_OP_LOAD;
			METHOD_LOG(LOG_INFO, MSG_LOADED);
		} else if (rc == KMOD_LOAD_L_EBUSY) {
			/*
			 * If the driver is already loaded, return an
			 * error status - without deleting any driver state
			 * because the driver is still loaded.
			 */
			METHOD_LOG(LOG_ERR, rc);
			return(-1);
		} else {
			METHOD_LOG(LOG_ERR, KLDR_ELOAD);
			/*
			 * The load operation has failed.  We need to delete
			 * the associated driver state information and return
			 * an error status.
			 */
		    	if (DEVICE_delete_devmod(logp, AFentname(entry)) == 0) {
				DEVICE_free_devmod(dev_mod);
		    	}
			return(-1);
		}
	}
	/*
	 * The assumption is that a driver must be loaded first and then
	 * the other op types will be done after that.  So for these other
	 * op types find the already existing devmod struct.  If it does not
	 * exist it is an error.
	 */
	else {
		dev_mod = DEVICE_find_devmod(logp, AFentname(entry));
		if (dev_mod == NULL) {
			METHOD_LOG(LOG_ERR, SUBSYS_NOT_REG);
			return(-1);
		}
	}

	if (op & CM_OP_CONFIGURE) {
		rc = DEVICE_method_configure(logp, entry, dev_mod);
		if (rc == 0) {
			*rop = CM_OP_CONFIGURE;
			METHOD_LOG(LOG_INFO, MSG_CONFIGURED);
		} else {
			/* The configure operation has failed.  Since this
			 * means the driver or DEVICE_method_configure
			 * routine encountered a fatal error.  We will
			 * unload the driver from the system.  
			 *
			 * NOTE:  Drivers should not report an error unless
			 *    they have cleaned up and are expecting to be
			 *    unloaded.
			 */
			METHOD_LOG(LOG_INFO, rc);
			rc = DEVICE_method_unload(logp, entry, dev_mod);
			if (rc == 0) {
				*rop = CM_OP_UNLOAD;
				METHOD_LOG(LOG_INFO, MSG_UNLOADED);
			} else {
				METHOD_LOG(LOG_ERR, SUBSYS_DEL_FAIL);
				METHOD_LOG(LOG_INFO, rc);
			}
		    	if (DEVICE_delete_devmod(logp, AFentname(entry)) == 0) {
				DEVICE_free_devmod(dev_mod);
		    	}
		}
	}


	if (op & CM_OP_UNCONFIGURE) {
		rc = DEVICE_method_unconfigure(logp, entry, dev_mod);
		if (rc == 0) {
			*rop = CM_OP_UNCONFIGURE;
			METHOD_LOG(LOG_INFO, MSG_UNCONFIGURED);
		} else {
			METHOD_LOG(LOG_INFO, rc);
		}
	}


	if (op & CM_OP_QUERY) {
		rc = DEVICE_method_query(logp, entry, dev_mod);
		if (rc == 0) {
			*rop = CM_OP_QUERY;
			METHOD_LOG(LOG_INFO, MSG_QUERIED);
		} else {
			METHOD_LOG(LOG_INFO, rc);
		}
	}

	/*
	 * Place the unload op last because it deallocates the devmod
	 * struct for this driver.
	 */
	if (op & CM_OP_UNLOAD) {
		rc = DEVICE_method_unload(logp, entry, dev_mod);
		if (rc == 0) {
			*rop = CM_OP_UNLOAD;
			METHOD_LOG(LOG_INFO, MSG_UNLOADED);
		} else {
			METHOD_LOG(LOG_ERR, SUBSYS_DEL_FAIL);
			METHOD_LOG(LOG_INFO, rc);
		}
		/*
		 * Don't delete the devmod struct if the unconfigure failed
		 * because the driver is currently busy.  If this was done 
		 * there would be no way to further query or unload the driver.
		 */
		if (rc != KMOD_UNLOAD_C_EBUSY) {
		    if (DEVICE_delete_devmod(logp, AFentname(entry)) == 0) {
			DEVICE_free_devmod(dev_mod);
		    }
		    else {
			METHOD_LOG(LOG_ERR, SUBSYS_DEL_FAIL);
		    }
		}
	}
	return(rc == 0 ? 0 : -1);
}


/*
 * Load the driver object module into kernel memory.  
 */
int
DEVICE_method_load( cm_log_t * logp, ENT_t entry, dev_mod_t * dev_mod )
{
	int	rc;

	if (dev_mod == NULL) {
		return(SUBSYS_GET_FAIL);
	}
	if (dev_mod->dev_load_flags & DEV_LOADED)
		return(KMOD_LOAD_L_EBUSY);
	if ((rc=cm_kls_load(entry, &dev_mod->dev_id)) != 0)
		return(rc);
	dev_mod->dev_load_flags |= DEV_LOADED;
	return(0);
}


/*
 * Unload the driver object module from the kernel memory space.
 */
int
DEVICE_method_unload( cm_log_t * logp, ENT_t entry, dev_mod_t * dev_mod )
{
	int	rc;

	if (dev_mod == NULL) {
		return(SUBSYS_GET_FAIL);
	}
	if ((dev_mod->dev_load_flags & DEV_LOADED) == 0)
		return(KMOD_UNLOAD_L_EEXIST);
	if (dev_mod->dev_load_flags & DEV_CONFIGURED)
		return(KMOD_UNLOAD_C_EBUSY);
	if ((rc=cm_kls_unload(dev_mod->dev_id)) != 0)
		return(rc);
	dev_mod->dev_id = LDR_NULL_MODULE;
	dev_mod->dev_load_flags &= ~DEV_LOADED;
	return(0);
}


/*
 * Configure a driver.  This consists of setting up the data structure which
 * will be passed to the driver's configure routine.  The data structure will
 * be setup in accordance with the fields of the stanza entry.  The
 * device topology information will be parsed and passed down to the kernel.
 * Upon return from the driver's configure routine the appropriate device
 * special files will be created.  The driver's state will be added to a
 * kernel resident state list to allow method restart.
 */
int
DEVICE_method_configure( cm_log_t * logp, ENT_t entry, dev_mod_t * dev_mod )
{
	int	rc;
	device_config_t	dev_inadm;
	char *any_string;
	int block_major_used = 0;
	int char_major_used = 0;
	int same_major_needed = 0;
	char *driver_name;
	char null_string = '\n';

	if (dev_mod == NULL) {
		return(SUBSYS_GET_FAIL);
	}
	if ((dev_mod->dev_load_flags & DEV_LOADED) == 0)
		return(KMOD_CONFIG_L_EEXIST);
	if (dev_mod->dev_load_flags & DEV_CONFIGURED)
		return(KMOD_CONFIG_C_EBUSY);

	/*
	 * Initialize some of the fields passed down to the driver's
	 * configure routine.
	 *
	 * The following fields are not initialized because they are not
	 * setup for the driver; rather they are fields for return values:
	 * dc_version, dc_errcode.
	 */
	dev_inadm.dc_begunit = 0;
	dev_inadm.dc_numunit = 0;
	dev_inadm.dc_dsflags = 0;
	dev_inadm.dc_ihflags = 0;
	dev_inadm.dc_ihlevel = 0;
	/*
	 * Parse the "config" lines of the stanza entry first.  This must
	 * be done prior to calling the driver's configure routine so that
	 * the appropriate kernel data structures are created.
 	 */
	if (cm_perform_config(logp, entry)) {
		return(CONFIG_EFAIL);
	}
	driver_name = dbattr_string(entry, MODULE_CONFIG_NAME, 
							"bogus_driver_name");
		bzero(dev_inadm.config_name, DEVNAMESZ);
	if (strcmp(driver_name, "bogus_driver_name") != 0) {
		if (strlen(driver_name) < DEVNAMESZ) {
			strcpy(dev_inadm.config_name, driver_name);
		}
	}
	/*
	 * Set this flag to indicate that the driver has been dynamically
	 * loaded.  This will enable any driver to make run-time decisions 
	 * where the functions performed by the loaded and static versions of
	 * the driver differ.
	 */
	dev_inadm.dc_dsflags |= IH_DRV_DYNAMIC;
	/*
	 * Look at the major number requirements field to see if the same
	 * major number is needed in both the block and character device
	 * switch tables.  If so setup flags passed down to the device
 	 * driver so that it knows to call dualdevsw_add.
	 */
	any_string = dbattr_string(entry, DEVICE_MAJOR_REQ, &null_string);
	convert_to_upper(any_string);
        if (strcmp(any_string, "SAME") == 0) {
		same_major_needed = 1;
		dev_inadm.dc_dsflags |= IH_DRV_SAMEMAJOR;
        }
	else if (*any_string != null_string) {
		cm_log(logp, LOG_ERR, "Invalid option `%s' for Device_Major_Req\n",
		       any_string);
		METHOD_LOG(LOG_ERR, SUBSYS_ADD_FAIL);
		cm_config_delete(logp, driver_name);
		return (-1);
	}
	/*
	 * Obtain the character device major number from the stanza entry.
	 * If it is specified as "Any" or "-1" then set dc_cmajnum to NODEV 
	 * which will cause cdevsw_add to use the next available slot.
	 */
	any_string = dbattr_string(entry, DEVICE_CHRMAJOR, &null_string);
	convert_to_upper(any_string);
	if ((strcmp(any_string, "ANY") == 0) || 
	    (strcmp(any_string, "-1") == 0)) {
		dev_inadm.dc_cmajnum = NODEV;
	}
	else if (is_a_number(any_string)) {
	    /*
	     * Notice here that the default is UNUSED_MAJOR.  This would be the
	     * case if the DEVICE_CHRMAJOR entry was not specified in the 
	     * stanza. For this case you don't want the driver to attempt to
	     * allocate a character major number.  To insure that this does
	     * not happen set the requested major number to UNUSED_MAJOR so that
	     * the cdevsw_try routine will always fail.
	     */
	    dev_inadm.dc_cmajnum = 
		dbattr_num(entry, DEVICE_CHRMAJOR, UNUSED_MAJOR);
	}
	else {
	    /*
	     * The option selected is unknown.  Return an error message and
	     * abort the load.
	     */
	    cm_log(logp, LOG_ERR, "Invalid argument `%s' for Device_Char_Major.\n",
		   any_string);
	    METHOD_LOG(LOG_ERR, SUBSYS_ADD_FAIL);
	    cm_config_delete(logp, driver_name);
	    return (-1);
	}
	if (dev_inadm.dc_cmajnum != UNUSED_MAJOR) {
	    char_major_used = 1;
	}
	/*
	 * Obtain the block device major number from the stanza entry.
	 * If it is specified as "Any" or "-1" then set dc_bmajnum to NODEV 
	 * which will cause bdevsw_add to use the next available slot.
	 */
	any_string = dbattr_string(entry, DEVICE_BLKMAJOR, &null_string);
	convert_to_upper(any_string);
	if ((strcmp(any_string, "ANY") == 0) || 
	    (strcmp(any_string, "-1") == 0)) {
		dev_inadm.dc_bmajnum = NODEV;
	}
	else if (is_a_number(any_string)) {
	    /*
	     * Notice here that the default is UNUSED_MAJOR.  This would be the
	     * case if the DEVICE_BLKMAJOR entry was not specified in the 
	     * stanza. For this case you don't want the driver to attempt to
	     * allocate a block major number.  To insure that this does
	     * not happen set the requested major number to UNUSED_MAJOR so that
	     * the bdevsw_try routine will always fail.
	     */
	    dev_inadm.dc_bmajnum = 
		dbattr_num(entry, DEVICE_BLKMAJOR, UNUSED_MAJOR);
	}
	else {
	    /*
	     * The option selected is unknown.  Return an error message and
	     * abort the load.
	     */
	    cm_log(logp, LOG_ERR, "Invalid argument `%s' for Device_Block_Major.\n",
		   any_string);
	    METHOD_LOG(LOG_ERR, SUBSYS_ADD_FAIL);
	    cm_config_delete(logp, driver_name);
	    return (-1);
	}

	if (dev_inadm.dc_bmajnum != UNUSED_MAJOR) {
	    block_major_used = 1;
	}
	/*
	 * If the major number requirements field has indicated that the
	 * same major number is needed see if the stanza writer has played by
	 * the rules in actually stating the same thing for the needs of the
	 * block and character number.  By putting the check here the driver
	 * writer doesn't have to guess at which of block/char to look at
	 * for the desired major number.
	 */
	if (same_major_needed) {
		if (dev_inadm.dc_bmajnum != dev_inadm.dc_cmajnum) {
			cm_log(logp, LOG_ERR, "%s: bmajor %d != cmajor %d\n",
			AFentname(entry), dev_inadm.dc_bmajnum,
			dev_inadm.dc_cmajnum);
			/*
		 	 * The load has failed.  Call cm_config_delete to insure
		 	 * that all the kernel resident configuration structures
		 	 * get removed.
		 	 */
			cm_config_delete(logp, driver_name);
				return(KMOD_ENODEV);
			}
	}
	/*
	 * Set device interrupt level field as specified in the stanza
	 * entry.  (This information would perhaps be better specified
	 * in the "config" lines though.)
	 */
	dev_inadm.dc_ihlevel = dbattr_num(entry, DEVICE_IHLEVEL, -1);

	/*
	 * Jump off to the driver's configure routine.
	 */
	if ((rc=cm_kls_call(dev_mod->dev_id, SYSCONFIG_CONFIGURE,
		    &(dev_inadm), sizeof(device_config_t),
		    &dev_mod->dev_outadm, sizeof(device_config_t))) != 0) {
		if (rc == KLDR_EFAIL) {
		    cm_log(logp, LOG_ERR, "%s: configure returned error: %s\n",
				AFentname(entry), strerror(errno));
		}
		else {
		    cm_log(logp, LOG_ERR, "%s: cm_kls_call failed configure.\n",
				AFentname(entry));
		}
		/*
		 * If the configure has failed cause kloadsrv to
		 * unload the module.   If this wasn't
		 * done kloadsrv would not reload the image on
		 * subsequent calls to configure the driver.  This
		 * is not desirable if you are replacing the load
		 * module to fix the failure of the configure routine.
		 */
		cm_kls_unload(dev_mod->dev_id);

		/*
		 * The load has failed.  Call cm_config_delete to insure
		 * that all the kernel resident configuration structures
		 * get removed.  Remove the driver's state since the load
		 * has failed.  The driver's state does not have to
		 * be removed from the kernel because it is only added
		 * into the kernel later in this routine if the
		 * driver configuration has succeeded.
		 */
		cm_config_delete(logp, driver_name);
		if (DEVICE_delete_devmod(logp, AFentname(entry)) == 0) {
			DEVICE_free_devmod(dev_mod);
		}
		else {
			METHOD_LOG(LOG_ERR, SUBSYS_DEL_FAIL);
		}

		return(rc);
	}
	/*
	 * All of the driver's config entries should have been
	 * consumed by the stanza resolver which gets called from
	 * the driver's configure routine.  If an error occurred some
	 * stanza entries may be lingering.  To make sure that there
	 * are no left-over configuration entries make the following
	 * call to delete them. 
	 */
	cm_config_delete(logp, driver_name);

	/*
	 * If the block or character major number is not used set it back
	 * to NODEV.  This way the driver doesn't have to insure that this
	 * unused field is setup appropriately.
	 */
	if (block_major_used == 0) {
	    dev_mod->dev_outadm.dc_bmajnum = NODEV;
	}
	if (char_major_used == 0) {
	    dev_mod->dev_outadm.dc_cmajnum = NODEV;
	}
	/*
	 * Save off the config name so that unwanted config structs can
	 * be deleted later.
	 */
	strcpy(dev_mod->dev_outadm.config_name, dev_inadm.config_name);

	DEVICE_prtcfg(logp, entry, dev_mod, "configured");
	/*
	 * Only create the device special files if there is a need.  If
	 * the stanza indicates that the major number of block and character
	 * must be the same and the driver didn't return that then the
	 * device special files will not be created.
	 *
	 * The return value of DEVICE_mknods is not checked here.  If that
	 * fails the user won't have access to the driver.  Should this happen
	 * they could unload the driver, fix the stanza entry and then try to
	 * reload the driver.
	 */
	if (block_major_used || char_major_used) {
		if ((same_major_needed == 0) || (dev_mod->dev_outadm.dc_bmajnum
				== dev_mod->dev_outadm.dc_cmajnum)) {
			DEVICE_mknods(logp, entry, dev_mod, MKNOD_CREATE);
		}
		else {
			cm_log(logp, LOG_ERR, "%s: no mknods performed.\n",
				AFentname(entry));
		}
	}
	dev_mod->dev_load_flags |= DEV_CONFIGURED;
	if (DEVICE_set_state(logp, entry, dev_mod, ADD_STATE) != 0) {
		cm_log(logp, LOG_ERR, 
			"%s: Unable to set method state in kernel.\n",
				AFentname(entry));
	}
	return(0);
}


/*
 * Unconfigure a driver.  This consists of calling the driver's configure 
 * routine with the unconfigure op-code.  Any device special files 
 * associated with the driver will be deleted.  The driver's state will be
 * removed from the kernel list.
 */
int
DEVICE_method_unconfigure( cm_log_t * logp, ENT_t entry, dev_mod_t * dev_mod )
{
	int	rc;

	if (dev_mod == NULL) {
		return(SUBSYS_GET_FAIL);
	}
	if ((dev_mod->dev_load_flags & DEV_LOADED) == 0)
		return(KMOD_UNCONFIG_L_EEXIST);
	if ((dev_mod->dev_load_flags & DEV_CONFIGURED) == 0)
		return(KMOD_UNCONFIG_C_EEXIST);
	if ((rc=cm_kls_call(dev_mod->dev_id, SYSCONFIG_UNCONFIGURE,
		NULL, 0, NULL, 0)) != 0) {
		if (rc == KLDR_EFAIL) {
			cm_log(logp, LOG_ERR, "%s: %s\n", AFentname(entry),
				strerror(errno));
		}
		else {
			cm_log(logp, LOG_ERR, 
				"%s: cm_kls_call failed unconfigure.\n",
				AFentname(entry));
		}
		return(rc);
	}

	DEVICE_prtcfg(logp, entry, dev_mod, "deconfigured");
	DEVICE_mknods(logp, entry, dev_mod, MKNOD_DELETE);
	dev_mod->dev_load_flags &= ~DEV_CONFIGURED;
	if (DEVICE_set_state(logp, entry, dev_mod, DELETE_STATE) != 0) {
		cm_log(logp, LOG_ERR, 
			"%s: Unable to deletd method state from kernel.\n",
				AFentname(entry));
	}
	return(0);
}

/*
 * Call the driver's configure routine to query the driver.  Format the
 * output for display.
 *
 * NOTE: This method currently does not support a very rich query command.
 * The reason for that is that for 1.1 there will be several variants of
 * query.
 */
int
DEVICE_method_query( cm_log_t * logp, ENT_t entry, dev_mod_t * dev_mod )
{
	int	rc;

	if (dev_mod == NULL) {
		return(SUBSYS_GET_FAIL);
	}
	if ((dev_mod->dev_load_flags & DEV_LOADED) == 0)
		return(KMOD_UNCONFIG_L_EEXIST);
	if ((dev_mod->dev_load_flags & DEV_CONFIGURED) == 0)
		return(KMOD_UNCONFIG_C_EEXIST);
	if ((rc=cm_kls_call(dev_mod->dev_id, SYSCONFIG_QUERY,
		NULL, 0, &dev_mod->dev_outadm, sizeof(device_config_t))) != 0) {
		if (rc == KLDR_EFAIL) {
			cm_log(logp, LOG_ERR, "%s: %s\n", AFentname(entry),
				strerror(errno));
		}
		else {
			cm_log(logp, LOG_ERR, "%s: cm_kls_call failed query.\n",
				AFentname(entry));
		}
		return(rc);
	}

	DEVICE_prtcfg(logp, entry, dev_mod, "queried");
	return(0);
}


/*
 * Print out the contents of a dev_mod_t structure.  This is called in
 * response to a query operation to display information.
 */
int
DEVICE_prtcfg(cm_log_t * logp, ENT_t entry, dev_mod_t * dev_mod, char * string)
{
	dev_t bmaj, cmaj;

	if (dev_mod == NULL) {
		return(SUBSYS_GET_FAIL);
	}
	if (dev_mod->dev_outadm.dc_version == OSF_DEVICE_CONFIG_10) {
			/* Fix a sign extension problem */
			bmaj = dev_mod->dev_outadm.dc_bmajnum;
			cmaj = dev_mod->dev_outadm.dc_cmajnum;
			if (bmaj == 255) bmaj = -1;
			if (cmaj == 255) cmaj = -1;
			cm_log(logp, LOG_ERR,
				"%s: %s DEVICE  bdev:%d cdev:%d \n",
				AFentname(entry),
				string,
				bmaj,
				cmaj);
	} else {
		cm_log(logp, LOG_ERR, "%s: %s DEVICE \"%s\" module\n",
			AFentname(entry), string, dev_mod->dev_name);
	}
}


/*
 * DEVICE_lookup_str_mod
 * Function: This routine is called to get a pointer to a dev_mod_list entry
 *	     which is used to represent the state of the loaded driver.  First
 *	     a check is done to see if an entry for this driver already exists.
 *	     If an entry is not already there an unused entry will be
 *	     initialized.
 *
 *	     This routine queries the state of what is currently loaded.
 *
 * Returns: 0 -success = matching entry found or new entry allocated.
 *	    nonzero = no dev_mod_t entry is associated with this driver.
 *	    The second parameter "p" is set to the address of the
 *	    associated dev_mod_t entry.
 */
int
DEVICE_lookup_str_mod(cm_log_t * logp, char * entname, dev_mod_t ** p)
{
	int 	rc;
	int	i;
	dev_mod_t *modp;

	/*
	 * Extract the stanza entry name from the stanza entry.
	 * Validate that the name consists of at least 1 character but
	 * not too many characters.
	 */
	if ((entname == NULL)
		|| (i=strlen(entname)) > DEVNAMESZ || i < 1)
		return(KMOD_ENOENT);


	/*
	 * Look to see if a devmod structure already exists for the specified
	 * driver name.
	 */
	modp = DEVICE_find_devmod(logp, entname);
	if (modp != NULL) {
		*p = modp;
		return(0);
	}
	/*
	 * No matching entry has been found in the linked list.  This
	 * implies that the driver is not currently loaded.  Allocate a
	 * new devmod entry and set it up for this driver.
	 */
	modp = DEVICE_alloc_devmod();
	if (modp == NULL) {
		return(KMOD_ENOMEM);
	}
	/*
	 * Setup the fields of the devmod structure.
	 */
	strncpy(modp->dev_name, entname, DEVNAMESZ);
	modp->dev_load_flags &= ~(DEV_LOADED | DEV_CONFIGURED);
	modp->dev_id = LDR_NULL_MODULE;
	bzero((char *)&modp->dev_outadm,
		(sizeof(device_config_t)));
	/*
	 * Link the structure onto the linked list.
	 */
	if (DEVICE_add_devmod(logp, modp) != 0) {
		DEVICE_free_devmod(modp);
		return(KMOD_ENOMEM);
	}
	*p = modp;
	return(0);
}


/*
 * Create the device special files for the driver.  The actual mknods are
 * not performed here.  The function of this routine is to setup a
 * data structure defining the driver's mknod requirements and calling the
 * actual routine in the generic portion of cfgmgr to perform the actual
 * mknod's.
 *
 * This routine is also called to delete the device special files on driver
 * unload.
 */
int
DEVICE_mknods( cm_log_t * logp, ENT_t entry, dev_mod_t * dev_mod, int make)
{
	cm_devices_t 	devices;
	struct ATTR	files_attr;
	struct ATTR	minors_attr;
	char		files_buf[32];
	char		minors_buf[32];
	char *		unit_name;
	int		dev_major;
	int		dev_minor;
	int		clone_major;
	int		clone_minor;
	int		unit_major;
	int		unit_num;
	int		rc_block = 0;
	int		rc_char = 0;
	int		mknod_op;

	if (dev_mod == NULL) {
		return(-1);
	}
	/*
	 * This seems pretty harsh.
	 */
	if (dev_mod->dev_outadm.dc_version != OSF_DEVICE_CONFIG_10)
		return(-1);
	/* first do parts common to block and char devices */
	if (make == MKNOD_CREATE) {
		/*
		 * Setup the operation field to remove all files with the
	   	 * specified major number, remove files of the same name,
		 * create the new device special file and print out a message
		 * saying what is being done.
		 */
		mknod_op = 	CM_RMNOD_MAJR | CM_RMNOD_FILE |
				CM_MKNOD_FILE | CM_RPT_MKNOD | 
				CM_RPT_HEADER;
	}
	else {
		/*
		 * Setup the operation field to remove all files with the
	   	 * specified major number, remove files of the same name,
		 * and print out a message saying what is being done.
		 */
		mknod_op = 	CM_RMNOD_MAJR | CM_RPT_RMNOD | 
				CM_RPT_HEADER;
	}
	devices.dir = dbattr_string(entry, DEVICE_DIR, "/dev");
	/*
	 * Setup the file permissions if specified in the stanza entry.
	 * Note: any bits set in the umask of whoever is starting up
	 * cfgmgr will take affect and be masked off of the permissions
	 * specified here when the mknod system call is made.
	 */
	devices.mode = dbattr_mode(entry, DEVICE_MODE, DEVMODE_DFLT);
	devices.uid = dbattr_user(entry, DEVICE_USER, 0);
	devices.gid = dbattr_group(entry, DEVICE_GROUP, 0);

	/* Now do block device parts */
	devices.subdir = dbattr_string(entry, DEVICE_BLKSUBDIR, NULL);
	if (!devices.subdir) 
	  devices.subdir = dbattr_string(entry, DEVICE_SUBDIR, NULL);
	devices.type = DEVTYPE_BLK;
	devices.devfiles  = AFgetatr(entry, DEVICE_BLKFILES);
	devices.devminors = AFgetatr(entry, DEVICE_BLKMINOR);
	devices.majno	= dev_mod->dev_outadm.dc_bmajnum;
	if (devices.majno != NODEV) {
	    rc_block = cm_mknods(logp, AFentname(entry), mknod_op, &devices);
	}


	/* Now do char device parts */
	devices.subdir = dbattr_string(entry, DEVICE_CHRSUBDIR, NULL);
	if (!devices.subdir) 
	  devices.subdir = dbattr_string(entry, DEVICE_SUBDIR, NULL);
	devices.type = DEVTYPE_CHR;
	devices.devfiles  = AFgetatr(entry, DEVICE_CHRFILES);
	devices.devminors = AFgetatr(entry, DEVICE_CHRMINOR);
	devices.majno	= dev_mod->dev_outadm.dc_cmajnum;
	if (devices.majno != NODEV) {
	    rc_char = cm_mknods(logp, AFentname(entry), mknod_op, &devices);
	}
	
	if ((rc_char == 0) && (rc_block == 0)) {
		return(0);
	}
	if (rc_char != 0) {
		return(rc_char);
	}
	return(rc_block);
}

/*
 * Routines used to make this method stateless.
 *
 * NOTE: when we incorporate the 1.1 scalibility changes the following code
 * will become obsolete.  In 1.1 there is a new syscall (subsys_info) used
 * to maintain method state in a generic manner.  Since that code wasn't
 * available in time for this release the following driver method specific
 * routines are used to save the method's state via setsysinfo.
 */

/*
 * Dynamically allocate memory to be used as a devmod struct.
 *
 * Returns a pointer to that structure or NULL on error.
 */
dev_mod_t *
DEVICE_alloc_devmod()
{
	dev_mod_t *modp;

	modp = (dev_mod_t *)(malloc(sizeof(dev_mod_t)));
	if ((char *)modp <= (char *)0) {
		modp = NULL;
	}
	return(modp);
}
/*
 * Release dynamically allocated memory.  Bzero it out first to make any
 * dangling references obvious.
 */
int
DEVICE_free_devmod(modp)
	dev_mod_t *modp;
{
	/*
	 * Paranoia check.
	 */
	if (modp == NULL) {
		return(-1);
	}
	bzero((char *)modp, sizeof(dev_mod_t));
	free(modp);
	return(0);
}

/*
 * Walk the linked list of devmod structs looking for a match on the name
 * field.
 *
 * Returns a pointer to the matching devmod struct or NULL if no match
 * is found.
 */
dev_mod_t *
DEVICE_find_devmod(cm_log_t * logp, char *name)
{
	dev_mod_t *modp;

	/*
	 * Paranoia check first.
	 */
	if ((strlen(name) < 1) || (strlen(name) > DEVNAMESZ)) {
		return(NULL);
	}
	modp = devmod_head;
	while (modp != NULL) {
		if (strcmp(name, modp->dev_name) == 0) {
			return(modp);
		}
		modp = modp->next;
	}
	return(NULL);
}

/*
 * Add a new entry to the linked list of devmod structs.  
 *
 * Returns 0 on success, -1 on error.
 */
int
DEVICE_add_devmod(logp, modp)
	cm_log_t *logp;
	dev_mod_t *modp;
{
	dev_mod_t *other_modp;

	if (modp == NULL) {
		return(-1);
	}
	/*
	 * As a sanity check insure that there isn't already an entry of
	 * the same name on the list.  This check should have been performed
	 * prior to this call though.
	 */
	other_modp = DEVICE_find_devmod(logp, modp->dev_name);
	if (other_modp != NULL) {
		return(-1);
	}
	/*
	 * The list is not ordered.  Place the new devmod at the head of
	 * the list.
	 */
        if (devmod_head != NULL) {
                devmod_head->prev = modp;
        }
	modp->next = devmod_head;
	modp->prev = NULL;
	devmod_head = modp;

	return(0);
}

/*
 * Delete a driver from the linked list of devmod structs.
 *
 * Returns 0 - success, -1 error.
 */
int
DEVICE_delete_devmod(logp, name)
	cm_log_t *logp;
	char *name;
{
	dev_mod_t *modp;
	dev_mod_t *next_modp;
	dev_mod_t *prev_modp;

	modp = DEVICE_find_devmod(logp, name);
	if (modp == NULL) {
		return(-1);
	}

	next_modp = modp->next;
	prev_modp = modp->prev;

	if ((next_modp == NULL) && (prev_modp == NULL)) {
		devmod_head = NULL;
	}
	else {
		if (prev_modp == NULL) {
			devmod_head = next_modp;
		}
		else {
			prev_modp->next = next_modp;
		}
		if (next_modp != NULL) {
			next_modp->prev = modp->prev;
		}
	}
	return(0);
}
/*
 * Via getsysinfo retrieve the state of any previously loaded drivers.
 * NOTE: this will change with the 1.1 registration table.
 *
 * Returns 0 on success, -1 on error.
 */
int
DEVICE_get_state( cm_log_t *logp, ENT_t entry )
{
	long start = 0;
	dev_mod_t modbuf;
	dev_mod_t *modp;
	dev_mod_t *modp_next;
	dev_mod_t *modp_prev;
	long *prev_start = 0;

	/*
	 * It is only necessary to query the state once.  After that assume
	 * that the cfgmgr's version of state is consistent with the kernel's
	 * version of the same state.
	 */
	device_method_state_known = 1;
	/*
	 * By passing an address of 0 in the start field the getsysinfo
	 * call will return the first entry on the list.
	 */
	if (getsysinfo(GSI_DEV_MOD,&modbuf,sizeof(modbuf),&start,0) != 0) {
		cm_log(logp, LOG_ERR, "%s: unable to get state from kernel.\n",
			 AFentname(entry));
		return(-1);
	}
	/*
	 * The name field will be null when the end of the list of entries
	 * has been reached.
	 */
	while (modbuf.dev_name[0]) {
		/*	
		 * A paranoia check to see if the entry looks reasonable.
		 */
		if (strlen(modbuf.dev_name) > DEVNAMESZ) {
			cm_log(logp, LOG_ERR, "%s: invalid name length.\n",
				 AFentname(entry));
			return(-1);
		}
		/*
		 * If the entry is already in the list a new one will not 
		 * be allocated.  Copy in the fields from the kernel.  Of
		 * particular interest is the id field to allow further
		 * operations on the module.
		 */
		if (DEVICE_lookup_str_mod(logp, modbuf.dev_name, &modp)) {
			METHOD_LOG(LOG_ERR, SUBSYS_ADD_FAIL);
			return(-1);
		}
		/*
		 * The kernel has its own version of a linked list where
		 * the next and prev pointers are different.  Save off the
		 * propper pointers prior to doing this bcopy, then replace
		 * with the user space pointers.
		 */
		modp_next = modp->next;
		modp_prev = modp->prev;
		bcopy(&modbuf, modp, sizeof(modbuf));
		modp->next = modp_next;
		modp->prev = modp_prev;
		modp->dev_load_flags |= (DEV_LOADED | DEV_CONFIGURED);
		/*
		 * The variable start will be setup to point to the next
		 * entry in the kernel linked list.  Use that to retrieve
		 * the next entry.  If it is set to 0 the end of the list
	 	 * has been encountered.
		 */
		if (prev_start == (long *)start) {
			break;
		}
		prev_start = (long *)start;
		if (start == NULL) {
			break;
		}
		(void)getsysinfo(GSI_DEV_MOD,&modbuf,sizeof(modbuf), &start, 0);
	}
	return(0);
}

/*
 * Via setsysinfo set the driver state in the kernel.  This is done so that if
 * cfgmgr is killed and restarted the state of what was loaded can be
 * retrieved.
 * NOTE: this will change with the 1.1 registration table.
 *
 * Returns 0 on success, -1 on error.
 */
int
DEVICE_set_state(cm_log_t *logp, ENT_t entry, dev_mod_t *modp, int set_type)
{
	struct driver_ssi ssi_struct;

	/* sanity check */
	if (modp == NULL) {
		return(-1);
	}
	if (set_type == ADD_STATE)
		ssi_struct.op = SSIN_LOAD_DEVSTATE;
	else
		ssi_struct.op = SSIN_UNLOAD_DEVSTATE;
	bcopy(modp, &ssi_struct.devmod, sizeof(dev_mod_t));
	if (setsysinfo(SSI_NVPAIRS,&ssi_struct,1,0,0) != 0) {
		cm_log(logp, LOG_ERR, cm_msg(CONFIG_SSI));
		return(-1);
	}
	return(0);
}
/*
 * Simple routine to convert a null terminated string to uppercase.
 */
static void
convert_to_upper (char *string)
{
   char *sptr;

   if (string == NULL) return;
   for (sptr = string; (*sptr != '\n') && (*sptr != NULL); sptr++) {
       if (islower(*sptr))
	  *sptr = toupper(*sptr);
   }
}
/*
 * Simple routine to check if the supplied string is a number
 * or not.
 *
 * Returns:  0 if false
 *	     1 if true
 */
static int
is_a_number(char *string)
{
   char	*sptr;

   for (sptr = string; (*sptr != '\n') && (*sptr != NULL); sptr++) {
       if (!isdigit(*sptr))
	   return (0);
   }
   return (1);
}
