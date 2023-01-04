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
static char     *sccsid = "@(#)$RCSfile: getconfig_utils.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/04/28 15:54:18 $";
#endif lint
/************************************************************************
 *									*
 *			Copyright (c) 1987 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any  other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/


/************************************************************************
 *
 * Name: getconfig_utils.c
 *
 * Modification History
 *
 * Jun 5, 1991 - jaa
 *	ported to OSF/1.  modified to use new bus/controller/device
 *	kernel structures.  in porting, we don't use /dev/kmem anymore.
 *	getsysinfo(3) was modified to return the needed information.
 *	NOTE: ifdef TODO's in this code will have to be done as the 
 *	pieces are added (i.e. floating csr space, workstation specifics)
 *
 * Jan 12, 1990 - Alan Frechette
 *	Initialize "alive_unit" field in init_adl_entry().
 *
 * Dec 12, 1989 - Alan Frechette
 *	Set the "alive_unit" field in the alive device structure.
 *
 * Oct 11, 1989 - Alan Frechette
 *	Fixed sizer to get the CSR for a UNIBUS controller from the
 *	UNIBUS controller structure instead of from the UNIBUS device
 *	structure.
 *
 * July 14, 1989 - Alan Frechette
 *	Fixed handling of unsupported devices not found in the
 *	configuration device table "config_device[]". Add check
 *	for (XMINODE) in "check_connection()". Null out "pname"
 *	and "cname" in "get_config_adpt()".
 *
 * July 6, 1989 - Alan Frechette
 *	Added call to "check_connection()" for a "ci" adapter
 *	attached to a "vaxbi" or "xmi".
 *
 * May 02, 1989 - Alan Frechette
 *	Changes to deal with new unique "cpu" handling for both
 *	vax and mips architectures.
 *
 * Feb 12, 1989 - Alan Frechette
 *	New sizer code which supports multiple architectures.
 *      This file is a complete redesign and rewrite of the 
 *	original V3.0 sizer code by Tungning Cherng.
 *
 ***********************************************************************/

#include "sizer.h"

#ifdef notdef
/* we may need this for floating */

/****************************************************************
* Name:		add_alive_device				*
*    								*
* Abstract:	Add an alive device to the alive device list	*
*		table "adltbl[]".				*
*								*
* Inputs:							*
* devtype	The device type (BUS, CONTROLLER, DEVICE).    	*
* busp      	The pointer to a bus structure. 		*
* ctlrp		The pointer to a controller structure.          *
* devp		The pointer to a device structure.		*
*								*
* Outputs:	None.						*
*								*
* Return Values: None.						*
*								*
* Side Effects:	If the alive device is not configured in the	*
*		configuration device table "devtbl[]" then a	*
*		message will be printed out to the user that	*
*		this device will be ignored. This following	*
*		routines are also called:			*
* 								*
* init_adl_entry()	Initialize an alive device table entry.	*
* match_device_name()	Search device name in device table.	*
* match_node_name()	Search connection name in node table.	*
* check_connection()	Check for a BI/XMI connection point.	*
****************************************************************/
add_alive_device(devtype, busp, ctlrp, devp)
	int devtype;
	struct bus *busp;
	struct controller *ctlrp;
	struct device *devp;
{
	int index, nodeindex;

	/* Initialize an allocate an alive device list entry */
	init_adl_entry(Adlindex);
	index = -1;
	
