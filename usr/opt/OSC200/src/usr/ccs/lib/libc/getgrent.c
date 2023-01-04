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
static char     *sccsid = "@(#)$RCSfile: getgrent.c,v $ $Revision: 4.2.3.4 $ (DEC) $Date: 1992/03/26 12:06:22 $";
#endif

/****************************************************************
 *								*
 *  Licensed to Digital Equipment Corporation, Maynard, MA	*
 *		Copyright 1985 Sun Microsystems, Inc.		*
 *			All rights reserved.			*
 *								*
 ****************************************************************/
/*
 * Modification History:
 * 
 * 27-Nov-91    terry
 *      Added OSF's getgrent_r, getgrnam_r, and getgrgid_r for libc_r.
 *
 * 29-May-91    terry
 *      Removed all references to BIND/Hesiod.  Added thread safety
 *      for setgrent() and endgrent() to offer OSF equivalent thread
 *      safety for the DEC OSF/1 release.
 * 
 * 13-Nov-89	sue
 *	Changed svc_getgrflag initial value to -2 and now perform a
 *	check in getgrent to see if the setgrent has been called yet.
 *
 * 24-Jul-89	logcher
 *	Removed generic setgrent and endgrent calls from generic
 *	getgrnam and getgrgid.  Added the specific set and end calls
 *	in the specific get routines.  Changed setgrent_yp to return
 *	NULL on failure, not exit 1.
 *
 * 16-May-89	logcher
 *	Modularized the code to have separate local, yp, bind/hesiod
 *	routines.  Added front end to check the /etc/svc.conf file
 *	for the service ordering.
 *
 */

#include <stdio.h>
#include <errno.h>
#include <grp.h>
#include <rpcsvc/ypclnt.h>
#include <netdb.h>
#include <sys/svcinfo.h>

#ifdef _THREAD_SAFE
#include "rec_mutex.h"

extern struct rec_mutex _group_rmutex;
#endif /* _THREAD_SAFE */

#if defined(lint) && !defined(DEBUG)
#define DEBUG
#endif /* defined(lint) && !defined(DEBUG) */

#define MAXINT 0x7fffffff
#define	MAXGRP	200

static char *domain;
static char GROUP[] = "/etc/group";
static struct group NULLGP = {NULL, NULL, 0, NULL};
static char *gr_mem[MAXGRP];
static FILE *grf = NULL;	/* pointer into /etc/group */

#ifndef _THREAD_SAFE
static char *yp;		/* pointer into yellow pages */
static int yplen;
static char *oldyp = NULL;	
static int oldyplen;
static struct list {
    char *name;
    struct list *nxt
} *minuslist;			/* list of - items */
static struct svcinfo *svcinfo;
int svc_getgrflag = -2;

static struct group *interpret();
static struct group *interpretwithsave();
static struct group *save();
static struct group *getnamefromyellow();
static struct group *getgidfromyellow();
char *yellowup();

/*
 * Declare all service routines
 */
int setgrent_local ();
int setgrent_yp ();
int endgrent_local ();
int endgrent_yp ();
struct group *getgrent_local ();
struct group *getgrent_yp ();
struct group *getgrgid_local (); 
struct group *getgrgid_yp (); 
struct group *getgrnam_local ();
struct group *getgrnam_yp ();

static int	(*setgrents []) ()={
		setgrent_local,
		setgrent_yp,
};
static int 	(*endgrents []) ()={
		endgrent_local,
		endgrent_yp,
};
static struct group * (*getgrents []) ()={
		getgrent_local,
		getgrent_yp,
};
static struct group * (*getgrgids []) ()={
		getgrgid_local,
		getgrgid_yp,
};
static struct group * (*getgrnames []) ()={
		getgrnam_local,
		getgrnam_yp,
};
#endif /* NOT _THREAD_SAFE */

/*
 * generic getgr service routines
 */

#ifdef _THREAD_SAFE
void
setgrent()
{
	rec_mutex_lock(&_group_rmutex);

	setgrent_local();

        rec_mutex_unlock(&_group_rmutex);
}
#else /* _THREAD_SAFE */

void
setgrent ()
{
	register i;

	svc_getgrflag = -1;
	if ((svcinfo = getsvc()) != NULL)
		for (i=0; (svc_lastlookup = svcinfo->svcpath[SVC_GROUP][i]) != SVC_LAST; i++)
			(*(setgrents [svcinfo->svcpath[SVC_GROUP][i]])) ();
}
#endif /* _THREAD_SAFE */

