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
static char *rcsid = "@(#)$RCSfile: awktab.c,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/05/25 20:06:35 $";
#endif

# line 27 "awk.y"
#ifdef DEBUG
#define YYDEBUG 12
#endif

#define	YYMAXDEPTH	300
#define	YYSSIZE	YYMAXDEPTH

#include "awk.h"

static void yyerror (); /* va_alist */
static char *get_src_buf P((void));
static int yylex P((void));
static NODE *node_common P((NODETYPE op));
static NODE *snode P((NODE *subn, NODETYPE op, int sindex));
static NODE *mkrangenode P((NODE *cpair));
static NODE *make_for_loop P((NODE *init, NODE *cond, NODE *incr));
static NODE *append_right P((NODE *list, NODE *new));
static void func_install P((NODE *params, NODE *def));
static void pop_var P((NODE *np, int freeit));
static void pop_params P((NODE *params));
static NODE *make_param P((char *name));
static NODE *mk_rexp P((NODE *exp));

static int want_assign;		/* lexical scanning kludge */
static int want_regexp;		/* lexical scanning kludge */
static int can_return;		/* lexical scanning kludge */
static int io_allowed = 1;	/* lexical scanning kludge */
static char *lexptr;		/* pointer to next char during parsing */
static char *lexend;
static char *lexptr_begin;	/* keep track of where we were for error msgs */
static char *lexeme;		/* beginning of lexeme for debugging */
static char *thisline = NULL;
#define YYDEBUG_LEXER_TEXT (lexeme)
static int param_counter;
static char *tokstart = NULL;
static char *token = NULL;
static char *tokend;

NODE *variables[HASHSIZE];

extern char *source;
extern int sourceline;
extern char *cmdline_src;
extern char **srcfiles;
extern int errcount;
extern NODE *begin_block;
extern NODE *end_block;

# line 76 "awk.y"
typedef union  {
	long lval;
	AWKNUM fval;
	NODE *nodeval;
	NODETYPE nodetypeval;
	char *sval;
	NODE *(*ptrval)();
} YYSTYPE;
# define FUNC_CALL 257
# define NAME 258
# define REGEXP 259
# define ERROR 260
# define YNUMBER 261
# define YSTRING 262
# define RELOP 263
# define APPEND_OP 264
# define ASSIGNOP 265
# define MATCHOP 266
# define NEWLINE 267
# define CONCAT_OP 268
# define LEX_BEGIN 269
# define LEX_END 270
# define LEX_IF 271
# define LEX_ELSE 272
# define LEX_RETURN 273
# define LEX_DELETE 274
# define LEX_WHILE 275
# define LEX_DO 276
# define LEX_FOR 277
# define LEX_BREAK 278
# define LEX_CONTINUE 279
# define LEX_PRINT 280
# define LEX_PRINTF 281
# define LEX_NEXT 282
# define LEX_EXIT 283
# define LEX_FUNCTION 284
# define LEX_GETLINE 285
# define LEX_IN 286
# define LEX_AND 287
# define LEX_OR 288
# define INCREMENT 289
# define DECREMENT 290
# define LEX_BUILTIN 291
# define LEX_LENGTH 292
# define UNARY 293
#define yyclearin yychar = -1
#define yyerrok yyerrflag = 0
extern int yychar;
extern int yyerrflag;
#ifndef YYMAXDEPTH
#define YYMAXDEPTH 150
#endif
YYSTYPE yylval, yyval;
typedef int yytabelem;
# define YYERRCODE 256

# line 748 "awk.y"


struct token {
	char *operator;		/* text to match */
	NODETYPE value;		/* node type */
	int class;		/* lexical class */
	unsigned flags;		/* # of args. allowed and compatability */
#	define	ARGS	0xFF	/* 0, 1, 2, 3 args allowed (any combination */
#	define	A(n)	(1<<(n))
#	define	VERSION	0xFF00	/* old awk is zero */
#	define	NOT_OLD		0x0100	/* feature not in old awk */
#	define	NOT_POSIX	0x0200	/* feature not in POSIX */
#	define	GAWKX		0x0400	/* gawk extension */
	NODE *(*ptr) ();	/* function that implements this keyword */
};

extern NODE
	*do_exp(),	*do_getline(),	*do_index(),	*do_length(),
	*do_sqrt(),	*do_log(),	*do_sprintf(),	*do_substr(),
	*do_split(),	*do_system(),	*do_int(),	*do_close(),
	*do_atan2(),	*do_sin(),	*do_cos(),	*do_rand(),
	*do_srand(),	*do_match(),	*do_tolower(),	*do_toupper(),
	*do_sub(),	*do_gsub(),	*do_strftime(),	*do_systime();

/* Tokentab is sorted ascii ascending order, so it can be binary searched. */

static struct token tokentab[] = {
{"BEGIN",	Node_illegal,	 LEX_BEGIN,	0,		0},
{"END",		Node_illegal,	 LEX_END,	0,		0},
{"atan2",	Node_builtin,	 LEX_BUILTIN,	NOT_OLD|A(2),	do_atan2},
{"break",	Node_K_break,	 LEX_BREAK,	0,		0},
{"close",	Node_builtin,	 LEX_BUILTIN,	NOT_OLD|A(1),	do_close},
{"continue",	Node_K_continue, LEX_CONTINUE,	0,		0},
{"cos",		Node_builtin,	 LEX_BUILTIN,	NOT_OLD|A(1),	do_cos},
{"delete",	Node_K_delete,	 LEX_DELETE,	NOT_OLD,	0},
{"do",		Node_K_do,	 LEX_DO,	NOT_OLD,	0},
{"else",	Node_illegal,	 LEX_ELSE,	0,		0},
{"exit",	Node_K_exit,	 LEX_EXIT,	0,		0},
{"exp",		Node_builtin,	 LEX_BUILTIN,	A(1),		do_exp},
{"for",		Node_K_for,	 LEX_FOR,	0,		0},
{"func",	Node_K_function, LEX_FUNCTION,	NOT_POSIX|NOT_OLD,	0},
{"function",	Node_K_function, LEX_FUNCTION,	NOT_OLD,	0},
{"getline",	Node_K_getline,	 LEX_GETLINE,	NOT_OLD,	0},
{"gsub",	Node_builtin,	 LEX_BUILTIN,	NOT_OLD|A(2)|A(3), do_gsub},
{"if",		Node_K_if,	 LEX_IF,	0,		0},
{"in",		Node_illegal,	 LEX_IN,	0,		0},
{"index",	Node_builtin,	 LEX_BUILTIN,	A(2),		do_index},
{"int",		Node_builtin,	 LEX_BUILTIN,	A(1),		do_int},
{"length",	Node_builtin,	 LEX_LENGTH,	A(0)|A(1),	do_length},
{"log",		Node_builtin,	 LEX_BUILTIN,	A(1),		do_log},
{"match",	Node_builtin,	 LEX_BUILTIN,	NOT_OLD|A(2),	do_match},
{"next",	Node_K_next,	 LEX_NEXT,	0,		0},
{"print",	Node_K_print,	 LEX_PRINT,	0,		0},
{"printf",	Node_K_printf,	 LEX_PRINTF,	0,		0},
{"rand",	Node_builtin,	 LEX_BUILTIN,	NOT_OLD|A(0),	do_rand},
{"return",	Node_K_return,	 LEX_RETURN,	NOT_OLD,	0},
{"sin",		Node_builtin,	 LEX_BUILTIN,	NOT_OLD|A(1),	do_sin},
{"split",	Node_builtin,	 LEX_BUILTIN,	A(2)|A(3),	do_split},
{"sprintf",	Node_builtin,	 LEX_BUILTIN,	0,		do_sprintf},
{"sqrt",	Node_builtin,	 LEX_BUILTIN,	A(1),		do_sqrt},
{"srand",	Node_builtin,	 LEX_BUILTIN,	NOT_OLD|A(0)|A(1), do_srand},
{"strftime",	Node_builtin,	 LEX_BUILTIN,	GAWKX|A(1)|A(2), do_strftime},
{"sub",		Node_builtin,	 LEX_BUILTIN,	NOT_OLD|A(2)|A(3), do_sub},
{"substr",	Node_builtin,	 LEX_BUILTIN,	A(2)|A(3),	do_substr},
{"system",	Node_builtin,	 LEX_BUILTIN,	NOT_OLD|A(1),	do_system},
{"systime",	Node_builtin,	 LEX_BUILTIN,	GAWKX|A(0),	do_systime},
{"tolower",	Node_builtin,	 LEX_BUILTIN,	NOT_OLD|A(1),	do_tolower},
{"toupper",	Node_builtin,	 LEX_BUILTIN,	NOT_OLD|A(1),	do_toupper},
{"while",	Node_K_while,	 LEX_WHILE,	0,		0},
};

/* VARARGS0 */
static void
yyerror(va_alist)
va_dcl
{
	va_list args;
	char *mesg;
	register char *bp, *cp;
	char *scan;
	char buf[120];

	errcount++;
	/* Find the current line in the input file */
	if (lexptr) {
		if (!thisline) {
			for (cp=lexeme; cp != lexptr_begin && *cp != '\n'; --cp)
				;
			if (*cp == '\n')
				cp++;
			thisline = cp;
		}
		/* NL isn't guaranteed */
		bp = lexeme;
		while (bp < lexend && *bp && *bp != '\n')
			bp++;
	} else {
		thisline = "(END OF FILE)";
		bp = thisline + 13;
	}
	msg("%.*s", (int) (bp - thisline), thisline);
	bp = buf;
	cp = buf + sizeof(buf) - 24;	/* 24 more than longest msg. input */
	if (lexptr) {
		scan = thisline;
		while (bp < cp && scan < lexeme)
			if (*scan++ == '\t')
				*bp++ = '\t';
			else
				*bp++ = ' ';
		*bp++ = '^';
		*bp++ = ' ';
	}
	va_start(args);
	mesg = va_arg(args, char *);
	strcpy(bp, mesg);
	err("", buf, args);
	va_end(args);
	exit(2);
}

static char *
get_src_buf()
{
	static int samefile = 0;
	static int nextfile = 0;
	static char *buf = NULL;
	static int fd;
	int n;
	register char *scan;
	static int len = 0;
	static int did_newline = 0;
#	define	SLOP	128	/* enough space to hold most source lines */

	if (cmdline_src) {
		if (len == 0) {
			len = strlen(cmdline_src);
			if (len == 0)
				cmdline_src = NULL;
			sourceline = 1;
			lexptr = lexptr_begin = cmdline_src;
			lexend = lexptr + len;
		} else if (!did_newline && *(lexptr-1) != '\n') {
			/*
			 * The following goop is to ensure that the source
			 * ends with a newline and that the entire current
			 * line is available for error messages.
			 */
			int offset;

			did_newline = 1;
			offset = lexptr - lexeme;
			for (scan = lexeme; scan > lexptr_begin; scan--)
				if (*scan == '\n') {
					scan++;
					break;
				}
			len = lexptr - scan;
			emalloc(buf, char *, len+1, "get_src_buf");
			memcpy(buf, scan, len);
			thisline = buf;
			lexptr = buf + len;
			*lexptr = '\n';
			lexeme = lexptr - offset;
			lexptr_begin = buf;
			lexend = lexptr + 1;
		} else
			lexeme = lexptr = lexptr_begin = NULL;
		return lexptr;
	}
	if (!samefile) {
		source = srcfiles[nextfile];
		if (source == NULL) {
			if (buf) {
				free(buf);
				buf = NULL;
			}
			return lexeme = lexptr = lexptr_begin = NULL;
		}
		fd = pathopen(source);
		if (fd == -1)
			fatal("can't open source file \"%s\" for reading (%s)",
				source, strerror(errno));
		len = optimal_bufsize(fd);
		if (buf)
			free(buf);
		emalloc(buf, char *, len + SLOP, "get_src_buf");
		lexptr_begin = buf + SLOP;
		samefile = 1;
		sourceline = 1;
	} else {
		/*
		 * Here, we retain the current source line (up to length SLOP)
		 * in the beginning of the buffer that was overallocated above
		 */
		int offset;
		int linelen;

		offset = lexptr - lexeme;
		for (scan = lexeme; scan > lexptr_begin; scan--)
			if (*scan == '\n') {
				scan++;
				break;
			}
		linelen = lexptr - scan;
		if (linelen > SLOP)
			linelen = SLOP;
		thisline = buf + SLOP - linelen;
		memcpy(thisline, scan, linelen);
		lexeme = buf + SLOP - offset;
		lexptr_begin = thisline;
	}
	n = read(fd, buf + SLOP, len);
	if (n == -1)
		fatal("can't read sourcefile \"%s\" (%s)",
			source, strerror(errno));
	if (n == 0) {
		samefile = 0;
		nextfile++;
		return get_src_buf();
	}
	lexptr = buf + SLOP;
	lexend = lexptr + n;
	return buf;
}

