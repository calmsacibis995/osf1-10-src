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
static char	*sccsid = "@(#)$RCSfile: interactive.c,v $ $Revision: 4.2.6.3 $ (DEC) $Date: 1993/12/10 20:11:07 $";
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
 * Copyright (c) 1985 Regents of the University of California. All
 * rights reserved.  The Berkeley software License Agreement specifies the
 * terms and conditions for redistribution.
 */

#include	"restore.h"
#undef		isprint
#include	<setjmp.h>

#define	round(a, b) (((a) + (b) - 1) / (b) * (b))

static void	help();
static void	getcmd();
static void	expandarg();
static void	printlist();
static void	mkentry();
static void	formatf();

/* command codes */

#define	CMD_UNK		0	/* unknown command */
#define	CMD_HLP		1	/* help */
#define	CMD_DBG		2	/* debug */
#define	CMD_ADD		3	/* add */
#define	CMD_CHD		4	/* change directory */
#define	CMD_DEL		5	/* delete */
#define	CMD_EXT		6	/* extract */
#define	CMD_LST		7	/* list */
#define	CMD_PWD		8	/* print working directory */
#define	CMD_QIT		9	/* quit */
#define	CMD_STM		10	/* setmodes */
#define	CMD_VRB		11	/* verbose */
#define	CMD_WHT		12	/* what */
#define	CMD_AMB		13	/* ambiguous command */
#define CMD_MAX		14	/* maximal value of cmd codes */
/*
 * Structure and routines associated with listing directories.
 */

struct argitem
{
	ino_t		fnum;	/* inode number of file */
	chfl	       *fname;	/* file name */
	int		fflags;	/* extraction flags, if any */
	int		ftype;	/* file type, e.g. LEAF or NODE */
};

struct arglist
{
	struct argitem *next;	/* start of argument list */
	struct argitem *last;	/* end of argument list */
	struct argitem *base;	/* current list arena */
	unsigned int	nitems;	/* maximum size of list */
	char	       *cmd;	/* the current command */
};

/* command string and its command code */

struct cmd_sel
{
	char	       *cmd_str;
	int		cmd_cod;
};

static struct cmd_sel  *cmd_setup();
static chfl	       *copynext();

/*
 * Things to handle interruptions.
 */

static jmp_buf	reset;
static long     reset_valid=FALSE;
extern chfl	nullcfs;
static chfl	*nextarg;

/*
 * Read and execute commands from the terminal.
 */

