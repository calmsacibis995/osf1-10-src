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
static char	*sccsid = "@(#)$RCSfile: utilities.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/11/10 21:57:40 $";
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
#if !defined( lint) && !defined(_NOIDENT)

#endif

/*
 * This module contains IBM CONFIDENTIAL code. -- (IBM Confidential Restricted
 * when combined with the aggregated modules for this product) OBJECT CODE ONLY
 * SOURCE MATERIALS (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or disclosure
 * restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * Copyright (c) 1983 Regents of the University of California. All
 * rights reserved.  The Berkeley software License Agreement specifies the
 * terms and conditions for redistribution.
 */

#include	"restore.h"
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/time.h>
#include	<sys/errno.h>
#include	<sys/syscall.h>

/*
 * Insure that all the components of a pathname exist.
 */

void
pathcheck(name)
	register char  *name;
{
	register int	i;
	struct entry   *ep;

	if (strchr(name, '/') == NULL)
	{
		return;
	}

	for (i = 0; name[i] != '\0'; ++i)
	{
		if (name[i] == '/')
		{
			name[i] = '\0';
			ep = lookupname(name);
			if (ep == NULL)
			{
				ep = addentry(name, psearch(name), NODE);
				newnode(ep);
			}
			ep->e_flags |= NEW | KEEP;
			name[i] = '/';
		}
	}
}

/*
 * Change a name to a unique temporary name.
 */

void
mktempname(ep)
	register struct entry *ep;
{
	char		oldname[MAXPATHLEN];

	if (ep->e_flags & TMPNAME)
	{
		badentry(ep, MSGSTR(MKTEMP, "mktempname: called with TMPNAME"));
	}
	ep->e_flags |= TMPNAME;
	(void) strcpy(oldname, myname(ep));
	free(ep->e_name);
	ep->e_name = strdup(gentempname(ep));
	ep->e_namlen = strlen(ep->e_name);
	renameit(oldname, myname(ep));
}

/*
 * Generate a temporary name for an entry.
 */

char	       *
gentempname(ep)
	struct entry   *ep;
{
	static char	name[MAXPATHLEN];
	struct entry   *np;
	int		i;

	for (np = lookupino(ep->e_ino), i = 0; np != NULL; np = np->e_links, ++i)
	{
		if (np == ep)
		{
			break;
		}
	}
	if (np == NULL)
	{
		badentry(ep, MSGSTR(NOTILIST, "not on ino list"));
	}
	(void) sprintf(name, "%s%d%d", TMPHDR, i, ep->e_ino);
	return(name);
}

/*
 * Rename a file or directory.
 */

void
renameit(from, to)
	char	       *from, *to;
{
	if (Nflag == FALSE && rename(from, to) < 0)
	{
		msg(MSGSTR(CANTRES, "Warning: cannot rename %s to %s"), from, to);
		perror("");
		return;
	}
	vmsg(MSGSTR(RENAME, "rename %s to %s\n"), from, to);
}

/*
 * Create a new node (directory).
 */

void
newnode(np)
	struct entry   *np;
{
	char	       *cp;

	if (np->e_type != NODE)
	{
		badentry(np, MSGSTR(NEWNODE, "newnode: not a node"));
	}
	cp = myname(np);
	if (Nflag  == FALSE && mkdir(cp, 0777) < 0)
	{
		np->e_flags |= EXISTED;
		msg(MSGSTR(WARNING, "Warning: "));
		perror(cp);
		return;
	}
	vmsg(MSGSTR(MAKENODE, "Make node %s\n"), cp);
}

/*
 * Remove an old node (directory).
 */

void
removenode(ep)
	register struct entry *ep;
{
	char	       *cp;

	if (ep->e_type != NODE)
	{
		badentry(ep, MSGSTR(REMNODE, "removenode: not a node"));
	}
	if (ep->e_entries != NULL)
	{
		badentry(ep, MSGSTR(REMNONE, "removenode: non-empty directory"));
	}
	ep->e_flags |= REMOVED;
	ep->e_flags &= ~TMPNAME;
	cp = myname(ep);
	if (Nflag  == FALSE && rmdir(cp) < 0)
	{
		msg(MSGSTR(WARNING, "Warning: "));
		perror(cp);
		return;
	}
	vmsg(MSGSTR(REMOVEN, "Remove node %s\n"), cp);
}

/*
 * Remove a leaf.
 */