	switch(devtype) {
	      case BUS:	
		if((index = match_device_name(busp->bus_name)) == -1)
			sprintf(adltbl[Adlindex].unsupp_devname, "%s%d", 
				busp->bus_name, busp->bus_num);

		strcat(adltbl[Adlindex].conn_name, busp->connect_bus);
		adltbl[Adlindex].bus_type = busp->bus_type;
		adltbl[Adlindex].conn_number = busp->connect_num;
		adltbl[Adlindex].device_index = index;
		adltbl[Adlindex].device_unit = busp->bus_num;
		adltbl[Adlindex].alive_unit = 1;
		check_connection(busp->connect_bus, busp->connect_num,
				 busp->slot, Adlindex);
		break;

	      case CONTROLLER:
		if((index = match_device_name(ctlrp->ctlr_name)) == -1)
			sprintf(adltbl[Adlindex].unsupp_devname,"%s%d",
				ctlrp->ctlr_name, ctlrp->ctlr_num);
		adltbl[Adlindex].bus_type = busp->bus_type;
		strcat(adltbl[Adlindex].conn_name, busp->bus_name);
		adltbl[Adlindex].conn_number = ctlrp->bus_num;
		adltbl[Adlindex].device_index = index;
		adltbl[Adlindex].device_unit = ctlrp->ctlr_num;
		adltbl[Adlindex].alive_unit = ctlrp->alive;
		if((nodeindex = match_node_name(busp->bus_name)) >= 0){
			adltbl[Adlindex].rctlr = ctlrp->rctlr;
			adltbl[Adlindex].node_index = nodeindex;
			adltbl[Adlindex].node_number = ctlrp->slot;
		}
		
		/* Get the CSR address for this controller */
		if(busp->bus_type == BUS_UNIBUS)
			adltbl[Adlindex].csr = (u_short) ctlrp->physaddr;

		check_connection(busp->bus_name, ctlrp->bus_num, 
				 ctlrp->slot, Adlindex);
		break;
		
	      case DEVICE:	/* Store information about a DEVICE */
		if((index = match_device_name(devp->dev_name)) == -1)
			sprintf(adltbl[Adlindex].unsupp_devname,
				"%s%d", devp->dev_name, devp->unit);
		if(busp != NULL) {
			adltbl[Adlindex].bus_type = busp->bus_type;
			strcat(adltbl[Adlindex].conn_name, 
			       busp->connect_bus);
			adltbl[Adlindex].conn_number = busp->connect_num;
		}
		if(ctlrp != NULL) {
			strcat(adltbl[Adlindex].conn_name,
			       ctlrp->ctlr_name);
			adltbl[Adlindex].conn_number = ctlrp->ctlr_num;
		}
		adltbl[Adlindex].device_index = index;
		adltbl[Adlindex].device_unit = devp->logunit;
		adltbl[Adlindex].device_drive = devp->unit;
		adltbl[Adlindex].alive_unit = devp->alive;
		if((nodeindex = match_node_name(adltbl[Adlindex].conn_name)) >= 0)
			adltbl[Adlindex].node_index = nodeindex;
		if(strcmp(adltbl[Adlindex].conn_name, "uba") == 0) {
			/* Get the CSR address for this device */
			adltbl[Adlindex].csr = 
				(u_short)devp->ctlr_hd->physaddr;
		}
		if(busp != NULL)
			check_connection(busp->connect_bus,
					 busp->connect_num,  
					 busp->slot, Adlindex);
		break;
		
	      default:
		Adlindex--;
		break;
	}
	Adlindex++;
}

