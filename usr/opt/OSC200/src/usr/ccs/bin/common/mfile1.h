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
 *	@(#)$RCSfile: mfile1.h,v $ $Revision: 4.2.6.6 $ (DEC) $Date: 1993/11/22 21:22:12 $
 */ 
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
 * COMPONENT_NAME: (CMDPROG) mfile1.h
 *
 * FUNCTIONS: BTYPE, CHARCAST, DECREF, DEUNSIGN, ENUNSIGN, FIXARG, FIXDEF    
 *            FIXSTRUCT, FOLD_GENERAL, FOLD_INTEGRAL, HASCONST, HASVOLATILE   
 *            INCREF, ISAGGREGATE, ISARITHM, ISARY, ISBTYPE, ISCHAR, ISCONST  
 *            ISCONSTANT, ISFLOAT, ISFTN, ISINTEGRAL, ISNUMBER, ISPTR         
 *            ISQUALIFIED, ISSCALAR, ISTSIGNED, ISTUNSIGNED, ISUNION          
 *            ISUNSIGNED, ISVOLATILE, MODTYPE, NO_FOLD, QUALIFIERS, SETDCON   
 *            TOOLSTR, TOPQTYPE, TOPTYPE, UNSIGNABLE, checkst, tyalloc        
 *
 * ORIGINS: 27 03 09 32 00 
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Changes for ANSI C were developed by HCR Corporation for IBM
 * Corporation under terms of a work made for hire contract.
 */
#ifndef __MFILE1_H
#define __MFILE1_H

#ifndef HOSTIEEE
# include <sys/fpfp.h>
#endif /* HOSTIEEE */
# include "macdefs.h"
# include "manifest.h"

#ifdef MSG
#include "ctools_msg.h"
#define         TOOLSTR(Num, Str) NLcatgets(catd, MS_CTOOLS, Num, Str)
nl_catd catd;
#else
#define         TOOLSTR(Num, Str) Str
#endif

/*	storage classes  */
# define SNULL		0
# define AUTO		1
# define AUTOREG	2
# define PARAM		3
# define PARAMREG	4
# define PARAMFAKE	5
# define REGISTER	6
# define LABEL		7
# define ULABEL		8
# define STATIC		9
# define USTATIC	10
# define FORTRAN	11
# define UFORTRAN	12
# define EXTDEF		13
# define EXTENT		14
# define EXTERN		15
# define STNAME		16
# define UNAME		17
# define ENAME		18
# define MOS		19
# define MOU		20
# define MOE		21
# define TYPEDEF	22
# define TCSYM		23	/* Variable which is addressd off of TOC */
	/* field size is ORed in */
# define FIELD 0100
# define FLDSIZ 077
# ifndef BUG1
extern char *scnames();
# endif

/*	location counters */
# define PROG 0
# define DATA 1
# define ADATA 2
# define STRNG 3
# define ISTRNG 4
# define STAB 5
#ifdef XCOFF
# define RESET 6
#endif

/* symbol table flags */
# define SNSPACE	03	/* name space mask: */
#	define SREG		0	/* regular name */
#	define SMOS		01	/* structure member */
#	define STAG		02	/* structure tag */
#	define SLABEL		03	/* label name */
# define SNONUNIQ	04	/* non-unique structure member name */
# define SHIDDEN	010	/* currently hidden by nested definition */
# define SHIDES		020	/* definition hides earlier declaration */
# define SEXTRN		040	/* external symbol (extern or static) */
# define SSCOPED	0100	/* scoped out block scoped external symbol */
# define SSET		0200	/* symbol was assigned to */
# define SREF		0400	/* symbol value was used */
# define SLNAME		01000	/* symbol can be lname (auto variable) */
# define SPNAME		02000	/* symbol can be pname (parameter) */
#ifdef XCOFF
# define SFCALLED	04000	/* function was called */
# define SFADDR		010000	/* function address was referenced */
#endif
# define SBUILTIN	040000	/* function is a BUILTIN */
#ifdef LINT
# define SISOLDSTYLE   0100000	/* Function is Old Style and has proto */
#endif

# ifndef FIXDEF
# define FIXDEF(p)
# endif
# ifndef FIXARG
# define FIXARG(p)
# endif
# ifndef FIXSTRUCT
# define FIXSTRUCT(a,b)
# endif

# ifndef SETDCON
# define SETDCON(p) 0
# endif

	/* alignment of initialized quantities */
# ifndef AL_INIT
#	define	AL_INIT ALINT
# endif

/*	type names, used in symbol table building */
/*
** If you change this list, there are several other places that have to
** be changed:  m_dep/local.c, m_ind/tytable.h, m_ind/treewalk.h
*/
# define TNULL		000
# define TELLIPSIS	001
# define FARG		002
# define MOETY		003
# define SIGNED		004
# define UNDEF		005

/* "real" basic types start here */

# define TVOID		006
# define CHAR		007
# define SCHAR		010
# define SHORT		011
# define INT		012
# define LONG		013
# define LLONG		014
# define FLOAT		015
# define DOUBLE		016
# define LDOUBLE	017
# define STRTY		020
# define UNIONTY	021
# define ENUMTY		022
# define UCHAR		023
# define USHORT		024
# define UNSIGNED	025
# define ULONG		026
# define ULLONG		027

# define NBTYPES	030
# define NRBTYPES	(NBTYPES - TVOID)

/* type modifiers */
# define PTR		035
# define FTN		036
# define ARY		037

/* type flag/qualifier constants (must fit into a short due to dope[]) */
# define TSIGNED	040000	/* explicitly signed (for bitfields) */
# define HAVEUNALIGNED	0100000	/* struct/union has unaligned member */
# define HAVECONST	020000	/* struct/union has const member */
# define HAVEVOLATILE	010000	/* struct/union has volatile member */
# define UNALIGNED	000400
# define CONST		000200
# define VOLATILE	000100

/* type packing constants */
# define TMASK		0377	/* mask for basic type & qualifiers */
# define BTMASK		037	/* mask for basic type */

/* type encoding constants */
# define PTROUT		01
# define FTNOUT		02
# define ARYOUT		03
# define TSHIFT		2	/* number of bits for type modifiers */
# define BTSHIFT	4	/* number of bits for basic type */

/*	macros	*/
#define TOPTYPE(t)	((t)->tword&BTMASK)
#define TOPQTYPE(t)	((t)->tword&TMASK)
#define MODTYPE(t,bt)	(t=modtype(t,bt))
#define BTYPE(t)	(TOPTYPE(btype(t)))
#define ISBTYPE(t)	((t)->next==TNIL)
#define ISUNSIGNED(t)	( TOPTYPE(t) <= ULLONG && TOPTYPE(t) >= UCHAR )
#define ISINTEGRAL(t)	( (TOPTYPE(t) >= CHAR && TOPTYPE(t) <= LLONG) || \
			  (TOPTYPE(t) >= ENUMTY && TOPTYPE(t) <= ULLONG))
