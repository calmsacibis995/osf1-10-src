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
static char	*sccsid = "@(#)$RCSfile: local.c,v $ $Revision: 4.2.4.6 $ (DEC) $Date: 1993/11/22 21:21:18 $";
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
 * COMPONENT_NAME: (CMDPROG) local.c
 *
 * FUNCTIONS: CanBeLNAME, LONGLINE, NAMEPTR, addAryID, addTypID, addstabx    
 *            cast, cendarg, cinit, cisreg, clocal, ctype, docast, ecode      
 *            eline, fincode, findAryID, findTypID, fixdef, fltprint          
 *            getTypeID, incode, isarray, isitfloat, isitlong, isptr, offcon  
 *            pTypeID, parray, pptr, prdef, strend, strfind, strname, tlen    
 *            vfdzero                                                         
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

/* # include <a.out.h> */
# include "mfile1.h"
# include "messages.h"

extern FILE *outfile;
extern int lastloc;
extern int eprint();

extern int startln, oldln;
extern char startfn[];
extern int bb_flags[];
extern int xdebug;
extern int gdebug;
extern int ddebug;
# define Lineno (lineno-startln+1)

extern char *rnames[];

/*	this file contains code which is dependent on the target machine */

/* -------------------- cast -------------------- */

NODE *
cast( p, t ) register NODE *p; TPTR t; {
	/* cast node p to type t */

	p = buildtree(CAST, block(NAME, NIL, NIL, t), p);
	p->in.left->in.op = FREE;
	p->in.op = FREE;
	return( p->in.right );
	}

/* -------------------- clocal -------------------- */

