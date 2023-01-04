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
static char	*sccsid = "@(#)$RCSfile: authcap.c,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1993/05/26 15:41:52 $";
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
/* Copyright (c) 1988-90 SecureWare, Inc.
 *   All rights reserved
 *
 * Authentication Database support routines.
 */


/* Based on:

 */

/*LINTLIBRARY*/

#include <sys/secdefines.h>

/* #if SEC_BASE */ /*{*/

#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <alloca.h>

#include <sys/security.h>
#include <sys/audit.h>
#include <sys/sec_objects.h>
#include <prot.h>

#define AUTH_MAX_CAPBUFSIZ AUTH_CAPBUFSIZ /* maximum size buffers can grow */
#define AUTH_INIT_CAPBUFSIZ 128		  /* initial size of buffers */

/* Default database pointers are kept in this file for packaging. */

static FILE *default_fp;
static long default_filepos;

/*
 * Root of the authentication database and templates for the files
 * containing the descriptions of the system files, commands, and
 * defaults
 */
#define	AUTHCAPDIR	"/tcb/files/auth"
#define	TEMPLATE	"/etc/auth/%s/%s"
#define	FILES		"files"
#define	TTYS		"ttys"
#define	DEFAULT		"default"
#define DEVASG		"devassign"
#if SEC_MAC || SEC_ILB
#define LPS		"lps"
#endif
#define SUBSYS		"subsystems"

/*
 * authcap - routines for dealing with the authentication capability data base
 *
 * BUG:		Should use a "last" pointer in abuf, so that searching
 *		for capabilities alphabetically would not be a n**2/2
 *		process when large numbers of capabilities are given.
 *
 * Essentially all the work here is scanning and decoding escapes
 * in string capabilities.
 */

static	char *abuf;
static char *default_name = "system";

/* pointers to heap space buffers allocated during processing */

static char *user_buf = (char *) 0;
static int  user_len = 0;
static char *file_buf = (char *) 0;
static int   file_len = 0;
static char *tty_buf = (char *) 0;
static int   tty_len = 0;
static char *default_buf = (char *) 0;
static int   default_len = 0;
static char *devasg_buf = (char *) 0;
static int   devasg_len = 0;
#if SEC_MAC
static char *lp_buf = (char *) 0;
static int   lp_len = 0;
#endif

static int match_name();
static int match_file();
static char	*askip();
static char	*askipto();
static char	*adecode();
static	int agetent();
long adecodenum();


/*
 * Get an entry for user name from the user's authentication file.
 */
int
agetuser(user)
	char *user;
{
	int ent_ret;
	FILE *fp;

	open_auth_file(user, OT_PRPWD, &fp);
	if (fp == (FILE *) 0)
		ent_ret = -1;
	else {
		ent_ret = agetent(&user_buf, &user_len, user, OT_PRPWD,
				  (long *) 0, fp);
		(void) fclose(fp);
	}

	return ent_ret;
}


/*
 * Get an entry for file name.
 */
char *
agetfile(filepos, fp, nam)
	long *filepos;
	FILE *fp;
	char *nam;
{
	if (agetent(&file_buf, &file_len, nam, OT_FILE_CNTL, filepos, fp) == 1)
		return file_buf;
	return (char *) 0;
}


/*
 * Get an entry for tty name.  Assumes tty resides in /dev .
 */
int
agettty(filepos, fp, name)
	long *filepos;
	FILE *fp;
	char *name;
{
	return agetent(&tty_buf, &tty_len, name, OT_TERM_CNTL, filepos, fp);
}


/*
 * Get an entry for system default.
 */
int
agetdefault()
{
	if (default_fp == (FILE *) 0)
		setprdfent();

	return agetent(&default_buf, &default_len, (char *) 0, OT_DFLT_CNTL,
			&default_filepos, default_fp);
}


/*
 * Get an entry for system default, and return the buffer with the entry
 */
char *
agetdefault_buf()
{
	if (agetdefault() == 1)
		return (default_buf);
	return ((char *) 0);
}


/*
 * Get an entry for device name.
 */
char *
agetdvag(filepos, fp, nam)
	long *filepos;
	FILE *fp;
	char *nam;
{
	if (agetent(&devasg_buf, &devasg_len, nam, OT_DEV_ASG,
			filepos, fp) == 1)
		return (devasg_buf);

	return ((char *) 0);
}


#if SEC_MAC
/*
 * Get an entry for lp name.
 */
int
agetlp(filepos, fp, name)
	long *filepos;
	FILE *fp;
	char *name;
{
	return agetent(&lp_buf, &lp_len, name, OT_LP_CNTL, filepos, fp);
}
#endif


/*
 * Get a database entry from one of the authcap-format files.
 * Routine understands escapes and removes newlines
 * Returns -1 if the file it needs does not exist, 0 if the entry cannot be
 * found and 1 if the entry was found.
 */
