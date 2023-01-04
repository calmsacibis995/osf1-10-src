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
static char	*sccsid = "@(#)$RCSfile: NLunesctab.c,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1993/06/08 01:19:02 $";
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

#if !defined(lint) && !defined(_NOIDENT)

#endif

/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: NLunesctab, _NLunescval
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * sccsid[] = "NLunesctab.c	1.9  com/lib/c/nls,3.1,9021 3/23/90 16:44:03";
 */
 
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#include <NLctype.h>

/*
 * NAME: NLunesctab
 *
 * NOTE: A table to control translation from informtion-preserving ASCII
 *       mnemonic to NLS code point.
 */
/*
 *
 *  Table to control translation from information-preserving ASCII
 *  mnemonic to NLS code point (plus function to access table).
 *  _NLunescval returns the NLchar corresponding to the mnemonic
 *  given as input, or -1 if the mnemonic is unknown.  If a
 *  corresponding NLchar is found, it is written to the input
 *  NLchar pointer as well.
 */

#ifndef KJI
/* 
 *  Code page offsets
 */
#define P0B  0

struct NLescdata _NLunesctab[] = {

/* To work this table MUST BE SORTED by first field */
/* Beware! backslashes for C's sake make things sort out of order */

(unsigned char *)"!",      P0B+0xad,    /* SP03       Spanish exclamation sign*/
(unsigned char *)"##",     P0B+0xff,    /*            All ones                */
(unsigned char *)"#1",     P0B+0xb0,    /* SF14       Quarter hashed          */
(unsigned char *)"#2",     P0B+0xb1,    /* SF15       Half hashed             */
(unsigned char *)"#3",     P0B+0xb2,    /* SF16       Full hashed             */
(unsigned char *)"&m",     P0B+0xe6,    /* GM01/SM17  Mu small (Micro)        */
(unsigned char *)"+-",     P0B+0xf1,    /* SA02       Plus or minus           */
(unsigned char *)"--",     P0B+0xee,    /* SM15       overbar                 */
(unsigned char *)"-.",     P0B+0xaa,    /* SM66       Logical not             */
(unsigned char *)"-a",     P0B+0xa6,    /* SM21     ? Feminine sign           */
(unsigned char *)"-o",     P0B+0xa7,    /* SM20     ? Masculine sign          */
(unsigned char *)"..",     P0B+0xfa,    /* SM26       Middle dot (Product dot) (aka SD63) */
(unsigned char *)"12",     P0B+0xab,    /* NF01       One half                */
(unsigned char *)"14",     P0B+0xac,    /* NF04       One quarter             */
(unsigned char *)"34",     P0B+0xf3,    /* NF05       three quarters          */
(unsigned char *)":-",     P0B+0xf6,    /* SA06       Divide                  */
(unsigned char *)"?",      P0B+0xa8,    /* SP16       Spanish question mark   */
(unsigned char *)"A\"",    P0B+0x8e,    /* LA18/LA38  a umlaut capital        */
(unsigned char *)"A'",     P0B+0xb5,    /* LA12       a acute capital         */
(unsigned char *)"AE",     P0B+0x92,    /* LA52       ae diphthong capital    */
(unsigned char *)"A^",     P0B+0xb6,    /* LA16       a circumflex capital    */
(unsigned char *)"A`",     P0B+0xb7,    /* LA14       a grave capital         */
(unsigned char *)"Ao",     P0B+0x8f,    /* LA28       a overcircle capital    */
(unsigned char *)"A~",     P0B+0xc7,    /* LA20       a tilde capital         */
(unsigned char *)"B",      P0B+0xdb,    /* SF61       bright cell             */
(unsigned char *)"B2",     P0B+0xdc,    /* SF57       bright cell - lower half*/
(unsigned char *)"B8",     P0B+0xdf,    /* SF60       bright cell - upper half*/
(unsigned char *)"BO",     P0B+0xdd,    /* SM65       broken vertical bar     */
(unsigned char *)"C,",     P0B+0x80,    /* LC42       c cedilla capital       */
(unsigned char *)"D+",     P0B+0xd1,    /* LD62       eth icelandic capital   */
(unsigned char *)"D.",     P0B+0xcd,    /* SF43       double center box bar   */
(unsigned char *)"D0",     P0B+0xba,    /* SF24       double vertical bar     */
(unsigned char *)"D1",     P0B+0xc8,    /* SF38       double lower left corner box   */
(unsigned char *)"D2",     P0B+0xca,    /* SF40       double bottom side middle      */
(unsigned char *)"D3",     P0B+0xbc,    /* SF26       double lower right corner box  */
(unsigned char *)"D4",     P0B+0xcc,    /* SF42       double left side middle */
(unsigned char *)"D5",     P0B+0xce,    /* SF44       double intersection     */
(unsigned char *)"D6",     P0B+0xb9,    /* SF23       double right side middle*/
(unsigned char *)"D7",     P0B+0xc9,    /* SF39       double upper left corner box   */
(unsigned char *)"D8",     P0B+0xcb,    /* SF41       double top side middle  */
(unsigned char *)"D9",     P0B+0xbb,    /* SF25       double upper right corner box  */
(unsigned char *)"E\"",    P0B+0xd3,    /* LE18       e umlaut capital        */
(unsigned char *)"E'",     P0B+0x90,    /* LE12       e acute capital         */
(unsigned char *)"E^",     P0B+0xd2,    /* LE16       e circumflex capital    */
(unsigned char *)"E`",     P0B+0xd4,    /* LE14       e grave capital         */
(unsigned char *)"I\"",    P0B+0xd8,    /* LI18       i umlaut capital        */
(unsigned char *)"I'",     P0B+0xd6,    /* LI12       i acute capital         */
(unsigned char *)"IP",     P0B+0xe8,    /* LT64       thorn icelandic capital */
(unsigned char *)"I^",     P0B+0xd7,    /* LI16       i circumflex capital    */
(unsigned char *)"I`",     P0B+0xde,    /* LI14       i grave capital         */
(unsigned char *)"Ip",     P0B+0xe7,    /* LT63       thorn icelandic small   */
(unsigned char *)"L=",     P0B+0x9c,    /* SC02       English pound sign      */
(unsigned char *)"N~",     P0B+0xa5,    /* LN20       n tilde capital         */
(unsigned char *)"O\"",    P0B+0x99,    /* LO18/LO38  o umlaut capital        */
(unsigned char *)"O'",     P0B+0xe0,    /* LO12       o acute capital         */
(unsigned char *)"O/",     P0B+0x9d,    /* LO62       o slash capital         */
(unsigned char *)"O^",     P0B+0xe2,    /* LO16       o circumflex capital    */
(unsigned char *)"O`",     P0B+0xe3,    /* LO14       o grave capital         */
(unsigned char *)"O~",     P0B+0xe5,    /* LO20       o tilde capital         */
(unsigned char *)"S.",     P0B+0xc4,    /* SF10/SM12  Center box bar          */
(unsigned char *)"S0",     P0B+0xb3,    /* SF11/SM13  Vertical bar            */
(unsigned char *)"S1",     P0B+0xc0,    /* SF02       Lower left corner box   */
(unsigned char *)"S2",     P0B+0xc1,    /* SF07       Bottom side middle      */
(unsigned char *)"S3",     P0B+0xd9,    /* SF04       Lower right corner box  */
(unsigned char *)"S4",     P0B+0xc3,    /* SF08       Left side middle        */
(unsigned char *)"S5",     P0B+0xc5,    /* SF05       Intersection            */
(unsigned char *)"S6",     P0B+0xb4,    /* SF09       Right side middle       */
(unsigned char *)"S7",     P0B+0xda,    /* SF01       Upper left corner box   */
(unsigned char *)"S8",     P0B+0xc2,    /* SF06       Top side middle         */
(unsigned char *)"S9",     P0B+0xbf,    /* SF03       Upper right corner box  */
(unsigned char *)"U\"",    P0B+0x9a,    /* LU18/LU38  u umlaut capital        */
(unsigned char *)"U'",     P0B+0xe9,    /* LU12       u acute capital         */
(unsigned char *)"U^",     P0B+0xea,    /* LU16       u circumflex capital    */
(unsigned char *)"U`",     P0B+0xeb,    /* LU14       u grave capital         */
(unsigned char *)"Y'",     P0B+0xed,    /* LY12       y acute capital         */
(unsigned char *)"Y=",     P0B+0xbe,    /* SC05       Yen sign                */
(unsigned char *)"[]",     P0B+0xfe,    /* SV18       Vertical solid rectangle (aka SM47) */
(unsigned char *)"^-",     P0B+0xf0,    /* SP32       syllable hyphen (SP10)  */
(unsigned char *)"^1",     P0B+0xfb,    /* NS01       Superscript one         */
(unsigned char *)"^2",     P0B+0xfd,    /* NS02       Superscript two (aka ND021)    */
(unsigned char *)"^3",     P0B+0xfc,    /* ND031      superscript three       */
(unsigned char *)"_\"",    P0B+0xf9,    /* SD17       umlaut accent           */
(unsigned char *)"_'",     P0B+0xef,    /* SD11       acute accent            */
(unsigned char *)"_,",     P0B+0xf7,    /* SD41       cedilla accent          */
(unsigned char *)"__",     P0B+0xf2,    /* SM10       double underscore       */
(unsigned char *)"a\"",    P0B+0x84,    /* LA17/LA37  a umlaut small          */
(unsigned char *)"a'",     P0B+0xa0,    /* LA11       a acute small           */
(unsigned char *)"a^",     P0B+0x83,    /* LA15       a circumflex small      */
(unsigned char *)"a`",     P0B+0x85,    /* LA13       a grave small           */
(unsigned char *)"ae",     P0B+0x91,    /* LA51       ae diphthong small      */
(unsigned char *)"ao",     P0B+0x86,    /* LA27       a overcircle small      */
(unsigned char *)"a~",     P0B+0xc6,    /* LA19       a tilde small           */
(unsigned char *)"c,",     P0B+0x87,    /* LC41       c cedilla small         */
(unsigned char *)"c/",     P0B+0xbd,    /* SC04       Cent sign               */
(unsigned char *)"cO",     P0B+0xb8,    /* SM52       Copyright symbol        */
(unsigned char *)"d+",     P0B+0xd0,    /* LD63       eth icelandic small     */
(unsigned char *)"e\"",    P0B+0x89,    /* LE17/LE37  e umlaut small          */
(unsigned char *)"e'",     P0B+0x82,    /* LE11       e acute small           */
(unsigned char *)"e^",     P0B+0x88,    /* LE15       e circumflex small      */
(unsigned char *)"e`",     P0B+0x8a,    /* LE13       e grave small           */
(unsigned char *)"f",      P0B+0x9f,    /* SC07       Florin sign             */
(unsigned char *)"i",      P0B+0xd5,    /* LI61       small i dotless         */
(unsigned char *)"i\"",    P0B+0x8b,    /* LI17       i umlaut small          */
(unsigned char *)"i'",     P0B+0xa1,    /* LI11       i acute small           */
(unsigned char *)"i^",     P0B+0x8c,    /* LI15       i circumflex small      */
(unsigned char *)"i`",     P0B+0x8d,    /* LI13       i grave small           */
(unsigned char *)"n~",     P0B+0xa4,    /* LN19       n tilde small           */
(unsigned char *)"o",      P0B+0xf8,    /* SM19       Degree (Overcircle)     */
(unsigned char *)"o\"",    P0B+0x94,    /* LO17/LO37  o umlaut small          */
(unsigned char *)"o'",     P0B+0xa2,    /* LO11       o acute small           */
(unsigned char *)"o*",     P0B+0xcf,    /* SC01       international currency symbol  */
(unsigned char *)"o/",     P0B+0x9b,    /* LO61       o slash small           */
(unsigned char *)"o^",     P0B+0x93,    /* LO15       o circumflex small      */
(unsigned char *)"o`",     P0B+0x95,    /* LO13       o grave small           */
(unsigned char *)"o~",     P0B+0xe4,    /* LO19       o tilde small           */
(unsigned char *)"rO",     P0B+0xa9,    /* SM53       registered trademark symbol    */
(unsigned char *)"ss",     P0B+0xe1,    /* LS61       s sharp small           */
(unsigned char *)"u\"",    P0B+0x81,    /* LU17/LU37  u umlaut small          */
(unsigned char *)"u'",     P0B+0xa3,    /* LU11       u acute small           */
(unsigned char *)"u^",     P0B+0x96,    /* LU15       u circumflex small      */
(unsigned char *)"u`",     P0B+0x97,    /* LU13       u grave small           */
(unsigned char *)"x",      P0B+0x9e,    /* SA07       Multiply sign           */
(unsigned char *)"y\"",    P0B+0x98,    /* LY17/LY37  y umlaut small          */
(unsigned char *)"y'",     P0B+0xec,    /* LY11       y acute small           */
(unsigned char *)"{{",     P0B+0xae,    /* SP17       Left angle quotes       */
(unsigned char *)"|P",     P0B+0xf4,    /* SM25       Paragraph               */
(unsigned char *)"|S",     P0B+0xf5,    /* SM24       Section                 */
(unsigned char *)"}}",     P0B+0xaf,    /* SP18       Right angle quotes      */
};

