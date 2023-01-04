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
static char	*sccsid = "@(#)$RCSfile: pftn.c,v $ $Revision: 4.2.6.5 $ (DEC) $Date: 1993/11/22 21:22:27 $";
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
 * COMPONENT_NAME: (CMDPROG) pftn.c
 *
 * FUNCTIONS: AttachProto, CheckEnum, CheckQualifier, CheckStruct, CheckType 
 *            CheckTypedef, CountMembers, CreateProto, DiagnoseType           
 *            FakeNamealloc, InitParse, OutArgType, OutArguments, OutFileBeg  
 *            OutFileEnd, OutFtnDef, OutFtnRef, OutFtnUsage, OutMembers       
 *            OutSymbol, OutType, ResultType, SeenType, StabInfoPrint         
 *            TagStruct, WriteType, beginit, bstruct, checkst, chkty          
 *            clearst, dclargs, dclstruct, defid, deftents, doinit            
 *            dumpstack, endinit, extrndec, falloc, fixclass, fixlab          
 *            fixtype, ftnarg, ftnend, getFakeName, getstr, gotscal, hide     
 *            ilbrace, inforce, instk, irbrace, iscall, lookup, makeghost     
 *            markaddr, mknonuniq, moedef, nidcl, oalloc, protopop            
 *            protopush, psave, putbyte, rstruct, savestr, strip      
 *            talign, tsize, tymerge, tyreduce, uclass, unbuffer_str, unhide  
 *            upoff, vfdalign, yyaccpt, yyerror, CopyStabEnt, ListLook, hash
 *            FreeStabEnt, debug_print_stab, ScanStab, StabIdInvalid, PrintHash
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

#include <stdio.h> 		/* for printf */
#include <stdlib.h> 	/* for realloc */
#include <string.h>
/* AIWS C compiler */
# include "mfile1.h"
# include "messages.h"

#ifdef LINT
extern char *bycode();
#endif
/* --------------------- New Symbol Table Functions ---------------------------
	The symbol table and its use has been modified to allow it to
	be dynamically expandable. The new algorithm is explained below.	
	There are many places in the code where pointer are used to point
	to symbol table entires, but these are mostly used locally. Those
	which were used accross functions where the symbol table could
	be realloced have been explicitly reset, or access is by 
	subscript. 

1. Statically allocate a fixed length hash table which points to the 
   first entry in the stab for this hash. Note that the hashing
   algorithm was not changed. The statistics appear to be ok, in
   that for several runs only about 10% of the inserts are collisions.
   

2. The stab becomes a heap. Add a field to the stab as the index to
   the next entry for this hash such that for each hash, we have a linked
   list of entries (by index).

3. Add a freelist pointer(index value) and a last-used pointer (index
   value). stab[0] is the head of the free list, and is never used.

4. If the STAB is full, use reallocmem to grow it. Note that
   reallocmem is similar to getmem in that it calls cerror if it
   runs out of memory.

5. Lookups are much more efficient. In general, the algorithm is:
	hash to a bucket. If the bucket is empty then the symbol can be
	simply placed into the slot. If it is non empty, then follow the
	list to find a match on name and namespace. 

6. Sequential searches of the symbol table is performed more
   straighforwardly also. A ScanStab function is used to get the
   index to the next logical entry in the stab.

7. Deletions from the Stab are much different.
   Previously, entries were freed by setting the stype to TNULL, and
   in some contexts this sometimes required that symbols be rehashed
   into the slots. Now, freeing a symbol, is done by freeing the
   allocated name (which was not done before because 2 stab entries
   could actually point to the same allocated name), removing the
   stab entry from the linked list at its hash slot, and putting the
   entry on the free list for later use. The entry stype is still
   set to TNULL more as a sanity check. In those routines which 
   searched the STAB and checked for TNULL, I added a call to cerror
   if found because this constituted an inconsistency in the stab.  
	
---------------------------------------------------------------------------	
*                     Functions Added 
*   Name        Sclass    Description
*  CopyStabEnt  static    Copies the fields from the source to destination.
*  ListLook     static    Traverses the link list for an entry within namespace
*  hash         static    Computes the index into the hash table 0<= M <HASHTSZ
*  FreeStabEnt  static    Frees a stab entry, and places it into the free list
*  debug_print_stab ext   Prints the contents of a stab entry.
*  ScanStab     ext       Returns the next logical entry in the stab.
*  StabIdInvalid ext      Returns TRUE if the stab index is invalid.
*  PrintHash     ext      Prints statistics about the stab.
---------------------------------------------------------------- */
/*  ----------- Prototypes for new or changed functions 
 * Note that all the stab manipulation functions have been prototyped
 * because soem arguments changed, and I wanted to make sure that all
 * occurrences would be caught at compile time.
 */
static int mknonuniq(int);
static int hide(int, int);
static void unhide(int);
static int externdec(int);
static void CopyStabEnt(int, int);
static int ListLook(char *, int, int);
static int hash(const char *);
static void FreeStabEnt(int);

extern int minsvarg;  /* minimum offset of an arg to save */
extern int adebug;
extern int bdebug;
int odebug = 0; /* Used in output functions */
extern int aflag;
extern void printtyinfo( TPTR typtr, int flag);

#define INIT_STACK_SIZE 32

struct instk {
	int in_sz;   /* size of array element */
	int in_x;    /* current index for structure member in structure initializations */
	int in_n;    /* number of initializations seen */
	TPTR in_t;    /* type */
	int in_id;   /* stab index */
	int in_fl;   /* flag which says if this level is controlled by {} */
	OFFSZ in_off;  /* offset of the beginning of this level */
	}
instack[INIT_STACK_SIZE],
*pstk, *tempstack;

	/* defines used for getting things off of the initialization stack */


/*
 * a flag to inhibit multiple warnings of partial 
 * elided initialization.
 * perhaps a global mechanism that works for all 
 * errors is better
 */
static int partelided = 0; 

int ddebug = 0;
static int LocalUniqid = REGSZ+1;
static int GlobalUniqid = 1;
static int RecurseCnt = 0;

int paramsz = 150; /* variable for increasing size of paramstk */
int protosz = 20;  /* variable for increasing size of protostk */

/* -------------------- dumpstack -------------------- */

/*
 * this macro is used to debug the initialization stack
 */
#define dumpstack(which) {\
	struct instk *temp = instack; \
	int i =0; \
	printf("-->%s\n@ n\t\tsz x n t id fl off\n", which); \
	for (; temp <= pstk; i++, temp++) { \
		printf("%d 0x%x\t%d %d %d 0x%x %d %d %d\t%s\n", i, temp, \
			temp->in_sz, temp->in_x, temp->in_n, \
			temp->in_t, temp->in_id, \
			temp->in_fl, temp->in_off, stab[temp->in_id].psname);\
	}\
}

/* -------------------- defid -------------------- */

defid( q, class )  NODE *q; {
	register struct symtab *p;
	int idp;
	TPTR type;
	TPTR stp;
	int scl;
	int slev;
	TPTR temp;

	if( q == NIL ) return;  /* an error was detected */

	if( q < node || q >= &node[ntrnodes] ) cerror(TOOLSTR(M_MSG_216, "defid call" ));

	idp = q->tn.rval;

	if( idp < 0 ) cerror(TOOLSTR(M_MSG_217, "tyreduce" ));
	p = &stab[idp];

# ifndef BUG1
	if( ddebug ){
		printf( "defid( %s (%d), ", p->psname, idp );
		tprint( q->in.type );
		printf( ", %s ), level %d\n", scnames(class), blevel );
		}
# endif

	fixtype( q, class );

	type = q->in.type;
	class = fixclass( class, type );

	stp = p->stype;
	slev = p->slevel;

# ifndef BUG1
	if( ddebug ){
		printf( "\tmodified to " );
		tprint( type );
		printf( ", %s\n", scnames(class) );
		printf( "\tprevious def'n: " );
		tprint( stp );
		printf( ", %s ), level %d\n", scnames(p->sclass), slev );
		}
# endif

	if( ISFTN(type) && ( class == EXTDEF || class == STATIC ||
			class == FORTRAN ) ) {
		curftn = idp;
		funcConflict = 0;  
	}

	if( blevel == 1 && TOPTYPE(stp) != FARG ){
		switch( class ){
		case PARAM:
		case PARAMREG:
		case PARAMFAKE:
		case REGISTER:
			/* "declared argument %s is missing" */
			UERROR( ALWAYS, MESSAGE(28), p->psname );
			break;
		}
	}
	if( TOPTYPE(stp) == UNDEF || TOPTYPE(stp) == FARG ){
		goto enter;
	}
	if( ISFTN(stp) && TOPTYPE(DECREF(stp)) == UNDEF && p->sclass == SNULL )
		/* name encountered as function, not yet defined */
		goto enter;

	if( !comtypes( type, stp, 0 ) ) goto mismatch;

	/*
	** Detect old-style function definitions in the presence of
	** a new-style prototype declaration.
	*/
	if( ISFTN(stp) && stp->ftn_parm != PNIL && type->ftn_parm == PNIL &&
			( class == EXTDEF || class == STATIC ||
				class == FORTRAN ) ){
		/* "using old-style argument definition in presence of
		 *  prototype" */
#ifdef LINT
	        if (AL_MIGCHK && AL_PROTO) AL_PRINTIT = 1;
#endif
		WARNING( WPROTO, MESSAGE(153) );
		funcConflict = 1;  /* signal dclargs that this happened */
	} 

	scl = p->sclass;

# ifndef BUG1
	if( ddebug ) printf( "\tprevious class: %s\n", scnames(scl) );
# endif

	if( class & FIELD ){
		/* redefinition */
		if( !falloc( p, class&FLDSIZ, 1, NIL ) ) {
			/* successful allocation */
			if( instruct & INUNION ) strucoff = 0;
			psave( idp );
			return;
			}
		/* blew it: resume at end of switch... */
		}

	else switch( class ){

	case EXTERN:
		switch( scl ){
		case STATIC:
			if( slev != 0 )
				break;
		case USTATIC:
		case EXTDEF:
		case EXTENT:
		case EXTERN:
		case FORTRAN:
		case UFORTRAN:
			goto match;
		}
		break;

	case EXTENT:
	case EXTDEF:
		switch( scl ){
		case EXTDEF:
			if( class == EXTDEF )
				break;
			class = EXTDEF;
		case EXTENT:
			/* "redeclaration of %s" */
			WARNING( WDECLAR, MESSAGE(96), p->psname );
		case EXTERN:
			p->sclass = class;
			goto match;
		case USTATIC:
			if( ISFTN(type) ){
				p->sclass = STATIC;
				goto match;
			}
			break;
		}
		break;

	case USTATIC:
		switch( scl ){
		case USTATIC:
			if( ISFTN(type) )
				goto match;
		case STATIC:
			/* "redeclaration of %s" */
			WARNING( WDECLAR, MESSAGE(96), p->psname );
			goto match;
		case EXTERN:
			if( !ISFTN(type) && p->suse < 0 )
				/* "%s declared both static and extern" */
				UERROR( ALWAYS, MESSAGE(164), p->psname );
			else
				/* "%s declared both static and extern" */
				WERROR( devdebug[COMPATIBLE],
					MESSAGE(164), p->psname );
			p->sclass = USTATIC;
			goto match;
		}
		break;

	case STATIC:
		if( blevel != 0 )
			break;
		if( scl == USTATIC ){
			if( !ISFTN(type) ){
				/* "redeclaration of %s" */
				WARNING( WDECLAR, MESSAGE(96), p->psname );
			}
			p->sclass = STATIC;
			goto match;
		} else if( scl == EXTERN ){
			if( !ISFTN(type) && p->suse < 0 )
				/* "%s declared both static and extern" */
				UERROR( ALWAYS, MESSAGE(164), p->psname );
			else
				/* "%s declared both static and extern" */
				WERROR( devdebug[COMPATIBLE],
					MESSAGE(164), p->psname );
			p->sclass = STATIC;
			goto match;
		}
		break;

	case UFORTRAN:
		if( scl == UFORTRAN || scl == FORTRAN ){
			goto match;
		}
		break;

	case FORTRAN:
		if( scl == UFORTRAN ){
			p->sclass = FORTRAN;
			goto match;
		}
		break;

	case ULABEL:
		if( scl == LABEL || scl == ULABEL )
			return;
		break;

	case LABEL:
		if( scl == ULABEL ){
			p->sclass = LABEL;
			deflab( p->offset );
			return;
		}
		break;

	case STNAME:
	case UNAME:
	case ENAME:
		if( scl == class && slev == blevel &&
				dimtab[type->typ_size] == 0 )
			/* previous entry just a mention */
			return;
		break;

	case MOS:
	case MOU:
	case MOE:
	case PARAM:
	case PARAMREG:
	case PARAMFAKE:
	case AUTO:
	case AUTOREG:
	case REGISTER:
	case TYPEDEF:
		break;	/* mismatch.. */
	}

mismatch:	/* Resolve the redeclaration */
	/*
	** Allow nonunique structure/union member names.
	*/
	if( class == MOS || class == MOU || ( class & FIELD ) ){
		/* Make a new entry */
		int *memp;

		p->sflags |= SNONUNIQ;  /* old entry is nonunique */
		/* Determine if name has occurred in this structure/union */
		if (paramno == 0)
			cerror(TOOLSTR(M_MSG_218, "paramstk error" ));
		for( memp = &paramstk[paramno-1]; *memp >= 0; --memp ){
			register struct symtab *sym = &stab[*memp];

			if( sym->sclass == STNAME || sym->sclass == UNAME )
				break;
			if( sym->sflags & SNONUNIQ ){
				if( strcmp( p->psname, sym->psname ) )
					continue;
				/* "illegal redeclaration of %s" */
				UERROR( ALWAYS, MESSAGE(160), p->psname );
				break;
			}
		}
		/* Update p and idp to new entry */
		idp = mknonuniq( idp );
		p = &stab[idp];
		goto enter;
	}
	/*
	** Allow hiding of declarations.
	*/
	if( blevel > slev && class != EXTERN && class != USTATIC &&
			class != UFORTRAN && class != ULABEL &&
			class != LABEL ){
		if( slev == 1 && blevel == 2 )
			/* "redeclaration of parameter %s inside function" */
			WERROR( devdebug[SCOPING], MESSAGE(185), p->psname );
#if defined (DBG_HASH)
		if (ddebug) {
		  if ((p-stab) != idp) {
			printf("Pointer to %s(%d) != idp:%s(%d)\n", 
				   p->psname, p-stab, stab[idp].psname, idp);
		  } else {
			printf("Calling hide: %s(%d)\n", 
				   p->psname, idp);
		  }
		}
#endif
		q->tn.rval = idp = hide( idp, 0 );
		p = &stab[idp];
		goto enter;
	  }

	/*
	** For global declarations inside blocks, use the appropriate
	** global symbol.
	*/
	if( blevel > slev && !( p->sflags & SEXTRN ) && ( class == EXTERN ||
			class == USTATIC || class == UFORTRAN ) ) {
		/* Find the hidden declaration at the highest level */
		if( ( idp = extrndec( p ) ) >= 0 ){
			/* Make a new entry for the external symbol */
			if( comtypes(q->in.type, stab[idp].stype, 0) ){
				if( slev == 1 && blevel == 2 )
					/* "redeclaration of parameter %s inside function" */
					WERROR( devdebug[SCOPING], MESSAGE(185),
						p->psname );
				goto match;
			}
		} else if( !ISFTN(type) || devdebug[SCOPING] ){
			/* Didn't find existing one.  Have to create it. */
#if defined (DBG_HASH)
		  if (ddebug) {
			if ((p-stab) != idp) {
			  printf("Pointer to %s(%d) != idp:%s(%d)\n", 
					 p->psname, p-stab, stab[idp].psname, idp);
			} else {
			  printf("Calling hide: %s(%d)\n", 
					 p->psname, idp);
			}
		  }
#endif
		  q->tn.rval = idp = hide( idp, 0 );
		  p = &stab[idp];
		  if( slev == 1 && blevel == 2 )
			/* "redeclaration of parameter %s inside function" */
			WERROR( devdebug[SCOPING], MESSAGE(185),
				   p->psname );
		  goto enter;
		}
	}

	/*
	** We tried!
	*/
	/* "illegal redeclaration of %s" */
	UERROR( ALWAYS, MESSAGE(160), p->psname );
	return;

match:	/* Update the symbol type to the composite type */
	if( mkcomposite( p->stype, type, p->slevel ) )
		/* incomplete type for %s has already been completed */
		WARNING( WDECLAR, MESSAGE(145), p->psname );
	if( blevel > p->slevel ){
		/* Make new entry to detect redeclaration in same scope */
#if defined (DBG_HASH)
	  if (ddebug) {
	  if ((p-stab) != idp) {
		printf("Pointer to %s(%d) != idp:%s(%d)\n", 
			   p->psname, p-stab, stab[idp].psname, idp);
	  } else {
		printf("Calling hide: %s(%d)\n", 
			   p->psname, idp);
	  }
	}
#endif
	  q->tn.rval = hide( idp, 1 );
	  p = &stab[q->tn.rval];
	  /* Note: This used to be a structure copy, 
	   * but we can't do that any more. If there are more
	   * places, we should add a function to do the copy.
	   */
	  CopyStabEnt(q->tn.rval, idp);
	  p->sflags = ( stab[idp].sflags & (SNSPACE|SEXTRN) ) | SHIDES;
	  p->slevel = blevel;
	}
	return;

enter:	/* Make a new entry */

# ifndef BUG1
	if( ddebug ) printf( "\tnew entry made\n" );
# endif
	if( TOPTYPE(type) == TVOID ){
		if( class == PARAMFAKE ){
			paramFlg |= SAW_VOID;
		} else if( class != TYPEDEF ){
			/* "void type for %s" */
			UERROR( ALWAYS, MESSAGE(117), p->psname );
			type = tyalloc(INT);
		}
	}
	if( class == STNAME || class == UNAME || class == ENAME ){
		type = tynalloc(TOPTYPE(type));	/* Make a new node */
		type->typ_size = curdim;
		if (curdim >= ndiments) 
		  cerror("CURDIM >= NDIMENTS at %s in %s\n", __LINE__, __FILE__);
		dstash( 0 );		/* size */
		dstash( -1 );		/* index to members */
		dstash( ALSTRUCT );	/* alignment */
		dstash( idp );		/* tag symbol */
	}
	p->stype = type;
	p->sclass = class;
	p->slevel = blevel;
	p->offset = NOOFFSET;
	p->suse = lineno;
#if	defined (LINT) || defined (CFLOW)
	p->line = lineno;	/* symbol definition line number, p->suse
				   gets clobbered by each reference */
#endif

	/* allocate offsets */
	if( class&FIELD ){
		falloc( p, class&FLDSIZ, 0, NIL );  /* new entry */
		if( instruct & INUNION ) strucoff = 0;
		psave( idp );
		}
	else switch( class ){

	case AUTO:
	case AUTOREG:
		/*
		 * defer automatic arrays of unknown size till later.
		 * N.B.: see below for the same test.
		 */
		if (!(ISARY(q->in.type) && (q->in.type->ary_size == 0)
			&& ISHAVEINIT))
			oalloc( p, &autooff );
#		ifdef ONEPASS
			break;
#		endif
	case PARAM:
	case PARAMREG:
	case PARAMFAKE:
			p->uniqid = LocalUniqid++;
#ifndef XCOFF
#	ifndef ONEPASS
		if( class == AUTO || class == AUTOREG )
			if (!(ISARY(q->in.type) && (q->in.type->ary_size == 0)
				&& ISHAVEINIT))
				StabInfoPrint(p);
#	endif
#endif
		if( CanBeLNAME(p->stype) )
			p->sflags |= (class == AUTO || class == AUTOREG) ?
				SLNAME : SPNAME;
		break;

	case REGISTER:
		p->offset = ISFLOAT(type) ? fpregvar--: regvar--;
		p->uniqid = LocalUniqid++;
		if( blevel == 1 || paramlevel > 0 ) p->sflags |= SSET;
#ifndef XCOFF
#		ifndef ONEPASS
			if( blevel != 1 && paramlevel == 0)
				StabInfoPrint(p);
#		endif
#endif
		break;

	case EXTERN:
	case EXTENT:
	case EXTDEF:
	case USTATIC:
	case UFORTRAN:
	case FORTRAN:
		if( blevel > 0 && ISFTN(type) && !devdebug[SCOPING] ){
			p->slevel = 0;
			dimptr->cextern = 1;
			p->stype = copytype(p->stype, 0);
			p->sflags |= SEXTRN;

			/* Detect redeclaration in same scope */
#if defined (DBG_HASH)
			if (ddebug) {
			if ((p-stab) != idp) {
			  printf("Pointer to %s(%d) != idp:%s(%d)\n", 
					 p->psname, p-stab, stab[idp].psname, idp);
			} else {
			  printf("Calling hide: %s(%d)\n", 
					 p->psname, idp);
			}
		  }
#endif
			q->tn.rval = hide( idp, 1 );
			p = &stab[q->tn.rval];
			/* Note: This used to be a structure copy, 
			 * but we can't do that any more. If there are more
			 * places, we should add a function to do the copy.
			 */
			CopyStabEnt(q->tn.rval, idp);
			p->sflags = ( stab[idp].sflags & (SNSPACE|SEXTRN) ) | SHIDES;
			p->slevel = blevel;
			idp = q->tn.rval;
		  }
		p->sflags |= SEXTRN;
	case STATIC:
		p->uniqid = GlobalUniqid++;
		p->offset = getlab();
#ifndef XCOFF
#		ifndef ONEPASS
			StabInfoPrint(p);
#		endif
#endif
		if( blevel == 0 || ( ISFTN(type) && !devdebug[SCOPING] ) ){
		  register struct symtab *r;
		  register int tempid = p - stab;
#if defined (DBG_HASH)
		  if (ddebug > 2) {
			if (tempid != idp) {
			  printf("DEFID SCOPING:Pointer to %s(%d) != idp:%s(%d)\n", 
					 p->psname, p-stab, stab[idp].psname, idp);
			} else {
			  printf("DEFID SCOPING:Duplicating %s(%d)\n", 
					 p->psname, idp);
			}
		  }
#endif
			p->sflags |= SEXTRN;
			idp = lookup( p->psname, ( p->sflags & SNSPACE ) | SSCOPED );
		    r = &stab[idp];
		    p = &stab[tempid]; /* Lookup can reallocate stab */
			if( TOPTYPE(r->stype) == UNDEF ){
			  FreeStabEnt(idp);
			  idp = tempid; /* don't want to invalidate index */
			} else if( ISFTN(p->stype) == ISFTN(r->stype) ){
				if( !comtypes( p->stype, r->stype, 0 ) )
					/* "external symbol type clash for %s" */
					WARNING( WDECLAR, MESSAGE(193),
						p->psname );
				p->suse = - lineno;
#ifdef XCOFF
				p->sflags |= r->sflags &
						(SSET|SREF|SFCALLED|SFADDR);
#else
				p->sflags |= r->sflags & (SSET|SREF);
#endif
	/*
	** At this point, we have a redundant symtab entry for the scoped-out
	** extern.  Unfortunately, we can't remove it now or at any other
	** convenient time, since its removal will cause the actual symbol
	** table entry to move into the newly vacated hole, which breaks
	** function definitions quite badly.  Therefore, use the old entry
	** position for the new symbol and mark the former position for removal
	** at the end of the next function (not now, since parameters would
	** get screwed up).
	*/
				/* GAF: With the new structure, we can probably
				 * do a removal, but, for now let's keep what we got
				 * Copy p's data to r
				 */
				CopyStabEnt(idp, tempid);
				p->slevel = 1;
				p->sflags |= SSCOPED;
				p = r;
				q->tn.rval = idp;
				if( curftn >= 0 )
					curftn = idp;
			} else {
				/* "external symbol type clash for %s" */
				UERROR( ALWAYS, MESSAGE(193), p->psname );
			}
			idp = q->tn.rval;
		}
		break;

	case ULABEL:
	case LABEL:
		p->offset = getlab();
		p->slevel = 2;
		if( class == LABEL ){
			locctr( PROG );
			deflab( p->offset );
			}
		break;

	case MOS:
	case MOU:
		oalloc( p, &strucoff );
		if( class == MOU ) strucoff = 0;
		psave( idp );
		break;

	case MOE:
		p->offset = strucoff++;
		psave( idp );
		break;
		}

	/* user-supplied routine to fix up new definitions */

	FIXDEF(p);

# ifndef BUG1
	if( ddebug ) printf( "\toffset: %d\n", p->offset );
# endif

	}

