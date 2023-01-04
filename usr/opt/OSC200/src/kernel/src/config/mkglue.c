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
static char	*sccsid = "@(#)$RCSfile: mkglue.c,v $ $Revision: 4.3.3.2 $ (DEC) $Date: 1992/01/28 11:59:17 $";
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
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/************************************************************************
 *									*
 *			Copyright (c) 1990 by				*
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
 *   This software is  derived  from  software  received  from  the	*
 *   University    of   California,   Berkeley,   and   from   Bell	*
 *   Laboratories.  Use, duplication, or disclosure is  subject  to	*
 *   restrictions  under  license  agreements  with  University  of	*
 *   California and with AT&T.						*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/
/* ------------------------------------------------------------------ */
/* | Copyright Unpublished, MIPS Computer Systems, Inc.  All Rights | */
/* | Reserved.  This software contains proprietary and confidential | */
/* | information of MIPS and its suppliers.  Use, disclosure or     | */
/* | reproduction is prohibited without the prior express written   | */
/* | consent of MIPS.                                               | */
/* ------------------------------------------------------------------ */


/*		Change History						*
 *									*
 * 24-Apr-91  afd
 *            Add Alpha support.
 *									*
 * 3-20-91	robin-							*
 *		Made changes to support new device data structures.	*
 *									*
 * 4-10-91	robin-							*
 *		Added BINARY build and make depend functions		*
 *									*
 */



#include <stdio.h>
#include "config.h"
#include "y.tab.h"
#include <ctype.h>

/*
 * Create the UNIBUS interrupt vector glue file.
 */
ubglue()
{
	register FILE *fp, *gp;
	register struct device_entry *dp, *mp;
	struct unique_list *uql, *uqlh;

	uql = uqlh = 0;
	fp = fopen(path("scb_vec.c"), "w");
	if (fp == 0) {
		perror(path("scb_vec.c"));
		Exit(1);
	}

	/* make a list with only ONE entry for every name & unit pair
	 */
	for (dp = dtab; dp != 0; dp = dp->d_next) {

			if(uql == (struct unique_list *) 0)
			{
				uql = (struct  unique_list *)malloc(sizeof (struct  unique_list));
				uqlh = uql;
				uql->name = dp->d_name;
				uql->number = dp->d_unit;
				uql->dp = dp;
				uql->next = (struct unique_list *)0;
			}else{

				uql->next = (struct unique_list *)malloc(sizeof (struct unique_list));
				uql = uql->next;
				uql->name = dp->d_name;
				uql->number = dp->d_unit;
				uql->dp = dp;
				uql->next = (struct unique_list *)0;
			}
		}
	make_unique(uqlh);

	/*
	 * Now generate  interrupt vectors for the unibus
	 */
	for (uql = uqlh; uql != 0; uql = uql->next) {

		dp = uql->dp;
		mp = dp->d_conn;
		 if (( mp != 0)
		     && ((dp->d_type == BUS) || (dp->d_type == CONTROLLER)))
		 {
			struct idlst *id, *id2;

			for (id = dp->d_vec; id; id = id->id_next) {
				for (id2 = dp->d_vec; id2; id2 = id2->id_next) {
					if (id2 == id) {
						dump_vec(fp, id->id, 
							 dp->d_unit);
						break;
					}
					if (!strcmp(id->id, id2->id))
						break;
				}
			}
		}
	}
	(void) fclose(fp);
}

static int cntcnt = 0;		/* number of interrupt counters allocated */

int dmzr = 0;
int dmzx = 0;

/*
 * print an interrupt vector
 */
dump_vec(fp, vector, number)
	register FILE *fp;
	char *vector;
	int number;
{
	char nbuf[80];
	register char *v = nbuf;

	fprintf(fp,"X%s%d(stray_arg)\nint stray_arg;\n",vector,number);
	fprintf(fp,"/* stray_arg is not used here but locore calls it with\n");
	fprintf(fp," * an argument that is the offset into the scb data structure;\n");
	fprintf(fp," * which the stray interrupt routine uses to find where the\n");
	fprintf(fp," * stray came from.  The unused arg keeps everything consistent.\n");
	fprintf(fp," */\n{\n");
	fprintf(fp,"\textern %s();\n",vector);
	fprintf(fp,"\t%s(%d);\n",vector,number);
	fprintf(fp,"}\n");

}

#ifdef NOTDEF

