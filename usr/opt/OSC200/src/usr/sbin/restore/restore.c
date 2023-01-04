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
static char	*sccsid = "@(#)$RCSfile: restore.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 06:25:20 $";
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

/*
 * This implements the 't' option. List entries on the tape.
 */

long
listfile(name, ino, type)
	char	       *name;
	ino_t		ino;
	int		type;
{
	long		descend = (children_flag == TRUE)? GOOD : FAIL;

	if (!MAPBITTEST(dumpmap, ino))
	{
		return(descend);
	}
	vmsg("%s", type == LEAF ? "leaf" : "dir ");
	msg("%10d\t%s\n", ino, name);
	return(descend);
}

/*
 * This implements the 'x' option. Request that new entries be extracted.
 */

long
addfile(name, ino, type)
	char	       *name;
	ino_t		ino;
	int		type;
{
	register struct entry *ep;
	long		descend = (children_flag == TRUE)? GOOD : FAIL;
	char		buf[100];

	if (!MAPBITTEST(dumpmap, ino))
	{
		dmsg(MSGSTR(NOTONTAPE, "%s: not found on the tape\n"), name);
		return(descend);
	}
	if (by_name_flag == FALSE)
	{
		(void) sprintf(buf, "./%u", ino);
		name = buf;
		if (type == NODE)
		{
			(void) genliteraldir(name, ino);
			return(descend);
		}
	}
	ep = lookupino(ino);
	if (ep != NULL)
	{
		if (strcmp(name, myname(ep)) == 0)
		{
			ep->e_flags |= NEW;
			return(descend);
		}
		type |= LINK;
	}
	ep = addentry(name, ino, type);
	if (type == NODE)
	{
		newnode(ep);
	}
	ep->e_flags |= NEW;
	return(descend);
}

/*
 * This is used by the 'i' option to undo previous requests made by addfile.
 * Delete entries from the request queue.
 */

/* ARGSUSED */

long
deletefile(name, ino, type)
	char	       *name;
	ino_t		ino;
	int		type;
{
	long		descend = (children_flag == TRUE)? GOOD : FAIL;
	struct entry   *ep;

	if (!MAPBITTEST(dumpmap, ino))
	{
		return(descend);
	}
	ep = lookupino(ino);
	if (ep != NULL)
	{
		ep->e_flags &= ~NEW;
	}
	return(descend);
}

/*
 * The following four routines implement the incremental restore algorithm. The
 * first removes old entries, the second does renames and calculates the
 * extraction list, the third cleans up link names missed by the first two, and
 * the final one deletes old directories.
 *
 * Directories cannot be immediately deleted, as they may have other
 * files in them which need to be moved out first. As directories
 * to be deleted are found, they are put on the following deletion list.
 * After all deletions and renames are done, this list is actually deleted.
 */

static struct entry *removelist;

/*
 * Remove unneeded leaves from the old tree. Remove directories from the lookup
 * chains.
 */

void
removeoldleaves()
{
	register struct entry *ep;
	register ino_t	i;

	vmsg(MSGSTR(MARKREM, "Mark entries to be removed.\n"));
	for (i = ROOTINO + 1; i < maxino; ++i)
	{
		ep = lookupino(i);
		if (ep == NULL)
		{
			continue;
		}
		if (MAPBITTEST(clrimap, i))
		{
			continue;
		}
		for (; ep != NULL; ep = ep->e_links)
		{
			dmsg("%s: REMOVE\n", myname(ep));
			if (ep->e_type == LEAF)
			{
				removeleaf(ep);
				freeentry(ep);
			}
			else
			{
				mktempname(ep);
				deleteino(ep->e_ino);
				ep->e_next = removelist;
				removelist = ep;
			}
		}
	}
}

/*
 * For each directory entry on the incremental tape, determine which category
 * it falls into as follows: KEEP - entries that are to be left alone. NEW -
 * new entries to be added. EXTRACT - files that must be updated with new
 * contents. LINK - new links to be added. Renames are done at the same time.
 */

