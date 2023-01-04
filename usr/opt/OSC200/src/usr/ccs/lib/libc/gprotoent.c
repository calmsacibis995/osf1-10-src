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
static char	*sccsid = "@(#)$RCSfile: gprotoent.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 03:36:32 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
#if !defined(lint) && !defined(_NOIDENT)

#endif
/*
static char sccsid[] = "@(#)gprotoent.c	3.1  @(#)gprotoent.c	3.1 2/26/91 10:14:34";
*/
/* 
 * COMPONENT_NAME: CMDNET gprotoent.c
 * 
 * FUNCTIONS: any, endprotoent, getprotobyname, getprotobynumber, 
 *            getprotoent,  setprotoent
 *
 * ORIGINS: 24  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * 
 */
/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * getproto.c  5.6 (Berkeley) 6/1/90
 * getprotoent.c       5.7 (Berkeley) 6/1/90
 * getprotoname.c      5.6 (Berkeley) 6/1/90
 */
#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <ctype.h>

#define	MAXALIASES	35

int _proto_stayopen;

struct protoent *
getprotobynumber(proto)
	register int proto;
{
	register struct protoent *p;

	setprotoent(_proto_stayopen);
	while (p = getprotoent())
		if (p->p_proto == proto)
			break;
	if (!_proto_stayopen)
		endprotoent();
	return (p);
}

static FILE *protof = NULL;
static char line[BUFSIZ+1];
static struct protoent proto;
static char *proto_aliases[MAXALIASES];
static char *any();

setprotoent(f)
	int f;
{
	if (protof == NULL)
		protof = fopen(_PATH_PROTOCOLS, "r" );
	else
		rewind(protof);
	_proto_stayopen |= f;
}

endprotoent()
{
	if (protof) {
		fclose(protof);
		protof = NULL;
	}
	_proto_stayopen = 0;
}

struct protoent *
getprotoent()
{
	char *p;
	register char *cp, **q;

	if (protof == NULL && (protof = fopen(_PATH_PROTOCOLS, "r" )) == NULL)
		return (NULL);
again:
	if ((p = fgets(line, BUFSIZ, protof)) == NULL)
		return (NULL);
	if (*p == '#')
		goto again;
	cp = any(p, "#\n");
	if (cp == NULL)
		goto again;
	*cp = '\0';
	proto.p_name = p;
	cp = any(p, " \t");
	if (cp == NULL)
		goto again;
	*cp++ = '\0';
	while (*cp == ' ' || *cp == '\t')
		cp++;
	p = any(cp, " \t");
	if (p != NULL)
		*p++ = '\0';
	proto.p_proto = atoi(cp);
	q = proto.p_aliases = proto_aliases;
	if (p != NULL) {
		cp = p;
		while (cp && *cp) {
			if (*cp == ' ' || *cp == '\t') {
				cp++;
				continue;
			}
			if (q < &proto_aliases[MAXALIASES - 1])
				*q++ = cp;
			cp = any(cp, " \t");
			if (cp != NULL)
				*cp++ = '\0';
		}
	}
	*q = NULL;
	return (&proto);
}

static char *
any(cp, match)
	register char *cp;
	char *match;
{
	register char *mp, c;

	while (c = *cp) {
		for (mp = match; *mp; mp++)
			if (*mp == c)
				return (cp);
		cp++;
	}
	return ((char *)0);
}

struct protoent *
getprotobyname(name)
	register const char *name;
{
	register struct protoent *p;
	register char **cp;

	setprotoent(_proto_stayopen);
	while (p = getprotoent()) {
		if (strcmp(p->p_name, name) == 0)
			break;
		for (cp = p->p_aliases; *cp != 0; cp++)
			if (strcmp(*cp, name) == 0)
				goto found;
	}
found:
	if (!_proto_stayopen)
		endprotoent();
	return (p);
}
