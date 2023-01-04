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
static char	*sccsid = "@(#)$RCSfile: Utils.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:01:20 $";
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
/*
 * Copyright(c) 1989-90 SecureWare, Inc.
 *   All rights reserved.
 */



/*
	filename:
		Utils.c
	
	FUNCTIONS:

	IsPrIsso()	- True if protect password struct is ISSO.
	IsIsso()	- True if user is retired.
	IsPrRetired()	- True if protect password struct is retired.
	IsPrLocked()	- True if protect password struct is locked.

	Malloc()	- malloc that won't puke if asked for 0 bytes
	Calloc()	- calloc that won't puke if asked for 0 bytes
	Realloc()	- realloc that won't puke if asked for 0 bytes
	Free()		- make sure address != 0 before freeing

	MallocChar()	- malloc a char
	MallocInt()	- malloc an int char

	MemoryError()	- Called when out of memory somewhere

	stricmp()       - Compares two strings, ignoring case.
	strincmp()      - Compares two strings, ignoring case.
	strstr()        - Search for one string inside another
	strdup()        - Malloc space for string and copy

	alloc_cw_table()	- alloc a common-width table
	expand_cw_table()	- expand a common-width table
	free_cw_table()	- free a common-width table
	sort_cw_table()	- (alpha) sort a common-width table
	alloc_table()	- alloc space for table of namepair table names

	TimeToWeeks()	- Converts time to number of weeks
	TimeToDays()	- Converts time to number of days

	GetAllGroups()	- Returns a list of all the groups
	GetUserByName()	- return pr_passwd struct for named user
	GetUserByUID()	- return pr_passwd struct for designated UID
	GetAllUsers()	- Returns a list of all the users
	GetAllIssoUsers()	- Returns a list of all ISSOs
	GetAllNonIssoUsers()	- Returns a list of all non ISSOs users
	GetAllDevices()	- Returns a list of all the devices
	GetAllTerminals()	- Returns a list of all terminals
	GetAllPrinters()	- Returns a list of all printers
	GetAllRemovables()	- Returns a list of all removable devices
	GetAllHosts()	- Returns a list of all hosts

	internet_to_hostname() - Converts from a hostname to internet address
	hostname_to_internet() - Converts from a internet address to hostname

	popen_all_output()	- run a program down a pipe with std{out,err}
	pclose_all_output()	- close the pipes opened above

	InvalidUser()	- is it a legal user name?
*/

#include <sys/secdefines.h>

#if SEC_BASE

#include <sys/types.h>
#include <sys/security.h>
#include <sys/audit.h>
#include <prot.h>

#ifdef SEC_NET_TTY
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#endif /* SEC_NET_TTY */

#include <stdio.h>
#include <malloc.h>
/* This include file is strings on some machines - PORTABILITY */
#ifdef AUX
#include <string.h>
#else
#include <strings.h>
#endif /* AUX */

#include <grp.h>
#include <pwd.h>

#include <prot.h>

#include "Accounts.h"
#include "kitch_sink.h"
#include "logging.h"
#include "Utils.h"

/* External routines */ 
extern int strcmp();

/* for debugging malloc problems - routines are in dbmalloc.c (not used now) */

#ifdef DEBUG_MALLOC
extern char *dbmalloc(), *dbrealloc(), *dbcalloc(), *dbstrdup();
extern void dbfree();
#define malloc(bytes)	dbmalloc(bytes)
#define realloc(pointer,bytes)	dbrealloc(pointer,bytes)
#define free(pointer)		dbfree(pointer)
#define calloc(nelem,elsize)	dbcalloc(nelem,elsize)
#define strdup(pointer)		dbstrdup(pointer)
#else
extern char *malloc(), *realloc(), *calloc(), *strdup();
extern void free();
#endif


static void GetAllDeviceType();

/* Start of general non-X utilities */
	/* Some machines return a NULL value when passed a 0 argument for 
	 * memory allocation. This solves the problem */
char *
Realloc (x,y)
	int x,y;
{
	return (realloc ( (x), ((y) == 0) ? 1 : (y) ));
}

char *
Malloc (x)
	int x;
{
	return (malloc ( ((x) == 0) ? 1 : (x) ));
}

char *
Calloc (x,y)
	int x,y;
{
	return (calloc ((((x)==0)?1:(x)), ((y)==0)?1:(y)));
}

