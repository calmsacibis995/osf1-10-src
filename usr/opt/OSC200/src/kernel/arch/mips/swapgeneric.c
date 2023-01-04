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
static char	*sccsid = "@(#)$RCSfile: swapgeneric.c,v $ $Revision: 1.2.3.6 $ (DEC) $Date: 1992/06/03 11:18:37 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/* 
 * derived from swapgeneric.c	4.2	(ULTRIX)	10/9/90
 */
/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 */

/************************************************************************
 *
 *	Modification History: swapgeneric.c
 *
 *  4-Dec-1991 -- Fred Canter
 *	Fix compiler warning caused by #define SWAPTYPE 2.
 *
 * 16-Sep-1990 -- Szczypek
 *	Added TURBOchannel console support.
 *
 * 29-May-1990 -- Robin
 *	Moved gets() to a new file in machine/common so that it
 *	can be used by diskless kernels.
 *
 * 12-Dec-1989 -- burns
 *	fixed to handle booting from multiple controllers for SCSI
 *	on DECsystem 5000.
 *
 * 02-Oct-1989 -- burns
 *	fixed to handle unit numbers greater than 7 for systems other than
 *	DECstation/DECsystem 3100s.
 *
 * 12-Jun-1989 -- gg
 *	Removed variables dmmin, dmmax and dmtext.	
 *
 *************************************************************************/

#include <sys/param.h>
#include <hal/cpuconf.h>
#include <io/dec/tc/tc.h>

#define BOOTDEVLEN 80
char bootdevice[BOOTDEVLEN] = { 0 };	/* the name of the bootdevice */

/*
 * Generic configuration;  all in one
 */
dev_t	rootdev = NODEV;
dev_t	dumpdev = NODEV;
int	nswap;

#include <asc.h>
#include <sii.h>
#include <kzq.h>
#include <uq.h>
#include <hsc.h>
#include <dssc.h>

extern int	siidriver(), ascdriver(), kzqdriver,
		uqdriver(), hscdriver(), dsscdriver();

struct	genericconf {
	caddr_t	gc_driver;
	char	*gc_name;
	dev_t	gc_root;
} genericconf[] = {
#if NUQ > 0
	{ (caddr_t)&uqdriver, "ra",	makedev(23,0),	},
#endif
#if NHSC > 0
	{ (caddr_t)&hscdriver, "ra",	makedev(23,0),	},
#endif
#if NDSSC > 0
	{ (caddr_t)&dsscdriver,	"ra",	makedev(23,0),	},
#endif

#if NDKIPC > 0
	{ (caddr_t)&dkipcdriver,	"ip",	makedev(0, 0),	},
#endif /* NDKIPC > 0 */

#if NRD > 0
	{ (caddr_t)&rcdriver,		"rd",	makedev(4, 0),	},
#endif /* NRD > 0 */
#if NSII > 0
	{ (caddr_t)&siidriver,	"rz",	makedev(8,0),	 },
#endif
#if NASC > 0
	{ (caddr_t)&ascdriver,	"rz",	makedev(8,0),	 },
#endif
#if NKZQ > 0
	{ (caddr_t)&kzqdriver,	"rz",	makedev(8,0),	 },
#endif
	{ 0 },
};

long	dumplo;

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

int askme;				 /* set by getargs */
extern struct genericconf genericconf[]; /* set in conf.c */
extern int	cpu;			 /* needed for DS5800 */
extern struct 	tc_slot tc_slot[8];	/* contains TURBOchannel info */
extern int	rex_base;		/* base address of callback table */
extern char	**ub_argv;		/* input parameters */
extern int	ub_argc;		/* input parameter count */

#define	SWAPTYPE 2

/*
 * This routine has been upgraded to handle disk unit numbers > 7 for
 * systems other than DECstation/DECsystem 3100s. It realy needs a re-write.
 *
 * Now we can handle multiple scsi controllers. It still needs a re-write.
 */