#define	tokadd(x) (*token++ = (x), token == tokend ? tokexpand() : token)

char *
tokexpand()
{
	static int toksize = 60;
	int tokoffset;

	tokoffset = token - tokstart;
	toksize *= 2;
	if (tokstart)
		erealloc(tokstart, char *, toksize, "tokexpand");
	else
		emalloc(tokstart, char *, toksize, "tokexpand");
	tokend = tokstart + toksize;
	token = tokstart + tokoffset;
	return token;
}

#if DEBUG
char
nextc() {
	if (lexptr && lexptr < lexend)
		return *lexptr++;
	else if (get_src_buf())
		return *lexptr++;
	else
		return '\0';
}
#else
#define	nextc()	((lexptr && lexptr < lexend) ? \
			*lexptr++ : \
			(get_src_buf() ? *lexptr++ : '\0') \
		)
#endif
#define pushback() (lexptr && lexptr > lexptr_begin ? lexptr-- : lexptr)

/*
 * Read the input and turn it into tokens.
 */

static int
yylex()
{
	register int c;
	int seen_e = 0;		/* These are for numbers */
	int seen_point = 0;
	int esc_seen;		/* for literal strings */
	int low, mid, high;
	static int did_newline = 0;
	char *tokkey;

	if (!nextc())
		return 0;
	pushback();
	lexeme = lexptr;
	thisline = NULL;
	if (want_regexp) {
		int in_brack = 0;

		want_regexp = 0;
		token = tokstart;
		while ((c = nextc()) != 0) {
			switch (c) {
			case '[':
				in_brack = 1;
				break;
			case ']':
				in_brack = 0;
				break;
			case '\\':
				if ((c = nextc()) == '\0') {
					yyerror("unterminated regexp ends with \\ at end of file");
				} else if (c == '\n') {
					sourceline++;
					continue;
				} else
					tokadd('\\');
				break;
			case '/':	/* end of the regexp */
				if (in_brack)
					break;

				pushback();
				tokadd('\0');
				yylval.sval = tokstart;
				return REGEXP;
			case '\n':
				pushback();
				yyerror("unterminated regexp");
			case '\0':
				yyerror("unterminated regexp at end of file");
			}
			tokadd(c);
		}
	}
retry:
	while ((c = nextc()) == ' ' || c == '\t')
		;

	lexeme = lexptr ? lexptr - 1 : lexptr;
	thisline = NULL;
	token = tokstart;
	yylval.nodetypeval = Node_illegal;

	switch (c) {
	case 0:
		return 0;

	case '\n':
		sourceline++;
		return NEWLINE;

	case '#':		/* it's a comment */
		while ((c = nextc()) != '\n') {
			if (c == '\0')
				return 0;
		}
		sourceline++;
		return NEWLINE;

	case '\\':
#ifdef RELAXED_CONTINUATION
		if (!strict) {	/* strip trailing white-space and/or comment */
			while ((c = nextc()) == ' ' || c == '\t') continue;
			if (c == '#')
				while ((c = nextc()) != '\n') if (!c) break;
			pushback();
		}
#endif /*RELAXED_CONTINUATION*/
		if (nextc() == '\n') {
			sourceline++;
			goto retry;
		} else
			yyerror("inappropriate use of backslash");
		break;

	case '$':
		want_assign = 1;
		return '$';

	case ')':
	case ']':
	case '(':	
	case '[':
	case ';':
	case ':':
	case '?':
	case '{':
	case ',':
		return c;

	case '*':
		if ((c = nextc()) == '=') {
			yylval.nodetypeval = Node_assign_times;
			return ASSIGNOP;
		} else if (do_posix) {
			pushback();
			return '*';
		} else if (c == '*') {
			/* make ** and **= aliases for ^ and ^= */
			static int did_warn_op = 0, did_warn_assgn = 0;

			if (nextc() == '=') {
				if (do_lint && ! did_warn_assgn) {
					did_warn_assgn = 1;
					warning("**= is not allowed by POSIX");
				}
				yylval.nodetypeval = Node_assign_exp;
				return ASSIGNOP;
			} else {
				pushback();
				if (do_lint && ! did_warn_op) {
					did_warn_op = 1;
					warning("** is not allowed by POSIX");
				}
				return '^';
			}
		}
		pushback();
		return '*';

	case '/':
		if (want_assign) {
			if (nextc() == '=') {
				yylval.nodetypeval = Node_assign_quotient;
				return ASSIGNOP;
			}
			pushback();
		}
		return '/';

	case '%':
		if (nextc() == '=') {
			yylval.nodetypeval = Node_assign_mod;
			return ASSIGNOP;
		}
		pushback();
		return '%';

	case '^':
	{
		static int did_warn_op = 0, did_warn_assgn = 0;

		if (nextc() == '=') {

			if (do_lint && ! did_warn_assgn) {
				did_warn_assgn = 1;
				warning("operator `^=' is not supported in old awk");
			}
			yylval.nodetypeval = Node_assign_exp;
			return ASSIGNOP;
		}
		pushback();
		if (do_lint && ! did_warn_op) {
			did_warn_op = 1;
			warning("operator `^' is not supported in old awk");
		}
		return '^';
	}

	case '+':
		if ((c = nextc()) == '=') {
			yylval.nodetypeval = Node_assign_plus;
			return ASSIGNOP;
		}
		if (c == '+')
			return INCREMENT;
		pushback();
		return '+';

	case '!':
		if ((c = nextc()) == '=') {
			yylval.nodetypeval = Node_notequal;
			return RELOP;
		}
		if (c == '~') {
			yylval.nodetypeval = Node_nomatch;
			want_assign = 0;
			return MATCHOP;
		}
		pushback();
		return '!';

	case '<':
		if (nextc() == '=') {
			yylval.nodetypeval = Node_leq;
			return RELOP;
		}
		yylval.nodetypeval = Node_less;
		pushback();
		return '<';

	case '=':
		if (nextc() == '=') {
			yylval.nodetypeval = Node_equal;
			return RELOP;
		}
		yylval.nodetypeval = Node_assign;
		pushback();
		return ASSIGNOP;

	case '>':
		if ((c = nextc()) == '=') {
			yylval.nodetypeval = Node_geq;
			return RELOP;
		} else if (c == '>') {
			yylval.nodetypeval = Node_redirect_append;
			return APPEND_OP;
		}
		yylval.nodetypeval = Node_greater;
		pushback();
		return '>';

	case '~':
		yylval.nodetypeval = Node_match;
		want_assign = 0;
		return MATCHOP;

	case '}':
		/*
		 * Added did newline stuff.  Easier than
		 * hacking the grammar
		 */
		if (did_newline) {
			did_newline = 0;
			return c;
		}
		did_newline++;
		--lexptr;	/* pick up } next time */
		return NEWLINE;

	case '"':
		esc_seen = 0;
		while ((c = nextc()) != '"') {
			if (c == '\n') {
				pushback();
				yyerror("unterminated string");
			}
			if (c == '\\') {
				c = nextc();
				if (c == '\n') {
					sourceline++;
					continue;
				}
				esc_seen = 1;
				tokadd('\\');
			}
			if (c == '\0') {
				pushback();
				yyerror("unterminated string");
			}
			tokadd(c);
		}
		yylval.nodeval = make_str_node(tokstart,
					token - tokstart, esc_seen ? SCAN : 0);
		yylval.nodeval->flags |= PERM;
		return YSTRING;

	case '-':
		if ((c = nextc()) == '=') {
			yylval.nodetypeval = Node_assign_minus;
			return ASSIGNOP;
		}
		if (c == '-')
			return DECREMENT;
		pushback();
		return '-';

	case '.':
		c = nextc();
		pushback();
		if (!isdigit(c))
			return '.';
		else
			c = '.';	/* FALL THROUGH */
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		/* It's a number */
		for (;;) {
			int gotnumber = 0;

			tokadd(c);
			switch (c) {
			case '.':
				if (seen_point) {
					gotnumber++;
					break;
				}
				++seen_point;
				break;
			case 'e':
			case 'E':
				if (seen_e) {
					gotnumber++;
					break;
				}
				++seen_e;
				if ((c = nextc()) == '-' || c == '+')
					tokadd(c);
				else
					pushback();
				break;
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				break;
			default:
				gotnumber++;
			}
			if (gotnumber)
				break;
			c = nextc();
		}
		pushback();
		yylval.nodeval = make_number(atof(tokstart));
		yylval.nodeval->flags |= PERM;
		return YNUMBER;

	case '&':
		if ((c = nextc()) == '&') {
			yylval.nodetypeval = Node_and;
			for (;;) {
				c = nextc();
				if (c == '\0')
					break;
				if (c == '#') {
					while ((c = nextc()) != '\n' && c != '\0')
						;
					if (c == '\0')
						break;
				}
				if (c == '\n')
					sourceline++;
				if (! isspace(c)) {
					pushback();
					break;
				}
			}
			want_assign = 0;
			return LEX_AND;
		}
		pushback();
		return '&';

	case '|':
		if ((c = nextc()) == '|') {
			yylval.nodetypeval = Node_or;
			for (;;) {
				c = nextc();
				if (c == '\0')
					break;
				if (c == '#') {
					while ((c = nextc()) != '\n' && c != '\0')
						;
					if (c == '\0')
						break;
				}
				if (c == '\n')
					sourceline++;
				if (! isspace(c)) {
					pushback();
					break;
				}
			}
			want_assign = 0;
			return LEX_OR;
		}
		pushback();
		return '|';
	}

	if (c != '_' && ! isalpha(c))
		yyerror("Invalid char '%c' in expression\n", c);

	/* it's some type of name-type-thing.  Find its length */
	token = tokstart;
	while (is_identchar(c)) {
		tokadd(c);
		c = nextc();
	}
	tokadd('\0');
	emalloc(tokkey, char *, token - tokstart, "yylex");
	memcpy(tokkey, tokstart, token - tokstart);
	pushback();

	/* See if it is a special token.  */
	low = 0;
	high = (sizeof (tokentab) / sizeof (tokentab[0])) - 1;
	while (low <= high) {
		int i/* , c */;

		mid = (low + high) / 2;
		c = *tokstart - tokentab[mid].operator[0];
		i = c ? c : strcmp (tokstart, tokentab[mid].operator);

		if (i < 0) {		/* token < mid */
			high = mid - 1;
		} else if (i > 0) {	/* token > mid */
			low = mid + 1;
		} else {
			if (do_lint) {
				if (tokentab[mid].flags & GAWKX)
					warning("%s() is a gawk extension",
						tokentab[mid].operator);
				if (tokentab[mid].flags & NOT_POSIX)
					warning("POSIX does not allow %s",
						tokentab[mid].operator);
				if (tokentab[mid].flags & NOT_OLD)
					warning("%s is not supported in old awk",
						tokentab[mid].operator);
			}
			if ((strict && (tokentab[mid].flags & GAWKX))
			    || (do_posix && (tokentab[mid].flags & NOT_POSIX)))
				break;
			if (tokentab[mid].class == LEX_BUILTIN
			    || tokentab[mid].class == LEX_LENGTH
			   )
				yylval.lval = mid;
			else
				yylval.nodetypeval = tokentab[mid].value;

			return tokentab[mid].class;
		}
	}

	yylval.sval = tokkey;
	if (*lexptr == '(')
		return FUNC_CALL;
	else {
		want_assign = 1;
		return NAME;
	}
}

static NODE *
node_common(op)
NODETYPE op;
{
	register NODE *r;

	getnode(r);
	r->type = op;
	r->flags = MALLOC;
	/* if lookahead is NL, lineno is 1 too high */
	if (lexeme && *lexeme == '\n')
		r->source_line = sourceline - 1;
	else
		r->source_line = sourceline;
	r->source_file = source;
	return r;
}

/*
 * This allocates a node with defined lnode and rnode. 
 */
NODE *
node(left, op, right)
NODE *left, *right;
NODETYPE op;
{
	register NODE *r;

	r = node_common(op);
	r->lnode = left;
	r->rnode = right;
	return r;
}

/*
 * This allocates a node with defined subnode and proc for builtin functions
 * Checks for arg. count and supplies defaults where possible.
 */
