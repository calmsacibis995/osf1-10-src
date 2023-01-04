
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
/*
 * @(#)$RCSfile: eucset.c,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1993/10/11 20:04:49 $
 */
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */
#include <stdio.h>
#include <locale.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <stropts.h>
#include <sys/eucioctl.h>

#include <nl_types.h>
#include "eucset_msg.h"

/*
 * functions used by eucset
 */
void default_cswidth(eucioc_t *wp);
void parse_cswidth(char *cp, eucioc_t *wp);
void set_cswidth(eucioc_t *wp);
void get_cswidth(eucioc_t *wp);

/*
 * global variables used by eucset
 */
int verbose = 1;	/* print out what is going on */
nl_catd catd;		/* message catalog */

#define MSGS(num, str)	catgets(catd, MS_EUCSET, num, str)

main(ac, av)
	register int ac;
	register char **av;
{
	eucioc_t cs_width;

	(void)setlocale(LC_ALL, "");
	catd = catopen(MF_EUCSET, NL_CAT_LOCALE);

	if (ac == 1) {
		default_cswidth(&cs_width);
		set_cswidth(&cs_width);
	}
	else if (ac == 2) {
		if (!strcmp(av[1], "-p"))
			get_cswidth(&cs_width);
		else if (av[1][0] == '-') {
			print_usage();
		}
		else {
			parse_cswidth(av[1], &cs_width);
			set_cswidth(&cs_width);
		}
	}
	else {
		print_usage();
	}
	exit(0);
}

void
default_cswidth(wp)
	register eucioc_t *wp;
{
	register char *cp;

	cp = (char *)getenv("CSWIDTH");
	if (cp)
		parse_cswidth(cp, wp);
	else {
		wp->eucw[1] = 1;
		wp->scrw[1] = 1;
		wp->eucw[2] = wp->eucw[3] = 0;
		wp->scrw[2] = wp->scrw[3] = 0;
	}
}

/*
 * parse the cswidth string
 * it should be in the form: X1[[:Y1][,X2[:Y2][,X3[:Y3]]]]
 */
void
parse_cswidth(cp, wp)
	register char *cp;
	register eucioc_t *wp;
{
	register int x;

#define FIND_NUMBER(x, cp) \
{ \
	x = 0; \
	while (*cp && isdigit(*cp)) { \
		x *= 10; \
		x += (*cp - '0'); \
		cp++; \
	} \
}

	/*
	 * set all cs widths to 0
	 */
	wp->eucw[1] = wp->eucw[2] = wp->eucw[3] = 0;
	wp->scrw[1] = wp->scrw[2] = wp->scrw[3] = 0;
	if (!cp)
		return;
	/*
	 * read X1
	 */
	FIND_NUMBER(x, cp);
	wp->eucw[1] = x;
	if (!*cp) {
		wp->scrw[1] = wp->eucw[1];
		return;
	}
	/*
	 * Y1 might exist
	 */
	if (*cp == ':') {
		cp++;
		FIND_NUMBER(x, cp);
		wp->scrw[1] = x;
		if (!*cp)
			return;
	}
	else {
		wp->scrw[1] = wp->eucw[1];
	}
	cp++;
	/*
	 * read X2
	 */
	FIND_NUMBER(x, cp);
	wp->eucw[2] = x;
	if (!*cp) {
		wp->scrw[2] = wp->eucw[2];
		return;
	}
	/*
	 * Y2 might exist
	 */
	if (*cp == ':') {
		cp++;
		FIND_NUMBER(x, cp);
		wp->scrw[2] = x;
		if (!*cp)
			return;
	}
	else {
		wp->scrw[2] = wp->eucw[2];
	}
	cp++;
	/*
	 * read X3
	 */
	x = 0;
	FIND_NUMBER(x, cp);
	wp->eucw[3] = x;
	if (!*cp) {
		wp->scrw[3] = wp->eucw[3];
		return;
	}
	cp++;
	FIND_NUMBER(x, cp);
	wp->scrw[3] = x;
}

void
set_cswidth(wp)
	register eucioc_t *wp;
{
	struct strioctl i_str;

	i_str.ic_cmd = EUC_WSET;
	i_str.ic_timout = 0;
	i_str.ic_len = sizeof(struct eucioc);
	i_str.ic_dp = (char *)wp;
	wp->eucw[0] = 1;
	wp->scrw[0] = 1;
	if (verbose) {
		printf(MSGS(SETTING, "setting cswidth %d:%d,%d:%d,%d:%d\n"),
			wp->eucw[1], wp->scrw[1],
			wp->eucw[2], wp->scrw[2],
			wp->eucw[3], wp->scrw[3]);
	}
	if (ioctl(0, I_STR, &i_str) < 0)
		ioctl_error();
}

void
get_cswidth(wp)
	register eucioc_t *wp;
{
	struct strioctl i_str;

	bzero(wp, sizeof(struct eucioc));
	i_str.ic_cmd = EUC_WGET;
	i_str.ic_timout = 0;
	i_str.ic_len = sizeof(struct eucioc);
	i_str.ic_dp = (char *)wp;
	if (ioctl(0, I_STR, &i_str) < 0)
		ioctl_error();
	printf(MSGS(RESULT, "cswidth %d:%d,%d:%d,%d:%d\n"),
		wp->eucw[1], wp->scrw[1],
		wp->eucw[2], wp->scrw[2],
		wp->eucw[3], wp->scrw[3]);
}

ioctl_error()
{
	printf(MSGS(IOCTLERR, "eucset: ioctl error\n"));
	exit(1);
}

print_usage()
{
	printf(MSGS(USAGE1, "usage: eucset [ cswidth ]\n"));
	printf(MSGS(USAGE2, "       eucset -p\n"));
	exit(1);
}
