/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*  Copyright (c) 1988-90 SecureWare, Inc.
 *    All rights reserved
 *
 *  Yacc grammar for ACL synonym compiler
 */

%{
#ident "@(#)acl_parse.y	3.1 10:02:42 6/7/90 SecureWare"

/*
 * Based on:
 *   "@(#)acl_parse.y	2.2 13:53:50 6/6/89"
 */

#include <sys/secdefines.h>

#if !SEC_ACL_SWARE
This should not be compiled if SecureWare ACLs are not configured.
#endif

#include <sys/types.h>
#include <stdio.h>
#include <ctype.h>
#include <pwd.h>
#include <grp.h>
#include <acl.h>

#ifdef UID_MAX
#undef UID_MAX
#define	UID_MAX		60001
#endif

#define	STREQ(s1,s2)	(strcmp(s1,s2)==0)

extern FILE	*inputfp;	/* current input file descriptor */
extern char	*inputfile;	/* current input file name */
extern int	inputline;	/* current input line number */
extern int	syn_error;	/* cumulative error counter */
extern int	mem_error;

extern char	*strchr(), *malloc();
extern struct passwd	*getpwnam();
extern struct group	*getgrnam();

%}

%union {
	aclsyn_t	*synp;
	char		*wordp;
	int		num;
}

%token <num>	NUMBER
%token <wordp>	WORD

%type <synp>	synonym acl
%type <num>	uid gid id perm

%%

synonym_list:
	/* empty */
	| synonym_list synonym
		{
			acl_insert_syn($2);
		}
	;

synonym:
	WORD '=' acl '\n'
		{
			char	err[40];

			if (acl_lookup_syn($1)) {
				sprintf(err, "synonym redefined (%.16s)", $1);
				yyerror(err);
				free($1);
				acl_free_syn($3);
				$$ = (aclsyn_t *)NULL;
			} else {
				$3->syn_name = $1;
				$$ = $3;
			}
		}
	;
	
acl:
	/* empty */
		{
			$$ = acl_alloc_syn(0);
			if ($$ == NULL) {
				if (mem_error++ == 0)
					yyerror("out of memory");
			}
		}
	| acl WORD
		{
			aclsyn_t	*sp;
			char	err[40];

			if (sp = acl_lookup_syn($2))
				copy_entries(sp, $1);
			else {
				sprintf(err, "undefined synonym (%.16s)", $2);
				yyerror(err);
			}
			free($2);
			$$ = $1;
		}
	| acl '<' uid '.' gid ',' perm '>'
		{
			add_entry($1, $3, $5, $7);
			$$ = $1;
		}
	;

uid:
	WORD
		{
			struct passwd	*pw;
			char		err[40];

			if (pw = getpwnam($1))
				$$ = pw->pw_uid;
			else {
				sprintf(err, "undefined user name (%.16s)", $1);
				yyerror(err);
				$$ = -1;
			}
			free($1);
		}
	| id
	;

gid:
	WORD
		{
			struct group	*gr;
			char		err[40];

			if (gr = getgrnam($1))
				$$ = gr->gr_gid;
			else {
				sprintf(err, "undefined group name (%.16s)",
					$1);
				yyerror(err);
				$$ = -1;
			}
			free($1);
		}
	| id
	;

id:
	NUMBER
		{
			char	err[40];

			if ((unsigned)$1 > UID_MAX) {
				sprintf(err, "illegal user or group ID (%u)",
					$1);
				$$ = -1;
			} else
				$$ = $1;
		}
	| '@'
		{
			$$ = ACL_OWNER;
		}
	| '*'
		{
			$$ = ACL_WILDCARD;
		}
	;

