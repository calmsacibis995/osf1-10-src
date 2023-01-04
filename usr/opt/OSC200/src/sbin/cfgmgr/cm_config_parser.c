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
static char *rcsid = "@(#)$RCSfile: cm_config_parser.c,v $ $Revision: 1.1.3.5 $ (DEC) $Date: 1992/06/25 09:48:36 $";
#endif

/*
 * This file contins the routines needed to parse in the device related lines
 * from a stanza entry.  These lines are of the same format as used by the
 * config utility.
 *
 * This code is being placed in the generic portion of cfgmgr so that it
 * can be utilized by both the driver method and the streams method.
 */
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/sysinfo.h>
#include <machine/hal_sysinfo.h>
#include <sys/un.h>
#include <locale.h>

#include <machine/devdriver.h>

#include "cm.h"

#define MAX_CONFIG_LINES	CFGMGR_MAXATRNUM /* Max num of config lines */
#define MAX_WORD_SIZE 		256
/*
 * Types of connections to mark unknown and wildcarded.
 */
#define CONFIG_WILDCARDED (-99) /* Denotes a wildcarded connection */
#define CONFIG_QUES	(-99)	/* -99 means '?' */
#define CONFIG_UNKNOWN (-2)	/* -2 means not set yet */
#define CONFIG_MISSING (-3)	/* -3 means not specified yet */
#define CONFIG_TO_NEXUS       ((struct device_entry *)-1)

#define SUCCESS_RETURN	0	/* Routines return 0 for success status  */
#define ERROR_RETURN	1	/* Routines return 1 (non-zero) on error */
/*
 * Each device config line will be "translated" into a structure of this
 * type which gets malloc'ed on a per line basis.  This intermediate
 * translation is being done to enable the code here to be modeled off
 * the way the config program parses device config lines.
 *
 * Note: a lot of the fields that config supports have been removed here.
 * If a field is not present in this structure the support for it is not
 * present here.  For example there are some mscp, and vme fields not present.
 * Some fields are not applicable for loadable drivers because they are
 * specified within the driver itself.  Examples of this are interrupt
 * vectors, priorities and other interrupt related fields.  The driver
 * registers its interrupt related fields via the handler_add function.
 */
struct device_entry {
	int	d_type;			/* CONTROLLER, DEVICE, or BUS 	      */
	char    d_type_string[MAX_NAME];/* Holds device name.  		      */
	struct	device_entry *d_conn;	/* what it is connected to 	      */
	char	d_name[MAX_NAME];	/* name of device (e.g. tc,vba)       */
	int	d_slot;			/* slot or node number	              */
	int	d_unit;			/* unit number 			      */
	int	d_drive;		/* drive number 		      */
	int	d_nexus;		/* specifies bus connected to nexus   */
	struct	device_entry *d_next;	/* Next one in list 		      */
	char	d_wildcard[MAX_NAME];	/* marks wildcard connection 	      */
	char	d_conn_name[MAX_NAME];	/* name of missing connecting node    */
	int	d_conn_num;		/* number of missing connecting node  */
};

/*
 * Name-value pair structure to pass off to setsysinfo.
 */
struct config_ssi {
	int	op;			/* SSI operation type */
	struct config_entry entry;	/* Actual data passed */
};

struct device_entry *dtab;		/* Pointer to device table */
struct device_entry *last_device_entry; /* The end of dtab         */
struct bus *bus_list;			/* Head of bus list */
struct controller *controller_list;	/* Head of controller list */
struct device *device_list;		/* Head of device list */
char *WILDCARD = "*";			/* Wildcarded bus	   */


/*
 * Routine templates.
 */
int	config_bus(), config_controller(), config_device(); 
int	config_module();
int	getslot(), getvector(), getdrive();
void	free_lists();
void    init_device_entry();
void	link_bus(), link_controller(),link_device();
int	add_bus_entry(), add_controller_entry(), add_device_entry();
void	convert_dtab();
int	pass_entry(), pass_lists(), getoptions();
int	getword(), needword(), parse_config(), cm_perform_config(), test_config();
int	cm_config_delete(), break_name();
char	*getstruct();
struct bus *locate_bus();
struct controller *locate_controller();
struct device *locate_device();
struct device_entry *get_device_entry();
struct device_entry *findentry();

/*
 * Dispatch table used to locate the appropriate routine to be called to
 * parse the specific types of config lines.
 */
static struct config_dispatch {
	char	*config_type;		/* Type of config line		  */
	int	(*config_routine)();	/* Parsing function for this type */
} conf_dispatch_table[] = {
	{ "bus",	config_bus 		},
	{ "controller",	config_controller 	},
	{ "device",	config_device 		},
	{ NULL,		0 			}
};

/*
 * List of supported device types.  For configuration lines marked as
 * device type the next word on the config line must be one of the 
 * following:
 */
static char * device_types[] = {
	"disk",
	"tape",
	"\0"
};

/*
 * Dispatch table for the various options.  Options is "stuff" found at the
 * end of a config line that is not required.
 */
static struct options_dispatch {
	char	*name;
	int	(*option_routine)();
} option_table[] = {
	{ "slot",	getslot		},
	{ "vector",	getvector	},	/* tollerated, not supported */
	{ "drive",	getdrive	},	/* unit and drive are same   */
	{ "unit",	getdrive	},
	{ NULL,		0		}
};

/*
 * Initialize global variables.  This is called each time the parsing is
 * done for a new loaded subsystem.
 */
init_globals()
{
	dtab = NULL;
}

/*
 * Initialize a device entry struct.
 * If no slot, or drive is specified it is assumed to be wildcarded.
 */