void
Free(x)
	char *x;
{
	if (x)
		free (x);
}


int
stricmp(a, b)
	register char *a;
	register char *b;
{
	for( ; *a && *b && (ToLower(*a) == ToLower(*b)); a++, b++ )
		;
	if (! *a && ! *b)
		return 0;
	if (! *a)
		return -1;
	if (! *b)
		return 1;
	return (ToLower(*a) - ToLower(*b));
}

int
strincmp(a, b, n)
	register char *a;
	register char *b;
	register int 	n;
{
	int i=0;

	for( ; i<n && *a && *b && (ToLower(*a) == ToLower(*b)); a++, b++ ,i++)
		;
	if (i >= n)
		return 0;
	if (! *a && ! *b)
		return 0;
	if (! *a)
		return -1;
	if (! *b)
		return 1;
	return (ToLower(*a) - ToLower(*b));
}

#ifndef __STDC__
char *
strstr(s1,s2)
char *s1,*s2;
{
	int l;
	
	if (! s1 || ! s2 || ! *s2)
		return 0;
	l = strlen(s2);
	
	for(;;) {
		if (! *s1)
		    return 0;
		s1 = strchr(s1, s2[0]);
		if ( ! s1 )
		    break;
		if (! strncmp(s1, s2, l))
		    break;
		s1++;
	}
	return s1;
}

char *
strdup(string1)
	char *string1;
{
	char *string2;
	
	string2 = (char *) Malloc(strlen(string1) + 1);
	if (! string2)
		MemoryError();
		/* Dies */
	strcpy(string2, string1);
	return string2;
}
#endif /* ! __STDC__ */


/*
 * memory allocation failure
 *
 *	calls application-specific EOP_cleanup() routine
 */

void 
MemoryError()
{
	ENTERLFUNC("MemoryError");
	EOP_cleanup ();
	audit_no_resource("Memory", OT_MEMORY, "Memory allocation failure", ET_SYS_ADMIN);
	/* Can't really display an error message because we need space to 
	 * create one. Best thing we can do is just die ... */
	printf ("Memory allocation error\n");
	EXITLFUNC("MemoryError");
	exit(2);
}


/* Allocate a constant width table */
char **
alloc_cw_table(items, peritem)
int     items;
int     peritem;
{
	char    **table;
	char    *cp;
	int     i;
		
	ENTERLFUNC("alloc_cw_table");
	DUMPLARGS("items=<%d> peritem=<%d>", items, peritem, NULL);
	/* in the case of nothing allocated, make sure at least one is */
	if (items == 0)
		items = 1;
	table = (char **) Calloc(items, sizeof (char *));
	if (! table)
		MemoryError();
		/* Dies */
	cp = Calloc (items,  peritem);
	if (! cp)
		MemoryError();
		/* Dies */
	for (i = 0; i < items; i++, cp += peritem)
		table[i] = cp;
	EXITLFUNC("alloc_cw_table");
	return table;
}

/* Make a constant width table bigger */
char **
expand_cw_table(table, oldsize, newsize, peritem)
	char    **table;
	int     oldsize;
	int     newsize;
	int     peritem;
{
	char    *cp,
		    *mem;
	int     i;
		
	table = (char **) Realloc(table, newsize * sizeof (char *));
	if (! table)
		MemoryError();
		/* Dies */
	mem = Realloc(table[0], newsize * peritem);
	if (! mem)
		MemoryError();
		/* Dies */
	cp = mem + (oldsize * peritem);
	for (i = 0; i < (newsize - oldsize) * peritem; i++)
		*cp++ = '\0';
	for (i = 0; i < newsize; i++, mem += peritem)
		table[i] = mem;
	return table;
}

/* Free the memory of a constant width table */
void 
free_cw_table(table)
	char    **table;
{
	if (table) {
		if (table[0])
		    Free(table[0]);
		Free(table);
	}
	return;
}

/* sort the non-null entries of a constant width table */

void 
sort_cw_table(table, width, nentries)
	char    **table;
	int     width;
	int     nentries;
{
	int     i;
		
	for (i = 0; i < nentries; i++)
		if (table[i][0] == '\0')
		    break;
	if (i > 0)
		qsort(table[0], i, width, strcmp);
	return;
}