void
removeleaf(ep)
	register struct entry *ep;
{
	char	       *cp;

	if (ep->e_type != LEAF)
	{
		badentry(ep, MSGSTR(REMLEAF, "removeleaf: not a leaf"));
	}
	ep->e_flags |= REMOVED;
	ep->e_flags &= ~TMPNAME;
	cp = myname(ep);
	if (Nflag == FALSE && unlink(cp) < 0)
	{
		msg(MSGSTR(WARNING, "Warning: "));
		perror(cp);
		return;
	}
	vmsg(MSGSTR(REMLEAF2, "Remove leaf %s\n"), cp);
}

/*
 * Create a link.
 */

int
linkit(existing, new, type)
	char	       *existing, *new;
	int		type;
{
	if (type == SYMLINK)
	{
		if (Nflag == FALSE && symlink(existing, new) < 0)
		{

/* Changes made 11/3/93 for QAR 16463 by William_Soreth
   To enable restore to overwrite symbolic and hard links 
   when the overwrite_always flag is set, and only then
   so as not to break currently existing scripts 
*/

		if (overwrite_flag == OVERWRITE_ALWAYS && Nflag == FALSE)
			{
		if (unlink(new) < 0 )
				{
		vmsg("couldn't OVERWRITE existing file %s",new);
		msg(MSGSTR(SYMLNW, "Warning: cannot OVERWRITE existing file %s: "), new);
                        perror("");
			return(FAIL);
				}
		vmsg("overwriting existing file %s with symbolic link %s->%s \n",existing,new,existing);
		if (Nflag == FALSE && symlink(existing, new) < 0)
					{
		msg(MSGSTR(SYMLNW, "Warning: cannot OVERWRITE existing file %s with symbolic link %s->%s: "),existing, new, existing);
                        perror("");
                        return(FAIL);
					}
			}
			else
			{

/* end of changes for QAR 16463 William_Soreth */

			msg(MSGSTR(SYMLNW, "Warning: cannot create symbolic link %s->%s: "), new, existing);
			perror("");
			return(FAIL);
			}
		}

	}
	else if (type == HARDLINK)
	{
		struct stat	old_stats;
		struct timeval	restore_times[3];

		if (stat(existing, &old_stats) < 0)
		{
			restore_times[0].tv_sec = 0;
		}
		else
		{
		 	restore_times[0].tv_sec = old_stats.st_atime;
		 	restore_times[1].tv_sec = old_stats.st_mtime;
		 	restore_times[2].tv_sec = old_stats.st_ctime;
		}

		if (Nflag == FALSE && link(existing, new) < 0)
		{

/* Changes made 11/3/93 for QAR 16463 by William_Soreth
   To enable restore to overwrite symbolic and hard links 
   when the overwrite_always flag is set, and only then
   so as not to break currently existing scripts 
*/
		if (overwrite_flag == OVERWRITE_ALWAYS && Nflag == FALSE)
			{
		if (unlink(new) < 0 )
				{
		vmsg("couldn't overwrite existing file %s",existing);
		msg(MSGSTR(SYMLNW, "Warning: cannot OVERWRITE existing file %s with hard link %s->%s: "),existing, new, existing);
                        perror("");
			return(FAIL);
				}
		vmsg("overwriting existing file %s with hard link %s->%s \n",existing,new,existing);
		if (Nflag == FALSE && link(existing, new) < 0)
					{
		msg(MSGSTR(SYMLNW, "Warning: cannot OVERWRITE existing file %s with hard link %s->%s: "),existing, new, existing);
                        perror("");
                        return(FAIL);
					}
			}
			else
			{ 

/* end of changes for QAR 16463 William_Soreth */

			msg(MSGSTR(HARDLNW, "Warning: cannot create hard link %s->%s: "), new, existing);
			perror("");
			return(FAIL);
			}
		}
		else if (Nflag  == FALSE && restore_times[0].tv_sec != 0)
		{
			(void) xutimes(existing, restore_times);
		}
	}
	else
	{
		panic(MSGSTR(LINKIT, "linkit: unknown type %d\n"), type);
		return(FAIL);
	}
	vmsg(MSGSTR(CRLN, "Create %s link %s->%s\n"),
	     (type == SYMLINK)? MSGSTR(SYMBOLC, "symbolic"): MSGSTR(HARD, "hard"),
	     new, existing);
	return(GOOD);
}

/*
 * find lowest number file (above "start") that needs to be extracted
 */

ino_t
lowerbnd(start)
	ino_t		start;
{
	register struct entry *ep;

	for (; start < maxino; ++start)
	{
		ep = lookupino(start);
		if (ep == NULL || ep->e_type == NODE)
		{
			continue;
		}
		if (ep->e_flags & (NEW | EXTRACT))
		{
			return(start);
		}
	}
	return(start);
}

/*
 * find highest number file (below "start") that needs to be extracted
 */

