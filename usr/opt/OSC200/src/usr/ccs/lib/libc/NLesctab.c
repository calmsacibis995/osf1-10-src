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
static char	*sccsid = "@(#)$RCSfile: NLesctab.c,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1993/06/08 01:17:40 $";
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
 * FUNCTIONS: _NLescval, NLesctab
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
 * sccsid[] = "NLesctab.c	1.12  com/lib/c/nls,3.1,9021 3/23/90 16:43:49";
 */

/*
 * NAME: NLesctab
 *
 * FUNCTION: N/A
 *
 * NOTE: Table to control translation from NLchar to mnemonic for
 *       information preserving ASCII esc sequence. Table is indexed
 *	 by NLchar value when #ifndef KJI.
 *	 _NLescsize is alse initialized.
 *       When #ifdef KJ, _NLescval().   
 * 	
 * RETURN VALUE DESCRIPTION: N/A
 */
/*
 *  Table to control translation from NLchar to mnemonic for information-
 *  preserving ASCII escape sequence.  Table is indexed by NLchar value.
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#ifndef KJI
unsigned char _NLesctab[][2] = {
{'C',','},    /* P0B+0x80,    LC42       c cedilla capital              */
{'u','"'},   /* P0B+0x81,    LU17/LU37  u umlaut small                 */
{'e','\''},    /* P0B+0x82,    LE11       e acute small                  */
{'a','^'},    /* P0B+0x83,    LA15       a circumflex small             */
{'a','"'},   /* P0B+0x84,    LA17/LA37  a umlaut small                 */
{'a','`'},    /* P0B+0x85,    LA13       a grave small                  */
{'a','o'},    /* P0B+0x86,    LA27       a overcircle small             */
{'c',','},    /* P0B+0x87,    LC41       c cedilla small                */
{'e','^'},    /* P0B+0x88,    LE15       e circumflex small             */
{'e','"'},   /* P0B+0x89,    LE17/LE37  e umlaut small                 */
{'e','`'},    /* P0B+0x8a,    LE13       e grave small                  */
{'i','"'},   /* P0B+0x8b,    LI17       i umlaut small                 */
{'i','^'},    /* P0B+0x8c,    LI15       i circumflex small             */
{'i','`'},    /* P0B+0x8d,    LI13       i grave small                  */
{'A','"'},   /* P0B+0x8e,    LA18/LA38  a umlaut capital               */
{'A','o'},    /* P0B+0x8f,    LA28       a overcircle capital           */
{'E','\''},    /* P0B+0x90,    LE12       e acute capital                */
{'a','e'},    /* P0B+0x91,    LA51       ae diphthong small             */
{'A','E'},    /* P0B+0x92,    LA52       ae diphthong capital           */
{'o','^'},    /* P0B+0x93,    LO15       o circumflex small             */
{'o','"'},   /* P0B+0x94,    LO17/LO37  o umlaut small                 */
{'o','`'},    /* P0B+0x95,    LO13       o grave small                  */
{'u','^'},    /* P0B+0x96,    LU15       u circumflex small             */
{'u','`'},    /* P0B+0x97,    LU13       u grave small                  */
{'y','"'},   /* P0B+0x98,    LY17/LY37  y umlaut small                 */
{'O','"'},   /* P0B+0x99,    LO18/LO38  o umlaut capital               */
{'U','"'},   /* P0B+0x9a,    LU18/LU38  u umlaut capital               */
{'o','/'},    /* P0B+0x9b,    LO61       o slash small                  */
{'L','='},    /* P0B+0x9c,    SC02       English pound sign             */
{'O','/'},    /* P0B+0x9d,    LO62       o slash capital                */
{'x','\0'},     /* P0B+0x9e,    SA07       Multiply sign                  */
{'f','\0'},     /* P0B+0x9f,    SC07       Florin sign                    */
{'a','\''},    /* P0B+0xa0,    LA11       a acute small                  */
{'i','\''},    /* P0B+0xa1,    LI11       i acute small                  */
{'o','\''},    /* P0B+0xa2,    LO11       o acute small                  */
{'u','\''},    /* P0B+0xa3,    LU11       u acute small                  */
{'n','~'},    /* P0B+0xa4,    LN19       n tilde small                  */
{'N','~'},    /* P0B+0xa5,    LN20       n tilde capital                */
{'-','a'},    /* P0B+0xa6,    SM21     ? Feminine sign                  */
{'-','o'},    /* P0B+0xa7,    SM20     ? Masculine sign                 */
{'?','\0'},     /* P0B+0xa8,    SP16       Spanish question mark          */
{'r','O'},    /* P0B+0xa9,    SM53       registered trademark symbol    */
{'-','.'},    /* P0B+0xaa,    SM66       Logical not                    */
{'1','2'},    /* P0B+0xab,    NF01       One half                       */
{'1','4'},    /* P0B+0xac,    NF04       One quarter                    */
{'!','\0'},     /* P0B+0xad,    SP03       Spanish exclamation sign       */
{'{','{'},    /* P0B+0xae,    SP17       Left angle quotes              */
{'}','}'},    /* P0B+0xaf,    SP18       Right angle quotes             */
{'#','1'},    /* P0B+0xb0,    SF14       Quarter hashed                 */
{'#','2'},    /* P0B+0xb1,    SF15       Half hashed                    */
{'#','3'},    /* P0B+0xb2,    SF16       Full hashed                    */
{'S','0'},    /* P0B+0xb3,    SF11/SM13  Vertical bar                   */
{'S','6'},    /* P0B+0xb4,    SF09       Right side middle              */
{'A','\''},    /* P0B+0xb5,    LA12       a acute capital                */
{'A','^'},    /* P0B+0xb6,    LA16       a circumflex capital           */
{'A','`'},    /* P0B+0xb7,    LA14       a grave capital                */
{'c','O'},    /* P0B+0xb8,    SM52       Copyright symbol               */
{'D','6'},    /* P0B+0xb9,    SF23       double right side middle       */
{'D','0'},    /* P0B+0xba,    SF24       double vertical bar            */
{'D','9'},    /* P0B+0xbb,    SF25       double upper right corner box  */
{'D','3'},    /* P0B+0xbc,    SF26       double lower right corner box  */
{'c','/'},    /* P0B+0xbd,    SC04       Cent sign                      */
{'Y','='},    /* P0B+0xbe,    SC05       Yen sign                       */
{'S','9'},    /* P0B+0xbf,    SF03       Upper right corner box         */
{'S','1'},    /* P0B+0xc0,    SF02       Lower left corner box          */
{'S','2'},    /* P0B+0xc1,    SF07       Bottom side middle             */
{'S','8'},    /* P0B+0xc2,    SF06       Top side middle                */
{'S','4'},    /* P0B+0xc3,    SF08       Left side middle               */
{'S','.'},    /* P0B+0xc4,    SF10/SM12  Center box bar                 */
{'S','5'},    /* P0B+0xc5,    SF05       Intersection                   */
{'a','~'},    /* P0B+0xc6,    LA19       a tilde small                  */
{'A','~'},    /* P0B+0xc7,    LA20       a tilde capital                */
{'D','1'},    /* P0B+0xc8,    SF38       double lower left corner box   */
{'D','7'},    /* P0B+0xc9,    SF39       double upper left corner box   */
{'D','2'},    /* P0B+0xca,    SF40       double bottom side middle      */
{'D','8'},    /* P0B+0xcb,    SF41       double top side middle         */
{'D','4'},    /* P0B+0xcc,    SF42       double left side middle        */
{'D','.'},    /* P0B+0xcd,    SF43       double center box bar          */
{'D','5'},    /* P0B+0xce,    SF44       double intersection            */
{'o','*'},    /* P0B+0xcf,    SC01       international currency symbol  */
{'d','+'},    /* P0B+0xd0,    LD63       eth icelandic small            */
{'D','+'},    /* P0B+0xd1,    LD62       eth icelandic capital          */
{'E','^'},    /* P0B+0xd2,    LE16       e circumflex capital           */
{'E','"'},   /* P0B+0xd3,    LE18       e umlaut capital               */
{'E','`'},    /* P0B+0xd4,    LE14       e grave capital                */
{'i','\0'},     /* P0B+0xd5,    LI61       small i dotless                */
{'I','\''},    /* P0B+0xd6,    LI12       i acute capital                */
{'I','^'},    /* P0B+0xd7,    LI16       i circumflex capital           */
{'I','"'},   /* P0B+0xd8,    LI18       i umlaut capital               */
{'S','3'},    /* P0B+0xd9,    SF04       Lower right corner box         */
{'S','7'},    /* P0B+0xda,    SF01       Upper left corner box          */
{'B','\0'},     /* P0B+0xdb,    SF61       bright cell                    */
{'B','2'},    /* P0B+0xdc,    SF57       bright cell - lower half       */
{'B','O'},    /* P0B+0xdd,    SM65       broken vertical bar            */
{'I','`'},    /* P0B+0xde,    LI14       i grave capital                */
{'B','8'},    /* P0B+0xdf,    SF60       bright cell - upper half       */
{'O','\''},    /* P0B+0xe0,    LO12       o acute capital                */
{'s','s'},    /* P0B+0xe1,    LS61       s sharp small                  */
{'O','^'},    /* P0B+0xe2,    LO16       o circumflex capital           */
{'O','`'},    /* P0B+0xe3,    LO14       o grave capital                */
{'o','~'},    /* P0B+0xe4,    LO19       o tilde small                  */
{'O','~'},    /* P0B+0xe5,    LO20       o tilde capital                */
{'&','m'},    /* P0B+0xe6,    GM01/SM17  Mu small (Micro)               */
{'I','p'},    /* P0B+0xe7,    LT63       thorn icelandic small          */
{'I','P'},    /* P0B+0xe8,    LT64       thorn icelandic capital        */
{'U','\''},    /* P0B+0xe9,    LU12       u acute capital                */
{'U','^'},    /* P0B+0xea,    LU16       u circumflex capital           */
{'U','`'},    /* P0B+0xeb,    LU14       u grave capital                */
{'y','\''},    /* P0B+0xec,    LY11       y acute small                  */
{'Y','\''},    /* P0B+0xed,    LY12       y acute capital                */
{'-','-'},    /* P0B+0xee,    SM15       overbar                        */
{'_','\''},    /* P0B+0xef,    SD11       acute accent                   */
{'^','-'},    /* P0B+0xf0,    SP32       syllable hyphen (SP10)         */
{'+','-'},    /* P0B+0xf1,    SA02       Plus or minus                  */
{'_','_'},    /* P0B+0xf2,    SM10       double underscore              */
{'3','4'},    /* P0B+0xf3,    NF05       three quarters                 */
{'|','P'},    /* P0B+0xf4,    SM25       Paragraph                      */
{'|','S'},    /* P0B+0xf5,    SM24       Section                        */
{':','-'},    /* P0B+0xf6,    SA06       Divide                         */
{'_',','},    /* P0B+0xf7,    SD41       cedilla accent                 */
{'o','\0'},     /* P0B+0xf8,    SM19       Degree (Overcircle)            */
{'_','"'},   /* P0B+0xf9,    SD17       umlaut accent                  */
{'.','.'},    /* P0B+0xfa,    SM26       Middle dot (Product dot) (aka SD63)            */
{'^','1'},    /* P0B+0xfb,    NS01       Superscript one                */
{'^','3'},    /* P0B+0xfc,    ND031      superscript three              */
{'^','2'},    /* P0B+0xfd,    NS02       Superscript two (aka ND021)    */
{'[',']'},    /* P0B+0xfe,    SV18       Vertical solid rectangle (aka SM47)            */
{'#','#'},    /* P0B+0xff,               All ones                       */
};

