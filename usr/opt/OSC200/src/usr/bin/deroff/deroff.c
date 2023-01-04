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
static char	*sccsid = "@(#)$RCSfile: deroff.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/10/11 16:11:31 $";
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

#if !defined(lint) && !defined(_NOIDENT)

#endif

/*
 * COMPONENT_NAME: (CMDTEXT) Text Formatting Services
 *
 * FUNCTIONS:
 *
 * ORIGINS: 26, 27, 28
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * deroff.c	1.7  com/bsd.d/deroff.d,3.1,9021 4/2/90 15:25:38
 */

/* deroff.c	4.5	(Berkeley)	84/12/18 */

#include <stdio.h>

#include <locale.h>


#include "deroff_msg.h" 
#include "nl_types.h"
nl_catd	catd;
#define MSGSTR(n,s) catgets(catd,MS_DEROFF,n,s)


/*
 * Deroff command -- strip troff, eqn, and Tbl sequences from a file.  Has
 * two flags argument, -w, to cause output one word per line rather than in
 * the original format. Deroff follows .so and .nx commands, removes contents
 * of macro definitions, equations (both .EQ ... .EN and $...$), Tbl command
 * sequences, and Troff backslash constructions. Flag arguments: -w, to cause
 * output one word per line instead of original format -u  Gets rid of "_\b"
 * and "\b" in under-line and bold-face words. If -u is specified it
 * automaticly sets -w flag. -mm (or -ms) causes the corresponding macro's to
 * be interpreted so that just sentences are output -ml also gets rid of
 * lists. -i  suppress file inclusion by .so/.nx -l  suppress .so/.nx
 * inclusion if file name begins with "/usr/lib" 
 *
 * All input is through the Cget macro; the most recently read character is in
 * c. 
 *
 * Modified by Robert Henry to process -me and -man macros. 
 */

#define Cget ( (c=getc(infile)) == EOF ? eof() : ((c==ldelim)&&(filesp==files) ? skeqn() :( c == '\n'?(linect++,c):c) ))
/* #define Cget ( (c=getc(infile)) == EOF ? eof() : */
/* ((c==ldelim)&&(filesp==files) ? skeqn() : c) )   */
#define C1get ( (c=getc(infile)) == EOF ? eof() : (c == '\n' ? (linect++,c): c))

#ifdef DEBUG
#  define C	_C0()
#  define C1	_C1()
#else not DEBUG
#  define C	Cget
#  define C1	C1get
#endif not DEBUG

#define SKIP while(C != '\n')
#define SKIP_TO_COM SKIP; SKIP; pc=c; while(C != '.' || pc != '\n' || C > 'Z')pc=c

#define	YES 1
#define	NO 0
#define	MS 0			/* -ms */
#define	MM 1			/* -mm */
#define	ME 2			/* -me */
#define	MA 3			/* -man */

#ifdef DEBUG
char           *mactab[] = {
			    "-ms", "-mm", "-me", "-ma"};
#endif DEBUG

#define	ONE 1
#define	TWO 2

#define NOCHAR -2
#define SPECIAL 0
#define APOS 1
#define PUNCT 2
#define DIGIT 3
#define LETTER 4
#define	UNDERL 1

static char     libname[] = "/usr/lib";
static int      libchars = sizeof (libname) - 1;


int             linect = 0;
int             numflag = 0;
int             iflag;
int             lflag;
int             uflag;
int             wordflag;
int             msflag;		/* processing a source written using a mac
				 * package */
int             mac;		/* which package */
int             disp;
int             parag;
int             inmacro;
int             intable;
int             keepblock;	/* keep blocks of text; normally false when
				 * msflag */

char            chars[256];	/* SPECIAL, PUNCT, APOS, DIGIT, or LETTER */

char            line[512];
char           *lp;

int             c;
int             pc;
int             ldelim;
int             rdelim;

char           *calloc ();

int             argc;
char          **argv;

char            fname[50];
FILE           *files[15];
FILE          **filesp;
FILE           *infile;
FILE           *opn ();
/*
 * Flags for matching conditions other than the macro name 
 */
#define	NONE		0
#define	FNEST		1	/* no nested files */
#define	NOMAC		2	/* no macro */
#define	MAC		3	/* macro */
#define	PARAG		4	/* in a paragraph */
#define	MSF		5	/* msflag is on */
#define	NBLK		6	/* set if no blocks to be kept */

/*
 * Return codes from macro minions, determine where to jump, how to
 * repeat/reprocess text 
 */
#define	COMX		1	/* goto comx */
#define	COM		2	/* goto com */