/* Allocate memory large enough to hold a list of entries */
char	**
alloc_table (nametab, tabsize)
struct	namepair	nametab[];
int	tabsize;
{
	int	i;
	int	thiswide, widest = 0;

	for (i = 0; i < tabsize; i++)
		if ((thiswide = strlen (nametab[i].name)) > widest)
			widest = thiswide;
	return (alloc_cw_table (tabsize, widest + 1));
}


int
TimeToWeeks (t)
	long t;
{
	return ( (int) (t / (long) SECINWEEK) );
}

int
TimeToDays (t)
	long t;
{
	return ( (int) (t / (long) SECINDAY) );
}

char *
MallocChar(n)
	int n;
{
	char *s;

	s = (char *) Malloc(n);
	if (s == (char *) NULL)
		MemoryError();
	return(s);
}

int *
MallocInt(n)
	int n;
{
	int *s;

	s = (int *) Malloc(sizeof(int) * n);
	if (s == (int *) NULL)
		MemoryError();
	return(s);
}

/* Get all the groups on the system */
void 
GetAllGroups(pnitems, pitems)
	int     *pnitems;
	char    ***pitems;
{
	int     count;
	struct  group *grp;
	int     i;
	struct a {
		char name[GNAMELEN + 1];
		struct a *next;
	} *first = NULL, *last, *ap;

	setgrent();
	count = 0;
	while (grp = getgrent()) {
		ap = (struct a *) Calloc(sizeof(struct a), 1);
		if (ap == (struct a *) 0)
			MemoryError();
		strncpy(ap->name, grp->gr_name, GNAMELEN);
		if (first == NULL)
			first = last = ap;
		else {
			last->next = ap;
			last = ap;
		}
		count++;
	}
	last->next = (struct a *) 0;

	*pitems = alloc_cw_table(count, GNAMELEN + 1);

	for (ap = first, i = 0; ap; i++) {
		struct a *oap;

		strcpy((*pitems)[i], ap->name);
		oap = ap;
		ap = ap->next;
		Free(oap);
	}

	*pnitems = count;
	
	/* Alpha sort entries */
	sort_cw_table(*pitems, GNAMELEN + 1, *pnitems);
}


/*
 * GetUserByName() - return 1 (and the user entry) if user exists, else 0
 */

int
GetUserByName(name, upw)
	char *name;
	struct pr_passwd *upw;
{
	struct pr_passwd *user;

	user = getprpwnam(name);
		if (user != (struct pr_passwd *) NULL) {
			bcopy (user, upw, sizeof (struct pr_passwd));
			return 1;
		}
	return 0;
}


/*
 * GetUserByUID() - return 1 (and the user entry) if UID exists, else 0
 */

int
GetUserByUID(uid, upw)
	long uid;
	struct pr_passwd *upw;
{
	struct pr_passwd *user;

	user = getprpwuid(uid);
		if (user != (struct pr_passwd *) NULL) {
			bcopy (user, upw, sizeof (struct pr_passwd));
			return 1;
		}
	return 0;
}



/* Get list of all valid user names */
void 
GetAllUsers(pnitems, pitems)
	int     *pnitems;
	char    ***pitems;
{
	int     count;
	struct pr_passwd *user;
	int	numb_users;
	struct a {
		char name[UNAMELEN + 1];
		struct a *next;
	} *first = NULL, *last, *ap;

	/* Rewind the file */
	setprpwent();
	count = 0;
	numb_users = 0;

	while (user = getprpwent()) {
		if (IsPrRetired(user))
			continue;
		ap = (struct a *) Calloc(sizeof(struct a), 1);
		if (ap == (struct a *) 0)
			MemoryError();
		strncpy(ap->name, user->ufld.fd_name, UNAMELEN);
		if (first == NULL)
			first = last = ap;
		else {
			last->next = ap;
			last = ap;
		}
		count++;
	}
	ap->next = (struct a *) 0;

	*pitems = alloc_cw_table(count, UNAMELEN + 1);

	for (ap = first; ap; ) {
		struct a *oap;

		strcpy((*pitems)[numb_users ++], ap->name);
		oap = ap;
		ap = ap->next;
		Free(oap);
	}

	*pnitems = numb_users;
	
	/* Alpha sort entries */
	sort_cw_table(*pitems, UNAMELEN + 1, *pnitems);
}

