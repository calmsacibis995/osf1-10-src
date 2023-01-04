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
static char *rcsid = "@(#)$RCSfile: __cfe_support.c,v $ $Revision: 1.1.6.3 $ (DEC) $Date: 1993/06/08 01:23:05 $";
#endif



/*
** Rationale:  This code exists in libc to support cfe's implementation
** of optimizations to printf as suggested by Robert Morgan.   
**
** Certain forms of printf degenerate into essentially putc, fputc, puts,
** and fputs.   In order to support the ANSI Standard as regards return
** values from printf, we've encapsulated this functionality into
** jacket routines included herein.
*/


/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#include <stdio.h>


int __CFE_print_puts( char *x )
{
 return( fputs( x, stdout  ));
}


int __CFE_print_putc( int x )
{
 putc( x, stdout );
 return( 1 );
}


int __CFE_print_putc_nl( int x )
{
 putc(x, stdout ); putc( '\n',stdout );
 return( 2 );
}


int __CFE_fprint_puts_nl( char *x, FILE *f )
{ 
 register int i;
 i = fputs( x, f ); 
 putc( '\n', f  ); 
 return( i+1 );
 
}

int __CFE_fprint_putc( int x, FILE *f )
{
 putc( x, f );
 return( 1 );
}


int __CFE_fprint_putc_nl( int x, FILE *f )
{
 putc( x,f ); putc( '\n', f );
 return(2);
}