main (ac, av)
    int             ac;
    char          **av;
{
    register int    i;
    int             errflg = 0;
    register        optchar;
    FILE           *opn ();
    int             kflag = NO;
    char           *p;

    setlocale(LC_ALL, "");
    wordflag = NO;
    msflag = NO;
    mac = ME;
    disp = NO;
    parag = NO;
    inmacro = NO;
    intable = NO;
    ldelim = NOCHAR;
    rdelim = NOCHAR;
    keepblock = YES;

    catd=catopen(MF_DEROFF,NL_CAT_LOCALE);

    for (argc = ac - 1, argv = av + 1;
	 ((argc > 0)
	  && (argv[0][0] == '-')
	  && (argv[0][1] != '\0'));
	 --argc, ++argv
	)
    {
	for (p = argv[0] + 1; *p; ++p)
	{
	    switch (*p)
	    {
	    case 'p':
		parag = YES;
		break;
	    case 'n':
		numflag = 1;
		break;
	    case 'k':
		kflag = YES;
		break;
	    case 'w':
		wordflag = YES;
		kflag = YES;
		break;
	    case 'i':
		iflag = YES;
		break;
	    case 'l':
		lflag = YES;
		break;
	    case 'u':
		uflag = YES;
		wordflag = YES;
		break;
	    case 'm':
		msflag = YES;
		keepblock = NO;
		switch (p[1])
		{
		case 'm':
		    mac = MM;
		    p++;
		    break;
		case 's':
		    mac = MS;
		    p++;
		    break;
		case 'e':
		    mac = ME;
		    p++;
		    break;
		case 'a':
		    mac = MA;
		    p++;
		    break;
		case 'l':
		    disp = YES;
		    p++;
		    break;
		default:
		    errflg++;
		    break;
		}
		break;
	    default:
		errflg++;
	    }
	}
    }

    if (kflag)
	keepblock = YES;
    if (errflg)
	fatal(MSGSTR(USAGE, "usage: deroff [-w] [-i] [-k] [-p] [-l] [-u] [-m (a e m s l)] [file] ...\n"));

#ifdef DEBUG
    printf ("msflag = %d, mac = %s, keepblock = %d, disp = %d\n",
	    msflag, mactab[mac], keepblock, disp);
#endif DEBUG
    if (argc == 0)
    {
	infile = stdin;
    } else
    {
	infile = opn (argv[0]);
	--argc;
	++argv;
    }


    files[0] = infile;
    filesp = &files[0];

    for (i = 'a'; i <= 'z'; ++i)
	chars[i] = LETTER;
    for (i = 'A'; i <= 'Z'; ++i)
	chars[i] = LETTER;
    for (i = '0'; i <= '9'; ++i)
	chars[i] = DIGIT;
    chars['\''] = APOS;
    chars['&'] = APOS;
    chars['.'] = PUNCT;
    chars[','] = PUNCT;
    chars[';'] = PUNCT;
    chars['?'] = PUNCT;
    chars[':'] = PUNCT;
    chars[168] = PUNCT;  /* spanish question mark */
    chars[173] = PUNCT;  /* spanish exclamation point */
    if (uflag)
    {
	chars['_'] = UNDERL;
	chars['\b'] = UNDERL;
    }                             /* NLS LETTERS */ 
    for (i = 128; i <= 154; ++i) 
	chars[i] = LETTER;
    for (i = 160; i<= 167; ++i)
        chars[i] = LETTER;
    for (i = 181; i <= 183; ++i)
        chars[i] = LETTER;
    for (i = 198; i <= 199; ++i)
        chars[i] = LETTER;
    for (i = 209; i <= 216; ++i)
        chars[i] = LETTER;
    chars[222] = LETTER;
    for (i = 224; i <= 237; ++i)
        chars[i] = LETTER;
    work ();
}


skeqn ()
{
    while ((c = getc (infile)) != rdelim)
	if (c == '\n')
	    linect++;
	else
	if (c == EOF)
	    c = eof ();
	else
	if (c == '"')
	    while ((c = getc (infile)) != '"')
		if (c == '\n')
		    linect++;
		else
		if (c == EOF)
		    c = eof ();
		else
		if (c == '\\')
		{
		    if ((c = getc (infile)) == EOF)
			c = eof ();
		    else
		    if (c == '\n')
			/* code folded from here */
			linect++;
		    /* unfolding */
		}
    if (msflag)
	return (c = 'x');
    return (c = ' ');
}


FILE           *
opn (p)
    register char  *p;
{
    FILE           *fd;

    if ((fd = fopen (p, "r")) == NULL)
    {
	fprintf (stderr, MSGSTR(DEROFF,"deroff: "));
	perror (p);
	exit (1);
    }
    linect = 0;
    if (numflag)
	printf (".F %s\n", p);

    return (fd);
}

eof ()
{
    if (infile != stdin)
	fclose (infile);
    if (filesp > files)
	infile = *--filesp;
    else
    if (argc > 0)
    {
	infile = opn (argv[0]);
	--argc;
	++argv;
    } else
	exit (0);
    return (C);
}

