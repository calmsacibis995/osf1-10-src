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
static char     *sccsid = "@(#)$RCSfile: getgroup.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 21:59:42 $";
#endif
/*
 */


#include <stdio.h>
#include <ctype.h>
#include <strings.h>
#include "table.h"
#include "util.h"
#include "getgroup.h"

#define MAXGROUPLEN 1024

/*
 * Stolen mostly, from getnetgrent.c          
 *
 * my_getgroup() performs the same function as _getgroup(), but operates 
 * on /etc/netgroup directly, rather than doing NIS lookups.
 * 
 * /etc/netgroup must first loaded into a hash table so the matching
 * function can look up lines quickly.
 */


/* To check for cycles in netgroups */
struct list {
	char *name;
	struct list *nxt; 
};


extern stringtable ngtable; /* stored info from /etc/netgroup */

static struct grouplist *grouplist; /* stores a list of users in a group */

static char *any();
static char *match();
static char *fill();
static void freegrouplist();
static void doit();



static void
freegrouplist() 
{
	struct grouplist *gl;
	
	for (gl = grouplist; gl != NULL; gl = gl->gl_nxt) {
		FREE(gl->gl_name);
		FREE(gl->gl_domain);
		FREE(gl->gl_machine);
		FREE(gl);
	}
	grouplist = NULL;
}




struct grouplist *
my_getgroup(group)
	char *group;
{
	freegrouplist();
	doit(group,(struct list *) NULL);
	return grouplist;
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
	register struct list *ls, tmplist;
	char *val;
	struct grouplist *gpls;


	/*
	 * check for non-existing groups
  	 */ 
	if ((val = match(group)) == NULL) {
		return;
	}


	/* 
	 * check for cycles 
	 */
	for (ls = list; ls != NULL; ls = ls->nxt) {
		if (strcmp(ls->name, group) == 0) {
			(void)fprintf(stderr,
				"Cycle detected in /etc/netgroup: %s.\n",group);
			return;	
		}
	}


	ls = &tmplist;
	ls->name = group;
	ls->nxt = list;
	list = ls;
	
	p = val;
	while (p != NULL) {
		while (*p == ' ' || *p == '\t')
			p++;
		if (*p == EOS || *p =='#')
			break;
		if (*p == '(') {
			gpls = MALLOC(struct grouplist);
			p++;

			if (!(p = fill(p,&gpls->gl_machine,',')))  {
				goto syntax_error;
			}
			if (!(p = fill(p,&gpls->gl_name,','))) {
				goto syntax_error;
			}
			if (!(p = fill(p,&gpls->gl_domain,')'))) {
				goto syntax_error;
			}
			gpls->gl_nxt = grouplist;
			grouplist = gpls;
		} else {
			q = any(p, " \t\n#");
			if (q && *q == '#')
				break;
			*q = EOS;
			doit(p,list);
			*q = ' ';
		}
		p = any(p, " \t");
	}
	return;

syntax_error:
	(void) fprintf(stderr,"syntax error in /etc/netgroup\n");
	(void) fprintf(stderr,"--- %s %s\n",group,val);
	return;
}




/*
 * Fill a buffer "target" selectively from buffer "start".
 * "termchar" terminates the information in start, and preceding
 * or trailing white space is ignored.  If the buffer "start" is
 * empty, "target" is filled with "*". The location just after the 
 * terminating character is returned.
 */
static char *
fill(start,target,termchar)
	char *start;
	char **target;
	char termchar;
{
	register char *p;
	register char *q;
	register char *r;
	int size;

	for (p = start; *p == ' ' || *p == '\t'; p++)
		;
	r = index(p, termchar);
	if (r == NULL) {
		return(NULL);	
	}
	if (p == r) {
		*target = NULL;
	} else {
		for (q = r-1; *q == ' ' || *q == '\t'; q--)
			;
		size = q-p+1;
		STRNCPY(*target, p, size);
	}
	return(r+1);	
}


/* 
 * scans cp, looking for a match with any character
 * in match.  Returns pointer to place in cp that matched
 * (or NULL if no match)
 */
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
	return (NULL);
}



/*
 * The equivalent of yp_match. Returns the match, or NULL if there is none. 
 */
static char *
match(group)
	char *group;
{
	return(lookup(ngtable,group));
}