void
init_device_entry(dp)
	struct device_entry *dp;
{
	if (dp != NULL) {
		bzero(dp, sizeof(struct device_entry)); /* paranoia, zap */
		/* Only change the non-zero ones. */
		strcpy(dp->d_name, "OHNO!!!");
		dp->d_type = BOGUS_TYPE;
		dp->d_slot = CONFIG_WILDCARDED;
		dp->d_drive = CONFIG_WILDCARDED;
		dp->d_unit = dp->d_nexus = CONFIG_UNKNOWN;
	}
}
/*
 * Dynamically allocate a device_entry struct.  
 */
struct device_entry *
get_device_entry(logp)
	cm_log_t *logp;
{
	struct device_entry *dip;

	dip = (struct device_entry *)(malloc(sizeof(struct device_entry)));
	if ((char *)dip <= (char *)0) {
		dip = NULL;
		cm_log(logp, LOG_ERR, cm_msg(CONFIG_ENOMEM));
	}
	init_device_entry(dip);
	return(dip);
}
/*
 * Copies the first word of the input line into new_word.  The rest_of_line 
 * pointer is set to index after the first word to describe the remainder of
 * the line.
 *
 * Returns: 1 on end of line or comment character. 0 otherwise on success.
 */
#define COMMENT_CHAR	'#'
int
getword(logp, input_line, new_word, rest_of_line)
	cm_log_t *logp;
	char *input_line;	
	char *new_word;
	char **rest_of_line;
{
	char c;

	*new_word = NULL;
	c = *input_line;
	/* If the first character is a # then skip the rest of the line */
	if ((c == NULL) || (c == COMMENT_CHAR)) {
		return(ERROR_RETURN);
	}
	/* Strip off any whitespace */
	while ((c != NULL) && (isspace(c))) {
		input_line++;
		c = *input_line;
	}
	if (c == NULL) {
		return(ERROR_RETURN);
	}
	/* At this point we're up to the first non-whitespace character */
	while ((c != NULL) && ((isspace(c) == 0))) {
		*new_word++ = c;
		input_line++;
		c = *input_line;
	}
	*new_word = NULL;
	*rest_of_line = input_line;
	return(SUCCESS_RETURN);
}

/*
 * Fetch the next word in the config line and make sure that it matches
 * the specified word.  This is done to check for required fields in the 
 * config line.  For example "at".  Returns 1 on match, NULL on mismatch.
 */
int
needword(logp, input_line, expected_word, rest_ptr)
	cm_log_t *logp;
	char *input_line;	
	char *expected_word;
	char **rest_ptr;
{
	char new_word[MAX_NAME];

	if (getword(logp, input_line, new_word, rest_ptr)) {
		cm_log(logp, LOG_ERR, cm_msg(CONFIG_NEXT_WORD));
		return(NULL);
	}
	if (strcmp(expected_word, new_word) != 0) {
		cm_log(logp, LOG_ERR, cm_msg(CONFIG_NOT_EXPECTED),
			expected_word, new_word);
		return(NULL);
	}
	return(ERROR_RETURN);
}

/*
 * This routine is passed an individual "config" line from the stanza entry.
 * Here the parsing process begins.
 * Returns 0 on success, 1 on error.
 */
int
parse_config(logp, config_name, config_string)
	cm_log_t *logp;
	char *config_name;
	char *config_string;
{
	int linelen;
	char cnfg_type[MAX_WORD_SIZE];
	struct config_dispatch *config_dispatch_ptr;
	char *rest_of_line;
	char **rest_ptr = &rest_of_line;

	linelen = strlen(config_string);
	/*
	 * Note: the config lines from the stanza entry are not newline
	 * terminated; they are null-terminated.
 	 */
	if (linelen <= 0) {	/* nothing to do */
		return(SUCCESS_RETURN);
	}
	/*
	 * The first word of the line should specify the type of line 
	 * being represented.  Based on the entry type dispatch to the
	 * appropriate routine to construct a data structure.
	 */
	if (getword(logp, config_string, cnfg_type, rest_ptr)) {
		return(SUCCESS_RETURN);
	}
	for (config_dispatch_ptr = conf_dispatch_table; 
	     config_dispatch_ptr->config_type != NULL;
	     config_dispatch_ptr++) {
		if (strcmp(cnfg_type, config_dispatch_ptr->config_type) == 0) {
			break;
		}
	}
	if (config_dispatch_ptr->config_type != NULL) {
		/*
		 * The input lines will be translated into device_entry
		 * data structures which will be malloc'ed onto a linked
		 * list.  If the list is currently empty, then initialize
	 	 * it to point to the first entry.
	 	 */
		if (config_dispatch_ptr->config_routine  != NULL) {
			if (dtab == NULL) {
				dtab = get_device_entry(logp);
				last_device_entry = dtab;
				if (dtab == NULL) {
				   cm_log(logp, LOG_ERR, cm_msg(CONFIG_ENOMEM));
					return(ERROR_RETURN);
				}
			}
			else {
				last_device_entry->d_next = get_device_entry(logp);
				if (last_device_entry->d_next == NULL) {
				   cm_log(logp, LOG_ERR, cm_msg(CONFIG_ENOMEM));
					return(ERROR_RETURN);
				}
				last_device_entry = last_device_entry->d_next;
			}
			if ((*config_dispatch_ptr->config_routine)
			  (logp,config_name, rest_of_line, last_device_entry)) {
				return(ERROR_RETURN);
			}
		}
	}
	else {
		cm_log(logp, LOG_ERR, cm_msg(CONFIG_ETYPE), 
					cnfg_type, config_name);
		return(ERROR_RETURN);
	}
	return(SUCCESS_RETURN);
}

/*
 * Dynamically allocate memory of specified size and bzero.
 * Returns a pointer to the memory space.  It is expected that the caller
 * will cast the return pointer to point to a structure of the specified type.
 */
