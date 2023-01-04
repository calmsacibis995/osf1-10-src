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
static char	*sccsid = "@(#)$RCSfile: y2.c,v $ $Revision: 4.2.6.2 $ (DEC) $Date: 1993/08/02 19:43:35 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDLANG) Language Utilities
 *
 * FUNCTIONS: beg_debug, chfind, cpyact, cpycode, cpyunion,
 *            cstash, defin, defout, end_debug, end_toks,
 *            fdtype, finact, gettok, lhsfill, lrprnt, rhsfill,
 *            setup, skipcom
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#ifndef _BLD
#include <sys/localedef.h>
#endif /* _BLD */

#include "dextern"

#ifndef _BLD /*osf compatibility */
#include "yacc_msg.h"
extern nl_catd catd;
#define MSGSTR(Num, Str) catgets(catd,MS_YACC,Num,Str)
#else
#define MSGSTR(Num,Str) Str
#endif
#define IDENTIFIER 257
#define MARK 258
#define TERM 259
#define LEFT 260
#define RIGHT 261
#define BINARY 262
#define PREC 263
#define LCURLY 264
#define C_IDENTIFIER 265  /* name followed by colon */
#define NUMBER 266
#define START 267
#define TYPEDEF 268
#define TYPENAME 269
#define UNION 270
#define ENDFILE 0

        /* communication variables between various I/O routines */

char *infile;   /* input file name */
int numbval;    /* value of an input number */
char tokname[NAMESIZE]; /* input token name */

        /* storage of names */

int cnamsz = CNAMSZ;
char * cnamp;			/* place where next name is to be put in */
char *cnamb;			/* pointer to end of name buffer */
int ndefout = 3;		/* number of defined symbols output */

	/* storage of types */
int YYSTYPE_seen;		/* Have we generated a union {...} YYSTYPE */
int ntypes;			/* number of types defined */
char * typeset[NTYPES];		/* pointers to type tags */

        /* symbol tables for tokens and nonterminals */

int ntokens = 0;
struct toksymb tokset[NTERMS];
int toklev[NTERMS];
int nnonter = -1;
struct ntsymb nontrst[NNONTERM];
int start_sym;			/* start symbol */

        /* assigned token type values */
int extval = 0;

        /* input and output file descriptors */

FILE * finput;          /* yacc input file */
FILE * faction;         /* file for saving actions */
FILE * fdefine;         /* file for # defines */
FILE * ftable;          /* y.tab.c file */
FILE * ftemp;           /* tempfile to pass 2 */
FILE * fdebug;          /* where the strings for debugging are stored */
FILE * foutput;         /* y.output file */

        /* storage for grammar rules */

int *mem0; /* production storage */
int *mem;
int nprod= 1;   /* number of productions */
int *prdptr[NPROD];     /* pointers to descriptions of productions */
int levprd[NPROD] ;     /* precedence levels for the productions */
char had_act[NPROD];    /* set to 1 if the reduction has action code to do */

/*default location of the parser text file */

char *ParserPath = PARSER;

char *gsprefix = "";	/* Default is normal yy* global symbols */

int gen_lines = 1;      /* flag for generating the # line's default is yes */
int gen_testing = 0;    /* flag for whether to include runtime debugging */

extern int getopt();
extern char *optarg;
extern int optind, opterr, optopt;

/* Local stuff */

static void	beg_debug(void);
static void	end_debug(void);
static int	gettok(void);
static int	chfind(int, char*);
static void	cpyunion(void);
static int	skipcom(void);
static void	defout(void);
static void	cpycode(void);
static void	end_toks(void);
static void	rhsfill(char *);
static void	lhsfill(char *);

static void	cpyact(int);
static void	finact(void);

static void	lrprnt(void);