getfname ()
{
    register char  *p;
    struct chain
    {
	struct chain   *nextp;
	char           *datap;
    }              *chainblock;
    register struct chain *q;
    static struct chain *namechain = NULL;
    char           *copys ();

    while (C == ' ');

    for (p = fname; (*p = c) != '\n' && c != ' ' && c != '\t' && c != '\\'; ++p)
	C;
    *p = '\0';
    while (c != '\n')
	C;

    /* see if this name has already been used */

    for (q = namechain; q; q = q -> nextp)
	if (!strcmp (fname, q -> datap))
	{
	    fname[0] = '\0';
	    return;
	}
    q = (struct chain *) calloc (1, sizeof (*chainblock));
    q -> nextp = namechain;
    q -> datap = copys (fname);
    namechain = q;
}

fatal (s)
    char           *s;
{
    fprintf (stderr, "%s: %s", MSGSTR(DEROFF,"deroff: "), s);
    exit (1);
}

/* ARGSUSED */
textline (str, Const)
    char           *str;
    int             Const;
{
    if (wordflag)
    {
	msputwords (0);
	return;
    }
    puts (str);
}

work ()
{
    for (;;)
    {
	C;
#ifdef FULLDEBUG
	printf ("Starting work with `%c'\n", c);
#endif FULLDEBUG
	if (c == '.' || c == '\'')
	    comline ();
	else
	    regline (textline, TWO);
    }
}

regline (pfunc, Const)
    int             (*pfunc) ();
int             Const;
{
    line[0] = c;
    lp = line;
    for (;;)
    {
	if (c == '\\')
	{
	    *lp = ' ';
	    backsl ();
	}
	if (c == '\n')
	    break;
	if (intable && c == 'T')
	{
	    *++lp = C;
	    if (c == '{' || c == '}')
	    {
		lp[-1] = ' ';
		*lp = C;
	    }
	} else
	{
	    *++lp = C;
	}
    }

    *lp = '\0';

    if (line[0] != '\0')
	(*pfunc) (line, Const);
}

macro ()
{
    if (msflag)
    {
	do
	{
	    SKIP;
	} while (C != '.' || C != '.' || C == '.');	/* look for  .. */
	if (c != '\n')
	    SKIP;
	return;
    }
    SKIP;
    inmacro = YES;
}

tbl ()
{
    while (C != '.');
    SKIP;
    intable = YES;
}
stbl ()
{
    while (C != '.');
    SKIP_TO_COM;
    if (c != 'T' || C != 'E')
    {
	SKIP;
	pc = c;
	while (C != '.' || pc != '\n' || C != 'T' || C != 'E')
	    pc = c;
    }
}

eqn ()
{
    register int    c1,
                    c2;
    register int    dflg;
    char            last;

    last = 0;
    dflg = 1;
    SKIP;

    for (;;)
    {
	if (C1 == '.' || c == '\'')
	{
	    while (C1 == ' ' || c == '\t')
		;
	    if (c == 'E' && C1 == 'N')
	    {
		SKIP;
		if (msflag && dflg)
		{
		    putchar ('x');
		    putchar (' ');
		    if (last)
		    {
			putchar (last);
			putchar ('\n');
		    }
		}
		return;
	    }
	} else
	if (c == 'd')		/* look for delim */
	{
	    if (C1 == 'e' && C1 == 'l')
		if (C1 == 'i' && C1 == 'm')
		{
		    while (C1 == ' ');
		    if ((c1 = c) == '\n' || (c2 = C1) == '\n'
			|| (c1 == 'o' && c2 == 'f' && C1 == 'f'))
		    {
			ldelim = NOCHAR;
			rdelim = NOCHAR;
		    } else
		    {
			ldelim = c1;
			rdelim = c2;
		    }
		}
	    dflg = 0;
	}
	if (c != '\n')
	    while (C1 != '\n')
	    {
		if (chars[c] == PUNCT)
		    last = c;
		else
		if (c != ' ')
		    last = 0;
	    }
    }
}

backsl ()			/* skip over a complete backslash
				 * construction */
{
    int             bdelim;

sw:
    switch (C)
    {
    case '"':
	SKIP;
	return;
    case 's':
	if (C == '\\')
	    backsl ();
	else
	{
	    while (C >= '0' && c <= '9');
	    ungetc (c, infile);
	    c = '0';
	}
	--lp;
	return;

    case 'f':
    case 'n':
    case '*':
	if (C != '(')
	    return;

    case '(':
	if (msflag)
	{
	    if (C == 'e')
	    {
		if (C == 'm')
		{
		    *lp = '-';
		    return;
		}
	    } else
	    if (c != '\n')
		C;
	    return;
	}
	if (C != '\n')
	    C;
	return;

    case '$':
	C;			/* discard argument number */
	return;

    case 'b':
    case 'x':
    case 'v':
    case 'h':
    case 'w':
    case 'o':
    case 'l':
    case 'L':
	if ((bdelim = C) == '\n')
	    return;
	while (C != '\n' && c != bdelim)
	    if (c == '\\')
		backsl ();
	return;

    case '\\':
	if (inmacro)
	    goto sw;
    default:
	return;
    }
}

