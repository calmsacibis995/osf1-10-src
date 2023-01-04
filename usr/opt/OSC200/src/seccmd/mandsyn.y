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
/*
 * @(#)$RCSfile: mandsyn.y,v $ $Revision: 4.1.6.2 $ (DEC) $Date: 1992/04/14 17:08:07 $
 */
/*
 * OSF/1 Release 1.0
 */
/*  Copyright (c) 1988-90 SecureWare, Inc.
 *    All rights reserved
 *
 *  Synonym compiler for classifications, category sets, and sens. labels
 *
 *  Program to compile synonym files into a synonyms database.
 *
 *	mandsyn [-d database] [synonym] ...	# print synonyms
 *	mandsyn [-d database] -r [file] ...	# recompile the database
 *	mandsyn [-d database] -a [file] ...	# add to the database
 *	mandsyn [-d database] -u synonym ...	# undefine synonyms
 *
 */	

%{
#ident "@(#)mandsyn.y	3.1 10:03:04 6/7/90 SecureWare"
/*
 * Based on:
 *   "@(#)mandsyn.y	2.5 16:46:47 9/18/89 SecureWare"
 */

/* YACC input file for constructing the synonyms for sensitivity labels.
 * The tokens are received from the mandatory lexical analyzer, yylex().
 */

#include <sys/secdefines.h>

#if SEC_MAC_OB /*{*/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>
#include <fcntl.h>
#include <sys/security.h>
#include <mandatory.h>
#include <sys/signal.h>
#include <sys/stat.h>

#define	PRINT		'p'
#define	REPLACE		'r'
#define	ADD		'a'
#define	UNDEFINE	'u'

/* locking implemented on the synonyms database */

#define SYN_LOCK_RETRIES 3

/* synonyms are defined either as class only, category set only, or full
 * sensitivity label.  Each type has a synonym name.
 */

struct	synonym {
	int	type;		/* see below */
	char	*name;		/* synonym name itself */
	long	class;		/* classification */
	mask_t	*cat_set;	/* category set - indirect for var. size */
	struct	synonym	*next;	/* for linked list */
};
#define SYN_NULL ((struct synonym *) 0)

/*
 * A prefix is defined as anything that can appear at the front of
 * the right hand side of a synonym definition.
 */

struct prefix {
	char	*partial_word;	/* partial word unmatched so far */
	struct synonym *synonym;/* synonym matched so far */
};
#define PREFIX_NULL ((struct prefix *) 0)

struct	synonym *mand_syns = SYN_NULL; /* linked list of synonyms */
FILE		*syn_fp;		/* synonym file pointer */
FILE		*db_fp;			/* database file pointer */
int		syn_error = 0;		/* incremented on error */
static	int	mem_error = 0;		/* only report memory alloc once */
static	int	line_number = 1;	/* for syntax error messages */

static	char	*outfile = (char *) 0;
static	char	*infile = (char *) 0;
static	char	*prog;
/* new database file, which must be removed if program aborts abnormally */
static	char	new_db_file[sizeof(MAND_SYN_DB) + sizeof(MAND_EXTENSION) - 1];
static	char	*temp_db = (char *) 0;
static	char	*temp_file = (char *) 0; /* temp file used for rules */

static struct synonym	*category_syn();
static struct synonym	*add_word();
static struct synonym	*add_category();
static char		*new_word();
static mask_t		*alloc_categories();
static void		copy_categories();
static struct synonym	*alloc_synonym();
static void		free_synonym();
static struct synonym	*insert_into_list();
static struct synonym	*lookup_synonym_by_name();
static	void		usage();
static void		add_synonyms();
static void		replace_synonyms();
static void		print_synonym();
static void		rm_tmpfile();
static struct synonym	*add_cats_to_syn();
static struct synonym	*sub_cats_from_syn();
static struct synonym	*make_cat_syn();
static struct synonym	*alias_synonym();

static struct prefix	*allocate_prefix();
static void		free_prefix();
static struct prefix	*add_to_prefix();
static struct synonym	*add_cat_to_prefix();
static struct prefix	*make_prefix();

extern char		*malloc(), *calloc();
#endif /*} SEC_MAC_OB */
%}

/* NOTE: end marker is 0 or negative */

%union {
	struct	synonym		*synp;
	char			*wordp;
	int			number;
	struct prefix		*prefixp;
}

%token EQUAL SLASH COMMA QUOTE WHITE NEWLINE PLUS MINUS WORD

%type <synp> synonym_list synonym rhs
%type <synp> cat_synonym cat_token_list cat_literal cat_set
%type <prefixp> prefix
%type <wordp> WORD words quoted_word

%LEFT PLUS MINUS

%%	/* beginning of rules section */

synonym_list : synonym
	{
		if ($1)
			$$ = insert_into_list($1);
	}
	| synonym_list synonym
	{
		if ($2)
			$$ = insert_into_list($2);
	}
	| synonym_list NEWLINE
	{
		line_number++;
	}
	| NEWLINE
	{
		line_number++;
	}
	;

