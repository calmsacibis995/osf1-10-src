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
static char	*sccsid = "@(#)$RCSfile: Utils.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:04:35 $";
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

#include <sys/secdefines.h>
#if SEC_BASE

/*
	filename:
		Utils.c
	
	copyright:
		Copyright (c) 1989-1990 SecureWare Inc.
		ALL RIGHTS RESERVED

	functions:
		Realloc()	-- Equivalent memory allocation routines that
		Malloc()	-- do not return NULL when passed a zero
		Calloc()	-- argument
		tolower()	-- Returns the lowercase letter
		stricmp()       -- Compares two strings, ignoring case.
		strincmp()      -- Compares two strings, ignoring case.
		strstr()        -- Search for one string inside another
		strdup()        -- Malloc space for string and copy
		MemoryError()	-- Called when out of memory somewhere
		alloc_cw_table()
		expand_cw_table()
		free_cw_table()
		sort_cw_table()
		alloc_table()
	TimeToWeeks()	- Converts time to number of weeks
	TimeToDays()	- Converts time to number of days
	MallocChar()
	MallocInt()
	MallocWidget()	- This is in XUtils.c (it uses Widget)
	GetAllGroups()	- Returns a list of all the groups
	GetAllUsers()	- Returns a list of all the users
	GetAllIssoUsers()	- Returns a list of all ISSOs
	GetAllNonIssoUsers()	- Returns a list of all non ISSOs users
	GetAllDevices()	- Returns a list of all the devices
	GetAllTerminals() - Returns a list of all terminals
	GetAllHosts() - Returns a list of all hosts
	internet_to_hostname() - Converts from a hostname to internet address
	hostname_to_internet() - Converts from a internet address to hostname
	IsPrRetired()	- True if protect password entry is retired.
	popen_all_output()
	pclose_all_output()
		
	notes:
		This file contains general utilities
*/


#include <sys/types.h>

#ifdef SEC_BASE
#include <sys/security.h>
#include <sys/audit.h>
#include <prot.h>
#endif /* SEC_BASE */

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

#if SEC_BASE
#include <prot.h>
#else
struct namepair {
	char *name;
	ulong	value;
}
#endif

#include "XMain.h"		/* needed for UNAMELEN */

/* External routines */ 
extern int strcmp();
void MemoryError();

/* All routines defined here */
extern  char	*calloc();
extern	char	*malloc();
extern	char	*realloc();

#ifdef SEC_BASE
int	IsPrIsso();
int	IsIsso();
int	IsPrRetired();
char	*internet_to_hostname();
#endif /* SEC_BASE */

#ifdef DEBUG
void
dump_table (table, nentries)
	char **table;
	int     nentries;
{
	int i;
	for (i = 0; i < nentries; i++)
		printf ("Table values %d %s\n", i, table[i]);
}
#endif /* DEBUG */

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
	return (malloc  ( ((x) == 0) ? 1 : (x) ));
}

char *
Calloc (x,y)
	int x,y;
{
	return (calloc  ( (x), ((y) == 0) ? 1 : (y) ));
}

#ifdef notdef
	/* There is a bug in the current OSF snapshot with these routines so
	 * do them locally for the time being. When OSF fixes the problems
	 * the regular library routine can be called */
#ifdef OSF
char
tolower (c)
        char    c;
{
        int i;

        i = (int) c;
        if (i >= (int) 'A' && i<= (int) 'Z')
                i = i + (int) 'a' - (int) 'A';
        return ((char) i);
}

char
toupper (c)
        char    c;
{
        int i;

        i = (int) c;
        if (i >= (int) 'a' && i<= (int) 'z')
                i = i + (int) 'A' - (int) 'a';
        return ((char) i);
}
#endif /* OSF */
#endif

int
stricmp(a, b)
	register char *a;
	register char *b;
{
	for( ; *a && *b && (tolower(*a) == tolower(*b)); a++, b++ )
		;
	if (! *a && ! *b)
		return 0;
	if (! *a)
		return -1;
	if (! *b)
		return 1;
	return (tolower(*a) - tolower(*b));
}