NODE *
clocal(p) NODE *p; {

	/* this is called to do local transformations on
	   an expression tree preparitory to its being
	   written out in intermediate code.
	*/

	/* the major essential job is rewriting the
	   automatic variables and arguments in terms of
	   REG and OREG nodes */
	/* conversion ops which are not necessary are also clobbered here */
	/* in addition, any special features (such as rewriting
	   exclusive or) are easily handled here as well */

	register struct symtab *q;
	register NODE *r;
	register o;
	register TWORD m, ml;
	TPTR mlt;

	switch( o = p->in.op ){

	case NAME:
		if( p->tn.rval < 0 ) { /* already processed; ignore... */
			return(p);
			}
		q = &stab[p->tn.rval];
		switch( q->sclass ){

		case AUTO:
		case AUTOREG:
		case PARAM:
		case PARAMREG:
			if( (q->sflags & (SLNAME|SPNAME)) != 0 )
			{
				/* have atomic id:
				 * turn it into an LNAME or PNAME
				 */
				p->in.op = ((q->sflags & SLNAME) != 0)
					? LNAME
					: PNAME;
				r = offcon(q->offset, INCREF(q->stype, PTR));
				p->tn.lval = r->tn.lval;
				r->in.op = FREE;
				if( q->sclass == AUTOREG || q->sclass==PARAMREG )
				  /* Mark tree to detect &x */
				  p->in.flags = NOTLVAL;
			}
			else
			{
				/* fake up a structure reference */
				r = block(REG, NIL, NIL,
					INCREF(tyalloc(STRTY), PTR));
				r->tn.rval = ((q->sclass==AUTO||q->sclass==AUTOREG)?STKREG:ARGREG);
				r->tn.lval = r->tn.rval; /* supply id cookie */
				p = stref(block(STREF, r, p, tyalloc(UNDEF)));
				if( q->sclass == AUTOREG || q->sclass==PARAMREG )
				  /* Mark tree to detect struct register assigns */
				  p->in.flags = REGSTRUCT;
			}
			break;

			/* Note that this was formerly after LABEL: */
		case STATIC:
			break;

		case ULABEL:
		case LABEL:
			if( q->sflags & SEXTRN ) break;
			p->tn.lval = 0;
			p->tn.rval = -q->offset;
			break;

		case REGISTER:
			{
				/* provide id cooked for optimizer */
				p->tn.lval = IsRegVar(q->offset)
					? q->uniqid
					: q->offset;
				p->in.op = REG;
				p->tn.rval = q->offset;
			}
			break;

			}
		break;

	case PCONV:
		if( p->in.left->in.op != ICON &&
				tsize(p->in.left->in.type) != SZPOINT )
			break;

		/* pointers all have the same representation; the type is inherited */

	inherit:
		p->in.left->in.type = p->in.type;
		p->in.op = FREE;
		return( p->in.left );

	case SCONV:
		m = TOPTYPE(p->in.type);
		mlt = p->in.left->in.type;
		ml = TOPTYPE(mlt);

#ifdef LINT
		if( ((SZLONG > SZINT && ( ml == LONG || ml == ULONG )) 
			 || (SZLLONG > SZINT) && (( ml == LLONG || ml == ULLONG )))
		    && m != LONG && m != ULONG && m!= LLONG && m != ULLONG  && 
		   m != UNDEF  && m != FLOAT && m != DOUBLE ) {
		/* "conversion from long may lose accuracy" */
		     if ((AL_MIGCHK && AL_LONGASSGN)) AL_PRINTIT = 1;
			WARNING( WLONGASSGN, MESSAGE( 26 ) );
		   };
		
	/* GAF: This code formerly warned that 
	 * "conversion to long may sign-extend incorrectly". This was a 
	 * result of an early bug that was corrected. However, some customers 
	 * still expect to see a warning for the migration check that 
	 * they may get an unexpected sign extension
	 */

		if( ml != LONG && ml != ULONG 
		     && ml != LLONG && ml != ULLONG 
		     && ml != PTR	/* GAF: added PTR (now 64bits) */
		     && ml != FLOAT && ml != DOUBLE
		     && ( m == LONG || m == ULONG )
		     && ( m == LLONG || m == ULLONG )
		     && p->in.left->in.op != ICON 
		     && AL_MIGCHK && AL_SIGNEXT) {
		         /* "conversion to long may sign-extend */
		         WARNING( (AL_PRINTIT = 1), MESSAGE( 27 ) );
		};

#endif

		if ((m == FLOAT || m == DOUBLE || m == LDOUBLE) !=
		    (ml == FLOAT || ml == DOUBLE || ml == LDOUBLE))
			break;

		if (NO_FOLD() && p->in.left->in.op == ICON && !ISPTR(mlt)) {
			/*
			 * simulate the conversion here
			 * ICON PTR has a name keep the SCONV. test???
			 */
			return(docast(p));
		}
		else {
			/* meaningful ones are conversion of int to char,
			   int to short, and short to char, and unsigned
			   version of them.  fields cannot have the type
			   colored down when signed-ness changes */
			if( p->in.left->in.op == FLD &&
					( ISTUNSIGNED(m) != ISTUNSIGNED(ml) ) ){
				break;
			}
			if( m==SCHAR || m==CHAR || m==UCHAR ){
				if( ml!=SCHAR && ml!=CHAR && ml!= UCHAR ) break;
				}
			else if( m==SHORT ){
				/* only get rid of redundent composes of SCONV*/
				if( (ml!=SCHAR && ml!=CHAR && ml!=UCHAR &&
				     ml!=SHORT && ml!=USHORT) ||
				    p->in.left->in.op != SCONV ) break;
				}
			else if( m==USHORT ){
				/* only get rid of redundent composes of SCONV*/
				/* (unsigned short)char is meaningful */
				if( (ml!=CHAR && ml!=UCHAR &&
				     ml!=SHORT && ml!=USHORT) ||
				    p->in.left->in.op != SCONV) break;
				}
			else if( m==FLOAT && ( ml==DOUBLE || ml==LDOUBLE)){
				break;
				}
			else if( (m==INT||m==LONG||m==LLONG) && 
					ISPTR(p->in.left->in.type) &&
                                  blevel != 0 ) {
				break;
				}
			else if( (m==INT||m==LONG||m==LLONG) && 
				(ml==USHORT || ml==UCHAR || ml == CHAR) ){
				/* put in "or of UCHAR" when doing signed chars
				   because of patterning, not programmer
				   understanding
				*/
				break;
				}
			else if( (m==INT||m==LONG||m==LLONG) && 
			         (ml == UNSIGNED || ml == ULONG || ml == ULLONG)) {
				break;
			}
			else if( (m==UNSIGNED||m==ULONG) && 
			         (ml == INT || ml == LONG || ml == LLONG)) {
				break;
			}
		}

		/* clobber conversion */
		if( tlen(p) == tlen(p->in.left) && 
		    !((m==UNSIGNED||m==ULONG||m==ULLONG) && ISPTR(mlt) && 
		      (TOPTYPE(DECREF(mlt)) == UNSIGNED ||
		      TOPTYPE(DECREF(mlt)) == ULONG ||
		      TOPTYPE(DECREF(mlt)) == ULLONG)) ) {
			if(p->in.left->in.op == QUEST) 
				break;
			else
				goto inherit;
		}
		p->in.op = FREE;
		return( p->in.left );  /* conversion gets clobbered */

	case PVCONV:
	case PMCONV:
		if( p->in.right->in.op != ICON ) cerror(TOOLSTR(M_MSG_248, "bad conversion"));
		p->in.op = FREE;
		return( buildtree( o==PMCONV?MUL:DIV, p->in.left, p->in.right ) );

	case FLD:
		/* make sure that the second pass does not make the
		   descendant of a FLD operator into a doubly indexed OREG */

		if( p->in.left->in.op == UNARY MUL
				&& (r=p->in.left->in.left)->in.op == PCONV)
			if( r->in.left->in.op == PLUS || r->in.left->in.op == MINUS )
				if( ISPTR(r->in.type) ) {
					if( ISUNSIGNED(p->in.left->in.type) )
						p->in.left->in.type =
							tyalloc(UCHAR);
					else
						p->in.left->in.type =
							tyalloc(CHAR);
				}
		break;
		}

	return(p);
	}

/* -------------------- docast -------------------- */