char *
getstruct(logp, struct_size)
	cm_log_t *logp;
	int struct_size;
{
	char *data_ptr;

	if (struct_size < 0) {
		cm_log(logp, LOG_ERR, cm_msg(CONFIG_ESIZE), struct_size);
		return(NULL);
	}
	data_ptr = (char *)(malloc(struct_size));
	if ((char *)data_ptr <= (char *)0) {
		cm_log(logp, LOG_ERR, cm_msg(CONFIG_ENOMEM));
		return(NULL);
	}
	bzero(data_ptr, struct_size); /* paranoia, zap */
	return(data_ptr);
}

/*
 * Deallocate all memory which has been allocated for device configuration.
 */
void
free_lists()
{
	struct bus *bp, *bp_prev;
	struct controller *cp, *prev_cp;
	struct device *dp, *prev_dp;
	struct device_entry *devp, *prev_devp;

	bp = bus_list;
	while (bp != NULL) {
		bp_prev = bp;
		bp = bp->nxt_bus;
		free(bp_prev);
	}
	bus_list = NULL;
	cp = controller_list;
	while (cp != NULL) {
		prev_cp = cp;
		cp = cp->nxt_ctlr;
		free(prev_cp);
	}
	controller_list = NULL;
	dp = device_list;
	while (dp != NULL) {
		prev_dp = dp;
		dp = dp->nxt_dev;
		free(prev_dp);
	}
	device_list = NULL;

	devp = dtab;
	while (devp != NULL) {
		prev_devp = devp;
		devp = devp->d_next;
		free(prev_devp);
	}
	devp = NULL;
}

/*
 * Search the bus list for the specified bus name and number.  Returns a pointer
 * to the bus struct or NULL if no match is found.
 */
struct bus *
locate_bus(busname, bus_number)
	char *busname;
	int bus_number;
{
	struct bus *bp;

	bp = bus_list;
	while (bp != NULL) {
		if ((strcmp(bp->bus_name, busname) == 0) &&
		    (bp->bus_num == bus_number)) {
			return(bp);
		}
		bp = bp->nxt_bus;
	}
	return(NULL);
}

/*
 * Search the controller list for the specified device name and number.  
 * Returns a pointer to the controller struct or NULL if no match is found.
 */
struct controller *
locate_controller(ctlrname, ctlr_number)
	char *ctlrname;
	int ctlr_number;
{
	struct controller *cp;

	cp = controller_list;
	while (cp != NULL) {
		if ((strcmp(cp->ctlr_name, ctlrname) == 0) &&
		    (cp->ctlr_num == ctlr_number)) {
			return(cp);
		}
		cp = cp->nxt_ctlr;
	}
	return(NULL);
}

/*
 * Search the device list for the specified device name and number.  Returns a 
 * pointer to the device struct or NULL if no match is found.
 *
 * Note: this currently only checks the logical unit and not the physical
 * unit as well.  This may have to be added also.
 */
struct device *
locate_device(devicename, device_number)
	char *devicename;
	int device_number;
{
	struct device *dp;

	dp = device_list;
	while (dp != NULL) {
		if ((strcmp(dp->dev_name, devicename) == 0) &&
		    (dp->logunit == device_number)) {
			return(dp);
		}
		dp = dp->nxt_dev;
	}
	return(NULL);
}

/*
 * Add a bus structure to the end of the bus list.
 */
void
link_bus(logp, bp)
	cm_log_t *logp;
	struct bus *bp;
{
	struct bus *busptr;

	/*
	 * Before adding in a bus make sure it isn't already in
	 * the bus list.  This should have already been weeded out by this
	 * point in the processing though.
	 */
	if (locate_bus(bp->bus_name, bp->bus_num) != (struct bus *)NULL) {
		cm_log(logp, LOG_ERR, cm_msg(CONFIG_ELINKED),
				bp->bus_name, bp->bus_num);
		return;
	}
	if (bus_list == NULL) {
		bus_list = bp;
	}
	else {
		busptr = bus_list;
		while (busptr->nxt_bus != NULL) {
			busptr = busptr->nxt_bus;
		}
		busptr->nxt_bus = bp;
	}
}

/*
 * Add a controller structure to the end of the controller list.
 */
void
link_controller(logp, cp)
	cm_log_t *logp;
	struct controller *cp;
{
	struct controller *ctlrptr;

	/*
	 * Before adding in a controller make sure it isn't already in
	 * the controller list.  This should have already been weeded out by 
	 * this point in the processing though.
	 */
	if (locate_controller(cp->ctlr_name, cp->ctlr_num) != 
					(struct controller *)NULL) {
		cm_log(logp, LOG_ERR, cm_msg(CONFIG_ELINKED),
			cp->ctlr_name, cp->ctlr_num);
		return;
	}
	if (controller_list == NULL) {
		controller_list = cp;
	}
	else {
		ctlrptr = controller_list;
		while (ctlrptr->nxt_ctlr != NULL) {
			ctlrptr = ctlrptr->nxt_ctlr;
		}
		ctlrptr->nxt_ctlr = cp;
	}
}

/*
 * Add a device structure to the end of the device list.
 */
void
link_device(logp, dp)
	cm_log_t *logp;
	struct device *dp;
{
	struct device *devptr;

	/*
	 * Before adding in a device make sure it isn't already in
	 * the device list.  This should have already been weeded out by 
	 * this point in the processing though.
	 *
	 * Note; This only checks the logical unit and not the physical unit.
	 * This check may have to be added.
	 */
	if (locate_device(dp->dev_name, dp->logunit) != 
					(struct device *)NULL) {
		cm_log(logp, LOG_ERR, cm_msg(CONFIG_ELINKED),
			dp->dev_name, dp->logunit);
		return;
	}
	if (device_list == NULL) {
		device_list = dp;
	}
	else {
		devptr = device_list;
		while (devptr->nxt_dev != NULL) {
			devptr = devptr->nxt_dev;
		}
		devptr->nxt_dev = dp;
	}
}

/*
 * Allocate and setup the fields of the bus structure.
 */