#define ISINT(t)        (TOPTYPE(t) == INT)
#define ISLONG(t)       (TOPTYPE(t) == LONG)
#define ISLLONG(t)      (TOPTYPE(t) == LLONG)
#define ISULONG(t)      (TOPTYPE(t) == ULONG)
#define ISULLONG(t)     (TOPTYPE(t) == ULLONG)
#define IS64BIT(t)      ( ISLONG(t) || ISULONG(t) || \
						 ISLLONG(t) || ISULLONG(t) || ISPTR(t))
#define ISCHAR(t)	(TOPTYPE(t) == UCHAR || TOPTYPE(t) == SCHAR || \
			 TOPTYPE(t) == CHAR )
#define ISFLOAT(t)	(TOPTYPE(t)<=LDOUBLE&&TOPTYPE(t)>=FLOAT)
#define ISARITHM(t)	(ISINTEGRAL(t) || ISFLOAT(t))
#define ISSCALAR(t)	(ISPTR(t) || ISARITHM(t))
#define ISTUNSIGNED(bt)	( ( (bt) <= ULLONG && (bt) >= UCHAR))
#define UNSIGNABLE(bt)	((bt)<=LLONG&&(bt)>=SCHAR)
#define ENUNSIGN(bt)	((bt)+(UNSIGNED-INT))
#define DEUNSIGN(bt)	((bt)+(INT-UNSIGNED))
#define ISPTR(t)	(TOPTYPE(t)==PTR)
#define ISFTN(t)	(TOPTYPE(t)==FTN)
#define ISARY(t)	(TOPTYPE(t)==ARY)
#define ISSTRTY(t)	(TOPTYPE(t)==STRTY)
#define DECREF(t)	((t)->next!=TNIL?(t)->next:(t))
#define INCREF(t,bt)	incref(t,bt)
#define tyalloc(bt)	(&btypes[bt])
#define ISQUALIFIED(t,qt)	((t)->tword&(qt))
#define ISCONST(t)          ((t)->tword&CONST)
#define ISVOLATILE(t)	    ((t)->tword&VOLATILE)
#define ISUNALIGNED(t)      ((t)->tword&UNALIGNED)
#define QUALIFIERS(t)       ((t)->tword&(CONST|VOLATILE|UNALIGNED))
#define ISTSIGNED(t)        ((t)->tword&TSIGNED)
#define HASCONST(t)	        ((t)->tword&HAVECONST)
#define HASVOLATILE(t)      ((t)->tword&HAVEVOLATILE)
#define HASUNALIGNED(t)	    ((t)->tword&HAVEUNALIGNED)
#define ISCONSTANT(t)       ((t)->in.op == ICON || (t)->in.op == FCON)
#define ISNUMBER(t)	(((t)->in.op == ICON && (t)->tn.rval == NONAME) || \
				(t)->in.op == FCON)
