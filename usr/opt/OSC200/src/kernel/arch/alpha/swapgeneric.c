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
static char *rcsid = "@(#)$RCSfile: swapgeneric.c,v $ $Revision: 1.2.8.2 $ (DEC) $Date: 1993/06/24 22:29:44 $";
#endif

/*
 * Modification History: machine/alpha/swapgeneric.c
 *
 * 10-Sep-90 -- afd
 *	Created this file for Alpha support.
 */

#include <sys/param.h>
#include <hal/cpuconf.h>
#include <io/dec/tc/tc.h>

/* for console callbacks */
#include <machine/rpb.h>
#include <machine/entrypt.h>

/*
 * Generic configuration;  all in one
 */
dev_t	rootdev = NODEV;
dev_t	dumpdev = NODEV;
int	nswap;

#include <scsi.h>
#include <asc.h>
#include <sii.h>
#include <skz.h>
#include <siop.h>
#include <uq.h>
#include <hsc.h>
#include <kzq.h>
#include <aha.h>

extern int siidriver(), ascdriver(), scsidriver(), uqdriver(), siopdriver();
extern int kzqdriver(), skzdriver(), hscdriver(), ahadriver();

extern int bootdevice_parser();
#define BOOTDEVLEN 80
char bootdevice[BOOTDEVLEN];	/* the name of the bootdevice */

struct	genericconf {
	caddr_t	gc_driver;
	char	*gc_name;
	dev_t	gc_root;
} genericconf[] = {
#if NDKIPC > 0
	{ (caddr_t)dkipcdriver,	"ip",	makedev(0, 0),	},
#endif /* NDKIPC > 0 */

#if NRD > 0
	{ (caddr_t)rcdriver,	"rd",	makedev(4, 0),	},
#endif /* NRD > 0 */
#if NSCSI > 0
	{ (caddr_t)scsidriver,	"rz",	makedev(8,0),	 },
#endif
#if NSII > 0
	{ (caddr_t)siidriver,	"rz",	makedev(8,0),	 },
#endif
#if NASC > 0
	{ (caddr_t)ascdriver,	"rz",	makedev(8,0),	 },
#endif
#if NSKZ > 0
	{ (caddr_t)skzdriver,	"rz",	makedev(8,0),	 },
#endif
#if NADUSZ > 0
	{ (caddr_t)aduszdriver, "rz",	makedev(8,0),	 },
#endif
#if NSIOP > 0
	{ (caddr_t)siopdriver,	"rz",   makedev(8,0),    },
#endif
#if NHSC > 0
	{ (caddr_t)hscdriver,	"ra",	makedev(23,0),	 },
#endif
#if NUQ > 0
	{ (caddr_t)uqdriver,	"ra",	makedev(23,0),	 },
#endif
#if NKZQ > 0
	{ (caddr_t)kzqdriver,	"rz",	makedev(8,0),	 },
#endif
#if NAHA > 0
	{ (caddr_t)ahadriver,	"rz",	makedev(8,0),	 },
#endif
	{ 0 },
};

daddr_t	dumplo;

/*
 * the boot_data struct is used by the parser routine for mips based
 * systems to place the boot string information.
 */
struct boot_data {
	int	node;
	int	controller;
	int	unit;
	int	partition;
	char	name[128];
};

#define SWAP_TYPE 2			/* activate console callback */

long askme = 0;				/* set by getargs */

/*
 * This routine has been upgraded to handle disk unit numbers > 7 for
 * systems other than DECstation/DECsystem 3100s. It really needs a re-write.
 *
 * Now we can handle multiple scsi controllers. It still needs a re-write.
 */
setconf()
{
	register struct genericconf *gc;
	int i, major_offset; 
	char *name_hold = "  ";
	struct boot_data boot_data;

	major_offset = 0;

	if (rootdev != NODEV)
		return; /* goto doswap; */

	if (askme || rootdev == NODEV) {
		boot_data.node = 0;
		boot_data.controller = 0;
		boot_data.unit = 0;
		boot_data.partition = 0;
		boot_data.name[3] = NULL;
		boot_data.name[4] = NULL;
		boot_data.name[5] = NULL;
		boot_data.name[6] = NULL;
#if SWAP_TYPE == 2
/* bootdevice is initialized within pmap_bootstrap before *The Switch* */
		if (bootdevice_parser(bootdevice, &boot_data.name[0])) {
#ifdef DEBUG_PRINT_XLATE
printf("setconf: bootdevice_parser translated '%s' to '%s'\n",
       bootdevice, &boot_data.name[0]);
#endif /* DEBUG_PRINT_XLATE */
			goto swap_on_boot;
		}		   
#ifdef DEBUG_PRINT_XLATE
else
printf("setconf: bootdevice_parser failed to translate '%s'\n", bootdevice);
#endif /* DEBUG_PRINT_XLATE */
#endif /* SWAP_TYPE == 2 */

retry:
		boot_data.unit = 0;
		boot_data.partition = 0;
		printf("root device? ");
		gets(boot_data.name);
swap_on_boot:
		for (gc = genericconf; gc->gc_name; gc++)
			if (gc->gc_name[0] == boot_data.name[0] &&
			    gc->gc_name[1] == boot_data.name[1])
				goto gotit;
		goto bad;
gotit:
		boot_data.unit = 0;
		i = 2;
		if ((boot_data.name[i] >= '0') &&
		    (boot_data.name[i] <= '9')) {
			while (boot_data.name[i] != '\0') {
				boot_data.unit = ((boot_data.unit * 10) +
					(boot_data.name[i] - '0'));
				i++;
			}
			goto found;
		}
		printf("bad/missing unit number\n");
bad:
		printf("bad root specification, use one of:");
		for (gc = genericconf; gc->gc_name; gc++) {
			/*
		 	 * Don't print disk type twice !!
		 	 */
			if (strcmp(name_hold, gc->gc_name) != 0)
				printf("%s%%d ", gc->gc_name);
			name_hold = gc->gc_name;
		}
		printf("\n");
		goto retry;
	}
	boot_data.unit = 0;
found:
	/*
	 * We differentiate between SCSI and MSCP here because each
	 * has a different underlying minor number format (SCSI uses
	 * bus, target, lun notation while MSCP simply uses unit
	 * number).
	 */
	if (boot_data.name[1] == 'z') {
	    gc->gc_root = makedev(major(gc->gc_root),
	                  (MAKECAMMINOR(boot_data.unit, boot_data.partition)));
	} else {
	    gc->gc_root = makedev(major(gc->gc_root),
	                  (MAKEMINOR(boot_data.unit, boot_data.partition)));
	}
	rootdev = gc->gc_root;

/* doswap: */
	/* swap size and dumplo set during autoconfigure */
}

