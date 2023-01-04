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
static char	*sccsid = "@(#)$RCSfile: symtab.c,v $ $Revision: 4.2.2.3 $ (DEC) $Date: 1992/11/05 13:54:41 $";
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

/*
 * These routines maintain the symbol table which tracks the state of the file
 * system being restored. They provide lookup by either name or inode number.
 * They also provide for creation, deletion, and renaming of entries. Because
 * of the dynamic nature of pathnames, names should not be saved, but always
 * constructed just before they are needed, by calling "myname".
 */

#include	"restore.h"
#include	<sys/stat.h>

/*
 * The following variables define the inode symbol table. The primary hash
 * table is dynamically allocated based on the number of inodes in the file
 * system (maxino), scaled by HASHFACTOR. The variable "entry" points to the
 * hash table; the variable "entrytblsize" indicates its size (in entries).
 */

#define	HASHFACTOR	5

static struct entry **entry;
static long	entrytblsize;

static void	removeentry();

/*
 * Look up an entry by inode number
 */

struct entry   *
lookupino(inum)
	ino_t		inum;
{
	register struct entry *ep;

	if (inum < ROOTINO || inum >= maxino)
	{
		return(NULL);
	}
	for (ep = entry[inum % entrytblsize]; ep != NULL; ep = ep->e_next)
	{
		if (ep->e_ino == inum)
		{
			return(ep);
		}
	}
	return(NULL);
}

/*
 * Add an entry into the entry table
 */

static void
addino(inum, np)
	ino_t		inum;
	struct entry   *np;
{
	struct entry  **epp;

	if (inum < ROOTINO || inum >= maxino)
	{
		panic(MSGSTR(ADDINO, "addino: out of range %d\n"), inum);
	}
	epp = &entry[inum % entrytblsize];
	np->e_ino = inum;
	np->e_next = *epp;
	*epp = np;
	if (debug_flag == TRUE)
	{
		for (np = np->e_next; np != NULL; np = np->e_next)
		{
			if (np->e_ino == inum)
			{
				badentry(np, MSGSTR(DUPINUM, "duplicate inum"));
			}
		}
	}
}

/*
 * Delete an entry from the entry table
 */

void
deleteino(inum)
	ino_t		inum;
{
	register struct entry *next;
	struct entry  **prev;

	if (inum < ROOTINO || inum >= maxino)
	{
		panic(MSGSTR(DELETINO, "deleteino: out of range %d\n"), inum);
	}
	prev = &entry[inum % entrytblsize];
	for (next = *prev; next != NULL; next = next->e_next)
	{
		if (next->e_ino == inum)
		{
			next->e_ino = 0;
			*prev = next->e_next;
			return;
		}
		prev = &next->e_next;
	}
	panic(MSGSTR(DELINON, "deleteino: %d not found\n"), inum);
}

/*
 * Look up an entry by name
 */

struct entry   *
lookupname(name)
	char	       *name;
{
	register struct entry *ep;
	register char  *np, *cp;
	char		buf[MAXPATHLEN];

	cp = name;
	for (ep = lookupino(ROOTINO); ep != NULL; ep = ep->e_entries)
	{
		for (np = buf; *cp != '/' && *cp != '\0';)
		{
			*np = *cp;
			++np, ++cp;
		}
		*np = '\0';
		for (; ep != NULL; ep = ep->e_sibling)
		{
			if (strcmp(ep->e_name, buf) == 0)
			{
				break;
			}
		}
		if (ep == NULL)
		{
			break;
		}
		if (*cp == '\0')
		{
			return(ep);
		}
		++cp;
	}
	return(NULL);
}

/*
 * Look up the parent of a pathname
 */

struct entry   *
lookupparent(name)
	char	       *name;
{
	struct entry   *ep;
	char	       *tailindex;

	tailindex = rindex(name, '/');
	if (tailindex == 0)
	{
		return(NULL);
	}
	*tailindex = '\0';
	ep = lookupname(name);
	*tailindex = '/';
	if (ep == NULL)
	{
		return(NULL);
	}
	if (ep->e_type != NODE)
	{
		panic(MSGSTR(NOTDIR, "%s is not a directory\n"), name);
	}
	return(ep);
}

/*
 * Determine the current pathname of a node or leaf
 */