void
setup(int argc,char *argv[])
{       int i,j,lev,t, ty;
        int c, len, filed_flag = 0, fileu_flag = 0;
        int *p;
        char actname[8];

        foutput = NULL;
        fdefine = NULL;
        i = 1;

	while ((c = getopt(argc, argv, ":svdltb:p:P:N:")) != -1) {
	    switch( c ){
	      case 's':
		SplitFlag++;
		break;

	      case 'v':
		fileu_flag = 1;
		break;

	      case 'd':
		filed_flag = 1;
		break;

	      case 'l':
		gen_lines = !gen_lines;
		break;

	      case 't':
		gen_testing = !gen_testing;
		break;

	      case 'b':       /* Must be followed by the prefix to use */
		prefix = optarg;
                break;

	      case 'p':		/* global-symbol prefix */
		if (strcmp(optarg,"yy"))	/* Defend against -p yy */
		  gsprefix = optarg;		/*  which would cause recursive macro */
		break;

	      case 'N':		/* Re-size the "state-space" allocation */
		{
		    char *ep;
		    memsize = strtoul(optarg, &ep, 10);
		    if (ep == NULL || *ep != '\0')
		      error( MSGSTR(NOPT,"The -N flag format is not correct."));

		    if (memsize < MEMSIZE)
		      error( MSGSTR(NMNUM, "Use an integer greater than 40000 with the -N flag."));
		    break;
		}

	      case 'P':      /* yaccpar file */
		ParserPath = optarg;
		break;

	      case ':':
		switch (optopt) {
		  case 'p':
		  case 'b':	error( MSGSTR(PREMIS,
					      "option -%c must be followed by a prefix"),optopt);
		  case 'P':	error( MSGSTR(NEEDARG,
					      "Missing option argument for -%c flag."), optopt);
		  case 'N':	error( MSGSTR(NMNUM,
					      "Use an integer greater than 40000 with the -N flag."));
		}
		/* NOTREACHED */
		break;


	      case '?':
		fprintf(stderr, MSGSTR(USAGE, "usage: yacc -svdlt [-b prefix] [-p sym_prefix] [-P path] [-N num] file\n"));
		exit(1);
	    }
	}


        fdebug = fopen( DEBUGNAME, "w" );
        if ( fdebug == NULL )
                error( MSGSTR(NODEBUG, "cannot open yacc.debug") );

	if (!prefix)
	    prefix = "y";

	len = strlen(prefix);

        if (filed_flag) {
                filed = calloc(len + 7, sizeof(char));
                if( !filed ) error(MSGSTR(OUTMEM, "Out of Memory" ));
                sprintf(filed, "%s.tab.h", prefix);
                fdefine = fopen( filed, "w" );
                if ( fdefine == NULL )
                        error(MSGSTR(NOTOPN, "cannot open %s"), filed);
        }

        if (fileu_flag) {
                fileu = calloc(len + 8, sizeof(char));
                if( !fileu ) error(MSGSTR(OUTMEM, "Out of Memory" ));
                sprintf(fileu, "%s.output", prefix);
                foutput = fopen(fileu, "w" );
                if( foutput == NULL )
                        error( MSGSTR(NOTOPN, "cannot open %s"), fileu);
        }

        ofile = calloc(len + 7, sizeof(char));
        if( !ofile ) error(MSGSTR(OUTMEM, "Out of Memory" ));
        sprintf(ofile, "%s.tab.c", prefix);
        ftable = fopen( ofile, "w" );
        if( ftable == NULL )
                error( MSGSTR(NOTOPN, "cannot open %s" ), ofile);

        ftemp = fopen( TEMPNAME, "w" );
        faction = fopen( ACTNAME, "w" );
        if( ftemp==NULL || faction==NULL )
                error(MSGSTR(NOTEMP, "cannot open temp file") );

        if ( optind >= argc )
                error( MSGSTR(NOINPUT, "No Input File given") );

        if( ((finput=fopen( infile=argv[optind], "r" )) == NULL ) ){
                error( MSGSTR(NOTOPN, "cannot open %s"), argv[optind] );
                }

        /* Allocate storage for mem0 array depending on value of memsize */
        mem0 = calloc(sizeof(int), memsize);
        if( !mem0 ) error(MSGSTR(OUTMEM, "Out of Memory" ));

        cnamp = malloc(CNAMSZ);
        if( !cnamp ) error(MSGSTR(OUTMEM, "Out of Memory" ));
        cnamb = &cnamp[cnamsz];

        mem = mem0;
        defin(0,"$end");
#ifndef _BLD
        extval = __lc_ctype->max_wc+1;
#else /* _BLD */
        extval = 0400;
#endif
        defin(0,"error");
        defin(1,"$accept");
        mem=mem0;
        lev = 0;
        ty = 0;
        i=0;
        beg_debug();    /* initialize fdebug file */

        /* sorry -- no yacc parser here.....
                we must bootstrap somehow... */

        for( t=gettok();  t!=MARK && t!= ENDFILE; ){
                switch( t ){

                case ';':
                        t = gettok();
                        break;

                case START:
                        if( (t=gettok()) != IDENTIFIER ){
                                error( MSGSTR(BADSTART,
                                        "bad %%start construction") );
                                }
                        start_sym = chfind(1,tokname);
                        t = gettok();
                        continue;

                case TYPEDEF:
                        if( (t=gettok()) != TYPENAME )
                                error( MSGSTR(BADSYNTAX,
                                        "bad syntax in %%type") );
                        ty = numbval;
                        for(;;){
                                t = gettok();
                                switch( t ){

                                case IDENTIFIER:
                                        if((t=chfind(1, tokname)) < NTBASE) {
                                                j = TYPE( toklev[t] );
                                                if( j!= 0 && j != ty ){
                                                        error( MSGSTR(EREDECLR, "type redeclaration of token %s"),
                                                          tokset[t].name);
                                                        }
                                                else SETTYPE( toklev[t],ty);
                                                }
                                        else {
                                                j = nontrst[t-NTBASE].tvalue;
                                                if( j != 0 && j != ty ){
                                                        error(MSGSTR(EREDEC2, "type redeclaration of nonterminal %s"),
                                                                nontrst[t-NTBASE].name );
                                                        }
                                                else
                                                 nontrst[t-NTBASE].tvalue = ty;
                                                }
                                case ',':
                                        continue;

                                case ';':
                                        t = gettok();
                                        break;
                                default:
                                        break;
                                        }
                                break;
                                }
                        continue;

                case UNION:
                        /* copy the union declaration to the output */
                        cpyunion();
                        t = gettok();
                        continue;

                case LEFT:
                case BINARY:
                case RIGHT:
                        ++i;
                case TERM:
                        lev = t-TERM;  /* nonzero means new prec. and assoc. */
                        ty = 0;

                        /* get identifiers so defined */

                        t = gettok();
                        if( t == TYPENAME ){ /* there is a type defined */
                                ty = numbval;
                                t = gettok();
                                }

                        for(;;) {
                                switch( t ){

                                case ',':
                                        t = gettok();
                                        continue;

                                case ';':
                                        break;

                                case IDENTIFIER:
                                        j = chfind(0,tokname);
                                        if( lev ){
                                                if( ASSOC(toklev[j]) ) error( MSGSTR(EPRECID, "redeclaration of precedence of %s"), tokname );
                                                SETASC(toklev[j],lev);
                                                SETPLEV(toklev[j],i);
                                                }
                                        if( ty ){
                                                if( TYPE(toklev[j]) ) error( MSGSTR(ERETYPE, "redeclaration of type of %s"), tokname );
                                                SETTYPE(toklev[j],ty);
                                                }
                                        if( (t=gettok()) == NUMBER ){
                                                tokset[j].value = numbval;
                                                if( j < ndefout && j>2 ){
                                                        error( MSGSTR(LATETYPE, "please define type number of %s earlier"),
                                                          tokset[j].name);
                                                        }
                                                t=gettok();
                                                }
                                        continue;

                                        }

                                break;
                                }

                        continue;

                case LCURLY:
                        defout();
                        cpycode();
                        t = gettok();
                        continue;

                default:
                        error( MSGSTR(SYNTAX, "syntax error") );

                        }

                }

        if( t == ENDFILE ){
                error(MSGSTR(PEOF, "unexpected EOF before %%%%") );
                }

        /* t is MARK */

        defout();
        end_toks();     /* all tokens dumped - get ready for reductions */

	if (gsprefix[0] != '\0') {
	    /*
	     * A global-symbol prefix has been requested.  This will "hide"
	     * symbols which would prevent linking TWO parsers into the same program
	     */
	    char *defs[] = {"parse", "lex", "error",		/* Functions */
			      "lval",
			      "val",
			      "char",
			      "debug",
			      "errflag",
			      "nerrs",
			      NULL };	/* Variables */
	    int i;

	    for (i=0; defs[i]; i++) fprintf( ftable, "#define yy%s %s%s\n",
					    defs[i], gsprefix, defs[i] );
	}

        fprintf( ftable,  "#define yyclearin yychar = -1\n" );
        fprintf( ftable,  "#define yyerrok yyerrflag = 0\n" );
        fprintf( ftable,  "extern int yychar;\nextern int yyerrflag;\n" );
        fprintf( ftable,
          "#ifndef YYMAXDEPTH\n#define YYMAXDEPTH 150\n#endif\n" );
        if( !YYSTYPE_seen && !ntypes )
          fprintf( ftable,  "#ifndef YYSTYPE\n#define YYSTYPE long\n#endif\n" );
        fprintf( ftable,  "YYSTYPE yylval, yyval;\n" );
        fprintf( ftable,  "typedef int yytabelem;\n" );

        prdptr[0]=mem;
        /* added production */
        *mem++ = NTBASE;
        *mem++ = start_sym;  /* if start_sym is 0, we will
                              * overwrite with the lhs of the first rule */
        *mem++ = 1;
        *mem++ = 0;
        prdptr[1]=mem;

        while( (t=gettok()) == LCURLY ) cpycode();

        if( t != C_IDENTIFIER )
                error( MSGSTR(SNTAX1, "bad syntax on first rule" ));

        if( !start_sym ) prdptr[0][1] = chfind(1,tokname);

        /* read rules */

        while( t!=MARK && t!=ENDFILE ){

                /* process a rule */

                if( t == '|' ){
                        rhsfill( (char *) 0 );  /* restart fill of rhs */
                        *mem++ = *prdptr[nprod-1];
                        }
                else if( t == C_IDENTIFIER ){
                        *mem = chfind(1,tokname);
                        if( *mem < NTBASE )
                          error( MSGSTR(BADLHS,
                            "token illegal on LHS of grammar rule") );
                        ++mem;
                        lhsfill( tokname );     /* new rule: restart strings */
                        }
                else error( MSGSTR(BADDRULE,
                        "illegal rule: missing semicolon or | ?") );

                /* read rule body */


                t = gettok();
        more_rule:
                while( t == IDENTIFIER ) {
                        *mem = chfind(1,tokname);
                        if( *mem<NTBASE ) levprd[nprod] = toklev[*mem];
                        ++mem;
                        rhsfill( tokname );     /* add to rhs string */
                        t = gettok();
                        }


                if( t == PREC ){
                        if( gettok()!=IDENTIFIER)
                                error( MSGSTR(BADPREC,
                                        "illegal %%prec syntax") );
                        j = chfind(2,tokname);
                        if( j>=NTBASE)error(MSGSTR(NONTERM2,
                                "nonterminal %s illegal after %%prec"),
                                nontrst[j-NTBASE].name);
                        levprd[nprod]=toklev[j];
                        t = gettok();
                        }

                if( t == '=' ){
                        had_act[nprod] = 1;
                        levprd[nprod] |= ACTFLAG;
                        if (SplitFlag)
                                 fprintf(faction,"\nstatic yyf%d() {",nprod);
                            else fprintf( faction, "\ncase %d:", nprod );
                        cpyact( mem-prdptr[nprod]-1 );

                        if (!SplitFlag) fprintf( faction, " /*NOTREACHED*/ break;" );
                            else fprintf(faction,"\nreturn(-1);}");
                        if( (t=gettok()) == IDENTIFIER ){
                                /* action within rule... */

                                lrprnt();               /* dump lhs, rhs */
                                sprintf( actname, "$$%d", nprod );
                                j = chfind(1,actname); 
                                /* make it a nonterminal */

                                /* the current rule will become rule
                                 * number nprod+1 */
                                /* move the contents down,
                                 * and make room for the null */

                                for( p=mem; p>=prdptr[nprod]; --p ) p[2] = *p;
                                mem += 2;

                                /* enter null production for action */

                                p = prdptr[nprod];

                                *p++ = j;
                                *p++ = -nprod;

                                /* update the production information */

                                levprd[nprod+1] = levprd[nprod] & ~ACTFLAG;
                                levprd[nprod] = ACTFLAG;

                                if( ++nprod >= NPROD ) error( MSGSTR(MANYRULES,
                                   "more than %d rules"), NPROD );
                                prdptr[nprod] = p;

                                /* make the action appear
                                 * in the original rule */
                                *mem++ = j;

                                /* get some more of the rule */

                                goto more_rule;
                                }

                        }

                while( t == ';' ) t = gettok();

                *mem++ = -nprod;

                /* check that default action is reasonable */

                if( ntypes && !(levprd[nprod]&ACTFLAG) &&
                        nontrst[*prdptr[nprod]-NTBASE].tvalue ){
                        /* no explicit action, LHS has value */
                        register tempty;
                        tempty = prdptr[nprod][1];
                        if( tempty < 0 ) error( MSGSTR(NOVALUE, "must return a value, since LHS has a type") );
                        else if( tempty >= NTBASE )
                                tempty = nontrst[tempty-NTBASE].tvalue;
                        else tempty = TYPE( toklev[tempty] );
                        if( tempty != nontrst[*prdptr[nprod]-NTBASE].tvalue ){
                                error( MSGSTR(TCLASH, "default action causes potential type clash") );
                                }
                        }

                if( ++nprod >= NPROD ) error( MSGSTR(MANYRULES,
                        "more than %d rules"), NPROD );
                prdptr[nprod] = mem;
                levprd[nprod]=0;

                }

        /* end of all rules */

        end_debug();            /* finish fdebug file's input */
        finact();
        if( t == MARK ){
                if ( gen_lines )
                        fprintf( ftable, "\n# line %d \"%s\"\n",
                                lineno, infile );
                while( (c=getc(finput)) != EOF ) putc( c, ftable );
                }
        fclose( finput );
        }