# ifndef ONEPASS
#ifndef XCOFF
StabInfoPrint(p)
	register struct symtab *p;
{
#if defined DBG_HASH
  debug_print_stab(ddebug, p - stab);
#endif
#if	!defined (CXREF) && !defined (LINT) && !defined (CFLOW)
	int id;		/* unique identifier to pass to the optimizer */
	TPTR type;	/* type to pass to the optimizer	*/
	int op;		/* op that will go on tree nodes	*/
 	NODE *t;	/* tree node for ENUM kludge */

	type = p->stype;
	id   = p->uniqid;

	switch( p->sclass )
	{
		case PARAM:
		case PARAMREG:
			op = PNAME;
			break;

		case AUTO:
		case AUTOREG:
			op = LNAME;
			break;

		case REGISTER:
			/* Register variables are known by unique id
			 * cookies, all others are known by register
			 * number
			 */
			if( ! IsRegVar(p->offset) )
				id = p->offset;
			op = REG;
			break;

		case STATIC:
			if( p->slevel > 1 )
				id = - p->offset;
			op = NAME;
			break;

		default:
			op = NAME;
			break;
	}

 	if( BTYPE(type) == ENUMTY )
 	{
 		/* Pass 1 always lies about ENUMs, even on the lhs of
 		 * assignments.  We must do likewise.
 		 */

 		t = talloc();
 		t->in.op = op;
 		t->in.type = type;
 		econvert(t);
 		t->in.op = FREE;
 		type = t->in.type;
 	}

	printf ("%c%d\t%o\t%s\t_%s\n", STABINFO, id, tyencode(type),
		opst[op], p->psname);
#endif
}
#endif

# endif ONEPASS

/* -------------------- psave -------------------- */

psave( i ){
	if( paramno >= paramsz ){ /* then make paramstk big enough */
		paramsz = paramno + 150;
		paramstk = (int *)reallocmem(paramstk,paramsz*sizeof(int));
		if ( paramstk == NULL )
			cerror(TOOLSTR(M_MSG_219, "parameter stack overflow"));
		}
	paramstk[ paramno++ ] = i;
	}

/* -------------------- ftnend -------------------- */

ftnend(){ /* end of function */
	if( retlab != NOLAB ){ /* inside a real function */
		efcode();
		LocalUniqid = REGSZ + 1;
		}
	checkst(0);
	retstat = 0;
	tcheck();
	brklab = contlab = retlab = NOLAB;
	flostat = 0;
	if( nerrors == 0 ){
		if( psavbc != & asavbc[0] ) cerror(TOOLSTR(M_MSG_220, "bcsave error"));
		if( paramno != 0 ) cerror(TOOLSTR(M_MSG_221, "parameter reset error"));
		if( swx != 0 ) cerror(TOOLSTR(M_MSG_222, "switch error"));
		}
	psavbc = &asavbc[0];
	paramno = 0;
	autooff = AUTOINIT;
	regvar = MAXRVAR;
	fpregvar = MAXFPREG;
	reached = 1;
	swx = 0;
	swp = swtab;
	curftn = -1;
	locctr(DATA);
	}

/* -------------------- dclargs -------------------- */

dclargs(){
	register i, j;
	register struct symtab *p;
	register NODE *q;
	register TPTR t;
	register PPTR parm;
	PPTR CreateProto();
    extern PPTR copyparms();

	argoff = ARGINIT;
# ifndef BUG1
	if( ddebug > 2) printf("dclargs()\n");
# endif
	if (funcConflict) {
		parm = stab[curftn].stype->ftn_parm;
		if( TOPTYPE(parm->type) == TVOID ){
			parm = parm->next;
		}
#if defined (LINT)
	} else {
	  /* GAF: Attach arguments for function for local checking */
	  if (paramno && stab[curftn].stype->ftn_parm == NULL) {
	    stab[curftn].stype->ftn_parm = copyparms(CreateProto(0), 0);
	    stab[curftn].sflags |= SISOLDSTYLE;
	  }
#endif /* LINT */
	}
	for( i=0; i<paramno; ++i ){
		if( (j = paramstk[i]) < 0 ) continue;
		p = &stab[j];
		if (TOPTYPE(p->stype) == TVOID) {
			/* parameter list has a void, act
			 * as if there were no parameters seen.
			 */
			paramno = 0;
			break;
		}
# ifdef	CXREF
		CXDefName(j, p->suse);	/* params of functions */
# endif
# ifndef BUG1
		if( ddebug > 2 ){
			printf("\t%s (%d) ",p->psname, j);
			tprint(p->stype);
			printf("\n");
			}
# endif
		if( p->sflags & SHIDES ){
			/* "%s redefinition hides earlier one" */
			WARNING( (WDECLAR || WHEURISTIC) && WKNR, MESSAGE(2), p->psname );
		}
		if (p->sclass == PARAMFAKE) {
			/* "no name for definition parameter" */
			UERROR( ALWAYS, MESSAGE(155) );
		}
		if (TOPTYPE(p->stype) == FARG) {
			q = block(FREE, NIL, NIL, tyalloc(INT));
			q->tn.rval = j;
			defid( q, PARAM );
			}
		FIXARG(p); /* local arg hook, eg. for sym. debugger */
		if (funcConflict) {
			if( parm == PNIL ){
				/* "wrong number of arguments
				 *  in function definition"
				 */
#ifdef COMPAT
				if( devdebug[KLUDGE] && !devdebug[COMPATIBLE] )
					WERROR( ALWAYS, MESSAGE(165) );
				else
					UERROR( ALWAYS, MESSAGE(165) );
#else
				UERROR( ALWAYS, MESSAGE(165) );
#endif
				/* prevent multiple error messages */
				funcConflict = 0;
			} else {
				if (ISINTEGRAL(p->stype)) {
					t = tyalloc( prmtint(block(FREE, NIL,
							   NIL, p->stype)));
				} else if (TOPTYPE(p->stype) == FLOAT) {
					t = tyalloc(DOUBLE);
				} else {
					t = unqualtype( p->stype );
				}
				if( !comtypes( parm->type, t, 0 ) ){
					/* "prototype type mismatch of
					    formal parameter %s" */
#ifdef COMPAT
/************************************************************************
** NOTE:  This must not be used if the compiler is modified to pass	*
** <4 byte parameters, or else bad code will result!			*
************************************************************************/
					if( devdebug[KLUDGE] &&
							!devdebug[COMPATIBLE] ) {
#ifdef LINT
					       if (AL_MIGCHK && AL_PROTO) AL_PRINTIT = 1;
#endif

						WERROR( ALWAYS, MESSAGE(187),
							p->psname );
					     }
					else {

#ifdef LINT
					       if (AL_MIGCHK && AL_PROTO) AL_PRINTIT = 1;
#endif
						UERROR( ALWAYS, MESSAGE(187),
							p->psname );
					      };
#else
#ifdef LINT
				        if (AL_MIGCHK && AL_PROTO) AL_PRINTIT = 1;
#endif
					UERROR( ALWAYS, MESSAGE(187),
						p->psname );
#endif
				}
				parm = parm->next;
			}

		}
		if ( TOPTYPE(p->stype) != TELLIPSIS) {
			/* always set aside space,
			 * even for register arguments
			 */
			oalloc( p, &argoff );
		} else {
			/* this parameter is just an ellipsis
			 * marker, it always appears at the
			 * top of the parameter stack so decrement
			 * paramno to avoid seeing it in bfcode.
			 */
			paramno--;
		}

	}

	if (funcConflict && parm != PNIL) {
		/* "wrong number of arguments in function definition" */
#ifdef COMPAT
		if( devdebug[KLUDGE] && !devdebug[COMPATIBLE] )
			WERROR( ALWAYS, MESSAGE(165) );
		else
			UERROR( ALWAYS, MESSAGE(165) );
#else
		UERROR( ALWAYS, MESSAGE(165) );
#endif
	}
	cendarg();
	locctr(PROG);
	defalign(ALINT);
	++ftnno;
	bfcode( paramstk, paramno );
	paramno = 0;
	}

/* -------------------- rstruct -------------------- */

NODE *
rstruct( idn, soru ){ /* reference to a structure or union, with no definition */
	register NODE *q;

	switch (TOPTYPE(stab[idn].stype)) {
	case UNDEF:
	def:
		q = block(FREE, NIL, NIL, tyalloc(UNDEF));
		q->tn.rval = idn;
		if( soru & INSTRUCT ){
			q->in.type = tyalloc(STRTY);
			defid( q, STNAME );
		} else if( soru & INUNION ){
			q->in.type = tyalloc(UNIONTY);
			defid( q, UNAME );
		} else {
			/* "unknown enumeration" */
			WERROR( devdebug[COMPATIBLE], MESSAGE(167) );
			q->in.type = tyalloc(ENUMTY);
			defid( q, ENAME );
		}
		break;

	case STRTY:
		if( soru & INSTRUCT ) break;
		goto def;

	case UNIONTY:
		if( soru & INUNION ) break;
		goto def;

	case ENUMTY:
		if( !(soru&(INUNION|INSTRUCT)) ) break;
		goto def;

		}
	stwart = instruct;
	q = mkty( stab[idn].stype );
	q->tn.rval = idn;	/* added for needs of TagStruct() */
	stab[idn].suse = -lineno;
	return( q );
	}

/* -------------------- moedef -------------------- */

moedef( idn ){
	register NODE *q;

	if( idn >= 0 ){
		q = block(FREE, NIL, NIL, tyalloc(MOETY));
		q->tn.rval = idn;
		defid( q, MOE );
		}
	}

/* -------------------- bstruct -------------------- */

bstruct( idn, soru ){ /* begining of structure or union declaration */
	register NODE *q;
	register struct symtab *s;

	psave( instruct );
	psave( curclass );
	psave( strucoff );
	strucoff = 0;
	instruct = soru;
	q = block(FREE, NIL, NIL, tyalloc(UNDEF));
	if( ( q->tn.rval = idn ) >= 0 ){
		s = &stab[idn];
		q->in.type = s->stype;
		}
	if( instruct==INSTRUCT ){
		curclass = MOS;
		if( idn >= 0 ){
			if( s->sclass != STNAME )
				q->in.type = tyalloc(STRTY);
			defid( q, STNAME );
			}
		}
	else if( instruct == INUNION ) {
		curclass = MOU;
		if( idn >= 0 ){
			if( s->sclass != UNAME )
				q->in.type = tyalloc(UNIONTY);
			defid( q, UNAME );
			}
		}
	else { /* enum */
		curclass = MOE;
		if( idn >= 0 ){
			if( s->sclass != ENAME )
				q->in.type = tyalloc(ENUMTY);
			defid( q, ENAME );
			}
		}
	psave( idn = q->tn.rval );
	/* the "real" definition is where the members are seen */
	if ( idn >= 0 )
		stab[idn].suse = lineno;
	return( paramno-4 );
	}

/* -------------------- TagStruct -------------------- */

TagStruct(q)
register NODE *q;
{
	/*
	** Tag an incomplete structure or union declaration.
	*/
	register struct symtab *s;

	if( q->tn.rval < 0 )
		cerror(TOOLSTR(M_MSG_223, "tagging unknown structure name" ));
	s = &stab[q->tn.rval];
	if( blevel > s->slevel )
		defid( q, s->sclass );
	else
		/* "redeclaration of %s" */
		WARNING( WDECLAR, MESSAGE(96), s->psname );
}

/* -------------------- dclstruct -------------------- */