unsigned _NLunesctsize = sizeof (_NLunesctab) / sizeof (struct NLescdata);

#endif

#ifdef KJI
struct NLescdata _NLunesctab[] = {

/* To work this table MUST BE SORTED by first field */
/* Beware! backslashes for C's sake make things sort out of order */

(unsigned char *)"j\"",    0xde,
(unsigned char *)"j\"\"",  0xdf,
(unsigned char *)"j'",     0xa3,
(unsigned char *)"j*",     0xa5,
(unsigned char *)"j,",     0xa4,
(unsigned char *)"j-",     0xb0,
(unsigned char *)"j.",     0xa1,
(unsigned char *)"jA",     0xb1,
(unsigned char *)"jCHI",   0xc1,
(unsigned char *)"jE",     0xb4,
(unsigned char *)"jHA",    0xca,
(unsigned char *)"jHE",    0xcd,
(unsigned char *)"jHI",    0xcb,
(unsigned char *)"jHO",    0xce,
(unsigned char *)"jHU",    0xcc,
(unsigned char *)"jI",     0xb2,
(unsigned char *)"jKA",    0xb6,
(unsigned char *)"jKE",    0xb9,
(unsigned char *)"jKI",    0xb7,
(unsigned char *)"jKO",    0xba,
(unsigned char *)"jKU",    0xb8,
(unsigned char *)"jMA",    0xcf,
(unsigned char *)"jME",    0xd2,
(unsigned char *)"jMI",    0xd0,
(unsigned char *)"jMO",    0xd3,
(unsigned char *)"jMU",    0xd1,
(unsigned char *)"jNA",    0xc5,
(unsigned char *)"jNE",    0xc8,
(unsigned char *)"jNI",    0xc6,
(unsigned char *)"jNN",    0xdd,
(unsigned char *)"jNO",    0xc9,
(unsigned char *)"jNU",    0xc7,
(unsigned char *)"jO",     0xb5,
(unsigned char *)"jRA",    0xd7,
(unsigned char *)"jRE",    0xda,
(unsigned char *)"jRI",    0xd8,
(unsigned char *)"jRO",    0xdb,
(unsigned char *)"jRU",    0xd9,
(unsigned char *)"jSA",    0xbb,
(unsigned char *)"jSE",    0xbe,
(unsigned char *)"jSI",    0xbc,
(unsigned char *)"jSO",    0xbf,
(unsigned char *)"jSU",    0xbd,
(unsigned char *)"jTA",    0xc0,
(unsigned char *)"jTE",    0xc3,
(unsigned char *)"jTO",    0xc4,
(unsigned char *)"jTSU",   0xc2,
(unsigned char *)"jU",     0xb3,
(unsigned char *)"jWA",    0xdc,
(unsigned char *)"jWO",    0xa6,
(unsigned char *)"jYA",    0xd4,
(unsigned char *)"jYO",    0xd6,
(unsigned char *)"jYU",    0xd5,
(unsigned char *)"j`",     0xa2,
(unsigned char *)"ja",     0xa7,
(unsigned char *)"je",     0xaa,
(unsigned char *)"ji",     0xa8,
(unsigned char *)"jo",     0xab,
(unsigned char *)"jtsu",   0xaf,
(unsigned char *)"ju",     0xa9,
(unsigned char *)"jya",    0xac,
(unsigned char *)"jyo",    0xae,
(unsigned char *)"jyu",    0xad,
};