char	       *
myname(ep)
	register struct entry *ep;
{
	register char  *cp;
	static char	namebuf[MAXPATHLEN];

	for (cp = &namebuf[MAXPATHLEN - 2]; cp > &namebuf[ep->e_namlen];)
	{
		cp -= (long)ep->e_namlen;
		memcpy(cp, ep->e_name, (size_t) ep->e_namlen);
		if (ep == lookupino(ROOTINO))
		{
			return(cp);
		}
		--cp;
		*cp = '/';
		ep = ep->e_parent;
	}
	panic(MSGSTR(PATHMAX, "%s: pathname too long\n"), cp);
	return(cp);
}

/*
 * Unused symbol table entries are linked together on a freelist headed by the
 * following pointer.
 */

static struct entry *freelist = NULL;

/*
 * add an entry to the symbol table
 */

struct entry   *
addentry(name, inum, type)
	char	       *name;
	ino_t		inum;
	int		type;
{
	register struct entry *np, *ep;

	if (freelist != NULL)
	{
		np = freelist;
		freelist = np->e_next;
		memset((char *) np, (int) '\0', sizeof(struct entry));
	}
	else
	{
		np = (struct entry *) calloc(1, sizeof(struct entry));
	}
	np->e_type = type & ~LINK;
	ep = lookupparent(name);
	if (ep == NULL)
	{
		if (inum != ROOTINO || lookupino(ROOTINO) != NULL)
		{
			panic(MSGSTR(BADNAMEA, "bad name to addentry %s\n"), name);
		}
		np->e_name = strdup(name);
		np->e_namlen = strlen(name);
		np->e_parent = np;
		addino(ROOTINO, np);
		return(np);
	}
	np->e_name = strdup(rindex(name, '/') + 1);
	np->e_namlen = strlen(np->e_name);
	np->e_parent = ep;
	np->e_sibling = ep->e_entries;
	ep->e_entries = np;
	if (type & LINK)
	{
		ep = lookupino(inum);
		if (ep == NULL)
		{
			panic(MSGSTR(LNNONEXT, "link to non-existant name\n"));
		}
		np->e_ino = inum;
		np->e_links = ep->e_links;
		ep->e_links = np;
	}
	else if (inum != 0)
	{
		if (lookupino(inum) != NULL)
		{
			panic(MSGSTR(DUPENT, "duplicate entry\n"));
		}
		addino(inum, np);
	}
	return(np);
}

/*
 * delete an entry from the symbol table
 */

void
freeentry(ep)
	register struct entry *ep;
{
	register struct entry *np;
	ino_t		inum;

	if (ep->e_flags != REMOVED)
	{
		badentry(ep, MSGSTR(NOTMARKR, "not marked REMOVED"));
	}
	if (ep->e_type == NODE)
	{
		if (ep->e_links != NULL)
		{
			badentry(ep, MSGSTR(FREEDIR, "freeing referenced directory"));
		}
		if (ep->e_entries != NULL)
		{
			badentry(ep, MSGSTR(FREENOND, "freeing non-empty directory"));
		}
	}
	if (ep->e_ino != 0)
	{
		np = lookupino(ep->e_ino);
		if (np == NULL)
		{
			badentry(ep, MSGSTR(LOOKINO, "lookupino failed"));
		}
		if (np == ep)
		{
			inum = ep->e_ino;
			deleteino(inum);
			if (ep->e_links != NULL)
			{
				addino(inum, ep->e_links);
			}
		}
		else
		{
			for (; np != NULL; np = np->e_links)
			{
				if (np->e_links == ep)
				{
					np->e_links = ep->e_links;
					break;
				}
			}
			if (np == NULL)
			{
				badentry(ep, MSGSTR(LINKNOF, "link not found"));
			}
		}
	}
	removeentry(ep);
	free(ep->e_name);
	ep->e_next = freelist;
	freelist = ep;
}

/*
 * Relocate an entry in the tree structure
 */

