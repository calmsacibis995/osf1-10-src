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
static char *rcsid = "@(#)$RCSfile: isaconf.c,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/06/24 22:40:58 $";
#endif

#include <cpus.h>
#include <machine/pmap.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/map.h>
#include <sys/vmmac.h>
#include <sys/buf.h>
#include <sys/conf.h>
#include <sys/config.h>

#include <io/dec/uba/ubareg.h>
#include <io/dec/uba/ubavar.h>
#include <kern/xpr.h>
#include <machine/cpu.h>
#include <machine/scb.h>
#include <hal/cpuconf.h>
#include <io/common/devdriver.h>
#include <vm/vm_kern.h>
#include <io/dec/mbox/mbox.h>

#include "isa.h"

extern struct device	device_list[];

/* Stolen from ibus/ibus.c. */

/*
 * Name:	isa_config_cont (slot, name, bus);
 *
 * Args:	slot	- The mbus slot number containing the "controller"
 *		name	- The name of the "controller" to match with in the
 *			  ubminit and ubdinit structures
 *		bus	- ptr to the bus structure
 *
 * Returns:	1 - if the "controller" was found
 *		0 - if the "controller" wasn't found
 */
int
isa_config_controller(u_long slot, char *name, struct bus *bus)
{
	register struct controller *ctlr;
	register struct device *device;
	register struct driver *drp;
	int savectlr, savebus;
	char *savectname, *savebusname;

	if( ((ctlr = get_ctlr(name, slot, bus->bus_name, bus->bus_num)) == 0) &&
	    ((ctlr = get_ctlr(name, -1, bus->bus_name, bus->bus_num)) == 0) &&
	    ((ctlr = get_ctlr(name, -1, bus->bus_name, -1)) == 0) &&
	    ((ctlr = get_ctlr(name, -1, "*", -99)) == 0) ) {
		return(0);
	}

	savebus = ctlr->bus_num;
	savebusname = ctlr->bus_name;
	ctlr->bus_name = bus->bus_name;
	ctlr->bus_num = bus->bus_num;

	drp = ctlr->driver;

	if((*drp->probe)(ctlr) == 0) {
	    ctlr->bus_num = savebus;
	    ctlr->bus_name = savebusname;
	    return(0);
        }
	ctlr->slot = slot;
	ctlr->alive = ALV_ALIVE;

	drp->ctlr_list[ctlr->ctlr_num] = ctlr;
	config_fillin(ctlr);
	printf("\n");
	if (drp->cattach)
		(*drp->cattach)(ctlr);
	conn_ctlr(bus, ctlr);
	
	for (device = device_list; device->dev_name; device++) {
		if (((device->ctlr_num != ctlr->ctlr_num) &&
		     (device->ctlr_num !=-1) && (device->ctlr_num != -99)) ||
		    ((strcmp(device->ctlr_name, ctlr->ctlr_name)) &&
		     (strcmp(device->ctlr_name, "*"))) ||
		    (device->alive) ) {
			continue;
		}
		savectlr = device->ctlr_num;
		savectname = device->ctlr_name;
		device->ctlr_num = ctlr->ctlr_num;
		device->ctlr_name = ctlr->ctlr_name;
		if ((drp->slave) && (*drp->slave)(device)) {
			device->alive = ALV_ALIVE;
			device->ctlr_num = ctlr->ctlr_num;
			conn_device(ctlr, device);
			drp->dev_list[device->logunit] = device;
			printf("%s%d at %s%d",
			       device->dev_name, device->logunit,
			       drp->ctlr_name, ctlr->ctlr_num);
			if(device->unit >= 0) 
				printf(" unit %d", device->unit);
			if (drp->dattach)
				(*drp->dattach)(device);
			printf("\n");
		} else {
			device->ctlr_num = savectlr;
			device->ctlr_name = savectname;
		}
	}
	return (1);
}



/*
 * Name:	eisa_config_controller (nxv, slot, name, bus);
 *
 * Args:	nxv	- The virtual address of the "controller"
 *
 *		slot	- The mbus slot number containing the "controller"
 *
 *		name	- The name of the "controller" to match with in the
 *			  ubminit and ubdinit structures
 *
 *		bus	- ptr to the bus structure
 *
 * Returns:	1 - if the "controller" was found
 *		0 - if the "controller" wasn't found
 */
int
eisa_config_controller(char *nxv, u_long slot, char *name, struct bus *bus)
{
  register struct controller *ctlr;
  register struct device *device;
  register struct driver *drp;
  int savectlr, savebus;
  char *savectname, *savebusname;

  if( ((ctlr = get_ctlr(name, slot, bus->bus_name, bus->bus_num)) == 0) &&
     ((ctlr = get_ctlr(name, -1, bus->bus_name, bus->bus_num)) == 0) &&
     ((ctlr = get_ctlr(name, -1, bus->bus_name, -1)) == 0) &&
     ((ctlr = get_ctlr(name, -1, "*", -99)) == 0) ) {
    return(0);
  }
  
  savebus = ctlr->bus_num;
  savebusname = ctlr->bus_name;
  ctlr->bus_name = bus->bus_name;
  ctlr->bus_num = bus->bus_num;
  
  drp = ctlr->driver;
  
  if((*drp->probe)(nxv, ctlr) == 0) {
    ctlr->bus_num = savebus;
    ctlr->bus_name = savebusname;
    return(0);
  }
  
  ctlr->slot = slot;
  ctlr->alive = ALV_ALIVE;
  ctlr->addr = (char *)nxv;
  
  drp->ctlr_list[ctlr->ctlr_num] = ctlr;
  config_fillin(ctlr);
  printf("\n");
  
  if (drp->cattach)
    (*drp->cattach)(ctlr);
  conn_ctlr(bus, ctlr);
  
  for (device = device_list; device->dev_name; device++) 
    {
      if (((device->ctlr_num != ctlr->ctlr_num) &&
	   (device->ctlr_num !=-1) && (device->ctlr_num != -99)) ||
	  ((strcmp(device->ctlr_name, ctlr->ctlr_name)) &&
	   (strcmp(device->ctlr_name, "*"))) ||
	  (device->alive) ) 
	{
	  continue;
	}

      savectlr = device->ctlr_num;
      savectname = device->ctlr_name;
      device->ctlr_num = ctlr->ctlr_num;
      device->ctlr_name = ctlr->ctlr_name;
      
      if ((drp->slave) && (*drp->slave)(device, nxv)) 
	{
	  device->alive = ALV_ALIVE;
	  device->ctlr_num = ctlr->ctlr_num;

	  conn_device(ctlr, device);
	  drp->dev_list[device->logunit] = device;
	  printf("%s%d at %s%d",
		 device->dev_name, device->logunit,
		 drp->ctlr_name, ctlr->ctlr_num);
	  if(device->unit >= 0) 
	    printf(" unit %d", device->unit);

	  if (drp->dattach)
	    (*drp->dattach)(device);
	  printf("\n");
	} 
      else 
	{
	  device->ctlr_num = savectlr;
	  device->ctlr_name = savectname;
	}
    }  
  
  return (1);
}
