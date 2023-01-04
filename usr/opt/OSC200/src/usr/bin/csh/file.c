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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: file.c,v $ $Revision: 4.2.10.3 $ (DEC) $Date: 1993/12/21 19:12:28 $";
#endif
/*
 * HISTORY
 */
/*
 * COMPONENT_NAME: CMDCSH  c shell(csh)
 *
 * FUNCTIONS: setup_tty, back_to_col_1, pushback, catn, copyn, filetype, 
 *            print_by_column, tilde, retype, beep, print_recognized_stuff, 
 *            extract_dir_and_name, getentry, free_items, search, recognize, 
 *	      is_prefix, is_suffix, ignored, sortscmp, tenex_search, tenex
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 *
 *	1.10  com/cmd/csh/file.c, 9123320, bos320 5/20/91 12:48:37"	
 *
 *	sh.file.c	5.6 (Berkeley) 5/18/86";
 */

/*
 * Tenex style file name recognition, .. and more.
 * History:
 *	Author: Ken Greer, Sept. 1975, CMU.
 *	Finally got around to adding to the Cshell., Ken Greer, Dec. 1981.
 */

#include <dirent.h>
#include <pwd.h>
#include <string.h>
#include "sh.h"

#define ON	TRUE	
#define OFF	FALSE

#define ESC	'\033'

#define FREE_ITEMS(items) {					\
	int omask;						\
								\
	omask = sigblock(sigmask(SIGINT));			\
	free_items(items);					\
	items = NULL;						\
	(void)sigsetmask(omask);				\
}


/*
 * Local static function prototypes.
 */

static void		setup_tty(int);
static void		back_to_col_1();
static void		pushback(uchar_t *);
static void		catn(uchar_t *, uchar_t *, int);
static void		copyn(uchar_t *, uchar_t *, int);
static uchar_t		filetype(uchar_t *, uchar_t *); 
static void		print_by_column(uchar_t *, uchar_t *[], int);
static uchar_t *	tilde(uchar_t *, uchar_t *);
static void		retype();
static void		beep();
static void		print_recognized_stuff(uchar_t *);
static void		extract_dir_and_name(uchar_t *, uchar_t *, uchar_t *);
static uchar_t *	getentry(DIR *, int);
static void		free_items(uchar_t **);
static int		recognize(uchar_t *, uchar_t *, int, int);
static int		is_prefix(uchar_t *, uchar_t *);
static int		is_suffix(uchar_t *, uchar_t *);
static int		ignored(uchar_t *);

/*
 * Put this here so the binary can be patched with adb to enable file
 * completion by default.  Filec controls completion, nobeep controls
 * ringing the terminal bell on incomplete expansions.
 */

/* bool filec = 0; */

static void
setup_tty(int on)
{
	struct termios fx_resize;
	struct sgttyb sgtty;
	static struct tchars tchars;    /* INT, QUIT, XON, XOFF, EOF, BRK */

	if (on) {
		IOCTL(SHIN, TIOCGETC, &tchars, "2");
		tchars.t_brkc = ESC;
		IOCTL(SHIN, TIOCSETC, &tchars, "3");
		/*
		 * This must be done after every command: if
		 * the tty gets into raw or cbreak mode the user
		 * can't even type 'reset'.
		 */
		IOCTL(SHIN, TIOCGETA, &fx_resize, "44");
		if ((sgtty.sg_flags & ECHOCTL) == 0) { /* to fix filec after */
			fx_resize.c_lflag |= ECHOCTL; /* resize C. Richmond */
			IOCTL(SHIN, TIOCSETA, &fx_resize, "55");
		}
		IOCTL(SHIN, TIOCGETP, &sgtty, "4");
		if (sgtty.sg_flags & (RAW|CBREAK)) {
			sgtty.sg_flags &= ~(RAW|CBREAK);
			IOCTL(SHIN, TIOCSETP, &sgtty, "5");
		}
	} else {
		tchars.t_brkc = -1;
		IOCTL(SHIN, TIOCSETC, &tchars, "6");
	}
}