/* key values */

#define ONTAPE	0x1	/* inode is on the tape */
#define INOFND	0x2	/* inode already exists */
#define NAMEFND	0x4	/* name already exists */
#define MODECHG	0x8	/* mode of inode changed */

long
nodeupdates(name, ino, type)
	char	       *name;
	ino_t		ino;
	int		type;
{
	register struct entry *ep, *np, *ip;
	long		descend = GOOD;
	int		lookuptype = 0;
	int		key = 0;

	/*
	 * This routine is called once for each element in the directory
	 * hierarchy, with a full path name. The "type" value is incorrectly
	 * specified as LEAF for directories that are not on the dump tape.
	 *
	 * Check to see if the file is on the tape.
	 */

	if (MAPBITTEST(dumpmap, ino))
	{
		key |= ONTAPE;
	}

	/*
	 * Check to see if the name exists, and if the name is a link.
	 */

	np = lookupname(name);
	if (np != NULL)
	{
		key |= NAMEFND;
		ip = lookupino(np->e_ino);
		if (ip == NULL)
		{
			panic(MSGSTR(CORSYM, "corrupted symbol table\n"));
		}
		if (ip != np)
		{
			lookuptype = LINK;
		}
	}

	/*
	 * Check to see if the inode exists, and if one of its links
	 * corresponds to the name (if one was found).
	 */

	ip = lookupino(ino);
	if (ip != NULL)
	{
		key |= INOFND;
		for (ep = ip->e_links; ep != NULL; ep = ep->e_links)
		{
			if (ep == np)
			{
				ip = ep;
				break;
			}
		}
	}

	/*
	 * If both a name and an inode are found, but they do not correspond to
	 * the same file, then both the inode that has been found and the inode
	 * corresponding to the name that has been found need to be renamed.
	 * The current pathname is the new name for the inode that has been
	 * found. Since all files to be deleted have already been removed, the
	 * named file is either a now unneeded link, or it must live under a
	 * new name in this dump level. If it is a link, it can be removed. If
	 * it is not a link, it is given a temporary name in anticipation that
	 * it will be renamed when it is later found by inode number.
	 */

	if (((key & (INOFND | NAMEFND)) == (INOFND | NAMEFND)) && ip != np)
	{
		if (lookuptype == LINK)
		{
			removeleaf(np);
			freeentry(np);
		}
		else
		{
			dmsg("name/inode conflict, mktempname %s\n",
				myname(np));
			mktempname(np);
		}
		np = NULL;
		key &= ~NAMEFND;
	}
	if ((key & ONTAPE) &&
	    (((key & INOFND) && ip->e_type != type) ||
	     ((key & NAMEFND) && np->e_type != type)))
	{
		key |= MODECHG;
	}

	/*
	 * Decide on the disposition of the file based on its flags. Note that
	 * we have already handled the case in which a name and inode are found
	 * that correspond to different files. Thus if both NAMEFND and INOFND
	 * are set then ip == np.
	 */

	switch (key)
	{
		/*
		 * A previously existing file has been found. Mark it as KEEP
		 * so that other links to the inode can be detected, and so
		 * that it will not be reclaimed by the search for unreferenced
		 * names.
		 */

	case INOFND | NAMEFND:
		ip->e_flags |= KEEP;
		dmsg("[%s] %s: %s\n", keyval(key), name, flagvalues(ip));
		break;

		/*
		 * A file on the tape has a name which is the same as a name
		 * corresponding to a different file in the previous dump.
		 * Since all files to be deleted have already been removed,
		 * this file is either a now unneeded link, or it must live
		 * under a new name in this dump level. If it is a link, it can
		 * simply be removed. If it is not a link, it is given a
		 * temporary name in anticipation that it will be renamed when
		 * it is later found by inode number (see INOFND case below).
		 * The entry is then treated as a new file.
		 */

	case ONTAPE | NAMEFND:
	case ONTAPE | NAMEFND | MODECHG:
		if (lookuptype == LINK)
		{
			removeleaf(np);
			freeentry(np);
		}
		else
		{
			mktempname(np);
		}

		/* fall through */

		/*
		 * A previously non-existent file. Add it to the file system,
		 * and request its extraction. If it is a directory, create it
		 * immediately. (Since the name is unused there can be no
		 * conflict)
		 */

	case ONTAPE:
		ep = addentry(name, ino, type);
		if (type == NODE)
		{
			newnode(ep);
		}
		ep->e_flags |= NEW | KEEP;
		dmsg("[%s] %s: %s\n", keyval(key), name, flagvalues(ep));
		break;

		/*
		 * A file with the same inode number, but a different name has
		 * been found. If the other name has not already been found
		 * (indicated by the KEEP flag, see above) then this must be a
		 * new name for the file, and it is renamed. If the other name
		 * has been found then this must be a link to the file. Hard
		 * links to directories are not permitted, and are either
		 * deleted or converted to symbolic links. Finally, if the file
		 * is on the tape, a request is made to extract it.
		 */

	case ONTAPE | INOFND:
		if (type == LEAF && (ip->e_flags & KEEP) == 0)
		{
			ip->e_flags |= EXTRACT;
		}

		/* fall through */

	case INOFND:
		if ((ip->e_flags & KEEP) == 0)
		{
			renameit(myname(ip), name);
			moveentry(ip, name);
			ip->e_flags |= KEEP;
			dmsg("[%s] %s: %s\n", keyval(key), name, flagvalues(ip));
			break;
		}
		if (ip->e_type == NODE)
		{
			descend = FAIL;
			msg(MSGSTR(DELLN, "deleted hard link %s to directory %s\n"), name, myname(ip));
			break;
		}
		ep = addentry(name, ino, type | LINK);
		ep->e_flags |= NEW;
		dmsg("[%s] %s: %s|LINK\n", keyval(key), name, flagvalues(ep));
		break;

		/*
		 * A previously known file which is to be updated.
		 */

	case ONTAPE | INOFND | NAMEFND:
		if (type == LEAF && lookuptype != LINK)
		{
			np->e_flags |= EXTRACT;
		}
		np->e_flags |= KEEP;
		dmsg("[%s] %s: %s\n", keyval(key), name, flagvalues(np));
		break;

		/*
		 * An inode is being reused in a completely different way.
		 * Normally an extract can simply do an "unlink" followed by a
		 * "creat". Here we must do effectively the same thing. The
		 * complications arise because we cannot really delete a
		 * directory since it may still contain files that we need to
		 * rename, so we delete it from the symbol table, and put it on
		 * the list to be deleted eventually. Conversely if a directory
		 * is to be created, it must be done immediately, rather than
		 * waiting until the extraction phase.
		 */

	case ONTAPE | INOFND | MODECHG:
	case ONTAPE | INOFND | NAMEFND | MODECHG:
		if (ip->e_flags & KEEP)
		{
			badentry(ip, MSGSTR(CANKEEP, "cannot KEEP and change modes"));
			break;
		}
		if (ip->e_type == LEAF)
		{
			/* changing from leaf to node */

			removeleaf(ip);
			freeentry(ip);
			ip = addentry(name, ino, type);
			newnode(ip);
		}
		else
		{
			/* changing from node to leaf */

			if ((ip->e_flags & TMPNAME) == 0)
			{
				mktempname(ip);
			}
			deleteino(ip->e_ino);
			ip->e_next = removelist;
			removelist = ip;
			ip = addentry(name, ino, type);
		}
		ip->e_flags |= NEW | KEEP;
		dmsg("[%s] %s: %s\n", keyval(key), name, flagvalues(ip));
		break;

		/*
		 * A hard link to a diirectory that has been removed. Ignore
		 * it.
		 */

	case NAMEFND:
		dmsg("[%s] %s: Extraneous name\n", keyval(key), name);
		descend = FAIL;
		break;

		/*
		 * If we find a directory entry for a file that is not on the
		 * tape, then we must have found a file that was created while
		 * the dump was in progress. Since we have no contents for it,
		 * we discard the name knowing that it will be on the next
		 * incremental tape.
		 */

	case NULL:
		msg(MSGSTR(NOTONT, "%s: (inode %d) not found on tape\n"), name, ino);
		break;

		/*
		 * If any of these arise, something is grievously wrong with
		 * the current state of the symbol table.
		 */

	case INOFND | NAMEFND | MODECHG:
	case NAMEFND | MODECHG:
	case INOFND | MODECHG:
		panic(MSGSTR(INCSTATE, "[%s] %s: inconsistent state\n"), keyval(key), name);
		break;

		/*
		 * These states "cannot" arise for any state of the symbol
		 * table.
		 */

	case ONTAPE | MODECHG:
	case MODECHG:
	default:
		panic(MSGSTR(IMPOS, "[%s] %s: impossible state\n"), keyval(key), name);
		break;
	}
	return(descend);
}