char           *
copys (s)
    register char  *s;
{
    register char  *t,
                   *t0;

    if ((t0 = t = calloc ((unsigned) (strlen (s) + 1), sizeof (*t))) == NULL)
	fatal(MSGSTR(NOMEM,"Not enough memory available."));

    while (*t++ = *s++)
	;
    return (t0);
}

sce ()
{
    register char  *ap;
    register int    n,
                    i;
    char            a[10];
    for (ap = a; C != '\n'; ap++)
    {
	*ap = c;
	if (ap == &a[9])
	{
	    SKIP;
	    ap = a;
	    break;
	}
    }
    if (ap != a)
	n = atoi (a);
    else
	n = 1;
    for (i = 0; i < n;)
    {
	if (C == '.')
	{
	    if (C == 'c')
	    {
		if (C == 'e')
		{
		    while (C == ' ');
		    if (c == '0')
		    {
			SKIP;
			break;
		    } else
			SKIP;
		} else
		    SKIP;
	    } else
	    if (c == 'P' || C == 'P')
	    {
		if (c != '\n')
		    SKIP;
		break;
	    } else
	    if (c != '\n')
		SKIP;
	} else
	{
	    SKIP;
	    i++;
	}
    }
}

refer (c1)
{
    register int    c2;
    if (c1 != '\n')
	SKIP;
    while (1)
    {
	if (C != '.')
	    SKIP;
	else
	{
	    if (C != ']')
		SKIP;
	    else
	    {
		while (C != '\n')
		    c2 = c;
		if (chars[c2] == PUNCT)
		    putchar (c2);
		return;
	    }
	}
    }
}

inpic ()
{
    register int    c1;
    register char  *p1;
    SKIP;
    p1 = line;
    c = '\n';
    while (1)
    {
	c1 = c;
	if (C == '.' && c1 == '\n')
	{
	    if (C != 'P')
	    {
		if (c == '\n')
		    continue;
		else
		{
		    SKIP;
		    c = '\n';
		    continue;
		}
	    }
	    if (C != 'E')
	    {
		if (c == '\n')
		    continue;
		else
		{
		    SKIP;
		    c = '\n';
		    continue;
		}
	    }
	    SKIP;
	    return;
	} else
	if (c == '\"')
	{
	    while (C != '\"')
	    {
		if (c == '\\')
		{
		    if (C == '\"')
			continue;
		    ungetc (c, infile);
		    backsl ();
		} else
		    *p1++ = c;
	    }
	    *p1++ = ' ';
	} else
	if (c == '\n' && p1 != line)
	{
	    *p1 = '\0';
	    if (wordflag)
		msputwords (NO);
	    else
	    {
		puts (line);
		putchar ('\n');
	    }
	    p1 = line;
	}
    }
}

#ifdef DEBUG
_C1 ()
{
    return (C1get);
}
_C0 ()
{
    return (Cget);
}
#endif DEBUG


/*
 * Macro processing 
 *
 * Macro table definitions 
 */
#define	reg	register
typedef int     pacmac;		/* compressed macro name */
int             argconcat = 0;	/* concat arguments together (-me only) */

#define	tomac(c1, c2)		((((c1) & 0xFF) << 8) | ((c2) & 0xFF))
#define	frommac(src, c1, c2)	(((c1)=((src)>>8)&0xFF),((c2) =(src)&0xFF))

struct mactab
{
    int             condition;
    pacmac          macname;
    int             (*func) ();
};
struct mactab   troffmactab[];
struct mactab   ppmactab[];
struct mactab   msmactab[];
struct mactab   mmmactab[];
struct mactab   memactab[];
struct mactab   manmactab[];

/*
 * macro table initialization 
 */
#define	M(cond, c1, c2, func) {cond, tomac(c1, c2), func}

/*
 * Put out a macro line, using ms and mm conventions. 
 */
msputmac (s, Const)
    register char  *s;
    int             Const;
{
    register char  *t;
    register        found;
    int             last;
    found = 0;

    if (wordflag)
    {
	msputwords (YES);
	return;
    }
    while (*s)
    {
	while (*s == ' ' || *s == '\t')
	    putchar (*s++);
	for (t = s; *t != ' ' && *t != '\t' && *t != '\0'; ++t)
	    ;
	if (*s == '\"')
	    s++;
	if (t > s + Const && chars[s[0]] == LETTER && chars[s[1]] == LETTER)
	{
	    while (s < t)
		if (*s == '\"')
		    s++;
		else
		    putchar (*s++);
	    last = *(t - 1);
	    found++;
	} else
	if (found && chars[s[0]] == PUNCT && s[1] == '\0')
	    putchar (*s++);
	else
	{
	    last = *(t - 1);
	    s = t;
	}
    }
    putchar ('\n');
    if (msflag && chars[last] == PUNCT)
    {
	putchar (last);
	putchar ('\n');
    }
}
/*
 * put out words (for the -w option) with ms and mm conventions 
 */