synonym : words EQUAL rhs NEWLINE
	{
		if ($3)
			$$ = add_word($1, $3);
		else
			$$ = SYN_NULL;
		line_number++;
	}
	| error NEWLINE
	{
		(void) fprintf(stderr,
		  "Line %3d: syntax error\n", line_number);
		line_number++;
		yyerrok;
		syn_error++;
		$$ = SYN_NULL;
	}
	| words EQUAL error NEWLINE
	{
		(void) fprintf(stderr,
		  "Line %3d: syntax error on synonym %s\n", line_number, $1);
		line_number++;
		yyerrok;
		syn_error++;
		$$ = SYN_NULL;
	}
	;
	
rhs	: prefix cat_literal
	{
		$$ = add_cat_to_prefix($1, $2);
	}
	| cat_literal
	| rhs PLUS cat_set
	{
		$$ = add_cats_to_syn($1, $3);
	}
	| rhs MINUS cat_set
	{
		$$ = sub_cats_from_syn($1, $3);
	}
	| prefix
	{
		if ($1 == PREFIX_NULL)
			$$ = SYN_NULL;
		else if ($1->partial_word) {
				fprintf(stderr,
		"Line %3d: partial word did not match properly: '%s'.\n",
				  line_number, $1->partial_word);
				free_prefix($1);
				syn_error++;
				$$ = SYN_NULL;
		} else {
			$$ = $1->synonym;
			$1->synonym = SYN_NULL;
			free_prefix($1);
		}
	}
	;

prefix	: prefix WHITE WORD
	{
		$$ = add_to_prefix($1, $3);
	}
	| prefix WORD
	{
	/*
	 * This rule handles words following quoted words.
	 */

		$$ = add_to_prefix($1, $2);
	}
	| WORD
	{
		$$ = make_prefix($1);
	}
	| quoted_word
	{
		$$ = make_prefix($1);
	}
	| prefix quoted_word
	{
		if ($1->partial_word) {
			fprintf(stderr,
		"Line %3d: unrecognizable word '%s' before quoted item.\n",
			  line_number, $1->partial_word);
			free_prefix($1);
			free($2);
			syn_error++;
			$$ = PREFIX_NULL;
		}
		else
			$$ = add_to_prefix($1, $2);
	}
	;

cat_set : cat_literal
	| cat_synonym
	;

cat_synonym : words
	{
		$$ = make_cat_syn($1);
	}
	;

cat_literal : SLASH cat_token_list SLASH
	{
		$$ = $2;
	}
	| SLASH SLASH
	{
		$$ = category_syn((char *) 0);
	}
	| SLASH cat_token_list error
	{
		(void) fprintf(stderr,
		  "Line %3d: Missing end slash\n", line_number);
		yyerrok;
		syn_error++;
		if ($2 != SYN_NULL)
			free_synonym($2);
		$$ = SYN_NULL;
	}
	;

cat_token_list : words
	{
		$$ = category_syn($1);
	}
	| cat_token_list COMMA words
	{
		$$ = add_category($1, $3);
	}
	;

words	: words WHITE WORD
	{
		$$ = new_word($1, $3);
	}
	| WORD
	| quoted_word
	| QUOTE words error
	{
		(void) fprintf(stderr,
		  "Line %3d: missing end quote\n", line_number);
		yyerrok;
		if ($2)
			free($2);
		$$ = NULL;
		syn_error++;
	}
	;
quoted_word : QUOTE words QUOTE
	{
		$$ = $2;
	}
	;

%%	/* beginning of subroutines section */

static void
rm_tmpfile(sig)
int	sig;
{
	if (temp_file != (char *) 0)
		(void) unlink(temp_file);
	if (temp_db != (char *) 0)
		(void) unlink(temp_db);
	exit(sig);
}

extern int mand_print_all_syns();
extern int mand_delete_syn();

extern	char	*optarg;
extern	int	optind;

char *dbfile = NULL;