/****************************************************************
*  Name:	check_connection				*
*    								*
*  Abstract:	Search for the final connection point of a 	*
*		particular device. If the final connection 	*
*		point is itself at a BI or XMI connection  	*
*		then we set the node number for this BI or	*
*		XMI connection. Also if the final connection 	*
*		point is not at a UNIBUS ADAPTER then we set 	*
*		the CSR equal to -1.				*
*								*
*    	(EX)	adapter	     vaxbi2  	at  nexus?		*
*		controller   kdb0  	at  vaxbi2	node?	*
*		controller   uq4  	at  kdb0		*
*								*
*    		In this example the device is "uq4" and its 	*
*		final connection point is at "vaxbi2", so we 	*
*		need to patch the node number for this BI 	*
*		connection.					*
*								*
*  Inputs:							*
*  pname	The connection point of a given device.		*
*  pnumber	The connection number of a given device.	*
*  nexus	The nexus number to set for the BI/XMI node.	*
*  index	The index of the alive device list entry.	*
*								*
*  Outputs:	None.						*
*								*
*  Return Values: None.						*
****************************************************************/
check_connection(pname, pnumber, nexus, index)
char *pname;
int pnumber;
int nexus;
int index;
{

    int i, nodeindex;

    /*
     * Search through the alive device linked list looking
     * for the connection point of the given DEVICE passed
     * in as "pname" and "pnumber". If the connection point 
     * is itself at a BI or XMI connection then we need 
     * to set the node number for this BI or XMI connection.
     */
    for(i = 0; i < Adlindex; i++) {
	    /* Check if this is the DEVICE'S connection point */
	    if((strcmp(devtbl[adltbl[i].device_index].devname, pname) == 0) && 
	       (adltbl[i].device_unit == pnumber)) {
		    /*
		     * If the DEVICE's final connection point is itself
		     * at a BI or XMI connection then patch the node 
		     * number. 
		     */
		    nodeindex = match_node_name(adltbl[i].conn_name);
		    if(nodeindex != -1 && 
		       (nodetbl[nodeindex].nodetype == BINODE) ||
		       (nodetbl[nodeindex].nodetype == XMINODE)) {
			    adltbl[i].node_index = nodeindex;
			    adltbl[i].node_number = nexus;
		    } else if(nodeindex == -1 && 
			      strcmp(adltbl[i].conn_name, "nexus") == 0) {
			    nodeindex = match_node_name(pname);
			    if(nodeindex != -1 && 
			       (nodetbl[nodeindex].nodetype == BINODE) ||
			       (nodetbl[nodeindex].nodetype == XMINODE)) {
				    adltbl[index].node_index = nodeindex;
				    adltbl[index].node_number = nexus;
			    }
		    }
		    /*
		     * If the DEVICE's final connection point is not
		     * attached to the UNIBUS ADAPTER then make sure 
		     * the CSR is set equal to -1.
		     */
		    if(strcmp(adltbl[i].conn_name, "uba") != 0 &&
		       strcmp(adltbl[index].conn_name, "uba") != 0)
			    adltbl[index].csr = -1;
		    break;
	    }
    }
}
#endif /* notdef */

/****************************************************************
*  Name:	init_adl_entry					*
*    								*
*  Abstract:	Initialize an alive device table entry to its 	*
*		default settings.				*
*								*
*  Inputs:							*
*  i		The current index in the alive device table.	*
*								*
*  Outputs:	None.						*
*								*
*  Return Values: None.						*
****************************************************************/
init_adl_entry(i)
int i;
{
    	adltbl[i].bus_type = -1;
    	adltbl[i].device_index = -1;
    	adltbl[i].device_unit = -1;
    	adltbl[i].device_drive = -1;
    	adltbl[i].conn_name[0] = NULL;
    	adltbl[i].conn_number = -1;
    	adltbl[i].node_number = -1;
    	adltbl[i].node_index = -1;
    	adltbl[i].rctlr = -1;
    	adltbl[i].csr = -1;
    	adltbl[i].alive_unit = 1;
}


/****************************************************************
*  Name:	match_device_name				*
*    								*
*  Abstract:	Search for the given device name in the 	*
*		configuration device table "devtbl[]".		*
*								*
*  Inputs:							*
*  devname	The device name to search for.			*
*								*
*  Outputs:	None.						*
*								*
*  Return Values: 						*
*  i		The index into the device table "devtbl[]".	*
****************************************************************/
match_device_name(devname)
	char *devname;
{

    int i;

    /* Search for device name in "devtbl" table */
    for(i = 0; i < Devtblsize; i++) {
	if(strcmp(devname, devtbl[i].devname) == 0)
	    return(i);
    }
    return(-1);
}

/****************************************************************
*  Name:	match_node_name					*
*    								*
*  Abstract:	Search for the given connection name in the 	*
*		configuration node table "nodetbl[]".		*
*								*
*  Inputs:							*
*  conn_name	The connection name to search for.		*
*								*
*  Outputs:	None.						*
*								*
*  Return Values: 						*
*  i		The index into the node table "nodetbl[]".	*
****************************************************************/
match_node_name(conn_name)
char *conn_name;
{

    int i;

    /* Search for connection name in "nodetbl" table */
    for(i=0; i<Nodetblsize; i++) {
	if(strcmp(conn_name, nodetbl[i].conn_name) == 0)
	    return(i);
    }
    return(-1);
}

