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
static char *rcsid = "@(#)$RCSfile: cam_config.c,v $ $Revision: 1.1.14.3 $ (DEC) $Date: 1993/08/13 20:42:52 $";
#endif

/************************************************************************
 *
 * File:	cam_config.c
 * Date:	March 8, 1991
 * Author:	Robin T. Miller
 *
 * Description:
 *	CAM peripheral driver configuration information.
 *
 * Modification History:
 *
 *	05/24/91	maria
 *			Added an entry in the CAM peripheral driver
 *			configuration table for the CAM disk driver.
 *
 *	06/07/91	maria
 *			Changed uba_driver structure to reflect the
 *			use of rz/asc for booting.
 *			Changed ifdef checks of NCAM to be NASC.
 *	06/20/91	dallas
 *			Added ctape to this file once again.
 *	06/28/91        rps
 *                      Added new configuration code.  Dynamic allocation
 *                      and addition of sim's.  
 *	07/03/91	jag
 *			changed the routines cdrv_*() to ccfg_*().
 *	07/31/91	dallas
 *			Added tz routines for the wrapper
 *	12/04/91	jag
 *			Corrected the comments for VENDORS to add their drivers.
 *
 */

/*
 * This version of cam_config.c is intended for OSF use.
 */
#include <io/common/iotypes.h>
#include <sys/types.h>			/* system level types */

#include <sys/param.h>			/* system level parameter defs */
#include <sys/buf.h>
#include <mach/vm_param.h>
#include <io/cam/dec_cam.h>
#include <io/cam/cam.h>
#include <hal/cpuconf.h>
#include <io/common/devdriver.h>
#include <io/cam/cam_config.h>
#include <io/cam/sim_config.h>
#include <io/cam/cam_debug.h>

#include "asc.h"
#include "sii.h"
#include "tza.h"

#ifdef __alpha
#include "skz.h"
#include "siop.h"
#include "aha.h"
#define NKZQ	0
#else
#include "kzq.h"
#define NSKZ 0
#define NSIOP	0
#define NAHA	0
#endif

extern int cpu;

#if NSII > 0 || NASC > 0 || NSIOP > 0 || NSKZ > 0 || NKZQ > 0 || NAHA > 0 || NTZA > 0

int cam_probe();

/* JAG: these should be together */
caddr_t camstd[] = { 0 };
extern struct controller *camminfo[];
extern struct device *camdinfo[];

extern int ccfg_slave();
extern int ccfg_attach(); 

extern CAM_HBA_LIST_ENTRY cam_hba_list[];
extern CAM_DME_LIST_ENTRY cam_dme_list[];
extern int cam_hba_entries;
extern int cam_hba_limit;
extern int cam_dme_entries;
extern int cam_dme_limit;

/*
 * Setup the driver structures for auto-configuration code.
 */

struct driver camdriver = {
	cam_probe,		/* CAM SIM ASC HBA probe function.	*/
	ccfg_slave,		/* CAM cdrv slave function.		*/
	(int (*)()) 0,
	ccfg_attach,		/* CAM cdrv attach function.		*/
	(int (*)()) 0,		/* Driver start transfer function.	*/
	camstd,			/* Pointer to device CSR addresses.	*/
	"rz",			/* Name of the device.			*/
	camdinfo,		/* Pointers to device structures.	*/
	"cam",			/* Name of the controller.		*/
	camminfo,		/* Pointers to controller structures.	*/
	0,			/* Want exclusive use of bdp's flag.	*/
	0,			/* Size of first csr area .		*/
	0,			/* Address space of first csr area.	*/
	0,			/* Size of second csr area.		*/
	0 			/* Address space of second csr area.	*/
};

#if NASC > 0
/*
 * Controller driver structure for ASC Host Bus Adapter (HBA).
 */
extern int sim94_probe();
extern int sim94_attach(), sim94_unload();
extern int ram94_dme_attach(), ram94_dme_unload();
extern int dma94_dme_attach(), dma94_dme_unload();

struct driver ascdriver = {
	sim94_probe,		/* CAM SIM ASC HBA probe function.	*/
	ccfg_slave,		/* CAM cdrv slave function.		*/
	0,
	ccfg_attach,		/* CAM cdrv attach function.		*/
	(int (*)()) 0,		/* Driver start transfer function.	*/
	camstd,			/* Pointer to device CSR addresses.	*/
	"rz",			/* Name of the device.			*/
	camdinfo,		/* Pointers to device structures.	*/
	"asc",			/* Name of the controller.		*/
	camminfo,		/* Pointers to controller structures.	*/
	0,			/* Want exclusive use of bdp's flag.	*/
	0,			/* Size of first csr area .		*/
	0,			/* Address space of first csr area.	*/
	0,			/* Size of second csr area.		*/
	0 			/* Address space of second csr area.	*/
};
#endif /* NASC > 0 */

#if NSII > 0
/*
 * Controller driver structure for SII Host Bus Adapter (HBA).
 */