perm:
	WORD
		{
			register char	*cp;
			char		err[40];
			int		perm;

			for (cp = $1; *cp; ++cp)
				if (isupper(*cp))
					*cp = tolower(*cp);

			if (STREQ($1, "all"))
				perm = ACL_READ | ACL_WRITE | ACL_EXEC;
			else if (STREQ($1, "none") || STREQ($1, "null"))
				perm = 0;
			else for (perm = 0, cp = $1; *cp; ++cp)
				switch (*cp) {
				case 'r':
					perm |= ACL_READ;
					break;
				case 'w':
					perm |= ACL_WRITE;
					break;
				case 'x':
					perm |= ACL_EXEC;
					break;
				case '-':
					break;
				default:
					sprintf(err,
						"illegal permissions (%.16s)",
						$1);
					yyerror(err);
					cp[1] = '\0';
					perm = 0;
					break;
				}
			free($1);
			$$ = perm;
		}
	| NUMBER
		{
			char	err[40];

			if ((unsigned)$1 > 7) {
				sprintf(err, "illegal permissions (%u)", $1);
				yyerror(err);
				$$ = 0;
			} else
				$$ = $1;
		}

%%

/*
 * YACC support functions
 */

yyerror(msg)
	char	*msg;
{
	fprintf(stderr, "\"%s\", line %d: %s\n", inputfile, inputline, msg);
	syn_error++;
}

static char	line[256];
static char	*lp = line;
static char	whitespace[] =	" \t";
static char	wordends[] =	" \t=<@*.,>\n";

yylex()
{
	register char	*cp = lp;
	char		c;
	register int	num, r;
	int		token;

	cp += strspn(cp, whitespace);	/* skip whitespace */
	/*
	 * Remember the current input character for end of
	 * line processing.  We only return a newline when
	 * the token following it starts at the beginning of
	 * a line.
	 */
	c = *cp;

	/*
	 * Handle the end of an input line.  Strip comments
	 * and skip over empty lines.
	 */
	while (*cp == '\0' || *cp == '\n') {
		if (feof(inputfp)
		||  fgets(line, sizeof(line), inputfp) == NULL) {
			line[0] = '\0';
			lp = line;
#ifdef DEBUG
			printf("yylex returns %d at EOF\n", c);
#endif
			return c;
		}
		++inputline;
		if (cp = strchr(line, '#')) {	/* truncate comments */
			*cp++ = '\n';
			*cp = '\0';
		}
		cp = line + strspn(line, whitespace);	/* skip whitespace */
	}

	/*
	 * If token starts at begining of line and last
	 * token on previous line was newline (always true
	 * except at startup), return the newline first.
	 */
	if (cp == line && c == '\n') {
		lp = cp;
#ifdef DEBUG
		printf("yylex returns NEWLINE\n");
#endif
		return c;
	}

	/*
	 * Figure out what kind of token we have.
	 */
	switch (*cp) {
	case '<':	/* single character tokens represent themselves */
	case '>':
	case '.':
	case ',':
	case '=':
	case '@':
	case '*':
		token = *cp++;
#ifdef DEBUG
		printf("yylex returns '%c'\n", token);
#endif
		break;
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
		num = *cp++ - '0';
		/*
		 * Interpret numbers with leading 0 or 0x
		 * according to C conventions.
		 */
		if (num == 0)
			if (*cp == 'x' || *cp == 'X') {
				r = 16;
				++cp;
			} else
				r = 8;
		else
			r = 10;
		while (isdigit(*cp) || r == 16 && isxdigit(*cp)) {
			num *= r;
			num += *cp - (isdigit(*cp)?'0':(islower(*cp)?'a':'A'));
			++cp;
		}
		yylval.num = num;
		token = NUMBER;
#ifdef DEBUG
		printf("yylex returns NUMBER (%u)\n", num);
#endif
		break;
	default:
		/*
		 * Anything that's not a number or a single
		 * character token is the start of a WORD
		 * token which is handled below.
		 */
		token = WORD;
		break;
	}

	/*
	 * If token is a WORD, note its starting position.
	 * Otherwise, remember input position to start
	 * scanning on next call and return.
	 */
	lp = cp;
	if (token != WORD)
		return token;
	
	/*
	 * Find end of WORD
	 */
	while (*++cp && strchr(wordends, *cp) == NULL)
		;

	yylval.wordp = malloc(cp - lp + 1);
	if (yylval.wordp == NULL) {
		if (mem_error++ == 0)
			yyerror("out of memory");
	} else {
		strncpy(yylval.wordp, lp, cp - lp);
		yylval.wordp[cp - lp] = '\0';
	}
	lp = cp;
#ifdef DEBUG
	printf("yylex returns WORD \"%s\"\n", yylval.wordp);
#endif
	return WORD;
}