msputwords (macline)
    int             macline;	/* is this is a macro line */
{
    register char  *p,
                   *p1;
    int             i,
                    nlet;

    for (p1 = line;;)
    {
	/*
	 * skip initial specials ampersands and apostrophes 
	 */
	while (chars[*p1] < DIGIT)
	    if (*p1++ == '\0')
		return;
	nlet = 0;
	for (p = p1; (i = chars[*p]) != SPECIAL; ++p)
	    if (i == LETTER)
		++nlet;

	if (nlet > 1 && chars[p1[0]] == LETTER)
	{
	    /*
	     * delete trailing ampersands and apostrophes 
	     */
	    while ((i = chars[p[-1]]) == PUNCT || i == APOS)
		--p;
	    for (; p1 < p; p1++)
	    {
		if (uflag)
		{
		    if (*p1 == '_')
		    {		/* underline? */
			p1++;
			if (*p1 != '\b')
			    break;
			continue;
		    } else
		    if (*p1 == '\b')
		    {
			p1++;	/* boldface? */
			if (*(p1 - 2) != *p1)
			    break;
			continue;
		    }
		}
		putchar (*p1);
	    }
	    putchar ('\n');
	} else
	{
	    p1 = p;
	}
    }
}
/*
 * put out a macro using the me conventions 
 */
#define SKIPBLANK(cp)	while(*cp == ' ' || *cp == '\t') { cp++; }
#define SKIPNONBLANK(cp) while(*cp !=' ' && *cp !='\cp' && *cp !='\0') { cp++; }

meputmac (cp, Const)
    reg char       *cp;
    int             Const;
{
    reg char       *np;
    int             found;
    int             argno;
    int             last;
    int             inquote;

    if (wordflag)
    {
	meputwords (YES);
	return;
    }
    for (argno = 0; *cp; argno++)
    {
	SKIPBLANK (cp);
	inquote = (*cp == '"');
	if (inquote)
	    cp++;
	for (np = cp; *np; np++)
	{
	    switch (*np)
	    {
	    case '\n':
	    case '\0':
		break;
	    case '\t':
	    case ' ':
		if (inquote)
		{
		    continue;
		} else
		{
		    goto endarg;
		}
	    case '"':
		if (inquote && np[1] == '"')
		{
		    strcpy (np, np + 1);
		    np++;
		    continue;
		} else
		{
		    *np = ' ';	/* bye bye " */
		    goto endarg;
		}
	    default:
		continue;
	    }
	}
endarg:
	;
	/*
	 * cp points at the first char in the arg np points one beyond the
	 * last char in the arg 
	 */
	if ((argconcat == 0) || (argconcat != argno))
	{
	    putchar (' ');
	}
#ifdef FULLDEBUG
	{
	    char           *p;
	    printf ("[%d,%d: ", argno, np - cp);
	    for (p = cp; p < np; p++)
	    {
		putchar (*p);
	    }
	    printf ("]");
	}
#endif FULLDEBUG
	/*
	 * Determine if the argument merits being printed 
	 *
	 * Const is the cut off point below which something is not a word. 
	 */
	if (((np - cp) > Const) && (inquote || (chars[cp[0]] == LETTER)))
	{
	    for (cp = cp; cp < np; cp++)
	    {
		putchar (*cp);
	    }
	    last = np[-1];
	    found++;
	} else
	if (found && (np - cp == 1) && chars[*cp] == PUNCT)
	    putchar (*cp);
	else
	    last = np[-1];
	cp = np;
    }
    if (msflag && chars[last] == PUNCT)
	putchar (last);
    putchar ('\n');
}

/*
 * put out words (for the -w option) with ms and mm conventions 
 */
meputwords (macline)
    int             macline;
{
    msputwords (macline);
}
/* 
 * Skip over a nested set of macros 
 *
 * Possible arguments to noblock are: 
 *
 * fi	end of unfilled text PE	pic ending DE	display ending 
 *
 * for ms and mm only: KE	keep ending 
 *
 * NE	undocumented match to NS (for mm?) LE	mm only: matches RL or *L
 * (for lists) 
 *
 * for me: ([lqbzcdf] */

noblock (a1, a2)
    char            a1,
                    a2;
{
    register int    c1,
                    c2;
    register int    eqnf;
    int             lct;
    lct = 0;
    eqnf = 1;
    SKIP;
    while (1)
    {
	while (C != '.')
	    if (c == '\n')
		continue;
	    else
		SKIP;
	if ((c1 = C) == '\n')
	    continue;
	if ((c2 = C) == '\n')
	    continue;
	if (c1 == a1 && c2 == a2)
	{
	    SKIP;
	    if (lct != 0)
	    {
		lct--;
		continue;
	    }
	    if (eqnf)
		putchar ('.');
	    putchar ('\n');
	    return;
	} else
	if (a1 == 'L' && c2 == 'L')
	{
	    lct++;
	    SKIP;
	}
	/*
	 * equations (EQ) nested within a display 
	 */
	else
	if (c1 == 'E' && c2 == 'Q')
	{
	    if ((mac == ME && a1 == ')')
		|| (mac != ME && a1 == 'D'))
	    {
		eqn ();
		eqnf = 0;
	    }
	}
	/*
	 * turning on filling is done by the paragraphing macros 
	 */
	else
	if (a1 == 'f')
	{			/* .fi */
	    if ((mac == ME && (c2 == 'h' || c2 == 'p'))
		|| (mac != ME && (c1 == 'P' || c2 == 'P')))
	    {
		SKIP;
		return;
	    }
	} else
	{
	    SKIP;
	}
    }
}

