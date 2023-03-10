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
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: value.c,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1993/09/30 18:39:58 $";
#endif

/* value.c	1.3  com/cmd/tip,3.1,9013 10/15/89 10:44:51"; */
/* 
 * COMPONENT_NAME: UUCP value.c
 * 
 * FUNCTIONS: MSGSTR, vaccess, vassign, vinit, vinterp, vlex, 
 *            vlookup, vprint, vstring, vtoken 
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/* static char sccsid[] = "value.c	5.1 (Berkeley) 4/30/85"; */

#include "tip.h"

#define MIDDLE	35

static value_t *vlookup();
static int col = 0;
static	int vtoken();
static	int vprint();
static	int vaccess();

/*
 * Variable manipulation
 */
vinit()
{
	register value_t *p;
	register char *cp;
	FILE *f;
	char file[256];

	for (p = vtable; p->v_name != NULL; p++) {
		if (p->v_type&ENVIRON)
			if (cp = (char *) getenv( p->v_name))
				p->v_value = cp;
		if (p->v_type&IREMOTE)
			number(p->v_value) = *address(p->v_value);
	}
	/*
	 * Read the .tiprc file in the HOME directory
	 *  for sets
	 */
	strcpy(file, value(HOME));
	strcat(file, "/.tiprc");
	if ((f = fopen(file, "r")) != NULL) {
		register char *tp;

		while (fgets(file, sizeof(file)-1, f) != NULL) {
			if (vflag)
				printf(MSGSTR(SET2, "set %s"), file); /*MSG*/
			if (tp = (char *) rindex( file, '\n'))
				*tp = '\0';
			vlex(file);
		}
		fclose(f);
	}
	/*
	 * To allow definition of exception prior to fork
	 */
	vtable[EXCEPTIONS].v_access &= ~(WRITE<<PUBLIC);
}

/*VARARGS1*/
vassign(p, v)
	register value_t *p;
	char *v;
{

	if (!vaccess(p->v_access, WRITE)) {
		printf(MSGSTR(ACCESSDENY, "access denied\r\n")); /*MSG*/
		return;
	}
	switch (p->v_type&TMASK) {

	case STRING:
            	if (p->v_value != (char *) 0) {
                    if (equal(p->v_value, v))
                        return;
                    if (!(p->v_type&(ENVIRON|INIT)))
                        free(p->v_value);
                }
                if ((p->v_value = (char *)malloc(size(v)+1)) == NOSTR) {
			printf(MSGSTR(NOCORE, "out of core\r\n")); /*MSG*/
			return;
		}
		p->v_type &= ~(ENVIRON|INIT);
		strcpy(p->v_value, v);
		break;

	case NUMBER:
		if (number(p->v_value) == number(v))
			return;
		number(p->v_value) = number(v);
		break;

	case BOOL:
		if (boolean(p->v_value) == (*v != '!'))
			return;
		boolean(p->v_value) = (*v != '!');
		break;

	case CHAR:
		if (character(p->v_value) == *v)
			return;
		character(p->v_value) = *v;
	}
	p->v_access |= CHANGED;
}

vlex(s)
	register char *s;
{
	register value_t *p;

	if (equal(s, "all")) {
		for (p = vtable; p->v_name; p++)
			if (vaccess(p->v_access, READ))
				vprint(p);
	} else {
		register char *cp;

		do {
			if (cp = (char *) vinterp((char *) s, (char) ' '))
				cp++;
			vtoken(s);
			s = cp;
		} while (s);
	}
	if (col > 0) {
		printf("\r\n");
		col = 0;
	}
}

static int
vtoken(s)
	register char *s;
{
	register value_t *p;
	register char *cp;
	char *expand();

	if (cp = (char *) index(s, '=')) {
		*cp = '\0';
		if (p = vlookup(s)) {
			cp++;
			if (p->v_type&NUMBER)
				vassign(p, atoi(cp));
			else {
				if (strcmp(s, "record") == 0)
					cp = expand(cp);
				vassign(p, cp);
			}
			return;
		}
	} else if (cp = (char *) index(s, '?')) {
		*cp = '\0';
		if ((p = vlookup(s)) && vaccess(p->v_access, READ)) {
			vprint(p);
			return;
		}
	} else {
		if (*s != '!')
			p = vlookup(s);
		else
			p = vlookup(s+1);
		if (p != NOVAL) {
			vassign(p, s);
			return;
		}
	}
	printf(MSGSTR(UNKVAR, "%s: unknown variable\r\n"), s); /*MSG*/
}

static int
vprint(p)
	register value_t *p;
{
	register char *cp;
	extern char *interp(), *ctrl();

	if (col > 0 && col < MIDDLE)
		while (col++ < MIDDLE)
			putchar(' ');
	col += size(p->v_name);
	switch (p->v_type&TMASK) {

	case BOOL:
		if (boolean(p->v_value) == FALSE) {
			col++;
			putchar('!');
		}
		printf("%s", p->v_name);
		break;

	case STRING:
		printf("%s=", p->v_name);
		col++;
		if (p->v_value) {
			cp = interp(p->v_value, NULL);
			col += size(cp);
			printf("%s", cp);
		}
		break;

	case NUMBER:
		col += 6;
		printf("%s=%-5d", p->v_name, number(p->v_value));
		break;

	case CHAR:
		printf("%s=", p->v_name);
		col++;
		if (p->v_value) {
			cp = ctrl(character(p->v_value));
			col += size(cp);
			printf("%s", cp);
		}
		break;
	}
	if (col >= MIDDLE) {
		col = 0;
		printf("\r\n");
		return;
	}
}


static int
vaccess(mode, rw)
	register unsigned mode, rw;
{
	if (mode & (rw<<PUBLIC))
		return (1);
	if (mode & (rw<<PRIVATE))
		return (1);
	return ((mode & (rw<<ROOT)) && getuid() == 0);
}

static value_t *
vlookup(s)
	register char *s;
{
	register value_t *p;

	for (p = vtable; p->v_name; p++)
		if (equal(p->v_name, s) || (p->v_abrev && equal(p->v_abrev, s)))
			return (p);
	return (NULL);
}

char *
vinterp(s, stop)
	register char *s;
	char stop;
{
	register char *p = s, c;
	int num;

	while ((c = *s++) && c != stop)
		switch (c) {

		case '^':
			if (*s)
				*p++ = *s++ - 0100;
			else
				*p++ = c;
			break;

		case '\\':
			num = 0;
			c = *s++;
			if (c >= '0' && c <= '7')
				num = (num<<3)+(c-'0');
			else {
				register char *q = "n\nr\rt\tb\bf\f";

				for (; *q; q++)
					if (c == *q++) {
						*p++ = *q;
						goto cont;
					}
				*p++ = c;
			cont:
				break;
			}
			if ((c = *s++) >= '0' && c <= '7') {
				num = (num<<3)+(c-'0');
				if ((c = *s++) >= '0' && c <= '7')
					num = (num<<3)+(c-'0');
				else
					s--;
			} else
				s--;
			*p++ = num;
			break;

		default:
			*p++ = c;
		}
	*p = '\0';
	return (c == stop ? s-1 : NULL);
}

/*
 * assign variable s with value v (for NUMBER or STRING or CHAR types)
 */

vstring(s,v)
	register char *s;
	register char *v;
{
	register value_t *p;
	char *expand();

	p = vlookup(s); 
	if (p == 0)
		return (1);
	if (p->v_type&NUMBER)
		vassign(p, atoi(v));
	else {
		if (strcmp(s, "record") == 0)
			v = expand(v);
		vassign(p, v);
	}
	return (0);
}