setconf()
{
	register struct genericconf *gc;
	int i, j, major_offset, swaponroot; 
	extern dev_t swapdev;
	char *name_hold = "  ";
	char *cp;
	char bootstr[80];
	int ctlr, bbflag;
	extern int networkboot;

	struct boot_data boot_data;

	if ((!strncmp(bootdevice, "tftp", 4)) || 
	    (!strncmp(&bootdevice[2], "tftp", 4))) {
		networkboot = 1;
		return;
	}

	major_offset = swaponroot = 0;

	if(rex_base) {
		i=0;
		cp = ub_argv[1];
		if(strncmp(cp+1,"/rz",3)==0) {
			for(i=0;i<=8;i++) {
				if((strcmp(tc_slot[i].devname,"asc")==0) &&
				((*cp - '0')==tc_slot[i].slot)) {
					ctlr = tc_slot[i].unit;
				}
			}
		}
		i=0;
		for(i=0;i<2;i++)
			bootstr[i] = *(cp+2+i);  
		bootstr[i++]='(';
		bootstr[i++]= ctlr+'0';
		bootstr[i++]=',';
		bootstr[i++]= *(cp+4);
		bootstr[i++]=',';
		for(j=1;j<ub_argc;j++) {
			if(strcmp(ub_argv[j],"-bb")==0) {
				strcpy(&bootstr[i],ub_argv[j+1]);
				i=i+strlen(ub_argv[j+1]);
				break;
			}
		}
		if(j == ub_argc)
			bootstr[i++] = '0';
		bootstr[i++]=')';
		bootstr[i]='\0';
		cp = bootstr;	

	}

	if (rootdev != NODEV)
		goto doswap;

	if (askme || rootdev == NODEV) {
		boot_data.node = 0;
		boot_data.controller = 0;
		boot_data.unit = 0;
		boot_data.partition = 0;
		boot_data.name[3] = NULL;
		boot_data.name[4] = NULL;
		boot_data.name[5] = NULL;
		boot_data.name[6] = NULL;
#if SWAPTYPE == 2
		if(rex_base) {
			if(parser(cp, &boot_data)) {
				goto swap_on_boot;
			}		   
		}
		else {
			if(parser((char *)prom_getenv("boot"), &boot_data)) {
				goto swap_on_boot;
			}		   
		}
#endif

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
		if (strlen(boot_data.name) &&
		    boot_data.name[strlen(boot_data.name)-1] == '*') {
                        boot_data.name[strlen(boot_data.name)-1] = '\0';
                        swaponroot++;
                }
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
		 	 * Don't print ra twice !!
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
doswap:
	/* swap size and dumplo set during autoconfigure */
	if (swaponroot) {
		rootdev = dumpdev;
	}
}


#define	AT_CONTROLLER	0
#define	AT_UNIT		1
#define	AT_PARTITION	2

parser(str, boot_data)
char *str;
struct boot_data *boot_data;
{
	int ret = 0, i;
	char *frn, *bck, *cp, *index();
	int where_are_we, unit, mod;

	/*
	 * Check the sanity of the boot string
	 */
	if(str == NULL)
		return(0);
	if(index(str, '(') && index(str, ')') && (index(str, '(') < index(str,')')))
		ret = 1;
	/*
	 * Put the two character device name into the boot_data struct
	 */
	if(ret && strncmp(str, "rz(", 3) == 0) {
		bcopy("rz", &boot_data->name[0], 3);
		ret = 1;
	}
	if(ret && strncmp(str, "ra(", 3) == 0) {
		bcopy("ra", &boot_data->name[0], 3);
		ret = 1;
	}
	if(ret && strncmp(str, "du(", 3) == 0) {	/* for DS5800 */
		bcopy("ra", &boot_data->name[0], 3);
		ret = 1;
	}
	if(ret && strncmp(str, "rf(", 3) == 0) {
		/* RF drives on DSSI are MSCP devices and are therefore
		 * ra devices to ULTRIX.
		 */
		bcopy("ra", &boot_data->name[0], 3);
		ret = 1;
	}

	/*
	 * Position to the 'meat' of the boot string
	 */
	if(ret == 1) {
		char *subval;
		frn = index(str,'(');
		bck = index(str,')');

		if(frn + 1 == bck) {
			boot_data->name[2] = '0';
			return(ret);
		}
		subval = frn + 1;
		*bck = '\0';
			

		/*
		 * Convert alpha partition number to numeric
		 */
		for (i = 0; i < strlen(subval); i++) {
			if (subval[i] >= 'A' && subval[i] <= 'H')
				subval[i] = subval[i] - 'A' + '0';
			if (subval[i] >= 'a' && subval[i] <= 'h')
				subval[i] = subval[i] - 'a' + '0';
		}

		/*
		 * Parse the 'controller,unit,partition'. We begin at the first character
		 * after the open paren.
		 */
		where_are_we = AT_CONTROLLER;
		while (*subval != '\0') {
			/*
			 * Here we decide if we need to process the 'controller'
			 * section of the boot string. Basically, it's ignored
			 * for all systems but those with multiple scsi controllers.
			 * So if we see that we are not dealing with an 'rz' type disk,
			 * we just skip over the controller section to the unit.
			 */
			if ((where_are_we == AT_CONTROLLER) && (boot_data->name[1] != 'z')) {
				while((*subval != ',') && (*subval != '\0'))
					subval++;
				if (*subval == ',') {
					where_are_we++;
					subval++;
					continue;
				}
				if (*subval == '\0')
					return(ret);
			}
			if (*subval == ',') {
				where_are_we++;
				subval++;
				continue;
			}
			if (*subval <= '0' && *subval >= '9') {
				ret = 0;
				return(ret);
			}
			switch (where_are_we) {
			      case AT_CONTROLLER:
				boot_data->controller =
					((boot_data->controller * 10) + (*subval - '0'));
				break;
			      case AT_UNIT:
				boot_data->unit =
					((boot_data->unit * 10) + (*subval - '0'));	
				break;
			      case AT_PARTITION:
				boot_data->partition =
					((boot_data->partition * 10) + (*subval - '0'));
				break;
			      default:
				ret = 0;
				return(ret);
			}
			subval++;
		}
		/*
		 * Now finish building up the device name
		 */
		unit = (boot_data->controller * 8) + boot_data->unit;
		i = 2;
		if (mod = (unit / 100)) {
			boot_data->name[i++] = ('0' + mod);
			unit -= (mod * 100);
		}
		if (mod = (unit / 10)) {
			boot_data->name[i++] = ('0' + mod);
			unit -= (mod * 10);
		}
	        boot_data->name[i] = ('0' + unit);
	}
	return(ret);
}