void 
GetAllIssoUsers(pnitems, pitems)
	int     *pnitems;
	char    ***pitems;
{
	int		numb_users;
	int     count;
	struct pr_passwd *user;
	struct a {
		char name[UNAMELEN + 1];
		struct a *next;
	} *first = NULL, *last, *ap;


	setprpwent();
	count = 0;
	numb_users = 0;

	while (user = getprpwent()) {
		if (!IsPrIsso(user) || IsPrRetired(user))
			continue;
		ap = (struct a *) Calloc(sizeof(struct a), 1);
		if (ap == (struct a *) 0)
			MemoryError();
		strncpy(ap->name, user->ufld.fd_name, UNAMELEN);
		if (first == NULL)
			first = last = ap;
		else {
			last->next = ap;
			last = ap;
		}
		count++;
	}
	ap->next = (struct a *) 0;

	*pitems = alloc_cw_table(count, UNAMELEN + 1);

	for (ap = first; ap; ) {
		struct a *oap;

		strcpy((*pitems)[numb_users ++], ap->name);
		oap = ap;
		ap = ap->next;
		Free(oap);
	}

	*pnitems = count;

	/* Alpha sort entries */
	sort_cw_table(*pitems, UNAMELEN + 1, *pnitems);
}

void 
GetAllNonIssoUsers(pnitems, pitems)
	int     *pnitems;
	char    ***pitems;
{
	int		numb_users;
	int     count;
	struct pr_passwd *user;
	struct a {
		char name[UNAMELEN + 1];
		struct a *next;
	} *first = NULL, *last, *ap;


	setprpwent();
	count = 0;
	numb_users = 0;

	while (user = getprpwent()) {
		if (IsPrIsso(user) || IsPrRetired(user))
			continue;
		ap = (struct a *) Calloc(sizeof(struct a), 1);
		if (ap == (struct a *) 0)
			MemoryError();
		strncpy(ap->name, user->ufld.fd_name, UNAMELEN);
		if (first == NULL)
			first = last = ap;
		else {
			last->next = ap;
			last = ap;
		}
		count++;
	}
	ap->next = (struct a *) 0;

	*pitems = alloc_cw_table(count, UNAMELEN + 1);

	for (ap = first; ap; ) {
		struct a *oap;

		strcpy((*pitems)[numb_users ++], ap->name);
		oap = ap;
		ap = ap->next;
		Free(oap);
	}

	*pnitems = count;
	
	/* Alpha sort entries */
	sort_cw_table(*pitems, UNAMELEN + 1, *pnitems);
}

int
IsPrIsso(pr)
	struct pr_passwd *pr;
{
	if (pr->uflg.fg_cprivs)
		return (hascmdauth ("isso", pr->ufld.fd_cprivs));
	if (pr->sflg.fg_cprivs)
		return (hascmdauth ("isso", pr->sfld.fd_cprivs));
	return (0);
}

int
IsIsso(user)
	char *user;
{
	struct pr_passwd	*pr;

	pr = getprpwnam (user);
	if (pr == (struct pr_passwd *) NULL)
		return (0);
	if (pr->uflg.fg_cprivs)
		return (hascmdauth ("isso", pr->ufld.fd_cprivs));
	if (pr->sflg.fg_cprivs)
		return (hascmdauth ("isso", pr->sfld.fd_cprivs));
	return (0);
}

/* Returns true if account is retired */
int
IsPrRetired (pr)
	struct pr_passwd *pr;
{
	if (pr->uflg.fg_retired && pr->ufld.fd_retired)
		return (1);
	return (0);
}


/* Returns true if account is locked */
int
IsPrLocked (pr)
	struct pr_passwd *pr;
{
	if (pr->uflg.fg_lock && pr->ufld.fd_lock)
		return (1);
	return (0);
}

/*
 * Gets a list of all device names. If include_hosts then host names (in
 * hostname format) are included
 */

