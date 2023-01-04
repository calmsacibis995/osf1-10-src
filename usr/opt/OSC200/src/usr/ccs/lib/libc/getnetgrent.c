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
static char     *sccsid = "@(#)$RCSfile: getnetgrent.c,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/06/07 23:00:16 $";
#endif
/*
 */


/****************************************************************
 *                                                              *
 *  Licensed to Digital Equipment Corporation, Maynard, MA      *
 *              Copyright 1985 Sun Microsystems, Inc.           *
 *                      All rights reserved.                    *
 *								*
 ****************************************************************/

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak NETGROUP = __NETGROUP
#pragma weak endnetgrent = __endnetgrent
#pragma weak getnetgrent = __getnetgrent
#pragma weak setnetgrent = __setnetgrent
#endif
#endif
#include <stdio.h>
#include <ctype.h>
#include <rpcsvc/ypclnt.h>

#if defined(lint) && !defined(DEBUG)
#define DEBUG
#endif

#define MAXGROUPLEN 1024

/* 
 * access members of a netgroup
 */

static struct grouplist {		/* also used by pwlib */
	char	*gl_machine;
	char	*gl_name;
	char	*gl_domain;
	struct	grouplist *gl_nxt;
} *grouplist, *grlist;

extern char *strcpy(),*strncpy();

extern char *strpbrk(), *index();

static struct list {			/* list of names to check for loops */
	char *name;
	struct list *nxt;
};

static  void getgroup();
static	void doit();
static	char *fill();
static	char *match();

static	char domain[256];
static	char *oldgrp;

char	NETGROUP[] = "netgroup";

setnetgrent(grp)
	char *grp;
{
	
	if (oldgrp == NULL)
		oldgrp = (char *)calloc(1,256);
	if (strcmp(oldgrp, grp) == 0)
		grlist = grouplist;
	else {
		if (grouplist != NULL)
			endnetgrent();
		getgroup(grp);
		grlist = grouplist;
		(void) strcpy(oldgrp, grp);
	}
}

endnetgrent()
{
	register struct grouplist *gl;
	
	for (gl = grouplist; gl != NULL; gl = gl->gl_nxt) {
		if (gl->gl_name)
			free(gl->gl_name);
		if (gl->gl_domain)
			free(gl->gl_domain);
		if (gl->gl_machine)
			free(gl->gl_machine);
		free((char *) gl);
	}
	grouplist = NULL;
	grlist = NULL;
	if (oldgrp) {
		free(oldgrp);
		oldgrp = 0;
	}
}

getnetgrent(machinep, namep, domainp)
	char **machinep, **namep, **domainp;
{

	if (grlist == 0)
		return (0);
	*machinep = grlist->gl_machine;
	*namep = grlist->gl_name;
	*domainp = grlist->gl_domain;
	grlist = grlist->gl_nxt;
	return (1);
}

static void
getgroup(grp)
        char *grp;
{
        if (getdomainname(domain, sizeof(domain)) < 0) {
               (void) fprintf(stderr,
                    "getnetgrent: getdomainname system call missing\n");
            exit(1);
        }
        doit(grp,(struct list *) NULL);
}

/*
 * recursive function to find the members of netgroup "group". "list" is
 * the path followed through the netgroups so far, to check for cycles.
 */
static void
doit(group,list)
	char *group;
	struct list *list;
{
	register char *p, *q;
	register struct list *ls;
	struct list this_group;
	char *val;
	struct grouplist *gpls;
 
	/*
	 * check for non-existing groups
	 */
	if ((val = match(group)) == NULL)
		return;
 
	/*
	 * check for cycles
	 */
	for (ls = list; ls != NULL; ls = ls->nxt)
		if (strcmp(ls->name, group) == 0) {
			(void) fprintf(stderr,
			    "Cycle detected in /etc/netgroup: %s.\n", group);
			return;
		}
 
	ls = &this_group;
	ls->name = group;
	ls->nxt = list;
	list = ls;
    
	p = val;
	while (p != NULL) {
		while (*p == ' ' || *p == '\t')
			p++;
		if (*p == 0 || *p =='#')
			break;
		if (*p == '(') {
			gpls = (struct grouplist *)
			    malloc(sizeof(struct grouplist));
			p++;
			if (!(p = fill(p,&gpls->gl_machine,',')))
				goto syntax_error;
			if (!(p = fill(p,&gpls->gl_name,',')))
				goto syntax_error;
			if (!(p = fill(p,&gpls->gl_domain,')')))
				goto syntax_error;
			gpls->gl_nxt = grouplist;
			grouplist = gpls;
		} else {
			q = strpbrk(p, " \t\n#");
			if (q && *q == '#')
				break;
			*q = 0;
			doit(p,list);
			*q = ' ';
		}
		p = strpbrk(p, " \t");
	}
	return;
 
syntax_error:
	(void) fprintf(stderr,"syntax error in /etc/netgroup\n");
	(void) fprintf(stderr,"--- %s\n",val);
	return;
}

/*
 * Fill a buffer "target" selectively from buffer "start".
 * "termchar" terminates the information in start, and preceding
 * or trailing white space is ignored. The location just after the
 * terminating character is returned.  
 */
static char *
fill(start,target,termchar)
	char *start, **target, termchar;
{
	register char *p, *q; 
	char *r;
	size_t size;
 
	for (p = start; *p == ' ' || *p == '\t'; p++)
		;
	r = index(p, termchar);
	if (r == NULL)
		return (NULL);
	if (p == r)
		*target = NULL;	
	else {
		for (q = r-1; *q == ' ' || *q == '\t'; q--)
			;
		size = q - p + 1;
		*target = (char *) malloc(size+1);
		(void) strncpy(*target,p,(int) size);
		(*target)[size] = 0;
	}
	return (r+1);
}

static char *
match(group)
	char *group;
{
	char *val;
	int vallen;

	if (yp_match(domain, NETGROUP, group, strlen(group), &val, &vallen))
		return (NULL);
	return (val);
}