void
moveentry(ep, newname)
	register struct entry *ep;
	char	       *newname;
{
	struct entry   *np;
	char	       *cp;

	np = lookupparent(newname);
	if (np == NULL)
	{
		badentry(ep, MSGSTR(CANTROOT, "cannot move ROOT"));
	}
	if (np != ep->e_parent)
	{
		removeentry(ep);
		ep->e_parent = np;
		ep->e_sibling = np->e_entries;
		np->e_entries = ep;
	}
	cp = rindex(newname, '/') + 1;
	free(ep->e_name);
	ep->e_name = strdup(cp);
	ep->e_namlen = strlen(cp);
	if (strcmp(gentempname(ep), ep->e_name) == 0)
	{
		ep->e_flags |= TMPNAME;
	}
	else
	{
		ep->e_flags &= ~TMPNAME;
	}
}

/*
 * Remove an entry in the tree structure
 */

static void
removeentry(ep)
	register struct entry *ep;
{
	register struct entry *np;

	np = ep->e_parent;
	if (np->e_entries == ep)
	{
		np->e_entries = ep->e_sibling;
	}
	else
	{
		for (np = np->e_entries; np != NULL; np = np->e_sibling)
		{
			if (np->e_sibling == ep)
			{
				np->e_sibling = ep->e_sibling;
				break;
			}
		}
		if (np == NULL)
		{
			badentry(ep, MSGSTR(PALIST, "cannot find entry in parent list"));
		}
	}
}

/*
 * Useful quantities placed at the end of a dumped symbol table.
 */

struct symtableheader
{
	long		volno;
	long		stringsize;
	long		entrytblsize;
	time_t		dumptime;
	time_t		dumpdate;
	ino_t		maxino;
	long		ntrec;
};

/*
 * dump a snapshot of the symbol table
 */

void
dumpsymtable(filename, checkpt)
	char	       *filename;
	long		checkpt;
{
	register struct entry *ep, *tep;
	register ino_t	i;
	struct entry	temp, *tentry;
	long		mynum = 1, stroff = 0;
	FILE	       *fd;
	struct symtableheader hdr;

	vmsg(MSGSTR(CHECKPT, "Check pointing the restore\n"));
	if (Nflag == TRUE)
	{
		return;
	}
	if ((fd = fopen(filename, "w")) == NULL)
	{
		perror("dumpsymtable(): open()");
		panic(MSGSTR(SAVEFIL, "cannot create save file %s for symbol table\n"), filename);
	}
	clearerr(fd);

	/*
	 * Assign indicies to each entry Write out the string entries
	 */

	for (i = ROOTINO; i < maxino; ++i)
	{
		for (ep = lookupino(i); ep != NULL; ep = ep->e_links)
		{
			ep->e_index = mynum;
			++mynum;
			(void) fwrite(ep->e_name, sizeof(char), ep->e_namlen, fd);
		}
	}

	/*
	 * Convert pointers to indexes, and output
	 */

	tep = &temp;
	stroff = 0;
	for (i = ROOTINO; i < maxino; ++i)
	{
		for (ep = lookupino(i); ep != NULL; ep = ep->e_links)
		{
			memcpy((char *) tep, (char *) ep, (size_t) sizeof(struct entry));
			tep->e_name = (char *) stroff;
			stroff += ep->e_namlen;
			tep->e_parent = (struct entry *) ep->e_parent->e_index;
			if (ep->e_links != NULL)
			{
				tep->e_links = (struct entry *) ep->e_links->e_index;
			}
			if (ep->e_sibling != NULL)
			{
				tep->e_sibling = (struct entry *) ep->e_sibling->e_index;
			}
			if (ep->e_entries != NULL)
			{
				tep->e_entries = (struct entry *) ep->e_entries->e_index;
			}
			if (ep->e_next != NULL)
			{
				tep->e_next = (struct entry *) ep->e_next->e_index;
			}
			(void) fwrite((char *) tep, sizeof(struct entry), 1, fd);
		}
	}

	/*
	 * Convert entry pointers to indexes, and output
	 */

	for (i = 0; i < entrytblsize; ++i)
	{
		if (entry[i] == NULL)
		{
			tentry = NULL;
		}
		else
		{
			tentry = (struct entry *) entry[i]->e_index;
		}
		(void) fwrite((char *) &tentry, sizeof(struct entry *), 1, fd);
	}
	hdr.volno = checkpt;
	hdr.maxino = maxino;
	hdr.entrytblsize = entrytblsize;
	hdr.stringsize = stroff;
	hdr.dumptime = dumptime;
	hdr.dumpdate = dumpdate;
	hdr.ntrec = ntrec;
	(void) fwrite((char *) &hdr, sizeof(struct symtableheader), 1, fd);
	if (ferror(fd))
	{
		perror("dumpsymtable(): fwrite()");
		panic(MSGSTR(EWRSYMF, "output error to file %s writing symbol table\n"), filename);
	}
	if (fclose(fd) != 0)
	{
		perror("dumpsymtable(): fclose()");
		panic(MSGSTR(EWRSYMF, "output error to file %s writing symbol table\n"), filename);
	}
}