void
runcmdshell()
{
	static struct arglist alist = { NULL, NULL, NULL, 0, NULL };

	struct cmd_sel	*cmd_list;
	struct entry	*np;
	ino_t		ino;
	char		curdir[MAXPATHLEN];
	char		cmd[BUFSIZ];
	char		arg[MAXPATHLEN];
	chfl		cfsbuf1[BUFSIZ];
	chfl		cfsbuf2[BUFSIZ];
	int		cmd_code;

	cmd_list = cmd_setup();

	strtocfs(cfsbuf1, "/");
	canon(cfsbuf2, cfsbuf1);
	cfstostr(curdir, cfsbuf2);

	nextarg = &nullcfs;
	while (TRUE)
	{
		int i;

		if (setjmp(reset) != 0)
		{
			for (; alist.next < alist.last; ++alist.next)
			{
				free(alist.next->fname);
			}
			alist.next = alist.last = alist.base;
			nextarg = &nullcfs;
			volno = 0;
		}

		reset_valid=TRUE;

		getcmd(curdir, cmd, arg, &alist);

		cmd_code = CMD_UNK;
		for (i = 0; cmd_list[i].cmd_str != NULL; ++i)
		{
			if (strncmp(cmd, cmd_list[i].cmd_str, strlen(cmd)) == 0)
			{
				if (cmd_code == CMD_UNK)
				{
					cmd_code = cmd_list[i].cmd_cod;
				}
				else
				{
					cmd_code = CMD_AMB;
					break;
				}
			}
		}

		switch (cmd_code)
		{
		case CMD_UNK:

			/*
			 * Unknown command.
			 */

			msg(MSGSTR(UNKCMD, "%s: unknown command; type ? for help\n"), cmd);
			break;

		case CMD_HLP:

			/*
			 * List available commands.
			 */

			help();
			break;

		case CMD_DBG:

			/*
			 * toggle debugging.
			 */

			if (debug_flag == TRUE)
			{
				msg(MSGSTR(DEBGOFF, "debugging mode off\n"));
				debug_flag = FALSE;
			}
			else
			{
				msg(MSGSTR(DEBGON, "debugging mode on\n"));
				debug_flag = TRUE;
			}
			break;

		case CMD_ADD:

			/*
			 * Add elements to the extraction list.
			 */

			ino = dirlookup(arg);
			if (ino == (ino_t) 0)
			{
				break;
			}
			if (by_name_flag == TRUE)
			{
				pathcheck(arg);
			}
			treescan(arg, ino, addfile);
			break;

		case CMD_CHD:

			/*
			 * Change working directory.
			 */

			ino = dirlookup(arg);
			if (ino == (ino_t) 0)
			{
				break;
			}
			if (inodetype(ino) == LEAF)
			{
				msg(MSGSTR(NOTDIR1, "%s: not a directory\n"), arg);
				break;
			}
			(void) strcpy(curdir, arg);
			break;

		case CMD_DEL:

			/*
			 * Delete elements from the extraction list.
			 */

			np = lookupname(arg);
			if (np == NULL || ! (np->e_flags & NEW))
			{
				msg(MSGSTR(NOTONLIST, "%s: not on extraction list\n"), arg);
				break;
			}
			treescan(arg, np->e_ino, deletefile);
			break;

		case CMD_EXT:

			/*
			 * Extract the requested list.
			 */

			createfiles();
			createlinks();
			setdirmodes();
			if (debug_flag == TRUE)
			{
				checkrestore();
			}
			volno = 0;
			break;

		case CMD_LST:

			/*
			 * List a directory.
			 */

			ino = dirlookup(arg);
			if (ino == (ino_t) 0)
			{
				break;
			}
			printlist(arg, ino, curdir);
			break;

		case CMD_PWD:

			/*
			 * Print current directory.
			 */

			if (curdir[1] == '\0')
			{
				/* curdir == "/" */

				msg("%s\n", curdir);
			}
			else
			{
				/* do not print leading '/' */

				msg("%s\n", (curdir + 1));
			}
			break;

		case CMD_QIT:

			/*
			 * Quit.
			 */

			return;

		case CMD_STM:

			/*
			 * Just restore requested directory modes.
			 */

			setdirmodes();
			break;

		case CMD_VRB:

			/*
			 * Toggle verbose mode.
			 */

			if (verbose_flag == TRUE)
			{
				msg(MSGSTR(VOFF, "verbose mode off\n"));
				verbose_flag = FALSE;
			}
			else
			{
				msg(MSGSTR(VON, "verbose mode on\n"));
				verbose_flag = TRUE;
			}
			break;

		case CMD_WHT:

			/*
			 * Print out dump header information.
			 */

			printdumpinfo();
			break;

		case CMD_AMB:

			/*
			 * ambiguous command.
			 */

			break;
		}
	}
}

static struct cmd_sel  *
cmd_setup()
{
	struct cmd_sel *cmd_array;

	cmd_array = (struct cmd_sel *) malloc((CMD_MAX + 1) * sizeof(struct cmd_sel));

	cmd_array[0].cmd_str = strdup(MSGSTR(CMDQST, "?"));
	cmd_array[0].cmd_cod = CMD_HLP;

	cmd_array[1].cmd_str = strdup(MSGSTR(CMDDBG, "Debug"));
	cmd_array[1].cmd_cod = CMD_DBG;

	cmd_array[2].cmd_str = strdup(MSGSTR(CMDADD, "add"));
	cmd_array[2].cmd_cod = CMD_ADD;

	cmd_array[3].cmd_str = strdup(MSGSTR(CMDCHD, "cd"));
	cmd_array[3].cmd_cod = CMD_CHD;

	cmd_array[4].cmd_str = strdup(MSGSTR(CMDDEL, "delete"));
	cmd_array[4].cmd_cod = CMD_DEL;

	cmd_array[5].cmd_str = strdup(MSGSTR(CMDEXT, "extract"));
	cmd_array[5].cmd_cod = CMD_EXT;

	cmd_array[6].cmd_str = strdup(MSGSTR(CMDHLP, "help"));
	cmd_array[6].cmd_cod = CMD_HLP;

	cmd_array[7].cmd_str = strdup(MSGSTR(CMDLST, "ls"));
	cmd_array[7].cmd_cod = CMD_LST;

	cmd_array[8].cmd_str = strdup(MSGSTR(CMDPWD, "pwd"));
	cmd_array[8].cmd_cod = CMD_PWD;

	cmd_array[9].cmd_str = strdup(MSGSTR(CMDQIT, "quit"));
	cmd_array[9].cmd_cod = CMD_QIT;

	cmd_array[10].cmd_str = strdup(MSGSTR(CMDSTM, "setmodes"));
	cmd_array[10].cmd_cod = CMD_STM;

	cmd_array[11].cmd_str = strdup(MSGSTR(CMDVRB, "verbose"));
	cmd_array[11].cmd_cod = CMD_VRB;

	cmd_array[12].cmd_str = strdup(MSGSTR(CMDWHT, "what"));
	cmd_array[12].cmd_cod = CMD_WHT;

	cmd_array[13].cmd_str = strdup(MSGSTR(CMDXIT, "xit"));
	cmd_array[13].cmd_cod = CMD_QIT;

	cmd_array[14].cmd_str = NULL;
	cmd_array[14].cmd_cod = CMD_UNK;

	return(cmd_array);
}