static int
agetent(bpp, blenp, name, type, pos, fp)
	char **bpp;
	int  *blenp;
	char *name;
	int type;
	long *pos;
	FILE *fp;
{
	register char *bp = *bpp;
	register int cnt = 0;
	register char *cp;
	register long entry_end;
	register int buf_length = *blenp;
	int c;
	int found_entry;
	char ibuf[BUFSIZ];
	int new_size;
	unsigned long cp_offset;
	char *nbp;


	if (pos == (long *) 0)
		entry_end = 0L;
	else
		entry_end = *pos;

	if (fp == (FILE *) 0)
		return (-1);

	if (ftell (fp) != entry_end)
		fseek (fp, entry_end, 0);

	if (bp == (char *) 0) {
		bp = malloc (AUTH_INIT_CAPBUFSIZ);
		if (bp == (char *) 0) {
			entry_end = lseek (fileno(fp), 0L, 2);
			return (0);
		}
		*bpp = bp;
		*blenp = AUTH_INIT_CAPBUFSIZ;
		buf_length = AUTH_INIT_CAPBUFSIZ;
	}

	abuf = bp;

	for (;;)  {

		cp = bp;
		*cp = '\0';

		/* gather up the entry into the buffer. */
		for (;;) {
			if (fgets (ibuf, sizeof (ibuf), fp) == NULL)
				return (0);
			cnt = strlen(ibuf) ;
			if (cnt <= 0)
				return(0) ;
			entry_end += cnt;
			if (ibuf[0] == '#')
				continue;
			if (ibuf[cnt - 1] == '\n') {
				cnt -= 1;	/* drop newline */
				c = ibuf[cnt - 1];
				if (c == '\\')
					cnt--;
			}
			else {
				c = '\\';	/* for continuation check */
			}

			/* grow the buffer if it's too big, up to max allowed */
			new_size = cnt + strlen (bp) + 1;
			cp_offset = (unsigned long) cp - (unsigned long) bp;
			if (new_size > AUTH_MAX_CAPBUFSIZ) {
				entry_end = lseek (fileno(fp), 0L, 2);
				break;
			} else if (new_size > buf_length) {
				nbp = realloc (bp, new_size);
				if (nbp == (char *) 0) {
					entry_end = lseek (fileno(fp), 0L, 2);
					break;
				}
				/* reset to point to the new buffer. */
				*bpp = nbp;
				bp = nbp;
				abuf = bp;
				buf_length = new_size;
				*blenp = buf_length;
				cp = bp + cp_offset;
			}
			strncpy (cp, ibuf, cnt);
			cp += cnt;
			*cp = '\0';
			
			if (c != '\\')
				break;
		}

		/*
		 * The real work for the match.
		 */
		if ( name == (char *) 0 ||
		    (type == OT_PRPWD && match_name(name)) ||
		    (type == OT_FILE_CNTL && match_file(name)) ||
		    (type == OT_TERM_CNTL && match_name(name)) ||
#if SEC_MAC
		    (type == OT_LP_CNTL &&   match_name (name))  ||
#endif
		    (type == OT_DEV_ASG &&   match_name (name))) {

			/*
			 * As a sanity check when reading an entry,
			 * make sure the entry has a AUTH_CHKENT flag in
			 * it.  We place AUTH_CHKENT at the end when
			 * creating the entry s.t. when reading the
			 * entry later, we know the entry is complete.
			 */
			if (agetflag(AUTH_CHKENT) == 1)  {
				found_entry = 1;
				if (pos != (long *) 0)
					*pos = entry_end;
			}
			else  {
				if (pos != (long *) 0)
					*pos = lseek (fileno(fp), 0L, 2);
				found_entry = 0;
			}

			/* garbage-collect for Doug Gwyn's alloca() */
			if (name) (void) alloca(0);

			return found_entry;
		}
		if (name) (void) alloca(0);
	}
}

/*
 * Return a NULL-terminated list of pointers to the parsed list of names
 * from the current entry in abuf.  This is suitable for handing back to
 * aptentnames later.  Storage is a single malloc() block.  It is the caller's
 * responsibility to free() it.
 */
char **
agetnmlist()
{
	register char *bp, *bpe, *np, **rp;
	char **ret;
	char *fap;
	register size_t slen;
	register int count;

	bp = abuf;
	if (!bp || !*bp || *bp == '#')
		return (char **)0;

	/* First, find the end of the name field */
	bpe = askip(bp);
	/* Start with string length, then add overhead for each field */
	slen = bpe-bp+1;
	count = 2;
	while ((bp = askipto(bp, '|')) && (bp <= bpe)) {
		count++;
	}
	slen += count*sizeof(char *);
	ret = (char **)calloc((size_t)1,slen);
	if (!ret)
		return ret;
	rp = ret;
	fap = (char *)(ret + count);
	for (np = abuf;  np < bpe;  np = bp) {
		bp = askipto(np, '|');
		if (bp > bpe)
			bp = bpe;
		*rp++ = fap;		/* next name pointer */
		if (bp-np < 2) {
			*fap++ = '\0';	/* null name */
		}
		else {
			if (bp[-1] == '|') {
				*--bp = '\0';
				(void) adecode(np, &fap);
				*bp++ = '|';
			}
			else {
				(void) adecode(np, &fap);
			}
		}
	}
	return ret;
}

/*
 * Match_alternate deals with matching against the first field of the
 * authcap entry.  It is a sequence of names or filespecs separated by |'s,
 * so we compare against each such alternate.  The normal : terminator after
 * the last name (before the first field) stops us.
 */