/*
 * Move back to beginning of current line
 */
static void
back_to_col_1()
{
	int omask;
        struct sgttyb tty, tty_normal;

	omask = sigblock(sigmask(SIGINT));
	IOCTL(SHIN, TIOCGETP, &tty, "7");
	tty_normal = tty;
	tty.sg_flags &= ~CRMOD;
	IOCTL(SHIN, TIOCSETN, &tty, "8");
	(void) write(SHOUT, "\r", 1);
	IOCTL(SHIN, TIOCSETN, &tty_normal, "9");
	(void) sigsetmask(omask);
}

/*
 * Push string contents back into tty queue
 */
static void
pushback(uchar_t *string)
{
        int omask;
        register uchar_t *p;
        struct sgttyb tty, tty_normal;

	omask = sigblock(sigmask(SIGINT));
        IOCTL(SHOUT, TIOCGETP, &tty, "10");
        tty_normal = tty;
        tty.sg_flags &= ~ECHO;
        IOCTL(SHOUT, TIOCSETN, &tty, "11");

        for (p = string; *p; p++) {
		IOCTL(SHOUT, TIOCSTI, p, "12");
	}
	IOCTL(SHOUT, TIOCSETN, &tty_normal, "13");
	(void) sigsetmask(omask);	
}



/*
 * Concatenate src onto tail of des.
 * Des is a string whose maximum length is count.
 * Always null terminate.
 */
static void
catn(uchar_t *des, uchar_t *src, int count)
{

	while (--count >= 0 && *des)
		des++;
	while (--count >= 0)
		if ((*des++ = *src++) == 0)
			 return;
	*des = '\0';
}

/*
 * Like strncpy but always leave room for trailing \0
 * and always null terminate.
 */
static void
copyn(uchar_t *des, uchar_t *src, int count)
{

	while (--count >= 0)
		if ((*des++ = *src++) == 0)
			return;
	*des = '\0';
}

static uchar_t
filetype(uchar_t *dir, uchar_t *file)
{
	uchar_t path[PATH_MAX+1];
	struct stat statb;

	catn((uchar_t *)strcpy((char *)path, (char *)dir), file, sizeof path);
	if (lstat((char *)path, &statb) == 0) {
		switch(statb.st_mode & S_IFMT) {
		    case S_IFDIR:
			return ('/');

		    case S_IFLNK:
			if (stat((char *)path, &statb) == 0 && /* follow it */
			   (statb.st_mode & S_IFMT) == S_IFDIR)
				return ('>');
			else
				return ('@');

		    case S_IFSOCK:
			return ('=');

		    default:
			if (statb.st_mode & 0111)
				return ('*');
		}
	}
	return (' ');
}

static struct winsize win;

/*
 * Print sorted down columns
 */
static void
print_by_column(uchar_t *dir, uchar_t *items[], int count)
{
	register int i, rows, r, c, maxwidth = 0, columns;

	 if (ioctl(SHOUT, TIOCGWINSZ, (char *)&win) < 0 || win.ws_col == 0)
		win.ws_col = 80;
	for (i = 0; i < count; i++)
		maxwidth = maxwidth > (r = strlen((char *)items[i])) ? maxwidth : r;
	maxwidth += 2;			/* for the file tag and space */
	columns = win.ws_col / maxwidth;
	if (columns == 0)
		columns = 1;
	rows = (count + (columns - 1)) / columns;
	for (r = 0; r < rows; r++) {
		for (c = 0; c < columns; c++) {
			i = c * rows + r;
			if (i < count) {
				register int w;

				csh_printf("%s", items[i]);
				display_char(dir ? filetype(dir, items[i]) : ' ');
				if (c < columns - 1) {	/* last column? */
					w = strlen((char *)items[i]) + 1;
					for (; w < maxwidth; w++)
						display_char(' ');
				}
			}
		}
		display_char('\n');
	}
}