void 
GetAllDevices(pnitems, pitems, include_hosts)
	int     *pnitems;
	char    ***pitems;
	Boolean	include_hosts;
{
	int     count;
	struct dev_asg *device;
	int     i;
	struct a {
		char name[DEVICE_NAME_LEN + 1];
		struct a *next;
	} *first = NULL, *last, *ap;


	setdvagent();
	count = 0;

	while (device = getdvagent()) {
		if (!include_hosts &&
		     ISBITSET(device->ufld.fd_type, AUTH_DEV_REMOTE))
			continue;
		ap = (struct a *) Calloc(sizeof(struct a), 1);
		if (ap == (struct a *) 0)
			MemoryError();
		strncpy(ap->name, device->ufld.fd_name, DEVICE_NAME_LEN);
		if (first == NULL)
			first = last = ap;
		else {
			last->next = ap;
			last = ap;
		}
		count++;
	}
	ap->next = (struct a *) 0;

	*pitems = alloc_cw_table(count, DEVICE_NAME_LEN + 1);

	count = 0;
	for (ap = first; ap; ) {
		first = first->next;
		strcpy((*pitems)[count++], ap->name);
		free(ap);
		ap = first;
	}

	*pnitems = count;

	/* Alpha sort entries */
	sort_cw_table(*pitems, DEVICE_NAME_LEN + 1, *pnitems);
}

void 
GetAllTerminals(pnitems, pitems)
	int     *pnitems;
	char    ***pitems;
{
	GetAllDeviceType(pnitems, pitems, AUTH_DEV_TERMINAL);
	return;
}

void 
GetAllPrinters(pnitems, pitems)
	int     *pnitems;
	char    ***pitems;
{
	GetAllDeviceType(pnitems, pitems, AUTH_DEV_PRINTER);
	return;
}

void
GetAllRemovables(pnitems, pitems)
	int     *pnitems;
	char    ***pitems;
{
	GetAllDeviceType(pnitems, pitems, AUTH_DEV_TAPE);
	return;
}

#if SEC_NET_TTY
void
GetAllHosts(pnitems, pitems)
	int     *pnitems;
	char    ***pitems;
{
	char **host_table;
	int i;
	char **dev_table;
	int dev_count;

	GetAllDeviceType(pnitems, pitems, AUTH_DEV_REMOTE);

	/* allocate a table and convert each database entry to host name */

	dev_count = *pnitems;

	if (dev_count > 0) {
		dev_table = *pitems;
		host_table = alloc_cw_table(dev_count, HOST_NAME_LEN + 1);
		if (host_table == (char **) 0)
			MemoryError();

		for (i = 0; i < *pnitems; i++) {
			char *host_name;

			host_name = internet_to_hostname(dev_table[i]);
			if (host_name == NULL)
				strcpy(host_table[i], dev_table[i]);
			else {
				strcpy(host_table[i], host_name);
				Free(host_name);
			}
		}
		free_cw_table(dev_table);
		*pitems = host_table;
	}
	return;
}
#endif


static
void 
GetAllDeviceType(pnitems, pitems, device_type)
	int     *pnitems;
	char    ***pitems;
	int	device_type;
{
	int     count;
	struct dev_asg *dv;
	int     i;
	struct a {
		char name[TERMINAL_NAME_LEN + 1];
		struct a *next;
	} *first = NULL, *last, *ap;


	setdvagent();
	count = 0;
	while (dv = getdvagent()) {
		if (dv == (struct dev_asg *) 0)
			continue;
		if (device_type != -1 &&
		    !ISBITSET(dv->ufld.fd_type, device_type))
			continue;
		ap = (struct a *) Calloc(sizeof(struct a), 1);
		if (ap == (struct a *) 0)
			MemoryError();
		strncpy(ap->name, dv->ufld.fd_name, sizeof(ap->name) - 1);
		if (first == NULL)
			first = last = ap;
		else {
			last->next = ap;
			last = ap;
		}
		count++;
	}
	ap->next = (struct a *) 0;

	*pitems = alloc_cw_table(count, sizeof(ap->name));

	count = 0;
	for (ap = first; ap; ) {
		strcpy((*pitems)[count++], ap->name);
		first = ap->next;
		free(ap);
		ap = first;
	}

	*pnitems = count;
	
	/* Alpha sort entries */
	sort_cw_table(*pitems, sizeof(ap->name), *pnitems);
}

#if SEC_NET_TTY

/* Convert from our internal format of a host name to the real host name */
char *
internet_to_hostname (host)
char    *host;
{
	struct in_addr hostaddr;
	struct hostent  *hp;

	sscanf(host, "%8x", &hostaddr.s_addr);
	hostaddr.s_addr = htonl(hostaddr.s_addr);
	hp = gethostbyaddr((char *) &hostaddr.s_addr, sizeof hostaddr, AF_INET);
	if (hp == (struct hostent *) 0)
		return NULL;
	else
		return hp->h_name;
}

