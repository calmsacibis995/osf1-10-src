/*
/*
 *  Title:	reginit.h
 *
 *
 *  +------------------------------------------------------------------------+
 *  | Copyright © 1988, 1993 Digital Equipment Corporation.  All Rights      |
 *  | Reserved.  Unpublished rights reserved under the copyright laws of     |
 *  | the United States.                                                     |
 *  |                                                                        |
 *  | The software contained on this media is proprietary to and embodies    |
 *  | the confidential technology of Digital Equipment Corporation.          |
 *  | Possession, use, duplication or dissemination of the software and      |
 *  | media is authorized only pursuant to a valid written license from      |
 *  | Digital Equipment Corporation.                                         |
 *  |                                                                        |
 *  | RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure by the      |
 *  | U.S. Government is subject to restrictions as set forth in             |
 *  | Subparagraph (c)(1)(ii) of DFARS 252.227-7013, or in FAR 52.227-19,    |
 *  | as applicable.                                                         |
 *  |                                                                        |
 *  | The information in this software is subject to change  without  notice |
 *  | and  should  not  be  construed  as  a commitment by Digital Equipment |
 *  | Corporation.                                                           |
 *  |                                                                        |
 *  | DIGITAL assumes no responsibility for the use or  reliability  of  its |
 *  | software on equipment which is not supplied by DIGITAL.                |
 *  +------------------------------------------------------------------------+
 *  
 *  
 *  Module Abstract:
 *
 *  Procedures contained in this module:
 *
 *  Author:
 *
 *		David Larrick
 *		7-Feb-83
 *
 *  Modification history:
 *
 *  Eric Osman		30-Jul-1993	BL-D
 *	- Merge vxt and vms decterm sources.
 *
 * Pro 350 version of reginit.req
 */

#ifndef H_REGINIT_H
#define H_REGINIT_H 0

/** Note: see the Bliss equivalence below for comments. **/
/* used single characters for initializer because string too long */

static	char	reg_init[] =
	";S(A[0,0][799,479],C1,I0)					\
	  T(A0,D0,S1,I0,B)						\
	  W(F15,I3,M1,V,N0,P1(M2),S1,S0)				\
	  P[0,0];";

/*****	Bliss equivalent -- left in to preserve the structured comments.  ****/
/**
* bind
*	reg_init = uplit byte(
*					!After an invokation of NEW_SCANNER,
*					! this init string will be passed
*					! to the parser.
*	';',				!Reinit parsing
*
*	'S(',				! screen options
*	'A[0,0][959,599]',		!Init screen addressing
*	'C1',				! turn the cursor on
*	'I0',				!Init background intensity
*	')',				! endo of S options
*
*	'S[0,0]',			!Undo any scrolling
*
*	'W(',				! W options
*	'F3',				!Init foreground mask
*	'I3',				!Init foreground intensity
*	'M1',				!Init pixel vector multiplier
*	'V',				! overlay writing mode
*	'N0',				! negate off
*	'P1(M2)',			!Init pattern register and multiplier
*	')',				! end of W options
*
*	'P[0,0]',			!Init current position
*
*	'W(S0)',			!Init shading (off)
*
*	'T(',				! T options
*	'A0',				!Init text alphabet
*	'D0',				!Init text rotation
*	'S1',				!Init text size
*	'I0',				!Init text italic
*	')',				! end of T options
*
*	0	);			!Finish with a null - ASCIZ
**/
#define	REG_INIT_SIZE	( sizeof ( reg_unit ) )
/**
*	CMS REPLACEMENT HISTORY 
**/
/**
*	*1 A_VESPER 14-SEP-1983 14:03:01 ""
**/

#endif