static int
match_alternate(np, predp)
	char *np;
	int (*predp)();
{
	register char *Np, *Bp;
	register char *eptr;
	register char *farea;
	char *fap;

	Bp = abuf;
	if (*Bp == '#')
		return 0;

	/* First, find the end of the name field */
	eptr = askip(Bp);

	/* Make buffer space for decoding the alternates */
	farea = alloca(eptr - Bp);
	if (!farea)	/* out of memory */
		return 0;

	/* Now loop over attempts to match the input name against the choices */
	for (Np = Bp;  Np < eptr;  Np = Bp) {
		Bp = askipto(Np, '|');
		if (Bp > eptr)
			Bp = eptr;	/* terminate at end of name field */
		if ((Bp - Np) < 2)
			continue;	/* null alternate */
		(void) strncpy(farea, Np, Bp-Np-1);
		farea[Bp-Np-1] = '\0';	/* ensure a proper string */
		fap = farea;
		(void) adecode(farea, &fap); /* interpret any \-escapes */
		if ((*predp)(np, farea))
			return 1;	/* found a match */
	}
	return 0;
}

/*
 * Match_name deals with name matching.  The first field of the authcap
 * entry is a sequence of names separated by |'s, so we compare
 * against each such name.  The normal : terminator after the last
 * name (before the first field) stops us.
 */
static int
match_name_p(np, ap)
	register char *np, *ap;
{
	return strcmp(np, ap) == 0;
}

static int
match_name(np)
	register char *np;
{
	return match_alternate(np, match_name_p);
}

/*
 * ackentname - check against alternate buffer (optional) for match with name.
 */
int
ackentname(name, bp)
	register char *name, *bp;
{
	if (bp)
		abuf = bp;
	return !!abuf && !!*abuf && match_name(name);
}

/*
 * A name will match its exact equivalent in the database or it will
 * match a last wild card component.  Thus /etc/abc matches database
 * entries /etc/abc and /etc/* .  Note that the wild card entries must
 * appear in the database after all the real names for which it is
 * equivalent.
 */
static int
match_file_p(np, ap)
	register char *np, *ap;
{
	register char *Np, *Ap; 

	if (strcmp(np, ap) == 0)
		return 1;	/* strict match found */

	Np = strrchr(np, '/');
	Ap = strrchr(ap, '/');
	if (Np)
		Np++;
	else
		Np = np;
	if (Ap)
		Ap++;
	else
		Ap = ap;
	if (((Ap-ap) != (Np-np)) || ((Np-np) && strncmp(np, ap, Np-np)))
		return 0;	/* directory prefixes don't match */
	
	if (*Np && (*Ap == '*') && !Ap[1])
		return 1;	/* wildcard in entry & name tail not null */
	
	return 0;
}

static int
match_file(np)
	register char *np;
{
	return match_alternate(np, match_file_p);
}

/*
 * ackentfile - check against alternate buffer (optional) for match with file.
 */
int
ackentfile(name, bp)
	register char *name, *bp;
{
	if (bp)
		abuf = bp;
	return !!abuf && !!*abuf && match_file(name);
}

/*
 * Skip to the next unquoted delimiter.  This understands \-quoting.
 */
static char *
askipto(bp, delim)
	register char *bp;
	register char delim;
{
	register char *Bp = bp;
	register char *cp;
	register int quoted;

	while ((bp = strchr(bp, delim)) != NULL) {
		for (quoted=0, cp=bp;  cp-- > Bp; ) {
			if (*cp != '\\') break;
			quoted = !quoted;
		}
		if (!quoted) break;
		bp++;
	}
	if (bp && (*bp == delim))
		bp++;
	else
		bp = Bp + strlen(Bp);
	return bp;
}

/*
 * Skip to the next field.  This now understands \: escaping.
 */
static char *
askip(bp)
	register char *bp;
{
	return askipto(bp, ':');
}

/*
 * External interface to askip.
 */
char *
agetnextfield(bp)
	register char *bp;
{
	return askip(bp);
}

/*
 * Return pointer to start of next entry, along with the length of the
 * identifier tag.  (The idlen parameter is ref size_t.)
 */
char *
agetnextident(bp, idlen)
	register char *bp;
	register size_t *idlen;
{
	bp = askip(bp);
	if (!bp || !*bp) {
		*idlen = 0;
		return NULL;
	}
	*idlen = strcspn(bp, "@#=:");
	return bp;
}

/*
 * Find the next possible ident match for 'id' starting after 'bp'.
 * If 'bp' is NULL, start from beginning of abuf.
 * Return NULL if no match, or pointer to the argtype character after the
 * ident string (i.e., one of "@#=:").
 */
static char *
agetident(id, bp)
	register char *id;
	register char *bp;
{
	register int idlen;

	idlen = strlen(id);
	if (!bp) bp = abuf;
	for (;;) {
		bp = askip(bp);
		if (!*bp)
			return (char *) (0);
		if (strncmp(id, bp, idlen) == 0)  {
			bp += idlen;
			if (strchr("@#=:", *bp) != NULL)
				return bp;
		}
	}
}
/*
 * Return a time value.
 * If the value is "now", the first time the database is read the time is
 * taken from the system time.
 */