#ifdef _THREAD_SAFE
void
endgrent()
{
        rec_mutex_lock(&_group_rmutex);

	endgrent_local();

        rec_mutex_unlock(&_group_rmutex);
}
#else /* _THREAD_SAFE */

void
endgrent ()
{
	register i;

	svc_getgrflag = -1;
	if ((svcinfo = getsvc()) != NULL)
		for (i=0; (svc_lastlookup = svcinfo->svcpath[SVC_GROUP][i]) != SVC_LAST; i++)
			(*(endgrents [svcinfo->svcpath[SVC_GROUP][i]])) ();
}
#endif /* _THREAD_SAFE */

#ifndef _THREAD_SAFE
struct group *
getgrent()
{
	register struct group *p=NULL;
	register i;

	/*
	 * Check if setgrent was not made yet
	 */
	if (svc_getgrflag == -2)
		setgrent();
	/*
	 * Check if this is the first time through getgrent
	 */
	if (svc_getgrflag == -1) {
		/*
		 * If it is, init the svcinfo structure
		 */
		if ((svcinfo = getsvc()) == NULL)
			return((struct group *)NULL);
		i = 0;
	}
	else {
		/*
		 * If it is not, set the index to the last used one
		 */
		i = svc_getgrflag;
	}
	for (; (svc_lastlookup = svcinfo->svcpath[SVC_GROUP][i]) != SVC_LAST; i++)
		if (p = ((*(getgrents [svcinfo->svcpath[SVC_GROUP][i]])) () )) {
			svc_getgrflag = i;
			break;
		}
	return(p);
}

struct group *
getgrnam (name)
	register char *name;
{
	register struct group *p=NULL;
	register i;

	/* avoid null pointer de-reference on mips */
	if (name == 0)
		return(0);
	if ((svcinfo = getsvc()) != NULL)
		for (i=0; (svc_lastlookup = svcinfo->svcpath[SVC_GROUP][i]) != SVC_LAST; i++)
			if (p = ((*(getgrnames [svcinfo->svcpath[SVC_GROUP][i]])) (name) ))
				break;
	return(p);
}

struct group *
getgrgid (gid)
	register gid_t gid;
{
	register struct group *p=NULL;
	register i;

	if ((svcinfo = getsvc()) != NULL)
		for (i=0; (svc_lastlookup = svcinfo->svcpath[SVC_GROUP][i]) != SVC_LAST; i++)
			if (p = ((*(getgrgids [svcinfo->svcpath[SVC_GROUP][i]])) (gid) ))
				break;
	return(p);
}

/*
 * specific getgr service routines
 */

struct group *
getgrgid_local(gid)
	gid_t gid;
{
	struct group *gp;

#ifdef DEBUG
	fprintf(stderr, "getgrgid_local(%d)\n", gid);
#endif DEBUG
	setgrent_local();
	while (gp = getgrent_local()) {
		if (gp->gr_gid == gid)
			break;
	}
	endgrent_local();
	return(gp);
}


struct group *
getgrgid_yp(gid)
	gid_t gid;
{
	struct group *gp;
	char line[BUFSIZ+1];

#ifdef DEBUG
	fprintf(stderr, "getgrgid_yp(%d)\n", gid);
#endif DEBUG
	setgrent_yp();
	if (!grf)
		return(NULL);
	while (fgets(line, BUFSIZ, grf) != NULL) {
		gp = interpret(line, strlen(line));
		if (gp != NULL)		/* skip any bad group lines */
			if (matchgid(line, &gp, gid)) {
				endgrent_yp();
				return(gp);
			}
	}
	endgrent_yp();
	return(NULL);
}

struct group *
getgrnam_local(name)
register char *name;
{
	register struct group *gp;

#ifdef DEBUG
	fprintf(stderr, "getgrnam_local(%s)\n", name);
#endif DEBUG
	setgrent_local();
	while (gp = getgrent_local()) {
		if (strcmp(gp->gr_name, name) == 0)
			break;
	}
	endgrent_local();
	return(gp);
}

struct group *
getgrnam_yp(name)
	register char *name;
{
	struct group *gp;
	char line[BUFSIZ+1];

#ifdef DEBUG
	fprintf(stderr, "getgrnam_yp(%s)\n", name);
#endif DEBUG
	setgrent_yp();
	if (!grf)
		return(NULL);
	while (fgets(line, BUFSIZ, grf) != NULL) {
		gp = interpret(line, strlen(line));
		if (matchname(line, &gp, name)) {
			endgrent_yp();
			return(gp);
		}
	}
	endgrent_yp();
	return(NULL);
}