/*
 * Calculate the active flags in a key.
 */

char	       *
keyval(key)
	int		key;
{
	static char	keybuf[32];

	(void) strcpy(keybuf, "|NULL");
	keybuf[0] = '\0';
	if (key & ONTAPE)
	{
		(void) strcat(keybuf, "|ONTAPE");
	}
	if (key & INOFND)
	{
		(void) strcat(keybuf, "|INOFND");
	}
	if (key & NAMEFND)
	{
		(void) strcat(keybuf, "|NAMEFND");
	}
	if (key & MODECHG)
	{
		(void) strcat(keybuf, "|MODECHG");
	}
	return(&keybuf[1]);
}

/*
 * Find unreferenced link names.
 */

void
findunreflinks()
{
	register struct entry *ep, *np;
	register ino_t	i;

	vmsg(MSGSTR(FINDUNRE, "Find unreferenced names.\n"));
	for (i = ROOTINO; i < maxino; ++i)
	{
		ep = lookupino(i);
		if (ep == NULL || ep->e_type == LEAF || !MAPBITTEST(dumpmap, i))
		{
			continue;
		}
		for (np = ep->e_entries; np != NULL; np = np->e_sibling)
		{
			if (np->e_flags == 0)
			{
				dmsg("%s: remove unreferenced name\n", myname(np));
				removeleaf(np);
				freeentry(np);
			}
		}
	}

	/*
	 * Any leaves remaining in removed directories is unreferenced.
	 */

	for (ep = removelist; ep != NULL; ep = ep->e_next)
	{
		for (np = ep->e_entries; np != NULL; np = np->e_sibling)
		{
			if (np->e_type == LEAF)
			{
				if (np->e_flags != 0)
				{
					badentry(np, MSGSTR(UNREFF, "unreferenced with flags"));
				}
				dmsg("%s: remove unreferenced name\n", myname(np));
				removeleaf(np);
				freeentry(np);
			}
		}
	}
}