NODE *
docast(p)
NODE *p;
{
	register struct symtab *q;
	register NODE *r;
	register o;
	register TWORD m, ml;
	TPTR mlt;
	CONSZ val;
	unsigned long trunc_check = 0lu;
	char intstr[4] = "int\0";
	char shrtstr[6] = "short\0";
	char unsignstr[9] = "unsigned\0";
	char ushrtstr[15] = "unsigned short\0";
	char ulongstr[15] = "unsigned long\0";
	char longstr[5] = "long\0";

	char *src_type = NIL;
	char *des_type = NIL;
	/*
	 * do casts on constants
	 */

	m = TOPTYPE(p->in.type);
	mlt = p->in.left->in.type;
	ml = TOPTYPE(mlt);

	switch( p->in.op) {
	case SCONV:
		if (p->in.left->in.op == ICON && p->in.left->tn.rval == NONAME) {
			val = p->in.left->tn.lval;
			switch( m ) {
			case SCHAR:
				/* if val is neg, make lval a negative long */
				if( p->in.left->tn.lval & 0X80)
				    p->in.left->tn.lval = val | 0XFFFFFFFFFFFFFF00;
				else
				    p->in.left->tn.lval = val & 0XFF;
				break;
			case UCHAR:
			case CHAR:
				p->in.left->tn.lval = val & 0XFF;
				break;
			case USHORT:
				trunc_check = (unsigned long) USHRT_MAX;
				p->in.left->tn.lval = (unsigned short)val;
				break;
			case SHORT:
				trunc_check = (unsigned long) SHRT_MAX;
				p->in.left->tn.lval = (short)val;
				break;
			case ULONG:
			case ULLONG:
				p->in.left->tn.lval = (unsigned long)val;
				break;
			case UNSIGNED:
				trunc_check = (unsigned long) UINT_MAX;
				p->in.left->tn.lval = (unsigned)val;
				break;
			case LONG:
			case LLONG:
				p->in.left->tn.lval = (long) val;
				break;
			case INT:
				trunc_check = (unsigned long) INT_MAX;
				p->in.left->tn.lval = (int) val;
				break;
			case FLOAT:
			case DOUBLE:
			case LDOUBLE:
				if( ISUNSIGNED(p->in.left->in.type) ) {
					unsigned long i;
					/*
					 * warning: there is no unsigned
					 * integer conversion operation.
					 */
					i = p->in.left->tn.lval;
#ifdef HOSTIEEE
					p->in.left->fpn.dval = i & 0x7fffffff;
					if(i&0x80000000) {
						p->in.left->fpn.dval += 0x1;
						p->in.left->fpn.dval +=
								0x7fffffff;
					}
#else
					_FPi2d(1, i & 0x7fffffff);
					if(i&0x80000000) {
						_FPaddi(1, 0x1);
						p->in.left->fpn.dval = _FPaddi(1, 0x7fffffff);
					}
# endif
				}
				else {
#ifdef HOSTIEEE
					p->in.left->fpn.dval = p->in.left->tn.lval;
#else
					p->in.left->fpn.dval = _FPi2d(1, p->in.left->tn.lval);
# endif
				}
				p->in.left->in.op = FCON;
				break;
#ifdef COMPAT
			case TVOID:
				if( devdebug[KLUDGE] && !devdebug[COMPATIBLE] ){
					p->in.op = FREE;
					return( p->in.left );
				}
#endif
			default:
				/*
				 * otherwise, we have an unknown cast
				 */
				return(p);
			}
			p->in.left->in.type = p->in.type;
		}
		else if ( p->in.left->in.op == FCON ) {
			switch(m) {
			case FLOAT:
#ifdef HOSTIEEE
				p->in.left->fpn.dval =
				   (float) p->in.left->fpn.dval;
#else
				/*
				 * reads in a double and converts it into
				 * a float then puts it back into dval.
				 */
				p->in.left->fpn.dval =
					   _FPd2f(1, p->in.left->fpn.dval);
#endif
				break;
			case DOUBLE:
			case LDOUBLE:
#ifdef HOSTIEEE
				p->in.left->fpn.dval =
				   (double) p->in.left->fpn.dval;
#else
				if (TOPTYPE(p->in.left->in.type == FLOAT)) {
					p->in.left->fpn.dval =
						_FPf2d(1, p->in.left->fpn.dval);
				}
				else {
				/*
				 * this, essentially, does nothing but
				 * makeing sure the value in dval is a double.
				 */
					p->in.left->fpn.dval =
					     _FPcpdi(1, p->in.left->fpn->dval);
				}
#endif
				break;
			default:
				if( ISINTEGRAL(p->in.type) ){
					p->in.left->tn.lval =
#ifdef HOSTIEEE
						(long) p->in.left->fpn.dval;
#else
						_FPtrdi(1,p->in.left->fpn.dval);
#endif
					p->in.left->in.op = ICON;
					p->in.left->in.type = tyalloc(LONG);
					p = docast(p);
				}
				return(p);
			}
			p->in.left->in.type = p->in.type;
		}
		break;
	default:
		/*
		 * unknown operation.
		 */
		return(p);
	      }
#ifdef LINT
	if (trunc_check && AL_CONSTANT && AL_PRINTTRNC && AL_MIGCHK
	    && ( val > trunc_check)) {
	  switch (m) {
	  case INT:
	    des_type = intstr;
	    break;
	  case SHORT:
	    des_type = shrtstr;
	    break;
	  case UNSIGNED:
	    des_type = unsignstr;
	    break;
	  case USHORT:
	    des_type = ushrtstr;
	    break;

	  default:
	    break;
	  }; /* switch (m) */

	  switch (ml) {
	  case INT:
	    src_type = intstr;
	    break;
	  case SHORT:
	    src_type = shrtstr;
	    break;
	  case UNSIGNED:
	    src_type = unsignstr;
	    break;
	  case USHORT:
	    src_type = ushrtstr;
	    break;
	  case LONG:
	  case LLONG:
	    src_type = longstr;
	    break;
	  case ULONG:
	  case ULLONG:
	    src_type = ulongstr;
	    break;
	  default:
	    break;
	  }; /* switch (ml) */
	  WARNING ((AL_PRINTIT = 1), MESSAGE(195), src_type, des_type, opst[AL_PRINTTRNC]);
	};
	AL_PRINTTRNC = 0;
#endif
	p->in.op = FREE;
	return(p->in.left);
}

/* -------------------- cendarg -------------------- */

cendarg(){ /* at the end of the arguments of a ftn, set the automatic offset */
	autooff = AUTOINIT;
	}

/* -------------------- cisreg -------------------- */