static NODE *
snode(subn, op, idx)
NODETYPE op;
int idx;
NODE *subn;
{
	register NODE *r;
	register NODE *n;
	int nexp = 0;
	int args_allowed;

	r = node_common(op);

	/* traverse expression list to see how many args. given */
	for (n= subn; n; n= n->rnode) {
		nexp++;
		if (nexp > 3)
			break;
	}

	/* check against how many args. are allowed for this builtin */
	args_allowed = tokentab[idx].flags & ARGS;
	if (args_allowed && !(args_allowed & A(nexp)))
		fatal("%s() cannot have %d argument%c",
			tokentab[idx].operator, nexp, nexp == 1 ? ' ' : 's');

	r->proc = tokentab[idx].ptr;

	/* special case processing for a few builtins */
	if (nexp == 0 && r->proc == do_length) {
		subn = node(node(make_number(0.0),Node_field_spec,(NODE *)NULL),
		            Node_expression_list,
			    (NODE *) NULL);
	} else if (r->proc == do_match) {
		if (subn->rnode->lnode->type != Node_regex)
			subn->rnode->lnode = mk_rexp(subn->rnode->lnode);
	} else if (r->proc == do_sub || r->proc == do_gsub) {
		if (subn->lnode->type != Node_regex)
			subn->lnode = mk_rexp(subn->lnode);
		if (nexp == 2)
			append_right(subn, node(node(make_number(0.0),
						     Node_field_spec,
						     (NODE *) NULL),
					        Node_expression_list,
						(NODE *) NULL));
		else if (do_lint && subn->rnode->rnode->lnode->type == Node_val)
			warning("string literal as last arg of substitute");
	} else if (r->proc == do_split) {
		if (nexp == 2)
			append_right(subn,
			    node(FS_node, Node_expression_list, (NODE *) NULL));
		n = subn->rnode->rnode->lnode;
		if (n->type != Node_regex)
			subn->rnode->rnode->lnode = mk_rexp(n);
		if (nexp == 2)
			subn->rnode->rnode->lnode->re_flags |= FS_DFLT;
	}

	r->subnode = subn;
	return r;
}

/*
 * This allocates a Node_line_range node with defined condpair and
 * zeroes the trigger word to avoid the temptation of assuming that calling
 * 'node( foo, Node_line_range, 0)' will properly initialize 'triggered'. 
 */
/* Otherwise like node() */
static NODE *
mkrangenode(cpair)
NODE *cpair;
{
	register NODE *r;

	getnode(r);
	r->type = Node_line_range;
	r->condpair = cpair;
	r->triggered = 0;
	return r;
}

/* Build a for loop */
static NODE *
make_for_loop(init, cond, incr)
NODE *init, *cond, *incr;
{
	register FOR_LOOP_HEADER *r;
	NODE *n;

	emalloc(r, FOR_LOOP_HEADER *, sizeof(FOR_LOOP_HEADER), "make_for_loop");
	getnode(n);
	n->type = Node_illegal;
	r->init = init;
	r->cond = cond;
	r->incr = incr;
	n->sub.nodep.r.hd = r;
	return n;
}

/*
 * Install a name in the symbol table, even if it is already there.
 * Caller must check against redefinition if that is desired. 
 */
NODE *
install(name, value)
char *name;
NODE *value;
{
	register NODE *hp;
	register int len, bucket;

	len = strlen(name);
	bucket = hash(name, len);
	getnode(hp);
	hp->type = Node_hashnode;
	hp->hnext = variables[bucket];
	variables[bucket] = hp;
	hp->hlength = len;
	hp->hvalue = value;
	hp->hname = name;
	return hp->hvalue;
}

/* find the most recent hash node for name installed by install */
NODE *
lookup(name)
char *name;
{
	register NODE *bucket;
	register int len;

	len = strlen(name);
	bucket = variables[hash(name, len)];
	while (bucket) {
		if (bucket->hlength == len && STREQN(bucket->hname, name, len))
			return bucket->hvalue;
		bucket = bucket->hnext;
	}
	return NULL;
}

/*
 * Add new to the rightmost branch of LIST.  This uses n^2 time, so we make
 * a simple attempt at optimizing it.
 */
static NODE *
append_right(list, new)
NODE *list, *new;
{
	register NODE *oldlist;
	static NODE *savefront = NULL, *savetail = NULL;

	oldlist = list;
	if (savefront == oldlist) {
		savetail = savetail->rnode = new;
		return oldlist;
	} else
		savefront = oldlist;
	while (list->rnode != NULL)
		list = list->rnode;
	savetail = list->rnode = new;
	return oldlist;
}

/*
 * check if name is already installed;  if so, it had better have Null value,
 * in which case def is added as the value. Otherwise, install name with def
 * as value. 
 */
static void
func_install(params, def)
NODE *params;
NODE *def;
{
	NODE *r;

	pop_params(params->rnode);
	pop_var(params, 0);
	r = lookup(params->param);
	if (r != NULL) {
		fatal("function name `%s' previously defined", params->param);
	} else
		(void) install(params->param, node(params, Node_func, def));
}

static void
pop_var(np, freeit)
NODE *np;
int freeit;
{
	register NODE *bucket, **save;
	register int len;
	char *name;

	name = np->param;
	len = strlen(name);
	save = &(variables[hash(name, len)]);
	for (bucket = *save; bucket; bucket = bucket->hnext) {
		if (len == bucket->hlength && STREQN(bucket->hname, name, len)) {
			*save = bucket->hnext;
			freenode(bucket);
			if (freeit)
				free(np->param);
			return;
		}
		save = &(bucket->hnext);
	}
}

static void
pop_params(params)
NODE *params;
{
	register NODE *np;

	for (np = params; np != NULL; np = np->rnode)
		pop_var(np, 1);
}

static NODE *
make_param(name)
char *name;
{
	NODE *r;

	getnode(r);
	r->type = Node_param_list;
	r->rnode = NULL;
	r->param = name;
	r->param_cnt = param_counter++;
	return (install(name, r));
}

/* Name points to a variable name.  Make sure its in the symbol table */
NODE *
variable(name, can_free)
char *name;
int can_free;
{
	register NODE *r;
	static int env_loaded = 0;

	if (!env_loaded && STREQ(name, "ENVIRON")) {
		load_environ();
		env_loaded = 1;
	}
	if ((r = lookup(name)) == NULL)
		r = install(name, node(Nnull_string, Node_var, (NODE *) NULL));
	else if (can_free)
		free(name);
	return r;
}

static NODE *
mk_rexp(exp)
NODE *exp;
{
	if (exp->type == Node_regex)
		return exp;
	else {
		NODE *n;

		getnode(n);
		n->type = Node_regex;
		n->re_exp = exp;
		n->re_text = NULL;
		n->re_reg = NULL;
		n->re_flags = 0;
		n->re_cnt = 1;
		return n;
	}
}
yytabelem yyexca[] ={
-1, 1,
	0, -1,
	-2, 0,
-1, 125,
	266, 0,
	-2, 101,
-1, 127,
	263, 0,
	60, 0,
	62, 0,
	124, 0,
	-2, 105,
-1, 128,
	263, 0,
	60, 0,
	62, 0,
	124, 0,
	-2, 106,
-1, 129,
	263, 0,
	60, 0,
	62, 0,
	124, 0,
	-2, 107,
-1, 252,
	266, 0,
	-2, 118,
-1, 254,
	263, 0,
	-2, 120,
	};