static void
help()
{
	msg("%s\n", MSGSTR(HELP01, "Available commands are:"));
	msg("%s\n", MSGSTR(HELP02, "\tls [arg]     - list directory"));
	msg("%s\n", MSGSTR(HELP03, "\tcd arg       - change directory"));
	msg("%s\n", MSGSTR(HELP04, "\tpwd          - print current directory"));
	msg("%s\n", MSGSTR(HELP05, "\tadd [arg]    - add `arg' to list of files to be extracted"));
	msg("%s\n", MSGSTR(HELP06, "\tdelete [arg] - delete `arg' from list of files to be extracted"));
	msg("%s\n", MSGSTR(HELP07, "\textract      - extract requested files"));
	msg("%s\n", MSGSTR(HELP08, "\tsetmodes     - set modes of requested directories"));
	msg("%s\n", MSGSTR(HELP09, "\tquit         - immediately exit program"));
	msg("%s\n", MSGSTR(HELP10, "\tverbose      - toggle verbose flag"));
	msg("%s\n", MSGSTR(HELP11, "\twhat         - list dump header information (useful with ``ls'')"));
	msg("%s\n", MSGSTR(HELP12, "\thelp or `?'  - print this list"));
	msg("%s\n", MSGSTR(HELP13, "\tIf no `arg' is supplied, the current directory is used"));
}

/*
 * Read and parse an interactive command. The first word on the line is
 * assigned to "cmd". If there are no arguments on the command line, then
 * "curdir" is returned as the argument. If there are arguments on the line
 * they are returned one at a time on each successive call to getcmd. Each
 * argument is first assigned to "name". If it does not start with "/" the
 * pathname in "curdir" is prepended to it. Finally "canon" is called to
 * eliminate any embedded ".." components.
 */

static void
getcmd(curdir, cmd, arg, ap)
	char	       *curdir;
	char	       *cmd;
	char	       *arg;
	struct arglist *ap;
{
	char		inpstr[BUFSIZ];
	static chfl	inpcfs[BUFSIZ];
	chfl	       *cfp;
	chfl		currarg[BUFSIZ];
	char		strbuf[BUFSIZ];
	chfl		cfsbuf[BUFSIZ];
	int		i;
	int		len;

	/*
	 * Check to see if no previously obtained arguments are left
	 */

	if (ap->next == ap->last)
	{
		if (CHR(nextarg) == '\0')
		{
			/*
			 * Read a command line and trim off trailing white space.
			 */

			CHR(&inpcfs[0]) = '\0';
			while (CHR(&inpcfs[0]) == '\0')
			{
				msg("restore > ");

				if (fgets(inpstr, BUFSIZ, command_fp) == NULL)
				{
					(void) strcpy(cmd, MSGSTR(CMDQIT, "quit"));
					(void) strcpy(arg, "");
					return;
				}

				i = strlen(inpstr) - 1;
				if (inpstr[i] == '\n')
				{
					inpstr[i] = '\0';
				}

				(void) scanstr(inpcfs, inpstr);

				len=cfslen(inpcfs);
				if (len)
					for (cfp = &inpcfs[len - 1];
				     	     cfp >= inpcfs && ! (FLG(cfp) & QUOTED) && isspace(CHR(cfp)); --cfp)
					{
						CHR(cfp) = '\0';
					}
			}

			/*
			 * Copy the command into "cmd".
			 */

			nextarg = copynext(cfsbuf, inpcfs);
			cfstostr(cmd, cfsbuf);
			ap->cmd = cmd;

			/*
			 * If no argument, use curdir as the default.
			 */

			if (CHR(nextarg) == '\0')
			{
				(void) strcpy(arg, curdir);
				return;
			}
		}

		/*
		 * Find the next argument.
		 */

		nextarg = copynext(currarg, nextarg);

		if (CHR(&currarg[0]) != '/')
		{
			/*
			 * For relative pathnames, prepend the current directory to it
			 */

			(void) strcpy(strbuf, curdir);
			(void) strcat(strbuf, "/");
			(void) strtocfs(cfsbuf, strbuf);
			(void) cfscat(cfsbuf, currarg);
			(void) cfscpy(currarg, cfsbuf);
		}

		canon(cfsbuf, currarg);

		expandarg(ap, cfsbuf);
	}