cisreg (t)   /* is an automatic variable of type t OK for a register variable */
    TPTR t;
{
/* If a NAME becomes a REG, it becomes too hard for lint/cflow to detect. */
#if	!defined (LINT) && !defined (CFLOW)

    extern qdebug;
    TWORD ty;

    ty = TOPTYPE(t);
    if (ty == INT || ty == UNSIGNED || 
		ty == LONG || ty == ULONG ||
		ty == LLONG || ty == ULLONG ||
	qdebug && ( ty == CHAR || ty == UCHAR || ty == SHORT || ty == USHORT
			|| ty == SCHAR) || ISPTR(t))
	return(1);
    /*
    if (ty != DOUBLE)
	WERROR( ALWAYS, "type clash for register variable" );
    */
#endif
    return(0);
}

/* -------------------- CanBeLNAME -------------------- */

CanBeLNAME(t)
	TPTR t;
		/* is an automatic of type t OK for LNAME or PNAME?  */
{
	if( ISPTR(t) )
		return(1);
	else
	switch (TOPTYPE(t))
	{
		case SCHAR:
		case CHAR: case UCHAR:
		case SHORT: case USHORT:
		case INT: case UNSIGNED:
		case LONG: case ULONG:
		case LLONG: case ULLONG:
		case ENUMTY:
		case LDOUBLE:
		case DOUBLE: case FLOAT:
			return(1);
	}
	return(0);
}

/* -------------------- offcon -------------------- */

NODE *
offcon( off, t ) OFFSZ off; TPTR t; {

	/* return a node, for structure references, which is suitable for
	   being added to a pointer of type t, in order to be off bits offset
	   into a structure */

	register NODE *p;

	/* in general the type is necessary for offcon, but not now */

	p = bcon(0);
	p->tn.lval = off/SZCHAR;
	return(p);

	}

static inwd	/* current bit offsed in word */;
static long word /* word being built from fields */;

/* -------------------- incode -------------------- */

incode( p, sz ) register NODE *p; {

	/* generate initialization code for assigning a constant c
		to a field of width sz */
	/* we assume that the proper alignment has been obtained */
	/* inoff is updated to have the proper final value */
	/* we also assume sz  < SZINT */

	long val = p->tn.lval;

	if((sz+inwd) > SZINT)
		cerror(TOOLSTR(M_MSG_249, "incode: field > int"));
	val &= (1L<<sz)-1;       /* mask to correct size */
	word |= val << (32-sz-inwd);
	inwd += sz;
	inoff += sz;
	if(inoff%SZINT == 0) {
#if	!defined (CXREF) && !defined (LINT) && !defined (CFLOW)
		printf( "\t.long\t0x%lx\n", word);
#endif
		word = inwd = 0;
		}
	}

/* -------------------- fincode -------------------- */

fincode( d, sz )
#ifdef HOSTIEEE
double d;
#else
FP_DOUBLE d;
#endif
{
	/* output code to initialize space of size sz to the value d */
	/* the proper alignment has been obtained */
	/* inoff is updated to have the proper final value */
	/* on the target machine, write it out in octal! */

	outdouble(d, (sz == SZDOUBLE) ? DOUBLE :
			( (sz == SZLDOUBLE) ? LDOUBLE : FLOAT) );
	inoff += sz;
	}

/* -------------------- cinit -------------------- */

cinit( p, sz ) NODE *p; {
	extern int ddebug;
	/* arrange for the initialization of p into a space of
	size sz */
	/* the proper alignment has been opbtained */
	/* inoff is updated to have the proper final value */
	ecode( p );
	inoff += sz;

	/* let's make sure this isn't illegal initialization - see a1375 */

	if( p->in.op == INIT) {
	    if( p->in.left->in.op == ICON ||
		p->in.left->in.op == ADDR ||
		p->in.left->in.op == STADDR ||
		p->in.left->in.op == LADDR ||
		p->in.left->in.op == PADDR  ) return;
	    if (p->in.left->in.op == NAME &&
		    TOPTYPE(p->in.left->in.type) == MOE)
		return;
	    if(p->in.left->in.op == SCONV) {
		NODE *q;
		q = p->in.left->in.left;
		if( q->in.op == ICON || q->in.op == ADDR || q->in.op == STADDR ||
		    q->in.op == LADDR || q->in.op == PADDR ) return;
		if (q->in.op == NAME && TOPTYPE(q->in.type) == MOE) return;
	    }
	}
	if( ddebug ) fwalk( p, eprint, 0 );
	/* illegal initialization */
	UERROR( ALWAYS,  MESSAGE(61) );
}

/* -------------------- vfdzero -------------------- */

vfdzero( n ){ /* define n bits of zeros in a vfd */

	if( n <= 0 ) return;

	inwd += n;
	inoff += n;
	if( inoff%ALINT ==0 ) {
#if	!defined (CXREF) && !defined (LINT) && !defined (CFLOW)
		printf( "\t.long\t0x%lx\n", word );
#endif
		word = inwd = 0;
		}
	}

/* -------------------- ctype -------------------- */

TWORD
ctype (type)	/* map types which are not defined on the local machine */
    TWORD type;
{
    switch (type) {
    case LONG:
    case LLONG:
	return (INT);
    case ULONG:
    case ULLONG:
	return (UNSIGNED);
    }
    return (type);
}

/* -------------------- isitlong -------------------- */

isitlong( cb, ce ){ /* is lastcon to be long or short */
	/* cb is the first character of the representation, ce the last */

	if( ce == 'l' || ce == 'L' ||
		lastcon >= (1L << (SZINT-1) ) ) return (1);
	return(0);

	}

/* -------------------- isitfloat -------------------- */

isitfloat( s ) char *s; {
#ifdef HOSTIEEE
	double atof();
	dcon = atof(s);
#else
	dcon = ieeeatof(s);
# endif
	return( FCON );
	}

/* -------------------- ecode -------------------- */

