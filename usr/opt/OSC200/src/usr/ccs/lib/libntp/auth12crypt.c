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
static char     *sccsid = "@(#)$RCSfile: auth12crypt.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/05/04 17:36:54 $";
#endif
/*
 */

/*
 * auth12crypt.c - routines to support two stage NTP encryption
 */
#include <sys/types.h>

/*
 * For our purposes an NTP packet looks like:
 *
 *	a variable amount of encrypted data, multiple of 8 bytes, which
 *		is encrypted in pass 1, followed by:
 *	an 8 byte chunk of data which is encrypted in pass 2
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
 * Stat counters, from the database module
 */
extern u_int authencryptions;
extern u_int authkeyuncached;
extern u_int authnokey;


/*
 * auth1crypt - do the first stage of a two stage encryption
 */
void
auth1crypt(keyno, pkt, length)
	u_int keyno;
	u_int *pkt;
	int length;	/* length of all encrypted data */
{
	register u_int *pd;
	register int i;
	register u_char *keys;
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
	 * Do the first five encryptions.  Stick the intermediate result
	 * in the mac field.  The sixth encryption must wait until the
	 * caller freezes a transmit time stamp, and will be done in stage 2.
	 */
	pd = pkt;
	work[0] = work[1] = 0;

	for (i = (length/BLOCK_OCTETS - 1); i > 0; i--) {
		work[0] ^= *pd++;
		work[1] ^= *pd++;
		auth_des(work, keys);
	}

	/*
	 * Space to the end of the packet and stick the intermediate
	 * result in the mac field.
	 */
	pd += BLOCK_LONGS + NOCRYPT_LONGS;
	*pd++ = work[0];
	*pd = work[1];
}


/*
 * auth2crypt - do the second stage of a two stage encryption
 */
void
auth2crypt(keyno, pkt, length)
	u_int keyno;
	u_int *pkt;
	int length;	/* total length of encrypted area */
{
	register u_int *pd;
	register u_char *keys;
	void auth_des();

	/*
	 * Skip the key check.  The call to the first stage should
	 * have got it.
	 */
	if (keyno == 0)
		keys = zeroekeys;
	else
		keys = cache_ekeys;

	/*
	 * The mac currently should hold the results of the first `n'
	 * encryptions.  We xor in the last block in data section and
	 * do the final encryption in place.
	 *
	 * Get a pointer to the MAC block.  XOR in the last two words of
	 * the data area. Call the encryption routine.
	 */
	pd = pkt + (length/sizeof(u_int)) + NOCRYPT_LONGS;

	*pd ^= *(pd - NOCRYPT_LONGS - 2);
	*(pd + 1) ^= *(pd - NOCRYPT_LONGS - 1);
	auth_des(pd, keys);
}