extern simsii_probe();
extern int simsii_attach(), simsii_unload();
extern int ds3100_dme_attach(), ds3100_dme_unload();

struct driver siidriver = {
	simsii_probe,		/* CAM SIM SII HBA probe function.	*/
	ccfg_slave,		/* CAM cdrv slave function.		*/
	0,
	ccfg_attach,		/* CAM cdrv attach function.		*/
	(int (*)()) 0,		/* Driver start transfer function.	*/
	camstd,			/* Pointer to device CSR addresses.	*/
	"sii",		/* Name of the device.			*/
	camdinfo,		/* Pointers to device structures.	*/
	"sii",		/* Name of the controller.		*/
	camminfo,		/* Pointers to controller structures.	*/
	0,			/* Want exclusive use of bdp's flag.	*/
	0,			/* Size of first csr area .		*/
	0,			/* Address space of first csr area.	*/
	0,			/* Size of second csr area.		*/
	0 			/* Address space of second csr area.	*/
};

#endif /* NSII > 0 */

#if NSKZ > 0
/*
** Controller driver structure for the XZA/SCSI adapter.
*/
extern skz_probe();

struct driver skzdriver = {
	skz_probe,		/* A stub for XZA */
	ccfg_slave,		/* CAM cdrv slave function.		*/
	0,
	ccfg_attach,		/* CAM cdrv attach function.		*/
	(int (*)()) 0,		/* Driver start transfer function.	*/
	camstd,			/* Pointer to device CSR addresses.	*/
	"rz",			/* Name of the device.			*/
	camdinfo,		/* Pointers to device structures.	*/
	"skz",			/* Name of the controller.		*/
	camminfo,		/* Pointers to controller structures.	*/
	0,			/* Want exclusive use of bdp's flag.	*/
	0,			/* Size of first csr area .		*/
	0,			/* Address space of first csr area.	*/
	0,			/* Size of second csr area.		*/
	0 			/* Address space of second csr area.	*/
};

#endif /* NSKZ > 0 */


#if NSIOP > 0

/* Cobra SCSI driver */
extern siop_probe();
extern int siop_kn430_attach(), siop_unload();

struct driver siopdriver = {
    siop_probe,       	    /* CAM SIM SIOP HBA probe function.      */
    ccfg_slave,             /* CAM cdrv slave function.             */
    0,
    ccfg_attach,            /* CAM cdrv attach function.            */
    (int (*)()) 0,          /* Driver start transfer function.      */
    camstd,                 /* Pointer to device CSR addresses.     */
    "siop",                 /* Name of the device.                  */
    camdinfo,               /* Pointers to device structures.       */
    "siop",                 /* Name of the controller.              */
    camminfo,               /* Pointers to controller structures.   */
    0,                      /* Want exclusive use of bdp's flag.    */
    0,                      /* Size of first csr area .             */
    0,                      /* Address space of first csr area.     */
    0,                      /* Size of second csr area.             */
    0                       /* Address space of second csr area.    */
};

#endif /* NSIOP > 0 */

#if NTZA > 0
/*
 * Controller driver structure for TZA Host Bus Adapter (HBA).
 */
extern int kztsa_probe();
extern int kztsa_attach(), kztsa_unload();

struct driver tzadriver = {
        kztsa_probe,            /* CAM SIM ASC HBA probe function.      */
        ccfg_slave,             /* CAM cdrv slave function.             */
        0,
        ccfg_attach,            /* CAM cdrv attach function.            */
        (int (*)()) 0,          /* Driver start transfer function.      */
        camstd,                 /* Pointer to device CSR addresses.     */
        "tza",                  /* Name of the device.                  */
        camdinfo,               /* Pointers to device structures.       */
        "tza",                  /* Name of the controller.              */
        camminfo,               /* Pointers to controller structures.   */
        0,                      /* Want exclusive use of bdp's flag.    */
        0,                      /* Size of first csr area .             */
        0,                      /* Address space of first csr area.     */
        0,                      /* Size of second csr area.             */
        0                       /* Address space of second csr area.    */
};
#endif /* NTZA > 0 */

#if NAHA > 0

/* Jensen SCSI driver */
extern aha_probe();
extern int aha_attach(), aha_unload();

struct driver ahadriver = {
    aha_probe,       	    /* CAM SIM AHA HBA probe function.      */
    ccfg_slave,             /* CAM cdrv slave function.             */
    0,
    ccfg_attach,            /* CAM cdrv attach function.            */
    (int (*)()) 0,          /* Driver start transfer function.      */
    camstd,                 /* Pointer to device CSR addresses.     */
    "aha",                  /* Name of the device.                  */
    camdinfo,               /* Pointers to device structures.       */
    "aha",                  /* Name of the controller.              */
    camminfo,               /* Pointers to controller structures.   */
    0,                      /* Want exclusive use of bdp's flag.    */
    0,                      /* Size of first csr area .             */
    0,                      /* Address space of first csr area.     */
    0,                      /* Size of second csr area.             */
    0                       /* Address space of second csr area.    */
};