NODE *
dclstruct( oparam ){
	register struct symtab *p;
	register i, al, sa, j, sz, szindex;
	register TWORD temp;
	register high, low;
	TWORD qual = 0;
	TPTR type;

	/* paramstack contains:
		paramstack[ oparam ] = previous instruct
		paramstack[ oparam+1 ] = previous class
		paramstk[ oparam+2 ] = previous strucoff
		paramstk[ oparam+3 ] = structure name

		paramstk[ oparam+4, ... ]  = member stab indices

		*/


	if( (i=paramstk[oparam+3]) < 0 ){
		szindex = curdim;
		dstash( 0 );  /* size */
		dstash( -1 );  /* index to member names */
		dstash( ALSTRUCT );  /* alignment */
		dstash( -lineno );	/* name of structure */
		}
	else {
		szindex = stab[i].stype->typ_size;
		}

# ifndef BUG1
	if( ddebug || curdim >= ndiments || szindex >= ndiments){
		printf( "dclstruct( %s ), szindex = %d, params:%d, curdim:%d, ndiments:%d\n",
			(i>=0)? stab[i].psname : "??", 
			   szindex, paramno, curdim, ndiments );
		}
# endif
	if( instruct & INSTRUCT )
		temp = STRTY;
	else if( instruct & INUNION )
		temp = UNIONTY;
	else {
		temp = ENUMTY;
		type = tynalloc(MOETY);
		type->typ_size = szindex;
		}
	stwart = instruct = paramstk[ oparam ];
	curclass = paramstk[ oparam+1 ];
	dimtab[ szindex+1 ] = curdim;
	if ((szindex+1) >= ndiments || curdim >= ndiments) 
	  cerror("Dimention table overflow at %s in %s\n", __LINE__, __FILE__);
	al = ALSTRUCT;

	high = low = 0;

	for( i = oparam+4;  i< paramno; ++i ){
		dstash( j=paramstk[i] );
		if( StabIdInvalid(j) ) 
		  cerror(TOOLSTR(M_MSG_224, "gummy structure member" ));

# ifndef BUG1
	if( ddebug ){
		printf( "param:%d id:%d", i, j);
		debug_print_stab(1, j);
		}
# endif
		p = &stab[j];
		if( temp == ENUMTY ){
			if( p->offset < low ) low = p->offset;
			if( p->offset > high ) high = p->offset;
			p->stype = type;
			continue;
			}
		sa = talign( p->stype );
		if( p->sclass & FIELD ){
			sz = p->sclass&FLDSIZ;
			}
		else {
			sz = tsize( p->stype );
			}
		if(  sz > ( unsigned ) strucoff ) strucoff = sz;  /* for use with unions */
		SETOFF( al, sa );
		/* set al, the alignment, to the lcm of the alignments of the members */

		/*
		** If any members are qualified, propagate the qualifications
		** to the struct/union type.
		*/
		type = p->stype;
		if (!devdebug[STRUCTBUG])
			while (ISARY(type))
				type = DECREF(type);
		if (ISCONST(type) || HASCONST(type))
			qual |= HAVECONST;
		if (ISVOLATILE(type) || HASVOLATILE(type))
			qual |= HAVEVOLATILE;
		}
	dstash( -1 );  /* endmarker */
	SETOFF( strucoff, al );

	if( temp == ENUMTY ){
		register TWORD ty;

# ifdef ENUMSIZE
		ty = ENUMSIZE(high,low);
# else
		if( (char)high == high && (char)low == low )
			ty = SCHAR;
		else if( (short)high == high && (short)low == low )
			ty = SHORT;
		else
			ty = INT;
# endif
		strucoff = tsize(tyalloc(ty));
		dimtab[szindex+2] = al = talign(tyalloc(ty));
		}

	if( strucoff == 0 ){
		/* "zero sized structure" */
		UERROR( ALWAYS, MESSAGE(121) );
		strucoff = SZINT;
		al = ALINT;
		}
	dimtab[ szindex ] = strucoff;
	dimtab[ szindex+2 ] = al;
	dimtab[ szindex+3 ] = paramstk[ oparam+3 ];  /* name index */

	FIXSTRUCT( szindex, oparam ); /* local hook, eg. for sym debugger */
# ifndef BUG1
	if( ddebug>1 || (szindex+3) >= ndiments){
		printf( "\tdimtab[%d,%d,%d] = %d,%d,%d\n",
			szindex,szindex+1,szindex+2,
			dimtab[szindex],dimtab[szindex+1],dimtab[szindex+2] );
		for( i = dimtab[szindex+1]; dimtab[i] >= 0; ++i ){
			printf( "\tmember %s(%d)\n",
				stab[dimtab[i]].psname, dimtab[i] );
			}
		}
# endif

	strucoff = paramstk[ oparam+2 ];
	paramno = oparam;

	if ((i = paramstk[oparam+3]) >= 0) {
		type = stab[i].stype;
	} else {
		type = tynalloc(temp);
		type->typ_size = szindex;
	}
	if (qual)
		type = qualmember(type, qual);

	return (mkty(type));
	}

/* -------------------- yyerror -------------------- */

	/* VARARGS */
yyerror( s ) char *s; { /* error printing routine in parser */
	UERROR( ALWAYS, s );
	}

/* -------------------- yyaccpt -------------------- */

yyaccpt(){
	ftnend();
	}

/* -------------------- ftnarg -------------------- */

ftnarg( idn ) {
	switch (TOPTYPE(stab[idn].stype)) {

	case UNDEF:
		/* this parameter, entered at scan */
		if( stab[idn].slevel < blevel )
			idn = hide( idn, 0 );
		break;
	default:
		switch( stab[idn].sclass ){
		case PARAM:
		case PARAMREG:
		case REGISTER:
			if( stab[idn].slevel == blevel ){
				/* "redeclaration of formal parameter, %s" */
				UERROR( ALWAYS, MESSAGE(97), stab[idn].psname );
				goto enter;
			}
			break;
		}
		idn = hide( idn, 0 );
		break;
	case TNULL:
		cerror(TOOLSTR(M_MSG_225, "unprocessed parameter" ));
	}
enter:
	stab[idn].stype = tyalloc(FARG);
	stab[idn].sclass = PARAM;
	stab[idn].slevel = blevel;
	psave( idn );
	return( idn );
}

/* -------------------- talign -------------------- */

talign( ty ) register TPTR ty; {
	/* compute the alignment of an object with type ty */

	for (; !ISBTYPE(ty); ty = DECREF(ty)) {
		switch (TOPTYPE(ty)) {

		case FTN:
			return( ALFTN );
		case PTR:
			return( ALPOINT );
		case ARY:
			continue;
			}
		}

	switch (TOPTYPE(ty)) {

	case UNIONTY:
	case ENUMTY:
	case STRTY:
		return( (unsigned int) dimtab[ ty->typ_size+2 ] );
	case SCHAR:
	case CHAR:
	case UCHAR:
	case TVOID:
		return( ALCHAR );
	case FLOAT:
		return( ALFLOAT );
	case DOUBLE:
		return( ALDOUBLE );
	case LDOUBLE:
		return( ALLDOUBLE );
	case LONG:
	case ULONG:
		return( ALLONG );
	case SHORT:
	case USHORT:
		return( ALSHORT );
	default:
		return( ALINT );
		}
	}

/* -------------------- tsize -------------------- */

OFFSZ
tsize( ty ) register TPTR ty; {
	/* compute the size associated with type ty */
	/* BETTER NOT BE CALLED WHEN ty REFERS TO A BIT FIELD... */

	OFFSZ mult;

	mult = 1;
	for( ; !ISBTYPE(ty); ty = DECREF(ty) ){
		switch( TOPTYPE(ty) ){
		case FTN:
			/* "cannot take size of a function" */
			UERROR( ALWAYS, MESSAGE(170) );
			return( SZINT );
		case PTR:
			return( SZPOINT * mult );
		case ARY:
			if( ty->ary_size == 0 ){
				/* "unknown array size" */
				UERROR( ALWAYS, MESSAGE(114) );
				ty->ary_size = 1;
			}
			mult *= ty->ary_size;
			continue;
		}
	}

	if( dimtab[ty->typ_size] == 0 ){
		switch( TOPTYPE(ty) ){
		case TVOID:
			/* "illegal use of void type" */
			UERROR( ALWAYS, MESSAGE(147) );
			return( SZINT * mult );
			/*NOTREACHED*/
			break;
		case STRTY:
		case UNIONTY:
			/* "undefined structure or union" */
			UERROR( ALWAYS, MESSAGE(112) );
		case ENUMTY:	/* Error handled in rstruct() */
			dimtab[ty->typ_size] = SZINT;
			break;
		default:
			cerror(TOOLSTR(M_MSG_226, "unknown size for type 0%o"), TOPTYPE(ty) );
		}
	}
	return( (OFFSZ) dimtab[ty->typ_size] * mult );
}

/* -------------------- chkty -------------------- */

chkty( q ) register struct symtab *q; {
	register TPTR ty;

	for( ty = q->stype; !ISBTYPE(ty); ty = DECREF(ty) ){
		switch( TOPTYPE(ty) ){
		case FTN:
			cerror(TOOLSTR(M_MSG_227, "defining function variable %s"), q->psname );
		case PTR:
			return;
		case ARY:
			if( ty->ary_size == 0 ){
				/* "unknown array size for %s" */
				UERROR( ALWAYS, MESSAGE(168), q->psname );
				ty->ary_size = 1;
			}
			continue;
		}
	}

	if( dimtab[ty->typ_size] == 0 ){
		switch( TOPTYPE(ty) ){
		case STRTY:
		case UNIONTY:
			/* "undefined structure or union for %s" */
			UERROR( ALWAYS, MESSAGE(169), q->psname );
		case ENUMTY:	/* Error handled in rstruct() */
			dimtab[ty->typ_size] = SZINT;
			break;
		default:
			cerror(TOOLSTR(M_MSG_228, "unknown size for %s type 0%o"), q->psname,
				TOPTYPE(ty) );
		}
	}
}

/* -------------------- inforce -------------------- */

inforce( n ) OFFSZ n; {  /* force inoff to have the value n */
	/* inoff is updated to have the value n */
	OFFSZ wb;
	register rest;
	/* rest is used to do a lot of conversion to ints... */

	if( inoff == n ) return;
	if( inoff > n ) {
		cerror(TOOLSTR(M_MSG_229, "initialization alignment error"));
		}
	wb = inoff;
	SETOFF( wb, SZINT );

	/* wb now has the next higher word boundary */

	if( wb >= n ){ /* in the same word */
		rest = n - inoff;
		vfdzero( rest );
		return;
		}

	/* otherwise, extend inoff to be word aligned */

	rest = wb - inoff;
	vfdzero( rest );

	/* now, skip full words until near to n */

	rest = (n-inoff)/SZINT;
	zecode( rest );

	/* now, the remainder of the last word */

	rest = n-inoff;
	vfdzero( rest );
	if( inoff != n ) cerror(TOOLSTR(M_MSG_230, "inoff error"));

	}

/* -------------------- vfdalign -------------------- */

vfdalign( n ) int n;
{ /* make inoff have the offset the next alignment of n */
	OFFSZ m;

	m = inoff;
	SETOFF( m, n );
	inforce( m );
	}

int idebug = 0;

int ibseen = 0;  /* the number of } constructions which have been filled */

int iclass;  /* storage class of thing being initialized */

int ilocctr = 0;  /* location counter for current initialization */

/* -------------------- beginit -------------------- */

beginit(curid){
	/* beginning of initilization; set location ctr and set type */
	register struct symtab *p;

# ifndef BUG1
	if( idebug >= 3 ) printf( "beginit(), curid = %d\n", curid );
# endif

	p = &stab[curid];

	iclass = p->sclass;
	switch( iclass ){

	case UNAME:
		return;
	case EXTERN:
	case AUTO:
	case AUTOREG:
	case REGISTER:
		break;
	case EXTDEF:
	case STATIC:
		ilocctr = ISARY(p->stype)?ADATA:DATA;
		locctr( ilocctr );
		defalign( talign( p->stype ) );
		defnam( p );

		}

	inoff = 0;
	ibseen = 0;
	partelided = 0;
	pstk = 0;

	instk( curid, p->stype, inoff );

	}

/* -------------------- instk -------------------- */

instk( id, t, off ) OFFSZ off; TPTR t; {
	/* make a new entry on the parameter stack to initialize id */

	register struct symtab *p;

	tempstack = pstk;

	for(;;){
# ifndef BUG1
		if( idebug ) printf( "instk((%d, %o, %d)\n",
			id, tyencode(t), off );
# endif

		/* save information on the stack */

		if ( (pstk+1) > &instack[INIT_STACK_SIZE]) {
			if (idebug)
				dumpstack("Initialization Stack Overflow");
			cerror(TOOLSTR(M_MSG_231, "Initialization Stack Overflow"));
		}

		if( !pstk ) { 
			pstk = instack;
			tempstack = pstk;
		}
		else ++pstk;

		pstk->in_fl = 0;	/* left brace flag */
		pstk->in_id =  id ;
		pstk->in_t =  t ;
		pstk->in_n = 0;  /* number seen */
		pstk->in_x = (TOPTYPE(t) == STRTY || TOPTYPE(t) == UNIONTY)?
				dimtab[t->typ_size+1] : 0;

		pstk->in_off =  off;   /* offset at beginning of this element */
		/* if t is an array, DECREF(t) can't be a field */
		/* INS_sz has size of array elements, and -size for fields */
		if( ISARY(t) ){
			pstk->in_sz = tsize( DECREF(t) );
			}
		else if( stab[id].sclass & FIELD ){
			pstk->in_sz = - ( stab[id].sclass & FLDSIZ );
			}
		else {
			pstk->in_sz = 0;

			}

		/* now, if this is not a scalar, put on another element */

		if( ISARY(t) ){
			t = DECREF(t);
			continue;
			}
		else if (TOPTYPE(t) == STRTY) {
			id = dimtab[pstk->in_x];
			if (id<0 || id>nstabents ) {
				/* "illegal structure initialization" */
				UERROR( ALWAYS, MESSAGE(176) );
				return;
			}
			p = &stab[id];
			if( p->sclass != MOS && !(p->sclass&FIELD) ) {
				/* "illegal structure initialization" */
				UERROR( ALWAYS, MESSAGE(176) );
				return;
			}
			t = p->stype;
			off += p->offset;
			continue;
			}
		else if (TOPTYPE(t) == UNIONTY) {
			id = dimtab[pstk->in_x];
			if (id<0 || id>nstabents ) {
				/* "illegal union initialization" */
				UERROR( ALWAYS, MESSAGE(177) );
				return;
			}
			p = &stab[id];
			if( p->sclass != MOU && !(p->sclass&FIELD) ) {
				/* "illegal union initialization" */
				UERROR( ALWAYS, MESSAGE(177) );
				return;
			}
			t = p->stype;
			off += p->offset;
			continue;
		}
		else return;
	}
}

/* -------------------- getstr -------------------- */

NODE *
	/* decide if the string is external or an initializer,	*/
	/* and get the contents accordingly 			*/
getstr(){
	register l;
#ifndef XCOFF
	register temp;
#endif
	register NODE *p;
#ifndef XCOFF
# ifndef ONEPASS
	struct symtab STEnt;
# endif
#endif

	if (	(iclass==EXTDEF||iclass==STATIC) &&
		( TOPTYPE(pstk->in_t) == CHAR  ||
		  TOPTYPE(pstk->in_t) == SCHAR ||
		  TOPTYPE(pstk->in_t) == UCHAR ) && pstk!=instack &&
		ISARY(pstk[-1].in_t) ) {

		/* treat "abc" as { 'a', 'b', 'c', 0 } */

		strflg = 1;
		Initializer = LIST;
		pstk[-1].in_fl = 1; /* simulate left brace -- hardwired */
		inforce( pstk->in_off );
		/* if the array is inflexible (not top level), pass in the
		 * size and be prepared to throw away unwanted initializers */
				/* get the contents */
		lxstr((pstk-1)!=instack?(pstk-1)->in_t->ary_size:0);
		irbrace();  /* simulate right brace */
		Initializer = NOINIT;
		return( buildtree( STRING, NIL, NIL ) );
		}
	else { /* make a label, and get the contents and stash them away */
		Initializer = LIST;
		if( iclass != SNULL ){ /* initializing */
			/* fill out previous word, to permit pointer */
			vfdalign( ALPOINT );
			}
					/* set up location counter */
#ifndef XCOFF
		temp = locctr( blevel==0?ISTRNG:STRNG );
#endif
		/* ROMP has a Dhrystone problem..its too slow */
		/* character string constants (& char arrays) */
		/* are word aligned (crm) */
#ifdef XCOFF
		if (saved_strings >= max_strings) {
		    saved_lab = (int *) reallocmem(saved_lab,
				 (max_strings+100)*sizeof(int));
		    max_strings += 100;
		    }
		saved_lab[saved_strings++] = l = getlab();
#else
#if	!defined (CXREF) && !defined (LINT) && !defined (CFLOW)
		printf("\t.align\t2\n");
#endif
		deflab( l = getlab() );
#endif
		strflg = 0;
#ifdef  LINT
		saved_str_ptr = NIL;
#endif
		lxstr(0); /* get the contents */
#ifndef XCOFF
		locctr( blevel==0?ilocctr:temp );
#endif
		p = buildtree( STRING, NIL, NIL );
#ifdef LINT
		p->tn.scptr = saved_str_ptr; /* save the string in node  */
		saved_str_ptr = NIL;
#endif
		p->tn.rval = -l;
#ifndef XCOFF
# ifndef ONEPASS
		/* print out fake symbol table info for this string */
		STEnt.sclass = STATIC;
		STEnt.slevel = 2;
		STEnt.stype = p->in.type;
		STEnt.offset = l;
		STEnt.psname = "string!";
		StabInfoPrint(&STEnt);
# endif ONEPASS
#endif
		Initializer = NOINIT;
		return(p);
		}
	}

#ifdef XCOFF
/* -------------------- savestr -------------------- */

savestr(val)
int val;
{
	if (saved_chars >= max_chars) {
		saved_str = (int *) reallocmem(saved_str,
			    (max_chars + 2000) * sizeof(int));
		max_chars += 2000;
		}
	saved_str[saved_chars++] = val;
}

/* -------------------- unbuffer_str() -------------------- */

/*      while there are more strings to output
	    print align 2
	    print label
	    print the string (ends with -1)
	    print 0
	    print align 2
*/

unbuffer_str()
{
	register i = 0;
	register j = 0;
	register k;

	if (i >= saved_strings) return;

	locctr( STRNG );

	while (i < saved_strings) {
#if	!defined (CXREF) && !defined (LINT) && !defined (CFLOW)
		printf("\t.align\t2\n");
#endif
		deflab(saved_lab[i++]);
		for (k=0; saved_str[j] != -1; )
			bycode(saved_str[j++], k++);
		j++;    /* move past that -1 */
		bycode(0, k++);
#ifdef LINT
		free (bycode(-1,k));
#else
		bycode(-1,k);
#endif
#if	!defined (CXREF) && !defined (LINT) && !defined (CFLOW)
		printf("\t.align\t2\n");
#endif
		}

	saved_strings = saved_chars = 0;

	locctr( PROG );
}
#endif

/* -------------------- putbyte -------------------- */

putbyte( v ){ /* simulate byte v appearing in a list of integer values */
	register NODE *p;
	p = bcon(v);
	incode( p, SZCHAR );
	tfree( p );
	gotscal();
	}