int
add_bus_entry(logp, dp)
	cm_log_t *logp;
	struct device_entry *dp;
{
	struct bus *bp;

	if (dp->d_type != BUS_TYPE) {	/* paranoia */
		return;
	}

	bp = (struct bus *)getstruct(logp, sizeof(struct bus));
	if (bp == NULL) {
		return(ERROR_RETURN);
	}

	/*
	 * A lot of the fields are just set to 0.  This is essentially a
	 * no-op as the structure is bzero'ed anyways.  They are being done
	 * here to note that the field has not been overlooked.
	 */
	bp->nxt_bus = 0;
	bp->ctlr_list = 0;
	bp->bus_hd = 0;
	bp->bus_list = 0;
	bp->bus_type = 0;
	bp->bus_name = dp->d_name;
	bp->bus_num = dp->d_unit;
	bp->slot = dp->d_slot;
	if(strcmp(dp->d_wildcard, "*") == 0) {
		 bp->connect_bus = dp->d_wildcard;
		 bp->connect_num = CONFIG_WILDCARDED;
	}
	else {
	    switch ((long)dp->d_conn){
	      case ((int)CONFIG_TO_NEXUS):
		bp->connect_bus = "nexus";
		bp->connect_num  = -1;
		break;
	      case 0:
	      case CONFIG_UNKNOWN:
		bp->connect_bus = "";
		bp->connect_num  = 0;
		break;
	      /*
	       * The stanza line states that the bus is connected to a node
	       * that was not previously specified in the stanza file.  The
	       * missing node may have been statically configured or loaded
	       * in a separate module.  Hope that the user knows what they are
	       * doing in this case.
	       */
	      case CONFIG_MISSING:
	 	bp->connect_bus = dp->d_conn_name;
	 	bp->connect_num = dp->d_conn_num;
		break;
	      default:
		bp->connect_num	= dp->d_conn->d_unit;
		bp->connect_bus = dp->d_conn->d_name ?
				dp->d_conn->d_name:"";
	    };
	}
	bp->confl1 = 0;
	bp->confl2 = 0;
	bp->pname	= "";
	bp->port	= 0;
	bp->intr = 0;
	bp->alive = 0;
	bp->framework = NULL;
	/*
	 * Just leave these fields at 0: private, conn_priv, rsvd.
	 */
	/*
	 * Chain the new bus structure onto the bus list.
	 */
	link_bus(logp, bp);
	return(SUCCESS_RETURN);
}

/*
 * Add a new controller structure to the linked list.
 */
int
add_controller_entry(logp, dp)
	cm_log_t *logp;
	struct device_entry *dp;
{
	struct controller *cp;

	if (dp->d_type != CONTROLLER_TYPE) {	/* paranoia */
		return;
	}

	cp = (struct controller *)getstruct(logp, sizeof(struct controller));
	if (cp == NULL) {
		return(ERROR_RETURN);
	}

	/*
	 * A lot of the fields are just set to 0.  This is essentially a
	 * no-op as the structure is bzero'ed anyways.  They are being done
	 * here to note that the field has not been overlooked.
	 */
	cp->nxt_ctlr 	= 0;
	cp->dev_list 	= 0;
	cp->bus_hd   	= 0;
	cp->driver	= 0;
	cp->ctlr_type	= 0;
	cp->ctlr_name	= dp->d_name;
	cp->ctlr_num	= dp->d_unit;
	/* If the entry is wildcard connected make this by setting the
	 * bus name its connected to as "*" and the bus number to 
	 * CONFIG_WILDCARDED.
	 */
	if (strcmp(dp->d_wildcard, "*") == 0) {
		cp->bus_name = dp->d_wildcard;
		cp->bus_num  = CONFIG_WILDCARDED;
	}
	else {
	    switch ((long)dp->d_conn){
	      case ((int)CONFIG_TO_NEXUS):
		cp->bus_name = "nexus";
		cp->bus_num  = -1;
		break;
	      case 0:
	      case CONFIG_UNKNOWN:
		cp->bus_name = "";
		cp->bus_num  = 0;
		break;
	      /*
	       * The stanza line states that the ctlr is connected to a node
	       * that was not previously specified in the stanza file.  The
	       * missing node may have been statically configured or loaded
	       * in a separate module.  Hope that the user knows what they are
	       * doing in this case.
	       */
	      case CONFIG_MISSING:
	 	cp->bus_name = dp->d_conn_name;
	 	cp->bus_num = dp->d_conn_num;
		break;
	      default:
		cp->bus_name = dp->d_conn->d_name;
		cp->bus_num  = dp->d_conn->d_unit;
	    };
	}
	cp->rctlr	= 0;
	cp->slot	= dp->d_slot;
	cp->alive	= 0;
	cp->pname 	= "";
	cp->port	= 0;
	cp->addr	= (caddr_t)0;
	cp->addr2 	= (caddr_t)0;
	cp->flags	= 0;
	cp->bus_priority= 0;
	cp->ivnum	= 0;
	cp->priority	= 0;
	cp->cmd		= 0;
	cp->physaddr	= 0;
	cp->physaddr2	= 0;

	/*
	 * Chain the new controller structure onto the controller list.
	 */
	link_controller(logp, cp);
	return(SUCCESS_RETURN);
}

/*
 * Add a new device structure to the linked list.
 */
int
add_device_entry(logp, dp)
	cm_log_t *logp;
	struct device_entry *dp;
{
	struct device *devp;

	if (dp->d_type != DEVICE_TYPE) {	/* paranoia */
		return;
	}

	devp = (struct device *)getstruct(logp, sizeof(struct device));
	if (devp == NULL) {
		return(ERROR_RETURN);
	}