ecode( p ) NODE *p; {

	/* walk the tree and write out the nodes.. */
	extern int contx(), WarnWalk();

	if( nerrors ) return;

	if( xdebug ) {
		printf( "ecode\n" );
		fwalk( p, eprint, 0 );
	}

	/* Walk the tree, looking for bad code to barf at. */
	if( WNULLEFF )
		fwalk( p, contx, EFF );
	lnp = lnames;
	WarnWalk( p, EFF, 0 );

# ifdef ONEPASS
	p2tree( p );
	p2compile( p );
# else
#if	!defined (CXREF) && !defined (LINT) && !defined (CFLOW)
	printf( "%c%d\t%s\n", EXPR, lineno, ftitle );
#endif
	prtree(p);
# endif
	}

/* -------------------- fixdef -------------------- */

fixdef(p) struct symtab *p; {
#ifndef	CXREF
	/* print debugging info
	 *
	 * don't do params, externs, structure names or members
	 * at this time; they are delayed until more information
	 * is known about them
	 */

	if ( !gdebug )
		return;

	switch( p->sclass ) {

	case STATIC:
		if( p->slevel != 0 )
			break;
	case EXTENT:
	case USTATIC:
	case PARAM:
	case PARAMREG:
	case PARAMFAKE:
		return;
	case ENAME:
	case STNAME:
	case UNAME:
		addTypID(p->stype->typ_size);
		return;
	case EXTERN:
	case EXTDEF:
	case MOS:
	case MOE:
	case MOU:
		if(isarray(p->stype)) {
		    parray (p);
		}
		else if(isptr(p->stype)){
		    pptr (p, 0);
		}
		return;
	default:
		if( p->sclass & FIELD ) return;
		}

	/* register parameters */
	if( p->slevel == 1 ) return;

	prdef(p,0);
#endif
}

/* local table of fakes for un-names structures
 * typ_size for .ifake is stored in mystrtab[i]
 */
#define FAKENM 300	/* maximum number of fakenames */
#define FAKESIZE 10	/* size of a fake label in chars for printf */
int mystrtab[FAKENM], ifake = 0;
struct symtab mytable;
char tagname[FAKESIZE] = "";

/* table of debugger User-Defined typedefs
 * TypIDtab contains sue/ptr User-Defined typedefs
 * AryIDtab contains array User-Defined typedefs
 * GAF: Both tables now grow upwards allowing them to be realloced.
 * Also, We do not allocate until first used.
	Nxt_TypID = TypIDtab = (int *)getmem(ndiments * sizeof(int));
	Nxt_AryID = AryIDtab = TypIDtab + ndiments - 1;
 */
#define TYPARYTABSZ (DIMTABSZ / 2)
static int *TypIDtab;          /* table containing sue/ptr debugger type IDs*/
static int *AryIDtab;          /* table containing array debugger type IDs  */
static int nTypID;	
static int nAryID;
/* Set this up as an index */
static int Nxt_TypID;
static int Nxt_AryID;

static int *ExpandIDTab(int *tab, int *entries)
{
  register int size;
  size = (*entries += TYPARYTABSZ) * sizeof(int);
  tab = reallocmem( tab, size);
  return tab;
}
  
/* -------------------- prdef -------------------- */

# define STABX  0
# define STABA  1
# define STABT  2
# define STABS  3

#ifndef _H_DBXSTCLASS
/***********************************************************************
     STORAGE CLASSES AND TYPE DEFINES MAY BE DELETED WHEN XCOFF.H
     INCLUDED.  THESE DEFINITIONS ARE FOUND IN DBXSTCLASS.H
***********************************************************************/
/*
 *   XCOFF STORAGE CLASSES AND STABSTRINGS DESIGNED SPECIFICALLY FOR DBX
 */
#define DBXMASK			0x80

#define C_GSYM			0x80
#define C_LSYM			0x81
#define C_PSYM			0x82
#define C_RSYM			0x83
#define C_RPSYM			0x84
#define C_STSYM			0x85
#define C_TCSYM			0x86
#define C_BCOMM			0x87
#define C_ECOML			0x88
#define C_ECOMM			0x89
#define C_DECL			0x8c
#define C_ENTRY			0x8d
#define C_FUN			0x8e

#define TP_INT		-1
#define TP_CHAR		-2
#define TP_SHORT	-3
#define TP_LONG		-4
#define TP_UCHAR	-5
#define TP_SCHAR	-6
#define TP_USHORT	-7
#define TP_UINT		-8
#define TP_UNSIGNED	-9
#define TP_ULONG	-10
#define TP_VOID		-11
#define TP_FLOAT	-12
#define TP_DOUBLE	-13
#define TP_LDOUBLE	-14
#define TP_PASINT	-15
#define TP_BOOL		-16
#define TP_SHRTREAL	-17
#define TP_REAL		-18
#define TP_STRNGPTR	-19
#define TP_FCHAR	-20
#define TP_LOGICAL1	-21
#define TP_LOGICAL2	-22
#define TP_LOGICAL4	-23
#define TP_LOGICAL	-24
#define TP_COMPLEX	-25
#define TP_DCOMPLEX	-26
#define TP_LLONG	-27
#define TP_ULLONG	-28
#endif

static int defntypes[NBTYPES] = { 0, 0, 0, 0, 0, 0, 0,
	TP_CHAR, TP_SCHAR, TP_SHORT, TP_INT, TP_LONG, TP_LLONG,
	TP_FLOAT, TP_DOUBLE, TP_LDOUBLE, 0, 0, 0,
	TP_UCHAR, TP_USHORT, TP_UINT, TP_ULONG, TP_ULLONG };