static void
finact(void){
        /* finish action routine */

        fclose(faction);

        fprintf( ftable, "# define YYERRCODE %d\n", tokset[2].value );

        }

int defin(int t, char *s ) {
/*      define s to be a terminal if t=0
        or a nonterminal if t=1         */

        register val;
	int	 i;	/* return value for mbtowc */
	wchar_t	 wc;
	int	 mb_cur_max = MB_CUR_MAX;

        if (t) {
                if( ++nnonter >= NNONTERM ) error(MSGSTR(MANYNTERMS,
                  "too many nonterminals, limit %d"),NNONTERM);
                nontrst[nnonter].name = cstash(s);
                return( NTBASE + nnonter );
                }
        /* must be a token */
        if( ++ntokens >= NTERMS ) error(MSGSTR(MANYTERMS,
                "too many terminals, limit %d"),NTERMS );
        tokset[ntokens].name = cstash(s);

        /* establish value for token */

        if( s[0]==' ' && s[2]=='\0' ) /* single character literal */
                val = s[1];
        else if ( s[0]==' ' && s[1]=='\\' ) { /* escape sequence */
                if( s[3] == '\0' ){ /* single character escape sequence */
                        switch ( s[2] ){
                                         /* character which is escaped */
                        case 'n': val = '\n'; break;
                        case 'r': val = '\r'; break;
                        case 'b': val = '\b'; break;
                        case 't': val = '\t'; break;
                        case 'f': val = '\f'; break;
                        case 'a': val = '\a'; break;	/* XPG4 */
                        case 'v': val = '\v'; break;	/* XPG4 */
                        case '\'': val = '\''; break;
                        case '\?': val = '\?'; break;	/* XPG4 */
                        case '"': val = '"'; break;
                        case '\\': val = '\\'; break;
                        default: error( MSGSTR(ESCINVAL,
                                        "invalid escape") );
                                }
                        }
                else if( s[2] <= '7' && s[2]>='0' ){ /* \nnn sequence */
                        if( s[3]<'0' || s[3] > '7' || s[4]<'0' ||
                                s[4]>'7' || s[5] != '\0' )
                          error(MSGSTR(BADCONSTRT,
                                "illegal \\nnn construction" ));
                        val = 64*s[2] + 8*s[3] + s[4] - 73*'0';
                        if( val == 0 ) error( MSGSTR(OZERO,
                                "'\\000' is illegal") );
                        }
                else if( s[2] == 'x' ){ /* \xnn sequence */
                        if( isxdigit(s[3]) && isxdigit(s[4]) && s[5] == '\0' )
				val = (int)strtoul(s+3, NULL, 16);
			else	/* Illegal hex numbers */
                	error(MSGSTR(BADHEXCONST,
                                "The \\xnn construction is not valid." ));
		}
		else 	/* Illegal numbers */
                	error(MSGSTR(BADCONSTRT,
                                "illegal \\nnn construction" ));
	}
        else {
#ifndef _BLD
		i = mbtowc(&wc, s+1, mb_cur_max);
		if (i >= 2)
			val = (int)wc;
                else
#endif	/* _BLD */
			val = extval++;
                }
        tokset[ntokens].value = val;
        toklev[ntokens] = 0;
        return( ntokens );
}