# define YYNPROD 158
# define YYLAST 1934
yytabelem yyact[]={

    16,   106,   215,   166,   165,    89,    20,    90,    91,    24,
   228,   122,    44,    17,    63,   295,    35,   239,    34,    96,
    25,    63,     4,    36,    44,   108,    90,    91,   157,    90,
    91,   113,   114,   171,   203,    63,    63,   168,   169,   290,
   264,   289,   263,   268,    24,   186,   265,    51,    62,   253,
    68,   161,   174,   126,    22,    62,   187,    62,    95,    62,
    62,    99,   176,    66,    62,    63,   202,   136,   188,   221,
   106,   231,   177,    44,   174,   267,    22,   223,   163,    67,
   152,   144,   102,   142,   112,    63,    92,   100,   103,   111,
   104,   110,   101,     6,    63,    26,   205,   274,   256,    38,
    63,    63,    63,    63,    63,    63,   230,   170,   102,   160,
   159,   109,    88,   100,   120,    45,    40,   134,   101,   167,
   107,   164,    63,    96,    63,    63,    63,    79,    63,    63,
    63,    63,   261,    63,    73,   148,   141,   149,    11,    99,
   262,    10,     5,     1,    49,    12,   133,     0,     0,    47,
    20,    63,   140,    24,    96,     0,    63,    17,     0,    63,
    35,     0,    34,     0,    25,    99,     0,     0,   189,   190,
     0,   134,     0,   200,     0,   184,    44,     0,    63,   117,
     0,   192,     0,   206,   119,     0,    63,     0,     0,   220,
     0,     0,   134,     0,   198,     0,   240,     0,    63,     0,
   191,   194,     0,    21,     0,     0,     0,   152,   152,   152,
   152,   152,   216,   152,   152,     0,   201,    61,     0,     0,
     4,   196,     0,    63,    98,    63,    63,     0,    63,    94,
    29,    23,     4,     0,    32,    33,   204,    63,   115,   116,
    22,     0,    70,     0,     0,    63,    63,    63,    63,     0,
    63,    63,    63,    63,     0,    63,    63,   152,    18,   235,
   172,     0,    30,    31,    27,    28,    23,   172,    98,   172,
    63,   278,   172,   152,    63,    20,   207,     0,    24,   257,
     0,    63,    17,   156,     0,    35,    63,    34,    61,    25,
   259,     0,     0,     0,     0,     0,   277,    61,     0,   283,
     0,    44,     0,   178,   179,   180,   181,   182,   183,   271,
     0,   293,    64,     0,     0,    24,     0,   279,     0,    65,
     0,     0,    35,     0,    34,    61,   287,    61,    61,    61,
     0,    61,    61,    61,    61,    20,    61,     0,    24,    58,
     0,    59,    17,   297,     0,    35,     0,    34,     0,    25,
   302,   303,   304,     0,   214,     0,     0,     0,     0,    98,
     0,    44,    61,     0,     0,    22,     0,    70,     0,     0,
     0,     0,     0,   135,    29,    23,     0,     0,    32,    33,
     0,   229,     0,     0,     0,     0,     0,     0,    85,    61,
    82,    83,    74,    75,    76,    77,    78,    86,    87,    80,
    81,    61,    18,    52,     0,     0,    30,    31,    27,    28,
   156,   156,   156,   156,   156,     0,   156,   156,     0,     0,
    20,     0,     0,    24,     0,    22,    61,    17,    61,    61,
    35,    61,    34,     0,    25,     0,     0,     0,     0,     0,
    61,     0,     0,     0,     0,     0,    44,     0,    61,    61,
    61,   214,     0,   214,   214,   214,   214,     0,   214,   214,
   156,    72,     0,     0,    14,     0,     0,    14,     0,     0,
     0,     0,    14,    61,    50,     0,   156,   214,     0,     0,
     0,     0,     0,    64,    61,     0,    24,     0,     0,   214,
    65,     0,     0,    35,     0,    34,     0,     0,    69,    29,
    23,     0,    14,    32,    33,     0,     0,    14,     0,     0,
    22,     0,     0,    85,     0,    82,    83,    74,    75,    76,
    77,    78,    86,    87,    80,    81,     0,    18,     0,     0,
     0,    30,    31,    27,    28,     0,    29,    23,     0,     0,
    32,    33,    57,    84,     0,    55,    13,     0,     0,    13,
     0,     0,     0,     0,     0,     0,     0,     0,    69,    29,
    23,    93,     0,    32,    33,    56,    53,     0,    30,    31,
    27,    28,     0,    85,     0,    82,    83,    74,    75,    76,
    77,    78,    86,    87,    80,    81,     0,    18,     0,     0,
     0,    30,    31,    27,    28,   121,     0,   123,   124,   125,
     0,   127,   128,   129,   130,    64,     0,     0,    24,   132,
   150,     0,    65,     0,     0,    35,     0,    34,     0,     0,
     0,     0,     0,     0,   158,   158,     0,     0,     0,     0,
     2,    44,    58,     0,    59,    60,    37,     0,     0,     0,
     0,     0,     0,     0,    29,    23,     0,     0,    32,    33,
   185,     0,     0,   105,   185,   185,   185,     0,    85,     0,
    82,    83,    74,    75,    76,    77,    78,    86,    87,    80,
    81,     0,    18,     0,   118,     0,    30,    31,    27,    28,
     0,     0,     0,     0,     0,     0,   197,     0,   158,     0,
     0,    93,     0,   131,     0,     0,    52,     0,     0,     0,
     0,   138,   139,     0,   158,     0,   143,   222,    23,     0,
     0,    32,    33,   212,   224,   225,   227,     0,     0,     0,
     0,     0,     0,     0,    64,     0,     0,    24,     0,     0,
     0,    65,   174,     0,    35,     0,    34,   236,     0,    30,
    31,     0,     0,     0,     0,     0,     0,   244,   245,   246,
     0,    58,     0,    59,    60,     0,     0,     0,     0,     0,
     0,    64,     0,     0,    24,   185,     0,   195,    65,     0,
     0,    35,     0,    34,    64,     0,     0,    24,     0,     0,
     0,    65,     0,     0,    35,   269,    34,    44,    58,     0,
    59,    60,     0,     0,     0,     0,     0,     0,     0,   193,
     0,    58,     0,    59,    60,     0,     0,     0,     0,     0,
     0,   280,     0,     0,   158,    52,     0,   247,   249,   250,
   251,   252,     0,   254,   255,   237,   158,     0,     0,    29,
    23,     0,     0,    32,    33,    57,     0,    64,    55,     4,
    24,     0,     0,    64,    65,     0,    24,    35,     0,    34,
    65,     0,    52,    35,    62,    34,     0,     0,    56,    53,
    54,    30,    31,    27,    28,    52,     0,   273,     0,   266,
    58,     0,    59,    60,     0,     0,     0,     0,     0,     0,
     0,     0,     0,   285,     0,    20,     0,     0,    24,     0,
     0,   275,    17,     0,     0,    35,     0,    34,     0,    25,
     0,    64,     0,     0,    24,     0,     0,   288,    65,     0,
     0,    35,   292,    34,     0,     0,     0,     0,     0,     0,
     0,     0,   296,     0,     0,   299,   300,     0,    58,   301,
    59,    60,     0,     0,    52,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,    29,    23,
     0,     0,    32,    33,    57,    20,     0,    55,    24,     0,
     0,     0,    17,     0,     0,    35,     0,    34,     0,    25,
     0,     0,     0,     0,     0,    22,     0,    56,    53,    54,
    30,    31,    27,    28,     0,    29,    23,     0,     0,    32,
    33,    57,    52,     0,    55,     0,     0,     0,    29,    23,
     0,     0,    32,    33,    57,     0,     0,    55,     0,     0,
     0,     0,     0,     0,    56,    53,    54,    30,    31,    27,
    28,     0,     0,     0,     0,     0,     0,    56,    53,    54,
    30,    31,    27,    28,    64,     0,     0,    24,     0,     0,
     0,    65,     0,     0,    35,    22,    34,     0,    64,     0,
     0,    24,     0,     0,     0,    65,     0,     0,    35,   272,
    34,     0,    23,     0,   213,    32,    33,    29,    23,     0,
     0,    32,    33,    57,     0,     0,    55,     0,   213,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,    30,    31,     0,    56,    53,    54,    30,
    31,    27,    28,     0,     0,     0,     0,     0,    39,    29,
    23,     0,     0,    32,    33,     0,     0,     0,     0,     4,
     0,     8,     9,     0,     0,    29,    23,     0,     0,    32,
    33,    57,     0,     0,    55,     0,    15,    18,     3,     0,
     0,    30,    31,    27,    28,     0,     0,    42,    42,    42,
     0,     0,     0,     0,    56,    53,    54,    30,    31,    27,
    28,    64,     0,     0,    24,     0,     0,     0,    65,     0,
     0,    35,     0,    34,     0,     0,     0,     0,     7,    29,
    23,     0,     0,    32,    33,     0,     0,     0,    58,     0,
    59,     8,     9,     0,     0,     0,     0,     0,    64,     0,
     0,    24,     0,     0,     0,    65,    15,    18,    35,     0,
    34,    30,    31,    27,    28,    64,    42,    42,    24,     0,
     0,     0,    65,    42,     0,    35,     0,    34,     0,     0,
     0,     0,     0,     0,    20,     0,     0,    24,     0,     0,
     0,    17,     0,     0,    35,     0,    34,     0,    25,     0,
     0,     0,    52,     0,     0,     0,     0,     0,    29,    23,
    44,     0,    32,    33,   212,     0,     0,   210,     0,     0,
     0,     0,    29,    23,     0,     0,    32,    33,   212,     0,
     0,   210,     0,     0,     0,     0,     0,   211,   208,   209,
    30,    31,    27,    28,     0,     0,    42,     0,    42,     0,
     0,   211,   208,   209,    30,    31,    27,    28,   155,     0,
     0,    24,     0,     0,     0,    65,    20,     0,    35,    24,
    34,     0,    25,    17,   155,     0,    35,    24,    34,     0,
    25,   147,     0,     0,    35,     0,    34,     0,    25,     0,
     0,    42,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,    42,
    20,     0,     0,    24,     0,     0,     0,    17,   155,     0,
    35,    24,    34,     0,    25,    65,     0,     0,    35,     0,
    34,     0,    25,     0,     0,    29,    23,     0,     0,    32,
    33,    57,     0,     0,    55,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    42,     0,   175,    42,    56,     0,     0,    30,    31,    27,
    28,     0,    29,    23,     0,     0,    32,    33,   212,     0,
     0,   210,     0,     0,     0,     0,     0,     0,     0,    29,
    23,     0,     0,    32,    33,   212,     0,     0,   210,     0,
     0,   211,   208,     0,    30,    31,    27,    28,    29,    23,
     0,     0,    32,    33,     0,     0,     0,     0,   211,     0,
     0,    30,    31,    27,    28,    20,     0,     0,    24,     0,
     0,     0,    17,     0,     0,    35,    18,    34,     0,    25,
    30,    31,    27,    28,    64,     0,     0,    24,     0,     0,
    64,    65,     0,    24,    35,   173,    34,    65,    25,     0,
    35,     0,    34,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,   248,    29,    23,     0,     0,    32,    33,     0,   226,
    29,    23,     0,     0,    32,    33,     0,   151,    29,    23,
     0,     0,    32,    33,     0,     0,     0,     0,     0,     0,
   153,     0,     0,     0,    30,    31,    27,    28,    18,     0,
     0,     0,    30,    31,    27,    28,   153,     0,     0,     0,
    30,    31,    27,    28,    29,    23,     0,     0,    32,    33,
     0,     0,    29,    23,     0,     0,    32,    33,     0,     0,
   232,    64,   233,   234,    24,     0,     0,     0,    65,     0,
   238,    35,    18,    34,   242,     0,    30,    31,    27,    28,
   153,     0,     0,     0,    30,    31,    27,    28,    58,     0,
    59,    41,    19,     0,     0,   260,     0,     0,     0,     0,
     0,    46,    48,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,    97,     0,     0,     0,     0,    71,     0,
     0,     0,     0,     0,     0,     0,     0,    43,    43,    43,
     0,     0,     0,     0,   276,     0,     0,     0,     0,     0,
     0,   281,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,    52,   291,     0,     0,   294,     0,     0,    29,
   199,     0,     0,    32,    33,     0,   298,     0,     0,   145,
   146,     0,   154,     0,     0,     0,   162,     0,    29,    23,
     0,     0,    32,    33,    29,    23,   137,    18,    32,    33,
     0,    30,    31,    27,    28,     0,    43,    43,     0,     0,
     0,     0,     0,    43,     0,     0,     0,     0,     0,     0,
    30,    31,    27,    28,     0,     0,    30,    31,    27,    28,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,   217,   218,
     0,   219,   137,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,    43,     0,    43,     0,
     0,     0,     0,     0,     0,     0,    23,     0,     0,    32,
    33,    57,     0,     0,   243,     0,     0,     0,     0,   154,
   154,   154,   154,   154,     0,   154,   154,     0,     0,     0,
   137,     0,   258,     0,     0,     0,     0,    30,    31,   241,
     0,    43,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,    43,
     0,     0,     0,     0,     0,     0,     0,     0,     0,   154,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
   270,     0,     0,   284,     0,   154,   286,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,   282,     0,
    43,     0,     0,    43 };
yytabelem yypact[]={

  -245, -1000,   922,  -244, -1000,   852, -1000, -1000,   -35,   -35,
   -47, -1000,   -69,   810,   242, -1000,  -260,   -27,     8, -1000,
  1461,    45,  -245,   -21,  1467, -1000, -1000,    51,    49,    44,
     8,     8, -1000, -1000,  1467,  1467, -1000, -1000, -1000, -1000,
   -69, -1000,  -244,  -245, -1000,   -69, -1000, -1000, -1000, -1000,
   302,  1327,  -274,  1327,  1327,  1327,  -205,  1327,  1327,  1327,
  1327,    45,  -245,  -282,  1467,  1327,   117,    14, -1000, -1000,
  -245,  -245,   242, -1000,    43,  -245,    41,   -35,   -35,  1291,
  1327,  1327, -1000,  -207,   572,    38, -1000, -1000,  -254, -1000,
 -1000, -1000,    11,   691, -1000,    12, -1000, -1000,   -33,  1467,
  1467,  1467,  1467,  1467,  1467, -1000,   -27, -1000, -1000,  -214,
   -27,   -27,   -27, -1000, -1000,   -33,   -33, -1000, -1000, -1000,
   117,   868,     8,  1128,   279,  1568, -1000,   804,   804,   804,
   741, -1000,   691,    14, -1000, -1000,  -245, -1000, -1000, -1000,
 -1000,   117,  1327,   387,  1442, -1000, -1000,   -27,   -28,    20,
  1015, -1000,  -263,     8, -1000,  1461,    45,   -35,   868,   -35,
  1327,   -22, -1000,  1327,    37, -1000, -1000, -1000, -1000, -1000,
  1327,  1283,  1327,  -276, -1000, -1000, -1000,  1467,   -33,   -33,
   -33,   -33,    71,    71,    13,   868,    24,    33,    16,    33,
    33,    14, -1000,  1327,  -245, -1000, -1000,   691,  -258,   -90,
    14,    11,   -35,  1327,  1327,  1327,  1275,  1335,  1335,  1335,
  1335,  -209,  1335,  1335,    45, -1000,    12, -1000, -1000, -1000,
   -35,   -27,   691,  -216,   868,   868, -1000,   868,  -212,    45,
 -1000, -1000, -1000, -1000, -1000, -1000,   868, -1000,  -245,    35,
  -215,  1201,   -28, -1000,   868,   868,   868,  1015, -1000,  1015,
  1182,  1165,   450, -1000,   804,  1001,  1335, -1000, -1000,     4,
  -245,    33,    15, -1000, -1000, -1000,   387,  1327,    33,   728,
  1327,   -35,  1335,  1015,   -35,   387,  -245,  -217, -1000, -1000,
   691,  -245,  1327,    33, -1000,  1015, -1000,  -257, -1000, -1000,
 -1000,  -245,   387,    33,  -245,  -245, -1000, -1000,  -245,   387,
   387,   387, -1000, -1000, -1000 };
yytabelem yypgo[]={

     0,   145,   144,   610,   543,   143,   142,    93,   203,    95,
   141,   138,     0,   140,   137,   135,    68,    56,    63,    50,
   134,   132,    28,    58,  1632,    62,    66,   127,   121,   119,
   630,   116,   115,  1631,   112,  1412,   461,    79,    67,    33,
   111,  1138,  1658,   109,   107,    98 };
yytabelem yyr1[]={

     0,     5,     6,     6,     6,     6,    31,     7,    32,     7,
     7,     7,     7,     7,     7,     7,    28,    28,    28,    29,
    29,    34,     1,     2,    10,    10,    40,    24,    11,    11,
    18,    18,    18,    18,    33,    33,    19,    19,    19,    19,
    19,    19,    19,    19,    19,    19,    19,    19,    19,    19,
    19,    43,    19,    19,    19,    27,    27,    20,    20,    41,
    41,    30,    30,    25,    25,    26,    26,    26,    26,    21,
    21,    13,    13,    13,    13,    13,    22,    22,    15,    15,
    14,    14,    14,    14,    14,    14,    17,    17,    16,    16,
    16,    16,    16,    16,    44,     4,     4,     4,     4,     4,
     4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
     4,    45,     3,     3,     3,     3,     3,     3,     3,     3,
     3,     3,     3,     3,     8,     8,     8,     8,     8,     8,
     8,     8,     8,     9,     9,     9,     9,     9,     9,     9,
     9,     9,     9,     9,     9,     9,    23,    23,    12,    12,
    12,    36,    37,    35,    38,    38,    42,    39 };
yytabelem yyr2[]={

     0,     7,     3,     5,     3,     5,     1,     7,     1,     7,
     5,     5,     5,     3,     5,     5,     3,     3,     3,     2,
     2,     1,    15,     9,     3,     7,     1,     9,    11,     9,
     3,     5,     3,     5,     2,     4,     5,     5,     7,     3,
    13,    17,    17,    21,    19,     5,     5,    13,     9,     7,
     7,     1,     9,    13,     5,     3,     3,    13,    19,     3,
     4,     0,     2,     1,     5,     1,     5,     5,     5,     1,
     3,     3,     7,     3,     5,     7,     1,     3,     1,     3,
     3,     7,     3,     5,     7,     7,     1,     3,     3,     7,
     3,     5,     7,     7,     1,     9,    11,     9,     7,     7,
     7,     7,     3,     5,     7,     7,     7,     7,    11,     3,
     5,     1,     9,     7,     7,     7,     3,     5,     7,     7,
     7,    11,     3,     5,     2,     7,     7,     7,     7,     7,
     7,     5,     5,     5,     7,     9,     9,     3,     9,     2,
     5,     5,     3,     3,     5,     5,     1,     3,     3,     9,
     5,     4,     5,     3,     0,     2,     3,     5 };