int
strincmp(a, b, n)
	register char *a;
	register char *b;
	register int 	n;
{
	int i=0;

	for( ; i<n && *a && *b && (tolower(*a) == tolower(*b)); a++, b++ ,i++)
		;
	if (i >= n)
		return 0;
	if (! *a && ! *b)
		return 0;
	if (! *a)
		return -1;
	if (! *b)
		return 1;
	return (tolower(*a) - tolower(*b));
}

#ifndef ANSI
#ifndef OSF
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
#endif /* ! OSF */
#endif /* ! ANSI */

void 
MemoryError()
{
	/* allocate memory failure */
	audit_no_resource("Memory", OT_MEMORY, "Memory allocation failure", ET_SYS_ADMIN);
	/* Can't really display an error message because we need space to 
	 * create one. Best thing we can do is just die ... */
	printf ("Memory allocation error\n");
	exit(2);
}


/* Allocate a constant width table */
char **
alloc_cw_table(items, peritem)
int     items;
int     peritem;
{
	int	width;
	int	round;
	char    **table;
	char    *cp;
	int     i;
		
	/* in the case of nothing allocated, make sure at least one is */
	if (items == 0)
		items = 1;

	/* Round up the width to a long word boundary */
	round = peritem % sizeof (int);
	width = peritem; 
	if (round)
		width = width + (sizeof (int) - round);

	table = (char **) Calloc(items, sizeof (char *));
	if (! table)
		MemoryError();
		/* Dies */
	cp = Malloc (items * width);
	if (! cp)
		MemoryError();
		/* Dies */
	for (i = 0; i < items * width; i++)
		cp[i] = '\0';
	for (i = 0; i < items; i++, cp += width)
		table[i] = cp;
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
	int	width;
	int	round;
	char    *cp,
		*mem;
	int     i;
		
	/* Round up the width to a long word boundary */
	round = peritem % sizeof (int);
	width = peritem; 
	if (round)
		width = width + (sizeof (int) - round);

	table = (char **) Realloc(table, newsize * sizeof (char *));
	if (! table)
		MemoryError();
		/* Dies */
	mem = Realloc(table[0], newsize * width);
	if (! mem)
		MemoryError();
		/* Dies */
	cp = mem + (oldsize * width);
	for (i = 0; i < (newsize - oldsize) * width; i++)
		*cp++ = '\0';
	for (i = 0; i < newsize; i++, mem += width)
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
		    free(table[0]);
		free(table);
	}
	return;
}

/* sort the non-null entries of a constant width table */
void 
sort_cw_table(table, peritem, nentries)
	char    **table;
	int     peritem;
	int     nentries;
{
	int     i;
	int	width;
	int	round;
		
	/* Round up the width to a long word boundary */
	round = peritem % sizeof (int);
	width = peritem; 
	if (round)
		width = width + (sizeof (int) - round);

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

#ifndef SECINWEEK
#define SECINWEEK	(60 * 60 * 24 * 7)
#endif

#ifndef SECINDAY
#define SECINDAY	(60 * 60 * 24)
#endif

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

	setgrent();
	count = 0;
	while (grp = getgrent())
		count++;
	*pitems = alloc_cw_table(count, NGROUPNAME + 1);

	setgrent();
	for(i = 0; ( (grp = getgrent()) && (i < count) ) ; i++)
		strcpy((*pitems)[i], grp->gr_name);
	*pnitems = count;
	
	/* Alpha sort entries */
	sort_cw_table(*pitems, NGROUPNAME + 1, *pnitems);
}

#ifdef SEC_BASE 
	/* This code uses pr_passwd which is in prot.h therefore ifdef
	 * SecureWare surrounds the next few routines */