static void
defout(void){ /* write out the defines (at the end of the declaration section) */

    register int i, c;
    register unsigned char *cp;

    for( i=ndefout; i<=ntokens; ++i ) {

	cp = (unsigned char *) tokset[i].name;
	if( *cp == ' ' ) {       /* literals */
	    fprintf( fdebug, "\t\"%s\",\t%d,\n", ++cp, tokset[i].value );
	    continue;
	}

	fprintf( fdebug, "\t\"%s\",\t%d,\n", tokset[i].name, tokset[i].value );

	/*
	 * Insure that name is a valid symbol for definition
	 */
	c = *cp++;
	if ( isalpha(c) || c == '_' ) {
	    for( ; (c= *cp)!='\0'; ++cp ){
		if (isalnum(c) || c == '_')
		  continue; 	/* Looks good so far */
		else
		  goto nodef;	/* Cannot be a symbol */
	    }
	}

	fprintf( ftable, "# define %s %d\n", tokset[i].name, tokset[i].value );

	if( fdefine != NULL )
	  fprintf( fdefine, "# define %s %d\n", tokset[i].name, tokset[i].value );

nodef:
	;
    }

    ndefout = ntokens+1;

}

char *
cstash( s ) register char *s; {
        char *temp;

	if ( (strlen(s)+cnamp+1) >= cnamb) {
	    cnamsz *=2;				/* Reduce chance of needing more */
	    cnamp = malloc( cnamsz );
	    if( !cnamp ) error(MSGSTR(OUTMEM, "Out of Memory" ));
	    cnamb = &cnamp[cnamsz];
	}

        temp = cnamp;
        do {
	    *cnamp++ = *s;
	}  while ( *s++ );
        return( temp );
        }

