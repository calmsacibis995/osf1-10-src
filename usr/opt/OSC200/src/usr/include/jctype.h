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
 *	@(#)$RCSfile: jctype.h,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/04/15 09:49:20 $
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
 * COMPONENT_NAME: jctype.h
 *                                                                    
 * ORIGIN: IBM
 *
 * Copyright International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the Rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */                                                                   

/*	jctype - Japanese (SJIS) extensions to ctype
 *						rcd  12-Apr-88
 */

#ifndef _JCTYPE_H_
#define _JCTYPE_H_

#include <standards.h>

extern unsigned int	_jistoatab[][91];
extern unsigned short	_atojistab[];
extern unsigned char	_jctype0_[], _jctype1_[][256];

#define _tojupper(c)	((c)-0x21)
#define _tojlower(c)	((c)+0x21)

/* Note that encodings are not strictly bit-per-attribute. */
#define	_J1	0xc0		/* field 1: japanese-symbol mask */
#define	_Jk	0x40		/* katakana */
#define	_JH	0x80		/* hiragana */
#define	_JK	0xc0		/* kanji */

#define	_JA	0x10		/* upper case alphabetic */
#define	_Ja	0x20		/* lower case alphabetic */

#define	_J2	0x0c		/* field 2: hex/digit/punct mask */
#define	_JX	0x04		/* hex-digit flag */
#define	_JD	0x0c		/* digit = numeral and hex */
#define	_JB	0x14		/* upper case hex letter */
#define	_Jb	0x24		/* lower case hex letter */

#define	_JP	0x08		/* punct (looks like non-hex numeral) */
#define	_JG	0x02		/* other graphic character */
#define	_JR	0x01		/* reserved (unassigned but valid) char */

#ifndef lint

#define	_jattr(c)	(_jctype1_[_jctype0_[(unsigned short)(c)>>8]][(c)&0xff])
#define	_legaltop(c)	(_jctype0_[c] > 1)
#define	_legalbot(c)	(_jctype1_[7][c] != 0)

#define isj1bytekana(c) (NCchrlen(c) == 1 && isjkata(c))
#endif /* NOT lint */
#ifdef   _ANSI_C_SOURCE
#ifdef _NO_PROTO
extern int isjalpha();
extern int isjalnum();
extern int isjdigit();
extern int isjpunct();
extern int isjprint();
extern int isjspace();
extern int isjgraph();
extern int isjxdigit();
extern int isjlower();
extern int isjupper();
extern int isjparen();
extern int isjkanji();
extern int isjhira();
extern int isjkata();
extern int tojupper();
extern int tojlower();

#else /* _NO_PROTO */
#if defined(__STDC__) || defined(__cplusplus)
#if defined(__cplusplus)
extern "C"
{
#endif

extern int isjalpha(int);
extern int isjalnum(int);
extern int isjdigit(int);
extern int isjgraph(int);
extern int isjprint(int);
extern int isjpunct(int);
extern int isjspace(int);
extern int isjxdigit(int);
extern int isjupper(int);
extern int isjlower(int);
extern int isjparen(int);
extern int isjkanji(int);
extern int isjhira(int);
extern int isjkata(int);
extern int tojlower(int);
extern int tojupper(int);

#if defined(__cplusplus)
}
#endif
#endif
#endif /* _NO_PROTO */
#endif /*  _ANSI_C_SOURCE */

#endif /* _JCTYPE_H_ */
