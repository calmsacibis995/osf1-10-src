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
static char	*sccsid = "@(#)$RCSfile: cm_db.c,v $ $Revision: 4.2.3.3 $ (DEC) $Date: 1992/03/19 13:57:31 $";
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

#include "cfgmgr.h"
#include "cm.h"
#include "cm_db_defs.h"
#include <sys/types.h>
#include <sys/param.h>
#include <pwd.h>
#include <grp.h>


/*
 *
 */
int
dbfile_open_dflt( AFILE_t * afd )
{
        return(dbfile_open(afd,CMGR.database,CMGR.maxrec,CMGR.maxatr));
}

/*
 *
 */
int
dbfile_open( AFILE_t * afd, char * filename, int maxrec, int maxatr )
{

	void module_close_db();
        if ((*afd = AFopen(filename, maxrec, maxatr)) == NULL)
                return(DBFILE_ENOENT);
	return(0);
}

/*
 *
 */
int
dbfile_rewind( AFILE_t afd )
{
	AFrewind(afd);
	return(0);
}

/*
 *
 */
int
dbfile_close( AFILE_t afd )
{
        if (afd != NULL)
                AFclose(afd);
	return(0);
}

/*
 *
 */
int
dbent_lookup( AFILE_t afd, char * name, ENT_t * entry )
{
	if ((*entry = AFgetent(afd, name)) == NULL)
		return(DBENT_ENOENT);
	return(0);
}

/*
 *
 */
int
dbent_next( AFILE_t afd, ENT_t * entry )
{
	if ((*entry = AFnxtent(afd)) == NULL)
		return(DBENT_ENOENT);
	return(0);
}

/*
 * 	Specialized attribute get
 */
char *
dbattr_value( ENT_t entry, char * field )
{
	return(AFgetval(AFgetatr(entry, field)));
}

/*
 * 	Specialized conversion from database value
 */
int
dbattr_match_type( ENT_t entry, char * fieldname, int fieldtype )
{
	extern	cvtlistlist_t cvtlists[];
	cvtlistlist_t *  listsp;
	cvtlist_t *	listp;
	cvtlist_t *	lp;
	ATTR_t		attr;
	char *		v;
	int		match;
	int		i;

	if ((attr=AFgetatr(entry, fieldname)) == NULL)
		return(SUB_GLOBAL_NONE);

	listp = NULL;
	for (listsp=cvtlists; listsp->type != 0; listsp++)
		if (listsp->type == fieldtype)
			listp = listsp->cvtlist;

	if (listp == NULL)
		return(SUB_GLOBAL_NONE);

	match = 0;
	for (v=AFnxtval(attr); v != NULL && *v != '\0'; v=AFnxtval(attr))
		for (lp=listp; *lp->value != '\0' ; lp++)
			if (!strcmp(lp->value, v))
			    	match |= lp->type;
	return (match);
}



/*
 * 	Specialized conversion from database value
int
dbattr_match_type( ENT_t entry, char * fieldname, int fieldtype )
{
	extern	cvtlistlist_t cvtlists[];
	cvtlistlist_t *  listsp;
	cvtlist_t *	listp;
	cvtlist_t *	lp;
	ATTR_t		attr;
	char *		v;
	int		match;
	int		i;

	if ((attr=AFgetatr(entry, fieldname)) == NULL)
		return(SUB_GLOBAL_NONE);

	listp = NULL;
	for (listsp=cvtlists; listsp->type != 0; listsp++)
		if (listsp->type == fieldtype)
			listp = listsp->cvtlist;

	if (listp == NULL)
		return(SUB_GLOBAL_NONE);

	match = 0;
	for (v=AFnxtval(attr); v != NULL && *v != '\0'; v=AFnxtval(attr))
		for (lp=listp; *lp->value != '\0' ; lp++)
			if (!strcmp(lp->value, v))
			    	match |= lp->type;
	return (match);
}
*/

/*
 * 	Convert UID or USERNAME string to pw_uid
 */
int
dbattr_user( ENT_t entry, char * fieldname, int dflt )
{
	ATTR_t		attr;
	struct passwd *	pw;
	char *		p;
	int		uid;
	int		error;

	if ((attr=AFgetatr(entry, fieldname)) == NULL)
		return(dflt);
						/* If numeric return as uid */
	uid = cm_aton((p=AFgetval(attr)), 0, &error);
	if (error >= 0)
		return(uid);
						/* If alpha use as pw_name */
	if ((pw=getpwnam(p)) == NULL)
		return(dflt);
	return(pw->pw_uid);
}

/*
 * 	Lookup attr string
 */