#endif /* NAHA > 0 */


#if NKZQ > 0
/*
 * Controller driver structure for KZQ Host Bus Adapter (HBA).
 */
extern int simkzq_probe(), simkzq_slave();
extern int simkzq_attach(), simkzq_unload();
extern int ds5500_dme_attach(), ds5500_dme_unload();

struct driver kzqdriver = {
	simkzq_probe,		/* CAM SIM KZQ HBA probe function.	*/
	simkzq_slave,		/* CAM cdrv slave function.		*/
	0,
	ccfg_attach,		/* CAM cdrv attach function.		*/
	(int (*)()) 0,		/* Driver start transfer function.	*/
	camstd,			/* Pointer to device CSR addresses.	*/
	"kzq",			/* Name of the device.			*/
	camdinfo,		/* Pointers to device structures.	*/
	"kzq",			/* Name of the controller.		*/
	camminfo,		/* Pointers to controller structures.	*/
	0,			/* Want exclusive use of bdp's flag.	*/
	0,			/* Size of first CSR address space.	*/
	0,			/* Address space of first CSR area.	*/
	0,			/* Size of second CSR address space.	*/
	0			/* Address space of second CSR area.	*/
};
#endif /* NKZQ > 0 */

extern int cdisk_slave(), cdisk_attach();	/* Disk Driver */
extern int ctape_slave(), ctape_attach();	/* Tape Driver */

/* VENDOR: Add the extern declaratios for your hardware following this
    comment line! */

/*
 * CAM Peripheral Driver Configuration Table.
 */

CAM_PERIPHERAL_DRIVER cam_peripheral_drivers[] = {

	{ "rz", cdisk_slave, cdisk_attach },
	{ "tz", ctape_slave, ctape_attach }

/* VENDOR: Add your hardware entries following this comment line! */

};
int cam_pdrv_entries =
	sizeof(cam_peripheral_drivers) / sizeof(cam_peripheral_drivers[0]);

void
init_cam_components( void )
{
}

cam_probe(csr, prb)
caddr_t csr;
struct controller *prb;
{
    int retval = 1;
    char *hbaname;
    char *dmename;
    char modname[32];             /* this shouldn't be hard coded - RPS */

    switch( cpu )
        {
	case DS_3100:             /* sii base */
	    hbaname = "sii";
	    dmename = "siiram";
	    break;

        case DS_5000:             /* asc base */
	    if ( prb->ctlr_num == 0 )          /* base scsi? */
	      {
              hbaname = "asc";
	      dmename = "ram94";
	      }
            else                               /* TC scsi */
	      {
		if( tc_addr_to_name( csr, modname ) == -1)
		  printf("cam_probe: tc_addr_to_name failed ");
		hbaname = modname;
		dmename = modname;
	      }
	    break;

        case DS_5000_100:         /* asc base */
	    if ( prb->ctlr_num == 0 )               /* base scsi? */
	      {
		hbaname = "asc";
		dmename = "dma94";
	      }
            else                               /* TC scsi */
	      {
		if( tc_addr_to_name( csr, modname ) == -1)
		  printf("cam_probe: tc_addr_to_name failed ");
		hbaname = modname;
		dmename = modname;
	      }
	    break;

        case DS_5100:             /* sii base and extension */
	    hbaname = "sii";
	    dmename = "DS5100";
	    break;


#ifdef __alpha
	case DEC_4000:
	    hbaname = "siopco";
	    dmename = "null";
	    break;

        case DEC_2000_300:
            hbaname = "aha";
            dmename = "null";
            break;
#endif __alpha

	case DS_5400:             /* ? */

	case DS_5500:             /* asc or kzq */
	    if(strcmp(prb->ctlr_name, "asc") == NULL){
		hbaname = "asc";
		dmename = "ram94";
	    }
	    else if( strcmp(prb->ctlr_name, "kzq") == NULL){
		hbaname = "kzq";
		dmename = "kzqram";
	    }
	    else {
		hbaname = modname;
		dmename = modname;
	    }
	    break;
	
	case DS_5800:             /* ? */

        default:
	    if ( prb->ctlr_num == 0 )               /* base scsi? */
	      {
		hbaname = "asc";
		dmename = "dma94";
	      }
            else                               /* TC scsi */
	      {
		if( tc_addr_to_name( csr, modname ) == -1)
		  printf("cam_probe: tc_addr_to_name failed ");
		hbaname = modname;
		dmename = modname;
	      }
	    break;
	}

    if ( name_lookup_hba_probe( hbaname, csr, prb ) != CAM_REQ_CMP )
        {
	/* 
	 * If the attach fails don't do any further initialization.
	 */
	PRINTD( prb->ctlr_num, NOBTL, NOBTL, CAMD_INOUT,
	   ("cam_probe: hba probe lookup ('%s') failed.\n", hbaname ) );
	retval = CAM_FAILURE;
        }

    return retval;
    }

#endif