static int
gettok(void) {
        register i, base;
        static int peekline; /* number of '\n' seen in lookahead */
        register c, match, reserve;

begin:
        reserve = 0;
        lineno += peekline;
        peekline = 0;
        c = getc(finput);
        while( c==' ' || c=='\n' || c=='\t' || c=='\f' ){
                if( c == '\n' ) ++lineno;
                c=getc(finput);
                }
        if( c == '/' ){ /* skip comment */
                lineno += skipcom();
                goto begin;
                }

        switch(c){

        case EOF:
                return(ENDFILE);
        case '{':
                ungetc( c, finput );
                return( '=' );  /* action ... */
        case '<':  /* get, and look up, a type name (union member name) */
                i = 0;
                while( (c=getc(finput)) != '>' && c>=0 && c!= '\n' ){
                        tokname[i] = c;
                        if( ++i >= NAMESIZE ) --i;
                        }
                if( c != '>' ) error( MSGSTR(UNTERM,
                        "unterminated < ... > clause" ));
                tokname[i] = '\0';
                for( i=1; i<=ntypes; ++i ){
                        if( !strcmp( typeset[i], tokname ) ){
                                numbval = i;
                                return( TYPENAME );
                                }
                        }
                typeset[numbval = ++ntypes] = cstash( tokname );
                return( TYPENAME );

        case '"':       
        case '\'':
                match = c;
                tokname[0] = ' ';
                i = 1;
                for(;;){
                        c = getc(finput);
                        if( c == '\n' || c == EOF )
                                error(MSGSTR(MISSQUOTE,
                                  "illegal or missing ' or \"") );
                        if( c == '\\' ){
                                c = getc(finput);
                                tokname[i] = '\\';
                                if( ++i >= NAMESIZE ) --i;
                                }
                        else if( c == match ) break;
                        tokname[i] = c;
                        if( ++i >= NAMESIZE ) --i;
                        }
                break;

        case '%':
        case '\\':

                switch(c=getc(finput)) {

                case '0':       return(TERM);
                case '<':       return(LEFT);
                case '2':       return(BINARY);
                case '>':       return(RIGHT);
                case '%':
                case '\\':      return(MARK);
                case '=':       return(PREC);
                case '{':       return(LCURLY);
                default:        reserve = 1;
                        }

        default:

                if( isdigit(c) ){ /* number */
                        numbval = c-'0' ;
                        base = (c=='0') ? 8 : 10 ;
                        for( c=getc(finput); isdigit(c) ; c=getc(finput) ){
                                numbval = numbval*base + c - '0';
                                }
                        ungetc( c, finput );
                        return(NUMBER);
                        }
                else if( islower(c) || isupper(c) || c=='_' ||
                         c=='.' || c=='$' ){
                        i = 0;
                        while( islower(c) || isupper(c) || isdigit(c) ||
                               c=='_' || c=='.' || c=='$' ){
                                tokname[i] = c;
                                if( reserve && isupper(c) )
                                        tokname[i] += 'a'-'A';
                                if( ++i >= NAMESIZE ) --i;
                                c = getc(finput);
                                }
                        }
                else return(c);

                ungetc( c, finput );
                }

        tokname[i] = '\0';

        if( reserve ){ /* find a reserved word */
                if( !strcmp(tokname,"term")) return( TERM );
                if( !strcmp(tokname,"token")) return( TERM );
                if( !strcmp(tokname,"left")) return( LEFT );
                if( !strcmp(tokname,"nonassoc")) return( BINARY );
                if( !strcmp(tokname,"binary")) return( BINARY );
                if( !strcmp(tokname,"right")) return( RIGHT );
                if( !strcmp(tokname,"prec")) return( PREC );
                if( !strcmp(tokname,"start")) return( START );
                if( !strcmp(tokname,"type")) return( TYPEDEF );
                if( !strcmp(tokname,"union")) return( UNION );
                error(MSGSTR(ILLWORD,
                        "invalid escape, or illegal reserved word: %s"),
                        tokname );
                }

        /* look ahead to distinguish IDENTIFIER from C_IDENTIFIER */

        c = getc(finput);
        while( c==' ' || c=='\t'|| c=='\n' || c=='\f' || c== '/' ) {
                if( c == '\n' ) ++peekline;
                else if( c == '/' ){ /* look for comments */
                        peekline += skipcom();
                        }
                c = getc(finput);
                }
        if( c == ':' ) return( C_IDENTIFIER );
        ungetc( c, finput );
        return( IDENTIFIER );
}