int _NLesctsize = sizeof (_NLesctab) / (sizeof (char) * 2);
#else
/*
 *  Values range from 0xa1 to 0xdf.  The "key" field contains the length of
 *  the escaped string.
 */
#include <NLctype.h>

struct NLescdata _NLesctab[] = {
(unsigned char *)"j.",  (NLchar)2,           /* 0xa1 */
(unsigned char *)"j`",  (NLchar)2,           /* 0xa2 */
(unsigned char *)"j'",  (NLchar)2,           /* 0xa3 */
(unsigned char *)"j,",  (NLchar)2,           /* 0xa4 */
(unsigned char *)"j*",  (NLchar)2,           /* 0xa5 */
(unsigned char *)"jWO", (NLchar)3,           /* 0xa6 */
(unsigned char *)"ja",  (NLchar)2,           /* 0xa7 */
(unsigned char *)"ji",  (NLchar)2,           /* 0xa8 */
(unsigned char *)"ju",  (NLchar)2,           /* 0xa9 */
(unsigned char *)"je",  (NLchar)2,           /* 0xaa */
(unsigned char *)"jo",  (NLchar)2,           /* 0xab */
(unsigned char *)"jya", (NLchar)3,           /* 0xac */
(unsigned char *)"jyu", (NLchar)3,           /* 0xad */
(unsigned char *)"jyo", (NLchar)3,           /* 0xae */
(unsigned char *)"jtsu",(NLchar)4,           /* 0xaf */
(unsigned char *)"j-",  (NLchar)2,           /* 0xb0 */
(unsigned char *)"jA",  (NLchar)2,           /* 0xb1 */
(unsigned char *)"jI",  (NLchar)2,           /* 0xb2 */
(unsigned char *)"jU",  (NLchar)2,           /* 0xb3 */
(unsigned char *)"jE",  (NLchar)2,           /* 0xb4 */
(unsigned char *)"jO",  (NLchar)2,           /* 0xb5 */
(unsigned char *)"jKA", (NLchar)3,           /* 0xb6 */
(unsigned char *)"jKI", (NLchar)3,           /* 0xb7 */
(unsigned char *)"jKU", (NLchar)3,           /* 0xb8 */
(unsigned char *)"jKE", (NLchar)3,           /* 0xb9 */
(unsigned char *)"jKO", (NLchar)3,           /* 0xba */
(unsigned char *)"jSA", (NLchar)3,           /* 0xbb */
(unsigned char *)"jSI", (NLchar)3,           /* 0xbc */
(unsigned char *)"jSU", (NLchar)3,           /* 0xbd */
(unsigned char *)"jSE", (NLchar)3,           /* 0xbe */
(unsigned char *)"jSO", (NLchar)3,           /* 0xbf */
(unsigned char *)"jTA", (NLchar)3,           /* 0xc0 */
(unsigned char *)"jCHI",(NLchar)4,           /* 0xc1 */
(unsigned char *)"jTSU",(NLchar)4,           /* 0xc2 */
(unsigned char *)"jTE", (NLchar)3,           /* 0xc3 */
(unsigned char *)"jTO", (NLchar)3,           /* 0xc4 */
(unsigned char *)"jNA", (NLchar)3,           /* 0xc5 */
(unsigned char *)"jNI", (NLchar)3,           /* 0xc6 */
(unsigned char *)"jNU", (NLchar)3,           /* 0xc7 */
(unsigned char *)"jNE", (NLchar)3,           /* 0xc8 */
(unsigned char *)"jNO", (NLchar)3,           /* 0xc9 */
(unsigned char *)"jHA", (NLchar)3,           /* 0xca */
(unsigned char *)"jHI", (NLchar)3,           /* 0xcb */
(unsigned char *)"jHU", (NLchar)3,           /* 0xcc */
(unsigned char *)"jHE", (NLchar)3,           /* 0xcd */
(unsigned char *)"jHO", (NLchar)3,           /* 0xce */
(unsigned char *)"jMA", (NLchar)3,           /* 0xcf */
(unsigned char *)"jMI", (NLchar)3,           /* 0xd0 */
(unsigned char *)"jMU", (NLchar)3,           /* 0xd1 */
(unsigned char *)"jME", (NLchar)3,           /* 0xd2 */
(unsigned char *)"jMO", (NLchar)3,           /* 0xd3 */
(unsigned char *)"jYA", (NLchar)3,           /* 0xd4 */
(unsigned char *)"jYU", (NLchar)3,           /* 0xd5 */
(unsigned char *)"jYO", (NLchar)3,           /* 0xd6 */
(unsigned char *)"jRA", (NLchar)3,           /* 0xd7 */
(unsigned char *)"jRI", (NLchar)3,           /* 0xd8 */
(unsigned char *)"jRU", (NLchar)3,           /* 0xd9 */
(unsigned char *)"jRE", (NLchar)3,           /* 0xda */
(unsigned char *)"jRO", (NLchar)3,           /* 0xdb */
(unsigned char *)"jWA", (NLchar)3,           /* 0xdc */
(unsigned char *)"jNN", (NLchar)3,           /* 0xdd */
(unsigned char *)"j\"", (NLchar)2,           /* 0xde */
(unsigned char *)"j\"\"", (NLchar)3,         /* 0xdf */
};
int _NLesctsize = sizeof (_NLesctab) / sizeof (struct NLescdata);

/*
 * NAME: _NLescval
 *
 * FUNCTION: Place the esc sequence representing src in dest.
 *
 * NOTE: When #ifdef KJI, _NLescval. When #else, NLesctab.
 * 
 * RETURN VALUE DESCRIPTION: The number of chars in the esc sequence.
 */
/*
 *  Place the "escaped" sequence representing src in dest.  Return the number
 *  of characters in the escaped sequence.
 */

int
_NLescval(register NLchar *src, register char *dest)
{
	register int i;
	register int length;
	register int index;

	dest[0] = '\\';
	dest[1] = '<'; 
	index = *src - MINESCVAL;
	length = _NLesctab[index].value;
	for (i = 0; i <= length; i++)
		dest[i + 2] = _NLesctab[index].key[i];
	dest[length + 2] = '>';
	return (length + 3);
}

#endif
