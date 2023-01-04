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
static char *rcsid = "@(#)$RCSfile: mmap_32.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/27 21:47:39 $";
#endif

/*
 * 
 * Description:  This routine is a "wrapper" for the mmap() system call for
 * use in programs restricting their address space to be addressible with 31
 * bits.  In certain cases, it modifies the parameters passed to prevent the
 * system from choosing an inappropriate mapping address.  In all cases it
 * checks the returned mapping address range and can be customized as to
 * how an error should be reported.
 * 
 * Inputs: same arguments as mmap()
 * 
 * Outputs: same as mmap()
 * 
 * Usage:  To use this routine in a program, the compilation flag
 * -Dmmap=_mmap_32_ must be used on all modules (or at least all that refer
 * to mmap).
 * 
 */

/*
 * Just in case the compilation flag given in the Usage note has been
 * applied to this file, too
 */
#ifdef mmap
#undef mmap
#endif /* defined(mmap) */

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/errno.h>

#if	defined(_MMAP_32_USERFUNC) || !defined(_MMAP_32_NOPRINT)
#include <stdio.h>
#endif	/*  _MMAP_32_USERFUNC || ! _MMAP_32_NOPRINT*/

#ifdef	_MMAP_32_LOGERR
#include <syslog.h>
#endif	/* _MMAP_32_LOGERR */

/*
 * The values in this array can be modified by the user to adapt to the
 * needs of a particular application.  Choose addresses in the range:
 *      0x0000 0000 0001 0000 to 0x0000 0000 7ffe 0000.
 * The addresses need not be different from each other, but avoid overlap 
 * with regions already allocated to the program.
 */
static caddr_t	_mmap_32_addressconf[] = {

	(caddr_t)0x58000000L,	/* _MMAP_32_MAPTEXT */

#define _MMAP_32_MAPTEXT	0


	(caddr_t)0x60000000L,	/* _MMAP_32_MAPDATA */

#define _MMAP_32_MAPDATA	1


	(caddr_t)0x68000000L	/* _MMAP_32_MAPBSS  */

#define _MMAP_32_MAPBSS		2
	};

#define _MMAP_32_MAXVA	(caddr_t)0x7fffffffL

 caddr_t