	cfstostr(arg, ap->next->fname);
	free(ap->next->fname);
	++(ap->next);
	if (ap->next == ap->last)
	{
		ap->next = ap->last = ap->base;
	}
}

/*
 * Strip off the next token of the input.
 */

static chfl    *
copynext(dest, src)
	register chfl  *dest;
	register chfl  *src;
{
	/* skip white space in input */

	while (! (FLG(src) & QUOTED) && isspace(CHR(src)))
	{
		++src;
	}

	/* copy white-space delimited token from input to output */

	while (CHR(src) != '\0' && ((FLG(src) & QUOTED) || ! isspace(CHR(src))))
	{
		CHR(dest) = CHR(src);
		FLG(dest) = FLG(src);
		++dest, ++src;
	}

	/* terminate output string */

	CHR(dest) = '\0';
	FLG(dest) = NOFLG;

	/* return pointer to unused portion of input string */

	return(src);
}

/*
 * Canonicalize file names to always start with ``./'' and remove any imbedded
 * "." and ".." components.
 */

void
canon(canonname, rawname)
	chfl	       *canonname;
	chfl	       *rawname;
{
	register chfl  *src, *dest;
	chfl		buf1[BUFSIZ];
	chfl		buf2[BUFSIZ];
	chfl		buf3[BUFSIZ];
	chfl		buf4[BUFSIZ];
	chfl		cfsbuf[BUFSIZ];

	/* put on leading and trailing '/'s, rawname ==> buf1 */

	(void) strtocfs(cfsbuf, "/");
	(void) cfscpy(buf1, cfsbuf);
	(void) cfscat(buf1, rawname);
	(void) cfscat(buf1, cfsbuf);

	/* reduce "//"s to '/', buf1 ==> buf2 */

	for (src = buf1, dest = buf2; CHR(src) != '\0'; ++src, ++dest)
	{
		while (CHR(src) == '/' && CHR(src + 1) == '/')
		{
			++src;
		}
		CHR(dest) = CHR(src);
		FLG(dest) = FLG(src);
	}
	CHR(dest) = '\0';

	/* reduce "/./"s to '/', buf2 ==> buf3 */

	for (src = buf2, dest = buf3; CHR(src) != '\0'; ++src, ++dest)
	{
		while (CHR(src) == '/' && CHR(src + 1) == '.' && CHR(src + 2) == '/')
		{
			src += 2;
		}
		CHR(dest) = CHR(src);
		FLG(dest) = FLG(src);
	}
	CHR(dest) = '\0';

	/* reduce "/<anything>/../"s to '/', buf3 ==> buf4 */

	for (src = buf3, dest = buf4; CHR(src) != '\0'; ++src, ++dest)
	{
		while (CHR(src) == '/' && CHR(src + 1) == '.' && CHR(src + 2) == '.' && CHR(src + 3) == '/')
		{
			/* back up in dest buffer over previous entry */

			if (dest > buf4)
			{
				do
				{
					--dest;
				}
				while (dest > buf4 && CHR(dest) != '/');
			}
			src += 3;
		}
		CHR(dest) = CHR(src);
		FLG(dest) = FLG(src);
	}
	CHR(dest) = '\0';

	/* delete trailing '/' within buf4 */

	dest = &buf4[cfslen(buf4) - 1];
	if (dest >= buf4 && CHR(dest) == '/')
	{
		CHR(dest) = '\0';
	}

	/* retain leading '/', within buf4 */

	/* add leading '.', buf4 ==> canonname */

	(void) strtocfs(canonname, ".");
	(void) cfscat(canonname, buf4);
}