/*
 * Expand file name with possible tilde usage
 *	~person/mumble
 * expands to
 *	home_directory_of_person/mumble
 */
static uchar_t *
tilde(uchar_t *new, uchar_t *old)
{
	register uchar_t *o, *p;
	register struct passwd *pw;
	static uchar_t person[MAX_LOG_NAMLEN];

	if (old[0] != '~')
		return ((uchar_t *)strcpy((char *)new, (char *)old));

	for (p = person, o = &old[1]; *o && *o != '/'; *p++ = *o++)
		;
	*p = '\0';
	if (person[0] == '\0')
		(void) strcpy((char *)new, (char *)value((uchar_t *)"home"));
	else {
		pw = getpwnam((char *)person);
		if (pw == NULL)
			return (NULL);
		(void) strcpy((char *)new, (char *)pw->pw_dir);
	}
	(void) strcat((char *)new, (char *)o);
	return (new);
}

/*
 * Cause pending line to be printed
 */
static void
retype()
{
        int pending_input = LPENDIN;
        IOCTL(SHOUT, TIOCLBIS, &pending_input, "14");
}

static void
beep()
{

	if (adrof((uchar_t *)"nobeep") == 0)
		(void) write(SHOUT, "\007", 1);
}

/*
 * Erase that silly ^[ and
 * print the recognized part of the string
 */
static void
print_recognized_stuff(uchar_t *recognized_part)
{

	/* An optimized erasing of that silly ^[ */
	switch (strlen((char *)recognized_part)) {

	case 0:				/* erase two character: ^[ */
		csh_printf("\b\b  \b\b");
		break;

	case 1:				/* overstrike the ^, erase the [ */
		csh_printf("\b\b%s \b", recognized_part);
		break;

	default:			/* overstrike both character ^[ */
		csh_printf("\b\b%s", recognized_part);
		break;
	}
	flush();
}

/*
 * Parse full path in file into 2 parts: directory and file names
 * Should leave final slash (/) at end of dir.
 */
static void
extract_dir_and_name(uchar_t *path, uchar_t *dir, uchar_t *name)
{
	register uchar_t  *p;

	p = (uchar_t *)rindex((char *)path, '/');
	if (p == NULL) {
		copyn(name, path, (PATH_MAX));
		dir[0] = '\0';
	} else {
		copyn(name, ++p, (PATH_MAX));
		copyn(dir, path, p - path);
	}
}

static uchar_t *
getentry(DIR *dir_fd, int looking_for_lognames)
{
	register struct passwd *pw;
	register struct dirent *dirp;

	if (looking_for_lognames) {
		if ((pw = getpwent()) == NULL)
			return (NULL);
		return ((uchar_t *)pw->pw_name);
	}
	if (dirp = readdir(dir_fd))
		return ((uchar_t *)dirp->d_name);
	return (NULL);
}

static void
free_items(uchar_t **items)
{
	register int i;

	for (i = 0; items[i]; i++)
		free(items[i]);
	free((char *)items);
}


/*
 * Object: extend what user typed up to an ambiguity.
 * Algorithm:
 * On first match, copy full entry (assume it'll be the only match) 
 * On subsequent matches, shorten extended_name to the first
 * character mismatch between extended_name and entry.
 * If we shorten it back to the prefix length, stop searching.
 */
static int
recognize(uchar_t *extended_name, uchar_t *entry, int name_length, int numitems)
{

	if (numitems == 1)			/* 1st match */
		copyn(extended_name, entry, PATH_MAX);
	else {					/* 2nd & subsequent matches */
		register uchar_t *x, *ent;
		register int len = 0;

		x = extended_name;
		for (ent = entry; *x && *x == *ent++; x++, len++)
			;
		*x = '\0';			/* Shorten at 1st uchar_t diff */
		if (len == name_length)		/* Ambiguous to prefix? */
			return (-1);		/* So stop now and save time */
	}
	return (0);
}

