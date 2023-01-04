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
static char	*sccsid = "@(#)$RCSfile: accept_pw.c,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/04/01 20:21:49 $";
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
 * Copyright (c) 1988-90 SecureWare, Inc.
 *   All rights reserved.
 *
 * This routine checks passwords against some criteria for proper
 * passwords.  If any check fails, the routine returns 0.  Otherwise,
 * it returns 1.  The checks that must be passed are as follows (in order
 * of least processing time):
 *
 * 1) The password is not a palindrome. (A palindrome is spelled the same
 *    backwards as forwards.)
 *
 * 2) The password is not too close a wording to a login name.
 *
 * 3) The password is not too close a wording to a group name.
 *
 * 4) The password shows as a spelling mistake by the spell program.
 */


/*
 * Based on:

 */


#include <sys/secdefines.h>

#include "libsecurity.h"

/* #if SEC_BASE */ /*{*/

#include <sys/types.h>
#include <stdio.h>
#include <ctype.h>
#include <pwd.h>
#include <grp.h>

#include <sys/security.h>
#include <sys/audit.h>
#include <prot.h>

#include <stdlib.h>
#include <string.h>
extern void qsort();

static int login_name_derivative();
static int group_name_derivative();
static int palindrome();
static int english_word();
static int derivative();
static int char_comp();

static char *envp[] = {
			"PATH=/usr/bin",
			"H_SPELL=/dev/null",
			"IFS= \t\n",
			(char *)0
		};
static char cr = '\n';
static char buff[80];

static char	*spell_table[] = {
	"/usr/bin/spell",
	(char *) 0
} ;

/*
 * This procedure execs the spell program to
 * test for an English word.
 */

static int 
do_spell(word)
	char *word;
{

	int id;
	int fd1[2],fd2[2],nb;
	char	*table_ptr ;
	int	index ;
	int	access_status ;

	for (table_ptr = spell_table[0], index = 0 ;
	     table_ptr ;
	     table_ptr = spell_table[++index]) {
#if SEC_ARCH
		access_status = eaccess(table_ptr, 01) ;
#else
		access_status = access(table_ptr, 01) ;
#endif
		if (access_status == 0)
			break ;
	}
	if (table_ptr == (char *) 0)
		return(-1) ;
/*
 * We create two pipes to talk to the child process. This
 * avoids having to put the word into a shell command
 * line & having ps spit it out.
 * The variable H_SPELL is used to redirect the list of
 * mis-spelled words to /dev/null - if left to its own
 * way, spell would happily generate a list of passwords,
 * user ids & dates for all to see.
 * We return: -1	error
 *	       0        not word
 *	       1        word.
 */
	if(pipe(fd1) != 0)
	   return(-1);

	if(pipe(fd2) != 0)
	   return(-1);

        id = fork();
	switch (id) {

	    case -1:	
			return(-1);		/* error */

	    case 0:		
			close(fd1[1]);		/* child */
			close(fd2[0]);		/* replace streams */
			close(0);
			close(1);	
			dup(fd1[0]);
			dup(fd2[1]);
			close(fd1[0]);
			close(fd2[1]);
			execle( 
				"/sbin/sh",
				"sh",
				table_ptr,
				(char *)0,
				envp
			);
			return(-1);
	   default:
				break;
	}

	/*
	 * Parent process.
	 * Shut down unwanted fds, then write the word to
	 * be spelled to the child. Shut down the pipe to
	 * the child's stdin, wait for the reply & return.
	 */

	close(fd1[0]);		/* Parent */
	close(fd2[1]);	        /* Shut down unwanted file descs */

	write(fd1[1],word,strlen(word));
	write(fd1[1],&cr,1);

	close(fd1[1]);

	nb = read(fd2[0],buff,80);

	close(fd2[0]);

	wait((char *)0);

	return( (nb < 0 ? -1 : (nb > 0 ? 0 : 1)));
}
	