main(argc, argv)
int	argc;
char	*argv[];
{
	int	c, mode = PRINT;
	extern	int	syn_error;
	mand_ir_t *mand_ir;

	(void) signal(SIGINT, rm_tmpfile);
	(void) signal(SIGQUIT, rm_tmpfile);
	(void) signal(SIGHUP, rm_tmpfile);
	(void) signal(SIGTERM, rm_tmpfile);

	prog = strrchr(argv[0], '/');
	if (prog != (char *) 0)
		prog++;
	else	prog = argv[0];

	while ((c = getopt(argc, argv, "d:rau")) != -1)
		switch (c) {
		case	'd':
			dbfile = optarg;
			break;

		case	'a':
			if(mode != PRINT)
				usage();
			mode = ADD;
			break;

		case	'r':
			if(mode != PRINT)
				usage();
			mode = REPLACE;
			break;

		case	'u':
			if(mode != PRINT)
				usage();
			mode = UNDEFINE;
			break;

		default:
			usage();
			break;
		}

	/*
	 * Set up the database pointer-use the specified database
	 * name or the default database.
	 */

	if(dbfile == NULL)
		dbfile = MAND_SYN_DB;

	/*
	 * Check that mandatory access control is initialized, and that
	 * the process is running at system low
	 */

	if (mand_init() != 0) {
		fprintf(stderr,
	"%s: unable to initialize mandatory access control policy.\n",
		  argv[0]);
		exit(1);
	}

	if (mode != PRINT &&
	    ((mand_ir = mand_alloc_ir()) == (mand_ir_t *) 0 ||
	    getslabel(mand_ir) < 0 ||
	    memcmp(mand_ir, mand_syslo, mand_bytes()) != 0))  {
		fprintf(stderr,
	"%s: must be run from a System Low process.\n",
		  argv[0]);
		exit(1);
	}
	mand_free_ir(mand_ir);

	/* Execute the requested function */

	switch(mode) {

	case ADD:
		add_synonyms(argc,argv);
		break;

	case REPLACE:
		replace_synonyms(argc,argv);
		break;

	case UNDEFINE:

		if(optind >= argc)
			usage();

	/* Open the specified or default database for synonyms */

		syn_fp = fopen(dbfile, "r+");
		if (syn_fp == (FILE *) 0) {
			(void) fprintf(stderr,
			  "%s: cannot open old synonyms database.\n", prog);
			exit(1);
		}

		if (mand_make_synonyms(syn_fp) <= 0) {
			fprintf(stderr,"mandsyn: can not locate synonym\n");
			exit(1);
		}

		for( ; optind < argc; ++optind)
			if(mand_delete_syn(dbfile,argv[optind]))
				exit(1);

		break;

	case PRINT:

	/* Open the specified or default database for synonyms */

		syn_fp = fopen(dbfile, "r+");
		if (syn_fp == (FILE *) 0) {
			(void) fprintf(stderr,
			  "%s: cannot open old synonyms database.\n", prog);
			exit(1);
		}

		if (mand_make_synonyms(syn_fp) < 0) {
			(void) fprintf(stderr,
	"mandsyn: synonym database file %s is not formatted properly.\n",
			  dbfile);
			exit(1);
		}

		if(optind >= argc)
			mand_print_all_syns();
		else
			for( ; optind < argc; ++optind)
				print_synonym(argv[optind]);
	}

	exit(0);
}
	
/* add synonyms to the synonyms database */
static void
add_synonyms(argc,argv)
int argc;
char *argv[];
{

	/* first, parse the old rules in preparation for the new ones */

	syn_fp = fopen(dbfile, "r+");
	if (syn_fp == (FILE *) 0) {
		fprintf(stderr,"mandsyn: cannot open old synonyms database.\n");
		exit(1);
	}
	if (mand_make_synonyms(syn_fp) < 0) {
		fprintf(stderr,
			"mandsyn: synonym database improperly formatted.\n");
		exit(1);
	}

	line_number = 1;

	/* parse the new rule(s) into a synonyms list */

	if (optind >= argc) {	/* no files, use stdin */
		syn_fp = stdin;
		yyparse();
	}
	else
		for( ; optind < argc; ++optind) {
			syn_fp = fopen(argv[optind],"r");
			if(syn_fp == NULL) {
				perror(argv[optind]);
				continue;
			}
			yyparse();
		}

	if (syn_error > 0) {
		fprintf(stderr,"mandsyn: error in parsing of new synonyms\n");
		exit(1);
	}

	/* write the synonyms database to a new file */

	if(mand_store_syns(dbfile,mand_syns))
		exit(1);
	else
		exit(0);
}

/* replace synonym output file */

static void
replace_synonyms(argc,argv)
int argc;
char *argv[];
{

	/* read the input files to build the new database */

	if (optind >= argc) {	/* no files, use default */
		syn_fp = fopen(MAND_SYN_FILE,"r");
		if(syn_fp == NULL) {
			perror(argv[optind]);
			exit(1);
		}
		yyparse();
	}
	else
		for( ; optind < argc; ++optind) {
			syn_fp = fopen(argv[optind],"r");
			if(syn_fp == NULL) {
				perror(argv[optind]);
				continue;
			}
			yyparse();
		}

	if (mand_syns == SYN_NULL) {
		fprintf(stderr,
		    "mandsyn: no synonyms defined, data base not replaced.\n");
		exit(1);
	}

	if (syn_error > 0) {
		fprintf(stderr,
			"mandsyn: error in parsing synonym definitions\n");
		exit(1);
	}

	/* write the synonyms database to the specified file */

	if(mand_store_syns(dbfile,mand_syns))
		exit(1);
	else
		exit(0);
}

/*
 * print a synonym, either from a local database file or from the global
 * database.
 */

static void
print_synonym(synonym)
char *synonym;
{
	mand_lookup_and_print_syn(synonym);
	return;
}

static void
usage()
{
	fprintf(stderr, "Usage:\n");
	fprintf(stderr,"  mandsyn [ -d database ] [ synonym ] ...\n");
	fprintf(stderr,"  mandsyn [ -d database ] -a [ file ] ...\n");
	fprintf(stderr,"  mandsyn [ -d database ] -r [ file ] ...\n");
	fprintf(stderr,"  mandsyn [ -d database ] -u synonym ...\n");
	exit(1);
}

/* error printing routine.
 * All error messages are printed in the rules themselves.
 */

yyerror(s)
char	*s;
{
}