int
agttime(id,val)
	char *id;
	int *val;
{
	register char *bp = 0;

	check_auth_parameters();

	for (bp = agetident(id, bp);  bp;  bp = agetident(id, bp)) {
		if (*bp == '@')
			return(-1);
		if (*bp != '#')
			continue;
		bp++;
		if (strncmp(bp, AUTH_TIME_NOW, strlen(AUTH_TIME_NOW)) == 0)
			*val=time((time_t *) 0);
		else
			*val=adecodenum(bp);
		return(0);
	}
	return -1;
}


/*
 * Return the (numeric) option id in val.
 * Note that this may be -1.
 * Returns 0 if field is present, else -1.
 * Numeric options look like
 *	mt#5
 * i.e. the option string is separated from the numeric value by
 * a # character.  If the option is not found we return -1.
 * Note that we handle octal numbers beginning with 0 and hex
 * digits beginning with 0x.
 */
int
agtnum(id,val)
	char *id;
	long *val;
{
	register char *bp = 0;

	check_auth_parameters();

	for (bp = agetident(id, bp);  bp;  bp = agetident(id, bp)) {
		if (*bp == '@')
			return(-1);
		if (*bp != '#')
			continue;
		bp++;
		*val=adecodenum(bp);
		return(0);
	}
	return -1;
}

/*
 * Decode a number according to C conventions.
 */
/*
 * There was once a long routine here, but it wasn't right and it was a
 * re-invention of the wheel.  Let strtol() do the work for us.
 */
long
adecodenum(bp)
	register char	*bp;
{
	char *ep;
	register long	i ;

	i = strtol(bp, &ep, 0);
	if (ep==bp)		/* not a number */
		return -1;
	else
		return i;
}

/*
 * Core of getting a {user|group}-id value.
 */
static int
agetxid(id, convfunc)
	char *id;
	int (*convfunc)();
{
	register char *bp;
	register int ret_id;
	char *tbuf = 0;

	check_auth_parameters();

	bp = agetstr(id, &tbuf);
	if (!bp)
		return -1;
	ret_id = (*convfunc)(bp);
	free(bp);
	return ret_id;
}

/*
 * Get a user ID valued field.  Look up the user identifier in
 * the password database (/etc/passwd) and return the user ID
 */

agetuid(id)
	char *id;
{
	extern int pw_nametoid();

	return agetxid(id, pw_nametoid);
}

/*
 * Get a group ID valued field.  Look up the group identifier in
 * the group database (/etc/group) and return the group ID
 */

agetgid(id)
	char *id;
{
	extern int gr_nametoid();

	return agetxid(id, gr_nametoid);
}

/*
 * Handle a flag option.
 * Flag options are given "naked", i.e. followed by a : or the end
 * of the buffer.  Return 1 if we find the option, 0 if it is present
 * and designated false, or -1 if it is not given.
 */
int
agetflag(id)
	char *id;
{
	register char *bp = 0;

	check_auth_parameters();

	for (bp = agetident(id, bp);  bp;  bp = agetident(id, bp)) {
		if (!*bp || *bp == ':')
			return (1);
		else if (*bp == '@')
			return(0);
	}
	return -1;
}

/*
 * Get a string valued option.
 * These are given as
 *	cp=ps,whodo
 * Much decoding is done on the strings, and the strings are
 * placed in area, which is a ref parameter which is updated.
 * No checking on area overflow.
 * If area or *area is NULL on input, it will be malloc'ed.  It is then
 * the caller's responsibility to free() the storage later.
 */
char *
agetstr(id, area)
	char *id;
	char **area;
{
	register char *bp = 0;

	check_auth_parameters();

	for (bp = agetident(id, bp);  bp;  bp = agetident(id, bp)) {
		if (*bp == '@')
			return((char *) 0);
		if (*bp != '=')
			continue;
		bp++;
		return (adecode(bp, area));
	}
	return ((char *)0);
}

/*
 * Adecode does the grunt work to decode the
 * string capability escapes.
 */
static char *
adecode(str, area)
	register char *str;
	char **area;
{
	register char *cp;
	register int c;
	register char *dp;
	int i;
	char *foo;

	if (!area) {
		foo = 0;
		area = &foo;
	}

	cp = *area;
	if (!cp) {
		cp = askip(str);
		if (!cp) return (char *)0;
		cp = (char *)malloc(cp-str+1);
		if (!cp) return (char *)0;
		*area = cp;
	}
	while ((c = *str++) && c != ':') {
		switch (c) {

		case '^':
			c = *str++ & 037;
			break;

		case '\\':
			dp = "E\033^^\\\\::n\nr\rt\tb\bf\fa\7v\v";
			c = *str++;
nextc:
			if (*dp++ == c) {
				c = *dp++;
				break;
			}
			dp++;
			if (*dp)
				goto nextc;
			if (isdigit(c)) {
				c -= '0', i = 2;
				if (isdigit(*str))
				do
					c <<= 3, c |= *str++ - '0';
				while (--i && isdigit(*str));
			}
			break;
		}
		*cp++ = c;
	}
	*cp++ = 0;
	str = *area;
	*area = cp;
	return (str);
}