/* -------------------- endinit -------------------- */

endinit(){
	register TPTR t;
	register n, d1;

# ifndef BUG1
	if( idebug ) printf( "endinit(), inoff = %d\n", inoff );
# endif

	foldMask = EXPRESSION;

	switch( iclass ){

	case EXTERN:
		if (blevel) WERROR( ALWAYS, MESSAGE(19) );
	case AUTO:
	case AUTOREG:
	case REGISTER:
		return;
		}

	pstk = instack;

	t = pstk->in_t;
	n = pstk->in_n;

	if( ISARY(t) ){
		d1 = t->ary_size;

		vfdalign( pstk->in_sz );  /* fill out part of the last element, if needed */
		n = inoff/pstk->in_sz;  /* real number of initializers */
		if( d1 >= n ){
			/* once again, t is an array, so no fields */
			inforce( tsize( t ) );
			n = d1;
			}

		/*
		 * if the number of characters in the string (including
		 * the terminating null) equals the size of the
		 * array plus 1 (i.e., d1) then we warn about
		 * the fact that the null is not guarranteed to be there at
		 * the end of the char array.
		 */
		if (ISCHAR(DECREF(t)) && (d1+1) == n && d1 != 0) {
			/* "array not large enough to store terminating null" */
			WARNING( WSTORAGE, MESSAGE(175) );
			goto endendinit;
		}

		/*
		 * "too many initializers" or 
		 * "non-null byte ignored in string initializer"
		 */
		if( d1!=0 && d1!=n ) {
			if (ISCHAR(DECREF(t)))
				WARNING( ALWAYS, MESSAGE(81) );
			else
				UERROR( ALWAYS, MESSAGE(108) );
		}

		if( n == 0 ){
			/* "empty array declaration" */
			UERROR( ALWAYS, MESSAGE(35) );
			n = 1;
			}
		t->ary_size = n;
		}

	else if (TOPTYPE(t) == STRTY || TOPTYPE(t) == UNIONTY) {
		/* clearly not fields either */
		inforce( tsize( t ) );
		}
	/* "bad scalar initialization" */
	else if( n > 1 ) UERROR( ALWAYS, MESSAGE(17) );
	/* this will never be called with a field element... */
	else inforce( tsize( t ) );

endendinit:
	paramno = 0;
	vfdalign( AL_INIT );
	inoff = 0;
	iclass = SNULL;

	}

/* -------------------- doinit -------------------- */

extern eprint();

doinit( p ) register NODE *p; {

	register sz;
	register TPTR t;
	register NODE *l;
	register struct instk *itemp = pstk;

	if (idebug>2)
		dumpstack("doinit initial stack dump");
	/*
	 * check for partial elidation ...
	 * search for topmost aggregate, and that should have
	 * a left brace associated with it, or otherwise 
	 * we have a partial elidation.
	 */
	for( ; itemp > (instack+1); ) {
		--itemp;
		t = itemp->in_t;
		if (ISAGGREGATE(t)) {
			if (itemp->in_fl == 0) {
				/* "partially elided initialization" */
				WARNING(partelided == 0 && WANSI && WKNR, MESSAGE(180));
				partelided++;
			}
			break;
		}
	}

	/*
	 * take care of generating a value for the initializer p.
	 *
	 * inoff has the current offset (i.e., last bit written)
	 * in the current word being generated.
	 *
	 * note: size of an individual initializer is assumed
	 * to fit into an int.
	 *
	 * reset the idname from the stack. however, for scalars
	 * we use the top of the stack, while for aggregates
	 * we use the stack entry at tempstack.
	 *
	 * first thing is to determine the FOLD_EXPR mode depending on
	 * the block level and storage class. 
	 */
	foldMask = iclass == STATIC || blevel==0? GENERAL_CONSTANT:EXPRESSION;

	if (TOPTYPE(pstk->in_t) == ENUMTY) 
		idname = pstk->in_id;
	else
		idname = tempstack->in_id;

	if( iclass < 0 ) goto leave;

	if( iclass == AUTO || iclass == AUTOREG || iclass == REGISTER || iclass == EXTERN ) {
		/*
		 * for scalars:
		 * do the initialization and get out, without regard
		 * for filing out the variable with zeros, etc.
		 * for aggregates or unions:
		 * 	1. initialize a data segment area with the structure.
		 * 	2. reserve an area on the stack suitable to hold
	 	 * the object.
		 * 	3. perform a structure assignment from the data
		 * segment to the stack area.
		 */
		bccode();
		ininit = 1;
		p = buildtree( ASSIGN, buildtree( NAME, NIL, NIL ), p );
		ininit = 0;
		ecomp(p);
		return;
	}

		/* for throwing away strings that have been turned into lists */
	if( p->in.op == NAME && p->tn.rval == NOLAB ){
		p->in.op = FREE;
		return;
	}

# ifndef BUG1
	if( idebug > 1 ) printf( "doinit(%o)\n", p );
# endif

	t = pstk->in_t;  /* type required */
	if( pstk->in_sz < 0 ) {  /* bit field */
		sz = -pstk->in_sz;
	}
	else {
		sz = tsize( t );
	}

	/*
	 * check if we have too many initializers. if so
	 * produce an error message rather that a compiler error message.
	 */
	if (inoff > pstk->in_off) {
		UERROR( ALWAYS, MESSAGE(108) );
		goto leave;
	}

	inforce( pstk->in_off );

	ininit = 1;
	/*
	 * if we have a single expression initializer then
	 * we want to make sure we can perform the object assignment since
	 * that expression should initialize the entire object.
	 * in case of an structure or a union that expression should
	 * have compatible type.
	 */

	if (ISSINGLE)
		p = buildtree( ASSIGN, buildtree( NAME, NIL,NIL), p );
	else
		p = buildtree( ASSIGN, block( NAME, NIL,NIL, t ), p );
	ininit = 0;
	p->in.left->in.op = FREE;
	p->in.left = p->in.right;
	p->in.right = NIL;
#ifndef BUG1
		/* tree before optimizing/folding */
		if (bdebug > 2) fwalk(p, eprint, 0);
#endif
	p->in.left = foldexpr( optim(p->in.left) );
#ifndef BUG1
		/* tree after optimizing/folding */
		if (bdebug > 2) fwalk(p, eprint, 0); 
#endif
	p->in.op = INIT;

	/*
	 * the following if-statement throws aways SCONVs to a float type
	 * in single precision mode. clocal does not throw these away.
	 * Now, the SCONV node is not thrown away unless we're n
	 * EXPRESSION mode (i.e., NO_FOLD()).
	 * (INIT (SCONV[ISFLOAT] x) nil) => (INIT x nil)
	 * or
	 * (INIT (U& x) nil) => (INIT x nil)
	 */
	l=p->in.left;
	if ( (l->in.op==UNARY AND)
#ifdef SINGLE_PRECISION
		|| (NO_FOLD() && l->in.op==SCONV && ISFLOAT(l->in.type))
#endif
	) {
		p->in.left->in.op = FREE;
		p->in.left = p->in.left->in.left;
	}

	/*
	 * special case: for bit fields sz may be less than SZINT
	 */
	if( sz < SZINT ) { 
		if( p->in.left->in.op != ICON ) 
			/* "illegal initialization"  */
			UERROR( ALWAYS, MESSAGE(61) );
		else incode( p->in.left, sz );
	}
	else if( p->in.left->in.op == FCON ) {
# ifndef NOFLOAT
		fincode( p->in.left->fpn.dval, sz );
#else
		cerror(TOOLSTR(M_MSG_232, "a floating point constant in a NOFLOAT compiler?"));
# endif
	}
	else {
		cinit( foldexpr( optim(p) ), sz );
	}
	gotscal();
leave:
	/*
	 * restore the folding mechanism into
	 * its standard mode.
	 */
	foldMask = EXPRESSION;

	tfree(p);
}

/* -------------------- gotscal -------------------- */

gotscal() {
	register TPTR t;
	register ix;
	register n, id;
	struct symtab *p;
	OFFSZ temp;

# ifndef BUG1
		if( idebug ) printf( "gotscal(%o)\n", pstk );
# endif

	for( ; pstk > instack; ) {
		--pstk;

# ifndef BUG1
		if( idebug ) printf( "gotscal(%o)\n", pstk );
# endif

		t = pstk->in_t;

		if (TOPTYPE(t) == STRTY) {
			ix = ++pstk->in_x;
			if( (id=dimtab[ix]) < 0 ) {
				if ((pstk)->in_fl)
					return;
				else if (partelided == 0 && ibseen > 0) {
					/* "partially elided initialization" */
					WARNING(WANSI && WKNR, MESSAGE(180));
					partelided++;
				}
				continue;
			}

			/* otherwise, put next element on the stack */

			p = &stab[id];
			instk( id, p->stype, p->offset+pstk->in_off );
			return;
		}
		else if( ISARY(t) ) {
			n = ++pstk->in_n;
			if(n >= t->ary_size && pstk > instack) {
				if ((pstk)->in_fl)
					return;
				else if (partelided == 0 && ibseen > 0) {
					/* "partially elided initialization" */
					WARNING(WANSI && WKNR, MESSAGE(180));
					partelided++;
				}
				continue;
			}

			/* put the new element onto the stack */

			temp = pstk->in_sz;
			instk (pstk->in_id, DECREF(pstk->in_t),
				pstk->in_off+n*temp );
			return;
		}
		else if( TOPTYPE(t) == UNIONTY ) {
			if (pstk->in_fl) 
				return;
			else if (partelided == 0 && ibseen > 0) {
				/* "partially elided initialization" */
				WARNING(WANSI && WKNR, MESSAGE(180));
				partelided++;
			}
			continue;
		}
	}
}

/* -------------------- ilbrace -------------------- */

ilbrace() { /* process an initializer's left brace */
register TPTR t;
struct instk *temp;
int structflag;

# ifndef BUG1
	if( idebug ) printf( "ilbrace(): paramno = %d on entry\n", paramno );
# endif

	temp = pstk;
	pstk = tempstack + 1;
	ibseen++;
	structflag = 0; /* no aggregate found in stack */

	for( ; pstk <= temp; pstk++ ) {
		t = pstk->in_t;
		if (TOPTYPE(t) == STRTY || ISARY(t) || TOPTYPE(t) == UNIONTY) {
			if( pstk->in_fl == 0 ) {
				/* we have one ... */
				pstk->in_fl = 1;
				structflag = 1; /* aggregate found in stack */
				break;
			}
			else {
				continue;
			}
		}
	}
	pstk = temp;
	if (structflag == 0) {
		pstk->in_fl = 1;
	}
# ifndef BUG1
	if( idebug ) dumpstack("ilbrace");
# endif
}

/* -------------------- irbrace -------------------- */

irbrace() {
	struct instk *temp = pstk;

# ifndef BUG1
	if( idebug ) printf( "irbrace(): paramno = %d on entry\n", paramno );
	if( idebug ) dumpstack("irbrace1");
# endif

	for( ; pstk > instack; --pstk ) {
		if( pstk->in_fl == 0 )
			continue;
		else {
			/* we have one now */
			if (ibseen > 0) {
				inforce(tsize(pstk->in_t) + pstk->in_off);
				pstk->in_fl = 0;
				--ibseen;
# ifndef BUG1
				if( idebug ) dumpstack("irbrace2");
# endif
				gotscal();
			}
			else {
				pstk->in_fl = 0;  /* cancel left brace */
				gotscal();  /* take it away... */
			}
			return;
		}
	}
	pstk = temp;
}

/* -------------------- upoff -------------------- */

upoff( size, alignment, poff ) register alignment;
register OFFSZ *poff;
{
	/* update the offset pointed to by poff; return the
	 * offset of a value of size `size', alignment `alignment',
	 * given that off is increasing */

	register OFFSZ off;

	off = *poff;
	SETOFF( off, alignment );
	if( (offsz-off) <  size ){
		if( instruct!=INSTRUCT )cerror(TOOLSTR(M_MSG_233, "too many local variables"));
		else cerror(TOOLSTR(M_MSG_234, "Structure too large"));
		}
	*poff = off+size;
	return( off );
	}

/* -------------------- oalloc -------------------- */

oalloc( p, poff ) register struct symtab *p; register *poff; {
	/* allocate p with offset *poff, and update *poff */
	register al, tsz;
	OFFSZ noff, off;

	al = talign( p->stype );
	noff = off = *poff;
	/* Note to KR: PARAMETERS of type FLOAT are converted into DOUBLE
	 * always. That may change, and don't forget LDOUBLEs as well.
	 */
	if( p->sclass == PARAM || p->sclass == PARAMREG )
		tsz = tsize(TOPTYPE(p->stype)==FLOAT?tyalloc(DOUBLE):p->stype);
	else
		tsz = tsize( p->stype );
#ifdef BACKAUTO
	if( p->sclass == AUTO || p->sclass == AUTOREG ){
		if( (offsz-off) < tsz ) cerror(TOOLSTR(M_MSG_233, "too many local variables"));
		noff = off + tsz;
		SETOFF( noff, al );
		off = -noff;
		}
	else
#endif
			/* align char/short PARAM and REGISTER to ALINT */
			/* but don't align structures and unions */
		if( (p->sclass == PARAM || p->sclass == PARAMREG || p->sclass == REGISTER)
			&& (tsz < SZINT) && (TOPTYPE(p->stype) != STRTY)
			&& (TOPTYPE(p->stype) != UNIONTY) ){
			off = upoff( SZINT, ALINT, &noff );
# ifndef RTOLBYTES
			off = noff - tsz;
#endif
			}
		else
		{
		/* automatic char arrays aligned on word boundaries..crm */
		/* (Don't look like that!  This is for Dhrystone wars.) */
		if ( ( BTYPE(p->stype)==CHAR || BTYPE(p->stype)==UCHAR ||
			BTYPE(p->stype)==SCHAR) &&
			ISARY(p->stype) && (p->sclass==AUTO || p->sclass==AUTOREG))
				off = upoff( tsz, ALINT, &noff);
		else
			off = upoff( tsz, al, &noff );
		}

	/* in case we are allocating stack space for register arguments */
	if( p->sclass != REGISTER ){
		if( p->offset == NOOFFSET ) p->offset = off;
		else if( off != p->offset ) return(1);
	}

	if (adebug && ISARY(p->stype) && (p->sclass==AUTO || p->sclass==AUTOREG ||
		p->sclass==PARAM || p->sclass==PARAMREG) ) {
		register int xoffset = off/SZCHAR;
#if	!defined (CXREF) && !defined (LINT) && !defined (CFLOW)
		printf("\t.copt\tsym,%d,%ld+L.%d%c\n", (tsz+SZCHAR-1)/SZCHAR,
			xoffset, ftnno, (p->sclass==PARAM || p->sclass==PARAMREG) ? 'A' : 'L' );
#endif
		if ((p->sclass==PARAM || p->sclass==PARAMREG) && (xoffset<minsvarg))
			minsvarg = xoffset;
	}
	*poff = noff;
	return(0);
	}

/* -------------------- markaddr ------------------ */

markaddr(p) register NODE *p; {
	/* If p has for *(STKREG+const) or *(ARGREG+const)    */
	/* mark node has had its address taken.               */
	/* minsvarg used to emit stores of reg args in prolog */
	/* This is yucchy but its the best we can do...       */

	register NODE *l,*r,*q;
	register int size;

	if (p->in.op == PNAME || p->in.op == LNAME)
	{
		/* atomic local or parameter - easy */
		size = tlen(p);
#if	!defined (CXREF) && !defined (LINT) && !defined (CFLOW)
		if (adebug)
			printf("\t.copt\tsym,%d,%ld+L.%d%c\n",
				size, p->tn.lval, ftnno,
				(p->in.op==PNAME) ? 'A' : 'L' );
#endif
		if (p->in.op == PNAME  &&  p->tn.lval < minsvarg)
			minsvarg = p->tn.lval;
	}
	else
	if ((p->in.op==UNARY MUL)
	     && ((q=p->in.left)->in.op==PLUS)
	     && ((l=q->in.left)->in.op==PCONV)
	     && ((l=l->in.left)->in.op==REG)
	     && ((l->tn.rval==STKREG) || (l->tn.rval==ARGREG))
	     && ((r=q->in.right)->in.op==ICON)
	) {
		if (TOPTYPE(p->in.type) == STRTY ||
		    TOPTYPE(p->in.type) == UNIONTY)
			size = tsize(p->in.type) / SZCHAR;
		else size = tlen(p);
#if	!defined (CXREF) && !defined (LINT) && !defined (CFLOW)
		if (adebug)
			printf("\t.copt\tsym,%d,%ld+L.%d%c\n",
				size, r->tn.lval, ftnno,
				(l->tn.rval==ARGREG) ? 'A' : 'L' );
#endif
		if ((l->tn.rval==ARGREG) && (r->tn.lval<minsvarg))
			minsvarg = r->tn.lval;
	}
}

/* -------------------- falloc -------------------- */