/*
 * Remove old nodes (directories). Note that this routine runs in O(N*D) where:
 * N is the number of directory entries to be removed. D is the maximum depth
 * of the tree. If N == D this can be quite slow. If the list were
 * topologically sorted, the deletion could be done in time O(N).
 */

void
removeoldnodes()
{
	register struct entry *ep, **prev;
	int		change;

	vmsg(MSGSTR(REMOLD, "Remove old nodes (directories).\n"));
	do
	{
		change = FALSE;
		prev = &removelist;
		for (ep = removelist; ep != NULL; ep = *prev)
		{
			if (ep->e_entries != NULL)
			{
				prev = &ep->e_next;
				continue;
			}
			*prev = ep->e_next;
			removenode(ep);
			freeentry(ep);
			change = TRUE;
		}
	} while (change == TRUE);
	for (ep = removelist; ep != NULL; ep = ep->e_next)
	{
		badentry(ep, MSGSTR(CANREMO, "cannot remove, non-empty"));
	}
}

/*
 * This is the routine used to extract files for the 'r' command. Extract new
 * leaves.
 */

void
createleaves(symtabfile)
	char	       *symtabfile;
{
	register struct entry *ep;
	ino_t		first;
	long		curvol;

	if (command == 'R')
	{
		vmsg(MSGSTR(CANEXT, "Continue extraction of new leaves\n"));
	}
	else
	{
		vmsg(MSGSTR(EXTNEW, "Extract new leaves.\n"));
		dumpsymtable(symtabfile, volno);
	}
	first = lowerbnd(ROOTINO);
	curvol = volno;
	while (curr_inumber < maxino)
	{
		first = lowerbnd(first);

		/*
		 * If the next available file is not the one which we expect
		 * then we have missed one or more files. Since we do not
		 * request files that were not on the tape, the lost files must
		 * have been due to a tape read error, or a file that was
		 * removed while the dump was in progress.
		 */

		while (first < curr_inumber)
		{
			ep = lookupino(first);
			if (ep == NULL)
			{
				panic(MSGSTR(BADF, "%d: bad first\n"), first);
			}
			msg(MSGSTR(NOTONTAPE, "%s: not found on the tape\n"), myname(ep));
			ep->e_flags &= ~(NEW | EXTRACT);
			first = lowerbnd(first);
		}

		/*
		 * If we find files on the tape that have no corresponding
		 * directory entries, then we must have found a file that was
		 * created while the dump was in progress. Since we have no
		 * name for it, we discard it knowing that it will be on the
		 * next incremental tape.
		 */

		if (first != curr_inumber)
		{
			msg(MSGSTR(EXPECF, "expected next file %d, got %d\n"),
				first, curr_inumber);
			skipfile();
			goto next;
		}
		ep = lookupino(curr_inumber);
		if (ep == NULL)
		{
			panic(MSGSTR(UNKFILE, "unknown file on tape\n"));
		}
		if ((ep->e_flags & (NEW | EXTRACT)) == 0)
		{
			badentry(ep, MSGSTR(UNEXPECF, "unexpected file on tape"));
		}

		/*
		 * If the file is to be extracted, then the old file must be
		 * removed since its type may change from one leaf type to
		 * another (eg "file" to "character special").
		 */

		if ((ep->e_flags & EXTRACT) != 0)
		{
			removeleaf(ep);
			ep->e_flags &= ~REMOVED;
		}
		(void) extractfile(myname(ep));
		ep->e_flags &= ~(NEW | EXTRACT);

		/*
		 * We checkpoint the restore after every tape reel, so as to
		 * simplify the amount of work re quired by the 'R' command.
		 */

next:
		if (curvol != volno)
		{
			dumpsymtable(symtabfile, volno);
			skipmaps();
			curvol = volno;
		}
	}
}

