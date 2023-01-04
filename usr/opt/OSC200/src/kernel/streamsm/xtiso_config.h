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
 *	@(#)$RCSfile: xtiso_config.h,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/05/12 15:17:58 $
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

#ifndef _XTISO_CONFIG_H
#define _XTISO_CONFIG_H
/*
 * Configuration structures for XTI-over-SOcket driver.
 */

/*
 * Protocol Information Structure
 */
struct xtiproto { 
	int	 xp_dom;		/* socket domain		*/
	int	 xp_type;		/* socket type  		*/
	int	 xp_proto;		/* socket protocol		*/
	int	 xp_servtype;		/* service type supported	*/
	long	 xp_tsdulen;		/* max TSDU size		*/
	long	 xp_etsdulen;		/* max ETSDU size		*/
	long	 xp_disconlen;		/* max disconnection data size	*/
	long	 xp_connectlen;		/* max connection data size 	*/
	long	 xp_addrlen;		/* max protocol addr size	*/
	long	 xp_optlen;		/* max protocol option size 	*/
	long	 xp_tidulen;		/* max trans protocol i/f size  */
        struct xtiso_options	 
		 *xp_options;		/* Options block so{get,set}opt supp */
};

/*
 * Following options mapping block is required because transport level
 * options may have different option names (& numbers) supporting the
 * functionality needed by XTI, ie. Names/numbers used by OSI transport
 * level options could be completely different from other hypothetical
 * transport implementation
 *
 */
struct xtiso_options {
	int	 xo_level;	/* protocol level for so{get,set}opt         */
        int	 xo_getinfo;	/* name for t_getinfo supp                   */
        int	 xo_mgmtneg;	/* name for t_optmgmt/T_NEGOTIATE            */
        int	 xo_mgmtchk;	/* name for t_optmgmt/T_CHECK                */
        int	 xo_mgmtdef;	/* name for t_optmgmt/T_DEFAULT              */
        int	 xo_condata;	/* name for passing connect data             */
        int	 xo_conopts;	/* name for setting connect options          */
        int	 xo_discondata;	/* name for getting disconnect data          */
        int	 xo_disconreas;	/* name for getting disconnect reason        */
        int	 xo_accept;	/* name for accepting deferred incoming conn */
};

#define xp_opt_level		xp_options->xo_level
#define xp_getinfo		xp_options->xo_getinfo
#define xp_mgmtneg		xp_options->xo_mgmtneg
#define xp_mgmtchk		xp_options->xo_mgmtchk
#define xp_mgmtdef		xp_options->xo_mgmtdef
#define xp_condata		xp_options->xo_condata
#define xp_conopts		xp_options->xo_conopts
#define xp_discondata		xp_options->xo_discondata
#define xp_disconreas		xp_options->xo_disconreas
#define xp_accept		xp_options->xo_accept

#define OSF_XTISO_CONFIG_10	0x01091790

/*
 * Input data structure from configuration manager via kmodcall()
 */
typedef struct {
	str_config_t		sc;		/* standard stream config */
	struct xtiproto         proto;		/* xtiso protocol info    */
} xtiso_inadm_t;

/*
 * Output data structure from configuration manager via kmodcall()
 */
typedef struct {
	str_config_t		sc;
} xtiso_outadm_t;

#endif	/* _XTISO_CONFIG_H */