falloc( p, w, new, pty )  register struct symtab *p; NODE *pty; {
	/* allocate a field of width w */
	/* new is 0 if new entry, 1 if redefinition, -1 if alignment */

	register al, sz;
	register TPTR type;
	TWORD qual;

	type = (new<0)? pty->in.type : p->stype;
	/* Convert signed types to unsigned */
	switch (TOPTYPE(type)) {
	case INT:
	  if (!ISTSIGNED(type)) {
	    MODTYPE(type, UNSIGNED);
	  }
	  break;
	case LONG:
	  if (!ISTSIGNED(type)) {
	    MODTYPE(type, ULONG);
	  }
	  break;
	case LLONG:
	  if (!ISTSIGNED(type)) {
	    MODTYPE(type, ULLONG);
	  }
	  break;
	case CHAR:
	case SCHAR:
	  if (!ISTSIGNED(type)) {
	    MODTYPE(type, UCHAR);
	  }
	  break;
	case SHORT:
	  if (!ISTSIGNED(type)) {
	    MODTYPE(type, USHORT);
	  }
	  break;
	}

	/* this must be fixed to use the current type in alignments */
	switch (TOPTYPE(type)) {

	case INT:
	case UNSIGNED:
	case LONG:
	case ULONG:
	case LLONG:
	case ULLONG:
	case CHAR:
	case SCHAR:
	case UCHAR:
	case SHORT:
	case USHORT:
		al = talign(type);
		sz = tsize(type);
		break;

	case ENUMTY:
		al = talign(type);
		sz = tsize(type);
		if( new < 0 && devdebug[BITFIELDS] ){
			/* "illegal field type" */
			WERROR( ALWAYS, MESSAGE(57) );
			break;
			/* Allow enum if !ANSI typing or if 
			 * strange BITFIELDS are allowed */
		  } else if (!devdebug[TYPING] || devdebug[BITFIELDS]) 
			break;
		/* Fall through */
	default:
		/* "illegal bit field type, unsigned assumed" */
		WERROR( ALWAYS, MESSAGE(125) );
		qual = QUALIFIERS(type);
		type = tyalloc(UNSIGNED);
		if( qual ) type = qualtype(type, qual, 0);
		al = talign(type);
		sz = tsize(type);
		}

	if( w > sz ) {
		/* "field too big" */
		UERROR( ALWAYS, MESSAGE(39) );
		w = sz;
		}

	if( w == 0 ){
		if( new < 0 ){
			/* align only */
			SETOFF( strucoff, al );
			return( 0 );
			}
		/* "zero size field for %s" */
		UERROR( ALWAYS, MESSAGE(120), p->psname );
		w = 1;
		}

	if( strucoff%al + w > sz ) SETOFF( strucoff, al );
	if( new < 0 ) {
		if( (offsz-strucoff) < w )
			cerror(TOOLSTR(M_MSG_234, "structure too large" ));
		strucoff += w;  /* we know it will fit */
		return(0);
		}

	/* establish the field */

	if( new == 1 ) { /* previous definition */
		if( p->offset != strucoff || p->sclass != (FIELD|w) ) return(1);
		}
	p->offset = strucoff;
	if( (offsz-strucoff) < w ) cerror(TOOLSTR(M_MSG_234, "structure too large" ));
	strucoff += w;
	p->stype = type;
	fldty( p );
	return(0);
	}

/* -------------------- nidcl -------------------- */

nidcl (p)
	NODE *p;
{
	/*
	** Handle unitialized declarations.
	*/
	register class = curclass;

	/* Save blevel, it will be chaged if class == EXTERN */
	register slev = blevel;	   

	/* Determine class */
	if( ISFTN(p->in.type) ){
		class = uclass( class );
	} else if( blevel == 0 ){
		if( class == SNULL ){
			class = EXTENT;
		} else if( ( class = uclass( class ) ) == USTATIC ){
			if( ISARY(p->in.type) ){
				if( p->in.type->ary_size == 0 )
					/* "cannot declare incomplete static object" */
					WERROR( ALWAYS, MESSAGE(191) );
			} else if( !ISPTR(p->in.type) ){
				if( dimtab[p->in.type->typ_size] == 0 )
					/* "cannot declare incomplete static object" */
					WERROR( ALWAYS, MESSAGE(191) );
			}
		}
		
	      } else if (blevel > 1 && class == EXTERN) {
		/* In the case of an extern declaration within a block
		 * set the scope to file scope.
		 */
		blevel = 0;
	      }

	defid( p, class );
	blevel = slev; /* Restore current block level */

	if( class == STATIC )
		/* Must be block scope so define it now */
		commdec( &stab[p->tn.rval] );
}

/* -------------------- deftents -------------------- */

deftents ()
{
#ifndef	CXREF
  register struct symtab *p;
  register int i = 0;
  
  while(i = ScanStab(i)) {
	p = &stab[i];
	
	if( TOPTYPE(p->stype) == TNULL ) {
	  debug_print_stab(ddebug, i);
	  cerror(TOOLSTR(M_MSG_241, "check error in deftents: %s"), 
			 stab[i].psname );
	  continue;
	}
	
	
	switch( p->sclass ){
		case USTATIC:
	  		if( ISFTN(p->stype) || p->suse > 0 )
				break;
		case EXTENT:
	  		commdec( p );
	  		break;
	}
#ifdef	LINT
	/*
	 ** Don't emit undefined/defined static symbols.
	 ** All symbols with class EXTENT should now be EXTDEF.
	 ** cflow needs the class promotion results.
	 */
	if( p->sclass != USTATIC && p->sclass != STATIC )
	  OutSymbol(p, 0);
#endif
#ifdef	CFLOW
	OutSymbol(p, 0);
#endif
  }
#endif
}

#if	defined (LINT) || defined (CFLOW)
/*
** The following lint support functions emit data to the second-pass
** using an intermediate file.
** The BIO define selects output in either binary mode (if defined)
** or ascii mode (if not defined) for debugging.
*/

#define	BIO	 	/* binary i/o selected */
/* These turn on the io debugging mechanisms */
#define FRED_DBG	/* fii's debug stuff */ 
#define	IODEBUG /* Other I/O debug stuff */

/*
** Indicate beginning of a new physical file.
*/
OutFileBeg(iocode)
	char iocode;
{
	if (!pfname)
		return;	/* Internal Error */
#ifdef	BIO
	fwrite((char *) &iocode, sizeof(char), 1, tmplint);
	fwrite(pfname, strlen(pfname)+1, 1, tmplint);
#else
	fprintf(tmplint, "%d\n", iocode);
	fprintf(tmplint, "%s\n", pfname);
#endif
#ifdef FRED_DBG
	if (odebug) {
	printf("OBF wrote iocode %d for %s\n",iocode,pfname);
      }
#endif
}

/*
** Indicate end of a physical file.
*/
OutFileEnd(iocode)
	char iocode;
{
#ifdef	BIO
	fwrite((char *) &iocode, sizeof(char), 1, tmplint);
#else
	fprintf(tmplint, "%d\n", iocode);
#endif
#ifdef FRED_DBG
	if (odebug) {
	printf("OBE ended iocode %d for FILE\n",iocode);
      }
#endif
}

/*
** Write the type information of a symbol table entry.
** Partially interpreted by second pass as a header record.
*/
OutSymbol(p, ftnfix)
	register struct symtab *p;
	int ftnfix;
{
	register char class;
	short rline;
	char iocode = 0;
	short usage = 0;

	/*
	** Don't emit any functions since these are emitted directly
	** from the grammar (cgram.y), with the exception of undefined
	** function prototypes (declared/used only), or unused old-style
	** function declarations. Builtin functions are also rejected
	*/
	if (!ftnfix && ISFTN(p->stype)) {
		if (p->sclass == EXTERN && !(p->sflags & SBUILTIN) &&
		    ((p->stype->ftn_parm != PNIL) ||
			((p->stype->ftn_parm == PNIL) && p->suse > 0)))
			;
		else
			return;
	}

	/* Members of complex types are emitted later on. */
	switch (class = p->sclass) {
	case MOS:
	case MOU:
	case MOE:
	case TYPEDEF:
		return;
	default:
		if (class & FIELD)	/* bit field? */
			return;
	}
	iocode = LINTSYM;

	/* Determine usage. */
	if (lintused)
		usage |= LINTNOT;	/* check for NOTUSED */
	if (lintdefd)
		usage |= LINTNDF;	/* check for NOTDEFINED */
	usage |= p->sflags & SNSPACE;
	if ((rline = p->suse) < 0) {
		rline = -rline;
		usage |= LINTREF;	/* symbol referenced */
#if defined (LINT)
		/* Turn on setonly flag */
		if ((p->sflags & (SSET|SREF|SMOS)) == SSET)
		  usage |= LINTSTO;
#endif
	}
	if (class == EXTERN)
		usage |= LINTDCL;	/* symbol declared */
	else if (class == EXTDEF)
		usage |= LINTDEF;	/* symbol defined */
#ifdef	CFLOW
	/* Case when called by WarnWalk (not by deftents). */
	else if (class == USTATIC || class == STATIC)
		usage |= LINTDEF;	/* symbol defined */
#endif
	if (class == STNAME || class == UNAME || class == ENAME) {
#ifdef	CFLOW	/* cflow doesn't like tagnames */
		return;
#else
		usage |= LINTDEF;	/* symbol defined */
		usage |= LINTMBR;	/* symbol has members */
/* GAF: the visibility of type names is local to module, and should not
 *      be emitted says I for now.
 */
		return;
#endif
	}
	if (ftnfix)
		usage &= ~LINTDEF;
	/* Prototype-specific checks. */
	if (ISFTN(p->stype)) {
		if (TOPTYPE(p->stype->next) != TVOID)
			usage |= LINTRET;	/* check for return value */
		if (lintrsvd)
			usage |= LINTDEF;	/* check for LINTSTDLIB */
	}

#ifdef	IODEBUG
	if (odebug) {
	printf("(S)%s <", p->psname);
	tprint(p->stype);
	printf("> %s 0%o\n", scnames(p->sclass), usage);
      }
#endif
#ifdef	BIO
	fwrite((char *) &iocode, sizeof(char), 1, tmplint);
	fwrite((char *) p->psname, strlen(p->psname)+1, 1, tmplint);
	fwrite((char *) p->ifname, strlen(p->ifname)+1, 1, tmplint);
	fwrite((char *) &p->line, sizeof(short), 1, tmplint);
	fwrite((char *) &rline, sizeof(short), 1, tmplint);
	fwrite((char *) &usage, sizeof(short), 1, tmplint);
#else
	fprintf(tmplint, "%d\n", iocode);
	fprintf(tmplint, "%s\n", p->psname);
	fprintf(tmplint, "%s\n", p->ifname);
	fprintf(tmplint, "%d\n", p->line);
	fprintf(tmplint, "%d\n", rline);
	fprintf(tmplint, "%d\n", usage);
#endif
#ifdef FRED_DBG
	if (odebug) {
	printf("OutSym wrote iocode %d\n\t Sname: %s\n\t\t Fname: %s\n\t\t\t line: %d rline: %d usage %d\n",
	      iocode,p->psname,p->ifname,p->line,rline,usage );
      }
#endif

	OutType(p->stype);

#ifndef	CFLOW	/* cflow(1) does not care about member information */
	/* Members are emitted together now. */
	if (usage & LINTMBR) {
		short cnt;
		cnt = (short) CountMembers(p);
#ifdef	BIO
		fwrite((char *) &cnt, sizeof(short), 1, tmplint);
		
#else
		fprintf(tmplint, "%d %s\n", cnt, "OutSym_Mbr_Count");
#endif
		OutMembers(p);
#ifdef FRED_DBG
		if (odebug) {
	printf("OutSym wrote member count: %d\n",cnt);
      }
#endif
	}
#endif
}

/*
** Count the number of struct/union/enum members.
*/
CountMembers(q)
	register struct symtab *q;
{
	register int j;
	register short c;

	/* Check for undefined symbol. */
	if ((j = dimtab[q->stype->typ_size+1]) < 0)
		return (0);
	for (c = 0; dimtab[j] >= 0; ++c, ++j);
	return (c);
}

/*
** Write struct/union/enum members.
*/
OutMembers(q)
	register struct symtab *q;
{
	register struct symtab *p;
	register int j, m;
	TWORD bt;
	short usage = 0;
	char *NullNamestr = "%UnNamedLintMem%"; /* This illegal string will prevent us from trashing the file */

	/* Check for undefined symbol. */
	if ((j = dimtab[q->stype->typ_size+1]) < 0)
		return;

	/* Write each member. */
	for (; (m = dimtab[j]) >= 0; ++j) {
		p = &stab[m];
#ifdef	BIO
		fwrite(p->psname, strlen(p->psname)+1, 1, tmplint);
#else
		fprintf(tmplint, "%s\n", p->psname);
#endif
#ifdef FRED_DBG
		if (odebug) {
	printf("Outmem wrote Sname: %s\n",p->psname);
      }
#endif
	      

		OutType(p->stype);

		/* Get tag name from base TPTR. */
		if ((bt = BTYPE(p->stype)) == STRTY || bt == UNIONTY || bt == ENUMTY) {
			TPTR t = p->stype;
			while (!ISBTYPE(t))
				t = DECREF(t);
			if (((m = dimtab[t->typ_size+3]) >= 0)
			   && (stab[m].psname != NIL))
				usage |= LINTTAG;	/* symbol has a tag */
			else
			  usage = 0;
#ifdef	BIO
			fwrite((char *) &usage, sizeof(short), 1, tmplint);
#else
			fprintf(tmplint, "%d %s\n", usage, "OutMB_Usage_2633");
#endif
#ifdef FRED_DBG
			if (odebug) {
	printf("Outmem linttag usage: %d\n",usage);
      }
#endif

			if ((usage & LINTTAG) && stab[m].psname) { 
			  if (stab[m].psname != NIL) {
#ifdef	BIO
				fwrite(stab[m].psname, strlen(stab[m].psname)+1, 1, tmplint);
#else
				fprintf(tmplint, "%s\n", stab[m].psname);
#endif
#ifdef FRED_DBG
				if (odebug) {
	printf("Outmem linttag psname: %s\n",stab[m].psname);
      }
#endif
			      }
			else {
#ifdef	BIO
		fwrite(NullNamestr, strlen(NullNamestr)+1, 1, tmplint);
#else
		fprintf(tmplint, "%s\n",NullNamestr);
#endif
#ifdef FRED_DBG
		if (odebug) {
	printf("Outmem linttag psname: %s\n",NullNamestr);
      }
#endif
	      }
		      }


		}
	}
}

/*
** Write the type information.
*/
OutType(t)
	register TPTR t;
{
	register PPTR p;

	do {
#ifdef	BIO
#ifdef FRED_DBG
	  if (odebug) {
	        printtyinfo(t,1);
	      }
#endif
		fwrite((char *) t, sizeof(struct tyinfo), 1, tmplint);
#else
		fprintf(tmplint, "0%o 0%lo 0%o %s\n", t->tword, t->next, t->typ_size,"OutType_ILI_2657");
#endif
 		if (ISFTN(t)) {
			if ((p = t->ftn_parm) != PNIL) {
				do {
#ifdef	BIO
					fwrite((char *) p, sizeof(struct parminfo), 1, tmplint);
#else
					fprintf(tmplint, "0%lo 0%lo %s\n", p->type, p->next,"FTN_PARMS_LL");
#endif

#ifdef FRED_DBG
					if (odebug) {
	printf("Outtype Parameters: calling outtype for %lx\n",p->type);
      }
#endif
					OutType(p->type);
				} while ((p = p->next) != PNIL);
			}
 		}
		else if (!ISARY(t) && !ISPTR(t))
			return;
	} while (t = DECREF(t));
}

/*
** Strip the full name to get the basename.
*/
char *
strip(s)
	register char *s;
{
	static char buf[BUFSIZ+1];
	register char *p;

	for (p = buf; *s; ++s) {
		if (*s == '/')
			p = buf;
		else if (*s != '"')
			*p++ = *s;
		if (p > &buf[BUFSIZ])
			cerror(TOOLSTR(M_MSG_235, "filename too long"));
	}
	*p = '\0';
	return (buf);
}

/*
** Return 1 if subtree represents a function call.
*/
iscall(p)
	NODE *p;
{
	if (!p)
		return (0);
	switch (p->in.op) {
	case CALL:
	case STCALL:
	case UNARY CALL:
	case UNARY STCALL:
		if (p->in.left->in.op == UNARY AND)
			return (1);
		break;
	default:
		if (optype(p->in.op) == BITYPE)
			return (iscall(p->in.right));
		else if (optype(p->in.op) == UTYPE)
			return (iscall(p->in.left));
	}
	return (0);
}

/*
** Write type information for an old-style or prototyped
** function reference.
*/
OutFtnRef(p, style)
	register NODE *p;
	int style;
{
	register TPTR t;
	register struct symtab *r;
	NODE *q;
	struct tyinfo ty;
	short rline;
	char iocode = 0;
	short usage = 0;

	iocode = LINTSYM;
	if (!iscall(p))
		return;
	q = p;
	while ((p->in.op != NAME && p->in.op != LNAME && p->in.op != PNAME) && (p = p->in.left));
	if (p == PNIL || (p->in.op != NAME && p->in.op != LNAME && p->in.op != PNAME))
		cerror(TOOLSTR(M_MSG_236, "cannot complete function treewalk"));
	r = &stab[p->tn.rval];
	if (r->sflags & SBUILTIN) {
#ifdef	IODEBUG
	  if (odebug) 
	    printf("OutFres:Suppressing output for function ref:%s\n", 
		   r->psname);
#endif
	  return;
	}
	t = p->in.type;
	p = q;
#ifndef	CFLOW
	if (r->sclass == USTATIC || r->sclass == STATIC)
		return;
#endif
	r->line = lineno;
	rline = (r->suse < 0) ? -r->suse : r->suse;

	/* Determine usage. */
	usage |= r->sflags & SNSPACE;
	usage |= LINTREF;
	if (r->sclass == EXTERN)
		usage |= LINTDCL;

#ifdef	IODEBUG
	if (odebug) {
	printf("(R)%s <", r->psname);
	tprint(r->stype);
	printf("> 0%o %s\n", usage, (style) ? "fake" : "");
      }
#endif
#ifdef	BIO
	fwrite((char *) &iocode, sizeof(char), 1, tmplint);
	fwrite((char *) r->psname, strlen(r->psname)+1, 1, tmplint);
	fwrite((char *) r->ifname, strlen(r->ifname)+1, 1, tmplint);
	fwrite((char *) &r->line, sizeof(short), 1, tmplint);
	fwrite((char *) &rline, sizeof(short), 1, tmplint);
	fwrite((char *) &usage, sizeof(short), 1, tmplint);
#else
	fprintf(tmplint, "%d\n", iocode);
	fprintf(tmplint, "%s\n", r->psname);
	fprintf(tmplint, "%s\n", r->ifname);
	fprintf(tmplint, "%d\n", r->line);
	fprintf(tmplint, "%d\n", rline);
	fprintf(tmplint, "%d\n", usage);
#endif
#ifdef FRED_DBG
	if (odebug) {
      printf("OutFref wrote iocode %d\n\t Sname: %s\n\t\t Fname: %s\n\t\t\t line: %d rline: %d usage %d\n",
             iocode,r->psname,r->ifname,r->line,rline,usage );
    }
#endif

	/* Output ANSI prototype. */
	if (style) {
		OutType(r->stype);
		return;
	}


	/* Old-style function reference must simulate arguments. */
	ty.tword = t->tword;
	ty.next = t->next;
	ty.ftn_parm = NULL; /* GAF _ Don't simulate arguments */
/*	ty.ftn_parm = (PPTR) ((optype(p->in.op) == BITYPE) ? 1 : 0); */
#ifdef	BIO
	fwrite((char *) &ty, sizeof(struct tyinfo), 1, tmplint);
#else
	fprintf(tmplint, "0%o 0%lo 0%lo %s\n", ty.tword, ty.next, ty.ftn_parm, "OutFtnRef_Ill_2796" );
#endif
#ifdef FRED_DBG
	if (odebug) {
      printf("OutFref simulated args for type:  ");
      printtyinfo(&ty,1);
    }
#endif

	/* Write function parameters. */
	if (style && optype(p->in.op) == BITYPE) {
		if (p->in.right->in.op == CM)
			OutArguments(p->in.right);
		else {
			/* Only one argument. */
			struct parminfo p;
			p.type = q->in.type; p.next = (PPTR) 0;
#ifdef	BIO
			fwrite((char *) &p, sizeof(struct parminfo), 1, tmplint);
#else
			fprintf(tmplint, "0%lo 0%lo, %s\n", p.type, p.next, "Parminfo_LL_2809");
#endif
#ifdef FRED_DBG
			if (odebug) {
      printf("OutFref one arg parminfo type %lx next %lx\n",p.type, p.next);
    }
#endif

			OutArgType(q->in.type);
		}
	}

	/* Type of function itself. */
	t = DECREF(t);
	OutArgType(t);
}

