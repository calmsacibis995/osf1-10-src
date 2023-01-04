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
static char     *sccsid = "@(#)$RCSfile: authencrypt.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/05/04 17:37:15 $";
#endif
/*
 */

/*
 * authencrypt - compute and encrypt the mac field in an NTP packet
 */
#include <sys/types.h>

/*
 * For our purposes an NTP packet looks like:
 *
 *	a variable amount of encrypted data, multiple of 8 bytes, followed by:
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
extern u_char cache_ekeys[];	/* cached decryption keys */
extern u_char zeroekeys[];	/* zero key decryption keys */

extern int authhavekey();

/*
 * Stat counters from the database module
 */
extern u_int authencryptions;
extern u_int authkeyuncached;
extern u_int authnokey;

void
authencrypt(keyno, pkt, length)
	u_int keyno;
	u_int *pkt;
	int length;	/* length of encrypted portion of packet */
{
	register u_int *pd;
	register int i;
	register u_char *keys;
	register int len;
	u_int work[2];
	void auth_des();

	authencryptions++;

	if (keyno == 0) {
		keys = zeroekeys;
	} else {
		if (keyno != cache_keyid) {
			authkeyuncached++;
			if (!authhavekey(keyno)) {
				authnokey++;
				return;
			}
		}
		keys = cache_ekeys;
	}

	/*
	 * Do the encryption.  Work our way forward in the packet, eight
	 * bytes at a time, encrypting as we go.  Note that the byte order
	 * issues are handled by the DES routine itself
	 */
	pd = pkt;
	work[0] = work[1] = 0;
	len = length / sizeof(u_int);

	for (i = (len/2); i > 0; i--) {
		work[0] ^= *pd++;
		work[1] ^= *pd++;
		auth_des(work, keys);
	}

	if (len & 0x1) {
		work[0] ^= *pd++;
		auth_des(work, keys);
	}

	/*
	 * Space past the keyid and stick the result back in the mac field
	 */
	pd += NOCRYPT_LONGS;
	*pd++ = work[0];
	*pd = work[1];
}