#define ISAGGREGATE(type) (TOPTYPE(type) == ARY || TOPTYPE(type) == STRTY)
#define ISUNION(type) (TOPTYPE(type) == UNIONTY)

typedef struct tyinfo *TPTR;
typedef struct parminfo *PPTR;

struct parminfo {
	TPTR	type;
	PPTR	next;
};

struct tyinfo {
	TWORD	tword;
	TWORD   spacer32;
	TPTR	next;
	union {
		unsigned size;
		PPTR	 parms;
	}	info;
};

#define typ_size	info.size
#define ary_size	info.size
#define ftn_parm	info.parms

#define TNIL (0)	/* (TPTR) */
#define PNIL (0)	/* (PPTR) */
extern struct tyinfo btypes[];	/* array containing some predefined types */

struct symtab {
#if	defined (LINT) || defined (CFLOW)
	char *ifname;	/* included filename */
	short line;	/* line number symbol occurance */
#endif
	char *psname;	/* symbol name */
	TPTR stype;	/* type word */
	char sclass;	/* storage class */
	char slevel;	/* scope level */
	short sflags;	/* flags for set, use, hidden, mos, etc. */
	int offset;	/* offset or value */
	short suse;	/* line number of last use of the symbol */
	short uniqid;	/* unique identifier number in this function */
	int nxtid;		/* Pointer to next element on the list */

};

/* Function prototypes for public symbol table manipulation functions */
extern int lookup(char *, int);
extern fixlab(int);
extern void clearst(void);
extern void debug_print_stab(int, int);
extern int ScanStab(int);
extern int StabIdInvalid(int);
#ifdef DBG_HASH
extern 	void PrintHash(void);
#endif

#ifndef DBG_HASH
# define checkst(x)
#else
extern int checkst(int);
#endif

# ifdef ONEPASS
/* NOPREF must be defined for use in first pass tree machine */
# define NOPREF 020000  /* no preference for register assignment */
# else
#ifndef OPTIMIZER
union ndu {