/*
** Output function usage information.  This ORed to previous
** symbol usage by lint2 - it concerns only function usage.
*/
OutFtnUsage(p, usage)
	struct symtab *p;
	short usage;
{
	char iocode = 0;

	iocode = LINTADD;
	if (p->sflags & SBUILTIN) {
#ifdef	IODEBUG
	  if (odebug) 
	    printf("OutFUsage:Suppressing output for function ref:%s\n", 
		   p->psname);
#endif
	  return;
	}
#ifndef	CFLOW
	if (p->sclass == USTATIC || p->sclass == STATIC)
		return;
#endif
#ifdef	IODEBUG
	if (odebug) {
	printf("(U)usage %s 0%o\n", p->psname, usage);
      }
#endif
#ifdef	BIO
	fwrite((char *) &iocode, sizeof(char), 1, tmplint);
	fwrite((char *) p->psname, strlen(p->psname)+1, 1, tmplint);
	fwrite((char *) &usage, sizeof(short), 1, tmplint);
#else
	fprintf(tmplint, "%d\n", iocode);
	fprintf(tmplint, "%s\n", p->psname);
	fprintf(tmplint, "%d\n", usage);
#endif
#ifdef FRED_DBG
	if (odebug) {
      printf("OutFUsage wrote iocode %d\n\t Sname: %s\n\t\t Usage: %d\n",
             iocode,p->psname,usage );
    }
#endif

}

/*
** Write function parameters.
** Make it look like a function prototype by introducing
** PPTR data structures with the last PPTR->next being NIL.
*/
OutArguments(q)
	register NODE *q;
{
	struct parminfo p;
	static int down = 0;		/* count #PPTRs */

	/* Find first function parameter, located at bottom-left. */
	if (q->in.op == CM && q->in.left->in.op == CM) {
		++down;
		OutArguments(q->in.left);
		--down;
	}

	/* Parameter appearing on the left side. */
	else if (q->in.op == CM && q->in.left->in.op != CM) {
		
		p.type = q->in.left->in.type; p.next = (PPTR) (down + 1);
#ifdef	BIO
		fwrite((char *) &p, sizeof(struct parminfo), 1, tmplint);
#else
		fprintf(tmplint, "0%lo 0%lo\n", p.type, p.next);
#endif
#ifdef FRED_DBG
		if (odebug) {
      printf("OutArg wrote leftside parms type: %lx next %lx\n",p.type, p.next );
    }
#endif

		OutArgType(q->in.left->in.type);
	}

	/* Parameter appearing on the right side. */
	p.type = q->in.right->in.type; p.next = (PPTR) down;
#ifdef	BIO
	fwrite((char *) &p, sizeof(struct parminfo), 1, tmplint);
#else
	fprintf(tmplint, "0%lo 0%lo\n", p.type, p.next);
#endif
#ifdef FRED_DBG
	if (odebug) {
      printf("OutArg wrote right side parms type: %lx next %lx\n",p.type, p.next );
    }
#endif
	OutArgType(q->in.right->in.type);
}

/*
** Write type information about a parameter.
** Don't elaborate if parameter is a prototype.
*/
OutArgType(t)
	register TPTR t;
{
	struct tyinfo ty;

	if (t == TNIL)
		return;

	do {
		ty.tword = t->tword;
		ty.next = t->next;
		ty.ftn_parm = 0l; /*( Fii clear high int!) */
		ty.typ_size = t->typ_size;

		/* Handle case where a function as a parameter may have a prototype. */
		if (ISFTN(t))
			ty.ftn_parm = (PPTR) 0;
#ifdef	BIO
		fwrite((char *) &ty, sizeof(struct tyinfo), 1, tmplint);
#else
		fprintf(tmplint, "0%o 0%lo 0%o\n", ty.tword, ty.next, ty.typ_size);
#endif
#ifdef FRED_DBG
		if (odebug) {
      printf("OutArgtype types for parameters :\n");
      printtyinfo(&ty,1);
    }
#endif

		if (!ISARY(t) && !ISPTR(t) && !ISFTN(t))
			return;
	} while (t = DECREF(t));
}

/*
** Write type information for an old-style function definition.
** Make it look like a function prototype.
*/
OutFtnDef()
{
	register int i,j;
	register TPTR t;
	struct tyinfo ty;
	struct parminfo p;
	struct symtab *r;
	short rline;
	char iocode = 0;
	short usage = 0;

	iocode = LINTSYM;
	r = &stab[curftn];
	if (!ISFTN(r->stype))
		return;
#ifndef	CFLOW
	if (r->sclass == USTATIC || r->sclass == STATIC)
		return;
#endif
	r->line = lineno;

	/* Determine usage. */
	usage |= r->sflags & SNSPACE;
	usage |= LINTDEF;
	if ((rline = r->suse) < 0) {
		rline = -rline;
		usage |= LINTREF;
	}
	if (lintused)
		usage |= LINTNOT;	/* check for NOTUSED */
	if (lintdefd)
		usage |= LINTNDF;	/* check for NOTDEFINED */
	/* Check for varargs. */
	if (lintvarg != -1)
		usage |= LINTVRG;

#ifdef	IODEBUG
	if (odebug) {
	printf("(D)%s <", r->psname);
	tprint(r->stype);
	printf("> 0%o %d/%d ret=%d %s\n", usage, r->line, rline, retstat,
		(funcstyle != NEW_STYLE) ? "fake" : "");
      }
#endif
#ifdef	BIO
	fwrite((char *) &iocode, sizeof(char), 1, tmplint);
	fwrite((char *) r->psname, strlen(r->psname)+1, 1, tmplint);
	fwrite((char *) r->ifname, strlen(r->ifname)+1, 1, tmplint);
	fwrite((char *) &r->line, sizeof(short), 1, tmplint);
	fwrite((char *) &rline, sizeof(short), 1, tmplint);
	fwrite((char *) &usage, sizeof(short), 1, tmplint);
#else
	fprintf(tmplint, "%d\n", iocode);
	fprintf(tmplint, "%s\n", r->psname);
	fprintf(tmplint, "%s\n", r->ifname);
	fprintf(tmplint, "%d\n", r->line);
	fprintf(tmplint, "%d\n", rline);
	fprintf(tmplint, "%d\n", usage);
#endif
#ifdef FRED_DBG
	if (odebug) {
      printf("OutFDEF wrote iocode %d\n\t Sname: %s\n\t\t Fname: %s\n\t\t\t line: %d rline: %d usage %d\n",
             iocode,r->psname,r->ifname,r->line,rline,usage );
    }
#endif

	/* Output ANSI prototype. */
	if (funcstyle == NEW_STYLE) {
		OutType(r->stype);
		return;
	}

	/* Old-style function definition must simulate arguments. */
	t = r->stype;
	ty.tword = t->tword;
	ty.next = t->next;
	ty.ftn_parm = (PPTR) ((paramno) ? 1 : 0);	/* simulate a prototype */
#ifdef	BIO
	fwrite((char *) &ty, sizeof(struct tyinfo), 1, tmplint);
#else
	fprintf(tmplint, "0%o 0%lo 0%lo\n", ty.tword, ty.next, ty.ftn_parm);
#endif
#ifdef FRED_DBG
	if (odebug) {
      printf("OutFDEF Simulate types: ");
      printtyinfo(&ty,1);
    }
#endif

	/*
	** Write type information about each parameter. 
	** Make it look like a function prototype by introducing
	** PPTR data structures with the last PPTR->next being NIL.
	*/
	for (i = 0; i < paramno; i++) {
		if ((j = paramstk[i]) < 0)
			continue;
		p.type = stab[j].stype;

		/* Add ellipsis if varargs specified.  Although, at least one
		   arg is required for syntax, we will get away with treating
		   varargs0 as an argument list consisting of ellipsis only. */
		if (i == lintvarg) {
			p.type = tyalloc(TELLIPSIS);
			i = paramno - 1;
		}
		p.next = (PPTR) (paramno - i - 1); 	/* count #PPTRs */
#ifdef	BIO
		fwrite((char *) &p, sizeof(struct parminfo), 1, tmplint);
#else
		fprintf(tmplint, "0%lo 0%lo\n", p.type, p.next);
#endif
#ifdef FRED_DBG
		if (odebug) {
      printf("OutFDEF  Parameters type: %lx next: %lx\n",p.type, p.next);
    }
#endif

		OutArgType(p.type);	/* parameter type */
	}

	/* Type of the function itself. */
	t = DECREF(t);
	OutArgType(t);
}

#endif

/* -------------------- types -------------------- */

#define VCHAR       01
#define VSHORT      02
#define VINT        04
#define VLONG      010
#define VFLOAT     020
#define VDOUBLE    040
#define VSIGNED   0100
#define VUNSIGNED 0200
#define VVOID  	  0400
#define VLLONG 	 01000
#define VALSIZE (VLLONG|VVOID|VUNSIGNED|VSIGNED|VDOUBLE|VFLOAT|VLONG|VINT|VSHORT|VCHAR)
TWORD validType[VALSIZE+1];

struct tytest {
	TWORD type;
	int mask;
};

static struct tytest typeANSI[] = {
	CHAR, VCHAR,
	CHAR, VSIGNED|VCHAR,
	UCHAR, VUNSIGNED|VCHAR,
	INT, VINT,
	INT, VSIGNED,
	INT, VSIGNED|VINT,
	UNSIGNED, VUNSIGNED,
	UNSIGNED, VUNSIGNED|VINT,
	SHORT, VSHORT,
	SHORT, VSHORT|VINT,
	SHORT, VSIGNED|VSHORT,
	SHORT, VSIGNED|VSHORT|VINT,
	USHORT, VUNSIGNED|VSHORT,
	USHORT, VUNSIGNED|VSHORT|VINT,
	LONG, VLONG,
	LONG, VLONG|VINT,
	LONG, VSIGNED|VLONG,
	LONG, VSIGNED|VLONG|VINT,
	ULONG, VUNSIGNED|VLONG,
	ULONG, VUNSIGNED|VLONG|VINT,
	FLOAT, VFLOAT,
	DOUBLE, VDOUBLE,
	LDOUBLE, VLONG|VDOUBLE,
	TVOID, VVOID,
	0, 0
};

static struct tytest typeEXTD[] = {
	CHAR, VCHAR,
	CHAR, VSIGNED|VCHAR,
	UCHAR, VUNSIGNED|VCHAR,
	INT, VINT,
	INT, VSIGNED,
	INT, VSIGNED|VINT,
	UNSIGNED, VUNSIGNED,
	UNSIGNED, VUNSIGNED|VINT,
	SHORT, VSHORT,
	SHORT, VSHORT|VINT,
	SHORT, VSIGNED|VSHORT,
	SHORT, VSIGNED|VSHORT|VINT,
	USHORT, VUNSIGNED|VSHORT,
	USHORT, VUNSIGNED|VSHORT|VINT,
	LONG, VLONG,
	LONG, VLONG|VINT,
	LONG, VSIGNED|VLONG,
	LONG, VSIGNED|VLONG|VINT,
	LLONG, VLONG|VLLONG,
	LLONG, VLLONG|VLONG|VINT,
	LLONG, VLLONG|VSIGNED|VLONG,
	LLONG, VLLONG|VSIGNED|VLONG|VINT,
	ULONG, VUNSIGNED|VLONG,
	ULONG, VUNSIGNED|VLONG|VINT,
	ULLONG, VUNSIGNED|VLONG|VLLONG,
	ULLONG, VUNSIGNED|VLONG|VINT|VLLONG,
	FLOAT, VFLOAT,
	DOUBLE, VDOUBLE,
	DOUBLE, VLONG|VFLOAT,
	LDOUBLE, VLONG|VDOUBLE,
	TVOID, VVOID,
	0, 0
};

static struct tytest typeMask[] = {
	CHAR, VCHAR,
	SCHAR, VSIGNED|VCHAR,
	UCHAR, VUNSIGNED|VCHAR,	
	SHORT, VSHORT,
	USHORT, VUNSIGNED|VSHORT,
	INT, VINT,
	UNSIGNED, VUNSIGNED,
	LONG, VLONG,
	ULONG, VUNSIGNED|VLONG,
	LLONG, VLLONG,
	ULLONG, VUNSIGNED|VLLONG,
	FLOAT, VFLOAT,
	DOUBLE, VDOUBLE,
	LDOUBLE, VLONG|VDOUBLE,
	SIGNED, VSIGNED,
	TVOID, VVOID,
	0, 0
};

int lookupMask[NBTYPES];

#define VENUM 01
#define VSTRUCT 02
#define VTYDEF 04

#define PARSEDEPTH 15
struct {
    int nlongs;
	int curTypeMask;	/* bit pattern of type */
	TWORD curQualifier;	/* type qualifier */
	int otherType;		/* other type flag */
} typeStack[PARSEDEPTH];

/*
** Initialize the type checking arrays.  Either ANSI or extended C mode.
*/
InitParse()
{
	register struct tytest *q;

	/* Initialize type validity and mask arrays. */
	q = (devdebug[TYPING]) ? typeANSI : typeEXTD;
	for ( ; q->type; ++q)
#ifdef COMPAT
		if( devdebug[KLUDGE] && !devdebug[COMPATIBLE] ){
			if( q->type == LONG )
				validType[q->mask] = INT;
			else if( q->type == ULONG )
				validType[q->mask] = UNSIGNED;
			else
				validType[q->mask] = q->type;
		} else {
			validType[q->mask] = q->type;
		}
#else
		validType[q->mask] = q->type;
#endif
	for (q = typeMask; q->type; ++q)
		lookupMask[q->type] = q->mask;
}

/*
** Running accumulation of a typedef.
*/
TPTR
CheckTypedef(type)
TPTR type;
{
	/* check for parse depth */
	if (curLevel > PARSEDEPTH)
		cerror(TOOLSTR(M_MSG_237, "Type Stack overflow"));

	/* Check for type clash. */
	if (typeStack[curLevel].curTypeMask)
		/* "basic type cannot mix with struct/union/enum/typedef" */
		UERROR( ALWAYS, MESSAGE(134) );
	else if (typeStack[curLevel].otherType)
		/* "illegal type specifier combination" */
		UERROR( ALWAYS, MESSAGE(70) );
	typeStack[curLevel].otherType |= VTYDEF;
	return(type);
}

/*
** Running accumulation of an enum.
*/
CheckEnum()
{
	/* check for parse depth */
	if (curLevel > PARSEDEPTH)
		cerror(TOOLSTR(M_MSG_237, "Type Stack overflow"));

	/* Check for type clash. */
	if (typeStack[curLevel].curTypeMask)
		/* "basic type cannot mix with struct/union/enum/typedef" */
		UERROR( ALWAYS, MESSAGE(134) );
	else if (typeStack[curLevel].otherType)
		/* "illegal type specifier combination" */
		UERROR( ALWAYS, MESSAGE(70) );
	typeStack[curLevel].otherType |= VENUM;
}

/*
** Running accumulation of a struct.
*/
CheckStruct()
{
	/* check for parse depth */
	if (curLevel > PARSEDEPTH)
		cerror(TOOLSTR(M_MSG_237, "Type Stack overflow"));

	/* Check for type clash. */
	if (typeStack[curLevel].curTypeMask)
		/* "basic type cannot mix with struct/union/enum/typedef" */
		UERROR( ALWAYS, MESSAGE(134) );
	else if (typeStack[curLevel].otherType)
		/* "illegal type specifier combination" */
		UERROR( ALWAYS, MESSAGE(70) );
	typeStack[curLevel].otherType |= VSTRUCT;
}

/*
** Running accumulation of type qualifier.
*/
CheckQualifier(qualifier)
TWORD qualifier;
{
	/* check for parse depth */
	if (curLevel > PARSEDEPTH)
		cerror(TOOLSTR(M_MSG_237, "Type Stack overflow"));

	/* Check for duplicate type qualifier. */
	if (typeStack[curLevel].curQualifier & qualifier)
		/* "illegal type qualifier combination" */
		WERROR( ALWAYS, MESSAGE(131) );
	typeStack[curLevel].curQualifier |= qualifier;
}

/*
** Running accumulation of type specifier.
*/
TPTR
CheckType(type)
TPTR type;
{
	int omask, tmask;

	/* check for parse depth */
	if (curLevel > PARSEDEPTH)
		cerror(TOOLSTR(M_MSG_237, "Type Stack overflow"));

	/* Check for type clash. */
	if (typeStack[curLevel].otherType)
		/* "basic type cannot mix with struct/union/enum/typedef" */
		UERROR( ALWAYS, MESSAGE(134) );

	/* If the BTYPE is LONG, and nlongs is non-zero, change to LLONG */
	if (!devdebug[TYPING] && ISLONG(type) && typeStack[curLevel].nlongs++)
	  type = tyalloc(LLONG);

	/* Check for type specifier duplication. */
	omask = typeStack[curLevel].curTypeMask;
	tmask = lookupMask[BTYPE(type)];
	tmask |= omask;
	if (tmask == omask)
		/* "illegal type specifier combination" */
		UERROR( ALWAYS, MESSAGE(70) );

	/* Check for valid type combination, assume INT. */
	if (!validType[tmask]) {
		/* "illegal type specifier combination" */
		UERROR( ALWAYS, MESSAGE(70) );
		tmask = VINT;
	}
	typeStack[curLevel].curTypeMask = tmask;
	return (tyalloc(validType[tmask]));
}