yytabelem yychk[]={

 -1000,    -5,   -30,   -41,   267,    -6,    -7,   256,   269,   270,
   -10,   -11,    -1,    -4,   -36,   284,   -12,    40,   285,   -24,
    33,    -8,   123,   258,    36,    47,    -9,   291,   292,   257,
   289,   290,   261,   262,    45,    43,   267,   -30,    -7,   256,
   -31,   -33,   -41,   -42,    59,   -32,   -33,   -11,   -33,    -2,
   -36,   -39,   124,   287,   288,   266,   286,   263,    60,    62,
    63,    -8,    44,   -12,    33,    40,   -18,   -37,   -19,   256,
   125,   -42,   -36,   -20,   275,   276,   277,   278,   279,   -27,
   282,   283,   273,   274,    -4,   271,   280,   281,   -34,   265,
   289,   290,   -16,    -4,   256,   -23,   -12,   -24,    -8,    94,
    42,    47,    37,    43,    45,   -30,    91,    -9,   -12,   -40,
    40,    40,    40,   -12,   -12,    -8,    -8,   -11,   -30,   -11,
   -18,    -4,   285,    -4,    -4,    -4,   258,    -4,    -4,    -4,
    -4,   -30,    -4,   -37,   -19,   256,   -38,   -42,   -30,   -30,
   -37,   -18,    40,   -30,    40,   -33,   -33,    40,   -15,   -14,
    -3,   256,   -12,   285,   -24,    33,    -8,   -22,    -4,   -22,
   -43,   258,   -33,    40,   -28,   258,   257,   -29,   291,   292,
   -44,   -39,   256,   -35,    41,   -35,   -25,    60,    -8,    -8,
    -8,    -8,    -8,    -8,   -16,    -4,   259,   -17,   -16,   -17,
   -17,   -37,   -23,    58,   -38,   -30,   -37,    -4,   -19,   258,
   -22,   -16,   -26,    62,   264,   124,   -39,   256,   287,   288,
   266,   286,   263,    63,    -8,   265,   -23,   -24,   -33,   -33,
   -22,    91,    -4,    40,    -4,    -4,   256,    -4,   286,    -8,
    93,    47,   -35,   -35,   -35,   -38,    -4,   -30,   -35,   275,
   286,   -42,   -35,   -33,    -4,    -4,    -4,    -3,   256,    -3,
    -3,    -3,    -3,   258,    -3,    -3,   -45,   -25,   -33,   -16,
   -35,   -21,   -13,   258,   256,   258,   -30,    40,   258,    -4,
   -42,   -26,    58,    -3,    93,   -30,   -35,   -39,   256,   -19,
    -4,   -35,   -42,   -22,   -33,    -3,   -33,   -19,   -30,   258,
   256,   -35,   -30,   -22,   -35,   272,   -30,   -19,   -35,   -30,
   -30,   -30,   -19,   -19,   -19 };
yytabelem yydef[]={

    61,    -2,     0,    62,    59,    61,     2,     4,     6,     8,
     0,    13,     0,    24,     0,    21,   139,     0,   146,   102,
     0,   109,    61,   148,     0,    26,   124,     0,   137,     0,
     0,     0,   142,   143,     0,     0,    60,     1,     3,     5,
     0,    10,    34,    61,   156,     0,    11,    12,    14,    15,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,   110,    61,   139,     0,     0,     0,   154,    30,    32,
    61,    61,     0,    39,     0,    61,     0,     0,     0,    78,
    76,    76,    51,     0,     0,     0,    55,    56,     0,    94,
   131,   132,     0,    88,    90,    63,   147,   103,   133,     0,
     0,     0,     0,     0,     0,   151,     0,   150,   139,     0,
    86,    86,    86,   140,   141,   144,   145,     7,    35,     9,
     0,    25,   146,    99,   100,    -2,   104,    -2,    -2,    -2,
     0,   157,     0,   154,    31,    33,    61,   155,   152,    36,
    37,     0,     0,     0,    76,    45,    46,     0,    65,    79,
    80,    82,   139,   146,   116,     0,   122,     0,    77,     0,
    76,     0,    54,     0,     0,    16,    17,    18,    19,    20,
     0,     0,    91,     0,   153,   134,    98,     0,   125,   126,
   127,   128,   129,   130,     0,    88,     0,     0,    87,     0,
     0,   154,    97,     0,    61,    29,    38,     0,     0,   148,
     0,     0,     0,     0,     0,     0,     0,    83,     0,     0,
     0,     0,     0,     0,   123,   111,    63,   117,    49,    50,
     0,     0,     0,    69,    95,    89,    93,    92,     0,    64,
   149,    27,   135,   136,   138,    23,   108,    28,    61,     0,
     0,     0,    65,    48,    66,    67,    68,    81,    85,    84,
   113,   114,    -2,   119,    -2,     0,     0,   115,    52,     0,
    61,     0,    70,    71,    73,    96,     0,     0,     0,     0,
    76,     0,     0,   112,     0,     0,    61,     0,    74,    40,
     0,    61,    76,     0,    47,   121,    53,    57,    22,    72,
    75,    61,     0,     0,    61,    61,    41,    42,    61,     0,
     0,     0,    44,    58,    43 };
typedef struct { char *t_name; int t_val; } yytoktype;
#ifndef YYDEBUG
#	define YYDEBUG	0	/* don't allow debugging */
#endif

#if YYDEBUG

yytoktype yytoks[] =
{
	"FUNC_CALL",	257,
	"NAME",	258,
	"REGEXP",	259,
	"ERROR",	260,
	"YNUMBER",	261,
	"YSTRING",	262,
	"RELOP",	263,
	"APPEND_OP",	264,
	"ASSIGNOP",	265,
	"MATCHOP",	266,
	"NEWLINE",	267,
	"CONCAT_OP",	268,
	"LEX_BEGIN",	269,
	"LEX_END",	270,
	"LEX_IF",	271,
	"LEX_ELSE",	272,
	"LEX_RETURN",	273,
	"LEX_DELETE",	274,
	"LEX_WHILE",	275,
	"LEX_DO",	276,
	"LEX_FOR",	277,
	"LEX_BREAK",	278,
	"LEX_CONTINUE",	279,
	"LEX_PRINT",	280,
	"LEX_PRINTF",	281,
	"LEX_NEXT",	282,
	"LEX_EXIT",	283,
	"LEX_FUNCTION",	284,
	"LEX_GETLINE",	285,
	"LEX_IN",	286,
	"LEX_AND",	287,
	"LEX_OR",	288,
	"INCREMENT",	289,
	"DECREMENT",	290,
	"LEX_BUILTIN",	291,
	"LEX_LENGTH",	292,
	"?",	63,
	":",	58,
	"<",	60,
	">",	62,
	"|",	124,
	"+",	43,
	"-",	45,
	"*",	42,
	"/",	47,
	"%",	37,
	"!",	33,
	"UNARY",	293,
	"^",	94,
	"$",	36,
	"(",	40,
	")",	41,
	"-unknown-",	-1	/* ends search */
};

char * yyreds[] =
{
	"-no such reduction-",
	"start : opt_nls program opt_nls",
	"program : rule",
	"program : program rule",
	"program : error",
	"program : program error",
	"rule : LEX_BEGIN",
	"rule : LEX_BEGIN action",
	"rule : LEX_END",
	"rule : LEX_END action",
	"rule : LEX_BEGIN statement_term",
	"rule : LEX_END statement_term",
	"rule : pattern action",
	"rule : action",
	"rule : pattern statement_term",
	"rule : function_prologue function_body",
	"func_name : NAME",
	"func_name : FUNC_CALL",
	"func_name : lex_builtin",
	"lex_builtin : LEX_BUILTIN",
	"lex_builtin : LEX_LENGTH",
	"function_prologue : LEX_FUNCTION",
	"function_prologue : LEX_FUNCTION func_name '(' opt_param_list r_paren opt_nls",
	"function_body : l_brace statements r_brace opt_semi",
	"pattern : exp",
	"pattern : exp comma exp",
	"regexp : '/'",
	"regexp : '/' REGEXP '/'",
	"action : l_brace statements r_brace opt_semi opt_nls",
	"action : l_brace r_brace opt_semi opt_nls",
	"statements : statement",
	"statements : statements statement",
	"statements : error",
	"statements : statements error",
	"statement_term : nls",
	"statement_term : semi opt_nls",
	"statement : semi opt_nls",
	"statement : l_brace r_brace",
	"statement : l_brace statements r_brace",
	"statement : if_statement",
	"statement : LEX_WHILE '(' exp r_paren opt_nls statement",
	"statement : LEX_DO opt_nls statement LEX_WHILE '(' exp r_paren opt_nls",
	"statement : LEX_FOR '(' NAME LEX_IN NAME r_paren opt_nls statement",
	"statement : LEX_FOR '(' opt_exp semi exp semi opt_exp r_paren opt_nls statement",
	"statement : LEX_FOR '(' opt_exp semi semi opt_exp r_paren opt_nls statement",
	"statement : LEX_BREAK statement_term",
	"statement : LEX_CONTINUE statement_term",
	"statement : print '(' expression_list r_paren output_redir statement_term",
	"statement : print opt_rexpression_list output_redir statement_term",
	"statement : LEX_NEXT opt_exp statement_term",
	"statement : LEX_EXIT opt_exp statement_term",
	"statement : LEX_RETURN",
	"statement : LEX_RETURN opt_exp statement_term",
	"statement : LEX_DELETE NAME '[' expression_list ']' statement_term",
	"statement : exp statement_term",
	"print : LEX_PRINT",
	"print : LEX_PRINTF",
	"if_statement : LEX_IF '(' exp r_paren opt_nls statement",
	"if_statement : LEX_IF '(' exp r_paren opt_nls statement LEX_ELSE opt_nls statement",
	"nls : NEWLINE",
	"nls : nls NEWLINE",
	"opt_nls : /* empty */",
	"opt_nls : nls",
	"input_redir : /* empty */",
	"input_redir : '<' simp_exp",
	"output_redir : /* empty */",
	"output_redir : '>' exp",
	"output_redir : APPEND_OP exp",
	"output_redir : '|' exp",
	"opt_param_list : /* empty */",
	"opt_param_list : param_list",
	"param_list : NAME",
	"param_list : param_list comma NAME",
	"param_list : error",
	"param_list : param_list error",
	"param_list : param_list comma error",
	"opt_exp : /* empty */",
	"opt_exp : exp",
	"opt_rexpression_list : /* empty */",
	"opt_rexpression_list : rexpression_list",
	"rexpression_list : rexp",
	"rexpression_list : rexpression_list comma rexp",
	"rexpression_list : error",
	"rexpression_list : rexpression_list error",
	"rexpression_list : rexpression_list error rexp",
	"rexpression_list : rexpression_list comma error",
	"opt_expression_list : /* empty */",
	"opt_expression_list : expression_list",
	"expression_list : exp",
	"expression_list : expression_list comma exp",
	"expression_list : error",
	"expression_list : expression_list error",
	"expression_list : expression_list error exp",
	"expression_list : expression_list comma error",
	"exp : variable ASSIGNOP",
	"exp : variable ASSIGNOP exp",
	"exp : '(' expression_list r_paren LEX_IN NAME",
	"exp : exp '|' LEX_GETLINE opt_variable",
	"exp : LEX_GETLINE opt_variable input_redir",
	"exp : exp LEX_AND exp",
	"exp : exp LEX_OR exp",
	"exp : exp MATCHOP exp",
	"exp : regexp",
	"exp : '!' regexp",
	"exp : exp LEX_IN NAME",
	"exp : exp RELOP exp",
	"exp : exp '<' exp",
	"exp : exp '>' exp",
	"exp : exp '?' exp ':' exp",
	"exp : simp_exp",
	"exp : exp simp_exp",
	"rexp : variable ASSIGNOP",
	"rexp : variable ASSIGNOP rexp",
	"rexp : rexp LEX_AND rexp",
	"rexp : rexp LEX_OR rexp",
	"rexp : LEX_GETLINE opt_variable input_redir",
	"rexp : regexp",
	"rexp : '!' regexp",
	"rexp : rexp MATCHOP rexp",
	"rexp : rexp LEX_IN NAME",
	"rexp : rexp RELOP rexp",
	"rexp : rexp '?' rexp ':' rexp",
	"rexp : simp_exp",
	"rexp : rexp simp_exp",
	"simp_exp : non_post_simp_exp",
	"simp_exp : simp_exp '^' simp_exp",
	"simp_exp : simp_exp '*' simp_exp",
	"simp_exp : simp_exp '/' simp_exp",
	"simp_exp : simp_exp '%' simp_exp",
	"simp_exp : simp_exp '+' simp_exp",
	"simp_exp : simp_exp '-' simp_exp",
	"simp_exp : variable INCREMENT",
	"simp_exp : variable DECREMENT",
	"non_post_simp_exp : '!' simp_exp",
	"non_post_simp_exp : '(' exp r_paren",
	"non_post_simp_exp : LEX_BUILTIN '(' opt_expression_list r_paren",
	"non_post_simp_exp : LEX_LENGTH '(' opt_expression_list r_paren",
	"non_post_simp_exp : LEX_LENGTH",
	"non_post_simp_exp : FUNC_CALL '(' opt_expression_list r_paren",
	"non_post_simp_exp : variable",
	"non_post_simp_exp : INCREMENT variable",
	"non_post_simp_exp : DECREMENT variable",
	"non_post_simp_exp : YNUMBER",
	"non_post_simp_exp : YSTRING",
	"non_post_simp_exp : '-' simp_exp",
	"non_post_simp_exp : '+' simp_exp",
	"opt_variable : /* empty */",
	"opt_variable : variable",
	"variable : NAME",
	"variable : NAME '[' expression_list ']'",
	"variable : '$' non_post_simp_exp",
	"l_brace : '{' opt_nls",
	"r_brace : '}' opt_nls",
	"r_paren : ')'",
	"opt_semi : /* empty */",
	"opt_semi : semi",
	"semi : ';'",
	"comma : ',' opt_nls",
};
#endif /* YYDEBUG */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/* @(#)yaccpar	1.3  com/cmd/lang/yacc,3.1, 9/7/89 18:46:37 */
/*
** Skeleton parser driver for yacc output
*/