	/*
	 * A lot of the fields are just set to 0.  This is essentially a
	 * no-op as the structure is bzero'ed anyways.  They are being done
	 * here to note that the field has not been overlooked.
	 */
	devp->ctlr_hd 	= 0;
	devp->dev_type 	= dp->d_type_string ? dp->d_type_string : "";
	devp->dev_name	= dp->d_name;
	devp->logunit	= dp->d_unit;  /* Logical # i.e. ra# */
	devp->unit	= dp->d_drive; /* Physical # i.e. unit # or drive # */
	if(strcmp(dp->d_wildcard, "*") == 0) {
		 devp->ctlr_name = "*";
		 devp->ctlr_num = CONFIG_WILDCARDED;
	}
	else {
	    switch ((long)dp->d_conn){
	      case 0:
	      case CONFIG_UNKNOWN:
		devp->ctlr_name = "";
		devp->ctlr_num  = 0;
		break;
	      /*
	       * The stanza line states that the bus is connected to a node
	       * that was not previously specified in the stanza file.  The
	       * missing node may have been statically configured or loaded
	       * in a separate module.  Hope that the user knows what they are
	       * doing in this case.
	       */
	      case CONFIG_MISSING:
	 	devp->ctlr_num = dp->d_conn_num;
	 	devp->ctlr_name = dp->d_conn_name;
		break;
	      default:
		devp->ctlr_num = dp->d_conn->d_unit;
		devp->ctlr_name = dp->d_conn->d_name;
	    };
	}
	devp->alive	= 0;

	/*
	 * Chain the new device structure onto the device list.
	 */
	link_device(logp, devp);
	return(SUCCESS_RETURN);
}

/*
 * Translate the datb into structures of type bus, controller and device.
 */
void
convert_dtab(logp)
	cm_log_t *logp;
{
	struct device_entry *dp;

	/*
	 * Initialize the pointers to the lists to be null.
	 */
	bus_list = (struct bus *) NULL;
	controller_list = (struct controller *) NULL;
	device_list = (struct device *) NULL;

	dp = dtab;
	while (dp != NULL) {
		switch (dp->d_type) {
			case BUS_TYPE:		add_bus_entry(logp, dp); 
						break;
			case CONTROLLER_TYPE:	add_controller_entry(logp, dp); 
						break;
			case DEVICE_TYPE:	add_device_entry(logp, dp); 
						break;
		};
		dp = dp->d_next;
	}
}

/*
 * Pass an individual list entry down to the kernel via setsysinfo.
 */
int
pass_entry(logp, ep)
	cm_log_t *logp;
	struct config_entry *ep;
{
	struct config_ssi ssi_struct;

	/* sanity check */
	switch (ep->e_type) {
		case BUS_TYPE:		break;
		case CONTROLLER_TYPE:	break;
		case DEVICE_TYPE:	break;
		default:
			cm_log(logp, LOG_ERR, cm_msg(CONFIG_EBADTP),ep->e_type);
			return(ERROR_RETURN);
	};
	ssi_struct.op = SSIN_LOAD_CONFIG;
	bcopy(ep, &ssi_struct.entry, sizeof(struct config_entry));
	if (setsysinfo(SSI_NVPAIRS,&ssi_struct,1,0,0) != 0) {
		cm_log(logp, LOG_ERR, cm_msg(CONFIG_SSI));
		return(ERROR_RETURN);
	}
	return(SUCCESS_RETURN);
}
/*
 * Take the bus, controller, and device lists; package them up and ship
 * down to the kernel via setsysinfo.
 */
int
pass_lists(logp, driver_name)
	cm_log_t *logp;
	char *driver_name;
{
	struct bus *bp;
	struct controller *cp;
	struct device *dp;
	struct config_entry entry;

	strcpy(entry.e_name, driver_name);

	/*
	 * Pass down the bus list.
	 */
	entry.e_type = BUS_TYPE;
	bp = bus_list;
	while (bp != NULL) {
		bzero(&entry.e_str, sizeof(struct bus));
		bcopy(bp, &entry.e_str, sizeof(struct bus));
		strcpy(entry.e_nm_1, bp->bus_name);
		strcpy(entry.e_nm_2, bp->connect_bus);
		strcpy(entry.e_nm_3, bp->pname);
		if (pass_entry(logp, &entry)) {
			return(ERROR_RETURN);
		}
		bp = bp->nxt_bus;
	}
	/*
	 * Pass down the controller list.
	 */
	entry.e_type = CONTROLLER_TYPE;
	cp = controller_list;
	while (cp != NULL) {
		bzero(&entry.e_str, sizeof(struct controller));
		bcopy(cp, &entry.e_str, sizeof(struct controller));
		strcpy(entry.e_nm_1, cp->ctlr_name);
		strcpy(entry.e_nm_2, cp->bus_name);
		strcpy(entry.e_nm_3, cp->pname);
		if (pass_entry(logp, &entry)) {
			return(ERROR_RETURN);
		}
		cp = cp->nxt_ctlr;
	}
	/*
	 * Pass down the device list.
	 */
	entry.e_type = DEVICE_TYPE;
	dp = device_list;
	while (dp != NULL) {
		bzero(&entry.e_str, sizeof(struct device));
		bcopy(dp, &entry.e_str, sizeof(struct device));
		strcpy(entry.e_nm_1, dp->dev_type);
		strcpy(entry.e_nm_2, dp->dev_name);
		strcpy(entry.e_nm_3, dp->ctlr_name);
		if (pass_entry(logp, &entry)) {
			return(ERROR_RETURN);
		}
		dp = dp->nxt_dev;
	}
	return(SUCCESS_RETURN);
}

/*
 * This routine is called to insure that all of the config related structures
 * in the kernel for this driver get removed.  Typically this routine is
 * called on error to mop up side effects.
 */
