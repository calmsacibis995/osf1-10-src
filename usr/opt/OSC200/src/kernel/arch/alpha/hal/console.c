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
static char *rcsid = "@(#)$RCSfile: console.c,v $ $Revision: 1.1.12.3 $ (DEC) $Date: 1993/07/30 18:24:33 $";
#endif

#include <sys/types.h>
#include <hal/cpuconf.h>

extern int cpu;			/* System type, value defined in cpuconf.h */
/*****************************************************************************
 *
 *	netboot support funcs for alpha.  In mips, these are in hal/console.c
 *
 *****************************************************************************/

/*
 * Generalized interface for netbootchk to use to
 * get the boot device in the boot string
 */

#define BOOTDEVLEN 80
extern char bootdevice[BOOTDEVLEN];	/* BOOTED_DEV console env string */
char *
getbootdev()
{
	char *bootdev;
	extern char **ub_argv;
	extern int ub_argc;
	int i;

	/* The boot info is already in the 'bootdevice' array */
	return(bootdevice);
}


/*
 * Generalized interface for netbootchk to use to
 * get the type of device network boot is being done
 * over ( ln, ne, fza, etc.).
 */

getboottype(boottype, bootdev)
	char *boottype;
	char *bootdev;
{
	int slot, unit, numblanks, mapfactor; 
	int unitlen, namelen;
	char buffer[BOOTDEVLEN];
	char *cp, *cpend, *ctname;
	extern char *atob(), *tc_slot_to_name(), *tc_slot_to_ctlr_name();
	extern int tc_get_unit_num();
	extern char *eisa_slot_to_name(), *eisa_slot_to_ctlr_name();
	extern int eisa_get_unit_num();

	/* bootdevice string for a network boot is of the generic form:
	 *	protocol 0 tc_slot 0 xx-xx-xx-xx-xx-xx 0 0 0
	 * Extract the tc_slot field from the string.
	 */
#define tc_slot_field 2

	strcpy(buffer, bootdev);
	cp = buffer;
	numblanks = 0;
	
	/* Skip to the beginning of the tc_slot field */
	while (*cp && numblanks != tc_slot_field) {
		while (*cp && *cp != ' ') cp++;	/* skip to next blank or end */
		numblanks++; cp++;
	}
	if (!*cp) {
		printf ("getboottype: bad device string1: %s\n", bootdev);
                boottype[0] = '\0';
		return;
	}

	/* Put a string terminator at the end of the tc_slot field */
	cpend = cp;
	while (*cpend && *cpend != ' ') cpend++;	
	if (!*cpend) {
		printf ("getboottype: bad device string2: %s\n", bootdev);
                boottype[0] = '\0';
		return;
	}
	*cpend++ = '\0';
 	(void)atob(cp, &slot);

	/* GMH: There is currently no algorithm for mapping the contents of the
	 * booted device string to the controller name and unit id.
	 * The hardcoded assumptions made in the following switch statement
	 * should be generalized once the algorithm is defined.
	 */
	switch (cpu) {

		/* For the birds: given a slot number, get the corresponding
		 * controller name (NOT the name of the option card from its
		 * rom) and controller number.  This is to build the string
		 * that correctly identifies the interface from which we were
		 * booted, such as "ln0", "ln1", "fza0", etc.
		 */
		case DEC_3000_300:
					 /* integral LANCE.  The tc_slot_to_* functions
					  * won't work for this slot since more than one
					  * device lives here
					  */
			if (slot == 5) { /* junk IO is in slot 5 on Pelican */
			    bcopy("ln", boottype, 2);
			    mapfactor = slot;
			    namelen = 2;
			} else {	/* real option slot */
			    cp = tc_slot_to_name(slot);

			    /* yuck: why -1 for failure?? */
			    if ((int)cp == -1) {
				printf("getboottype: can't figure out what is in slot %d\n", slot);
				boottype[0] = '\0';
				namelen = 0;
				break;
			    }

			    ctname = tc_slot_to_ctlr_name(slot);
			    if ((int)ctname == -1) {
				printf("getboottype: can't get controller name for interface in slot %d\n", slot);
				boottype[0] = '\0';
				namelen = 0;
				break;
			    }
			    namelen = strlen(ctname);
			    bcopy(ctname, boottype, namelen);
			    if ((unit = tc_get_unit_num(slot)) == -1) {
				printf("getboottype: can't figure out the unit number for the %s in slot %d\n", cp, slot);
				boottype[0] = '\0';
				namelen = 0;
				break;
			    }
			    mapfactor = slot - unit; /* kludge to make "unit" calculation
						      * below get the Right Answer
						      */
			}
                        break;
		case DEC_3000_500:
			if (slot == 7) { /* junk IO is in slot 7 on sandpiper and flamingo */
			    bcopy("ln", boottype, 2);
			    mapfactor = slot;
			    namelen = 2;
			} else {	/* real option slot */
			    cp = tc_slot_to_name(slot);

			    /* yuck: why -1 for failure?? */
			    if ((int)cp == -1) {
				printf("getboottype: can't figure out what is in slot %d\n", slot);
				boottype[0] = '\0';
				namelen = 0;
				break;
			    }

			    ctname = tc_slot_to_ctlr_name(slot);
			    if ((int)ctname == -1) {
				printf("getboottype: can't get controller name for interface in slot %d\n", slot);
				boottype[0] = '\0';
				namelen = 0;
				break;
			    }
			    namelen = strlen(ctname);
			    bcopy(ctname, boottype, namelen);
			    if ((unit = tc_get_unit_num(slot)) == -1) {
				printf("getboottype: can't figure out the unit number for the %s in slot %d\n", cp, slot);
				boottype[0] = '\0';
				namelen = 0;
				break;
			    }
			    mapfactor = slot - unit; /* kludge to make "unit" calculation
						  * below get the Right Answer
						  */
			}
                        break;

		case DEC_4000:
                        /*
			 * Assume that the controller is 'ln*',
			 * and that tc_slot 6 is unit 0;
                         */
			namelen = 2;
                        bcopy("te",boottype,namelen);
			mapfactor=6;
			break;

		case DEC_2000_300:
			/*
			 * Kn121 systems have no baseboard slots,
			 * all network boot devices will come
			 * from a logically numbered slot.
			 *
			 * This entire routine needs to be
			 * overhauled!
			 */
			cp = eisa_slot_to_name(slot);

			/* yuck: why -1 for failure?? */
			if ((int)cp == -1) {
			    printf("getboottype: can't figure out what is in slot %d\n", slot);
			    boottype[0] = '\0';
			    namelen = 0;
			    break;
			}

			ctname = eisa_slot_to_ctlr_name(slot);
			if ((int)ctname == -1) {
			    printf("getboottype: can't get controller name for interface in slot %d\n", slot);
			    boottype[0] = '\0';
			    namelen = 0;
			    break;
			}
			namelen = strlen(ctname);
			bcopy(ctname, boottype, namelen);
			if ((unit = eisa_get_unit_num(slot)) == -1) {
			    printf("getboottype: can't figure out the unit number for the %s in slot %d\n", cp, slot);
			    boottype[0] = '\0';
			    namelen = 0;
			    break;
			}
			mapfactor = slot - unit; /* kludge to make "unit" calculation
						  * below get the Right Answer
						  */
			break;
			
		case ALPHA_ADU:
		case DEC_7000:
			/* These aren't supposed to boot over the network */
			printf ("getboottype: no network boot support for cpu type %d\n", cpu);
                	boottype[0] = '\0';
			break;

		default:
			printf ("getboottype: no network boot support for cpu type %d\n", cpu);
                	boottype[0] = '\0';
			break;

	} /* end of switch */
	if (boottype[0] == '\0')
		return;
	slot = slot - mapfactor; /* do slot-to-unit mapping */
	unitlen=itoa(slot, buffer);
	bcopy (buffer, &boottype[namelen], unitlen);
	boottype[namelen+unitlen+1]='\0'; 
}

static
itoa(number, string)    /* integer to ascii routine */
unsigned long number;
char string[];
{
	int a, b, c;
	int i;
	i = 0;

	do {
		string[i++] = number % 10 + '0';
	} while ((number /= 10) > 0);
	string[i] = '\0';
	/* flip the string in place */
	for (b = 0, c = strlen(string)-1; b < c; b++, c--) {
		a = string[b];
		string[b] = string[c];
		string[c] = a;
	}
	return( strlen(string) );
}