#ifdef ACC_RAN_DEBUG
main(argc, argv)
	int argc;
	char *argv[];
{
	register int is_acceptable;

	set_auth_parameters(argc, argv);

	if (argc != 2)  {
		fprintf(stderr, MSGSTR(ACCEPT_PW_1, "Usage: %s word\n"), command_name);
		exit(1);
	}

	is_acceptable = acceptable_password(argv[1], stdout);
	fprintf(stdout, MSGSTR(ACCEPT_PW_2, "`%s' is%s an acceptable password\n"), argv[1],
		       is_acceptable ? "" : MSGSTR(ACCEPT_PW_3, " not"));
	fflush(stdout);
}
#endif


/*
 * The driver routine here for checking for acceptable passwords merely
 * invokes the individual checks.  They are invoked in such an order as to
 * run the quickest tests first.  The second argument is the stream to use
 * for explanatory messages for unacceptable passwords.  If it is NULL, no
 * messages are output.  The return value is 1 for a password that passes all
 * the checks and 0 for one that does not (or an internal error had occurred).
 */
int
acceptable_password(word, stream)
	register char *word;
	register FILE *stream;
{
	return	!palindrome(word, stream) &&
		!group_name_derivative(word, stream) &&
		!login_name_derivative(word, stream) &&
		!english_word(word, stream);
}


/*
 * Return 1 if the word is a palindrome and 0 if not.  A palindromic
 * word is one that is spelled the same backwards and forwards.  The
 * empty string is considered a palindrome.  Some palindromes are:
 * noon, radar, mom, redivider, dad, deed.
 *
 * The algorithm is simple -- just march 2 pointers in opposite directions,
 * one at the word start and one at the word end, and if their values have
 * been identical until the pointers cross, the word is a palindrome.
 */
static int
palindrome(word, stream)
	char *word;
	FILE *stream;
{
	register char *front;
	register char *back;
	register int is_palindromic = 1;

	front = word;
	back = word + strlen(word) - 1;

	while (is_palindromic && (front <= back))  {
		is_palindromic = (*front == *back);
		front++;
		back--;
	}

	if (is_palindromic && (stream != (FILE *) 0))  {
		fprintf(stream,
			MSGSTR(ACCEPT_PW_4, "Password must not be a palindrome, and `%s' is one.\n"),
			word);
		fflush(stream);
	}

	return is_palindromic;
}


/*
 * Return 1 if the word appears as a login name or a closely related
 * string, and a 0 if not.  Look at all login names because the similarities
 * between the password and login name may be with another account the user
 * has or is privy to.  Also, close the password file before returning.
 */
static int
login_name_derivative(word, stream)
	register char *word;
	FILE *stream;
{
	register struct passwd *p;
	register int found = 0;

	setpwent();
	while (!found && ((p = getpwent()) != (struct passwd *) 0))  {
		found = derivative(word, p->pw_name);
	}
	endpwent();

	if (found && (stream != (FILE *) 0))  {
		fprintf(stream,
	    MSGSTR(ACCEPT_PW_5, "`%s' is (or looks too much like) a login name to be a password.\n"),
			word);
		fflush(stream);
	}

	return found;
}


/*
 * Return 1 if the word appears as a group name or a closely related
 * string, and a 0 if not.  Look at all group names because the similarities
 * between the password and group name may be with another account the user
 * has or is privy to.  Also, close the password file before returning.
 */
static int
group_name_derivative(word, stream)
	char *word;
	FILE *stream;
{
	register struct group *g;
	register int found = 0;

	setgrent();
	while (!found && ((g = getgrent()) != (struct group *) 0))  {
		found = derivative(word, g->gr_name);
	}
	endgrent();

	if (found && (stream != (FILE *) 0))  {
		fprintf(stream,
	    MSGSTR(ACCEPT_PW_6, "`%s' is (or looks too much like) a group name to be a password.\n"),
			word);
		fflush(stream);
	}
	return found;
}