/* Lexical analysis routine:
 *
 *  Return the following tokens:
 *
 *	Token		yylval
 *      --------	------
 *	EQUAL		nothing
 *	SLASH		nothing
 *	COMMA		nothing
 *	WHITE		nothing
 *	WORD		char string pointing to word
 *	NEWLINE		nothing
 *	PLUS		nothing
 *	MINUS		nothing
 *	QUOTE		nothing
 *
 *  WHITE is only significant if it's between two words
 *  Main routine sets FILE pointer for this routine.
 */

static char	*tokens = "=/,+-\"";
static char	*white  = " \t";
static char	line[100];

int
yylex()
{
	static	int	last_token = NEWLINE;
	static	int	this_token;
	static	char	*cp;
	char	*begin_token, *end_token;
	char	*retcp;
	int	size;
	char	c;
	extern	FILE	*syn_fp;

	/* initially, or if at beginning of line, read in a new line */
	if (last_token == NEWLINE) {
		if (fgets(line, sizeof (line), syn_fp) == NULL)
			return 0;
		/* skip white space at beginning of line */
		cp = &line[strspn(line, white)];
		if (*cp == '\0' || line[0] == '#')
			return NEWLINE;
	}
	switch (*cp) {
	case	'+':
		this_token = PLUS;
		break;
	case	'-':
		this_token = MINUS;
		break;
	case	'/':
		this_token = SLASH;
		break;
	case	',':
		this_token = COMMA;
		break;
	case	'=':
		this_token = EQUAL;
		break;
	case	'\n':
		this_token = NEWLINE;
		break;
	case	'"':
		this_token = QUOTE;
		break;
	default:
		begin_token = cp;
		this_token = WORD;
		break;
	}
	/* if last token was a word and we're looking at another word,
	 * return whitespace so the yacc code can fabricate a "long word"
	 */
	if (last_token == WORD && this_token == WORD) {
		last_token = WHITE;
		return WHITE;
	}
	/* In all simple token cases, skip over trailing whitespace */
	if (this_token != WORD) {
		if (*cp != '\n')
			cp++;
		if (*cp != '\n')
			cp = &cp[strspn(cp, white)];
		last_token = this_token;
		return last_token;
	}
	/* At this point, this_token is a WORD.
	 * Skip forward to the next token, ignoring whitespace at the end.
	 */
	for ( ;; cp++ ) {
		c = *cp;
		/* return word and advance past white space */
		if (strchr(white, c)) {
			end_token = cp;
			cp = &cp[strspn(cp, white)];
			break;
		}
		/* return word and leave cp where it is */
		if (c == '\n' || strchr(tokens, c)) {
			end_token = cp;
			break;
		}
	}
	size = end_token - begin_token + 1;
	retcp = malloc(size);
	if (retcp == (char *) 0) {
		if (mem_error++ == 0)
			(void) fprintf(stderr,
			  "Line %3d: memory allocation error\n", line_number);
		syn_error++;
	} else {
		strncpy(retcp, begin_token, size - 1);
		retcp[size - 1] = '\0';
	}
	yylval.wordp = retcp;
	last_token = WORD;
	return WORD;
}

/* routine to create a category synonym.
 * Allocates a synonym structure, sets the type, and returns the structure.
 * The argument is the word (possibly NULL)
 * which is to be made into the synonym.
 */

static struct synonym	*
category_syn(word)
char	*word;		/* which one to use */
{
	register struct synonym	*retsyn;
	int	cat_number;

	retsyn = alloc_synonym(CATEGORY_SYN);
	if (retsyn == SYN_NULL) {
		if (mem_error++ == 0)
			(void) fprintf(stderr,
			  "Line %3d: memory allocation error\n",
			  line_number);
		if (word != (char *) 0)
			free(word);
		syn_error++;
		return(retsyn);
	}
	if (word != (char *) 0) {
		cat_number = mand_nametocat(word);
		if (cat_number == -1) {
			(void) fprintf(stderr,
			  "Line %3d: category \'%s\' not found\n",
			  line_number, word);
			syn_error++;
			free(word);
			free_synonym(retsyn);
			return SYN_NULL;
		}
	}
	retsyn->name = (char *) 0;
	retsyn->class = 0;
	if (word != (char *) 0) {
		ADDBIT(retsyn->cat_set, cat_number);
		free(word);
	}
	return retsyn;
}

/* add a category name to an existing synonym.
 * If the category name is invalid, return ERROR, otherwise return the
 * synonym.
 */
static struct synonym	*
add_category(syn, word)
struct synonym	*syn;
char		*word;
{
	int	cat_number;

	if (word == (char *) 0) {
		if (syn != SYN_NULL)
			free_synonym(syn);
		return SYN_NULL;
	}
	if (syn == SYN_NULL) {
		free(word);
		return SYN_NULL;
	}
	cat_number = mand_nametocat(word);
	if (cat_number == MAND_INVALID_CAT) {
		(void) fprintf(stderr,
		  "Line %3d: category \'%s\' not valid.\n", line_number, word);
		syn_error++;
		free_synonym(syn);
		free(word);
		return SYN_NULL;
	}
	ADDBIT(syn->cat_set, cat_number);
	free(word);
	return syn;
}

/*
 * Lookup and make a category set synonym from the passed words.
 */