/* Convert from host name to ASCII Internet address in host byte order */

char *
hostname_to_internet (name)
char	*name;
{
	struct hostent	*hp;
	struct in_addr hostaddr;
	static char host_string[13]; /* 8 bytes of inet addr + 4 0's */

	hp = gethostbyname(name);
	if (hp == (struct hostent *) 0)
		return (NULL);
	bcopy(hp->h_addr, &hostaddr.s_addr, hp->h_length);
	sprintf(host_string, "%8.8x0000", ntohl(hostaddr.s_addr));
	return host_string;
}

#endif /* SEC_NET_TTY */


FILE *
popen_all_output (program, argv)
char	*program;
char	*argv[];
{
	int	pipefd[2];
	int	pid;

	/* interrupts are always off here */
	if (pipe (pipefd) < 0)
		return ((FILE *) 0);
	switch (pid = fork())  {
		case -1:
			return ((FILE *) 0);
		case 0:
			/* close the reader end */
			close (pipefd[0]);
			/* make the stdout and stderr go up the pipe */
			close (fileno (stdout));
			dup (pipefd[1]);
			close (fileno (stderr));
			dup (pipefd[1]);
			close (pipefd[1]);
			if (execv (program, argv) < 0)
				exit (0);
		default:
			/* close the writer end */
			close (pipefd[1]);
			return (fdopen (pipefd[0], "r"));
	}
}

void
pclose_all_output (fp)
FILE	*fp;
{
	int	wait_stat;

	while (wait (&wait_stat) == -1)
		;
	fclose (fp);
}


/* check that it's a valid user name: no ':', no upper case as 1st char,
 * no digit as first char (messes up chmod and chown), no '/' (messes up
 * home directory).
 * returns 0 on success (user name ok), 1 if not ok.
 */

int
InvalidUser (name)
char *name;
{
	if (strchr (name, ':') || isupper (name[0])  ||
		isdigit (name[0]) || strchr (name, '/'))
			return (1);
	return (0);
}

/* THIS DOESN'T GET USED ANYWHERE ANYMORE */

/* create a new protected password entry for the user.  Use the
 * system defaults in most fields.
 */

static char *dflt_subsys_table[] = {
	"default"
};

static void
new_prpw (username,userid, pr)
char *username;
int userid;
struct pr_passwd *pr;
{
	/* clear out the structure to all zeros (defaults) */
	(void) memset (pr, NULL, sizeof (*pr));

#ifdef SEC_TMAC
	gen_admin_num(pr);
#endif /* SEC_TMAC */
	(void) strcpy (pr->ufld.fd_name, username);
	pr->uflg.fg_name = 1;
	pr->ufld.fd_uid = userid;
	pr->uflg.fg_uid = 1;
	pr->uflg.fg_lock = 1;
	pr->ufld.fd_lock = 0;

	/* assume that the user can change his own password. */
	pr->ufld.fd_pick_pwd = 'y';
	pr->uflg.fg_pick_pwd = 1;
#ifdef SEC_MAC
	mand_init() ;
	pr-> uflg.fg_clearance = 1 ;
#ifdef SEC_SHW
	mand_copy_ir(mand_syshi, &pr-> ufld.fd_clearance) ;
#else /* SEC_SHW */
	mand_copy_ir(mand_syslo, &pr-> ufld.fd_clearance) ;
#endif /* SEC_SHW */
#ifdef SEC_NCAV
	pr-> ufld.fd_nat_caveats = (ncav_ir_t *) Malloc(sizeof(ncav_ir_t)) ;
	if (pr-> ufld.fd_nat_caveats != (ncav_ir_t *) 0) {
		ncav_init() ;
		pr-> uflg.fg_nat_caveats = 1 ;
		memcpy(pr-> ufld.fd_nat_caveats, ncav_max_ir, sizeof(ncav_ir_t)) ;
	}
#endif /* SEC_NCAV */
#endif /* SEC_MAC */

	if (putprpwnam (username, pr) != 1)
		audit_subsystem (username,
		  "successful update of protected password entry", 
		  ET_SYS_ADMIN);

	/* Add user to the default subsystem authorizations file */
	write_authorizations (pr->ufld.fd_name, dflt_subsys_table, 1);

	return;
}
#endif /* SEC_BASE */