	struct {
		int op;
		int rall;
		TPTR type;
		short su, fpsu;
		char filler[8];	/* so name takes same space in struct */
		char *pname;
		NODE *left;
		NODE *right;
		short svsu; /* saved reg sethi ulman counter */
		short flags; /* extra flags field (lec) */
#define NOTLVAL 	2	/* to mark trees where we can't tell anymore */
#define REGSTRUCT 	4	/* Mark a register struct */
		}in; /* interior node */

	struct {
		int op;
		int rall;
		TPTR type;
		short su, fpsu;
		char filler[8];	/* so name takes same space in struct */
		char *pname;
		NODE *left;
		NODE *right;
		short svsu; /* saved reg sethi ulman counter */
		short flags; /* extra flags field (lec) */

		}frd; /* debugging node */

	struct {
		int op;
		int rall;
		TPTR type;
		short su, fpsu;
#ifdef __alpha && LINT
		char *scptr;    /* if a string constant it's ptr is here */
#ifdef __mips64
		char filler[4];	/* in cross ptrs are 32bits */
#endif
#else

		char filler[8];	/* so name takes same space in struct */
#endif
		char *pname;
		CONSZ lval;
		int rval;
		int fill1;	/* Make sure svsu and flags are aligned*/
		short svsu, flags; /* extra flags field (lec) */
#define SIDEFF 1 /* for floating register nodes, the result is also in r2,r3 */
		}tn; /* terminal node */

	struct {
		int op, rall;
		TPTR type;
		short su, fpsu;
		int label;      /* for use with branching */
		int notapplied; /* NOT has been applied - see genfpbranch */
		}bn; /* branch node */

	struct {
		int op, rall;
		TPTR type;
		short su, fpsu;
		int stsize;  /* sizes of structure objects */
		int stalign;  /* alignment of structure objects */
		}stn; /* structure node */

	struct {
		int op;
		int dummy1;
		TPTR type;
		int dummy2;
		}fn; /* front node */

	struct {
		/* this structure is used when a floating point constant
		   is being computed */
		int op;
		int dummy1;
		TPTR type;
		int dummy2;
#ifdef HOSTIEEE
		double dval;
#else
		FP_DOUBLE dval;
#endif
		}fpn; /* floating point node */

	};
#endif
# endif

/* These definitions are for lint(1) style warnings. */
# define VAL 0
# define EFF 1

# define LNAMES 250
struct lnm {
	short lid, flgs;
};
extern struct lnm lnames[LNAMES], *lnp;


extern struct sw swtab[];
extern struct sw *swp;
extern int swx;
#ifdef LINT
extern char *saved_str_ptr; /* global pointer for tracking current string const */
#endif
extern struct tyinfo labltype;

extern int nGlobTyEnts;
extern int nLoclTyEnts;
extern TPTR globTyTab;
extern TPTR loclTyTab;

extern unsigned int Initializer;
extern int foldadd;
extern unsigned int foldMask;
extern int ftnno;
extern int blevel;
extern int tempBlevel;
extern int instruct, stwart, funcstyle, volEmitted, funcConflict, paramFlg;

extern int lineno, nerrors, nmigchk;
typedef union {
	int intval;
	NODE * nodep;
	} YYSTYPE;
extern YYSTYPE yylval;

extern CONSZ lastcon;
#ifdef HOSTIEEE
extern double dcon;
#else
extern FP_DOUBLE dcon;
#endif
extern char ftitle[];
extern char ititle[];
extern int nstabents;
extern int ndiments;
extern struct symtab *stab;
#ifdef XCOFF
extern int *saved_str;
extern int max_chars;
extern int saved_chars;
extern int *saved_lab;
extern int max_strings;
extern int saved_strings;
#endif
extern int curftn;
extern int curLevel;
extern int curclass;
extern int curdim;
extern int *dimtab;
struct dnode {
	int index;
	short cextern;
	};
extern struct dnode *dimptr, dimrec[];
extern int *paramstk;
extern int *protostk;
extern int paramno;
extern int paramchk;
#ifdef LINT
extern int vflag;
#endif
#if defined(LINT) || defined(CFLOW) || defined(CXREF)
extern int func_ptr;
#endif
extern int paramlevel;
extern int autooff, argoff, strucoff;
extern int regvar;
extern int fpregvar;
extern char yytext[];

