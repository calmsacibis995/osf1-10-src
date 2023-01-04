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
 *	@(#)$RCSfile: kdbdefine.h,v $ $Revision: 1.2 $ (DEC) $Date: 1992/01/15 01:11:55 $
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
 * derived from kdbdefine.h	2.1	(ULTRIX/OSF)	12/3/90
 */

#define access	kdbaccess
#define bchkget	kdbbchkget
#define charpos	kdbcharpos
#define chkerr	kdbchkerr
#define chkget	kdbchkget
#define chkloc	kdbchkloc
#define command	kdbcommand
#define convdig	kdbconvdig
#define convert	kdbconvert
#define delbp	kdbdelbp
#define digit	kdbdigit
#define	echo	kdbecho
#define editchar	kdbeditchar
#define endline	kdbendline
#define eol	kdbeol
#define eqstr	kdbeqstr
#define eqsym	kdbeqsym
#define error	kdberror
#define execbkpt	kdbexecbkpt
#define execbkptf	kdbexecbkptf
#define exform	kdbexform
#define expr	kdbexpr
#define findsym	kdbfindsym
#define flushbuf	kdbflushbuf
#define get	kdbget
#define getformat	kdbgetformat
#define getnum	kdbgetnum
#define getreg	kdbgetreg
#define getreg_val	kdbgetreg_val
#define getsig	kdbgetsig
#define inkdot	kdbinkdot
#define item	kdbitem
#define length	kdblength
#define localsym	kdblocalsym
#define lookup	kdblookup
#define newline	kdbnewline
#define nextchar	kdbnextchar
#define nextpcs	kdbnextpcs
#define printc	kdbprintc
#define printdate	kdbprintdate
#define printdbl	kdbprintdbl
#define printesc	kdbprintesc
#define printf	kdbprintf
#define printins	kdbprintins
#define printnum	kdbprintnum
#define printoct	kdbprintoct
#define printpc	kdbprintpc
#define printregs	kdbprintregs
#define prints	kdbprints
#define printtrace	kdbprinttrace
#define psymoff	kdbpsymoff
#define put	kdbput
#define putchar	cnputc
#define quotchar	kdbquotchar
#define rdc	kdbrdc
#define read	kdbread
#define readchar	kdbreadchar
#define readsym	kdbreadsym
#define round	kdbround
#define runpcs	kdbrunpcs
#define rwerr	kdbrwerr
#define sbrk	kdbsbrk
#define scanbkpt	kdbscanbkpt
#define scanform	kdbscanform
#define setbp	kdbsetbp
#define shell	kdbshell
#define sigprint	kdbsigprint
#define subpcs	kdbsubpcs
#define symchar	kdbsymchar
#define term	kdbterm
#define valpr	kdbvalpr
#define varchk	kdbvarchk
#define within	kdbwithin
#define write	kdbwrite

#define adrflg	kdbadrflg
#define adrval	kdbadrval
#define bkpthead	kdbbkpthead
#define bpstate	kdbbpstate
#define callpc	kdbcallpc
#define cntflg	kdbcntflg
#define cntval	kdbcntval
#define cursym	kdbcursym
#define digitptr	kdbdigitptr
#define ditto	kdbditto
#define dot	kdbdot
#define dotinc	kdbdotinc
#define eof	kdbeof
#define eqformat	kdbeqformat
#define erasec	kdberasec
#define errflg	kdberrflg
#define esymtab	kdbesymtab
#define executing	kdbexecuting
#define expv	kdbexpv
#define fpenames	kdbfpenames
#define illinames	kdbillinames
#define isymbol	kdbisymbol
#define itolws	kdbitolws
#define lastc	kdblastc
#define lastcom	kdblastcom
#define lastframe	kdblastframe
#define line	kdbline
#define localval	kdblocalval
#define locmsk	kdblocmsk
#define locval	kdblocval
#define loopcnt	kdbloopcnt
#define lp	kdblp
#define maxoff	kdbmaxoff
#define maxpos	kdbmaxpos
#define pcb	kdbpcb
#define peekc	kdbpeekc
#define pid	kdbpid
#define printbuf	kdbprintbuf
#define printptr	kdbprintptr
#define radix	kdbradix
#define reglist	kdbreglist
#define regname	kdbregname
#define savframe	kdbsavframe
#define savlastf	kdbsavlastf
#define savpc	kdbsavpc
#define sbr	kdbsbr
#define signals	kdbsignals
#define signo	kdbsigno
#define space	kdbspace
#define stformat	kdbstformat
#define symtab	kdbsymtab
#define userpc	kdbuserpc
#define var	kdbvar

#define curmap	kdbcurmap
#define curpcb	kdbcurpcb
#define curpid	kdbcurpid

#define setexit() (kdbsetjmp(&kdb_save) ? kdb_setjmp_val : 0)
#define reset(x)  (kdb_setjmp_val = x, kdblongjmp(&kdb_save))
