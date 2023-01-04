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
 *	@(#)$RCSfile: treewalk.h,v $ $Revision: 4.2.6.3 $ (DEC) $Date: 1993/11/22 21:23:08 $
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
 * COMPONENT_NAME: (CMDPROG) treewalk.h
 *
 * FUNCTIONS: fwalk, tprint, walkf                                           
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

/* The ordering of this array corresponds to that in m_ind/manifest.h */
char * tnames[NBTYPES] = {
  "null",
  "ellipsis",
  "farg",
  "moety",
  "SIGNED",
  "undef",
  "VOID",
  "CHAR",
  "SIGNED CHAR",
  "SHORT",
  "INT",
  "LONG",
  "LONG LONG",
  "FLOAT",
  "DOUBLE",
  "LONG DOUBLE",
  "STRTY",
  "UNIONTY",
  "ENUMTY",
  "UNSIGNED CHAR",
  "UNSIGNED SHORT",
  "UNSIGNED",
  "UNSIGNED LONG",
  "UNSIGNED LONG LONG"
  };

/* -------------------- fwalk -------------------- */

fwalk( t, f, down ) register NODE *t; int (*f)(); {

	int down1, down2;

	more:
	down1 = down2 = 0;

	(*f)( t, down, &down1, &down2 );

	switch( optype( t->in.op ) ){

	case BITYPE:
		fwalk( t->in.left, f, down1 );
		t = t->in.right;
		down = down2;
		goto more;

	case UTYPE:
		t = t->in.left;
		down = down1;
		goto more;

		}
	}

/* -------------------- walkf -------------------- */

walkf( t, f ) register NODE *t;  int (*f)(); {
	register opty;

	opty = optype(t->in.op);

	if( opty == UTYPE || opty == BITYPE) walkf( t->in.left, f );
	if( opty == BITYPE ) walkf( t->in.right, f );
	(*f)( t );
	}


/* -------------------- tprint -------------------- */
#ifndef PASS_TWO
# ifndef BUG4
tprint( t )
TPTR t;
{ /* output a nice description of the type of t */

	PPTR p;
	TWORD bt;

	for( ;; t = DECREF(t) ){

		if( ISCONST(t) ) printf( "const " );
		if( ISVOLATILE(t) ) printf( "volatile " );
		if( ISUNALIGNED(t) ) printf( "__unaligned " );

		if( ISPTR(t) ) printf( "PTR " );
 		else if( ISFTN(t) ){
 			printf( "FTN (" );
			if( ( p = t->ftn_parm ) != PNIL ){
				for( ;; ){
					tprint( p->type );
					if( ( p = p->next ) == PNIL ) break;
					printf( ", " );
				}
 			}
			printf( ") " );
 		}
		else if( ISARY(t) ) printf( "ARY[%.0d] ", t->ary_size );
		else {
			if( ISTSIGNED(t) ) printf( "<signed> " );
			if( HASCONST(t) ) printf( "<HASCONST> " );
			if( HASUNALIGNED(t) ) printf( "<HASUNALIGNED> " );
			if( HASVOLATILE(t) ) printf( "<HASVOLATILE> " );
			printf( tnames[bt = TOPTYPE(t)] );
			if( t->typ_size != bt ) printf( "(0%o)", t->typ_size );
			return;
		}
	}
}
# endif /* BUG4 */
# endif /* PASS_TWO */