EQ ()
{
    eqn ();
    return (0);
}
domacro ()
{
    macro ();
    return (0);
}
PS ()
{
    if (!msflag)
    {
	inpic ();
    } else
    {
	noblock ('P', 'E');
    }
    return (0);
}

skip ()
{
    SKIP;
    return (0);
}

intbl ()
{
    if (msflag)
    {
	stbl ();
    } else
	tbl ();
    return (0);
}

outtbl ()
{
    intable = NO;
}

so ()
{
    getfname ();
    if (fname[0] && !iflag && (!lflag || strncmp (fname, libname, libchars)))
	infile = *++filesp = opn (fname);
    return (0);
}
nx ()
{
    getfname ();
    if (!iflag && (!lflag || strncmp (fname, libname, libchars)))
    {
	if (fname[0] == '\0')
	    exit (0);
	if (infile != stdin)
	    fclose (infile);
	infile = *filesp = opn (fname);
    }
    return (0);
}

skiptocom ()
{
    SKIP_TO_COM;
    return (COMX);
}

PP (c12)
    pacmac          c12;
{
    int             c1,
                    c2;

    frommac (c12, c1, c2);
    printf (".%c%c", c1, c2);
    while (C != '\n')
	putchar (c);
    putchar ('\n');
    return (0);
}
AU ()
{
    if (mac == MM)
    {
	return (0);
    } else
    {
	SKIP_TO_COM;
	return (COMX);
    }
}

SH (c12)
    pacmac          c12;
{
    int             c1,
                    c2;

    frommac (c12, c1, c2);

    if (parag)
    {
	printf (".%c%c", c1, c2);
	while (C != '\n')
	    putchar (c);
	putchar (c);
	putchar ('!');
	while (1)
	{
	    while (C != '\n')
		putchar (c);
	    putchar ('\n');
	    if (C == '.')
		return (COM);
	    putchar ('!');
	    putchar (c);
	}
	/* NOTREACHED */
    } else
    {
	SKIP_TO_COM;
	return (COMX);
    }
}

UX ()
{
    if (wordflag)
	printf ("UNIX\n");
    else
	printf ("UNIX ");
    return (0);
}

MMHU (c12)
    pacmac          c12;
{
    int             c1,
                    c2;

    frommac (c12, c1, c2);
    if (parag)
    {
	printf (".%c%c", c1, c2);
	while (C != '\n')
	    putchar (c);
	putchar ('\n');
    } else
    {
	SKIP;
    }
    return (0);
}

mesnblock (c12)
    pacmac          c12;
{
    int             c1,
                    c2;

    frommac (c12, c1, c2);
    noblock (')', c2);
    return (0);
}
mssnblock (c12)
    pacmac          c12;
{
    int             c1,
                    c2;

    frommac (c12, c1, c2);
    noblock (c1, 'E');
    return (0);
}
static int
nf ()
{
    noblock ('f', 'i');
    return (0);
}

ce ()
{
    sce ();
    return (0);
}

meip (c12)
    pacmac          c12;
{
    if (parag)
	mepp (c12);
    else
    if (wordflag)		/* save the tag */
	regline (meputmac, ONE);
    else
    {
	SKIP;
    }
    return (0);
}
/*
 * only called for -me .pp or .sh, when parag is on 
 */
mepp (c12)
    pacmac          c12;
{
    PP (c12);			/* eats the line */
    return (0);
}
/*
 * Start of a section heading; output the section name if doing words 
 */
mesh (c12)
    pacmac          c12;
{
    if (parag)
	mepp (c12);
    else
    if (wordflag)
	defcomline (c12);
    else
    {
	SKIP;
    }
    return (0);
}
/*
 * process a font setting 
 */
mefont (c12)
    pacmac          c12;
{
    argconcat = 1;
    defcomline (c12);
    argconcat = 0;
    return (0);
}
manfont (c12)
    pacmac          c12;
{
    return (mefont (c12));
}
manpp (c12)
    pacmac          c12;
{
    return (mepp (c12));
}