struct synonym *
make_cat_syn(word)
char *word;
{
	struct synonym *retsyn;
	struct synonym *syn;

	/* make sure that word is an existing category set synonym */

	syn = lookup_synonym_by_name(word);
	if (syn == SYN_NULL) {
		fprintf(stderr,
	"Line %3d: failed expecting category set synonym for '%s'.\n",
		  line_number, word);
		free(word);
		syn_error++;
		return SYN_NULL;
	}

	/* Allocate a new synonym based on the old one */

	return alias_synonym(word);
}

/* allocate a zeroed set of categories */

static mask_t *
alloc_categories()
{
	register mask_t	*ret;
	register int	i;

	ret = (mask_t *) calloc(CATWORDS, sizeof (*ret));
	return ret;
}

/* copy a category set from one to another */

static void
copy_categories(dest, source)
mask_t	*dest;
mask_t	*source;
{
	(void) memcpy((char *) dest, (char *) source,
	  CATWORDS * sizeof (mask_t));
}

/* allocate a new synonym structure */

static struct synonym *
alloc_synonym(type)
int	type;
{
	struct	synonym *syn;

	syn = (struct synonym *) calloc(sizeof(*syn), 1);
	if (syn != SYN_NULL) {
		syn->type = type;
		switch (type) {
		case	SENS_LABEL_SYN:
		case	CATEGORY_SYN:
			syn->cat_set = alloc_categories();
			if (syn->cat_set == (mask_t *) 0) {
				free((char *) syn);
				syn = SYN_NULL;
			}
			break;
		case	CLASS_SYN:
			syn->cat_set = (mask_t *) 0;
			break;
		}
	}
	syn->name = (char *) 0;
	return syn;
}

/* free a synonym structure */

static void
free_synonym(syn)
struct synonym *syn;
{
	switch (syn->type) {
	case	SENS_LABEL_SYN:
	case	CATEGORY_SYN:
		free((char *) syn->cat_set);
	case	CLASS_SYN:
		free((char *) syn);
	}
	return;
}

/* add a word to a synonym.
 * assumes that the word was allocated by the lexical analyzer.
 * Check that the word is not an existing classification, category, or synonym.
 */

static struct synonym *
add_word(word, syn)
char	*word;
struct	synonym *syn;
{
	int	class_number;
	int	cat_number;
	struct synonym *oldsyn;

	if (word == (char *) 0 || syn == SYN_NULL)
		goto out;
		
	/* check for an existing classification */
	class_number = mand_nametocl(word);
	if (class_number != MAND_INVALID_CLASS) {
		(void) fprintf(stderr,
		  "Line %3d: \'%s\' is an existing classification.\n",
		  line_number, word);
		syn_error++;
		goto out;
	}

	/* check for an existing category name */
	cat_number = mand_nametocat(word);
	if (cat_number != MAND_INVALID_CAT) {
		(void) fprintf(stderr,
		  "Line %3d \'%s\' is an existing category.\n",
		  line_number, word);
		syn_error++;
		goto out;
	}

	/* check for an existing synonym */
	oldsyn = lookup_synonym_by_name(word);
	if (oldsyn != SYN_NULL) {
		(void) fprintf(stderr,
		  "Line %3d: \'%s\' is an existing synonym.\n",
		  line_number, word);
		syn_error++;
		goto out;
	}

	syn->name = word;
	return syn;
out:
	if (syn != SYN_NULL)
		free_synonym(syn);
	if (word != (char *) 0)
		free(word);
	return SYN_NULL;
}

/* create a new word from two existing words.
 * frees up the storage occupied by the two words
 */

static char *
new_word(word1, word2)
char	*word1;
char	*word2;
{
	char	*retword;

	retword = malloc(strlen(word1) + strlen(word2) + 2);
	if (retword == (char *) 0) {
		if (mem_error++ == 0)
			(void) fprintf(stderr,
			  "Line %3d: memory allocation error.\n", line_number);
		syn_error++;
	}
	else
		(void) sprintf(retword, "%s %s", word1, word2);
	free(word1);
	free(word2);
	return retword;
}

/* insert a synonym into the in-core list, sorted lexically */

static struct synonym *
insert_into_list(syn)
struct	synonym	*syn;
{
	register struct	synonym	*this, *prev;

	/* error case */
	if (syn == SYN_NULL)
		return mand_syns;


	/* list empty case */
	if (mand_syns == SYN_NULL) {
		mand_syns = syn;
		syn->next = SYN_NULL;
		return mand_syns;
	}
	/* walk the list, looking for the first item that is lexically
	 * greater than the inserted item. Insert before that item.
	 * Follow behind with prev.
	 */
	for (this = mand_syns, prev = SYN_NULL;
	    this != SYN_NULL;
	    prev = this, this = this->next) {

		if (strcmp(this->name, syn->name) == 0) {
			fprintf(stderr,"mandsyn: %s duplicate synonym\n",
				syn->name);
			syn_error++;
			return(SYN_NULL);
		}

		if (strcmp(this->name, syn->name) > 0)  {
			/* insert at front of list */
			if (this == mand_syns)  {
				syn->next = mand_syns;
				mand_syns = syn;
			}
			/* insert in middle of list */
			else  {
				prev->next = syn;
				syn->next = this;
			}
			return mand_syns;
		}
	}
	/* insert at end of list */
	prev->next = syn;
	syn->next = SYN_NULL;
	return mand_syns;
}

