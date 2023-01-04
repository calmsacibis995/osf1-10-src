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
static char     *sccsid = "@(#)$RCSfile: revnetgroup.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/11/24 10:16:24 $";
#endif
/*
 */


#include <stdio.h>
#include <ctype.h>
#include <strings.h>
#include <pwd.h>
#include <rpcsvc/ypclnt.h>
#include "util.h"
#include "table.h"
#include "getgroup.h"

#define MAXDOMAINLEN 256
#define MAXGROUPLEN 1024

/*
 * Reverse the netgroup file. A flag of "-u" means reverse by username,
 * one of "-h" means reverse by hostname. Each line in the output file
 * will begin with a key formed by concatenating the host or user name
 * with the domain name. The key will be followed by a tab, then the
 * comma-separated, newline-terminated list of groups to which the 
 * user or host belongs.
 *
 * Exception: Groups to which everyone belongs (universal groups) will
 * not be included in the list.  The universal groups will be listed under
 * the special name "*".
 *
 * Thus to find out all the groups that user "foo" of domain "bar" is in,
 * lookup the groups under  foo.bar, foo.*, *.bar and *.*.
 *
 */



/* Stores a list of strings */
typedef struct stringnode *stringlist;
struct stringnode {
	char *str;
	stringlist next;
};
typedef struct stringnode stringnode;



/* Stores a list of (name,list-of-groups) */
typedef struct groupentrynode *groupentrylist;
struct groupentrynode {
	char *name;
	stringlist groups;
	groupentrylist next;
};
typedef struct groupentrynode groupentrynode;

stringtable ngtable;

static groupentrylist grouptable[TABLESIZE];  



static char *nextgroup();
static void storegroup();
static void enter();	 
static void appendgroup();
static groupentrylist newentry();
static void loadtable();
static void dumptable();

	

main(argc,argv)
	int argc;
	char *argv[];
{

	char *group;
	struct grouplist *glist;
	int byuser; 	

	loadtable(stdin);
	if (argc == 2 && argv[1][0] == '-' 
			&& (argv[1][1] == 'u' || argv[1][1] == 'h')) {
		byuser = (argv[1][1] == 'u');
	} else {
		(void) fprintf(stderr,
			"usage: %s -h (by host), %s -u (by user)\n",argv[0],argv[0]);
		exit(1);
	}

	while (group = nextgroup()) {
		glist = my_getgroup(group);
		storegroup(group,glist,byuser);
	}
	dumptable();

	exit(0);
	/* NOTREACHED */
}





/* 
 *	Get the next netgroup from /etc/netgroup 
 */
static char * 
nextgroup() 
{ 
	static int index = -1;
	static tablelist cur = NULL;
	char *group;

	while (cur == NULL) {
		if (++index == TABLESIZE) {
			return(NULL);
		}
		cur = ngtable[index];	
	}
	group = cur->key;
	cur = cur->next;	
	return(group);	
}
		


/* 
 * Dump out all of the stored info into a file 
 */
static void
dumptable()
{
	int i;
	groupentrylist entry;
	stringlist groups;

	for (i = 0; i < TABLESIZE; i++) { 
		if (entry = grouptable[i]) {
			while (entry) {
				fputs(entry->name,stdout);
				putc('\t',stdout);
				for (groups = entry->groups; groups
						; groups = groups->next) {
					fputs(groups->str,stdout);
					if (groups->next) {
						putc(',',stdout);
					}
				}
				putc('\n',stdout);
				entry = entry->next;
			}
		}
	}
}



		
/* 
 *	Add a netgroup to a user's list of netgroups 
 */
static void		
storegroup(group,glist,byuser)
	char *group;
	struct grouplist *glist;
	int byuser;
{
	char *name;	/* username or hostname */
	char *domain;
	char *key;
	static char *universal = "*";

	for (; glist; glist = glist->gl_nxt) {
		name = byuser ? glist->gl_name : glist->gl_machine;
		if (!name) {
			name = universal;
		} else if (!isalnum(*name) && *name != '_') {
			continue;
		}
		domain = glist->gl_domain;
		if (!domain) {
			domain = universal;
		}
		key = malloc((unsigned) (strlen(name)+strlen(domain)+2));
	 	(void) sprintf(key,"%s.%s",name,domain);
		enter(key,group);
	}
}



		
static groupentrylist
newentry(name,group)
	char *name;
	char *group;
{
	groupentrylist new;

	new = MALLOC(groupentrynode);

	STRCPY(new->name,name);

	new->groups = MALLOC(stringnode);
	new->groups->str = group;
	new->groups->next = NULL;

	new->next = NULL;
	return(new);
}



static void
appendgroup(grlist,group)
	groupentrylist grlist;
	char *group;
{
	stringlist cur,prev;

	for (cur = grlist->groups; cur; prev = cur,cur = cur->next) {
		if (strcmp(group,cur->str) == 0) {
			return;
		}
	}	
	prev->next = MALLOC(stringnode); 
	cur = prev->next;
	cur->str = group;
	cur->next = NULL;
}





static void
enter(name,group)
	char *name;
	char *group;
{
	int key;
	groupentrylist gel;
	groupentrylist gelprev;

	key = tablekey(name);
	if (grouptable[key] == NULL) {
		grouptable[key] = newentry(name,group);
	} else {
		gel = grouptable[key];
		while (gel && strcmp(gel->name,name)) { 
			gelprev = gel;
			gel = gel->next;
		}
		if (gel) {
			appendgroup(gel,group);
		} else {
			gelprev->next = newentry(name,group);
		}
	}
}
			
 
/*
 * Load up a hash table with the info in /etc/netgroup
 */
static void
loadtable(nf)
    FILE *nf;
{
    char buf[MAXGROUPLEN];
    char *p;
    char *group;
    char *line;
 
    while (getline(buf,MAXGROUPLEN,nf)) {
        for (p = buf; *p && *p != '#' && *p != ' ' && *p != '\t'; p++)
            ;
        if (*p == EOS || *p == '#')
            continue;
        *p++ = EOS;
 
        while (*p == ' ' || *p == '\t') {
            p++;
        }
        if (*p == EOS || *p == '#')
            continue;
    
        STRCPY(group,buf);
        STRCPY(line,p);
        store(ngtable,group,line);
    }
}
  






		
		