/*
 * This is the routine used to extract files for the 'x' and 'i' commands.
 * Efficiently extract a subset of the files on a tape.
 */

void
createfiles()
{
	register ino_t	first, next, last;
	register struct entry *ep;
	long		curvol;

	vmsg(MSGSTR(EXTREQ, "Extract requested files\n"));
	curr_action = SKIP;
	getvol((long) 1);
	skipmaps();
	skipdirs();
	first = lowerbnd(ROOTINO);
	last = upperbnd(maxino - 1);
	for (;;)
	{
		first = lowerbnd(first);
		last = upperbnd(last);

		/*
		 * Check to see if any files remain to be extracted
		 */

		if (first > last)
		{
			return;
		}

		/*
		 * Reject any volumes with inodes greater than the last one
		 * needed
		 */

		while (curr_inumber > last)
		{
			curr_action = SKIP;
			getvol((long) 0);
			skipmaps();
			skipdirs();
		}

		/*
		 * Decide on the next inode needed. Skip across the inodes
		 * until it is found or an out of order volume change is
		 * encountered
		 */

		next = lowerbnd(curr_inumber);
		do
		{
			curvol = volno;
			while (next > curr_inumber && volno == curvol)
			{
				skipfile();
			}
			skipmaps();
			skipdirs();
		} while (volno == curvol + 1);

		/*
		 * If volume change out of order occurred the current state
		 * must be recalculated
		 */

		if (volno != curvol)
		{
			continue;
		}

		/*
		 * If the current inode is greater than the one we were looking
		 * for then we missed the one we were looking for. Since we
		 * only attempt to extract files listed in the dump map, the
		 * lost files must have been due to a tape read error, or a
		 * file that was removed while the dump was in progress. Thus
		 * we report all requested files between the one we were
		 * looking for, and the one we found as missing, and delete
		 * their request flags.
		 */

		while (next < curr_inumber)
		{
			ep = lookupino(next);
			if (ep == NULL)
			{
				panic(MSGSTR(CORSYM, "corrupted symbol table\n"));
			}
			msg(MSGSTR(NOTONTAPE, "%s: not found on the tape\n"), myname(ep));
			ep->e_flags &= ~NEW;
			next = lowerbnd(next);
		}

		/*
		 * The current inode is the one that we are looking for, so
		 * extract it per its requested name.
		 */

		if (next == curr_inumber && next <= last)
		{
			ep = lookupino(next);
			if (ep == NULL)
			{
				panic(MSGSTR(CORSYM, "corrupted symbol table\n"));
			}
			(void) extractfile(myname(ep));
			ep->e_flags &= ~NEW;
			if (volno != curvol)
			{
				skipmaps();
			}
		}
	}
}