/* lookup function for synonyms.
 * returns NULL if not there, otherwise synonym pointer.
 */

static struct synonym *
lookup_synonym_by_name(name)
char	*name;
{
	struct	synonym	*this;
	int	ret;

	for (this = mand_syns;
	     this != SYN_NULL;
	     this = this->next) {
		ret = strcmp(name, this->name);
		/* match */
		if (ret == 0)
			return this;
		/* since sorted, the first one that compares greater means
		 * not found
		 */
		if (ret < 0)
			break;
	}
	return SYN_NULL;
}

/*  make an alias for another synonym or a classification name */

static struct	synonym *
alias_synonym(word)
char	*word;
{
	register struct	synonym *retsyn;
	register struct synonym *oldsyn;

	if (word == (char *) 0)
		return SYN_NULL;
	else {
		/* alias synonym case */
		oldsyn = lookup_synonym_by_name(word);
		if (oldsyn == SYN_NULL) {
#ifdef DEBUG
print_list();
#endif
			(void) fprintf(stderr,
"Line %3d: synonym alias must be for an existing synonym: \'%s\'.\n",
			  line_number, word);
			syn_error++;
			free(word);
			return SYN_NULL;
		}
		retsyn = alloc_synonym((int) oldsyn->type);
		if (retsyn == SYN_NULL) {
			if (mem_error++ == 0)
				(void) fprintf(stderr,
				  "Line %3d: memory allocation error.\n",
				  line_number);
			syn_error++;
			free(word);
			return(SYN_NULL);
		}
		if (oldsyn->cat_set != (mask_t *) 0)
			copy_categories(retsyn->cat_set, oldsyn->cat_set);
		retsyn->class = oldsyn->class;
	}
	free(word);
	return retsyn;
}

/*
 * Add a category set to an existing synonym.
 * If it's a classification synonym, make it a sensitivity label synonym.
 */

struct synonym *
add_cats_to_syn(syn, syn_to_add)
struct synonym *syn;
struct synonym *syn_to_add;
{
	int i;

	/*
	 * Check that both are defined and syn_to_add is category set
	 */

	if (syn == SYN_NULL) {
		if (syn_to_add != SYN_NULL)
			free_synonym(syn_to_add);
		return SYN_NULL;
	}
	if (syn_to_add == SYN_NULL) {
		free_synonym(syn);
		return SYN_NULL;
	}

	if (syn_to_add->type != CATEGORY_SYN) {
		fprintf(stderr,
		  "Line %3d: only category synonyms may be added.\n",
		  line_number);
		free_synonym(syn);
		free_synonym(syn_to_add);
		syn_error++;
		return SYN_NULL;
	}

	/*
	 * If a classification synonym, move categories from category set
	 * synonym to new synonym.  Remove old synonym.
	 */
	if (syn->type == CLASS_SYN) {
		syn->cat_set = syn_to_add->cat_set;
		syn_to_add->cat_set = (mask_t *) 0;
		free_synonym(syn_to_add);
		syn->type = SENS_LABEL_SYN;
		return(syn);
	}

	/*
	 * Add categories from second synonym to first one
	 */

	for (i = 0; i < CATWORDS; i++)
		syn->cat_set[i] |= syn_to_add->cat_set[i];

	free_synonym(syn_to_add);
	return syn;
}

/*
 * Subtract categories from a synonym.
 * Same checks apply as in routine above.  A warning is issued if
 * some categories in the subtracted set do not appear in the
 * set to be modified.  It is an error to subtract categories
 * from a classification or classification synonym.
 */

struct synonym *
sub_cats_from_syn(syn, syn_to_sub)
struct synonym *syn;
struct synonym *syn_to_sub;
{
	int i;
	char warned = 0;

	/*
	 * Check that both are defined and syn_to_sub is category set
	 */

	if (syn == SYN_NULL) {
		if (syn_to_sub != SYN_NULL)
			free_synonym(syn_to_sub);
		return SYN_NULL;
	}
	if (syn_to_sub == SYN_NULL) {
		free_synonym(syn);
		return SYN_NULL;
	}

	if (syn_to_sub->type != CATEGORY_SYN) {
		fprintf(stderr,
		  "Line %3d: only category sets may be subtracted.\n",
		  line_number);
		free_synonym(syn);
		free_synonym(syn_to_sub);
		syn_error++;
		return SYN_NULL;
	}

	/*
	 * If set is not a sens label synonym, cannot subtract
	 */

	if (syn->type != SENS_LABEL_SYN && syn->type != CATEGORY_SYN) {
		fprintf(stderr,
		  "Line %3d: Cannot subtract categories from classification.\n",
		  line_number);
		free_synonym(syn);
		free_synonym(syn_to_sub);
		syn_error++;
		return SYN_NULL;
	}

	/*
	 * subtract categories, warning if some were not implied in syn.
	 */

	for (i = 0; i < CATWORDS; i++) {
		if ((syn->cat_set[i] & syn_to_sub->cat_set[i]) !=
		     syn_to_sub->cat_set[i] && !warned) {
			warned++;
			fprintf(stderr,
	"Line %3d WARNING: categories subtracted were not in original label.\n",
			  line_number);
		}
		syn->cat_set[i] &= ~syn_to_sub->cat_set[i];
	}