#endif /* NOT _THREAD_SAFE */

setgrent_local()
{
	if (!grf)
		grf = fopen(GROUP, "r");
	else
		rewind(grf);
}

#ifndef _THREAD_SAFE
setgrent_yp()
{
	setgrent_local();
	if ((domain = yellowup(1)) == NULL)
		return(NULL);
	if (yp)
		free(yp);
	yp = NULL;
	freeminuslist();
}
#endif /* NOT _THREAD_SAFE */

endgrent_local()
{
	if (grf) {
		fclose(grf);
		grf = NULL;
	}
}

#ifndef _THREAD_SAFE
endgrent_yp()
{
	endgrent_local();
	if (yp)
		free(yp);
	yp = NULL;
	freeminuslist();
}

struct group *
getgrent_local()
{
	char line1[BUFSIZ+1];
	static struct group *gp;
#ifdef DEBUG
	fprintf (stderr, "getgrent_local\n");
#endif DEBUG
	if (grf == NULL && (grf = fopen(GROUP, "r")) == NULL)
		return(NULL);
again_local:
	if ((fgets(line1, BUFSIZ, grf)) == NULL) {
		setgrent_local();
		return(NULL);
	}
	gp = interpret(line1, strlen(line1));
		switch(line1[0]) {
			case '+':
				goto again_local;
			case '-':
				goto again_local;
			default:
				return(gp);
		}
}


struct group *
getgrent_yp()
{
	char line1[BUFSIZ+1];
	static struct group *savegp, *gp;
#ifdef DEBUG
	fprintf (stderr,"getgrent_yp\n");
#endif DEBUG
	if (grf == NULL && (grf = fopen(GROUP, "r")) == NULL && (domain = yellowup(1)) == NULL)
		return(NULL);
again:
	if (yp) {
		gp = interpretwithsave(yp, yplen, savegp);
		free(yp);
		getnextfromyellow();
		if (onminuslist(gp))
			goto again;
		else
			return(gp);
	}
	else if (fgets(line1, BUFSIZ, grf) == NULL)
		return(NULL);
	gp = interpret(line1, strlen(line1));
		switch(line1[0]) {
			case '+':
				if (strcmp(gp->gr_name, "+") == 0) {
					getfirstfromyellow();
					savegp = save(gp);
					goto again;
				}
				savegp = save(gp);
				gp = getnamefromyellow(gp->gr_name+1, savegp);
				if (gp == NULL)
					goto again;
				else if (onminuslist(gp))
					goto again;
				else
					return(gp);
			case '-':
				addtominuslist(gp->gr_name+1);
				goto again;
			default:
				if (onminuslist(gp))
					goto again;
				return(gp);
		}
}

static char *
grskip(p,c)
	register char *p;
	register c;
{
	while(*p && *p != c && *p != '\n') ++p;
	if(*p) *p++ = 0;
	return(p);
}

static struct group *
interpret(val, len)
	char *val;
{
	register char *p, **q;
	static struct group gp;
	static char line[BUFSIZ+1];

	strncpy(line, val, len);
	p = line;
	line[len] = '\n';
	line[len+1] = 0;

	gp.gr_name = p;
	gp.gr_passwd = p = grskip(p,':');
	gp.gr_gid = atoi(p = grskip(p,':'));
	gp.gr_mem = gr_mem;
	p = grskip(p,':');
	grskip(p,'\n');
	q = gr_mem;
	while(*p){
		if (q < &gr_mem[MAXGRP-1])
			*q++ = p;
		p = grskip(p,',');
	}
	*q = NULL;
	return(&gp);
}

static
freeminuslist() {
	struct list *ls;
	
	for (ls = minuslist; ls != NULL; ls = ls->nxt) {
		free(ls->name);
		free(ls);
	}
	minuslist = NULL;
}

static struct group *
interpretwithsave(val, len, savegp)
	char *val;
	struct group *savegp;
{
	struct group *gp;
	
	gp = interpret(val, len);
	if (savegp->gr_passwd && *savegp->gr_passwd)
		gp->gr_passwd =  savegp->gr_passwd;
	if (savegp->gr_mem && *savegp->gr_mem)
		gp->gr_mem = savegp->gr_mem;
	return(gp);
}

static
onminuslist(gp)
	struct group *gp;
{
	struct list *ls;
	register char *nm;
	
	nm = gp->gr_name;
	for (ls = minuslist; ls != NULL; ls = ls->nxt)
		if (strcmp(ls->name, nm) == 0)
			return(1);
	return(0);
}