/*
 * Return a malloc'ed copy of the decoded string starting at the specified
 * place.  Defaults to abuf if passed as NULL.  It is the caller's
 * responsibility to free the string.
 */
char *
adecodestr(bp)
	register char *bp;
{
	if (!bp)
		bp = abuf;
	return adecode(bp, (char **)0);
}


/* parse a comma-separated string list */

char **
agetstrlist (field)
char *field;
{
	register char *cp = field;
	char **ret;
	int count;
	int i;

	if (*cp == '\0')
		return (char **) 0;
	/* count the number of fields */
	for (count = 2; *cp && (cp = strchr (cp, ',')); cp++)
		count++;
	ret = (char **) calloc (1, count * sizeof (char *) + strlen(field)+1);
	/* copy the string */
	cp = (char *)(ret+count);
	(void) strcpy(cp, field);
	/* set the values of the array */
	for (i = 0; i < count - 1; i++) {
		ret[i] = cp;
		cp = strchr (cp, ',');
		if (cp)
			*cp++ = '\0';
	}
	ret[i] = (char *) 0;
	return (ret);
}


/*
 * Return the file descriptor for the Authentication database file
 * containing the entry.
 */
void
open_auth_file(name, type, fpp)
	char *name;
	int type;
	FILE **fpp;
{
	register char *pathname;
	register FILE *fp = (FILE *) 0;
	char *tpath, *opath;
	int i;
	struct stat sb;

	pathname = find_auth_file(name, type);
	make_transition_files(pathname, &tpath, &opath);

	if (pathname == (char *) 0)
		fp = (FILE *) 0;
	else  {
		int changed_i = 0;
		time_t orig_mtime = 0;

		for (i = 0; i < AUTH_LOCK_ATTEMPTS; i++) {

			/*
			 * If file exists, check conditions for forced removal
			 */

			if (stat(tpath, &sb) == 0) {
				if (i - changed_i == AUTH_LOCK_ATTEMPTS / 2 &&
				    sb.st_mtime == orig_mtime)
					unlink(tpath);
				/*
				 * If original time not set or changed, reset it
				 */

				if (!orig_mtime || sb.st_mtime != orig_mtime) {
					orig_mtime = sb.st_mtime;
					changed_i = i;
				}
				(void) sleep (AUTH_RETRY_DELAY);
			}
			else {
				fp = fopen(pathname, "r");
				if (fp != (FILE *) 0)
					break;
			}
		}

		/*
		 * To be on the safe side, make sure the database files
		 * are closed across an exec().  This is a safety valve --
		 * such files should be explicitly closed with endpr??ent().
		 */
		if (fp != (FILE *) 0)
			(void) fcntl(fileno (fp), F_SETFD, 1);
		free(pathname);
		free(tpath);
		free(opath);
	}

	*fpp = fp;
}


/*
 * Locate the file associated with the entry name and the database type.
 * return the malloc'd string of the file name.  Depending on the database,
 * the file will contain just the single entry or it will contain the entire
 * database.
 */
char *
find_auth_file(name, type)
	register char *name;
	int type;
{
	register char *pathname;
	register int pathlen;
	char subdirname[sizeof("a/")];

	switch(type)  {
		case OT_PRPWD:
			*subdirname = *name;
			subdirname[1] = '/';
			subdirname[2] = '\0';
			pathlen = strlen(AUTHCAPDIR) + 3 + strlen(subdirname) +
				  strlen(name);
			pathname = malloc(pathlen);

			if (pathname != (char *) 0) {
				(void) strcpy(pathname, AUTHCAPDIR);
				(void) strcat(pathname, "/");
				(void) strcat(pathname, subdirname);
				(void) strcat(pathname, name);
			}
			break;

		case OT_FILE_CNTL:
			pathlen = strlen(TEMPLATE) +
				  strlen(default_name) +
				  strlen(FILES) + 1;
			pathname = malloc(pathlen);

			if (pathname != (char *) 0) 
				(void) sprintf(pathname, TEMPLATE,
					       default_name, FILES);
			break;
		
		case OT_SUBSYS:
			pathlen = sizeof(AUTH_SUBSYSDIR) + 1 + strlen(name);
			pathname = malloc(pathlen);

			if (pathname != (char *) 0)
				(void) sprintf(pathname, "%s/%s",
						AUTH_SUBSYSDIR, name);
			break;

		case OT_TERM_CNTL:
			pathlen = strlen(TEMPLATE) +
				  strlen(default_name) +
				  strlen(TTYS) + 1;
			pathname = malloc(pathlen);

			if (pathname != (char *) 0)
				(void) sprintf(pathname, TEMPLATE,
					       default_name, TTYS);
			break;

		case OT_DFLT_CNTL:
			pathlen = strlen(TEMPLATE) +
				  strlen(default_name) +
				  strlen(DEFAULT) + 1;
			pathname = malloc(pathlen);

			if (pathname != (char *) 0)
				(void) sprintf(pathname, TEMPLATE,
					       default_name,
					       DEFAULT);
			break;
					
		case OT_DEV_ASG:
			pathlen = strlen(TEMPLATE) +
				  strlen(default_name) +
				  strlen(DEVASG) + 1;
			pathname = malloc(pathlen);

			if (pathname != (char *) 0)
				(void) sprintf(pathname, TEMPLATE,
					       default_name,
					       DEVASG);
			break;

#if SEC_MAC
		case OT_LP_CNTL:
			pathlen = strlen(TEMPLATE) +
				  strlen(default_name) +
				  strlen(LPS) + 1;
			pathname = malloc(pathlen);

			if (pathname != (char *) 0)
				(void) sprintf(pathname, TEMPLATE,
					       default_name, LPS);
			break;
#endif

		default:
			pathname = (char *) 0;
	}

	return pathname;
}