/*
 * See if the word is in the dictionary.  This is a slow
 * process, since we need to start the speller for each word.
 * The alternative is to recreate the hashing scheme for the
 * dictionary, which is vastly different on UNIX versions.
 * Return 1 if the word is legal or there is an error in the
 * determination, and return 0 if it can definitely be determined
 * (by the spell program or because all the characters were not
 * alphabetic) that the word is a misspelling.
 */
static int
english_word(word, stream)
	register char *word;
	FILE *stream;
{
	register char *actual_spell_line;
	register char *lower_case_word;
	register int word_scan;
	register int is_word = 1;
	int spell_program;
	int word_len;
	int line_len;
	char spell_out;

	word_len = strlen(word);
	lower_case_word = malloc(word_len + 1);
	if (lower_case_word != (char *) 0)  {
	  for (word_scan = 0; is_word && (word_scan < word_len);
	       word_scan++)  {
		is_word = (isascii(word[word_scan]) &&
			   isalpha(word[word_scan]));
		if (isascii(word[word_scan]) && isupper(word[word_scan]))
			lower_case_word[word_scan] = tolower(word[word_scan]);
		else
			lower_case_word[word_scan] = word[word_scan];
	  }
	  lower_case_word[word_len] = '\0';

	  if (is_word)  {
		spell_program = do_spell(lower_case_word);
		if(spell_program != -1)
		   is_word = spell_program; 
	  }

	  /*
	   * Wipe this remnant of the password from the system.
	   */
	  memset(lower_case_word, '\0', word_len);
	  free(lower_case_word);
	}

	if (is_word && (stream != (FILE *) 0))  {
		fprintf(stream,
			MSGSTR(ACCEPT_PW_7, "`%s' is an English word, and passwords may not be.\n"),
			word);
		fflush(stream);
	}

	return is_word;
}


/*
 * See if two words are `close'.  For now, that means that they share
 * the same characters (ignoring case).  In the future, more complex
 * checking can be done such as checking if MOST characters are shared or
 * that the character frequencies are similar.
 */
static int
derivative(word1, word2)
	char *word1;
	char *word2;
{
	register char *wordbuf1;
	register char *wordbuf2;
	register int alike = 1;
	register unsigned word_scan;
	unsigned word1_size;
	unsigned word2_size;

	word1_size = (unsigned) strlen(word1);
	wordbuf1 = malloc(word1_size + 1);
	if (wordbuf1 != (char *) 0) {
		word2_size = (unsigned) strlen(word2);
		wordbuf2 = malloc(word2_size + 1);
		if (wordbuf2 != (char *) 0) {
			for (word_scan = 0; word_scan <= word1_size;
			     word_scan++)
				if (isascii(word1[word_scan]) &&
				    isupper(word1[word_scan]))
					wordbuf1[word_scan] =
						tolower(word1[word_scan]);
				else
					wordbuf1[word_scan] = word1[word_scan];

			for (word_scan = 0; word_scan <= word2_size;
			     word_scan++)
				if (isascii(word2[word_scan]) &&
				    isupper(word2[word_scan]))
					wordbuf2[word_scan] =
						tolower(word2[word_scan]);
				else
					wordbuf2[word_scan] = word2[word_scan];

			qsort(wordbuf1, word1_size, sizeof(*word1), char_comp);
			qsort(wordbuf2, word2_size, sizeof(*word2), char_comp);
			alike = (strcmp(wordbuf1, wordbuf2) == 0);

			free(wordbuf2);
		}
		free(wordbuf1);
	}

	return alike;
}


/*
 * Character comparison routine for qsort().  qsort() expects to call this
 * routine with 2 arguments of type `char *'.  The return value is <0 if
 * the first argument is lexically less than the second, 0 if the same,
 * and >0 for all other cases.
 */
static int
char_comp(pel1, pel2)
	char *pel1;
	char *pel2;
{
	return ((int) (*pel1 - *pel2));
}
/* #endif */ /*} SEC_BASE */