/*
 * Add links.
 */

void
createlinks()
{
	register struct entry *np, *ep;
	register ino_t	i;
	char		name[BUFSIZ];

	vmsg(MSGSTR(ADDLN, "Add links\n"));
	for (i = ROOTINO; i < maxino; ++i)
	{
		ep = lookupino(i);
		if (ep == NULL)
		{
			continue;
		}
		for (np = ep->e_links; np != NULL; np = np->e_links)
		{
			if ((np->e_flags & NEW) == 0)
			{
				continue;
			}
			(void) strcpy(name, myname(ep));
			if (ep->e_type == NODE)
			{
				(void) linkit(name, myname(np), SYMLINK);
			}
			else
			{
				(void) linkit(name, myname(np), HARDLINK);
			}
			np->e_flags &= ~NEW;
		}
	}
}

/*
 * Check the symbol table. We do this to insure that all the requested work was
 * done, and that no temporary names remain.
 */

void
checkrestore()
{
	register struct entry *ep;
	register ino_t	i;

	vmsg(MSGSTR(CHECKSYM, "Check the symbol table.\n"));
	for (i = ROOTINO; i < maxino; ++i)
	{
		for (ep = lookupino(i); ep != NULL; ep = ep->e_links)
		{
			ep->e_flags &= ~KEEP;
			if (ep->e_type == NODE)
			{
				ep->e_flags &= ~(NEW | EXISTED);
			}
			if (ep->e_flags != NULL)
			{
				badentry(ep, MSGSTR(IMCOP, "incomplete operations"));
			}
		}
	}
}

/*
 * Compare with the directory structure on the tape A paranoid check that
 * things are as they should be.
 */

long
verifyfile(name, ino, type)
	char	       *name;
	ino_t		ino;
	int		type;
{
	struct entry   *np, *ep;
	long		descend = GOOD;

	ep = lookupname(name);
	if (ep == NULL)
	{
		msg(MSGSTR(MISSNAM, "Warning: missing name %s\n"), name);
		return(FAIL);
	}
	np = lookupino(ino);
	if (np != ep)
	{
		descend = FAIL;
	}
	for (; np != NULL; np = np->e_links)
	{
		if (np == ep)
		{
			break;
		}
	}
	if (np == NULL)
	{
		panic(MSGSTR(MISSINO, "missing inumber %d\n"), ino);
	}
	if (ep->e_type == LEAF && type != LEAF)
	{
		badentry(ep, MSGSTR(TYPELEAF, "type should be LEAF"));
	}
	return(descend);
}