_mmap_32_(
	caddr_t	addr,
	size_t	len,
	int	prot,
	int	flags,
	int	fd,
	off_t	off)
{
/*
 Decision tree for input arguments:

 addr      flags		Action

  0      MAP_FIXED		[This case mimics what happens in
				the kernel code, but may give
				behavior different than described
				in mmap(2).]
				Select alternate mapping address
				from _mmap_32_addressconf_ array,
				depending on whether anonymous or
				writable.

  0	 MAP_VARIABLE		Use alternative mapping address as
  				above.

  non-0	 MAP_FIXED		No modifications.

  non-0	 MAP_VARIABLE		No modifications.

 */
	int	ac_selector;
	int	severity = 0;
	caddr_t	use_address = addr;
	caddr_t result;

	if( use_address == 0L){

	    if( (flags&MAP_FIXED)) severity = 1;
			/* this situation merits a severity code because
			 * the AES and the Rel 1.2 manpages describe a
			 * behavior different from what was implemented.
			 * The implementation was chosen to behave more
			 * similarly to "traditional implementations of
			 * mmap().  No "default exact mapping address"
			 * handling is done.
			 */

	    if( (flags&MAP_TYPE)==MAP_FILE ){
		if( prot&PROT_WRITE )
		    ac_selector = _MMAP_32_MAPDATA;
		  else
		    ac_selector = _MMAP_32_MAPTEXT;
	      } else  /* MAP_TYPE == MAP_ANON */
		ac_selector = _MMAP_32_MAPBSS;

	    use_address = _mmap_32_addressconf[ac_selector];
	  }

	result = mmap( use_address, len, prot, flags, fd, off);

/* 
 *************************************************************
 *  
 * Severity assessment
 *  4   System call mmap() returned -1
 *  3	Request for in-bounds mapping returns out-of-bounds mapping
 *  2	Successful request for out-of-bounds mapping
 *  1	Request with addr 0 and MAP_FIXED
 *  0	Normal
 *
 * Reporting
 *  A	fprintf to stderr (unless NOPRINT or USERFUNC)
 *  B	syslog (if LOGERR)
 *  C   user supplied function (USERFUNC)
 *
 * Action
 *  X	Normal return (when severity is less than ERROR)
 *  Y	-1 Error return (ENOMEM) (when severity exceeds ERROR)
 *  Z	Fatal (when severity exceeds FATAL)
 *  
 */
	if( (result == (caddr_t)(-1)) ) severity = 4;
	   else if( ((result+len) > _MMAP_32_MAXVA) )
		severity = ( addr+len > _MMAP_32_MAXVA ) ? 2 : 3 ;

#ifndef	_MMAP_32_FATAL
#define	_MMAP_32_FATAL	5	/* Default is that no errors are FATAL */
#endif	/* _MMAP_32_FATAL */

#ifndef	_MMAP_32_ERROR
#define	_MMAP_32_ERROR	3	/* Default respects out-of-bounds requests */
#endif	/* _MMAP_32_ERROR */

#ifndef	_MMAP_32_WARNING
#define	_MMAP_32_WARNING 1	/* Default warns of suspicious situations */
#endif	/* _MMAP_32_WARNING */

#ifdef	DEBUG
#define _MMAP_32_DEBUG	1
#endif	/* DEBUG */

#ifndef	_MMAP_32_DEBUG
#define	_MMAP_32_DEBUG	5	/* Default is to turn off debugging */
#endif	/* _MMAP_32_DEBUG */

	if( severity < 4 ){
	    if( severity >= _MMAP_32_ERROR) {
	      munmap( result, len);
	      errno = ENOMEM;
	      }
	  }

#if	defined(_MMAP_32_USERFUNC) || !defined(_MMAP_32_NOPRINT)
	{   char buf[BUFSIZ];
	    char *sev_string = (char *) 0;
	
	    if( severity >= _MMAP_32_FATAL) sev_string = "Fatal";
	      else if( severity >= _MMAP_32_ERROR ) sev_string = "Error";
	      else if( severity >= _MMAP_32_WARNING ) sev_string = "Warning";
	      else if( severity >= _MMAP_32_DEBUG ) sev_string = "Debug";

	    if( sev_string != (char *) 0)
	      {
		sprintf( buf,
		    "%s: mmap returned %lx severity %d",
			    sev_string, result, severity);

#ifdef	_MMAP_32_USERFUNC
		_MMAP_32_USERFUNC(buf);
#else	/* here without _MMAP_32_USERFUNC implies ! _MMAP_32_NOPRINT */	
		fprintf( stderr, "_mmap_32: %s\n", buf);
#endif	/* _MMAP_32_USERFUNC */

	      }
	}
#endif	/*  _MMAP_32_USERFUNC || ! _MMAP_32_NOPRINT*/

#ifdef	_MMAP_32_LOGERR
	{   int priority = 0;
	
	    if( severity >= _MMAP_32_FATAL) priority = LOG_ERR;
	      else if( severity >= _MMAP_32_ERROR ) priority = LOG_ERR;
	      else if( severity >= _MMAP_32_WARNING ) priority = LOG_WARNING;
	      else if( severity >= _MMAP_32_DEBUG ) priority = LOG_DEBUG;

	    if( priority != 0)
		syslog( priority, 
		    "mmap returned %lx severity %d", result, severity);
	}
#endif	/* _MMAP_32_LOGERR */

#if	_MMAP_32_FATAL < 5
	if( severity >= _MMAP_32_FATAL ) abort();
#endif	/* _MMAP_32_FATAL < 5 */

	return (severity >= _MMAP_32_ERROR)? (caddr_t)(-1) : result;
}