/*
 * Initialize a symbol table from a file
 */

void
initsymtable(filename)
	char	       *filename;
{
	char	       *base;
	long		tblsize;
	register struct entry *ep;
	struct entry   *baseep, *lep;
	struct symtableheader hdr;
	struct stat	stbuf;
	register long	i;
	int		fd;

	vmsg(MSGSTR(INITSYM, "Initialize symbol table.\n"));
	if (filename == NULL)
	{
		entrytblsize = maxino / HASHFACTOR;
		entry = (struct entry **) calloc((unsigned) entrytblsize, sizeof(struct entry *));
		ep = addentry(".", ROOTINO, NODE);
		ep->e_flags |= NEW;
		return;
	}
	if ((fd = open(filename, O_RDONLY)) < 0)
	{
		perror("initsymtable(): open()");
		panic(MSGSTR(CANTOSYMF, "cannot open symbol table file %s\n"), filename);
	}
	if (fstat(fd, &stbuf) < 0)
	{
		perror("initsymtable(): fstat()");
		panic(MSGSTR(CANTSTSYF, "cannot stat symbol table file %s\n"), filename);
	}
	tblsize = stbuf.st_size - sizeof(struct symtableheader);
	base = calloc(sizeof(char), (unsigned) tblsize);
	if (read(fd, base, (int) tblsize) < 0 ||
	    read(fd, (char *) &hdr, sizeof(struct symtableheader)) < 0)
	{
		perror("initsymtable(): read()");
		panic(MSGSTR(SYMREAD, "cannot read symbol table file %s\n"), filename);
	}
	switch (command)
	{
		/*
		 * For normal continuation, insure that we are using the next
		 * incremental tape
		 */

	case 'r':
		if (hdr.dumpdate != dumptime)
		{
			if (hdr.dumpdate < dumptime)
			{
				msg(MSGSTR(INCLOW, "Incremental tape too low\n"));
			}
			else
			{
				msg(MSGSTR(INCHIGH, "Incremental tape too high\n"));
			}
			Exit(1);

			/* NOTREACHED */
		}
		break;

		/*
		 * For restart, insure that we are using the same tape
		 */

	case 'R':
		curr_action = SKIP;
		dumptime = hdr.dumptime;
		dumpdate = hdr.dumpdate;
		if (block_size_flag == FALSE)
		{
			newtapebuf(hdr.ntrec);
		}
		getvol(hdr.volno);
		break;

	default:
		panic(MSGSTR(INITSYC, "initsymtable called from command %c\n"), command);
		break;
	}
	maxino = hdr.maxino;
	entrytblsize = hdr.entrytblsize;
	entry = (struct entry **) (base + tblsize - (entrytblsize * sizeof(struct entry *)));
	baseep = (struct entry *) (base + hdr.stringsize - sizeof(struct entry));
	lep = (struct entry *) entry;
	for (i = 0; i < entrytblsize; ++i)
	{
		if (entry[i] == NULL)
		{
			continue;
		}
		entry[i] = &baseep[(long) entry[i]];
	}
	for (ep = &baseep[1]; ep < lep; ++ep)
	{
		ep->e_name = base + (long) ep->e_name;
		ep->e_parent = &baseep[(long) ep->e_parent];
		if (ep->e_sibling != NULL)
		{
			ep->e_sibling = &baseep[(long) ep->e_sibling];
		}
		if (ep->e_links != NULL)
		{
			ep->e_links = &baseep[(long) ep->e_links];
		}
		if (ep->e_entries != NULL)
		{
			ep->e_entries = &baseep[(long) ep->e_entries];
		}
		if (ep->e_next != NULL)
		{
			ep->e_next = &baseep[(long) ep->e_next];
		}
	}
}
