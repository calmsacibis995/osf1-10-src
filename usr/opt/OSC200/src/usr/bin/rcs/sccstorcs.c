/*
 * *********************************************************************
 * *                                                                   *
 * *       Modified by Digital Equipment Corporation, 1991, 1994       *
 * *                                                                   *
 * *       This file no longer matches the original Free Software      *
 * *       Foundation file.                                            *
 * *                                                                   *
 * *********************************************************************
 */
/*
 * HISTORY
 */
#ifndef lint
static char *rcsid = "@(#)$RCSfile: sccstorcs.c,v $ $Revision: 1.1.4.3 $ (DEC) $Date: 1993/05/28 02:53:03 $";
#endif
/*
 * SCCSTORCS - build RCS file from SCCS file preserving deltas.
 * Author: Ken Greer
 *
 * Copyright (c) 1983 by Kenneth L. Greer
 *
 * All rights reserved. No part of this software may be sold or distributed
 * in any form or by any means without the prior written permission of the
 * author.
 *
 * Revision 1.6  91/01/07  16:49:54  devrcs
 * 	rcsid/RCSfile header cleanup
 * 	[90/12/01  18:52:39  dwm]
 * 
 * Revision 1.5  90/10/07  21:46:14  devrcs
 * 	Added EndLog Marker.
 * 	[90/09/29  13:57:23  gm]
 * 
 * Revision 1.4  90/08/25  12:20:10  devrcs
 * 	Cleanup for time functions.
 * 	[90/08/14  18:06:43  gm]
 * 
 * Revision 1.3  90/07/17  12:33:03  devrcs
 * 	More changes to switch to gcc.
 * 	[90/07/03  19:37:09  gm]
 * 
 * Revision 1.2  90/04/14  00:51:57  devrcs
 * 	Removed extern of sprintf(), let it come from stdio.h.
 * 	[90/04/13            gm]
 * 
 * Revision 1.1  89/12/26  10:45:01  gm
 * 	Current version from CMU.
 * 	[89/12/23            gm]
 * 
 * Revision 2.4  89/09/15  01:02:30  bww
 * 	Updated use of variable argument lists.
 * 	[89/09/15  01:00:53  bww]
 * 
 * Revision 2.3  89/07/10  16:24:13  bww
 * 	sprintf() is now of type "int" in the BSD.
 * 	[89/07/10  16:23:51  bww]
 * 
 * Revision 2.2  89/05/23  20:51:35  bww
 * 	Included Guy Harris's lint cleanups.  Added HISTORY section.
 * 	[89/05/23            bww]
 * 
 * Revision 2.1  89/05/23  17:17:28  bww
 * 	BSD 4.3-tahoe release
 * 	[89/05/23  17:03:34  bww]
 * 
 * Revision 1.5  86/07/28  23:30:51  guy
 * 	More "lint" cleanups.
 * 
 * Revision 1.4  84/10/17  21:12:11  root
 * 	Added check for having multiple deltas in a row for the same revision.
 * 	--ks
 * 
 * Revision 1.3  84/10/17  20:53:18  root
 * 	Put in SCCS string in comment for telling who checked it in..
 * 	--ks
 * 
 * Revision 1.2  84/10/17  12:22:14  root
 * 	Fixed the case when a delta was removed.
 * 	Also, use -f on checkin so comments are kept even if the file
 * 	didn't change between deltas.
 * 	--ks
 * 
 * Revision 1.1  84/10/07  14:59:47  root
 * 	Initial revision
 * 
 * Revision 1.2  83/03/27  11:21:17  root
 * 	Returns non-zero exit codes on soft errors also.
 * 
 * Revision 1.1  83/03/24  14:33:24  root
 * 	Initial revision
 * 
 */

#if defined(USG) || defined(BSD)
#define VOID	(void)
#else
#define VOID
#endif

extern char *strcpy();
extern char *strcat();

#include <stdio.h>

#define TRUE	1
#define FALSE	0
#define SOH	001		/* SCCS lines start with SOH (Control-A) */
#define RCS	"rcs -q"
#define GET	"get -s"
#define CI	"ci -q -f"

#define prefix(a, b)	(strncmp(a, b, strlen(a)) == 0)
#define null(str)	((str) == NULL ? "<null>\n" : (str))

int
    trace = FALSE,	/* just show what would be done, don't run commands */
    verbose = FALSE;	/* Print commands before executing */

typedef struct delta
{
    char *revision;
    char *commentary;
    struct delta *next;
} DELTA;

typedef struct userlist
{
    char *user;
    struct userlist *next;
} USERLIST;

typedef struct header
{
    DELTA *deltas;
    USERLIST *userlist;
    char  *description;
} HEADER;