extern int strflg;
extern int strsize;

extern OFFSZ inoff;

extern int reached;

/* LINT flags. */
#define LINT_XDOLLAR	"Xdollar"	/* for checking the XDOLLAR directive */
extern int lintXdlr;    /* for the XDOLLAR  directive */
extern int lintargu;	/* for the ARGSUSED directive */
extern int lintvarg;	/* for the VARARGS directive */
extern int lintlib;	/* for the LINTLIBRARY directive */
extern int lintnrch;	/* for the NOTREACHED directive */
extern int lintused;	/* for the NOTUSED directive */
extern int lintdefd;	/* for the NOTDEFINED directive */
extern int lintrsvd;	/* for the LINTSTDLIB directive */

#if	defined(LINT) || defined(LINTP2) || defined(CFLOW) || defined(CFLOW2)

/* These three variables not needed by LINTP2, better to be general. */
extern char *pfname;		/* physical filename (stripped) */
extern char *ifname;		/* included filename (stripped) */
extern FILE *tmplint;		/* temporary output file */

/* Definitions for iocode (char). */
#define	LINTBOF		1	/* beginning of lint file */
#define	LINTEOF		2	/* end of lint file */
#define	LINTSYM		3	/* new symbol */
#define	LINTADD		4	/* add to previous symbol */
#define CFLOWBOF	5	/* beginning of cflow file */
#define CFLOWEOF	6	/* end of cflow file */

/*
** Usage definitions (short).
** Leave first two bits open for sflags&SNSPACE.
*/
#define	LINTDEF		04	/* symbol definition */
#define	LINTREF		010	/* symbol reference */
#define	LINTDCL		020	/* symbol declaration (extern) */
#define LINTMBR		040	/* symbol has member(s) */
#define LINTTAG		0100	/* symbol member has tagname */
#define LINTRET		0200	/* function has return value */
#define LINTUSE		0400	/* function return value used */
#define LINTIGN		01000	/* function return value ignored */
#define LINTLIB		02000	/* LINTLIBRARY flag */
#define LINTVRG		04000	/* VARARGSn flag */
#define LINTNOT		010000	/* NOTUSED flag */
#define LINTNDF		020000	/* NOTDEFINED flag */
#define LINTSTO		040000	/* Set only not otherwise referenced */

#endif

extern int ininit;

/*	tunnel to buildtree for name id's */

extern int idname;

extern NODE *node;
extern NODE *lastfree;

/* various labels */
extern int brklab;
extern int contlab;
extern int flostat;
extern int retlab;
extern int retstat;
extern int loop_level;
extern int asavbc[], *psavbc;

/*	flags used for name space determination */

# define SEENAME 01
# define INSTRUCT 02
# define INUNION 04
# define FUNNYNAME 010
# define TAGNAME 020
# define LABNAME 040

/*	flags used in the (elementary) flow analysis ... */

# define FBRK 02
# define FCONT 04
# define FDEF 010
# define FLOOP 020

/*	flags used for return status */

# define RETVAL 1
# define NRETVAL 2

/*	used to mark a constant with no name field */

# define NONAME 040000

/*	flags used for function prototypes */

# define OLD_STYLE 1
# define NEW_STYLE 2

/*  used to check for misuse of void and ellipsis in parameter lists */

# define SAW_VOID	1
# define SAW_ELLIP	2

	/* mark an offset which is undefined */

# define NOOFFSET (-10201)

/*	declarations of various functions */

extern NODE
	*docast(),
	*foldexpr(),
	*oldfoldexpr(),
	*buildtree(),
	*bdty(),
	*mkty(),
	*rstruct(),
	*dclstruct(),
	*getstr(),
	*tymerge(),
	*stref(),
	*offcon(),
	*bcon(),
	*bpsize(),
	*convert(),
	*pconvert(),
	*oconvert(),
	*ptmatch(),
	*tymatch(),
	*makety(),
	*block(),
	*doszof(),
	*talloc(),
	*optim(),
	*strargs(),
	*fixargs(),
	*cvtarg(),
	*clocal();