/*
 * Return true if check matches initial characters in template.
 * This differs from PWB imatch in that if check is null
 * it matches anything.
 */
static int
is_prefix(register uchar_t *check, register uchar_t *template)
{

	do
		if (*check == 0)
			return (TRUE);
	while (*check++ == *template++);
	return (FALSE);
}

/*
 *  Return true if the characters in template appear at the
 *  end of check, I.e., are it's suffix.
 */
static int
is_suffix(uchar_t *check, uchar_t *template)
{
	register uchar_t *c, *t;

	for (c = check; *c++;)
		;
	for (t = template; *t++;)
		;
	for (;;) {
		if (t == template)
			return 1;
		if (c == check || *--t != *--c)
			return 0;
	}
}

static int
ignored(register uchar_t *entry)
{
	struct varent *vp;
	register uchar_t **cp;

	if ((vp = adrof((uchar_t *)"fignore")) == NULL || 
		(cp = vp->vec) == NULL)
		return (FALSE);
	for (; *cp != NULL; cp++)
		if (is_suffix(entry, *cp))
			return (TRUE);
	return (FALSE);
}

/*
 * String compare for qsort.
 */
int
sortscmp(const uchar_t **a1, const uchar_t **a2)
{
	return (strcoll((char *)*a1, (char *)*a2));
}

/*
 * Perform a RECOGNIZE or LIST command on string "word".
 */
int
tenex_search(uchar_t *word, COMMAND command, int max_word_length)
{
#define MAXITEMS 1024
	static uchar_t **items = NULL;
	register DIR *dir_fd;
	register int numitems = 0, ignoring = TRUE, nignored = 0;
	register name_length, looking_for_lognames;
	uchar_t tilded_dir[PATH_MAX + 1], dir[PATH_MAX + 1];
	uchar_t name[PATH_MAX + 1], extended_name[PATH_MAX+1];
	uchar_t *entry;
	uchar_t *org_word;

	if (items != NULL)
		FREE_ITEMS(items);
        org_word = word;
        if (index ((char *)word,'$') != NULL) {
                uchar_t    *p;

                if ((p = (uchar_t *)rindex((char *)word, '/')) == NULL || 
		     index((char *)p, '$') != NULL) {
                        return (0);     /* leave if searching only $foo */
		}
                word = Dfix1(word);
        }

	looking_for_lognames = 
			((*word == '~') && (index((char *)word, '/') == NULL));
	if (looking_for_lognames) {
		(void) setpwent();
		copyn(name, &word[1], PATH_MAX);	/* name sans ~ */
	} else {
		extract_dir_and_name(word, dir, name);
		if (tilde(tilded_dir, dir) == 0)
			return (0);
		dir_fd = opendir(*tilded_dir ? (char *)tilded_dir : ".");
		if (dir_fd == NULL)
			return (0);
	}

again:	/* search for matches */
	name_length = strlen((char *)name);
	for (numitems = 0; entry = getentry(dir_fd, looking_for_lognames); ) {
		if (!is_prefix(name, entry))
			continue;
		/* Don't match . files on null prefix match */
		if (name_length == 0 && entry[0] == '.' &&
		    !looking_for_lognames)
			continue;
		if (command == LIST) {
			if (numitems >= MAXITEMS) {
				if(looking_for_lognames)
					csh_printf (MSGSTR(YIKES1,"\nYikes!! Too many names in password file!!\n"));
				else
					csh_printf (MSGSTR(YIKES2,"\nYikes!! Too many files!!\n"));
				break;
			}
			if (items == NULL)
				items = (uchar_t **)calloc(sizeof (items[1]),
				    MAXITEMS);
			items[numitems] = calloc(1,(unsigned)strlen((char *)entry)+1);
			copyn(items[numitems], entry, PATH_MAX);
			numitems++;
		} else {			/* RECOGNIZE command */
			if (ignoring && ignored(entry))
				nignored++;
			else if (recognize(extended_name,
			    entry, name_length, ++numitems))
				break;
		}
	}
	if (ignoring && numitems == 0 && nignored > 0) {
		ignoring = FALSE;
		nignored = 0;
		if (looking_for_lognames)
			(void) setpwent();
		else
			rewinddir(dir_fd);
		goto again;
	}

	if (looking_for_lognames)
		(void) endpwent();
	else
		closedir(dir_fd);
	if (numitems == 0)
		return (0);
	if (command == RECOGNIZE) {
		/* add extended name */
                catn(org_word, extended_name+name_length, max_word_length);
		return (numitems);
	}
	else { 				/* LIST */
		qsort(items, numitems, sizeof(items[1]), 
			(int(*)(const void *, const void *))sortscmp);
		print_by_column(looking_for_lognames ? NULL : tilded_dir,
		    items, numitems);
		if (items != NULL)
			FREE_ITEMS(items);
	}
	return (0);
}