char *
dbattr_string( ENT_t entry, char * fieldname, char *  dflt )
{
	ATTR_t		attr;
	char *		p;
	int		error;

	if ((attr=AFgetatr(entry, fieldname)) == NULL)
		return(dflt);
	if ((p=AFgetval(attr)) == NULL)
		return(dflt);
	return(p);
}

/*
 * 	Convert GID or GROUPNAME string to gr_gid
 */
int
dbattr_group( ENT_t entry, char * fieldname, int dflt )
{
	ATTR_t		attr;
	struct group *	gr;
	char *		p;
	int		gid;
	int		error;

	if ((attr=AFgetatr(entry, fieldname)) == NULL)
		return(dflt);
						/* If numeric return as gid */
	gid = cm_aton((p=AFgetval(attr)), 0, &error);
	if (error >= 0)
		return(gid);
						/* If alpha use as gr_name */
	if ((gr=getgrnam(p)) == NULL)
		return(dflt);
	return(gr->gr_gid);
}

dev_t
dbattr_devno( ENT_t entry, char * fieldname, dev_t dflt )
{
	ATTR_t		attr;
	char *		p;
	int		major_num;
	int		error;

	if ((attr=AFgetatr(entry, fieldname)) != NULL) {
		if ((p=AFgetval(attr)) != NULL) {
			if (isdigit(*p)) {
				major_num = cm_aton(p, 0, &error);
				if (error >= 0)
					return(makedev(major_num,0));
			} else {
				if (!strcmp(p,"?")
				|| !strcmp(p, "any")
				|| !strcmp(p, "ANY"))
					return(NODEV);
			}
		}
	}
	return(dflt);
}


int
dbattr_mode( ENT_t entry, char * fieldname, int dflt )
{
	ATTR_t		attr;
	char *		p;
	int		num;
	int		error;

	if ((attr=AFgetatr(entry, fieldname)) == NULL)
		return(dflt);
	num = cm_aton((p=AFgetval(attr)), 8, &error);
	if (error >= 0)
		return(num);
	return(dflt);
}


int
dbattr_num( ENT_t entry, char * fieldname, int dflt )
{
	ATTR_t		attr;
	char *		p;
	int		num;
	int		error;

	if ((attr=AFgetatr(entry, fieldname)) == NULL)
		return(dflt);
	num = cm_aton((p=AFgetval(attr)), 0, &error);
	if (error >= 0)
		return(num);
	return(dflt);
}

int
dbattr_flag( ENT_t entry, char * fieldname, int dflt )
{
	ATTR_t		attr;
	char *		p;

	if ((attr=AFgetatr(entry, fieldname)) != NULL) {
		if ((p=AFgetval(attr)) != NULL) {
			if (strcmp(p,"True") || strcmp(p,"Yes"))
				return(TRUE);
			else if (strcmp(p,"False") || strcmp(p,"No"))
				return(FALSE);
		}
	}
	return(dflt);
}


/*
 * Digits outside the range implied by the current radix shouldn't
 * be accepted, but they are...
 */
#define CMVALUE(c)	(('0' <= (c) && (c) <= '9') ? (c) - '0' : \
			('A' <= (c) && (c) <= 'F') ? (c) - 'A' + 10 : \
			('a' <= (c) && (c) <= 'f') ? (c) - 'a' + 10 : \
			-1)

/*
 * Convert numeric string at p to an integer, using radix indicated,
 * and skipping leading whitespace.  If at least one digit is present,
 * status is set to the number of characters scanned; or, if radix is
 * outside [2, 16], to -1; else to zero.
 */
int
cm_aton(char * p, int radix, int * status)
{
	register int 	n, n1;
	register int 	issigned;
	char *		fp = p;
	char *		op;

	*status = 0;
	if (p == NULL) {
		*status = -1;
		return (0);
	}
	n = 0;
	issigned = 0;
	for (; ; ++p) {
		if (isspace(*p))
			continue;
		switch(*p) {
		case '-':
			issigned++; /* fall through */
		case '+':
			++p;
		}
		break;
	}
	if (radix == 0) {
		if (*p != '0') {
			radix = 10;
		} else if (*++p == 'X' || *p == 'x') {
			radix = 16;
			++p;
		} else if (CMVALUE(*p) != -1) {
			radix = 8;
		} else {
			--p;	/* '0' not followed by digit */
		}
	} else if (radix < 2 || 16 < radix) {
		*status = -1;
		return (0);
	}
	op = p;
	for ( ; (n1 = CMVALUE(*p)) != -1; ++p)
		n = n * radix + n1;
	/*
	 * Bug fix: if the first character is not a number set status to
	 * -1.  Without this status would be returned as 0 because p-fp=0.
	 * This would cause the caller of this routine to think that the
	 * string pointed to the number 0.
	 */
	if (op == p)
		*status = -1;
	if (op < p)
		*status = p - fp;
	return(issigned ? -n : n);
}