/*
 * globals (file name generation)
 *
 * "*" in params matches r.e ".*"
 * "?" in params matches r.e. "."
 * "[...]" in params matches character class
 * "[...a-z...]" in params matches a through z.
 */

static void
expandarg(ap, pat)
	register struct arglist	*ap;
	chfl	       *pat;
{
	struct entry   *ep;
	char		strbuf[BUFSIZ];
	ino_t		inum;

	if (expand(ap, pat, FALSE) == 0)
	{
		cfstostr(strbuf, pat);
		ep = lookupname(strbuf);
		if (ep == NULL)
		{
			inum = (ino_t) 0;
		}
		else
		{
			inum = ep->e_ino;
		}
		mkentry(ap, pat, inum);
	}

	qsort((char *) ap->next, ap->last - ap->next, sizeof(struct argitem), argcmp);
}

/*
 * Expand a file name
 */

int
expand(ap, pattern, recursive_flag)
	register struct arglist	*ap;
	chfl	       *pattern;
	int		recursive_flag;
{
	int		count, size;
	int		dir = FALSE;
	chfl	       *rescan = NULL;
	RST_DIR	       *dirp;
	register chfl  *s, *cs;
	int		sindex, rindex, lindex;
	struct dirent  *dp;
	register int	slash;
	register chfl  *rs;
	register char	c;

	/*
	 * check for meta chars
	 */

	s = cs = pattern;

	slash = FALSE;
	while ((FLG(cs) & QUOTED) || (CHR(cs) != '*' && CHR(cs) != '?' && CHR(cs) != '['))
	{
		if (CHR(cs) == '\0')
		{
			++cs;
			if (recursive_flag == TRUE && slash == TRUE)
			{
				break;
			}
			else
			{
				return(0);
			}

			/* NOTREACHED */
		}
		++cs;
		if (CHR(cs) == '/')
		{
			slash == TRUE;
		}
	}

	for (;;)
	{
		if (cs == s)
		{
			s = &nullcfs;
			break;
		}
		else
		{
			--cs;
			if (CHR(cs) == '/')
			{
				CHR(cs) = '\0';
				if (s == cs)
				{
					s = cfsdup("/");
				}
				break;
			}
		}
	}

	if ((dirp = rst_opendir(s)) != NULL)
	{
		dir = TRUE;
	}

	count = 0;
	if (CHR(cs) == '\0')
	{
		CHR(cs) = 0200;
		++cs;
	}

	if (dir == TRUE)
	{
		/*
		 * check for rescan
		 */

		rs = cs;
		if (CHR(rs) == '/')
		{
			rescan = rs;
			CHR(rs) = '\0';
		}

		while (CHR(rs) != '\0')
		{
			++rs;
			if (CHR(rs) == '/')
			{
				rescan = rs;
				CHR(rs) = '\0';
			}
		}
		++rs;

		sindex = ap->last - ap->next;
		while ((dp = rst_readdir(dirp)) != NULL && dp->d_fileno != (ulong_t)0)
		{
			if (debug_flag == FALSE && !MAPBITTEST(dumpmap, dp->d_fileno))
			{
				continue;
			}
			if (*dp->d_name == '.' && CHR(cs) != '.')
			{
				continue;
			}
			if (gmatch(dp->d_name, cs) != 0)
			{
				if (addg(dp, s, rescan, ap) < 0)
				{
					return(-1);
				}
				++count;
			}
		}

		if (rescan != NULL)
		{
			rindex = sindex;
			lindex = ap->last - ap->next;
			if (count != 0)
			{
				count = 0;
				while (rindex < lindex)
				{
					size = expand(ap, ap->next[rindex].fname, TRUE);
					if (size < 0)
					{
						return(size);
					}
					count += size;
					++rindex;
				}
			}
			memcpy((char *) &ap->next[sindex],
			       (char *) &ap->next[lindex],
			       (ap->last - &ap->next[rindex]) * sizeof(struct argitem));
			ap->last -= lindex - sindex;
			CHR(rescan) = '/';
		}
	}

	s = pattern;
	while ((c = CHR(s)) != '\0')
	{
		CHR(s) = (c & 0177)? c : '/';
		++s;
	}

	return(count);
}

/*
 * Check for a name match
 */

