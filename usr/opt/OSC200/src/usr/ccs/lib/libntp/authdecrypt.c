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
static char     *sccsid = "@(#)$RCSfile: authdecrypt.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/05/04 17:37:01 $";
#endif
/*
 */

/*
 * authdecrypt - routine to decrypt a packet to see if this guy knows our key.
 */
#include <sys/types.h>

/*
 * For our purposes an NTP packet looks like:
 *
 *	a variable amount of unencrypted data, multiple of 8 bytes, followed by:
 *	NOCRYPT_OCTETS worth of unencrypted data, followed by:
 *	BLOCK_OCTETS worth of ciphered checksum.
 */ 
#define	NOCRYPT_OCTETS	4
#define	BLOCK_OCTETS	8

#define	NOCRYPT_LONGS	((NOCRYPT_OCTETS)/sizeof(u_int))
#define	BLOCK_LONGS	((BLOCK_OCTETS)/sizeof(u_int))

/*
 * Imported from the key data base module
 */
extern u_int cache_keyid;	/* cached key ID */
extern u_char cache_dkeys[];	/* cached decryption keys */
extern u_char zerodkeys[];	/* zero key decryption keys */

extern int authhavekey();

/*
 * Stat counters, imported from data base module
 */
extern u_int authdecryptions;
extern u_int authkeyuncached;
extern u_int authdecryptok;

int
authdecrypt(keyno, pkt, length)
	u_int keyno;
	u_int *pkt;
	int length;	/* length of variable data in octets */
{
	register u_int *pd;
	register int i;
	register u_char *keys;
	register int longlen;
	u_int work[2];

	authdecryptions++;
	
	if (keyno == 0)
		keys = zerodkeys;
	else {
		if (keyno != cache_keyid) {
			authkeyuncached++;
			if (!authhavekey(keyno))
				return 0;
		}
		keys = cache_dkeys;
	}

	/*
	 * Get encryption block data in host byte order and decrypt it.
	 */
	longlen = length / sizeof(u_int);
	pd = pkt + longlen;		/* points at NOCRYPT area */
	work[0] = *(pd + NOCRYPT_LONGS);
	work[1] = *(pd + NOCRYPT_LONGS + 1);

	if (longlen & 0x1) {
		auth_des(work, keys);
		work[0] ^= *(--pd);
	}

	for (i = longlen/2; i > 0; i--) {
		auth_des(work, keys);
		work[1] ^= *(--pd);
		work[0] ^= *(--pd);
	}

	/*
	 * Success if the encryption data is zero
	 */
	if ((work[0] == 0) && (work[1] == 0)) {
		authdecryptok++;
		return 1;
	}
	return 0;
}