/* end of processing for the databases.
 * free the static buffers, and the "ac" buf if allocated.
 */

void
end_authcap (type)
int type;
{
	switch (type) {
	case OT_PRPWD:
		if (user_buf != (char *) 0) {
			free (user_buf);
			user_buf = (char *) 0;
			user_len = 0;
		}
		break;
	case OT_FILE_CNTL:
		if (file_buf != (char *) 0) {
			free (file_buf);
			file_buf = (char *) 0;
			file_len = 0;
		}
		break;
	case OT_TERM_CNTL:
		if (tty_buf != (char *) 0) {
			free (tty_buf);
			tty_buf = (char *) 0;
			tty_len = 0;
		}
		break;
	case OT_DFLT_CNTL:
		if (default_buf != (char *) 0) {
			free (default_buf);
			default_buf = (char *) 0;
			default_len = 0;
		}
		break;
	case OT_DEV_ASG:
		if (devasg_buf != (char *) 0) {
			free (devasg_buf);
			devasg_buf = (char *) 0;
			devasg_len = 0;
		}
		break;
#if SEC_MAC
	case OT_LP_CNTL:
		if (lp_buf != (char *) 0) {
			free (lp_buf);
			lp_buf = (char *) 0;
			lp_len = 0;
		}
		break;
#endif
	}
}

/*
 * the default database routines are included here because otherwise
 * the entire Authentication database support software would be brought
 * in every time any user referenced any of the databases.
 */

/* This structure is visible to getprdfent.c for packaging reasons */
struct pr_default *pr_default = (struct pr_default *) 0;

/*
 * Close the file(s) related the to the System Default database.
 */
void
endprdfent()
{
	check_auth_parameters();

	if (default_fp != (FILE *) 0)  {
		(void) fclose(default_fp);
		default_fp = (FILE *) 0;
	}
	default_filepos = 0L;
	end_authcap (OT_DFLT_CNTL);
}


/*
 * Reset the position of the System Default database so that the
 * next time getprdfent() is invoked, it will return the first entry
 * in the database.
 */
void
setprdfent()
{
	static time_t modify_time;
	struct stat sb;
	char *filename;
	int ret;

	if (default_fp == (FILE *) 0) {
		open_auth_file((char *) 0, OT_DFLT_CNTL, &default_fp);
		if (default_fp != (FILE *) 0) {
			fstat (fileno(default_fp), &sb);
			modify_time = sb.st_mtime;
		}
	} else {
		filename = find_auth_file ((char *) 0, OT_DFLT_CNTL);
		ret = stat (filename, &sb);
		if (ret != 0 || sb.st_mtime > modify_time) {
			(void) fclose (default_fp);
			open_auth_file((char *) 0, OT_DFLT_CNTL, &default_fp);
			if (default_fp != (FILE *) 0) {
				fstat (fileno(default_fp), &sb);
				modify_time = sb.st_mtime;
			}
		}
		free (filename);
	}
	default_filepos = 0L;
	if (pr_default == (struct pr_default *) 0) {
		pr_default = (struct pr_default *)
		  malloc (sizeof (*pr_default));
		if (pr_default == (struct pr_default *) 0) {
			endprdfent();
		}
	}
}

/*
 * When writing the default database, need to resynchronize with the
 * current version of the database in case it has been changed since
 * the last time the program has accessed it.
 */

reset_default(pathname)
char *pathname;
{
	if (default_fp != (FILE *) 0)
		(void) fclose (default_fp);
	default_fp = fopen (pathname, "r");
	default_filepos = 0L;
}

/*
 * Aencode allocates a string buffer to hold a \-escaped form of the input
 * string.  Returns 1 on success and 0 on failure.  Freeing the allocated
 * storage is the caller's respnsibility.
 */
int
aencode(cbuf, str)
	char **cbuf;
	char *str;
{
	register u_char *sp;
	register u_char *cp;
	register u_char *xp;
	static u_char trbuf[] = "\033E\7a\bb\ff\nn\rr\tt\vv::^^||\\\\";

	if (!cbuf || !str)
		return 0;
	*cbuf = (char *)malloc(4*strlen(str)+1);	/* allow for \ooo */
	cp = (u_char *)*cbuf;
	if (!cp)
		return 0;

	for (sp = (u_char *)str;  *sp;  sp++) {
		xp = (u_char *)strchr((char *)trbuf, *sp);
		if (xp && !((xp-trbuf)&1)) {
			*cp++ = '\\';
			*cp++ = *++xp;
		}
		else if (*sp > '\0' && *sp <= '\037') {
			*cp++ = '^';
			*cp++ = 0100|*sp;
		}
		else if (*sp >= '!' && *sp <= '~') {
			*cp++ = *sp;
		}
		else {
			(void) sprintf((char *)cp, "\\%03o", *sp);
			cp += 4;
		}
	}
	*cp = '\0';
	return 1;
}