prdef(p,tsiz) struct symtab *p; int tsiz; {
#ifdef XCOFF
#if	!defined (CXREF) && !defined (LINT) && !defined (CFLOW)
	/* print symbol definition pseudos
	 */
	int class, saveloc, typeID;
	char type = '\0';

	if ( !gdebug )
		return;

	/* print a bb symbol if this is the first symbol in the block */

	if( blevel > 2 && !bb_flags[blevel]
		&& p->sclass != LABEL && p->sclass != ULABEL ) {
		printf( "\t.bb\t%d\n", Lineno);
		bb_flags[blevel] = 1;   /* don't let another bb print */
		}

	/* make sure that .defs in functions are in text section */
	if( blevel > 1 )
		saveloc = locctr( PROG );

	typeID = getTypeID(p);

	printf( "\t.stabx\t\"");

	/* translate storage class */

	switch( p->sclass ){
	case AUTO:
	case AUTOREG:
		class = C_LSYM;
		break;
	case EXTDEF:    /* data/undef*/
	case EXTENT:
	case EXTERN:
		class = C_GSYM;
		type = 'G';
		break;
	case STATIC:    /* data/bss */
	case USTATIC:
		class = C_STSYM;
		printf( "_");
		type = (blevel==0) ? 'S' : 'V';
		break;
	case REGISTER:
		class = (blevel==1) ? C_RPSYM : C_RSYM;
		type = 'r';
		break;
	case ULABEL:
	case LABEL:
		class = C_LABEL;
		break;
	case PARAM:
	case PARAMREG:
		class = C_PSYM;
		type = 'p';
		break;
	case TYPEDEF:
		class = C_DECL;
		type = 't';
		break;
	case TCSYM: 			/* not supported yet */
		class = C_TCSYM;
		break;
	case MOS:
	case STNAME:
	case MOU:
	case UNAME:
	case ENAME:
	case MOE:
		class = C_DECL;
		break;
	default:
		if( p->sclass & FIELD )
			class = C_DECL;
		else
			cerror(TOOLSTR(M_MSG_250, "bad storage class %d"), p->sclass );
		break;
		}

	printf( "%s:",p->psname);
	if( type=='t') {
		addTypID(0);
		printf ("t%d=", Nxt_TypID*2);
	}
	else if( type )
		printf ("%c", type);
	printf( "%d\"", typeID);

	switch( p->sclass ) {	/* print .val based on storage class */

	case AUTO:
	case AUTOREG:
	case MOS:
	case MOU:
	case PARAM:
	case PARAMREG:
		/* offset in bytes */
		printf( ",%d", p->offset/SZCHAR );
		if (p->sclass==AUTO || p->sclass==AUTOREG) printf("+L.%dL",ftnno);
		else if (p->sclass==PARAM || p->sclass==PARAMREG) printf("+L.%dA",ftnno);
		break;

	case MOE:
		/* internal value of enum symbol */
		printf( ",%d", p->offset );
		break;

	case REGISTER:
		/* offset in bytes in savearea for reg vars */
		/* actual offset determination is deferred to the asembler */
		printf( ",%d", p->offset );
		break;

	case STATIC:
	case USTATIC:
		/* actual or hidden name, depending on scope */
		if( p->sflags & SEXTRN )
			printf( ",_%s", p->psname);
		else
			printf( ",_L.%d", p->offset );
		break;
	case LABEL:
	case ULABEL:
	case EXTDEF:
	case EXTENT:
	case EXTERN:
		/* actual or hidden name, depending on scope */
		if( p->sflags & SEXTRN )
			printf( ",%s", p->psname);
		else
			printf( ",L.%d", p->offset );
		break;

	case TYPEDEF:
		/* not used */
		printf(",0");
		break;

	default:        if( p->sclass & FIELD ) {
				/* offset in bits */
				printf( ",%d", p->offset );
				break;
			}
			else
				cerror(TOOLSTR(M_MSG_251, "sdb value error on %s\n"), p->psname );
			break;
		}

	printf( ",%d", class );			/* class */
	printf(",%d", tyencode(p->stype));	/* type  */

	printf( "\n" );

	if( blevel > 1 )
		locctr( saveloc );
#endif
#endif
}


/* -------------------- getTypeID ----------------- */

getTypeID (p) register struct symtab *p;
{
	int typeID;

	if(isarray(p->stype)) {
	/* labels and arrays have the same type, need to distinguish them
	 * here.
	 */
		if (p->sclass!=LABEL&&p->sclass!=ULABEL)
		     typeID = parray(p) ;
		else typeID = pTypeID(p);
	}
	else if(isptr(p->stype))
		     typeID = pptr(p,0) ;
	else typeID = pTypeID(p) ;

	return (typeID);
}

/* -------------------- parray -------------------- */

#define MaxArray 4

parray( p ) struct symtab *p;  {
	/* print debugging info for dimensions
	 */

	TPTR temp;
	int aryDim[MaxArray];
	int dtemp;
	int typeID, ptype;

	dtemp = 0;	/* count printed dimensions */
	for (temp=p->stype; !ISBTYPE(temp) && dtemp < MaxArray; temp = DECREF(temp)) {
		/* put out a dimension for each instance of ARY in type word */
		if( ISARY(temp) ) {
				aryDim[dtemp++] = temp->ary_size;
			}
		}

	if (!(typeID = findAryID((ptype= pTypeID(p)),aryDim[--dtemp]))) {
		typeID = Nxt_AryID;
#ifdef XCOFF
#ifndef	CXREF
		printf( "\t.stabx\t\":t%d=ar0;0;%d;%d\",0,%d,0\n",
			typeID, aryDim[dtemp]-1, ptype, C_DECL);
#endif
#endif
		addAryID(ptype,aryDim[dtemp]);
  		}

	while (dtemp > 0) {
		if (!(typeID = findAryID((ptype = typeID),aryDim[--dtemp]))) {
			typeID = Nxt_AryID;
#ifdef XCOFF
#ifndef	CXREF
			printf( "\t.stabx\t\":t%d=ar0;0;%d;%d\",0,%d,0\n",
				typeID, aryDim[dtemp]-1, ptype, C_DECL);
#endif
#endif
			addAryID(ptype,aryDim[dtemp]);
  			}
	}

	if(isptr(p->stype))
		typeID = pptr(p,typeID);
	return (typeID);
}