int
cm_config_delete(logp, name)
	cm_log_t *logp;
	char *name;
{
	struct config_ssi ssi_struct;

	ssi_struct.op = SSIN_DEL_CONFIG;
	strcpy(ssi_struct.entry.e_nm_1, name);
	if (setsysinfo(SSI_NVPAIRS,&ssi_struct,1,0,0) != 0) {
		cm_log(logp, LOG_ERR, cm_msg(CONFIG_SSI));
		return(ERROR_RETURN);
	}
	return(0);
}
/*
 * This routine is called after the named stanza entry has been found.
 * Perform "config" operations.
 */
int
cm_perform_config(logp, entry)
	cm_log_t *logp;
	ENT_t 	entry;
{
	char *config_string;
	char *driver_name;
	int i;
	char entry_name[100];
	int num_config_lines;


	init_globals();
	num_config_lines = 0;
	/*
	 * Search through the stanza entry for all the config lines.  This
	 * is done by taking a base entry name of "Module_Config" and appending
	 * on numbers to form the full name.  A range of numbers is examined
	 * so that the entries do not have to be strictly sequentially ordered.
	 */
	for (i = 0; i < MAX_CONFIG_LINES; i++) {
		sprintf(entry_name, "%s%d",MODULE_CONFIG,i);
		config_string = dbattr_string(entry, entry_name, "no_config");
		if (strcmp(config_string, "no_config")) {
			if (parse_config(logp,entry_name, config_string)) {
				cm_log(logp, LOG_ERR, cm_msg(CONFIG_EPARSE));
				free_lists();
				return(ERROR_RETURN);
			}
			num_config_lines++;
		}
	}
	if (num_config_lines > 0) {
		/*
	 	 * Now that the dtab has been created translate it into the
	 	 * bus, controller and device data structures.
	 	 */
		convert_dtab(logp);
		/*
	 	 * Pass the bus, controller and device lists down to the kernel.
	 	 * There they will be used by the kernel configuration code.
	 	 */
		driver_name = dbattr_string(entry, MODULE_CONFIG_NAME, 
							"bogus_driver_name");
		if (strcmp(driver_name, "bogus_driver_name") == 0) {
			cm_log(logp, LOG_ERR, cm_msg(CONFIG_EREQ),MODULE_CONFIG_NAME);
			free_lists();
			return(ERROR_RETURN);
		}
		if (pass_lists(logp, driver_name)) {
			free_lists();
			return(ERROR_RETURN);
		}
	}
	/*
	 * Delete all dynamically allocated memory.
 	 */
	free_lists();
	return(SUCCESS_RETURN);
}


/*
 * Search the current dtab to see if an entry of the specified name and type has
 * been configured already.  If a matching entry is found return a pointer to 
 * its config entry, otherwise return NULL.
 */
struct device_entry *
findentry(entryname, unit, type)
	char *entryname;
	int  unit;
	u_int type;
{
	struct device_entry *dp;

	dp = dtab;
	while (dp != NULL) {
		if ((dp->d_type == type) && 
		    (strcmp(entryname, dp->d_name) == 0) &&
		    (dp->d_unit == unit)) {
			return(dp);
		}
		dp = dp->d_next;
	}
	return(NULL);
}

/*
 * Parse any options on this config line.
 */
int
getoptions(logp, config_name, input_line, rest_ptr, dp)
	cm_log_t *logp;
	char *config_name;	
	char *input_line;	
	char **rest_ptr;
	struct device_entry *dp;
{
	char optname[MAX_NAME];
	struct options_dispatch *options_dispatch_ptr;

    /*
     * The remainder of the config line is for options.  Parse it until the
     * end of line is encountered.  This is necessary because a line can
     * contain more than one option.
     */
    while (getword(logp, input_line, optname, rest_ptr) == SUCCESS_RETURN) {
	for (options_dispatch_ptr = option_table; 
	     options_dispatch_ptr->name != NULL;
	     options_dispatch_ptr++) {
		if (strcmp(optname, options_dispatch_ptr->name) == 0) {
			break;
		}
	}
	if (options_dispatch_ptr->name != NULL) {
		/*
		 * The input lines will be translated into device_entry
		 * data structures which will be malloc'ed onto a linked
		 * list.  If the list is currently empty, then initialize
	 	 * it to point to the first entry.
	 	 */
		if (options_dispatch_ptr->option_routine  != NULL) {
			if ((*options_dispatch_ptr->option_routine)
				(logp, config_name, *rest_ptr, rest_ptr, dp)) {
				return(ERROR_RETURN);
			}
		}
	}
	else {
		cm_log(logp, LOG_ERR, cm_msg(CONFIG_EOPTION),
			optname, config_name);
    		return(ERROR_RETURN);
	}
	input_line = *rest_ptr; /* setup for next iteration */
    }
    return(SUCCESS_RETURN);
}

/*
 * A type specific routine to parse a config line.
 */
int
config_bus(logp, line_name, config_line, dp)
	cm_log_t *logp;
	char *line_name;
	char *config_line;
	struct device_entry *dp;
{
	return(config_module(logp, line_name, config_line, dp, BUS_TYPE));
}

/*
 * A type specific routine to parse a config line.
 *
 * Format of the config line for a controller is:
 * controller <name><number> at { * or busname } <options>
 *  	Valid options are "slot", 
 */
int
config_controller(logp, line_name, config_line, dp)
	cm_log_t *logp;
	char *line_name;
	char *config_line;
	struct device_entry *dp;
{
	return(config_module(logp, line_name, config_line, dp, CONTROLLER_TYPE));
}

/*
 * A type specific routine to parse a config line.
 */
int
config_device(logp, line_name, config_line, dp)
	cm_log_t *logp;
	char *line_name;
	char *config_line;
	struct device_entry *dp;
{
	return(config_module(logp, line_name, config_line, dp, DEVICE_TYPE));
}

/*
 * Split a name into name and number as in split tc0 into tc and 0.
 * The name must start with a non-digit and end with a number, otherwise
 * return NULL.
 */