defcomline (c12)
    pacmac          c12;
{
    int             c1,
                    c2;

    frommac (c12, c1, c2);
    if (msflag && mac == MM && c2 == 'L')
    {
	if (disp || c1 == 'R')
	{
	    noblock ('L', 'E');
	} else
	{
	    SKIP;
	    putchar ('.');
	}
    } else
    if (c1 == '.' && c2 == '.')
    {
	if (msflag)
	{
	    SKIP;
	    return;
	}
	while (C == '.')
	     /* VOID */ ;
    }
    ++inmacro;
    /*
     * Process the arguments to the macro 
     */
    switch (mac)
    {
    default:
    case MM:
    case MS:
	if (c1 <= 'Z' && msflag)
	    regline (msputmac, ONE);
	else
	    regline (msputmac, TWO);
	break;
    case ME:
	regline (meputmac, ONE);
	break;
    }
    --inmacro;
}

comline ()
{
    reg int         c1;
    reg int         c2;
    pacmac          c12;
    reg int         mid;
    int             lb,
                    ub;
    int             hit;
    static int      tabsize = 0;
    static struct mactab *mactab = (struct mactab *) 0;
    reg struct mactab *mp;

    if (mactab == 0)
    {
	buildtab (&mactab, &tabsize);
    }
com:
    while (C == ' ' || c == '\t')
	;
comx:
    if ((c1 = c) == '\n')
	return;
    c2 = C;
    if (c1 == '.' && c2 != '.')
	inmacro = NO;
    if (msflag && c1 == '[')
    {
	refer (c2);
	return;
    }
    if (parag && mac == MM && c1 == 'P' && c2 == '\n')
    {
	printf (".P\n");
	return;
    }
    if (c2 == '\n')
	return;
    /*
     * Single letter macro 
     */
    if (mac == ME && (c2 == ' ' || c2 == '\t'))
	c2 = ' ';
    c12 = tomac (c1, c2);
    /*
     * binary search through the table of macros 
     */
    lb = 0;
    ub = tabsize - 1;
    while (lb <= ub)
    {
	mid = (ub + lb) / 2;
	mp = &mactab[mid];
	if (mp -> macname < c12)
	    lb = mid + 1;
	else
	if (mp -> macname > c12)
	    ub = mid - 1;
	else
	{
	    hit = 1;
#ifdef FULLDEBUG
	    printf ("preliminary hit macro %c%c ", c1, c2);
#endif FULLDEBUG
	    switch (mp -> condition)
	    {
	    case NONE:
		hit = YES;
		break;
	    case FNEST:
		hit = (filesp == files);
		break;
	    case NOMAC:
		hit = !inmacro;
		break;
	    case MAC:
		hit = inmacro;
		break;
	    case PARAG:
		hit = parag;
		break;
	    case NBLK:
		hit = !keepblock;
		break;
	    default:
		hit = 0;
	    }
	    if (hit)
	    {
#ifdef FULLDEBUG
		printf ("MATCH\n");
#endif FULLDEBUG
		switch ((*(mp -> func)) (c12))
		{
		default:
		    return;
		case COMX:
		    goto comx;
		case COM:
		    goto com;
		}
	    }
#ifdef FULLDEBUG
	    printf ("FAIL\n");
#endif FULLDEBUG
	    break;
	}
    }
    defcomline (c12);
}

int 
macsort (p1, p2)
    struct mactab  *p1,
                   *p2;
{
    return (p1 -> macname - p2 -> macname);
}

int 
sizetab (mp)
    reg struct mactab *mp;
{
    reg int         i;
    i = 0;
    if (mp)
    {
	for (; mp -> macname; mp++, i++)
	     /* VOID */ ;
    }
    return (i);
}

struct mactab  *
macfill (dst, src)
    reg struct mactab *dst;
    reg struct mactab *src;
{
    if (src)
    {
	while (src -> macname)
	{
	    *dst++ = *src++;
	}
    }
    return (dst);
}

buildtab (r_back, r_size)
    struct mactab **r_back;
    int            *r_size;
{
    int             size;

    struct mactab  *p,
                   *p1,
                   *p2;
    struct mactab  *back;

    size = sizetab (troffmactab);
    size += sizetab (ppmactab);
    p1 = p2 = (struct mactab *) 0;
    if (msflag)
    {
	switch (mac)
	{
	case ME:
	    p1 = memactab;
	    break;
	case MM:
	    p1 = msmactab;
	    p2 = mmmactab;
	    break;

	case MS:
	    p1 = msmactab;
	    break;
	case MA:
	    p1 = manmactab;
	    break;
	default:
	    break;
	}
    }
    size += sizetab (p1);
    size += sizetab (p2);
    back = (struct mactab *) calloc (size + 2, sizeof (struct mactab));

    p = macfill (back, troffmactab);
    p = macfill (p, ppmactab);
    p = macfill (p, p1);
    p = macfill (p, p2);

    qsort (back, size, sizeof (struct mactab), macsort);
    *r_size = size;
    *r_back = back;
}

/*
 * troff commands 
 */