unsigned _NLunesctsize = sizeof (_NLunesctab) / sizeof (struct NLescdata);
#endif

/*
 * NAME: _NLunescval
 *
 * FUNCTION: Accesses to the table that translates from information
 *	     preserving ASCII to NLS code point, i.e. _NLunesctab.
 *
 * RETURN VALUE DESCRIPTION: -1 if the mnemonic is unknown. If a
 *           corresponding NLchar is found, returns the value of  
 *	     NLchar string. 
 */
int
#ifdef _NO_PROTO
_NLunescval(src, slen, dest)
unsigned char *src;			/* the input string		*/
char slen; 				/* the length of a string	*/
NLchar *dest;				/* the destination string	*/
#else /* _NO_PROTO */
_NLunescval(unsigned char *src, char slen, NLchar *dest)
#endif /* _NO_PROTO */
{
	register int min, max, n, key;
#ifdef KJI
        register int tlen;
	register int mne = ((unsigned char)*src << 24) + 
		((1 < slen) ? ((unsigned char)*(src + 1) << 16)  + 
		((2 < slen) ? ((unsigned char)*(src + 2) << 8)  + 
		((3 < slen) ? (unsigned char)*(src + 3) : 0) : 0) : 0);
#else
	register int mne = ((unsigned char)*src << 8) +
		((1 < slen) ? (unsigned char)*(src + 1) : 0);
#endif
	int byte0;
	struct NLescdata *x;

	min = 0; max = _NLunesctsize;
	while(1) {
		n = min + ((max - min) >> 1);
		x = &_NLunesctab[n];
#ifdef KJI
                tlen = strlen(x->key);
		key = (x->key[0] << 24) + 
		((1 < tlen) ? (x->key[1] << 16)  + 
		((2 < tlen) ? (x->key[2] <<  8)  + 
		((3 < tlen) ? (x->key[3])  : 0) :0) :0); 
#else
		key = (x->key[0] << 8) + x->key[1];
#endif
		if (key == mne) {
			byte0 = (x->value >> 8) ? x->value >> 8 : x->value;
			_NCdec2(byte0, x->value, *dest);
			return (*dest);
		}
		if (key < mne) {
			if(n <= min)
				break;
			min = n;
		} else {
			if (max <= n)
				break;
			max = n;
		}
	};
	return(-1);
}