/*
** yacc user known macros and defines
*/
#ifdef YYSPLIT
#   define YYERROR	return(-2)
#else
#   define YYERROR	goto yyerrlab
#endif

#define YYACCEPT	return(0)
#define YYABORT		return(1)
#define YYBACKUP( newtoken, newvalue )\
{\
	if ( yychar >= 0 || ( yyr2[ yytmp ] >> 1 ) != 1 )\
	{\
		yyerror( "syntax error - cannot backup" );\
		goto yyerrlab;\
	}\
	yychar = newtoken;\
	yystate = *yyps;\
	yylval = newvalue;\
	goto yynewstate;\
}
#define YYRECOVERING()	(!!yyerrflag)
#ifndef YYDEBUG
#	define YYDEBUG	1	/* make debugging available */
#endif

/*
** user known globals
*/
int yydebug;			/* set to 1 to get debugging */

/*
** driver internal defines
*/
#define YYFLAG		(-1000)

#ifdef YYSPLIT
#   define YYSCODE { \
			extern int (*yyf[])(); \
			register int yyret; \
			if (yyf[yytmp]) \
			    if ((yyret=(*yyf[yytmp])()) == -2) \
				    goto yyerrlab; \
				else if (yyret>=0) return(yyret); \
		   }
#endif

/*
** global variables used by the parser
*/
YYSTYPE yyv[ YYMAXDEPTH ];	/* value stack */
int yys[ YYMAXDEPTH ];		/* state stack */

YYSTYPE *yypv;			/* top of value stack */
YYSTYPE *yypvt;			/* top of value stack for $vars */
int *yyps;			/* top of state stack */

int yystate;			/* current state */
int yytmp;			/* extra var (lasts between blocks) */

int yynerrs;			/* number of errors */
int yyerrflag;			/* error recovery flag */
int yychar;			/* current input token number */