struct mactab   troffmactab[] = {
				 M (NONE, '\\', '"', skip),	/* comment */
				 M (NOMAC, 'd', 'e', domacro),	/* define */
				 M (NOMAC, 'i', 'g', domacro),	/* ignore till .. */
				 M (NOMAC, 'a', 'm', domacro),	/* append macro */
				 M (NBLK, 'n', 'f', nf),	/* filled */
				 M (NBLK, 'c', 'e', ce),	/* centered */

				 M (NONE, 's', 'o', so),	/* source a file */
				 M (NONE, 'n', 'x', nx),	/* go to next file */

				 M (NONE, 't', 'm', skip),	/* print string on tty */
				 M (NONE, 'h', 'w', skip),	/* exception hyphen
								 * words */
				 M (NONE, 0, 0, 0)
};
/*
 * Preprocessor output 
 */
struct mactab   ppmactab[] = {
			      M (FNEST, 'E', 'Q', EQ),	/* equation starting */
			      M (FNEST, 'T', 'S', intbl),	/* table starting */
			      M (FNEST, 'T', 'C', intbl),	/* alternative table? */
			      M (FNEST, 'T', '&', intbl),	/* table reformatting */
			      M (NONE, 'T', 'E', outtbl),	/* table ending */
			      M (NONE, 'P', 'S', PS),	/* picture starting */
			      M (NONE, 0, 0, 0)
};
/*
 * Particular to ms and mm 
 */
struct mactab   msmactab[] = {
			      M (NONE, 'T', 'L', skiptocom),	/* title follows */
			      M (NONE, 'F', 'S', skiptocom),	/* start footnote */
			      M (NONE, 'O', 'K', skiptocom),	/* Other kws */

			      M (NONE, 'N', 'R', skip),	/* undocumented */
			      M (NONE, 'N', 'D', skip),	/* use supplied date */

			      M (PARAG, 'P', 'P', PP),	/* begin parag */
			      M (PARAG, 'I', 'P', PP),	/* begin indent parag,
							 * tag x */
			      M (PARAG, 'L', 'P', PP),	/* left blocked parag */

			      M (NONE, 'A', 'U', AU),	/* author */
			      M (NONE, 'A', 'I', AU),	/* authors institution */

			      M (NONE, 'S', 'H', SH),	/* section heading */
			      M (NONE, 'S', 'N', SH),	/* undocumented */
			      M (NONE, 'U', 'X', UX),	/* unix */

			      M (NBLK, 'D', 'S', mssnblock),	/* start display text */
			      M (NBLK, 'K', 'S', mssnblock),	/* start keep */
			      M (NBLK, 'K', 'F', mssnblock),	/* start float keep */
			      M (NONE, 0, 0, 0)
};

struct mactab   mmmactab[] = {
			      M (NONE, 'H', ' ', MMHU),	/* -mm ? */
			      M (NONE, 'H', 'U', MMHU),	/* -mm ? */
			      M (PARAG, 'P', ' ', PP),	/* paragraph for -mm */
			      M (NBLK, 'N', 'S', mssnblock),	/* undocumented */
			      M (NONE, 0, 0, 0)
};

struct mactab   memactab[] = {
			      M (PARAG, 'p', 'p', mepp),
			      M (PARAG, 'l', 'p', mepp),
			      M (PARAG, 'n', 'p', mepp),
			      M (NONE, 'i', 'p', meip),

			      M (NONE, 's', 'h', mesh),
			      M (NONE, 'u', 'h', mesh),

			      M (NBLK, '(', 'l', mesnblock),
			      M (NBLK, '(', 'q', mesnblock),
			      M (NBLK, '(', 'b', mesnblock),
			      M (NBLK, '(', 'z', mesnblock),
			      M (NBLK, '(', 'c', mesnblock),

			      M (NBLK, '(', 'd', mesnblock),
			      M (NBLK, '(', 'f', mesnblock),
			      M (NBLK, '(', 'x', mesnblock),

			      M (NONE, 'r', ' ', mefont),
			      M (NONE, 'i', ' ', mefont),
			      M (NONE, 'b', ' ', mefont),
			      M (NONE, 'u', ' ', mefont),
			      M (NONE, 'q', ' ', mefont),
			      M (NONE, 'r', 'b', mefont),
			      M (NONE, 'b', 'i', mefont),
			      M (NONE, 'b', 'x', mefont),
			      M (NONE, 0, 0, 0)
};


struct mactab   manmactab[] = {
			       M (PARAG, 'B', 'I', manfont),
			       M (PARAG, 'B', 'R', manfont),
			       M (PARAG, 'I', 'B', manfont),
			       M (PARAG, 'I', 'R', manfont),
			       M (PARAG, 'R', 'B', manfont),
			       M (PARAG, 'R', 'I', manfont),

			       M (PARAG, 'P', 'P', manpp),
			       M (PARAG, 'L', 'P', manpp),
			       M (PARAG, 'H', 'P', manpp),
			       M (NONE, 0, 0, 0)
};