#if __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif
/*VARARGS*/
#if __STDC__
quit (char *fmt, ...)
#else
quit (va_alist)
va_dcl
#endif
{
#if !__STDC__
    char *fmt;
#endif
    va_list ap;

#if __STDC__
    va_start(ap, fmt);
#else
    va_start(ap);
    fmt = va_arg(ap, char *);
#endif
    VOID fprintf (stderr, "sccstorcs: ");
#ifdef	NO_VFPRINTF
    VOID _doprnt(fmt, ap, stderr);
#else
    VOID vfprintf(stderr, fmt, ap);
#endif
    va_end(ap);
    exit (1);
}

char *
xalloc (size)
unsigned size;
{
    extern char *malloc ();
    char *p;
    if ((p = malloc (size)) == NULL)
	quit ("Out of Memory.\n");
    return (p);
}

/*
 * Allocate space for string and copy str to it.
 */
char *
string (str)
char *str;
{
    register char *p = xalloc ((unsigned) (strlen (str) + 1));
    VOID strcpy (p, str);
    return (p);
}

/*
 * Return pointer to the final file name in a path.
 * I.e. sname ("/foo/baz/mumble") returns a pointer to "mumble".
 */
char *
sname (s)
register char *s;
{
    register char *p;

    for (p = s; *p;)
       if (*p++ == '/')
	   s = p;
    return (s);
}

DELTA *
new_delta (line)
char *line;
{
    register DELTA *delta;
    char rev[32];

    VOID sscanf (line, "%*s %*s %s", rev);
    delta = (DELTA *) xalloc (sizeof (DELTA));
    delta -> revision = string (rev);
    delta -> commentary = NULL;
    return (delta);
}

char *
concat (old_str, str)
char *old_str, *str;
{
    register int len;
    register char *newstring;

    if (old_str == NULL)
	return (string (str));

    len = strlen (old_str) + strlen (str);
    newstring = (char *) xalloc ((unsigned) (len + 1));
    VOID strcpy (newstring, old_str);
    VOID strcat (newstring, str);
    free (old_str);
    return (newstring);
}

trimtail (line)
char *line;
{
    register char *p = line;
    while (*p) p++;
    while (p > line && p[-1] <= ' ')
	p--;
    *p = '\0';
}

USERLIST *
collect_userlist (fd)
FILE *fd;
{
    char line[128];
    USERLIST *userlist = NULL, *newuser;
    while (fgets (line, sizeof line, fd))
    {
	if (line[0] == SOH && line[1] == 'U')	/* End of userlist */
	    break;
	trimtail (line);
	newuser = (USERLIST *) xalloc (sizeof (USERLIST));
	newuser -> user = string (line);
	newuser -> next = userlist;
	userlist = newuser;
    }
    return (userlist);
}

HEADER *
collect_header (fd)
FILE *fd;
{
    DELTA *head = NULL, *delta;
    USERLIST *userlist = NULL;
    static HEADER header;
    char line[512], *description = NULL;
    while (fgets (line, sizeof line, fd))
    {
	if (line[0] != SOH)
	    continue;
	if (line[1] == 'I')		/* The first INCLUDE */
	    break;
	switch (line[1])
	{
	    case 'd': 		       /* New delta */
#ifdef	PURDUE_EE
		if (line[3] == 'R')
		    while (fgets (line, sizeof line, fd))
			if (line[0] == SOH && line[1] == 'd' && line[3] != 'R')
			    break;
#endif
		delta = new_delta (line);
#ifdef	PURDUE_EE
		if (!head || strcmp(delta -> revision, head -> revision)) {
#endif
		    delta -> next = head;
		    head = delta;
#ifdef	PURDUE_EE
		}
#endif
#ifndef	PURDUE_EE
		break;
#endif
	    case 'c': 		       /* Commentary */
		delta -> commentary = concat (delta -> commentary, &line[3]);
		break;
	    case 'u':
		userlist = collect_userlist (fd);
		break;
	    case 't':
		while (fgets (line, sizeof line, fd) && !prefix("\1T", line))
		    description = concat (description, line);
	}
    }
    header.userlist = userlist;
    header.deltas = head;
    header.description = description;
    return (&header);
}

/*
 * Convert SCCS file to RCS file
 */
HEADER *
read_sccs (sccsfile)
char *sccsfile;
{
    HEADER *header;
    FILE *fd;
    if (strncmp (sname (sccsfile), "s.", 2) != 0)	/* An SCCS file? */
    {
	VOID fprintf (stderr, "%s: not an SCCS file.\n", sccsfile);
	return (NULL);
    }
    if ((fd = fopen (sccsfile, "r")) == NULL)
    {
	VOID fprintf (stderr, "%s: cannot open.\n", sccsfile);
	return (NULL);
    }
    header = collect_header (fd);
    VOID fclose (fd);
    return (header);
}