int
gmatch(s, p)
	register char  *s, *p;
{
	register int	scc;
	char		c;
	int		ok;
	int		lc;

	if ((scc = *s) != '\0')
	{
		if ((scc &= 0177) == 0)
		{
			scc = 0200;
		}
	}
	++s;
	c = *p;
	++p;
	switch (c)
	{

	case '[':
		ok = FALSE;
		lc = 077777;
		while ((c = *p) != '\0')
		{
			++p;
			if (c == ']')
			{
				return((ok == TRUE)? gmatch(s, p) : 0);
			}
			else if (c == '-')
			{
				if (lc <= scc)
				{
					if (scc <= *p)
					{
						ok = TRUE;
					}
					++p;
				}
			}
			else
			{
				if (scc == (lc = (c & 0177)))
				{
					ok = TRUE;
				}
			}
		}
		return(0);

	default:
		if ((c & 0177) != scc)
		{
			return(0);
		}
		/* falls through */

	case '?':
		return((scc != 0)? gmatch(s, p) : 0);

	case '*':
		if (*p == 0)
		{
			return(1);
		}
		--s;
		while (*s)
		{
			if (gmatch(s, p) != 0)
			{
				return(1);
			}
			++s;
		}
		return(0);

	case 0:
		return(scc == 0);
	}
}

/*
 * Construct a matched name.
 */
chfl		slashcfs[] = {{ '/', SLASH }, { '\0', NOFLG }};

int
addg(dp, as1, as2, ap)
	struct dirent  *dp;
	chfl	       *as1, *as2;
	struct arglist *ap;
{
	chfl		cfsbuf1[BUFSIZ];
	chfl		cfsbuf2[BUFSIZ];

	cfscpy(cfsbuf1, as1);
	cfscat(cfsbuf1, slashcfs);
	cfscat(cfsbuf1, strtocfs(cfsbuf2, dp->d_name));

	if (as2)
	{
		cfscat(cfsbuf1, slashcfs);
		cfscat(cfsbuf1, as2);
	}
	mkentry(ap, cfsbuf1, dp->d_fileno);
	return(0);
}

/*
 * Do an "ls" style listing of a directory
 */

static void
printlist(name, ino, basename)
	char	       *name;
	ino_t		ino;
	char	       *basename;
{
	static struct arglist	alist = { NULL, NULL, NULL, 0, NULL };
	RST_DIR		       *dirp;
	register struct dirent *dp;
	chfl			cfsbuf[BUFSIZ];

	if (alist.cmd == NULL)
	{
		alist.cmd = strdup(MSGSTR(CMDLST, "ls"));
	}

	if ((dirp = rst_opendir(name)) == NULL)
	{
		strtocfs(cfsbuf, name + strlen(basename) + 1);
		mkentry(&alist, cfsbuf, ino);
	}
	else
	{
		msg("%s:\n", name);
		while ((dp = rst_readdir(dirp)) != NULL && dp->d_fileno != (ulong_t) 0)
		{
			if (debug_flag == FALSE && !MAPBITTEST(dumpmap, dp->d_fileno))
			{
				continue;
			}

			if (verbose_flag == FALSE &&
			    (strcmp(dp->d_name, ".") == 0 ||
			     strcmp(dp->d_name, "..") == 0))
			{
				continue;
			}

			strtocfs(cfsbuf, dp->d_name);
			mkentry(&alist, cfsbuf, dp->d_fileno);
		}

	}


	qsort((char *) alist.next, alist.last - alist.next, sizeof(struct argitem), argcmp);
	formatf(&alist);

	if (dirp != NULL)
	{
		msg("\n");
	}

	for (; alist.next < alist.last; ++alist.next)
	{
		free(alist.next->fname);
	}
	alist.next = alist.last = alist.base;
}

/*
 * Comparison routine for qsort.
 */

static int
argcmp(arg1, arg2)
	register struct argitem	       *arg1;
	register struct argitem	       *arg2;
{
	char		buf1[BUFSIZ];
	char		buf2[BUFSIZ];
	register int	retval;

	cfstostr(buf1, arg1->fname);
	cfstostr(buf2, arg2->fname);

	errno = 0;

	retval = strcoll(buf1, buf2);

	if (errno != 0)
	{
		retval = 0;
	}

	return(retval);
}

/*
 * Add filename and inode number entry to argument list ap
 */