static
getnextfromyellow()
{
	int reason = 0;
	char *key = NULL;
	int keylen = 0;
	
	if (reason = yp_next(domain, "group.byname", oldyp, oldyplen, &key, &keylen, &yp, &yplen)) {
#ifdef DEBUG
		fprintf(stderr, "reason yp_next failed is %d\n", reason);
#endif
		yp = NULL;
	}
	if (oldyp)
		free(oldyp);
	oldyp = key;
	oldyplen = keylen;
}

static
getfirstfromyellow()
{
	int reason = 0;
	char *key = NULL;
	int keylen = 0;
	
	if (reason =  yp_first(domain, "group.byname", &key, &keylen, &yp, &yplen)) {
#ifdef DEBUG
		fprintf(stderr, "reason yp_first failed is %d\n", reason);
#endif
		yp = NULL;
	}
	if (oldyp)
		free(oldyp);
	oldyp = key;
	oldyplen = keylen;
}

static struct group *
getnamefromyellow(name, savegp)
	char *name;
	struct group *savegp;
{
	struct group *gp;
	int reason;
	char *val = NULL;
	int vallen = 0;
	
	if (reason = yp_match(domain, "group.byname", name, strlen(name), &val, &vallen)) {
#ifdef DEBUG
		fprintf(stderr, "reason yp_next failed is %d\n", reason);
#endif
		return(NULL);
	}
	else {
		gp = interpret(val, vallen);
		free(val);
		if (savegp->gr_passwd && *savegp->gr_passwd)
			gp->gr_passwd =  savegp->gr_passwd;
		if (savegp->gr_mem && *savegp->gr_mem)
			gp->gr_mem = savegp->gr_mem;
		return(gp);
	}
}

static
addtominuslist(name)
	char *name;
{
	struct list *ls;
	char *buf;
	
	ls = (struct list *)malloc(sizeof(struct list));
	buf = (char *)malloc(strlen(name) + 1);
	strcpy(buf, name);
	ls->name = buf;
	ls->nxt = minuslist;
	minuslist = ls;
}

/* 
 * save away psswd, gr_mem fields, which are the only
 * ones which can be specified in a local + entry to override the
 * value in the yellow pages
 */
static struct group *
save(gp)
	struct group *gp;
{
	static struct group *sv;
	char *gr_mem[MAXGRP];	
	char **av, **q;
	int lnth;
	
	/*
	* free up stuff from last time around
	*/
	if (sv) {
		for (av = sv->gr_mem; *av != NULL; av++) {
			if (q >= &gr_mem[MAXGRP-1])
				break;
			free(*q);
			q++;
		}
		free(sv->gr_passwd);
		free(sv->gr_mem);
		free(sv);
	}
	sv = (struct group *)malloc(sizeof(struct group));
	sv->gr_passwd = (char *)malloc(strlen(gp->gr_passwd) + 1);
	strcpy(sv->gr_passwd, gp->gr_passwd);

	q = gr_mem;
	for (av = gp->gr_mem; *av != NULL; av++) {
		if (q >= &gr_mem[MAXGRP-1])
			break;
		*q = (char *)malloc(strlen(*av) + 1);
		strcpy(*q, *av);
		q++;
	}
	*q = 0;
	lnth = (sizeof (char *)) * (q - gr_mem + 1);
	sv->gr_mem = (char **)malloc(lnth);
	bcopy(gr_mem, sv->gr_mem, lnth);
	return(sv);
}

static
matchname(line1, gpp, name)
	char line1[];
	struct group **gpp;
	char *name;
{
	struct group *savegp;
	struct group *gp = *gpp;

	switch(line1[0]) {
		case '+':
			if (strcmp(gp->gr_name, "+") == 0) {
				savegp = save(gp);
				gp = getnamefromyellow(name, savegp);
				if (gp) {
					*gpp = gp;
					return(1);
				}
				else
					return(0);
			}
			if (strcmp(gp->gr_name+1, name) == 0) {
				savegp = save(gp);
				gp = getnamefromyellow(gp->gr_name+1, savegp);
				if (gp) {
					*gpp = gp;
					return(1);
				}
				else
					return(0);
			}
			break;
		case '-':
			if (strcmp(gp->gr_name+1, name) == 0) {
				*gpp = NULL;
				return(1);
			}
			break;
		default:
			if (strcmp(gp->gr_name, name) == 0)
				return(1);
	}
	return(0);
}

static
matchgid(line1, gpp, gid)
	char line1[];
	struct group **gpp;
{
	struct group *savegp;
	struct group *gp = *gpp;

