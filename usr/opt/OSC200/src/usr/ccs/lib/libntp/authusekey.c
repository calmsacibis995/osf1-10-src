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
static char     *sccsid = "@(#)$RCSfile: authusekey.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/05/04 17:37:40 $";
#endif
/*
 */

/*
 * authusekey - decode a key from ascii and use it
 */
#include <stdio.h>
#include <sys/types.h>
#include <strings.h>
#include <ctype.h>

/*
 * Types of ascii representations for keys.  "Standard" means a 64 bit
 * hex number in NBS format, i.e. with the low order bit of each byte
 * a parity bit.  "NTP" means a 64 bit key in NTP format, with the
 * high order bit of each byte a parity bit.  "Ascii" means a 1-to-8
 * character string whose ascii representation is used as the key.
 */
#define	KEY_TYPE_STD	1
#define	KEY_TYPE_NTP	2
#define	KEY_TYPE_ASCII	3

#define	STD_PARITY_BITS	0x01010101

extern int auth_parity();
extern void auth_setkey();

int
authusekey(keyno, keytype, str)
	u_int keyno;
	int keytype;
	char *str;
{
	u_int key[2];
	u_char keybytes[8];
	char *cp;
	char *xdigit;
	int len;
	int i;
	static char *hex = "0123456789abcdef";

	cp = str;
	len = strlen(cp);
	if (len == 0)
		return 0;

	switch(keytype) {
	case KEY_TYPE_STD:
	case KEY_TYPE_NTP:
		if (len != 16)		/* Lazy.  Should define constant */
			return 0;
		/*
		 * Decode hex key.
		 */
		key[0] = 0;
		key[1] = 0;
		for (i = 0; i < 16; i++) {
			if (!isascii(*cp))
				return 0;
			xdigit = index(hex, isupper(*cp) ? tolower(*cp) : *cp);
			cp++;
			if (xdigit == 0)
				return 0;
			key[i>>3] <<= 4;
			key[i>>3] |= (u_int)(xdigit - hex) & 0xf;
		}

		/*
		 * If this is an NTP format key, put it into NBS format
		 */
		if (keytype == KEY_TYPE_NTP) {
			for (i = 0; i < 2; i++)
				key[i] = ((key[i] << 1) & ~STD_PARITY_BITS)
				    | ((key[i] >> 7) & STD_PARITY_BITS);
		}

		/*
		 * Check the parity, reject the key if the check fails
		 */
		if (!auth_parity(key)) {
			return 0;
		}
		
		/*
		 * We can't find a good reason not to use this key.
		 * So use it.
		 */
		auth_setkey(keyno, key);
		break;
	
	case KEY_TYPE_ASCII:
		/*
		 * Make up key from ascii representation
		 */
		bzero(keybytes, sizeof(keybytes));
		for (i = 0; i < 8 && i < len; i++)
			keybytes[i] = *cp++ << 1;
		key[0] = keybytes[0] << 24 | keybytes[1] << 16
		    | keybytes[2] << 8 | keybytes[3];
		key[1] = keybytes[4] << 24 | keybytes[5] << 16
		    | keybytes[6] << 8 | keybytes[7];
		
		/*
		 * Set parity on key
		 */
		(void)auth_parity(key);

		/*
		 * Now set key in.
		 */
		auth_setkey(keyno, key);
		break;
	
	default:
		/* Oh, well */
		return 0;
	}

	return 1;
}