int
fdtype( int t ){ /* determine the type of a symbol */
        register int v;

        if( t >= NTBASE ) v = nontrst[t-NTBASE].tvalue;
        else v = TYPE( toklev[t] );
        if( v <= 0 ) error( MSGSTR(NOTYPE, "must specify type for %s"),
                (t>=NTBASE)?nontrst[t-NTBASE].name:
                        tokset[t].name );
        return( v );
        }
static int
chfind( int t, char *s ) {
        int i;

        if (s[0]==' ')t=0;
        TLOOP(i){
                if(!strcmp(s,tokset[i].name)){
                        return( i );
                        }
                }
        NTLOOP(i){
                if(!strcmp(s,nontrst[i].name)) {
                        return( i+NTBASE );
                        }
                }
        /* cannot find name */
        if( t>1 )
                error( MSGSTR(NONAME, "%s should have been defined earlier"),
                        s );
        return( defin( t, s ) );
        }

static void
cpyunion(void){
        /* copy the union declaration to the output,
         * and the define file if present */

        int level, c;

	YYSTYPE_seen++;

        if ( gen_lines )
                fprintf( ftable, "\n# line %d \"%s\"\n", lineno, infile );
        fprintf( ftable, "typedef union " );
        if( fdefine ) fprintf( fdefine, "\ntypedef union " );

        level = 0;
        for(;;){
                if( (c=getc(finput)) < 0 ) 
                        error( MSGSTR(EOFENC,
                          "EOF encountered while processing %%union"));
                putc( c, ftable );
                if( fdefine ) putc( c, fdefine );

                switch( c ){

                case '\n':
                        ++lineno;
                        break;

                case '{':
                        ++level;
                        break;

                case '}':
                        --level;
                        if( level == 0 ) { /* we are finished copying */
                                fprintf( ftable, " YYSTYPE;\n" );
                                if( fdefine ) fprintf( fdefine,
                                  " YYSTYPE;\nextern YYSTYPE yylval;\n" );
                                return;
                                }
                        }
                }
        }