int
break_name(name, name_part, number_part)
	char *name;
	char **name_part;
	int *number_part;
{
	static char name_buf[MAX_NAME];
	int i = 0;

	while (*name != '\0') {
		if (isdigit(*name)) {
			/* Must start with non-digit */
			if (i == 0) {
				return(NULL);
			}
			*number_part = atoi(name);
			break;
		}
		else {
			if (*name == '?') {	/* wildcarded number */
				*number_part = CONFIG_WILDCARDED;
				break;
			}
			name_buf[i] = *name;
			i++;
			name_buf[i] = '\0';
			name++;
		}
	}
	name_buf[++i] = '\0';
	*name_part = name_buf;
	return(ERROR_RETURN);
}

/*
 * Parses a config line.
 *
 * NOTE: Not all module configuration fields found in config are currently
 * supported here.  Shown below is what is supported.
 *
 * Supported Formats:
 *
 * bus <name><number> at <connection> <options>
 * controller <name><number> at <connection> <options>
 * device <device_type> <name><number> at <connection> <options>
 *
 * <connection> can be:
 *	nexus		- connected to system bus.
 *	*		- wildcarded
 *	<name><number>	- identifies the module to connect to.  Note: No
 *			  forward references are allowed.  So you can't say
 *			  connected to foo2 and on a later line declare foo2.
 *
 * Valid <options> are "slot", "vector", "drive", "unit".
 */
int
config_module(logp, line_name, config_line, dp, module_type)
	cm_log_t *logp;
	char *line_name;
	char *config_line;
	struct device_entry *dp;
	int module_type;
{
	char *rest_of_line;
	char **rest_ptr = &rest_of_line;
	char *name_part;
	int  number_part;
	char conn_name[MAX_NAME];
	char devicetype[MAX_NAME];
	char modulename[MAX_NAME];
	int  conn_type;
	int  i;
	int  badconn;

	rest_of_line = config_line;
	/*
	 * For devices, the next word will be "disk" or "tape".
	 * Make sure that the specified device type is in the list of 
	 * supported devices.
	 */
	if (module_type == DEVICE_TYPE) {
	    if (getword(logp, rest_of_line, dp->d_type_string, rest_ptr)) {
		cm_log(logp, LOG_ERR, cm_msg(CONFIG_EDEVTYPE), line_name);
		return(ERROR_RETURN);
	    }
 	    if (validate_device_type(dp->d_type_string)) {
		cm_log(logp, LOG_ERR, cm_msg(CONFIG_EDEVTYPE), line_name);
		return(ERROR_RETURN);
	    }
	}
	/*
	 * The next word following the module_type descriptor is the module name
	 */
	if (getword(logp, rest_of_line, modulename, rest_ptr)) {
		cm_log(logp, LOG_ERR, cm_msg(CONFIG_EMODNAME), line_name);
		return(ERROR_RETURN);
	}
	/*
	 * Separate the module name into the name and number parts.
	 */
	if (break_name(modulename, &name_part, &number_part) == NULL) {
		cm_log(logp, LOG_ERR, cm_msg(CONFIG_EBADMOD), 
				line_name, modulename);
		return(ERROR_RETURN);
	}
	/*
	 * The name field must be "something", it can't be "*".
	 */
	if (strcmp(name_part, "*") == 0) {
		cm_log(logp, LOG_ERR, cm_msg(CONFIG_EBADMOD), 
				line_name, modulename);
		return(ERROR_RETURN);
	}
	strcpy(dp->d_name, name_part);
	dp->d_unit = number_part;
	/*
	 * See if the module has already been configured.  
	 */
	if (findentry(dp->d_name, dp->d_unit, module_type) != NULL) {
		cm_log(logp, LOG_ERR, cm_msg(CONFIG_EALREADY), 
			line_name, dp->d_name);
		return(ERROR_RETURN);
	}
	dp->d_type = module_type;
	/*
	 * See what the module is connected to.  The field may be wildcarded
	 * in which case no check is done to see if the connecting node is
	 * present, otherwise make sure the connecting node has already been
	 * specified in its own config line.
	 */
	switch (module_type) {
		case BUS_TYPE:		conn_type = BUS_TYPE; break;
		case CONTROLLER_TYPE:	conn_type = BUS_TYPE; break;
		case DEVICE_TYPE:	conn_type = CONTROLLER_TYPE; break;
		default:
			cm_log(logp, LOG_ERR, cm_msg(CONFIG_ETYPE), 
				module_type, line_name);
			return(ERROR_RETURN);
	};
	/*
	 * The next word is a required field of "at".
	 */
	if (needword(logp, rest_of_line, "at", rest_ptr) == NULL) {
		cm_log(logp, LOG_ERR, cm_msg(CONFIG_EAT), line_name);
		return(ERROR_RETURN);
	}
	/*
	 * The next field specifies the connecting node.
	 * This field may be wildcarded with "*", if it is not, the field must
	 * represent an already configured module.  In that case check the 
	 * current configuration to make sure the bus has already been 
	 * specified.
	 */
	if (getword(logp, rest_of_line, conn_name, rest_ptr)) {
		cm_log(logp, LOG_ERR, cm_msg(CONFIG_ECONN), line_name);
		return(ERROR_RETURN);
	}
	/*
	 * Special case bus connections to "nexus".
	 * Don't bother looking at the nexus number.  Always wildcard it.
	 */
	if (strncmp("nexus",conn_name, strlen("nexus")) == 0) {
		/*
		 * Only system busses can connect to nexus.  At this time the
		 * code does not support loadable bootpath drivers.  For this
		 * reason you can't load a sysbus adapter driver.  As a result
		 * it is currently illegal to load anything to nexus.
		 */
		cm_log(logp, LOG_ERR, cm_msg(CONFIG_ENEXUS), line_name);
		return(ERROR_RETURN);
	}
	else if (strcmp(WILDCARD, conn_name) == 0) {	/* at "*" */
		dp->d_conn = (struct device_entry *)CONFIG_WILDCARDED;
		strcpy(dp->d_wildcard, WILDCARD); /* Mark as a WILDCARD entry */
	}
	else {	/* the connection type is <name><number> */
		if (break_name(conn_name, &name_part, &number_part) == NULL) {
			cm_log(logp, LOG_ERR, cm_msg(CONFIG_EBADCON),
				line_name, modulename);
			return(ERROR_RETURN);
		}
		dp->d_conn = findentry(name_part, number_part, conn_type);
		if (dp->d_conn == NULL) {
		    /*
		     * Look at other module types for invalid connections.
		     * In the above findentry call the "conn_type" field 
		     * specifies the type of the module we expect to find
		     * the connection to.  For example if this is a controller
		     * the findcall would be looking for the specified bus.
		     * Below we check the other types to see if an error has
		     * occurred in the stanza config lines.  This would catch
		     * errors like controllers connected to controllers, etc.
		     */
		    badconn = 0;
		    switch (dp->d_type) {
			case DEVICE_TYPE:
			    if ((findentry(name_part,number_part,BUS_TYPE)!=NULL) ||
			       (findentry(name_part,number_part,DEVICE_TYPE)!=NULL)){
				badconn = 1;
			    }
			    break;
			case BUS_TYPE:
			    if ((findentry(name_part,number_part,CONTROLLER_TYPE)!=NULL) ||
			       (findentry(name_part,number_part,DEVICE_TYPE)!=NULL)){
				badconn = 1;
			    }
			    break;
			case CONTROLLER_TYPE:
			    if ((findentry(name_part,number_part,CONTROLLER_TYPE)!=NULL) ||
			       (findentry(name_part,number_part,DEVICE_TYPE)!=NULL)){
				badconn = 1;
			    }
			    break;
		    };
		    if (badconn) {
			cm_log(logp, LOG_ERR, cm_msg(CONFIG_INVCON), line_name);
			return(ERROR_RETURN);
		    }
		    else {
			/*
			 * For the case of non-monolothic device drivers the
			 * stanza entry may not fully describe all modules.
		   	 * For example a "class" driver may not describe all of
			 * the underlying controller types.  For this reason
			 * the connecting node may not be represented.  When 
			 * this happens mark the connection as unknown.
			 */
			dp->d_conn = (struct  device_entry *)CONFIG_MISSING;
			strcpy(dp->d_conn_name, name_part);
			dp->d_conn_num = number_part;
		    }
		}
	}
	/*
	 * See if there are any options specified, such as "slot, vector".
	 */
	if (getoptions(logp, line_name, rest_of_line, rest_ptr, dp)) {
		return(ERROR_RETURN);
	}
	return(SUCCESS_RETURN);
}