/*
 * The following aptent* (authcap put entry *) routines accept a stdio
 * stream and the address of the flag for accumulated error status.
 * Except for aptentfin (finish), they also take a current field count
 * (passed into and back from pr_newline), and the field (or entry) name or
 * namelist.  Most also require the entry value as the final parameter.
 */

/*
 * Put out a string list with the specified separator.
 */
static int
aptstrlist(fp, errflgp, fields, names, sep)
	FILE *fp;
	register int *errflgp, fields;
	char **names, *sep;
{
	register char **np = names;
	char *tbuf;

	while (*np && !*errflgp) {
		*errflgp = !aencode(&tbuf, *np);
		if (!*errflgp)
			*errflgp = (0 > fprintf(fp, "%s%s", tbuf,
						*++np ? sep : ":"));
		if (tbuf) free(tbuf);
	}
	return pr_newline(fp, fields, errflgp);
}

/*
 * Initialize the error flag and start writing an entry on stream fp
 * which is known by any of the alternatives ``names''.
 */
int
aptentnames(fp, errflgp, fields, names)
	FILE *fp;
	register int *errflgp, fields;
	char **names;
{
	*errflgp = (fflush(fp) != 0) || !names;

	return aptstrlist(fp, errflgp, fields, names, "|");
}

/*
 * Put out the string-list valued entry named name with values list.
 */
int
aptentslist(fp, errflgp, fields, name, list)
	FILE *fp;
	register int *errflgp, fields;
	char *name, **list;
{
	if (!*errflgp)
		*errflgp = (0 > fprintf(fp, "%s=", name));
	return aptstrlist(fp, errflgp, fields, list, ",");
}

/*
 * Aptentname initializes the error flag and begins writing an authcap
 * entry named ``name'' on the stream fp.
 */
int
aptentname(fp, errflgp, fields, name)
	FILE *fp;
	register int *errflgp, fields;
	char *name;
{
	char *tbuf;

	*errflgp = (fflush(fp) != 0);

	if (name && !*errflgp)
		*errflgp = !aencode(&tbuf, name);
	if (name && !*errflgp) {
		*errflgp = (fprintf(fp, "%s:", tbuf) < 0);
		free(tbuf);
		return pr_newline(fp, fields, errflgp);
	}
	return fields;
}

/*
 * Initialize the error flag and start writing an entry on stream fp
 * which is known by ``name''.  Keep any alternate names if this was
 * the most recent buffer match.
 */
static int
aptentidq(fp, errflgp, fields, name, ckfp)
	FILE *fp;
	register int *errflgp, fields;
	char *name;
	int (*ckfp)();
{
	register char **names;
	register int status;

	if (!abuf || !*abuf || !name || !*name || !(*ckfp)(name, (char *)0))
		return aptentname(fp, errflgp, fields, name);

	names = agetnmlist();
	if (!!names && !!names[0] && !!names[1])
		status = aptentnames(fp, errflgp, fields, names);
	else
		status = aptentname(fp, errflgp, fields, name);
	if (names)
		free((char *)names);
	return status;
}

int
aptentnameq(fp, errflgp, fields, name)
	FILE *fp;
	register int *errflgp, fields;
	char *name;
{
	return aptentidq(fp, errflgp, fields, name, ackentname);
}

int
aptentfileq(fp, errflgp, fields, name)
	FILE *fp;
	register int *errflgp, fields;
	char *name;
{
	return aptentidq(fp, errflgp, fields, name, ackentfile);
}

/*
 * Append a string-valued field to the authcap entry on stream fp.
 */
int
aptentstr(fp, errflgp, fields, name, value)
	FILE *fp;
	register int *errflgp, fields;
	char *name, *value;
{
	char *tbuf;

	if (!*errflgp)
		*errflgp = !aencode(&tbuf, value);
	if (name && !*errflgp) {
		*errflgp = (fprintf(fp, "%s=%s:", name, tbuf) < 0);
		free(tbuf);
		return pr_newline(fp, fields, errflgp);
	}
	return fields;
}

/*
 * Append a string-valued field to the authcap entry on stream fp which
 * has a length limit (besides NUL-termination).
 */
int
aptentnstr(fp, errflgp, fields, name, maxlen, value)
	FILE *fp;
	register int *errflgp, fields;
	char *name, *value;
	size_t maxlen;
{
	char *tstr;

	if (*errflgp || !value)
		return fields;
	tstr = (char *)malloc(maxlen+1);
	if (!tstr) {
		*errflgp = 1;
		return fields;
	}
	(void) strncpy(tstr, value, maxlen);
	tstr[maxlen] = '\0';
	fields = aptentstr(fp, errflgp, fields, name, tstr);
	free(tstr);
	return fields;
}

/*
 * Append a bitmap-valued field to the authcap entry being written on stream
 * fp.  Input is a cross between aptentstr and storenamepair.
 */