/*
** Return final type with qualifiers and other flags.
*/
TPTR
ResultType(type)
TPTR type;
{
	TWORD qual;
	unsigned size;

	/* check for parse depth */
	if (curLevel > PARSEDEPTH)
		cerror(TOOLSTR(M_MSG_237, "Type Stack overflow"));

	qual = typeStack[curLevel].curQualifier;

	if (qual) {
		/* Check if the type is already so qualified (for typedefs) */
		if (ISQUALIFIED(type, qual)) {
			/* "illegal type qualifier combination" */
			WERROR( ALWAYS, MESSAGE(131) );
			qual &= ~QUALIFIERS(type);
			if (qual) {
				type = qualtype(type, qual, 1);
			}
		} else if (ISFTN(type)) {
			/* "function returns qualified type" */
			WERROR( ALWAYS, MESSAGE(132) );
		} else if (ISARY(type)) {
			size = type->ary_size;
			type = DECREF(type);
			if (ISQUALIFIED(type, qual)) {
				/* "illegal type qualifier combination" */
				WERROR( ALWAYS, MESSAGE(131) );
				qual &= ~QUALIFIERS(type);
				if (qual) {
					type = qualtype(type, qual, 1);
				}
			} else {
				type = qualtype(type, qual, 1);
			}
			type = INCREF(type, ARY);
			type->typ_size = size;
		} else {
			type = qualtype(type, qual, 1);
		}
	}

	if (typeStack[curLevel].curTypeMask & VSIGNED) {
		type = signedtype(type);
	}

	memset(&typeStack[curLevel], 0, sizeof typeStack[0]);
/******* Remove this so I can add another field without worrying ***********
 *	typeStack[curLevel].curTypeMask = 0;
 *	typeStack[curLevel].curQualifier = 0;
 *	typeStack[curLevel].otherType = 0;
 **************************************************************************/
	return (type);
}

SeenType()
{
	return (typeStack[curLevel].curTypeMask ||
		typeStack[curLevel].otherType);
}

DiagnoseType()
{
	register int i;

	printf("curLevel = %d\n", curLevel);
	printf("curTypeMask ");
	for (i = 0; i < PARSEDEPTH; i++)
		printf("%o ", typeStack[i].curTypeMask);
	printf("\n");
	printf("curQualifier ");
	for (i = 0; i < PARSEDEPTH; i++)
		printf("%o ", typeStack[i].curQualifier);
	printf("\n");
	printf("otherType ");
	for (i = 0; i < PARSEDEPTH; i++)
		printf("%o ", typeStack[i].otherType);
	printf("\n");
}

WriteType(s)
char *s;
{
	/* check for parse depth */
	if (curLevel > PARSEDEPTH)
		cerror(TOOLSTR(M_MSG_237, "Type Stack overflow"));

	fprintf(stderr, TOOLSTR(M_MSG_285, "%s type:%o qual:%o other:%o\n"),
	s, typeStack[curLevel].curTypeMask, typeStack[curLevel].curQualifier,
	typeStack[curLevel].otherType);
}

/* -------------------- tymerge -------------------- */

NODE *
tymerge( typ, idp ) NODE *typ, *idp; {
	/* merge type typ with identifier idp  */

	register TPTR t, t2;
	TWORD ty;
	register i;
	extern int eprint();

	if( typ->in.op != TYPE ) cerror(TOOLSTR(M_MSG_238, "tymerge: arg 1" ));
	if(idp == NIL ) return( NIL );

# ifndef BUG1
	if( ddebug > 2 ) fwalk( idp, eprint, 0 );
# endif

	idp->in.type = typ->in.type;
	RecurseCnt = 0;
	tyreduce( idp );

	return( idp );
	}

/* -------------------- tyreduce -------------------- */

tyreduce( p ) register NODE *p; {

	/* build a type, and stash away dimensions, from a parse */
	/* tree of the declaration the type is build top down, */
	/* the dimensions bottom up */
	register TPTR t;
	register o;
	TWORD qual;

	o = p->in.op;
	p->in.op = FREE;

	if( o == NAME ) return;

	t = p->in.type;

	if( o == UNARY CALL ){
		if( TOPTYPE(t) == ARY || TOPTYPE(t) == FTN ){
			/* "function returns illegal type" */
			UERROR( ALWAYS, MESSAGE(47) );
			t = INCREF(t, PTR);
			}
		else if( QUALIFIERS(t) ){
			/* "function returns qualified type" */
			WERROR( ALWAYS, MESSAGE(132) );
			t = unqualtype(t);
			}
		t = INCREF(t, FTN);
 		/* reattach the parameter list pointer to
		 * the function node in the type tree and remove
		 * it from the return type of the function node.
		 */
		if (p->in.right != NIL) {
			t->ftn_parm = p->in.right->in.type->ftn_parm;
			p->in.right->in.op = FREE;
		}
	}

	else if( o == LB ){
		if( TOPTYPE(t) == FTN ){
			/* "array of functions is illegal" */
			UERROR( ALWAYS, MESSAGE(14) );
			t = INCREF(t, PTR);
			}
		if (RecurseCnt++ > 13)
		  UERROR( ALWAYS, TOOLSTR(M_MSG_321,
			"too many dimensions, maximum is 13") );
		t = INCREF(t, ARY);
		t->ary_size = p->in.right->tn.lval;
		p->in.right->in.op = FREE;
		if( ( t->ary_size == 0 ) & ( p->in.left->tn.op == LB ) )
			/* "null dimension" */
			WERROR( ALWAYS, MESSAGE(85) );
		}

	else /* o == UNARY MUL */ {
		t = INCREF(t, PTR);
		if( ( qual = QUALIFIERS(p->in.right->in.type) ) != 0 )
			t = qualtype(t, qual, 0);
		p->in.right->in.op = FREE;
		}

	p->in.left->in.type = t;
	tyreduce( p->in.left );

	p->tn.rval = p->in.left->tn.rval;
	p->in.type = p->in.left->in.type;

	}

/* -------------------- fixtype -------------------- */

fixtype( p, class ) register NODE *p; {
	register TPTR type;
	/* fix up the types, and check for legality */

	type = p->in.type;
	if (TOPTYPE(type) == UNDEF) return;

	/* detect function arguments, watching out for structure declarations */
	/* for example, beware of f(x) struct { int a[10]; } *x; { ... } */
	/* the danger is that "a" will be converted to a pointer */

	if( class == SNULL && ( blevel == 1 || paramlevel > 0 ) &&
			!( instruct & (INSTRUCT|INUNION) ) ){
		class = PARAM;
	}
	if( class == PARAM || class == PARAMREG || class == PARAMFAKE ||
			( class==REGISTER && ( blevel==1 || paramlevel>0 ) ) ){
		if( ISARY(type) ){
			type = INCREF(DECREF(type), PTR);
		} else if( ISFTN(type) ){
			type = INCREF(type, PTR);
		}
	}
	if( instruct && ISFTN(type) ){
		/* "function illegal in structure or union"  */
		UERROR( ALWAYS, MESSAGE(46) );
		type = INCREF(type, PTR);
	}
	p->in.type = type;
}

/* -------------------- uclass -------------------- */

uclass( class ) register class; {
	/* give undefined version of class */
	if( class == SNULL ) return( EXTERN );
	else if( class == STATIC ) return( USTATIC );
	else if( class == FORTRAN ) return( UFORTRAN );
	else return( class );
	}

int NoRegisters = 0;	/* set if register declarations should be ignored */

/* -------------------- fixclass -------------------- */

fixclass( class, type )
	TPTR type;
{
#ifndef MSG
#define ILLEGALCLASS illegalClass
	char *illegalClass = "illegal class: %s";
#else
#define ILLEGALCLASS  NLcatgets(catd, MS_CTOOLS, M_MSG_302, "illegal class: %s")
#endif

	/*
	** If class is register and we're optimizing then
	** ignore the register classification.
	*/
	if( class == REGISTER && adebug )
		class = SNULL;

	/*
	** Fix null class.
	*/
	if( class == SNULL ){
		if( instruct & INSTRUCT )
			class = MOS;
		else if( instruct & INUNION )
			class = MOU;
		else if( blevel == 0 )
			class = EXTDEF;
		else if( blevel == 1 || paramlevel > 0 )
			class = PARAM;
		else if( ISFTN(type) )
			class = EXTERN;
		else
			class = AUTO;
	}

	/*
	** Check function classes.
	*/
	if( ISFTN(type) ){
		switch( class ){
		default:
			/* "function has illegal storage class" */
			WERROR( ALWAYS, MESSAGE(45) );
			class = EXTERN;
		case EXTDEF:
		case EXTERN:
		case FORTRAN:
		case UFORTRAN:
		case STATIC:
		case TYPEDEF:
			break;
		case USTATIC:
			if( blevel > 0 )
				/* "function has illegal storage class" */
				WERROR( ALWAYS, MESSAGE(45) );
			break;
		}
	}

	/*
	** Check fields.
	*/
	if( class & FIELD ){
		if( !( instruct & ( INSTRUCT | INUNION ) ) )
			/* "illegal use of field" */
			UERROR( ALWAYS, MESSAGE(72) );
		return( class );
	}

	/*
	** Check general classes.
	*/
	switch( class ){

	case AUTO:
		if( blevel < 2 || paramlevel > 0 ){
			/* "illegal class" */
			WERROR( ALWAYS, MESSAGE(52) );
			if( blevel == 0 )
				class = EXTDEF;
			else
				class = PARAM;
		}
		break;

	case EXTERN:
	case STATIC:
		if( blevel == 1 || paramlevel > 0 ){
			/* "illegal class" */
			WERROR( ALWAYS, MESSAGE(52) );
			class = PARAM;
		}
		break;

	case TYPEDEF:
		if( blevel == 1 || paramlevel > 0 )
			/* "illegal typedef declaration" */
			WERROR( ALWAYS, MESSAGE(154) );
		break;

	case REGISTER:
		if( blevel == 0 ){
			/* "illegal register declaration" */
			WERROR( ALWAYS, MESSAGE(68) );
			class = EXTDEF;
			break;
		}
		if( !NoRegisters ){
			if( cisreg(type) && regvar >= MINRVAR+aflag )
				break;
			if( ( TOPTYPE(type) == DOUBLE ||
					TOPTYPE(type) == LDOUBLE ) &&
					fpregvar >= MINFPVAR )
				break;
		}
		class = ( blevel == 1 || paramlevel > 0 ) ? PARAMREG : AUTOREG;
		break;

	case FORTRAN:
	case UFORTRAN:
# ifdef NOFORTRAN
		/* a condition which can regulate the FORTRAN usage */
		NOFORTRAN;
# endif
		if( !ISFTN(type) ){
			/* "fortran declaration must apply to function" */
			UERROR( ALWAYS, MESSAGE(40) );
			class = EXTERN;
		} else if( ISPTR(DECREF(type)) ){
			/* "fortran function has wrong type" */
			UERROR( ALWAYS, MESSAGE(41) );
		}
		break;

	case STNAME:
	case UNAME:
	case ENAME:
		break;

	case EXTDEF:
	case EXTENT:
		if( blevel != 0 )
			cerror( ILLEGALCLASS, scnames( class ) );
		break;

	case USTATIC:
		if( blevel == 1 || paramlevel > 0 )
			cerror( ILLEGALCLASS, scnames( class ) );
		break;

	case PARAM:
	case PARAMFAKE:
		if( blevel != 1 && paramlevel == 0 )
			cerror( ILLEGALCLASS, scnames( class ) );
		break;

	case LABEL:
	case ULABEL:
		if( blevel < 2 || paramlevel > 0 )
			cerror( ILLEGALCLASS, scnames( class ) );
		break;

	case MOS:
		if( !( instruct & INSTRUCT ) )
			cerror( ILLEGALCLASS, scnames( class ) );
		break;

	case MOU:
		if( !( instruct & INUNION ) )
			cerror( ILLEGALCLASS, scnames( class ) );
		break;

	case MOE:
		if( instruct & ( INSTRUCT|INUNION ) )
			cerror( ILLEGALCLASS, scnames( class ) );
		break;

	default:
		cerror(TOOLSTR(M_MSG_239, "illegal class: %d"), class );
		/*NOTREACHED*/
	}

	return( class );
}

/* STruct to check on hashing algorithm */
static struct hashstruct {
  int match, collide, new;
  int added, removed;
  int ngrowths;
  int nextavail;
} hashstats;

int StabIdInvalid(id) 
{
  return id <= 0 || id >= hashstats.nextavail;
}
/* 
 * Hash Table. Initialized to 0. Freed slots are set to -1.
 * hash_entry is an index into the symbol table. Indexes start at 1. 
 * Index values of 0 and -1 are used to flag empty entries.
 * I made this a structure because it gives us some flexibility
 * to make changes such as making the stab a doubly linked list instead
 * of strictly a forward linked list.
 */
typedef struct {
  int	entry;
} HASH_TBL;

static HASH_TBL hash_table[HASHTSZ];
#define FL 			stab[0].nxtid

#if defined (DBG_HASH)
void PrintHash(void)
{
  register int used = 0, neverused = 0, reused = 0, i = 0;
  extern int xdebug;
  if (xdebug) {
	printf("\nHASH STATS.....................\n");
	printf("STAB:size:%d, used: %d, added:%d, removed:%d, growths:%d\n",
		   nstabents, hashstats.nextavail, 
		   hashstats.added, hashstats.removed, hashstats.ngrowths);
	printf("     new: %d, match:%d, collisions:%d\n",
		   hashstats.new, hashstats.match, hashstats.collide);
	while(i<HASHTSZ) {
	  switch(hash_table[i].entry) {
	  case -1:
		reused++;
		break;
	  case 0:
		neverused++;
		break;
	  default:
		used++;
	  }
	  ++i;
	}
	printf("HASH:Used:%d, In Use:%d, Reused:%d, Never Used:%d\n\n",
		   used + reused, used, reused, neverused);
  }
}
#endif



void debug_print_stab(int dbglevel, int id)
{
  if (dbglevel) {
#if	defined (LINT) || defined (CFLOW)
	printf("\"%s\", line %d: ", stab[id].ifname, stab[id].line);
#endif
	printf("Sym:%s(%d) %s ", stab[id].psname, id, scnames(stab[id].sclass));
	tprint(stab[id].stype);
	printf(" lev:%d, flags:0%o, offset:%d, suse:%d, unique:%d, nxt:%d\n",
		   stab[id].slevel, stab[id].sflags, stab[id].offset, 
		   stab[id].suse, stab[id].uniqid, stab[id].nxtid);
  }
}

void InitStab(void)
{
  register int i;
  if(!(stab=getmem(nstabents * sizeof(struct symtab) )))
	cerror(TOOLSTR(M_MSG_212, "stab out of space\n"));
  hashstats.nextavail = 1;
  FL = 0;
}	
static int hash(const char *name)
{
  register unsigned int i = 0;
  register const char *p;
  for( p=name; *p ; ++p ){
	i = (i<<1)+ *p;
  }
  if (ddebug > 2)
	printf("hash %s ==> %u %d\n", name, i, i%HASHTSZ);
  return i%HASHTSZ;
}
/* --------------------- ScanStab ----------------------
 * Return the index of the next stab entry by following the
 * hash table and the lists. 
 * If not at end of list, return next entry in the list
 * else return the head of the list for the next hash entry
 * Note: I took a slight performance hit by rehashing rather
 * than declaring hashid as static from the last call. 
 */
int ScanStab(int lastid)
{
  register int hashid;
  if (lastid <= 0)
	hashid = 0;
  else if (stab[lastid].nxtid > 0)
	return stab[lastid].nxtid;
  else
	hashid = hash(stab[lastid].psname) + 1;

  while(hashid < HASHTSZ) {
	if (hash_table[hashid].entry > 0)
	  return hash_table[hashid].entry;
	hashid++;
  }
  return 0;
}
/* -------------------  CopyStabEnt ------------------------
 * Copies the fields in one stab entry to another.
 */
static void CopyStabEnt(int toid, int fromid)
{
#if	defined (LINT) || defined (CFLOW)
   stab[toid].ifname	= stab[fromid].ifname;
   stab[toid].line	= stab[fromid].line;
#endif
   stab[toid].stype  = stab[fromid].stype;
   stab[toid].sclass = stab[fromid].sclass;
   stab[toid].slevel = stab[fromid].slevel;
   stab[toid].sflags = stab[fromid].sflags;
   stab[toid].offset = stab[fromid].offset;
   stab[toid].suse   = stab[fromid].suse;
   stab[toid].uniqid = stab[fromid].uniqid;
   /* This should never occur, but leave it in for the time being
	* so we can verify it. The code doesn't hurt.
	*/
   if (strcmp(stab[toid].psname, stab[fromid].psname)) {
	 free(stab[toid].psname); /* Release old name */
	 stab[toid].psname	= getmem(strlen(stab[fromid].psname)+1);
	 strcpy(stab[toid].psname, stab[fromid].psname);
   }
 }  
/* -------------------  GetEmptyStabSlot ------------------------
 * initializes a new stab entry to UNDEF.
 * allocates name, sets flags to s, and initializes
 * other fields. Inserts it into the hash table. It is always
 * inserted at the head of the list if entries already exist in this bucket.
 */
static int GetEmptyStabSlot(const char *name, short s)
{
  register int thisid, nextid, hashslot;
  /* Grow the stab by SYMTINC */
  if (FL == 0 && hashstats.nextavail >= nstabents) {
	nstabents += SYMTINC;
	hashstats.ngrowths++;
	if(!(stab=reallocmem(stab, nstabents * sizeof(struct symtab) )))
	  cerror(TOOLSTR(M_MSG_212, "stab out of space\n"));
# ifndef BUG1
	if( ddebug ){
	  printf("\tGrowing STAB from %d to %d\n", nstabents - SYMTINC, nstabents);
	}
# endif
  }
  if (FL) {
	thisid = FL;
	FL  = stab[thisid].nxtid;
  } else {
	thisid = hashstats.nextavail++;
  }
  stab[thisid].sflags	= s;
  stab[thisid].stype	= tyalloc(UNDEF);
  stab[thisid].sclass	= SNULL;
  stab[thisid].slevel	= blevel;
#if	defined (LINT) || defined (CFLOW)
  stab[thisid].ifname	= ifname;
  stab[thisid].line	= lineno;
#endif
  stab[thisid].psname	= getmem(strlen(name)+1);
  strcpy(stab[thisid].psname, name);
  hashstats.new++;
  hashstats.added++;
  /* Now insert into the hash table */
  hashslot = hash(name);
  stab[thisid].nxtid = hash_table[hashslot].entry;
  return hash_table[hashslot].entry = thisid;
}