static void
cpycode(void){ /* copies code between \{ and \} */

        int c;
        c = getc(finput);
        if( c == '\n' ) {
                c = getc(finput);
                lineno++;
                }
        if ( gen_lines )
                fprintf( ftable, "\n# line %d \"%s\"\n", lineno, infile );
        while( c>=0 ){
                if( c=='\\' )
                        if( (c=getc(finput)) == '}' ) return;
                        else putc('\\', ftable );
                if( c=='%' )
                        if( (c=getc(finput)) == '}' ) return;
                        else putc('%', ftable );
                putc( c , ftable );
                if( c == '\n' ) ++lineno;
                c = getc(finput);
                }
        error(MSGSTR(FEOF, "eof before %%}") );
        }

static int
skipcom(void){ /* skip over comments */
        register c, i=0;  /* i is the number of lines skipped */

        /* skipcom is called after reading a / */

        if( getc(finput) != '*' ) error( MSGSTR(BADCOMNT,
                "illegal comment") );
        c = getc(finput);
        while( c != EOF ){
                while( c == '*' ){
                        if( (c=getc(finput)) == '/' ) return( i );
                        }
                if( c == '\n' ) ++i;
                c = getc(finput);
                }
        error( MSGSTR(COMTEOF, "EOF inside comment") );
        /* NOTREACHED */
        }