int
tenex(uchar_t *inputline, int inputline_size)
{
	register int numitems, num_read;
	setup_tty(ON);
	while ((num_read = read(SHIN, inputline, inputline_size)) > 0) {
		const static uchar_t delims[] = {  /* WORD DELIMITERS */
			' ',		/* space 		*/
			'\'',		/* apostrophe		*/
			'"',		/* quote		*/
			'\t',		/* tab			*/
			';',		/* semicolon		*/
			'&',		/* ampersand		*/
			'<',		/* left angle		*/
			'>',		/* right angle		*/
			'(',		/* left paren		*/
			')',		/* right paren		*/
			'|',		/* vertical bar		*/
			'^',		/* circumflex		*/
			'%',		/* percent		*/
			'\0'
		};
		register uchar_t *str_end, *word_start, *last_char, should_retype;
		register int space_left;
		COMMAND command;

		last_char = inputline + num_read - 1;
		if (*last_char == '\n' || num_read == inputline_size)
			break;
		command = (*last_char == ESC) ? RECOGNIZE : LIST;
		if (command == LIST)
			display_char('\n');
		str_end = last_char + 1;
		if (*last_char == ESC)
			--str_end;	/* wipeout trailing cmd uchar_t */
		*str_end = '\0';
		/*
		 * Find LAST occurence of a delimiter in the inputline.
		 * The word start is one character past it.
		 */
#ifndef _SBCS
		for (word_start = last_char = inputline; last_char < str_end;) {
			register int n;
			wchar_t nlc;
			n = mbtowc(&nlc, (char *)last_char, mb_cur_max);
			if (n < 1)
				n = 1;
			else if (any(nlc,(uchar_t *)delims) || 
					iswblank((wint_t)nlc))
				word_start = last_char + n;
			last_char += n;
		}
#else
		for (word_start = str_end - 1; word_start >= inputline; 
			--word_start)
			if (any(*word_start,delims))
				break;
		word_start++;
#endif
		space_left = inputline_size - (word_start - inputline) - 1;
		numitems = tenex_search(word_start, command, space_left);

		if (command == RECOGNIZE) {
			/* print from str_end on */
			print_recognized_stuff(str_end);
			if (numitems != 1)	/* Beep = No match/ambiguous */
				beep();
		}

		/*
		 * Tabs in the input line cause trouble after a pushback.
		 * tty driver won't backspace over them because column
		 * positions are now incorrect. This is solved by retyping
		 * over current line.
		 */
		should_retype = FALSE;
		if (strchr((char *)inputline, '\t')) {  /* tab in input line */
			back_to_col_1();
			should_retype = TRUE;
		}
		if (command == LIST)	/* Always retype after a LIST */
			should_retype = TRUE;
		if (should_retype)
			printprompt();
		pushback(inputline);
		if (should_retype)
			retype();
	}
	setup_tty(OFF);
	return (num_read);
}