	switch(line1[0]) {
		case '+':
			if (strcmp(gp->gr_name, "+") == 0) {
				savegp = save(gp);
				gp = getgidfromyellow(gid, savegp);
				if (gp) {
					*gpp = gp;
					return(1);
				}
				else
					return(0);
			}
			savegp = save(gp);
			gp = getnamefromyellow(gp->gr_name+1, savegp);
			if (gp && gp->gr_gid == gid) {
				*gpp = gp;
				return(1);
			}
			else
				return(0);
		case '-':
			if (gid == gidof(gp->gr_name+1)) {
				*gpp = NULL;
				return(1);
			}
			break;
		default:
			if (gp->gr_gid == gid)
				return(1);
	}
	return(0);
}

static
gidof(name)
	char *name;
{
	struct group *gp;
	struct group nullgp;
	
	nullgp = NULLGP;
	gp = getnamefromyellow(name, &nullgp);
	if (gp)
		return(gp->gr_gid);
	else
		return(MAXINT);
}

static struct group *
getgidfromyellow(gid, savegp)
	int gid;
	struct group *savegp;
{
	struct group *gp;
	int reason = 0;
	char *val = NULL;
	int vallen = 0;
	char gidstr[20];
	
	sprintf(gidstr, "%d", gid);
	if (reason = yp_match(domain, "group.bygid", gidstr, strlen(gidstr), &val, &vallen)) {
#ifdef DEBUG
		fprintf(stderr, "reason yp_next failed is %d\n", reason);
#endif
		return(NULL);
	}
	else {
		gp = interpret(val, vallen);
		free(val);
		if (savegp->gr_passwd && *savegp->gr_passwd)
			gp->gr_passwd =  savegp->gr_passwd;
		if (savegp->gr_mem && *savegp->gr_mem)
			gp->gr_mem = savegp->gr_mem;
		return(gp);
	}
}
#endif /* NOT _THREAD_SAFE */
 
#ifdef _THREAD_SAFE
int
getgrent_r(struct group *group, char *line, int len)
{
	register char *p, **q;
	char	**gr_mem;
	char	**end;

	if ((group == NULL) || (line == NULL) || (len < 1)) {
		seterrno(EINVAL);
		return(-1);
	}

	rec_mutex_lock(&_group_rmutex);

	if (!grf)
		if ((grf = open_groupfile()) == NULL) {
			rec_mutex_unlock(&_group_rmutex);
			return(-1);
		}

	if ((p = fgets(line, len, grf)) == NULL) {
		rec_mutex_unlock(&_group_rmutex);
		seterrno(EIO);
		return(-1);
	}
	gr_mem = (char **)ALIGN(p + strlen(p) + sizeof(char *) - 1);
	end = (char **)ALIGN(p + len) - sizeof(char *);
	group->gr_name = p;
	group->gr_passwd = p = grskip(p,':');
	group->gr_gid = atoi(p = grskip(p,':'));
	group->gr_mem = gr_mem;
	p = grskip(p,':');
	grskip(p,'\n');
	q = gr_mem;
	while (*p) {
		if (q < end)
			*q++ = p;
		p = grskip(p,',');
	}
	*q = NULL;
	rec_mutex_unlock(&_group_rmutex);
	return(0);
}


int 
getgrgid_r(gid_t gid, struct group *group, char *line, int len)
{
	if ((group == NULL) || (line == NULL) || (len < 1)) {
		seterrno(EINVAL);
		return(-1);
	}

	rec_mutex_lock(&_group_rmutex);
	setgrent();
	while (getgrent_r(group, line, len) == 0)
	       if (group->gr_gid == gid) {
	            endgrent();
		    rec_mutex_unlock(&_group_rmutex);
		    return(0);
               }
	endgrent();
	rec_mutex_unlock(&_group_rmutex);
	seterrno(ENOENT);
	return(-1);
}

int
getgrnam_r(char *name, struct group *group, char *line, int len)
{
	if ((name == NULL) || (group == NULL) || (line == NULL) || (len < 1)) {
		seterrno(EINVAL);
		return(-1);
	}
	rec_mutex_lock(&_group_rmutex);
	setgrent();
	while (getgrent_r(group, line, len) == 0)
	    if (!strcmp(group->gr_name, name)) {
	        endgrent();
	        rec_mutex_unlock(&_group_rmutex);
        	return(0);
            }		  
	endgrent();
        rec_mutex_unlock(&_group_rmutex);
	seterrno(ENOENT);
	return(-1);
}

#endif /* _THREAD_SAFE */