	free_synonym(syn_to_sub);
	return(syn);
}

/*
 * Following are the prefix manipulation macros. A prefix is a placeholder
 * for an optional synonym and a partially matched word.
 */

/* make_prefix: Start a prefix given an initial word. */

struct prefix *
make_prefix(word)
char *word;
{
	int class_number;
	int cat_number;
	struct synonym *syn;
	struct synonym *newsyn;
	struct prefix *prefix;

	/* Check for a classification match and if successful, allocate
	 * a classification synonym.
	 */

	class_number = mand_nametocl(word);
	if (class_number != MAND_INVALID_CLASS) {
		prefix = allocate_prefix();
		if (prefix == PREFIX_NULL) {
			if (mem_error++ == 0)
				fprintf(stderr, 
				  "Line %3d: memory allocation error\n",
				  line_number);
			free(word);
			syn_error++;
			return PREFIX_NULL;
		}
		prefix->synonym = alloc_synonym(CLASS_SYN);
		if (prefix->synonym == SYN_NULL) {
			if (mem_error++ == 0)
				fprintf(stderr, 
				  "Line %3d: memory allocation error\n",
				  line_number);
			free(word);
			free(prefix);
			syn_error++;
			return PREFIX_NULL;
		}
		prefix->synonym->class = class_number;
		free(word);
		return prefix;
	}

	/*
	 * Classification didn't match.  Now try a category match.
	 * A literal category name is not allowed as a prefix.
	 */

	cat_number = mand_nametocat(word);
	if (cat_number != MAND_INVALID_CAT) {
		fprintf(stderr,
	"Line %3d: Category literal name '%s' not allowed in rule prefix.\n",
		  line_number, word);
		free(word);
		syn_error++;
		return PREFIX_NULL;
	}

	/*
	 * Check for an existing synonym and duplicate it if found
	 */

	syn = lookup_synonym_by_name(word);
	if (syn != SYN_NULL) {
		newsyn = alias_synonym(word);
		free(word);
		if (newsyn == SYN_NULL)
			return PREFIX_NULL;
		prefix = allocate_prefix();
		if (prefix == PREFIX_NULL) {
			if (mem_error++ == 0)
				fprintf(stderr,
				  "Line %3d: memory allocation error.\n",
				  line_number);
			free_synonym(newsyn);
			syn_error++;
			return PREFIX_NULL;
		}
		prefix->synonym = newsyn;
		return prefix;
	}

	/*
	 * Prefix word is not classification, category, or synonym.
	 * allocate new prefix word.
	 */

	prefix = allocate_prefix();
	if (prefix == PREFIX_NULL) {
		if (mem_error++ == 0)
			fprintf(stderr,
			  "Line %3d: memory allocation error.\n",
			  line_number);
		syn_error++;
		return PREFIX_NULL;
	}
	prefix->partial_word = word;
	return prefix;
}

/*
 * add_cat_to_prefix: Add a category set definition to an existing prefix.
 */

static struct synonym *
add_cat_to_prefix(prefix, catsyn)
struct prefix *prefix;
struct synonym *catsyn;
{
	struct synonym *retsyn;

	/* check error cases */

	if (prefix == PREFIX_NULL) {
		if (catsyn != SYN_NULL)
			free_synonym(catsyn);
		return SYN_NULL;
	}

	if (catsyn == SYN_NULL) {
		free_prefix(prefix);
		return SYN_NULL;
	}

	/*
	 * If the prefix has a partial word, error due to unfinished business
	 */

	if (prefix->partial_word) {
		fprintf(stderr,
		  "Line %3d: Unmatched word(s) '%s' in synonym file.\n",
		  line_number, prefix->partial_word);
		free_prefix(prefix);
		free_synonym(catsyn);
		syn_error++;
		return SYN_NULL;
	}

	/*
	 * If no existing synonym in prefix, must have an internal error
	 */

	if (prefix->synonym == SYN_NULL) {
		fprintf(stderr,
		  "Line %3d: No partial name or synonym, internal error.\n",
		  line_number);
		free_prefix(prefix);
		free_synonym(catsyn);
		syn_error++;
		return SYN_NULL;
	}

	/*
	 * If the existing synonym is not classification or classification
	 * synonym, then this prefix combination is invalid.
	 */

	retsyn = prefix->synonym;
	if (retsyn->type != CLASS_SYN) {
		fprintf(stderr,
"Line %3d: prefix must start with classification synonym if\n",
		  line_number);
		fprintf(stderr, "           followed by categories.\n");
		free_synonym(catsyn);
		free_prefix(prefix);
		syn_error++;
		return SYN_NULL;
	}

	/*
	 * Add categories implied by category synonym to existing synonym
	 */

	retsyn->cat_set = catsyn->cat_set;
	catsyn->cat_set = (mask_t *) 0;
	prefix->synonym = SYN_NULL;
	free_prefix(prefix);
	free_synonym(catsyn);
	retsyn->type = SENS_LABEL_SYN;
	return retsyn;
}

/*
 * Try to add a word to an existing prefix.  The word must complete a
 * classification, a classification synonym (no existing prefix),
 * a category set synonym (classification prefix only), or a sensitivity
 * label synonym (no existing prefix).
 */