ino_t
upperbnd(start)
	ino_t		start;
{
	register struct entry *ep;

	for (; start > ROOTINO; --start)
	{
		ep = lookupino(start);
		if (ep == NULL || ep->e_type == NODE)
		{
			continue;
		}
		if (ep->e_flags & (NEW | EXTRACT))
		{
			return(start);
		}
	}
	return(start);
}

/*
 * report on a badly formed entry
 */

void
badentry(ep, message)
	register struct entry *ep;
	char	       *message;
{

	msg(MSGSTR(BADENTRY, "bad entry: %s\n"), message);
	msg(MSGSTR(NAME, "name: %s\n"), myname(ep));
	msg(MSGSTR(PANAME, "parent name %s\n"), myname(ep->e_parent));
	if (ep->e_sibling != NULL)
	{
		msg(MSGSTR(SIBLING, "sibling name: %s\n"), myname(ep->e_sibling));
	}
	if (ep->e_entries != NULL)
	{
		msg(MSGSTR(NEXTENT, "next entry name: %s\n"), myname(ep->e_entries));
	}
	if (ep->e_links != NULL)
	{
		msg(MSGSTR(NEXTLNN, "next link name: %s\n"), myname(ep->e_links));
	}
	if (ep->e_next != NULL)
	{
		msg(MSGSTR(NEXTHASH, "next hashchain name: %s\n"), myname(ep->e_next));
	}
	msg(MSGSTR(ENTRYTYP, "entry type: %s\n"),
	  ep->e_type == NODE ? MSGSTR(NODEE, "NODE") : MSGSTR(LEAFN, "LEAF"));
	msg(MSGSTR(INODEN, "inode number: %ld\n"), ep->e_ino);
	panic(MSGSTR(FLAGS, "flags: %s\n"), flagvalues(ep));
}

/*
 * Construct a string indicating the active flag bits of an entry.
 */

char	       *
flagvalues(ep)
	register struct entry *ep;
{
	static char	flagbuf[BUFSIZ];

	(void) strcpy(flagbuf, "|NULL");
	flagbuf[0] = '\0';
	if (ep->e_flags & REMOVED)
	{
		(void) strcat(flagbuf, "|REMOVED");
	}
	if (ep->e_flags & TMPNAME)
	{
		(void) strcat(flagbuf, "|TMPNAME");
	}
	if (ep->e_flags & EXTRACT)
	{
		(void) strcat(flagbuf, "|EXTRACT");
	}
	if (ep->e_flags & NEW)
	{
		(void) strcat(flagbuf, "|NEW");
	}
	if (ep->e_flags & KEEP)
	{
		(void) strcat(flagbuf, "|KEEP");
	}
	if (ep->e_flags & EXISTED)
	{
		(void) strcat(flagbuf, "|EXISTED");
	}
	return(&flagbuf[1]);
}

/*
 * Check to see if a name is on a dump tape.
 */

ino_t
dirlookup(name)
	char	       *name;
{
	ino_t		ino;

	ino = psearch(name);
	if (ino == 0 || !MAPBITTEST(dumpmap, ino))
	{
		msg(MSGSTR(NOTONTAPE, "%s: not found on the tape\n"), name);
	}
	return(ino);
}

/*
 * Elicit a reply.
 */

int
query(question)
	char	       *question;
{
	char		c;

	do
	{
		msg(MSGSTR(YESNO, "%s? [yn] "), question);
		c = getc(command_fp);
		while (c != '\n' && getc(command_fp) != '\n')
		{
			if (feof(command_fp))
			{
				return(NO);
			}
		}
	} while (c != 'y' && c != 'n');
	if (c == 'y')
	{
		return(YES);
	}
	return(NO);
}

/*
 * handle unexpected inconsistencies
 */

/* VARARGS1 */

void
panic(message, d1, d2)
	char	       *message;
	long		d1, d2;
{
	msg(message, d1, d2);
	if (query(MSGSTR(ABORT, "abort")) == YES)
	{
		if (query(MSGSTR(CORE, "dump core")) == YES)
		{
			abort();
		}
		Exit(1);

		/* NOTREACHED */
	}
}

void
xutimes(name, tv)
	char	       *name;
	struct timeval	tv[3];
{
#ifdef SYS_xutimes
	static int	mode = -1;

	if (mode < 0)
	{
		void	(*savesig)();

		savesig = signal(SIGSYS, SIG_IGN);
		++mode;
		if (syscall(SYS_xutimes, name, tv) < 0 && errno == EINVAL)
		{
			++mode;
		}
		(void) signal(SIGSYS, savesig);
	}
	if (mode == 0)
		syscall(SYS_xutimes, name, tv);
	else
#endif
		utimes(name, tv);
}