int
aptentnmpair(fp, errflgp, fields, name, values, maxval, pairings, pairingtype)
	FILE *fp;
	register int *errflgp, fields;
	char *name;
	mask_t *values;
	int maxval;
	struct namepair pairings[];
	char *pairingtype;
{
	register char *nmlist;

	nmlist = storenamepair(values, maxval, pairings, pairingtype);
	if (nmlist) {
		fields = aptentstr(fp, errflgp, fields, name, nmlist);
		free(nmlist);
	}
	else
		*errflgp = 1;
	return fields;
}

/*
 * Append a group-id-valued field to the authcap entry being written on
 * stream fp.  Input value is a gid, but the output is the corresponding
 * group name.
 */
int
aptentgid(fp, errflgp, fields, name, value)
	FILE *fp;
	register int *errflgp, fields;
	char *name;
	gid_t value;
{
	char *gname;

	if (!*errflgp)
		*errflgp = !(gname = gr_idtoname(value));
	return aptentstr(fp, errflgp, fields, name, gname);
}

/*
 * Append a user-id-valued field to the authcap entry being written on
 * stream fp.  Input value is a uid, but the output is the corresponding
 * username.
 */
int
aptentuid(fp, errflgp, fields, name, value)
	FILE *fp;
	register int *errflgp, fields;
	char *name;
	uid_t value;
{
	char *uname;

	if (!*errflgp)
		*errflgp |= !(uname = pw_idtoname(value));
	return aptentstr(fp, errflgp, fields, name, uname);
}

/*
 * Append an unsigned-long-valued field to the authcap entry being written
 * on stream fp.
 */
int
aptentunum(fp, errflgp, fields, name, value)
	FILE *fp;
	register int *errflgp, fields;
	char *name;
	u_long value;
{
	if (name && !*errflgp) {
		*errflgp = (fprintf(fp, "%s#%lu:", name, value) < 0);
		return pr_newline(fp, fields, errflgp);
	}
	return fields;
}

/*
 * Append an octal unsigned-long-valued field to the authcap entry being
 * written on stream fp.
 */
int
aptentonum(fp, errflgp, fields, name, value)
	FILE *fp;
	register int *errflgp, fields;
	char *name;
	u_long value;
{
	if (name && !*errflgp) {
		*errflgp = (fprintf(fp, "%s#0%lo:", name, value) < 0);
		return pr_newline(fp, fields, errflgp);
	}
	return fields;
}

/*
 * Append an unsigned-int-valued field to the authcap entry being written
 * on stream fp.
 */
int
aptentuint(fp, errflgp, fields, name, value)
	FILE *fp;
	register int *errflgp, fields;
	char *name;
	u_int value;
{
	return aptentunum(fp, errflgp, fields, name, (u_long)value);
}

/*
 * Append a signed-long-valued field to the authcap entry being written
 * on stream fp.
 */
int
aptentsnum(fp, errflgp, fields, name, value)
	FILE *fp;
	register int *errflgp, fields;
	char *name;
	long value;
{
	if (name && !*errflgp) {
		*errflgp = (fprintf(fp, "%s#%ld:", name, value) < 0);
		return pr_newline(fp, fields, errflgp);
	}
	return fields;
}

/*
 * Append a signed-int-valued field to the authcap entry being written
 * on stream fp.
 */
int
aptentsint(fp, errflgp, fields, name, value)
	FILE *fp;
	register int *errflgp, fields;
	char *name;
	int value;
{
	return aptentsnum(fp, errflgp, fields, name, (long)value);
}

/*
 * Append a time-valued field to the authcap entry being written
 * on stream fp.
 */
int
aptenttime(fp, errflgp, fields, name, value)
	FILE *fp;
	register int *errflgp, fields;
	char *name;
	time_t value;
{
	if (value != AUTH_TIME_NOW_VALUE)
		return aptentunum(fp, errflgp, fields, name, (ulong)value);
	if (name && !*errflgp) {
		*errflgp = (fprintf(fp, "%s#%s:", name, AUTH_TIME_NOW) < 0);
		return pr_newline(fp, fields, errflgp);
	}
	return fields;
}

/*
 * Append a boolean-valued field to the authcap entry being written
 * on stream fp.
 */
int
aptentbool(fp, errflgp, fields, name, value)
	FILE *fp;
	register int *errflgp, fields;
	char *name;
	int value;
{
	if (name && !*errflgp && (value >= 0)) {
		*errflgp = (fprintf(fp, "%s%s:", name, storebool(value)) < 0);
		return pr_newline(fp, fields, errflgp);
	}
	return fields;
}

/*
 * Write the final AUTH_CHKENT field (and newline) to the authcap entry
 * being written on stream fp.  Does an fflush(fp) to try to catch all
 * (possibly deferred) error stats.
 */
int
aptentfin(fp, errflgp)
	FILE *fp;
	register int *errflgp;
{
	if (!*errflgp)
		*errflgp = (fprintf(fp, "%s:\n", AUTH_CHKENT) < 0);
	*errflgp |= (fflush(fp) != 0);
	return !*errflgp;
}

/* #endif */ /*} SEC_BASE */