/*
** yyparse - return 0 if worked, 1 if syntax error not recovered from
*/
int
yyparse()
{
	/*
	** Initialize externals - yyparse may be called more than once
	*/
	yypv = &yyv[-1];
	yyps = &yys[-1];
	yystate = 0;
	yytmp = 0;
	yynerrs = 0;
	yyerrflag = 0;
	yychar = -1;

	goto yystack;
	{
		register YYSTYPE *yy_pv;	/* top of value stack */
		register int *yy_ps;		/* top of state stack */
		register int yy_state;		/* current state */
		register int  yy_n;		/* internal state number info */

		/*
		** get globals into registers.
		** branch to here only if YYBACKUP was called.
		*/
	yynewstate:
		yy_pv = yypv;
		yy_ps = yyps;
		yy_state = yystate;
		goto yy_newstate;

		/*
		** get globals into registers.
		** either we just started, or we just finished a reduction
		*/
	yystack:
		yy_pv = yypv;
		yy_ps = yyps;
		yy_state = yystate;

		/*
		** top of for (;;) loop while no reductions done
		*/
	yy_stack:
		/*
		** put a state and value onto the stacks
		*/
#if YYDEBUG
		/*
		** if debugging, look up token value in list of value vs.
		** name pairs.  0 and negative (-1) are special values.
		** Note: linear search is used since time is not a real
		** consideration while debugging.
		*/
		if ( yydebug )
		{
			register int yy_i;

			printf( "State %d, token ", yy_state );
			if ( yychar == 0 )
				printf( "end-of-file\n" );
			else if ( yychar < 0 )
				printf( "-none-\n" );
			else
			{
				for ( yy_i = 0; yytoks[yy_i].t_val >= 0;
					yy_i++ )
				{
					if ( yytoks[yy_i].t_val == yychar )
						break;
				}
				printf( "%s\n", yytoks[yy_i].t_name );
			}
		}
#endif /* YYDEBUG */
		if ( ++yy_ps >= &yys[ YYMAXDEPTH ] )	/* room on stack? */
		{
			yyerror( "yacc stack overflow" );
			YYABORT;
		}
		*yy_ps = yy_state;
		*++yy_pv = yyval;

		/*
		** we have a new state - find out what to do
		*/
	yy_newstate:
		if ( ( yy_n = yypact[ yy_state ] ) <= YYFLAG )
			goto yydefault;		/* simple state */
#if YYDEBUG
		/*
		** if debugging, need to mark whether new token grabbed
		*/
		yytmp = yychar < 0;
#endif
		if ( ( yychar < 0 ) && ( ( yychar = yylex() ) < 0 ) )
			yychar = 0;		/* reached EOF */
#if YYDEBUG
		if ( yydebug && yytmp )
		{
			register int yy_i;

			printf( "Received token " );
			if ( yychar == 0 )
				printf( "end-of-file\n" );
			else if ( yychar < 0 )
				printf( "-none-\n" );
			else
			{
				for ( yy_i = 0; yytoks[yy_i].t_val >= 0;
					yy_i++ )
				{
					if ( yytoks[yy_i].t_val == yychar )
						break;
				}
				printf( "%s\n", yytoks[yy_i].t_name );
			}
		}
#endif /* YYDEBUG */
		if ( ( ( yy_n += yychar ) < 0 ) || ( yy_n >= YYLAST ) )
			goto yydefault;
		if ( yychk[ yy_n = yyact[ yy_n ] ] == yychar )	/*valid shift*/
		{
			yychar = -1;
			yyval = yylval;
			yy_state = yy_n;
			if ( yyerrflag > 0 )
				yyerrflag--;
			goto yy_stack;
		}

	yydefault:
		if ( ( yy_n = yydef[ yy_state ] ) == -2 )
		{
#if YYDEBUG
			yytmp = yychar < 0;
#endif
			if ( ( yychar < 0 ) && ( ( yychar = yylex() ) < 0 ) )
				yychar = 0;		/* reached EOF */
#if YYDEBUG
			if ( yydebug && yytmp )
			{
				register int yy_i;

				printf( "Received token " );
				if ( yychar == 0 )
					printf( "end-of-file\n" );
				else if ( yychar < 0 )
					printf( "-none-\n" );
				else
				{
					for ( yy_i = 0;
						yytoks[yy_i].t_val >= 0;
						yy_i++ )
					{
						if ( yytoks[yy_i].t_val
							== yychar )
						{
							break;
						}
					}
					printf( "%s\n", yytoks[yy_i].t_name );
				}
			}
#endif /* YYDEBUG */
			/*
			** look through exception table
			*/
			{
				register int *yyxi = yyexca;

				while ( ( *yyxi != -1 ) ||
					( yyxi[1] != yy_state ) )
				{
					yyxi += 2;
				}
				while ( ( *(yyxi += 2) >= 0 ) &&
					( *yyxi != yychar ) )
					;
				if ( ( yy_n = yyxi[1] ) < 0 )
					YYACCEPT;
			}
		}

		/*
		** check for syntax error
		*/
		if ( yy_n == 0 )	/* have an error */
		{
			/* no worry about speed here! */
			switch ( yyerrflag )
			{
			case 0:		/* new error */
				yyerror( "syntax error" );
				goto skip_init;
			yyerrlab:
				/*
				** get globals into registers.
				** we have a user generated syntax type error
				*/
				yy_pv = yypv;
				yy_ps = yyps;
				yy_state = yystate;
				yynerrs++;
			skip_init:
			case 1:
			case 2:		/* incompletely recovered error */
					/* try again... */
				yyerrflag = 3;
				/*
				** find state where "error" is a legal
				** shift action
				*/
				while ( yy_ps >= yys )
				{
					yy_n = yypact[ *yy_ps ] + YYERRCODE;
					if ( yy_n >= 0 && yy_n < YYLAST &&
						yychk[yyact[yy_n]] == YYERRCODE)					{
						/*
						** simulate shift of "error"
						*/
						yy_state = yyact[ yy_n ];
						goto yy_stack;
					}
					/*
					** current state has no shift on
					** "error", pop stack
					*/
#if YYDEBUG
#	define _POP_ "Error recovery pops state %d, uncovers state %d\n"
					if ( yydebug )
						printf( _POP_, *yy_ps,
							yy_ps[-1] );
#	undef _POP_
#endif
					yy_ps--;
					yy_pv--;
				}
				/*
				** there is no state on stack with "error" as
				** a valid shift.  give up.
				*/
				YYABORT;
			case 3:		/* no shift yet; eat a token */
#if YYDEBUG
				/*
				** if debugging, look up token in list of
				** pairs.  0 and negative shouldn't occur,
				** but since timing doesn't matter when
				** debugging, it doesn't hurt to leave the
				** tests here.
				*/
				if ( yydebug )
				{
					register int yy_i;

					printf( "Error recovery discards " );
					if ( yychar == 0 )
						printf( "token end-of-file\n" );
					else if ( yychar < 0 )
						printf( "token -none-\n" );
					else
					{
						for ( yy_i = 0;
							yytoks[yy_i].t_val >= 0;
							yy_i++ )
						{
							if ( yytoks[yy_i].t_val
								== yychar )
							{
								break;
							}
						}
						printf( "token %s\n",
							yytoks[yy_i].t_name );
					}
				}
#endif /* YYDEBUG */
				if ( yychar == 0 )	/* reached EOF. quit */
					YYABORT;
				yychar = -1;
				goto yy_newstate;
			}
		}/* end if ( yy_n == 0 ) */
		/*
		** reduction by production yy_n
		** put stack tops, etc. so things right after switch
		*/
#if YYDEBUG
		/*
		** if debugging, print the string that is the user's
		** specification of the reduction which is just about
		** to be done.
		*/
		if ( yydebug )
			printf( "Reduce by (%d) \"%s\"\n",
				yy_n, yyreds[ yy_n ] );
#endif
		yytmp = yy_n;			/* value to switch over */
		yypvt = yy_pv;			/* $vars top of value stack */
		/*
		** Look in goto table for next state
		** Sorry about using yy_state here as temporary
		** register variable, but why not, if it works...
		** If yyr2[ yy_n ] doesn't have the low order bit
		** set, then there is no action to be done for
		** this reduction.  So, no saving & unsaving of
		** registers done.  The only difference between the
		** code just after the if and the body of the if is
		** the goto yy_stack in the body.  This way the test
		** can be made before the choice of what to do is needed.
		*/
		{
			/* length of production doubled with extra bit */
			register int yy_len = yyr2[ yy_n ];

			if ( !( yy_len & 01 ) )
			{
				yy_len >>= 1;
				yyval = ( yy_pv -= yy_len )[1];	/* $$ = $1 */
				yy_state = yypgo[ yy_n = yyr1[ yy_n ] ] +
					*( yy_ps -= yy_len ) + 1;
				if ( yy_state >= YYLAST ||
					yychk[ yy_state =
					yyact[ yy_state ] ] != -yy_n )
				{
					yy_state = yyact[ yypgo[ yy_n ] ];
				}
				goto yy_stack;
			}
			yy_len >>= 1;
			yyval = ( yy_pv -= yy_len )[1];	/* $$ = $1 */
			yy_state = yypgo[ yy_n = yyr1[ yy_n ] ] +
				*( yy_ps -= yy_len ) + 1;
			if ( yy_state >= YYLAST ||
				yychk[ yy_state = yyact[ yy_state ] ] != -yy_n )
			{
				yy_state = yyact[ yypgo[ yy_n ] ];
			}
		}
					/* save until reenter driver code */
		yystate = yy_state;
		yyps = yy_ps;
		yypv = yy_pv;
	}
	/*
	** code supplied by user is placed in this switch
	*/

		switch(yytmp){

case 1:
# line 137 "awk.y"
{ expression_value = yypvt[-1].nodeval; } break;
case 2:
# line 142 "awk.y"
{ 
			if (yypvt[-0].nodeval != NULL)
				yyval.nodeval = yypvt[-0].nodeval;
			else
				yyval.nodeval = NULL;
			yyerrok;
		} break;
case 3:
# line 151 "awk.y"
{
			if (yypvt[-0].nodeval == NULL)
				yyval.nodeval = yypvt[-1].nodeval;
			else if (yypvt[-1].nodeval == NULL)
				yyval.nodeval = yypvt[-0].nodeval;
			else {
				if (yypvt[-1].nodeval->type != Node_rule_list)
					yypvt[-1].nodeval = node(yypvt[-1].nodeval, Node_rule_list,
						(NODE*)NULL);
				yyval.nodeval = append_right (yypvt[-1].nodeval,
				   node(yypvt[-0].nodeval, Node_rule_list,(NODE *) NULL));
			}
			yyerrok;
		} break;
case 4:
# line 165 "awk.y"
{ yyval.nodeval = NULL; } break;
case 5:
# line 166 "awk.y"
{ yyval.nodeval = NULL; } break;
case 6:
# line 170 "awk.y"
{ io_allowed = 0; } break;
case 7:
# line 172 "awk.y"
{
		if (begin_block) {
			if (begin_block->type != Node_rule_list)
				begin_block = node(begin_block, Node_rule_list,
					(NODE *)NULL);
			(void) append_right (begin_block, node(
			    node((NODE *)NULL, Node_rule_node, yypvt[-0].nodeval),
			    Node_rule_list, (NODE *)NULL) );
		} else
			begin_block = node((NODE *)NULL, Node_rule_node, yypvt[-0].nodeval);
		yyval.nodeval = NULL;
		io_allowed = 1;
		yyerrok;
	  } break;
case 8:
# line 186 "awk.y"
{ io_allowed = 0; } break;
case 9:
# line 188 "awk.y"
{
		if (end_block) {
			if (end_block->type != Node_rule_list)
				end_block = node(end_block, Node_rule_list,
					(NODE *)NULL);
			(void) append_right (end_block, node(
			    node((NODE *)NULL, Node_rule_node, yypvt[-0].nodeval),
			    Node_rule_list, (NODE *)NULL));
		} else
			end_block = node((NODE *)NULL, Node_rule_node, yypvt[-0].nodeval);
		yyval.nodeval = NULL;
		io_allowed = 1;
		yyerrok;
	  } break;
case 10:
# line 203 "awk.y"
{
		warning("BEGIN blocks must have an action part");
		errcount++;
		yyerrok;
	  } break;
case 11:
# line 209 "awk.y"
{
		warning("END blocks must have an action part");
		errcount++;
		yyerrok;
	  } break;
case 12:
# line 215 "awk.y"
{ yyval.nodeval = node (yypvt[-1].nodeval, Node_rule_node, yypvt[-0].nodeval); yyerrok; } break;
case 13:
# line 217 "awk.y"
{ yyval.nodeval = node ((NODE *)NULL, Node_rule_node, yypvt[-0].nodeval); yyerrok; } break;
case 14:
# line 219 "awk.y"
{
		  yyval.nodeval = node (yypvt[-1].nodeval,
			     Node_rule_node,
			     node(node(node(make_number(0.0),
					    Node_field_spec,
					    (NODE *) NULL),
					Node_expression_list,
					(NODE *) NULL),
				  Node_K_print,
				  (NODE *) NULL));
		  yyerrok;
		} break;
case 15:
# line 232 "awk.y"
{
			func_install(yypvt[-1].nodeval, yypvt[-0].nodeval);
			yyval.nodeval = NULL;
			yyerrok;
		} break;
case 16:
# line 241 "awk.y"
{ yyval.sval = yypvt[-0].sval; } break;
case 17:
# line 243 "awk.y"
{ yyval.sval = yypvt[-0].sval; } break;
case 18:
# line 245 "awk.y"
{
		yyerror("%s() is a built-in function, it cannot be redefined",
			tokstart);
		errcount++;
		/* yyerrok; */
	  } break;
case 21:
# line 260 "awk.y"
{
			param_counter = 0;
		} break;
case 22:
# line 264 "awk.y"
{
			yyval.nodeval = append_right(make_param(yypvt[-4].sval), yypvt[-2].nodeval);
			can_return = 1;
		} break;
case 23:
# line 272 "awk.y"
{
		yyval.nodeval = yypvt[-2].nodeval;
		can_return = 0;
	  } break;
case 24:
# line 281 "awk.y"
{ yyval.nodeval = yypvt[-0].nodeval; } break;
case 25:
# line 283 "awk.y"
{ yyval.nodeval = mkrangenode ( node(yypvt[-2].nodeval, Node_cond_pair, yypvt[-0].nodeval) ); } break;
case 26:
# line 292 "awk.y"
{ ++want_regexp; } break;
case 27:
# line 294 "awk.y"
{
		  NODE *n;
		  int len;

		  getnode(n);
		  n->type = Node_regex;
		  len = strlen(yypvt[-1].sval);
		  n->re_exp = make_string(yypvt[-1].sval, len);
		  n->re_reg = make_regexp(yypvt[-1].sval, len, 0, 1);
		  n->re_text = NULL;
		  n->re_flags = CONST;
		  n->re_cnt = 1;
		  yyval.nodeval = n;
		} break;
case 28:
# line 312 "awk.y"
{ yyval.nodeval = yypvt[-3].nodeval ; } break;
case 29:
# line 314 "awk.y"
{ yyval.nodeval = NULL; } break;
case 30:
# line 319 "awk.y"
{ yyval.nodeval = yypvt[-0].nodeval; } break;
case 31:
# line 321 "awk.y"
{
			if (yypvt[-1].nodeval == NULL || yypvt[-1].nodeval->type != Node_statement_list)
				yypvt[-1].nodeval = node(yypvt[-1].nodeval, Node_statement_list,(NODE *)NULL);
	    		yyval.nodeval = append_right(yypvt[-1].nodeval,
				node( yypvt[-0].nodeval, Node_statement_list, (NODE *)NULL));
	    		yyerrok;
		} break;
case 32:
# line 329 "awk.y"
{ yyval.nodeval = NULL; } break;
case 33:
# line 331 "awk.y"
{ yyval.nodeval = NULL; } break;
case 36:
# line 341 "awk.y"
{ yyval.nodeval = NULL; } break;
case 37:
# line 343 "awk.y"
{ yyval.nodeval = NULL; } break;
case 38:
# line 345 "awk.y"
{ yyval.nodeval = yypvt[-1].nodeval; } break;
case 39:
# line 347 "awk.y"
{ yyval.nodeval = yypvt[-0].nodeval; } break;
case 40:
# line 349 "awk.y"
{ yyval.nodeval = node (yypvt[-3].nodeval, Node_K_while, yypvt[-0].nodeval); } break;
case 41:
# line 351 "awk.y"
{ yyval.nodeval = node (yypvt[-2].nodeval, Node_K_do, yypvt[-5].nodeval); } break;
case 42:
# line 353 "awk.y"
{
		yyval.nodeval = node (yypvt[-0].nodeval, Node_K_arrayfor, make_for_loop(variable(yypvt[-5].sval,1),
			(NODE *)NULL, variable(yypvt[-3].sval,1)));
	  } break;
case 43:
# line 358 "awk.y"
{
		yyval.nodeval = node(yypvt[-0].nodeval, Node_K_for, (NODE *)make_for_loop(yypvt[-7].nodeval, yypvt[-5].nodeval, yypvt[-3].nodeval));
	  } break;
case 44:
# line 362 "awk.y"
{
		yyval.nodeval = node (yypvt[-0].nodeval, Node_K_for,
			(NODE *)make_for_loop(yypvt[-6].nodeval, (NODE *)NULL, yypvt[-3].nodeval));
	  } break;
case 45:
# line 368 "awk.y"
{ yyval.nodeval = node ((NODE *)NULL, Node_K_break, (NODE *)NULL); } break;
case 46:
# line 371 "awk.y"
{ yyval.nodeval = node ((NODE *)NULL, Node_K_continue, (NODE *)NULL); } break;
case 47:
# line 373 "awk.y"
{ yyval.nodeval = node (yypvt[-3].nodeval, yypvt[-5].nodetypeval, yypvt[-1].nodeval); } break;
case 48:
# line 375 "awk.y"
{
			if (yypvt[-3].nodetypeval == Node_K_print && yypvt[-2].nodeval == NULL)
				yypvt[-2].nodeval = node(node(make_number(0.0),
					       Node_field_spec,
					       (NODE *) NULL),
					  Node_expression_list,
					  (NODE *) NULL);

			yyval.nodeval = node (yypvt[-2].nodeval, yypvt[-3].nodetypeval, yypvt[-1].nodeval);
		} break;
case 49:
# line 386 "awk.y"
{ NODETYPE type;

		  if (yypvt[-1].nodeval && yypvt[-1].nodeval == lookup("file")) {
			if (do_lint)
				warning("`next file' is a gawk extension");
			else if (strict || do_posix)
				yyerror("`next file' is a gawk extension");
		  	else if (! io_allowed)
				yyerror("`next file' used in BEGIN or END action");
			type = Node_K_nextfile;
		  } else {
		  	if (! io_allowed)
				yyerror("next used in BEGIN or END action");
			type = Node_K_next;
		  }
		  yyval.nodeval = node ((NODE *)NULL, type, (NODE *)NULL);
		} break;
case 50:
# line 404 "awk.y"
{ yyval.nodeval = node (yypvt[-1].nodeval, Node_K_exit, (NODE *)NULL); } break;
case 51:
# line 406 "awk.y"
{ if (! can_return) yyerror("return used outside function context"); } break;
case 52:
# line 408 "awk.y"
{ yyval.nodeval = node (yypvt[-1].nodeval, Node_K_return, (NODE *)NULL); } break;
case 53:
# line 410 "awk.y"
{ yyval.nodeval = node (variable(yypvt[-4].sval,1), Node_K_delete, yypvt[-2].nodeval); } break;
case 54:
# line 412 "awk.y"
{ yyval.nodeval = yypvt[-1].nodeval; } break;
case 55:
# line 417 "awk.y"
{ yyval.nodetypeval = yypvt[-0].nodetypeval; } break;
case 56:
# line 419 "awk.y"
{ yyval.nodetypeval = yypvt[-0].nodetypeval; } break;
case 57:
# line 424 "awk.y"
{
		yyval.nodeval = node(yypvt[-3].nodeval, Node_K_if, 
			node(yypvt[-0].nodeval, Node_if_branches, (NODE *)NULL));
	  } break;
case 58:
# line 430 "awk.y"
{ yyval.nodeval = node (yypvt[-6].nodeval, Node_K_if,
				node (yypvt[-3].nodeval, Node_if_branches, yypvt[-0].nodeval)); } break;
case 59:
# line 436 "awk.y"
{ want_assign = 0; } break;
case 63:
# line 447 "awk.y"
{ yyval.nodeval = NULL; } break;
case 64:
# line 449 "awk.y"
{ yyval.nodeval = node (yypvt[-0].nodeval, Node_redirect_input, (NODE *)NULL); } break;
case 65:
# line 454 "awk.y"
{ yyval.nodeval = NULL; } break;
case 66:
# line 456 "awk.y"
{ yyval.nodeval = node (yypvt[-0].nodeval, Node_redirect_output, (NODE *)NULL); } break;
case 67:
# line 458 "awk.y"
{ yyval.nodeval = node (yypvt[-0].nodeval, Node_redirect_append, (NODE *)NULL); } break;
case 68:
# line 460 "awk.y"
{ yyval.nodeval = node (yypvt[-0].nodeval, Node_redirect_pipe, (NODE *)NULL); } break;
case 69:
# line 465 "awk.y"
{ yyval.nodeval = NULL; } break;
case 70:
# line 467 "awk.y"
{ yyval.nodeval = yypvt[-0].nodeval; } break;
case 71:
# line 472 "awk.y"
{ yyval.nodeval = make_param(yypvt[-0].sval); } break;
case 72:
# line 474 "awk.y"
{ yyval.nodeval = append_right(yypvt[-2].nodeval, make_param(yypvt[-0].sval)); yyerrok; } break;
case 73:
# line 476 "awk.y"
{ yyval.nodeval = NULL; } break;
case 74:
# line 478 "awk.y"
{ yyval.nodeval = NULL; } break;
case 75:
# line 480 "awk.y"
{ yyval.nodeval = NULL; } break;
case 76:
# line 486 "awk.y"
{ yyval.nodeval = NULL; } break;
case 77:
# line 488 "awk.y"
{ yyval.nodeval = yypvt[-0].nodeval; } break;
case 78:
# line 493 "awk.y"
{ yyval.nodeval = NULL; } break;
case 79:
# line 495 "awk.y"
{ yyval.nodeval = yypvt[-0].nodeval; } break;
case 80:
# line 500 "awk.y"
{ yyval.nodeval = node (yypvt[-0].nodeval, Node_expression_list, (NODE *)NULL); } break;
case 81:
# line 502 "awk.y"
{
		yyval.nodeval = append_right(yypvt[-2].nodeval,
			node( yypvt[-0].nodeval, Node_expression_list, (NODE *)NULL));
		yyerrok;
	  } break;
case 82:
# line 508 "awk.y"
{ yyval.nodeval = NULL; } break;
case 83:
# line 510 "awk.y"
{ yyval.nodeval = NULL; } break;
case 84:
# line 512 "awk.y"
{ yyval.nodeval = NULL; } break;
case 85:
# line 514 "awk.y"
{ yyval.nodeval = NULL; } break;
case 86:
# line 519 "awk.y"
{ yyval.nodeval = NULL; } break;
case 87:
# line 521 "awk.y"
{ yyval.nodeval = yypvt[-0].nodeval; } break;
case 88:
# line 526 "awk.y"
{ yyval.nodeval = node (yypvt[-0].nodeval, Node_expression_list, (NODE *)NULL); } break;
case 89:
# line 528 "awk.y"
{
			yyval.nodeval = append_right(yypvt[-2].nodeval,
				node( yypvt[-0].nodeval, Node_expression_list, (NODE *)NULL));
			yyerrok;
		} break;
case 90:
# line 534 "awk.y"
{ yyval.nodeval = NULL; } break;
case 91:
# line 536 "awk.y"
{ yyval.nodeval = NULL; } break;
case 92:
# line 538 "awk.y"
{ yyval.nodeval = NULL; } break;
case 93:
# line 540 "awk.y"
{ yyval.nodeval = NULL; } break;
case 94:
# line 545 "awk.y"
{ want_assign = 0; } break;
case 95:
# line 547 "awk.y"
{
		  if (do_lint && yypvt[-0].nodeval->type == Node_regex)
			warning("Regular expression on left of assignment.");
		  yyval.nodeval = node (yypvt[-3].nodeval, yypvt[-2].nodetypeval, yypvt[-0].nodeval);
		} break;
case 96:
# line 553 "awk.y"
{ yyval.nodeval = node (variable(yypvt[-0].sval,1), Node_in_array, yypvt[-3].nodeval); } break;
case 97:
# line 555 "awk.y"
{
		  yyval.nodeval = node (yypvt[-0].nodeval, Node_K_getline,
			 node (yypvt[-3].nodeval, Node_redirect_pipein, (NODE *)NULL));
		} break;
case 98:
# line 560 "awk.y"
{
		  if (do_lint && ! io_allowed && yypvt[-0].nodeval == NULL)
			warning("non-redirected getline undefined inside BEGIN or END action");
		  yyval.nodeval = node (yypvt[-1].nodeval, Node_K_getline, yypvt[-0].nodeval);
		} break;
case 99:
# line 566 "awk.y"
{ yyval.nodeval = node (yypvt[-2].nodeval, Node_and, yypvt[-0].nodeval); } break;
case 100:
# line 568 "awk.y"
{ yyval.nodeval = node (yypvt[-2].nodeval, Node_or, yypvt[-0].nodeval); } break;
case 101:
# line 570 "awk.y"
{
		  if (yypvt[-2].nodeval->type == Node_regex)
			warning("Regular expression on left of MATCH operator.");
		  yyval.nodeval = node (yypvt[-2].nodeval, yypvt[-1].nodetypeval, mk_rexp(yypvt[-0].nodeval));
		} break;
case 102:
# line 576 "awk.y"
{ yyval.nodeval = yypvt[-0].nodeval; } break;
case 103:
# line 578 "awk.y"
{
		  yyval.nodeval = node(node(make_number(0.0),
				 Node_field_spec,
				 (NODE *) NULL),
		            Node_nomatch,
			    yypvt[-0].nodeval);
		} break;
case 104:
# line 586 "awk.y"
{ yyval.nodeval = node (variable(yypvt[-0].sval,1), Node_in_array, yypvt[-2].nodeval); } break;
case 105:
# line 588 "awk.y"
{
		  if (do_lint && yypvt[-0].nodeval->type == Node_regex)
			warning("Regular expression on left of comparison.");
		  yyval.nodeval = node (yypvt[-2].nodeval, yypvt[-1].nodetypeval, yypvt[-0].nodeval);
		} break;
case 106:
# line 594 "awk.y"
{ yyval.nodeval = node (yypvt[-2].nodeval, Node_less, yypvt[-0].nodeval); } break;
case 107:
# line 596 "awk.y"
{ yyval.nodeval = node (yypvt[-2].nodeval, Node_greater, yypvt[-0].nodeval); } break;
case 108:
# line 598 "awk.y"
{ yyval.nodeval = node(yypvt[-4].nodeval, Node_cond_exp, node(yypvt[-2].nodeval, Node_if_branches, yypvt[-0].nodeval));} break;
case 109:
# line 600 "awk.y"
{ yyval.nodeval = yypvt[-0].nodeval; } break;
case 110:
# line 602 "awk.y"
{ yyval.nodeval = node (yypvt[-1].nodeval, Node_concat, yypvt[-0].nodeval); } break;
case 111:
# line 607 "awk.y"
{ want_assign = 0; } break;
case 112:
# line 609 "awk.y"
{ yyval.nodeval = node (yypvt[-3].nodeval, yypvt[-2].nodetypeval, yypvt[-0].nodeval); } break;
case 113:
# line 611 "awk.y"
{ yyval.nodeval = node (yypvt[-2].nodeval, Node_and, yypvt[-0].nodeval); } break;
case 114:
# line 613 "awk.y"
{ yyval.nodeval = node (yypvt[-2].nodeval, Node_or, yypvt[-0].nodeval); } break;
case 115:
# line 615 "awk.y"
{
		  if (do_lint && ! io_allowed && yypvt[-0].nodeval == NULL)
			warning("non-redirected getline undefined inside BEGIN or END action");
		  yyval.nodeval = node (yypvt[-1].nodeval, Node_K_getline, yypvt[-0].nodeval);
		} break;
case 116:
# line 621 "awk.y"
{ yyval.nodeval = yypvt[-0].nodeval; } break;
case 117:
# line 623 "awk.y"
{ yyval.nodeval = node((NODE *) NULL, Node_nomatch, yypvt[-0].nodeval); } break;
case 118:
# line 625 "awk.y"
{ yyval.nodeval = node (yypvt[-2].nodeval, yypvt[-1].nodetypeval, mk_rexp(yypvt[-0].nodeval)); } break;
case 119:
# line 627 "awk.y"
{ yyval.nodeval = node (variable(yypvt[-0].sval,1), Node_in_array, yypvt[-2].nodeval); } break;
case 120:
# line 629 "awk.y"
{ yyval.nodeval = node (yypvt[-2].nodeval, yypvt[-1].nodetypeval, yypvt[-0].nodeval); } break;
case 121:
# line 631 "awk.y"
{ yyval.nodeval = node(yypvt[-4].nodeval, Node_cond_exp, node(yypvt[-2].nodeval, Node_if_branches, yypvt[-0].nodeval));} break;
case 122:
# line 633 "awk.y"
{ yyval.nodeval = yypvt[-0].nodeval; } break;
case 123:
# line 635 "awk.y"
{ yyval.nodeval = node (yypvt[-1].nodeval, Node_concat, yypvt[-0].nodeval); } break;
case 125:
# line 642 "awk.y"
{ yyval.nodeval = node (yypvt[-2].nodeval, Node_exp, yypvt[-0].nodeval); } break;
case 126:
# line 644 "awk.y"
{ yyval.nodeval = node (yypvt[-2].nodeval, Node_times, yypvt[-0].nodeval); } break;
case 127:
# line 646 "awk.y"
{ yyval.nodeval = node (yypvt[-2].nodeval, Node_quotient, yypvt[-0].nodeval); } break;
case 128:
# line 648 "awk.y"
{ yyval.nodeval = node (yypvt[-2].nodeval, Node_mod, yypvt[-0].nodeval); } break;
case 129:
# line 650 "awk.y"
{ yyval.nodeval = node (yypvt[-2].nodeval, Node_plus, yypvt[-0].nodeval); } break;
case 130:
# line 652 "awk.y"
{ yyval.nodeval = node (yypvt[-2].nodeval, Node_minus, yypvt[-0].nodeval); } break;
case 131:
# line 654 "awk.y"
{ yyval.nodeval = node (yypvt[-1].nodeval, Node_postincrement, (NODE *)NULL); } break;
case 132:
# line 656 "awk.y"
{ yyval.nodeval = node (yypvt[-1].nodeval, Node_postdecrement, (NODE *)NULL); } break;
case 133:
# line 661 "awk.y"
{ yyval.nodeval = node (yypvt[-0].nodeval, Node_not,(NODE *) NULL); } break;
case 134:
# line 663 "awk.y"
{ yyval.nodeval = yypvt[-1].nodeval; } break;
case 135:
# line 666 "awk.y"
{ yyval.nodeval = snode (yypvt[-1].nodeval, Node_builtin, (int) yypvt[-3].lval); } break;
case 136:
# line 668 "awk.y"
{ yyval.nodeval = snode (yypvt[-1].nodeval, Node_builtin, (int) yypvt[-3].lval); } break;
case 137:
# line 670 "awk.y"
{
		if (do_lint)
			warning("call of `length' without parentheses is not portable");
		yyval.nodeval = snode ((NODE *)NULL, Node_builtin, (int) yypvt[-0].lval);
		if (do_posix)
			warning( "call of `length' without parentheses is deprecated by POSIX");
	  } break;
case 138:
# line 678 "awk.y"
{
		yyval.nodeval = node (yypvt[-1].nodeval, Node_func_call, make_string(yypvt[-3].sval, strlen(yypvt[-3].sval)));
	  } break;
case 140:
# line 683 "awk.y"
{ yyval.nodeval = node (yypvt[-0].nodeval, Node_preincrement, (NODE *)NULL); } break;
case 141:
# line 685 "awk.y"
{ yyval.nodeval = node (yypvt[-0].nodeval, Node_predecrement, (NODE *)NULL); } break;
case 142:
# line 687 "awk.y"
{ yyval.nodeval = yypvt[-0].nodeval; } break;
case 143:
# line 689 "awk.y"
{ yyval.nodeval = yypvt[-0].nodeval; } break;
case 144:
# line 692 "awk.y"
{ if (yypvt[-0].nodeval->type == Node_val) {
			yypvt[-0].nodeval->numbr = -(force_number(yypvt[-0].nodeval));
			yyval.nodeval = yypvt[-0].nodeval;
		  } else
			yyval.nodeval = node (yypvt[-0].nodeval, Node_unary_minus, (NODE *)NULL);
		} break;
case 145:
# line 699 "awk.y"
{ yyval.nodeval = yypvt[-0].nodeval; } break;
case 146:
# line 704 "awk.y"
{ yyval.nodeval = NULL; } break;
case 147:
# line 706 "awk.y"
{ yyval.nodeval = yypvt[-0].nodeval; } break;
case 148:
# line 711 "awk.y"
{ yyval.nodeval = variable(yypvt[-0].sval,1); } break;
case 149:
# line 713 "awk.y"
{
		if (yypvt[-1].nodeval->rnode == NULL) {
			yyval.nodeval = node (variable(yypvt[-3].sval,1), Node_subscript, yypvt[-1].nodeval->lnode);
			freenode(yypvt[-1].nodeval);
		} else
			yyval.nodeval = node (variable(yypvt[-3].sval,1), Node_subscript, yypvt[-1].nodeval);
		} break;
case 150:
# line 721 "awk.y"
{ yyval.nodeval = node (yypvt[-0].nodeval, Node_field_spec, (NODE *)NULL); } break;
case 152:
# line 729 "awk.y"
{ yyerrok; } break;
case 153:
# line 733 "awk.y"
{ yyerrok; } break;
case 156:
# line 742 "awk.y"
{ yyerrok; want_assign = 0; } break;
case 157:
# line 745 "awk.y"
{ yyerrok; } break;
}


	goto yystack;		/* reset registers in driver code */
}
