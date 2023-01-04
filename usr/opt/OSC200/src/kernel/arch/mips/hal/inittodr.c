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
static char *rcsid = "@(#)$RCSfile: inittodr.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/04/14 16:29:09 $";
#endif

#include <sys/kernel.h>
#include <hal/cpuconf.h>
#include <hal/clock.h>

extern int cpu;
extern struct cpusw *cpup;

/*
 * Initialize the time of day register, based on the time base which is
 * from a filesystem.  Base provides the time to within six months.
 */
inittodr(base)
	time_t base;
{
	register u_int todr;
	long deltat;
	int year = YRREF;
	int checktodr();
/* DGD -- declared in sys/kern_errlog.c; omitted for now ...
	extern long savetime;   /* used to save the boot time for errlog */

	/*
	 * Once a day check for clock rollover
	 */
	timeout(checktodr, 0, SECDAY*hz);

	todr = read_todclk();	

	/*
	 * NOTE: 
	 * On pmax & 3max, read_todclk returns seconds from the
	 *    beginning of the year + (1 << 26).
	 * On Mipsfair or Isis (with 10mS VAX time of day clock),
 	 *    read_todclk returns the number of 10mS ticks from the
	 *    beginning of the year + (1 << 28).
	 */
	if (base < 5*SECYR) {
		printf("WARNING: preposterous time in file system");
		time.tv_sec = 6*SECYR + 186*SECDAY + SECDAY/2;
		resettodr();
		goto check;
	}
	/*
	 * cpup->todrzero is base used to detected loss of power to TODCLK
	 */
	if (todr < cpup->todrzero) {
		printf("WARNING: lost battery backup clock");
		time.tv_sec = base;
		/*
		 * Believe the time in the file system for lack of
		 * anything better, resetting the TODR.
		 */
		resettodr();
		goto check;
	}

	/*
	 * Sneak to within 6 month of the time in the filesystem,
	 * by starting with the time of the year suggested by the TODR,
	 * and advancing through succesive years.  Adding the number of
	 * seconds in the current year takes us to the end of the current year
	 * and then around into the next year to the same position.
	 */

	time.tv_sec = (todr - cpup->todrzero) / cpup->rdclk_divider;

	while (time.tv_sec < base-SECYR/2) {
		if (LEAPYEAR(year))
			time.tv_sec += SECDAY;
		year++;
		time.tv_sec += SECYR;
	}

	/*
	 * See if we gained/lost two or more days;
	 * if so, assume something is amiss.
	 */
	deltat = time.tv_sec - base;
	if (deltat < 0)
		deltat = -deltat;
	if (deltat >= 2*SECDAY) {
		printf("WARNING: clock %s %d days",
		time.tv_sec < base ? "lost" : "gained", deltat / SECDAY);
	}
	resettodr();
/* DGD -- omitted for now 
	savetime = time.tv_sec;
 */
	return;
check:
/* DGD -- omitted for now 
	savetime = time.tv_sec;
 */
	printf(" -- CHECK AND RESET THE DATE!\n");
	return;
}