static void
mkentry(ap, name, ino)
	register struct arglist	*ap;
	chfl	       *name;
	ino_t		ino;
{
	struct argitem *argptr;

	if (ap->base == NULL)
	{
		ap->nitems = 16;
		ap->base = (struct argitem *) calloc(ap->nitems, sizeof(struct argitem));
		ap->next = ap->last = ap->base;
	}

	while (ap->next != ap->base)
	{
		for (argptr = ap->next; argptr < ap->last; ++argptr)
		{
			memcpy((char *) (argptr - 1), (char *) argptr, sizeof(struct argitem));
		}
		--(ap->next), --(ap->last);
	}

	ap->last->fname = cfsdup(name);
	ap->last->fnum = ino;

	++(ap->last);

	if (ap->last - ap->base == ap->nitems)
	{
		ap->base = (struct argitem *) realloc((char *) ap->base, 2 * ap->nitems * sizeof(struct argitem));
		memset((char *) (ap->base + ap->nitems), (int) '\0', ap->nitems * sizeof(struct argitem));
		ap->next = ap->base;
		ap->last = ap->base + ap->nitems;
		ap->nitems *= 2;
	}
}

/*
 * Print out a pretty listing of an argument list to terminal
 */

static void
formatf(ap)
	register struct arglist	*ap;
{
	register struct argitem *fp;
	struct entry   *np;
	int		width = 0;
	int		w;
	int		nitems = ap->last - ap->next;
	int		i;
	int		j;
	int		len;
	int		columns;
	int		lines;
	char	       *cp;

	if (ap->next == ap->last)
	{
		return;
	}

	for (fp = ap->next; fp < ap->last; ++fp)
	{
		fp->ftype = inodetype(fp->fnum);

		np = lookupino(fp->fnum);
		if (np != NULL)
		{
			fp->fflags = np->e_flags;
		}
		else
		{
			fp->fflags = 0;
		}

		len = strlen(fmtentry(fp));
		if (len > width)
		{
			width = len;
		}
	}
	width += 2;

	columns = 80 / width;
	if (columns == 0)
	{
		columns = 1;
	}

	lines = (nitems + columns - 1) / columns;
	for (i = 0; i < lines; ++i)
	{
		for (j = 0; j < columns; ++j)
		{
			fp = ap->next + j * lines + i;

			cp = fmtentry(fp);
			msg("%s", cp);

			if (fp + lines >= ap->last)
			{
				msg("\n");
				break;
			}

			for (w = strlen(cp); w < width; ++w)
			{
				msg(" ");
			}
		}
	}
}

/*
 * Format an argument list entry for output to terminal
 */

char	       *
fmtentry(fp)
	register struct argitem *fp;
{
	static char	fmtres[BUFSIZ];
	static int	precision = 0;
	int		i;
	register chfl  *src;
	register char  *dest;

	if (precision == 0)
	{
		for (i = maxino; i > 0; i /= 10)
		{
			++precision;
		}
	}

	if (verbose_flag == FALSE)
	{
		fmtres[0] = '\0';
	}
	else
	{
		(void) sprintf(fmtres, "%*d ", precision, fp->fnum);
	}

	dest = &fmtres[strlen(fmtres)];

	if (debug_flag == TRUE && !MAPBITTEST(dumpmap, fp->fnum))
	{
		*dest = '^';
	}
	else if (fp->fflags & NEW)
	{
		*dest = '*';
	}
	else
	{
		*dest = ' ';
	}
	++dest;

	src = fp->fname;
	while (CHR(src) != '\0')
	{
		if (verbose_flag == TRUE || isprint(CHR(src)) || CHR(src) == ' ')
		{
			*dest = CHR(src);
		}
		else
		{
			*dest = '?';
		}
		++dest, ++src;
	}

	if (fp->ftype == NODE)
	{
		*dest = '/';
		++dest;
	}

	*dest = '\0';
	return(fmtres);
}

/*
 * respond to interrupts
 */

void
sigintr()
{
	if (command == 'i')
	{
		if (reset_valid == TRUE)
			longjmp(reset, 1);
		else
			Exit(1);
	}

	if (query(MSGSTR(RESTINT, "restore interrupted, continue")) == NO)
	{
		Exit(1);

		/* NOTREACHED */
	}
}


char *
strdup(s)
char *s;
{
	char *cp;
	extern char *strcpy();
	extern char *calloc();

	cp = calloc(strlen(s)+1, 1);

	strcpy(cp, s);
	return(cp);
}