/* -------------------- pptr ---------------------- */

#define PTRMASK 0x7FFFFFFF
#define NAMEPTR(x) ((x)|~PTRMASK)

pptr(p, typeID) struct symtab *p; int typeID;  {
	/* print debugging info for dimensions
	 */

	TPTR temp;
	int dflag=0;
	int ptype;

	for (temp = p->stype; !ISBTYPE(temp) && dflag < 4; temp = DECREF(temp))
		/* put out a dimension for each instance of PTR in type word */

	if( ISPTR(temp) ) {
		dflag++;
		if(!(typeID))
			typeID = pTypeID(p);
		ptype = typeID;
		if (!(typeID = findTypID(NAMEPTR(ptype)))) {
			addTypID(NAMEPTR(ptype));
#ifdef XCOFF
#if	!defined (CXREF) && !defined (LINT) && !defined (CFLOW)
			printf( "\t.stabx\t\":t%d=*%d\",0,%d,0\n",
				typeID = Nxt_TypID*2,ptype,C_DECL);
#endif
#endif
  			}
	}
	return(typeID);
}

/* ---------------------- pTypeID ---------------- */

pTypeID(p) register struct symtab *p;
{
	TPTR ty;
	TWORD bty;
	int typeID;

	ty = btype(p->stype);
	bty = TOPTYPE(ty);

	if (bty== STRTY || bty==UNIONTY || bty==ENUMTY ) {

		if (typeID = findTypID (ty->typ_size))
			return (typeID);
	}
	else
		return(defntypes[bty]);
}

/* -------------------- isarray -------------------- */

/* Is it an array? */
isarray(ty) register TPTR ty;
{
	for (; !ISBTYPE(ty); ty = DECREF(ty))
		if( ISARY(ty) )
			return 1;
	return 0;
}

/* -------------------- isptr ---------------------- */

/* Is it a pointer? */
isptr(ty) register TPTR ty;
{
	for (; !ISBTYPE(ty); ty = DECREF(ty))
		if( ISPTR(ty) )
			return 1;
	return 0;
}

/* -------------------- findTypID ---------------- */

findTypID(p) register int p;  /* p - typeID (top bit indicates ptr/sue) */
{
	int index=0;
	/* Note: Index is intentionally incremented before returning */
	while (index < nTypID) {
	  if(TypIDtab[index++]==p)
		return (index*2);
	}
	return (0);
}

/* -------------------- findAryID ---------------- */

findAryID(p,q) register int p; int q;  /* p - typeID */
				       /* q - array dimension */
{
	int index=0;
	/* Note: Index is intentionally incremented before returning */
	while (index < nAryID) {
	  if(AryIDtab[index] == p &&
		 AryIDtab[++index] ==q)
				return (index*2);
	  index++; /* Need to bump this a second time */
	}
	return (0);
}

/* -------------------- addTypID ---------------- */

addTypID(p) register int p;
{
  /* If not enough room in table, expand it */
  if (Nxt_TypID>=nTypID)
	TypIDtab = ExpandIDTab(TypIDtab, &nTypID);
	  
  TypIDtab[Nxt_TypID++]=p;
}

/* -------------------- addAryID ---------------- */

addAryID(p,q) register int p; register int q;
{
  /* If not enough room in table, expand it */
  if (Nxt_AryID>=nAryID)
	AryIDtab = ExpandIDTab(AryIDtab, &nAryID);

  AryIDtab[Nxt_AryID++]=p; 
  AryIDtab[Nxt_AryID++]=q; 
}

#ifdef XCOFF
/* -------------------- addstabx --------------- */

addstabx() {
#if	!defined (CXREF) && !defined (LINT) && !defined (CFLOW)
	printf("?\"\n\t.stabx\t\"");
#endif
}
#endif

#define MAXLINE 60
#define MAXDESC 20
#define LONGLINE(x,y) ((x = (x+(y)+MAXDESC))>MAXLINE) /* fit on a line? */
/* -------------------- strend -------------------- */