OFFSZ	tsize(),
	psize();

TWORD	ctype();

TPTR	incref(),
	btype(),
	modtype(),
	qualtype(),
	unqualtype(),
	signedtype(),
	qualmember(),
	tynalloc(),
	copytype(),
	CheckType(),
	CheckTypedef(),
	ResultType(),
	FindBType();

PPTR	parmalloc();

# define CHARCAST(x) (unsigned char)(x) /* how AIWS does it! */
# ifndef CHARCAST
/* to make character constants into character connstants */
/* this is a macro to defend against cross-compilers, etc. */
# define CHARCAST(x) (char)(x)
# endif

/* GAF: Changed tables to grow dynamically. */
#	define BCSZ 125		/* table size to save break, continue labels */
#   define HASHTSZ	5000	/* This is reasonable */
#	define SYMTSZ 	5000	/* Initial default size of symbol table, was 1500*/
#	define SYMTINC 	2000	/* Increment to realloc the symbol table */
#	define DIMTABSZ 3000	/* size of the dimension/size table was 2000*/
#	define BNEST 30		/* Block Nesting Depth */
#	define GLOBTYSZ 2000	/* global type table size */
#	define LOCLTYSZ 8000	/* local type table size */
#	define MAXBUNCH 20	/* parameter node allocation chunk */
	extern int paramsz;     /* size of the parameter stack */
	extern int protosz;     /* size of the prototype stack */
/*	special interfaces for yacc alone */
/*	These serve as abbreviations of 2 or more ops:
	ASOP	=, = ops
	RELOP	LE,LT,GE,GT
	EQUOP	EQ,NE
	DIVOP	DIV,MOD
	SHIFTOP	LS,RS
	ICOP	ICR,DECR
	UNOP	NOT,COMPL
	STROP	DOT,STREF

	*/
# define ASOP 25
# define RELOP 26
# define EQUOP 27
# define DIVOP 28
# define SHIFTOP 29
# define INCOP 30
# define UNOP 31
# define STROP 32

# define LP 50
# define RP 51
# define LC 52
# define RC 53

/*	These defines control "for" and "while" loop generation.
 *	Each of wloop_level and floop_level must be set to one
 *	of these values.
 */
# define LL_TOP  0	/* test at top of loop		*/
# define LL_BOT  1	/* test at bottom of loop	*/
# define LL_DUP  2	/* duplicate test at top and bottom of loop  */


extern int warnlevel[MXDBGFLG+1];
#ifdef LINT
extern int alphalevel[MXDBGFLG+1]; /* table to allow SUPPRESSION of various migration checks */
#endif