/*
 * A routine to parse the option type "slot"
 */
int
getslot(logp, line_name, input_line, rest_ptr, dp)
	cm_log_t *logp;
	char *line_name;
	char *input_line;
	char **rest_ptr;
	struct device_entry *dp;
{
	char number[MAX_NAME];
	int  value;

	if (getword(logp, input_line, number, rest_ptr)) {
		cm_log(logp, LOG_ERR, cm_msg(CONFIG_ESLOT), line_name);
		return(ERROR_RETURN);
	}
	if (strcmp(number, "?") == 0) {	/* wildcarded slot number */
		value = CONFIG_WILDCARDED;
	}
	else {
		value = atoi(number);
		/*
		 * No range or validity check is performed on the number.
		 */
	}
	dp->d_slot = value;
	return(SUCCESS_RETURN);
}

/*
 * A routine to parse the option type "drive" and the option "unit".
 * The two options do the same thing.
 */
int
getdrive(logp, line_name, input_line, rest_ptr, dp)
	cm_log_t *logp;
	char *line_name;
	char *input_line;
	char **rest_ptr;
	struct device_entry *dp;
{
	char number[MAX_NAME];
	int  value;

	if (getword(logp, input_line, number, rest_ptr)) {
		cm_log(logp, LOG_ERR, cm_msg(CONFIG_EDRIVE), line_name);
		return(ERROR_RETURN);
	}
	if (strcmp(number, "?") == 0) {	/* wildcarded drive number */
		value = CONFIG_WILDCARDED;	
	}
	else {
		value = atoi(number);
		/*
		 * No range or validity check is performed on the number.
		 */
	}
	dp->d_drive = value;
	return(SUCCESS_RETURN);
}

/*
 * A routine to parse the option type "vector".
 * In the static config file the vector field is used to represent routine
 * names for ISR registration.  For loadable drivers ISR registration is done
 * via handler_add in the driver itself.  For this reason the vector field of
 * these config lines is ignored.
 */
int
getvector(logp, line_name, input_line, rest_ptr, dp)
	cm_log_t *logp;
	char *line_name;
	char *input_line;
	char **rest_ptr;
	struct device_entry *dp;
{
	char vecname[MAX_NAME];

	if (getword(logp, input_line, vecname, rest_ptr)) {
		cm_log(logp, LOG_ERR, cm_msg(CONFIG_EVECT), line_name);
		return(ERROR_RETURN);
	}
	cm_log(logp, LOG_WARNING, cm_msg(CONFIG_EVECIGN), line_name, vecname);
	return(SUCCESS_RETURN);
}
/*
 * this routine is used to verify that a device is of a supported type.
 * An array above declares the supported device types.
 * Return 0 - device type matches one in the supported array; therefore the
 *	      device type is supported.  This is a success status.
 * nonzero  - no match, unsupported type, error status.
 */
int
validate_device_type(type_string)
	char *type_string;
{
	char *list_member;
	int i = 0;

	list_member = device_types[i++];
	while (*list_member) {
		if (strcmp(list_member, type_string) == 0) {
			return(0);	/* match found, valid device type */
		}
		list_member = device_types[i++];
	} 
	return(1);	/* no match found, not a valid device type */
}