/* Get list of all valid user names */
void 
GetAllUsers(pnitems, pitems)
	int     *pnitems;
	char    ***pitems;
{
	int     count;
	struct pr_passwd *user;
	int	numb_users;
	int     i;

	/* Rewind the file */
	setprpwent();
	count = 0;
	numb_users = 0;
	while (user = getprpwent())
		count++;
	/* Allocate space to hold the users */
	*pitems = alloc_cw_table(count, UNAMELEN + 1);

	setprpwent();
	for(i = 0; user = getprpwent(); i++) {
		if (! IsPrRetired(user) ) {
			strcpy((*pitems)[numb_users ++], user->ufld.fd_name);
		}
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
	int     i;

	setprpwent();
	count = 0;
	numb_users = 0;
	while (user = getprpwent())
		count++;
	*pitems = alloc_cw_table(count, UNAMELEN + 1);

	setprpwent();
	for(i = 0; user = getprpwent(); i++) {
		if (IsPrIsso(user) && (! IsPrRetired(user)) )  {
			strcpy((*pitems)[numb_users ++], user->ufld.fd_name);
		}
	}
	*pnitems = numb_users;
	
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
	int     i;

	setprpwent();
	count = 0;
	numb_users = 0;
	while (user = getprpwent())
		count++;
	*pitems = alloc_cw_table(count, UNAMELEN + 1);

	setprpwent();
	for(i = 0; user = getprpwent(); i++) {
		if ( (! IsPrIsso(user) ) && (! IsPrRetired(user)) )  {
			strcpy((*pitems)[numb_users ++], user->ufld.fd_name);
		}
	}
	*pnitems = numb_users;
	
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

/* Returns trus is account is retired */
int
IsPrRetired (pr)
	struct pr_passwd *pr;
{
	if (pr->uflg.fg_retired && pr->ufld.fd_retired)
		return (1);
	return (0);
}

/* Gets a list of all device names. If include_hosts then host names (in
 * hostname format) are included */
void 
GetAllDevices(pnitems, pitems, include_hosts)
	int     *pnitems;
	char    ***pitems;
	Boolean	include_hosts;
{
	int     count;
	struct dev_asg *device;
	int     i;

	setdvagent();
	count = 0;
	while (device = getdvagent())
		count++;
	*pitems = alloc_cw_table(count, DEVICE_NAME_LEN + 1);

	setdvagent();
	count = 0;
	/* Only include hosts if include_hosts is True */
	for(i = 0; device = getdvagent(); i++) {
#ifdef DEBUG
		printf ("device name %s\n", device->ufld.fd_name);
#endif
		if ( (include_hosts) ||
		     (! (ISBITSET (device->ufld.fd_type, AUTH_DEV_REMOTE) ) ) )
			strcpy((*pitems)[count++], device->ufld.fd_name);
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
	int     count;
	struct pr_term *terminal;
	struct dev_asg *device;
	int     i;

	setprtcent();
	count = 0;
	while (terminal = getprtcent())
		count++;
	*pitems = alloc_cw_table(count, TERMINAL_NAME_LEN + 1);

	/* Rewind the device assignment database as well */
	setdvagent();
	setprtcent();
	count = 0;
	for(i = 0; terminal = getprtcent(); i++) {
		device = getdvagnam (terminal->ufld.fd_devname);
		if ( (device != (struct dev_asg *) 0)  &&
		     (ISBITSET (device->ufld.fd_type, AUTH_DEV_TERMINAL) ) )
			strcpy((*pitems)[count++], terminal->ufld.fd_devname);
	}
	*pnitems = count;
	
	/* Alpha sort entries */
	sort_cw_table(*pitems, TERMINAL_NAME_LEN + 1, *pnitems);
}

#ifdef SEC_NET_TTY
void 
GetAllHosts(pnitems, pitems)
	int     *pnitems;
	char    ***pitems;
{
	int     count;
	struct pr_term *host;
	struct dev_asg *device;
	char	*host_name;
	int     i;

	setprtcent();
	count = 0;
	while (host = getprtcent())
		count++;
	*pitems = alloc_cw_table(count, HOST_NAME_LEN + 1);

	/* Rewind the device assignment database as well */
	setdvagent();
	setprtcent();
	count = 0;
	for(i = 0; host = getprtcent(); i++) {
#ifdef DEBUG
		printf ("Host name count= %d %s\n", count,
			host->ufld.fd_devname);
#endif
		device = getdvagnam (host->ufld.fd_devname);
#ifdef DEBUG
		if (device == (struct dev_asg *) 0)
			printf ("Device is NULL \n");
		printf ("Device name = %s\n", device->ufld.fd_name);
		printf ("Host name = %s\n", host->ufld.fd_devname);
		printf ("Assignment = %d ", device->ufld.fd_assign[0]);
		printf (" Type = %d\n", device->ufld.fd_type[0]);
	     if (ISBITSET (device->ufld.fd_type, AUTH_DEV_TAPE) ) 
		printf ("TAPE\n");
	     if (ISBITSET (device->ufld.fd_type, AUTH_DEV_TERMINAL) ) 
		printf ("TERMINAL\n");
	     if (ISBITSET (device->ufld.fd_type, AUTH_DEV_PRINTER) ) 
		printf ("PRINTER\n");
	     if (ISBITSET (device->ufld.fd_type, AUTH_DEV_REMOTE) ) 
		printf ("REMOTE\n");
#endif
		if ( (device != (struct dev_asg *) 0 ) &&
		     (ISBITSET (device->ufld.fd_type, AUTH_DEV_REMOTE) ) ) {
#ifdef DEBUG
			printf ("Storing one\n");
#endif
			host_name = internet_to_hostname(host->ufld.fd_devname);
#ifdef DEBUG
			printf ("host_name %s\n", host_name);
#endif
			strcpy((*pitems)[count++], host_name);
			free(host_name);
		}
	}
	*pnitems = count;
	
	/* Alpha sort entries */
	sort_cw_table(*pitems, HOST_NAME_LEN + 1, *pnitems);
}
/*
 * Convert the ASCII form of a network address to a real network address.
*/

host_nametoaddr(name, addr)
char *name;
struct in_addr *addr;
{
	sscanf (name, "%8x", &addr->s_addr);
}

/* Convert from our internal format of a host name to the real host name */
char *
internet_to_hostname (host)
char    *host;
{
	struct in_addr hostaddr;
	struct hostent  *hp;
	int     i;
	char    *name;
	char    *addr;

#ifdef DEBUG
	printf ("INT HOST NAE %s\n", host);
#endif
	host_nametoaddr(host, &hostaddr);
	hp = gethostbyaddr(&hostaddr, sizeof hostaddr, AF_INET);
	if (hp == (struct hostent *) 0) {
#ifdef DEBUG
		printf ("NULL\n");
#endif
		return (NULL);
	}

	name = strdup (hp->h_name);
#ifdef DEBUG
	printf ("Hostname is %s\n", hp->h_name);
	printf ("Aliases are %s\n", hp->h_aliases[0]);
	printf ("Returning name from inhost_name %s\n", name);
#endif
	return (name);
}

char *
hostname_to_internet (name)
char	*name;
{
	struct hostent	*hp;
	int	i;
	char	*host;
	char	*addr;

	hp = gethostbyname(name);
	if (hp == (struct hostent *) 0)
		return (NULL);
	
	host = (char *) Malloc(20);
	if (! host)
		MemoryError();

	addr = hp->h_addr_list[0];
	for (i=0; i<hp->h_length; i++) {
#ifdef DEBUG
		printf ("addrs =%d i=%d\n", addr[i] & 0xff, i);
#endif
		sprintf (&host[i*2], "%2.2x", addr[i] & 0xff);
	}
	strcat (host, "0000");
#ifdef DEBUG
	printf ("Returning name from host_name %s\n", host);
#endif
	return (host);

}

#endif /* SEC_NET_TTY */

#endif /* SEC_BASE */

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
#endif /* SEC_BASE */