static void
cpyact(int offset){ /* copy C action to the next ; or closing } */
        int brac, c, match, j, s, tok;

        if ( gen_lines )
                fprintf( faction, "\n# line %d \"%s\"\n", lineno, infile );

        brac = 0;

loop:
        c = getc(finput);
swt:
        switch( c ){

case ';':
                if( brac == 0 ){
                        putc( c , faction );
                        return;
                        }
                goto lcopy;

case '{':
                brac++;
                goto lcopy;

case '$':
                s = 1;
                tok = -1;
                c = getc(finput);
                if( c == '<' ){ /* type description */
                        ungetc( c, finput );
                        if( gettok() != TYPENAME ) error( MSGSTR(BADIDENT,
                          "bad syntax on $<ident> clause") );
                        tok = numbval;
                        c = getc(finput);
                        }
                if( c == '$' ){
                        fprintf( faction, "yyval");
                        if( ntypes ){ /* put out the proper tag... */
                                if( tok < 0 ) tok = fdtype( *prdptr[nprod] );
                                fprintf( faction, ".%s", typeset[tok] );
                                }
                        goto loop;
                        }
                if( c == '-' ){
                        s = -s;
                        c = getc(finput);
                        }
                if( isdigit(c) ){
                        j=0;
                        while( isdigit(c) ){
                                j= j*10+c-'0';
                                c = getc(finput);
                                }

                        j = j*s - offset;
                        if( j > 0 ){
                                error( MSGSTR(ILLOFF, "Illegal use of $%d"),
                                        j+offset );
                                }

                        fprintf( faction, "yypvt[-%d]", -j );
                        if( ntypes ){ /* put out the proper tag */
                                if( j+offset <= 0 && tok < 0 )
                                        error( MSGSTR(NOTYPE2,
                                                "must specify type of $%d"),
                                                j+offset );
                                if( tok < 0 )
                                  tok = fdtype( prdptr[nprod][j+offset] );
                                fprintf( faction, ".%s", typeset[tok] );
                                }
                        goto swt;
                        }
                putc( '$' , faction );
                if( s<0 ) putc('-', faction );
                goto swt;

case '}':
                if( --brac ) goto lcopy;
                putc( c, faction );
                return;


case '/':       /* look for comments */
                putc( c , faction );
                c = getc(finput);
                if( c != '*' ) goto swt;

                /* it really is a comment */

                putc( c , faction );
                c = getc(finput);
                while( c != EOF ){
                        while( c=='*' ){
                                putc( c , faction );
                                if( (c=getc(finput)) == '/' ) goto lcopy;
                                }
                        putc( c , faction );
                        if( c == '\n' )++lineno;
                        c = getc(finput);
                        }
                error( MSGSTR(COMTEOF, "EOF inside comment") );

case '\'':      /* character constant */
                match = '\'';
                goto string;

case '"':       /* character string */
                match = '"';

        string:

                putc( c , faction );
                while( c=getc(finput) ){

                        if( c=='\\' ){
                                putc( c , faction );
                                c=getc(finput);
                                if( c == '\n' ) ++lineno;
                                }
                        else if( c==match ) goto lcopy;
                        else if( c=='\n' ) error( MSGSTR(NLINSTR,
                                "newline in string or char. const.") );
                        putc( c , faction );
                        }
                error( MSGSTR(EOFINSTR,
                        "EOF in string or character constant") );

case EOF:
                error(MSGSTR(NONTERMACT, "action does not terminate"));

case '\n':      ++lineno;
                goto lcopy;

                }

lcopy:
        putc( c , faction );
        goto loop;
        }


#define RHS_TEXT_LEN            ( BUFSIZ * 4 )  /* length of rhstext */

char lhstext[ BUFSIZ ];         /* store current lhs (non-terminal) name */
char rhstext[ RHS_TEXT_LEN ];   /* store current rhs list */

static void
lhsfill( char *s )    /* new rule, dump old (if exists), restart strings */
{
        rhsfill( NULL );
        strcpy( lhstext, s );   /* don't worry about too long of a name */
}

static void
rhsfill( char *s )
{
        static char *loc = rhstext;     /* next free location in rhstext */
        register char *p;

        if ( !s )       /* print out and erase old text */
        {
                if ( *lhstext )         /* there was an old rule - dump it */
                        lrprnt();
                ( loc = rhstext )[0] = '\0';
                return;
        }
        /* add to stuff in rhstext */
        p = s;
        *loc++ = ' ';
        if ( *s == ' ' )        /* special quoted symbol */
        {
                *loc++ = '\'';  /* add first quote */
                p++;
        }
        while ( *loc = *p++ )
                if ( loc++ > &rhstext[ RHS_TEXT_LEN ] - 2 )
                        break;
        if ( *s == ' ' )
                *loc++ = '\'';
        *loc = '\0';            /* terminate the string */
}


static void
lrprnt(void)        /* print out the left and right hand sides */
{
        char *rhs;

        if ( !*rhstext )                /* empty rhs - print usual comment */
                rhs = " /* empty */";
        else
                rhs = rhstext;
        fprintf( fdebug, "      \"%s :%s\",\n", lhstext, rhs );
}


static void
beg_debug(void)     /* dump initial sequence for fdebug file */
{
        fprintf( fdebug,
                "typedef struct { char *t_name; int t_val; } yytoktype;\n" );
        fprintf( fdebug,
                "#ifndef YYDEBUG\n#\tdefine YYDEBUG\t%d", gen_testing );
        fprintf( fdebug, "\t/*%sallow debugging */\n#endif\n\n",
                gen_testing ? " " : " don't " );
        fprintf( fdebug, "#if YYDEBUG\n\nyytoktype yytoks[] =\n{\n" );
}

static void
end_toks(void)      /* finish yytoks array, get ready for yyred's strings */
{
        fprintf( fdebug, "\t\"-unknown-\",\t-1\t/* ends search */\n" );
        fprintf( fdebug, "};\n\nchar * yyreds[] =\n{\n" );
        fprintf( fdebug, "\t\"-no such reduction-\",\n" );
}

static void
end_debug(void)     /* finish yyred array, close file */
{
        lrprnt();               /* dump last lhs, rhs */
        fprintf( fdebug, "};\n#endif /* YYDEBUG */\n" );
        fclose( fdebug );
}