struct prefix *
add_to_prefix(prefix, word)
struct prefix *prefix;
char *word;
{
	struct synonym *syn;
	char *newname;
	int class_number;
	int cat_number;

	/* check error cases */

	if (prefix == PREFIX_NULL) {
		if (word)
			free(word);
		return prefix;
	}

	/* combine the word into any existing partial word. */

	if (prefix->partial_word) {
		newname = new_word(prefix->partial_word, word);
		prefix->partial_word = NULL;
		word = NULL;
	} else
		newname = word;

	if (newname == NULL) {
		if (mem_error++ == 0)
			fprintf(stderr,
		  	  "Line %3d: memory allocation error.\n",
			  line_number);
		free_prefix(prefix);
		syn_error++;
		return PREFIX_NULL;
	}

	/*
	 * If the word completes a classification name, there must not
	 * be an existing prefix synonym.
	 */

	class_number = mand_nametocl(newname);
	if (class_number != MAND_INVALID_CLASS) {
		if (prefix->synonym != SYN_NULL) {
			fprintf(stderr,
	"Line %3d: classification '%s' must be at the beginning of a rule.\n",
			  line_number, newname);
			free_prefix(prefix);
			free(newname);
			syn_error++;
			return PREFIX_NULL;
		}
		prefix->synonym = alloc_synonym(CLASS_SYN);
		if (prefix->synonym == SYN_NULL) {
			if (mem_error++ == 0)
				fprintf(stderr,
				  "Line %3d: memory allocation error.\n",
				  line_number);
			free_prefix(prefix);
			free(newname);
			syn_error++;
			return PREFIX_NULL;
		}
		prefix->synonym->class = class_number;
		free(newname);
		return prefix;
	}

	/*
	 * Check if the word is a literal category name
	 * These may not appear in prefix definitons
	 */

	cat_number = mand_nametocat(newname);
	if (cat_number != MAND_INVALID_CAT) {
		fprintf(stderr,
		  "Line %3d: category name '%s' may not appear in a prefix.\n",
		  line_number, newname);
		free_prefix(prefix);
		free(newname);
		syn_error++;
		return PREFIX_NULL;
	}

	/*
	 * Check if word is an existing synonym definition.
	 * If there is an existing synonym definition, it must be for
	 * a classification and this must be for a category set.
	 * If there is no existing synonym, duplicate the synonym.
	 */

	syn = lookup_synonym_by_name(newname);
	if (syn != SYN_NULL) {
		/*
		 * already a prefix synonym -- previous one must be
		 * class. and current one must be category synonym.
		 */
		if (prefix->synonym != SYN_NULL) {
			if (prefix->synonym->type != CLASS_SYN ||
			    syn->type != CATEGORY_SYN) {
				fprintf(stderr,
"Line %3d: prefix must contain classification and/or category synonym.\n",
				  line_number);
				free_prefix(prefix);
				free(newname);
				syn_error++;
				return PREFIX_NULL;
			}
			prefix->synonym->cat_set = alloc_categories();
			if (prefix->synonym->cat_set == (mask_t *) 0) {
				if (mem_error++)
					fprintf(stderr,
"Line %3d: memory allocation error.\n", line_number);
				free(prefix);
				free(newname);
				syn_error++;
				return PREFIX_NULL;
			}
			copy_categories(prefix->synonym->cat_set, syn->cat_set);
			prefix->synonym->type = SENS_LABEL_SYN;
			free(newname);
			return prefix;
		}

		/*
		 * no prefix synonym -- allocate a new one
		 */

		else {
			prefix->synonym = alias_synonym(newname);
			free(newname);
			if (prefix->synonym == SYN_NULL) {
				if (mem_error++)
					fprintf(stderr,
		"Line %3d: memory allocation error.\n", line_number);
				free(prefix);
				syn_error++;
				return PREFIX_NULL;
			}
			return prefix;
		}
	}

	/*
	 * No match on synonym, classification, or category. 
	 * Add to partial word.
	 */

	prefix->partial_word = newname;
	return prefix;
}

/*
 * allocate a prefix data structure
 */

static struct prefix *
allocate_prefix()
{
	struct prefix *pre;

	pre = (struct prefix *) calloc(sizeof(*pre), 1);
	return pre;
}

/* 
 * free a prefix data structure, including partial words and synonyms
 */

static void
free_prefix(pre)
struct prefix *pre;
{
	if (pre->partial_word)
		free(pre->partial_word);
	if (pre->synonym)
		free_synonym(pre->synonym);
	free(pre);
}

#ifdef DEBUG
print_list() {
	struct	synonym	*this;
	char	*t;
	char	buf[20];

	for (this = mand_syns; this != NULL; this = this->next) {
		switch (this->type) {
		case	CLASS_SYN:
			t = "classification";
			break;
		case	CATEGORY_SYN:
			t = "category";
			break;
		case	SENS_LABEL_SYN:
			t = "sensitivity label";
			break;
		default:
			(void) sprintf(buf, "%d", this->type);
			t = buf;
			break;
		}
		printf("synonym name %s type %s\n", this->name, t);
	}
}
#endif
