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
 * @(#)$RCSfile: auth_des.h,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/03/30 22:59:57 $
 */
/*	@(#)auth_des.h	1.3 90/07/17 4.1NFSSRC SMI	*/

/* 
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 * 1.3 88/02/08 SMI      
 */


/*
 * auth_des.h, Protocol for DES style authentication for RPC
 */

#ifndef _rpc_auth_des_h
#define	_rpc_auth_des_h

/*
 * There are two kinds of "names": fullnames and nicknames
 */
enum authdes_namekind {
	ADN_FULLNAME, 
	ADN_NICKNAME
};

/*
 * A fullname contains the network name of the client, 
 * a conversation key and the window
 */
struct authdes_fullname {
	char *name;		/* network name of client, up to MAXNETNAMELEN */
	des_block key;		/* conversation key */
	u_int window;		/* associated window */
};


/*
 * A credential 
 */
struct authdes_cred {
	enum authdes_namekind adc_namekind;
	struct authdes_fullname adc_fullname;
	u_int adc_nickname;
};



/*
 * A des authentication verifier 
 */
struct authdes_verf {
	union {
		struct timeval adv_ctime;	/* clear time */
		des_block adv_xtime;		/* crypt time */
	} adv_time_u;
	u_int adv_int_u;
};

/*
 * des authentication verifier: client variety
 *
 * adv_timestamp is the current time.
 * adv_winverf is the credential window + 1.
 * Both are encrypted using the conversation key.
 */
#define adv_timestamp	adv_time_u.adv_ctime
#define adv_xtimestamp	adv_time_u.adv_xtime
#define adv_winverf	adv_int_u

/*
 * des authentication verifier: server variety
 *
 * adv_timeverf is the client's timestamp + client's window
 * adv_nickname is the server's nickname for the client.
 * adv_timeverf is encrypted using the conversation key.
 */
#define adv_timeverf	adv_time_u.adv_ctime
#define adv_xtimeverf	adv_time_u.adv_xtime
#define adv_nickname	adv_int_u

#endif /*!_rpc_auth_des_h*/