install_userlist (userlist, rcsfile)
register USERLIST *userlist;
char *rcsfile;
{
    char command[512];
    int count;
    if (userlist == NULL)
	return (0);
    VOID sprintf (command, "%s -a", RCS);
    for (count = 0; userlist; userlist = userlist -> next, count++)
    {
	if (count > 0)
	    VOID strcat (command, ",");
	VOID strcat (command, userlist -> user);
    }
    VOID strcat (command, " ");
    VOID strcat (command, rcsfile);
    if (trace || verbose)
	VOID printf ("%% %s\n", command);
    if (trace)
	return (0);
    return (system (command));
}

initialize_rcsfile (description, rcsfile)
char *description, *rcsfile;
{
    char command[512];
    extern FILE *popen();
    FILE *pd;

    VOID sprintf (command, "%s -i -U %s", RCS, rcsfile);
    if (trace || verbose)
	VOID printf ("%% %s\n", command);
    if (trace)
    {
	VOID printf ("Description:\n%s\n", null(description));
	return (0);
    }
    if ((pd = popen (command, "w")) == NULL)
	return (-1);
    VOID fprintf (pd, "%s", description ? description : "\n");
    return (pclose (pd));
}

install_deltas (delta, sccsfile, rcsfile)
register DELTA *delta;
char *sccsfile, *rcsfile;
{
    char command[512];
    for (; delta; delta = delta -> next)
    {
	/*
	 * Get the SCCS file.
	 */
	VOID sprintf (command, "%s -p -r%s %s > %s",
	    GET, delta -> revision, sccsfile, rcsfile);
	if (trace || verbose)
	    VOID printf("%% %s\n", command);
	if (!trace)
	{
	    if (system (command))
		return (-1);
	}

	VOID sprintf (command, "%s -r%s %s", CI, delta -> revision, rcsfile);
	if (trace || verbose)
	    VOID printf("%% %s\n", command);
	if (trace)
	    VOID printf("Commentary:\n%s\n", null(delta -> commentary));
	else
	{
	    extern FILE *popen ();
	    FILE *pd;
	    int x;
	    if ((pd = popen (command, "w")) == NULL)
		return (-1);
	    if (delta -> commentary)
		VOID fprintf (pd, delta -> commentary);
	    if ((x = pclose (pd)) != 0)
		return (x);
	}
    }
    return (0);
}

finalize_rcsfile (rcsfile)
char *rcsfile;
{
    char command[512];
    VOID sprintf (command, "%s -L %s", RCS, rcsfile);
    if (trace || verbose)
	VOID printf ("%% %s\n", command);
    if (trace)
	return (0);
    return (system (command));
}

build_new_rcs_file (header, sccsfile)
HEADER *header;
char *sccsfile;
{
    char *rcsfile = &(sname (sccsfile))[2];

    if (initialize_rcsfile (header -> description, rcsfile))
	quit ("Error initializing new rcs file %s\n", rcsfile);

    if (install_userlist (header -> userlist, rcsfile))
	quit ("Error installing user access list to rcs file %s\n", rcsfile);

    if (install_deltas (header -> deltas, sccsfile, rcsfile))
	quit ("Error installing delta to rcs file %s\n", rcsfile);

    if (finalize_rcsfile (rcsfile))
	quit ("Error setting defaults to rcs file %s\n", rcsfile);
}

print_header (sccsfile, header)
char *sccsfile;
register HEADER *header;
{
    register DELTA *d;
    register USERLIST *u;

    VOID printf ("\n%s:\n", sccsfile);
    VOID printf ("------------------------------------------------------------\n");
    if (header -> description)
	VOID printf ("Descriptive text:\n%s", header -> description);

    if (header -> userlist)
    {
	VOID printf ("\nUser access list:\n");
	for (u = header -> userlist; u; u = u -> next)
	    VOID printf ("%s\n", u -> user);
    }

    for (d = header -> deltas; d; d = d -> next)
    {
	VOID printf ("\nRelease: %s\n", d -> revision);
	VOID printf ("Commentary:\n%s", d -> commentary);
    }
    VOID printf ("------------------------------------------------------------\n");
}

main (argc, argv)
char **argv;
{
    int errors = 0;

    for (; argc > 1 && argv[1][0] == '-'; argc--, argv++)
    {
	switch (argv[1][1])
	{
	    case 'v':
		verbose = TRUE;
		break;
	    case 't': 
		trace = TRUE;
		break;
	    default: 
		VOID fprintf (stderr, "Unknown switch \"%s\".\n", argv[1]);
		exit (1);
	}
    }

    if (argc <= 1)
	quit ("Usage: sccstorcs [-t -v] s.file ...\n");

    for (; argc > 1; argc--, argv++)
    {
	HEADER *header;
	char *sccsfile;
	sccsfile = argv[1];
	if ((header = read_sccs (sccsfile)) != NULL)
	{
	    if (trace)
		print_header (sccsfile, header);
	    build_new_rcs_file (header, sccsfile);
	}
	else
	    errors++;
    }
    exit (errors);
}