/* These define warning error classes for WARNING(). */
# define WALWAYS	1			/* always give warning */
# define WCONSTANT	warnlevel['C']		/* warnings for constant usages */
# define WANSI		warnlevel['a']		/* non-ansi features */
# define WUCOMPAR	warnlevel['c']		/* unsigned comparisons */
# define WUDECLAR	warnlevel['D']		/* unused declarations */
# define WDECLAR	warnlevel['d']		/* missing or re-declarations */
# define WHEURISTIC	warnlevel['h']		/* heuristic complaints */
# define WKNR		warnlevel['k']		/* K+R type code expected */
# define WLONGASSGN	warnlevel['l']		/* assignments of long values to not long variables */
# define WNULLEFF	warnlevel['n']		/* null effect usage */
# define WOBSOLETE	warnlevel['O']		/* obsolescent features */
# define WEORDER	warnlevel['o']		/* evaluation order check */
# define WPROTO		warnlevel['P']		/* prototype checks */
# define WPORTABLE	warnlevel['p']		/* portability checks */
# define WREACHED	warnlevel['R']		/* reachable code check */
# define WRETURN	warnlevel['r']		/* function return checks */
# define WSTORAGE	warnlevel['s']		/* storage packing check */
# define WUSAGE		warnlevel['u']		/* proper usage of variables and functions */
# define WUNUSED	warnlevel['U']		/* extern vars or ftn param not used */
#ifdef LINT
/* when migration checking the following provide control on WARNING msgs */
# define AL_MIGCHK      alphalevel[0]           /* Global flag for enabling migration checks */
# define AL_PRINTIT     alphalevel[1]           /* Global flag for enabling printing of errors */
# define AL_PRINTTRNC   alphalevel[2]           /* Global flag: enables constant truncation msg */
# define AL_STRCNTRL    alphalevel[3]           /* Global bitmask: Controls structure warning msg */
# define AL_PTRALG	alphalevel['a']		/* Pointer alignment problem */
# define AL_BITMASK	alphalevel['b']		/* Int long Mismatch in bit operators  */
# define AL_CASTS	alphalevel['c']		/* Problematic casts */
# define AL_FMTSTR	alphalevel['f']		/* Format control strings in scanf & printf */
# define AL_LONGASSGN	alphalevel['l']		/* Assignments of long values to not long variables */
# define AL_PTRINT	alphalevel['p']		/* Illegal combination of pointer & integer */
# define AL_SIGNEXT	alphalevel['s']		/* Problematic sign extintions to long */
# define AL_UB4SET	alphalevel['u']		/* Used before set  */
# define AL_CONSTANT    alphalevel['C']		/* Constant truncation of longs in assignment */
# define AL_FLDASG	alphalevel['F']		/* Precisionn lost in field assignment*/
# define AL_PROTO	alphalevel['P']		/* prototype checks */
# define AL_STRPTR	alphalevel['S']		/* Problematic combination of structure pointers */
# define AL_STRSIZE     alphalevel['z']         /* CAST of structures & pointers to differsizes */
# define AL_STRALG      alphalevel['g']         /* CAST of structures & pointers to differsizes */
#endif

/*
 * constant expression masks:
 * 1.	If bit 0 (from the right) is set then the expression is a
 *	constant expression.
 * 2.	If bit 1 (from the right) is set then the expression
 *	is an integral constant expression.
 *
 * EXPRESSION		-- a mask to indicate a full-fledged expression
 * GENERAL_CONSTANT	-- a mask to indicate a constant expression
 * INTEGRAL_CONSTANT	-- a mask to further restrict a constant expression
 *			   to be integral.
 * FOLD_INTEGRAL	-- are we folding an integral constant expression
 * FOLD_GENERAL		-- or are folding a general constant expression
 * NO_FOLD		-- run-time evalutation of expression only
 */
#define EXPRESSION		(0x0)
#define GENERAL_CONSTANT	(0x1)
#define INTEGRAL_CONSTANT	(0x2)
#define FOLDABLE		(INTEGRAL_CONSTANT | GENERAL_CONSTANT)
#define FOLD_INTEGRAL()		(foldMask & INTEGRAL_CONSTANT)
#define FOLD_GENERAL()		(foldMask & GENERAL_CONSTANT)
#define NO_FOLD()		(foldMask == EXPRESSION)

/*
 * initializers masks:
 * 	SINGLE	: single expression initializer;
 *	LIST	: initializer list (constant expression...see above)
 * 	HAVEINIT: a mask to indicate that we have an initialization
 *		  (for use with defid()).
 *
 *	HAVEINIT implicitly implies LIST. thus, a single expression
 * 	initializer is a further restricticion on initialization.
 *
 *	if no initialization is seen, then we have a bit pattern: 00
 */
#define NOINIT		0x0	/* bit pattern: 00 */
#define HAVEINIT	0x2	/* bit pattern: 10 */
#define SINGLE		0x3	/* bit pattern: 11 */
#define LIST		0x2	/* bit pattern: 10 */
#define ISHAVEINIT	(Initializer & HAVEINIT)
#define ISSINGLE	(Initializer == SINGLE)
#define ISLIST		(Initializer == LIST)


#endif /* __MFILE1_H */