static	char *vaxinames[] = {
	"clock", "cnr", "cnx", "tur", "tux",
	"mba0", "mba1", "mba2", "mba3",
	"uba0", "uba1", "uba2", "uba3"
};
static	struct stdintrs {
	char	**si_names;	/* list of standard interrupt names */
	int	si_n;		/* number of such names */
} stdintrs[] = {
	{ vaxinames, sizeof (vaxinames) / sizeof (vaxinames[0]) },
};
/*
 * Start the interrupt name table with the names
 * of the standard vectors not directly associated
 * with a bus.  Also, dump the defines needed to
 * reference the associated counters into a separate
 * file which is prepended to locore.s.
 */
dump_std(fp, gp)
	register FILE *fp, *gp;
{
	register struct stdintrs *si = &stdintrs[machine-1];
	register char **cpp;
	register int i;

	if(machine != MACHINE_VAX)
		return;
	fprintf(fp, "\n\t.globl\t_intrnames\n");
	fprintf(fp, "\n\t.globl\t_eintrnames\n");
	fprintf(fp, "\t.data\n");
	fprintf(fp, "_intrnames:\n");
	cpp = si->si_names;
	for (i = 0; i < si->si_n; i++) {
		register char *cp, *tp;
		char buf[80];

		cp = *cpp;
		if (cp[0] == 'i' && cp[1] == 'n' && cp[2] == 't') {
			cp += 3;
			if (*cp == 'r')
				cp++;
		}
		for (tp = buf; *cp; cp++)
			if (islower(*cp))
				*tp++ = toupper(*cp);
			else
				*tp++ = *cp;
		*tp = '\0';
		fprintf(gp, "#define\tI_%s\t%d\n", buf, i*sizeof (long));
		fprintf(fp, "\t.asciz\t\"%s\"\n", *cpp);
		cpp++;
	}
}

dump_intname(fp, vector, number)
	register FILE *fp;
	char *vector;
	int number;
{
	register char *cp = vector;

	fprintf(fp, "\t.asciz\t\"");
	/*
	 * Skip any "int" or "intr" in the name.
	 */
	while (*cp)
		if (cp[0] == 'i' && cp[1] == 'n' &&  cp[2] == 't') {
			cp += 3;
			if (*cp == 'r')
				cp++;
		} else {
			putc(*cp, fp);
			cp++;
		}
	fprintf(fp, "%d\"\n", number);
}

/*
 * Reserve space for the interrupt counters.
 */
dump_ctrs(fp)
	register FILE *fp;
{
	struct stdintrs *si = &stdintrs[machine-1];
	switch (machine) {

	case MACHINE_VAX:

		fprintf(fp, "_eintrnames:\n");
		fprintf(fp, "\n\t.globl\t_intrcnt\n");
		fprintf(fp, "\n\t.globl\t_eintrcnt\n");
		fprintf(fp, "\t.align 2\n");
		fprintf(fp, "_intrcnt:\n");
		fprintf(fp, "\t.space\t4 * %d\n", si->si_n);
		fprintf(fp, "_fltintrcnt:\n", cntcnt);
		fprintf(fp, "\t.space\t4 * %d\n", cntcnt);
		fprintf(fp, "_eintrcnt:\n\n");
		fprintf(fp, "\t.text\n");
        }
}

/*
 * Routines for making Sun mb interrupt file mbglue.s
 */

/*
 * print an interrupt handler for mainbus
 */
dump_mb_handler(fp, vec, number)
	register FILE *fp;
	register struct idlst *vec;
	int number;
{
	fprintf(fp, "\tVECINTR(_X%s%d, _%s, _V%s%d)\n",
		vec->id, number, vec->id, vec->id, number);
}

mbglue()
{
	register FILE *fp;
	char *name = "mbglue.s";

	fp = fopen(path(name), "w");
	if (fp == 0) {
		perror(path(name));
		exit(1);
	}
	fprintf(fp, "#include <machine/asm_linkage.h>\n\n");
	glue(fp, dump_mb_handler);
	(void) fclose(fp);
}
#endif /* NOTDEF */

glue(fp, dump_handler)
	register FILE *fp;
	register int (*dump_handler)();
{
	register struct device_entry *dp, *mp;

	for (dp = dtab; dp != 0; dp = dp->d_next) {
		mp = dp->d_conn;
		if (mp != 0 && mp != (struct device_entry *)-1 &&
		    !eq(mp->d_name, "mba")) {
			struct idlst *vd, *vd2;

			for (vd = dp->d_vec; vd; vd = vd->id_next) {
				for (vd2 = dp->d_vec; vd2; vd2 = vd2->id_next) {
					if (vd2 == vd) {
						(void)(*dump_handler)
							(fp, vd, dp->d_unit);
						break;
					}
					if (!strcmp(vd->id, vd2->id))
						break;
				}
			}
		}
	}
}