static void FreeStabEnt(int entry)
{
  register int prev, hix;
  hix = hash(stab[entry].psname);
  /* if this is the first entry in the list, update hash_table 
   * else unlink from the chain
   */
  if (hash_table[hix].entry == entry) {
	/* When freeing a slot, set to -1 for statistical purposes */
	if (!(hash_table[hix].entry = stab[entry].nxtid))
	  hash_table[hix].entry = -1;
  } else {
	/* Locate previous entry in list */
	for(prev=hash_table[hix].entry;
		prev && stab[prev].nxtid != entry; prev = stab[prev].nxtid);
	if (prev == 0) {
	  /*"Internal error in the locctr function: STAB is not used." */
	  cerror(TOOLSTR(M_MSG_270, "Internal error: STAB is not usable.\n"));
	}
	stab[prev].nxtid = stab[entry].nxtid;
  }
  stab[entry].stype = tyalloc(TNULL);
  free(stab[entry].psname);
   stab[entry].psname = NULL;
  stab[entry].nxtid = FL;
  FL = entry;
  hashstats.removed++;
}
/* -------------------- mknonuniq --------------------
 * locate a symbol table entry for
 * an occurrence of a nonunique structure member name
 * or field 
 */

int mknonuniq(int idindex) 
{	
  register int newid;

  newid = GetEmptyStabSlot(stab[idindex].psname, SNONUNIQ | SMOS);
# ifndef BUG1
  if( ddebug ){
	printf("\tnonunique entry for %s from %d to %d\n",
		   stab[idindex].psname, idindex, newid );
  }
# endif
  return newid;
}

/* ------------ ListLook -----------------------------
 * ListLook looks up an entry in the current list.
 * If found, it returns the index of that entry. Else it returns 0
 * found is defined to be an entry which matches by name and 
 * s w.r.t.		SNSPACE, SHIDDEN and SSCOPED 
 */
int ListLook(char *name, int s, int thisid)
{
  while(thisid>0) { /* look for name */
	
	/* Check for an exact match */
	if( strcmp(stab[thisid].psname, name) == 0) {
	  if( (stab[thisid].sflags & (SNSPACE|SHIDDEN|SSCOPED)) == s ) {
# ifndef BUG1
		if( ddebug > 2 ){
		  printf("lookup(%s, 0%o) ix:%d, MATCH\n",
				 name, s, thisid );
		}
# endif
		break;
	  }
	}
	thisid=stab[thisid].nxtid;
  }
  return thisid;
}

/* ------------ lookup -----------------------------
 * lookup looks up an entry in the symbol table.
 * If found, it returns the index of that entry.
 * found is defined to be an entry which matches by name and 
 * s w.r.t.		SNSPACE, SHIDDEN and SSCOPED 
 */

int lookup( char *name, int s)
{
  /* look up name: must agree with s w.r.t.
	 SNSPACE, SHIDDEN and SSCOPED */
  
  register int hix, thisid, collide;
  /* compute hash index */
  hix = hash(name);
  if ((thisid =hash_table[hix].entry) <= 0) {
	/*
	 * Empty slot in hash table, get an entry and return
	 */
	thisid = GetEmptyStabSlot(name, s);
# ifndef BUG1
	if( ddebug > 2 ){
	  printf( "lookup(%s, 0%o), stwart=%d, instruct=%d NEW\n",
			 name, s, stwart, instruct);
	  debug_print_stab(ddebug, thisid);
	}
# endif
	return thisid;
  }
  
  /*
   * There is an entry for this hash, find out if there is a match for
   * this name space.
   */
  collide = 0;
  thisid = ListLook(name, s, thisid);

  if (thisid <= 0) { /* No match for the name space, get a new entry */

#if defined (DBG_HASH)  
	/* For debug - do we have a collision. If there are any matches
	 * in the list, then a collision has already been signalled
	 */
	for(thisid = hash_table[hix].entry;thisid > 0;thisid=stab[thisid].nxtid) {
	  if( strcmp(stab[thisid].psname, name) == 0) { /* have a match */
		collide = 0;
		break;
	  } else
		collide = 1;
	}	  
#endif
	thisid = GetEmptyStabSlot(name, s);	
  }
#if defined (DBG_HASH)  
  if( ddebug > 2 ){
	printf( "lookup(%s, 0%o), stwart=%d, instruct=%d %s\n",
		   name, s, stwart, instruct, collide ? "COLLISION" : "MATCH");
	debug_print_stab(ddebug, thisid);
  }
# endif
  if (collide)
	hashstats.collide++;
  else
	hashstats.match++;
  return thisid;
}

/* -------------------- fixlab -------------------- */

fixlab( int id ) {
  /* Fix symbol `id' to be in label namespace */
  char save_name[512], *psname;
  if( TOPTYPE(stab[id].stype) == UNDEF ){
	psname = strcpy(save_name, stab[id].psname);
	FreeStabEnt(id);
  } else {
	psname = stab[id].psname;
  }
  id = lookup(psname, SLABEL);
  return id;
}

/* -------------------- checkst -------------------- */

#ifndef checkst
static void debug_print_sym(char *sym)
{
  register int id = hash(sym);
  if (id >=0) id = hash_table[id].entry;
  printf("-------- occurrences of Symbol %s\n", sym);
  while(id > 0) {
	if( strcmp(stab[id].psname, sym) == 0)
	  debug_print_stab(1, id);
	id=stab[id].nxtid;
  }
}  
/* if not debugging, make checkst a macro */
checkst(lev){
  register int s, i=0, j;

  if (ddebug > 2) {
	printf("Symbol List----------------------------\n");
	while(i = ScanStab(i))
	  debug_print_stab(1, i);
	printf("-------------------------------\n");
  }
  while(i = ScanStab(i)) {
	if (TOPTYPE(stab[i].stype) == TNULL) {
	  debug_print_stab(1, i);
	  cerror(TOOLSTR(M_MSG_241, "check error1: %s"), stab[i].psname );
	}
	j = lookup( stab[i].psname, stab[i].sflags&SNSPACE );
	if( j != i ){
	  if (TOPTYPE(stab[j].stype) == UNDEF ||
		  stab[j].slevel <= stab[i].slevel ){
		if (stab[i].sflags & SNONUNIQ == 0 || 
		  stab[j].sflags & SNONUNIQ == 0) {
			debug_print_stab(1, i);
			debug_print_stab(1, j);
			cerror(TOOLSTR(M_MSG_241, "check error2: %s"), stab[j].psname );
		  }
	  }
	} else if( stab[i].slevel > lev ) {
	  debug_print_sym(stab[i].psname);
	  cerror(TOOLSTR(M_MSG_242, "%s check3 at level %d"),
			 stab[i].psname, lev );
	}
  }
}
#endif


/* -------------------- clearst -------------------- 
 * clear entries of internal scope  from the symbol table 
 * Note: A couple of the cerrors are internal in nature and may not fully
 * agree with the message catalog. These were added to trap on 
 * a symbol table inconsistency.
 */
void clearst(void){ 
  register int temp;
  register int gid, id, nextid;

  temp = lineno;

  aobeg();
  /* Search through the stab. We must preserve the previous valid
   * id since freeing an entry removes it from the list
   */

#if DBG_HASH
	 if (ddebug) {
	   printf("CLEARST level %d--------------------------\n", blevel);
   }
#endif
  nextid = ScanStab(0);
  while(id = nextid) {
	nextid = ScanStab(id); /* Point to next entry */

	if (TOPTYPE(stab[id].stype) == TNULL) {
	  debug_print_stab(1, id);
	  cerror(TOOLSTR(M_MSG_241, "CLEARST:check error: %s"), stab[id].psname );
	}
#if DBG_HASH
	   if (ddebug) {
		 debug_print_stab(1, id);
	   }
#endif

	lineno = stab[id].suse;
	if( lineno < 0 ) lineno = - lineno;
	
	if( stab[id].slevel > blevel ){ /* must clobber */
	  if (TOPTYPE(stab[id].stype) == UNDEF
		  || ( stab[id].sclass == ULABEL && blevel < 2 ) ){
		lineno = temp;
		/* "%s undefined" */
		WARNING( ALWAYS, MESSAGE(4), stab[id].psname );
	  }
	  else 
		aocode(&stab[id]);
# ifndef BUG1
	  if (ddebug)
		printf("removing %s from stab[ %d], flags %o level %d\n",
			   stab[id].psname, id, stab[id].sflags, stab[id].slevel);
# endif
	  if( stab[id].sflags & SHIDES ){
		if( ( stab[id].sflags & SEXTRN ) &&
		   ( gid = extrndec( id ) ) >= 0 ){
		  /* Copy over usage info */
		  if( stab[id].suse < 0 )
			stab[gid].suse = stab[id].suse;
#ifdef XCOFF
		  stab[gid].sflags |= stab[id].sflags &
			(SSET|SREF|SFCALLED|SFADDR);
#else
		  stab[gid].sflags |= stab[id].sflags & (SSET|SREF);
#endif
		  stab[id].sflags &= ~SEXTRN;
		}
		unhide(id);
	  }
	  if( ( stab[id].sflags & (SEXTRN|SSCOPED) ) == SEXTRN &&
		 stab[id].suse < 0 ){
		/* look through chain */
		gid = ListLook( stab[id].psname,
					   ( stab[id].sflags & SNSPACE ) | SSCOPED , 
					   stab[id].nxtid) ;
		/* No more valid entries for this guy */
		if (gid > 0 ) {
		  if (TOPTYPE(stab[gid].stype) == UNDEF ){
			stab[id].sflags |= SSCOPED;
			stab[id].slevel = 0;
			dimptr->cextern = 1;
			stab[id].stype = copytype(stab[id].stype, 0);
			printf("Found UNDEF'd Entry in STAB\n");
			debug_print_stab(1, gid);
			if (nextid == gid)
			  nextid = ScanStab(gid);
			FreeStabEnt(gid);
			continue;
		  } else if( ISFTN(stab[id].stype) == ISFTN(stab[gid].stype) ){
			if( !comtypes( stab[id].stype, stab[gid].stype, 0 ) )
			  /* "external symbol type clash for %s" */
			  WARNING( WDECLAR, MESSAGE(193),
					  stab[id].psname );
			stab[gid].suse = stab[id].suse;
#ifdef XCOFF
			stab[gid].sflags |= stab[id].sflags &
			  (SSET|SREF|SFCALLED|SFADDR);
#else
			stab[gid].sflags |= stab[id].sflags & (SSET|SREF);
#endif
		  } else {
			/* "external symbol type clash for %s" */
			UERROR( ALWAYS, MESSAGE(193),
				   stab[id].psname );
		  }
		}
	  }
	  FreeStabEnt(id);
	}
  }
  lineno = temp;
  aoend();
}

/* -------------------- hide -------------------- */

static int hide( int id, int extcpy ) {
  register int newid;
  
  /* make new entry and mark as hides a previous */
  newid = GetEmptyStabSlot(stab[id].psname, 
						   ( stab[id].sflags & SNSPACE ) | SHIDES);
  /* Mark current entry as hidden */
  stab[id].sflags |= SHIDDEN;
  if( !extcpy && blevel != 1 && paramlevel == 0 )
	/* "%s redefinition hides earlier one" */
	WARNING( (WDECLAR || WHEURISTIC) && WKNR, MESSAGE(2), stab[id].psname );
  
# ifndef BUG1
  if( ddebug ) printf( "\t%s :%d hidden in %d\n", stab[id].psname, id, newid); 
# endif
  return( idname = newid );
}

/* -------------------- extrndec -------------------- */

int extrndec( int id ) 
{
  
  register int s, nxtid = id;
  /* GAF - need to just look down the list */
  /*
   * for an extern decalaration in a block,
   * the identifier referenced is the one declared
   * at file scope. we attempt to find it.
   *
   * we call this function when we have an "extern"
   * declaration of an identifier at block level (blevel >1).
   */
  
  s = stab[id].sflags & SNSPACE;
  while(nxtid = stab[nxtid].nxtid) {
	
	if( TOPTYPE(stab[nxtid].stype) == TNULL) {
	  debug_print_stab(1, nxtid);
	  cerror(TOOLSTR(M_MSG_270, "Internal error in extrndec: STAB is not usable.\n"));
	}
	if ( (stab[nxtid].sflags & SNSPACE ) == s &&
		strcmp(stab[id].psname, stab[nxtid].psname) == 0 ){
	  /* found the name */
# ifndef BUG1
	  if( ddebug )
		printf("extrndec of: %s:%d might be %d\n",
			   stab[id].psname, id, nxtid);
# endif
	  /*
	   * if the declaration found is not
	   * at file scope, then it is not
	   * the one we want.
	   */
	  switch( stab[nxtid].sclass ){
	  case EXTERN:
	  case EXTENT:
	  case EXTDEF:
	  case USTATIC:
	  case FORTRAN:
	  case UFORTRAN:
		break;
		
	  case STATIC:
		if( stab[nxtid].sflags & SEXTRN )
		  /* global symbol */
		  break;
		
	  default:
		continue;
	  }
	  
	  /* Found it! */
# ifndef BUG1
	  if( ddebug )
		printf( "extrndec of %s:%d is %d\n",
			   stab[id].psname, id, nxtid);
# endif
	  return nxtid;
	}
	
	if( ddebug )
	  printf( "extrndec of %s:%d not found\n",  stab[id].psname, id);
	return -1;
  }
}

/* -------------------- unhide -------------------- */

static void unhide( int id ) 
{
  register int s, hiddenid;
  
  s = stab[id].sflags & SNSPACE;
  for(hiddenid = stab[id].nxtid; 
	  hiddenid > 0;
	  hiddenid = stab[hiddenid].nxtid) {
	
	if( ( stab[hiddenid].sflags & (SNSPACE|SSCOPED) ) == s ){
	  if (!strcmp(stab[id].psname, stab[hiddenid].psname)){
		/* found the name */
		stab[hiddenid].sflags &= ~SHIDDEN;
# ifndef BUG1
		if( ddebug )
		  printf("unhide uncovered %s:%d from %d\n", 
				 stab[hiddenid].psname, id, hiddenid);
# endif
		return;
	  }
	}
	
  }
  cerror(TOOLSTR(M_MSG_286, "unhide fails" ));
}

/* -------------------- attachProto ------------------ */

AttachProto(p)
	 NODE *p;
{
  /* build up a prototype list and attach it to
   * the right node of the node p.  This will be
   * checked by tyreduce to attach the parameter list
   * to the FTN type node when it is built.
   */

	NODE *new;
	PPTR CreateProto();
	register int i;

	/* since this node is only a place holder for
	 * the parameter list, it doesn't matter if
	 * it's wrong.
	 */
	i = protopop();
	new = p->in.right = mkty( INCREF(tyalloc(INT),FTN) );
	paramlevel--;
	blevel--;
	new->in.type->ftn_parm = CreateProto(i);
	return( i );
}

/* -------------------- CreateProto -------------------- */

PPTR
CreateProto(firstparam)
int firstparam;
{
	/* create a parameter list structure from the
	 * parameters on the paramstk from firstparam
	 * to the top of the stack (paramno)
	 */

	PPTR origp;
	register PPTR *p = &origp;
	register PPTR pNew;
	int i;

	/* pick up each identifier from parameter stack and
 	 * add its type to the parameter list.
	 */

	for(i=firstparam; i<paramno ; i++){
		*p = pNew = parmalloc();
		pNew->type = unqualtype( stab[paramstk[i]].stype );
		p = &pNew->next;
	}
	*p = PNIL;

	return ( origp );
}

int prototop = 0;

protopush( i ){
	if( prototop >= protosz + 3 ){ /* then make protostk big enough */
		protosz = prototop + 20;
		protostk = (int *)realloc(protostk,protosz*sizeof(int));
		if ( protostk == NULL )
			cerror(TOOLSTR(M_MSG_243, "prototype stack overflow"));
		}
	protostk[ prototop++ ] = i;
	protostk[ prototop++ ] = curclass;
	protostk[ prototop++ ] = paramFlg;
	protostk[ prototop++ ] = instruct;
	instruct = 0;
}

protopop() {
	if ( prototop < 4 ) cerror(TOOLSTR(M_MSG_244, "protopop: dropped below stack"));
	instruct = protostk[ --prototop ];
	paramFlg = protostk[ --prototop ];
	curclass = protostk[ --prototop ];
	return ( protostk[ --prototop ] );
}

#define FAKENAME "%%FAKE%d"

char *
getFakeName()
{
	static int counter=0;
	char *s, *FakeNamealloc();

	s = FakeNamealloc();
	sprintf(s,FAKENAME,++counter);
	return( s );
}

char *
FakeNamealloc()
{
	/* use the generic alloc routine to get blocks of memory
	 * to store the generated fakenames in.  The maximum fakename
	 * that can be stored without causing problems is
	 * "%FAKE99999999".
	 */
    static int fkbunchsize = 0;
    static char * fakebunch;

    if (fkbunchsize == 0) {
	/* Allocate another bunch of type nodes */
	fakebunch = (char *)getmem(MAXBUNCH * (sizeof(FAKENAME)+5));
	fkbunchsize = MAXBUNCH;
    }
    return (&fakebunch[--fkbunchsize * (sizeof(FAKENAME)+5)]);
}

/* -------------------- makeghost ------------------ */

makeghost(realid)
int realid;
{
	int ghostid;
	char *ghostname;

	/*
	 * initialize the "ghost" of the
	 * identifier.
	 * the ghost identifier has:
	 * 1. a derivative name of the real id.
	 * 2. static declaration local to the
	 * block.
	 *
	 * first: allocate enough space for the
	 * realname, plus two % and a null
	 * then create new stab entry with a statictorage class
	 * and the same type as the real id.
	 * EXCEPTION: static automatic char arrays. for these
	 * the real id is the ghost id.
	 *
	 * we assume this function is called for automatic variables
	 */

	if (curclass == STATIC)
		return(realid);

	ghostname = (char *) malloc(strlen(stab[realid].psname) + 3);
	if (ghostname == NULL)
		cerror(TOOLSTR(M_MSG_245, "memory allocation problem for a ghost identifier"));
	strcpy(ghostname, "%%");
	strcat(ghostname, stab[realid].psname);
	ghostid = lookup(ghostname, 0);
	stab[ghostid].uniqid = LocalUniqid++;
	stab[ghostid].stype= copytype(stab[realid].stype, blevel);
	stab[ghostid].slevel = blevel;
	stab[ghostid].sclass = STATIC;
	stab[ghostid].offset= getlab();
	stab[ghostid].suse   = -lineno;
#ifndef XCOFF
	StabInfoPrint(&stab[ghostid]);
#endif
	return(ghostid);
}
