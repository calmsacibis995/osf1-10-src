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
static char	*sccsid = "@(#)$RCSfile: pic_isa.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:09:54 $";
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

#include <sys/types.h>
#include <i386/ipl.h>
#include <i386/pic.h>
#include <i386/handler.h>
#include <i386/AT386/atbus.h>


int
i386_hardclock(line)
{
	/*
	 * Not used because locore.s intercepts and calls hardclock
	 * directly. Need to find old pc and ps (later).
	 */
	hardclock((caddr_t)0, 0, line);
	return 1;
}

/*
 * The interrupt level for each pic line. Clock, keyboard and FP
 * are statically initialized, others added by pic_setup().
 */
u_char intpri[NINTR] = {
	/* 00 */	SPLHI, 	SPL6,	0,	0,
	/* 04 */	0,	0,	0,	0,
	/* 08 */	0,	0,	0,	0,
	/* 12 */	0,	SPL1,	0,	0,
};
/*
 * If a device or driver cannot share a pic line, due to
 * hardware or software restrictions, this array remembers.
 * However, not yet, as the information is not passed.
 */
u_char intpri_canshare[NINTR] = {
	/* 00 */	0, 	0,	0,	1,
	/* 04 */	1,	1,	1,	1,
	/* 08 */	1,	1,	1,	1,
	/* 12 */	1,	0,	1,	1,
};

/*
 * pic_setup() is called from the resolver to set up the intpri
 * array for locore, if valid. Checks for invalid or collision.
 * If line in use at same spl, assume sharable pic lines are OK.
 */
pic_setup(ih)
	ihandler_t *ih;
{
	int pic, spl;

	if (ih->ih_rdev) {
		pic = ih->ih_rdev->dev_pic;
		spl = ih->ih_rdev->dev_spl;
	} else if (ih->ih_rctlr) {
		pic = ih->ih_rctlr->ctlr_pic;
		spl = ih->ih_rctlr->ctlr_spl;
	} else
		return -1;

	switch (pic) {
	default:
		printf("Invalid irq assignment %d\n", pic);
		return -1;
	case 0: case 1: case 2: case 13:
		printf("Invalid use of reserved irq %d\n", pic);
		return -1;
	case 3: case 4: case 5: case 6: case 7: case 8:
	case 9: case 10: case 11: case 12: case 14: case 15:
		if ((unsigned)(spl-1) > SPLHI) {
			printf("Invalid spl %d on irq %d\n", spl, pic);
			return -1;
		}
		if (intpri[pic] &&	/* Already in use, is ok to share? */
		    (intpri[pic] != spl || !intpri_canshare[pic])) {
			/* Might remap device if possible, saving
			 * new line assignment in dev or ctlr. */
			printf("Irq %d already in use at spl %d\n", pic, spl);
			return -1;
		}
		break;
	}
	intpri[pic] = spl;
	form_pic_mask();
	return 0;
}

/*
 * Resolver for device conflicts. Called from handler_add.
 * Uses pic_setup to validate pic line use.
 * Does not currently check for:
 *	Device address conflicts (port or physaddr)
 *	Pic line sharability
 */
i386_resolver(ih)
	ihandler_t *ih;
{
	/* Check address collisions and other device parameters here. */
	/* ... */
	return pic_setup(ih);
}