strend( dimst ) int dimst; {
#ifdef XCOFF
	/* called at the end of a structure declaration
	 * this routine puts out the structure tag, its members
	 * and an eos.  dimst is the index in dimtab of the
	 * structure description
	 */
	int member, size, saveloc;
	int count=1;
	char savech;
	struct symtab *memptr, *tagnm, *strfind();

	if( !gdebug ) return;

	/* set locctr to text */
	saveloc = locctr( PROG );

	/* set up tagname */
	member = dimtab[dimst + 1];
	tagnm = strfind(dimst);

	if( tagnm == NULL ) {
		/* create a fake if there is no tagname */
		/* use the local symbol table */
		tagnm = &mytable;
		if( ifake == FAKENM )
			cerror(TOOLSTR(M_MSG_254, "fakename table overflow" ));

		/* generate the fake name and enter into the fake table */
		mytable.psname = getmem( FAKESIZE+1 );
		sprintf( mytable.psname, ".%dfake", ifake );
		mystrtab[ifake++] = dimst;
		memptr = &stab[dimtab[member]];

		/* fix up the fake's class and type based on class of its members */
		switch( memptr->sclass ) {
		case MOS:
			tagnm->sclass = STNAME;
			tagnm->stype = tynalloc(STRTY);
			break;
		case MOE:
			tagnm->sclass = ENAME;
			tagnm->stype = tynalloc(ENUMTY);
			break;
		case MOU:
			tagnm->sclass = UNAME;
			tagnm->stype = tynalloc(UNIONTY);
			break;
		default:
			if( memptr->sclass & FIELD ){
				tagnm->sclass = STNAME;
				tagnm->stype = tynalloc(STRTY);
				}
			else
				cerror(TOOLSTR(M_MSG_255, "can't identify type of fake tagname" ));
			}
		tagnm->slevel = 0;;
		tagnm->stype->typ_size = dimst;
		addTypID(dimst);

        	/* print .stabx definition */
		printf( "\t.stabx\t\":T%d=",
			findTypID(btype(tagnm->stype)->typ_size));
		savech = *tagnm->psname;
		}
	else {
		/* print out the structure header */
		savech = *tagnm->psname;
		if( savech == '$' )
			*tagnm->psname = '_';

#if	!defined (CXREF) && !defined (LINT) && !defined (CFLOW)
		/* print .stabx definition */
		printf( "\t.stabx\t\"%s:T%d=", tagnm->psname,
			findTypID(btype(tagnm->stype)->typ_size));
#endif
	}

	size = (unsigned)dimtab[dimst] / SZCHAR;

	switch ( tagnm->sclass ) {
	case STNAME:	/*Structure*/
#if	!defined (CXREF) && !defined (LINT) && !defined (CFLOW)
		printf( "s%d",size);
#endif
		break;
	case ENAME:	/*Enumeration*/
#if	!defined (CXREF) && !defined (LINT) && !defined (CFLOW)
		printf( "e" );
#endif
		break;
	case UNAME:	/*Union*/
#if	!defined (CXREF) && !defined (LINT) && !defined (CFLOW)
		printf( "u%d",size);
#endif
		break;
	default:
		cerror(TOOLSTR(M_MSG_256, "can't identify type of tagname"));
		}
	*tagnm->psname = savech;

	/* print out members */
	while( dimtab[member] >= 0 ) {
		memptr = &stab[dimtab[member++]];
		if (LONGLINE(count,strlen(memptr->psname))) {
			addstabx();
			count = 0;
			}
		savech = *memptr->psname;
		if( savech == '$' )
			*memptr->psname = '_';
#if	!defined (CXREF) && !defined (LINT) && !defined (CFLOW)
		if( tagnm->sclass == ENAME)
			printf( "%s:%d,", memptr->psname, memptr->offset);
		else {
			size = tsize( memptr->stype );
			printf( "%s:%d,%d,%d;", memptr->psname,
			getTypeID(memptr), memptr->offset, size );
			}
#endif
		*memptr->psname = savech;
		}

#if	!defined (CXREF) && !defined (LINT) && !defined (CFLOW)
	if( tagnm->sclass == ENAME)
	    printf(";\",%d,%d,%d\n", 0, C_DECL, tyencode(tagnm->stype));
	else
	    printf("\",%d,%d,%d\n", 0, C_DECL, tyencode(tagnm->stype));
#endif

	/* return to old locctr */
	locctr( saveloc );
#endif
	}

/* -------------------- strfind -------------------- */

struct symtab *
strfind( key ) int key; {
	/* find the structure tag in the symbol table, 0 == not found
	 */
	register int id = 0;  
	char spc;
	while(id = ScanStab(id)) {
		spc = stab[id].sclass;
		if( (spc == STNAME || spc == ENAME || spc == UNAME ) &&
				stab[id].stype->typ_size == key &&
				TOPTYPE(stab[id].stype) != TNULL )
			return( &stab[id]);
		}
	/* not found */
	return( NULL );
	}

/* -------------------- strname -------------------- */

char *
strname( key ) int key; {
	/* return a pointer to the tagname,
	 * the fake table is used if not found by strfind
	 */
	int i;
	struct symtab *tagnm, *strfind();
	tagnm = strfind( key );
	if( tagnm != NULL )
		return( tagnm->psname );

	for( i = 0; i < FAKENM; ++i )
		if( mystrtab[i] == key ) {
			sprintf( tagname, ".%dfake", i );
			return( tagname );
			}

	printf(TOOLSTR(M_MSG_287, "structure tagname not found\n" ));
/*
	cerror(TOOLSTR(M_MSG_287, "structure tagname not found" ));
*/
	return(NULL);
	}

#ifndef ONEPASS

/* -------------------- tlen -------------------- */

tlen(p) NODE *p;
{
	switch (TOPTYPE(p->in.type)) {
		case SCHAR:
		case CHAR:
		case UCHAR:
			return(1);

		case SHORT:
		case USHORT:
			return(2);

		case LDOUBLE:
		case DOUBLE:
			return(8);

		default:
			return(4);
		}
	}

/* -------------------- fltprint -------------------- */

fltprint(p)
	register NODE *p;
{
	printf( "%o\t%o\t", p->fpn.dval ); /* YECH */
}

#endif

/* -------------------- eline -------------------- */

eline()
{
	/* generate a new line number breakpoint if
	 * the line number has changed.
	 */
	if( gdebug && lineno != oldln ) {
		oldln = lineno;
		if( lastloc == PROG && strcmp( startfn, ftitle ) == 0 )
			printf( "\t.line\t%d\n", Lineno );
		}
}

