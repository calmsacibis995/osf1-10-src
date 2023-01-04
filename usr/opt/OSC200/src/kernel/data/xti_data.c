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
static char *rcsid = "@(#)$RCSfile: xti_data.c,v $ $Revision: 1.1.6.5 $ (DEC) $Date: 1993/07/07 19:37:34 $";
#endif


/* STREAMS include files */
#include <sys/stream.h>
#include <sys/stropts.h>
#include <sys/strstat.h>
#include <sys/strlog.h>

#include <net/net_globals.h>
#include <sys/socket.h>
#include <sys/socketvar.h>

#include <streamsm/xti.h>
#include <streamsm/xtiso.h>
#include <streamsm/xtiso_config.h>

#include <sys/socket.h>
#include <sys/socketvar.h>

#include <sys/un.h>
#include <netinet/in.h>
#include <netns/ns.h>

#ifdef OSI
#include <dec/netosi/osi.h>
extern struct xtiso_options osi_options;
#endif /* ifdef OSI */


xtiso_inadm_t xti_proto_info[] = {
/* UDP */
	{ 
	  { OSF_STREAMS_CONFIG_11, 0, "xtisoUDP", NODEV },
	  { AF_INET, SOCK_DGRAM, 0, T_CLTS,
	    	/* The ti[ts]dulen should be the same as udp_sendspace (9K) */
		9216, XTI_NS, XTI_NS, XTI_NS,
		sizeof(struct sockaddr_in), XTI_NS, 9216, 
		XTI_NO_OPTS
	  }
	}
/* TCP */
	,
	{ 
	  { OSF_STREAMS_CONFIG_11, 0, "xtisoTCP", NODEV },
	  { AF_INET, SOCK_STREAM, 0, T_COTS_ORD,
	   	/* The tsdulen should be the same as max stream msgsz (12K) */
		0, 1024, XTI_NS, XTI_NS,
		sizeof(struct sockaddr_in), XTI_NS, 12288, 
		XTI_NO_OPTS
	  } 
	}
/* Unix DGRAM */
	,
	{ 
	  { OSF_STREAMS_CONFIG_11, 0, "xtisoUDG", NODEV },
	  { AF_UNIX, SOCK_DGRAM, 0, T_CLTS,
		8192, XTI_NS, XTI_NS, XTI_NS,
		sizeof(struct sockaddr_un), XTI_NS, 4096, 
		XTI_NO_OPTS
	  }
	}
/* Unix STREAM */
	,
	{ 
	  { OSF_STREAMS_CONFIG_11, 0, "xtisoUST", NODEV },
	  { AF_UNIX, SOCK_STREAM, 0, T_COTS_ORD,
		0, 1024, XTI_NS, XTI_NS,
		sizeof(struct sockaddr_un), 0, 4096, 
		XTI_NO_OPTS
	  }
	}
/* IDP */
	,
	{ 
	  { OSF_STREAMS_CONFIG_11, 0, "xtisoIDP", NODEV },
	  { AF_NS, SOCK_DGRAM, 0, T_CLTS,
		8192, XTI_NS, XTI_NS, XTI_NS,
		sizeof(struct sockaddr_ns), XTI_NS, 4096, 
		XTI_NO_OPTS
	  }
	}
/* SPP */
	,
	{ 
	  { OSF_STREAMS_CONFIG_11, 0, "xtisoSPP", NODEV },
	  { AF_NS, SOCK_STREAM, 0, T_COTS_ORD,
		0, 1024, XTI_NS, XTI_NS,
		sizeof(struct sockaddr_ns), 0, 4096, 
		XTI_NO_OPTS
	  }
	}
#ifdef OSI
/* OSI Transport - Connection-Oriented Transport Service */
	,
	{ 
          { OSF_STREAMS_CONFIG_11, 0, "xtisoCOTS", NODEV },
          { AF_OSI, SOCK_RAW, 0, T_COTS,
		-1, 16, 64, 32, 
	    	/* sizeof (struct sockaddr_osi)+10+OSI_TLEN+OSI_NLEN = 106 */
		106, sizeof(struct isoco_options), 12288, 
		&osi_options
	  }
	}
/* OSI Transport - ConnectionLess Transport Service */
	,
	{ 
          { OSF_STREAMS_CONFIG_11, 0, "xtisoCLTS", NODEV },
          { AF_OSI, SOCK_DGRAM, 0, T_CLTS,
		9216, XTI_NS, XTI_NS, XTI_NS, 
		106, XTI_NS, 9216, 
		XTI_NO_OPTS
	  }
	}
#endif
/* End Marker for xti_proto_info                   [Don't move or modify] */
};

/* The following is temporary, can be removed */
#define XTI_NOSTATICS	(sizeof xti_proto_info / sizeof xti_proto_info[0])

int xti_nprotos = XTI_NOSTATICS;

#ifdef OSI
struct xtiso_options	osi_options = {
			    OSIPROTO_COTS,	/* xo_level		*/
			    TOPT_XTIINFO, 	/* xo_getinfo		*/
			    TOPT_XTINEGQOS, 	/* xo_mgmtneg		*/
			    TOPT_XTICHKQOS, 	/* xo_mgmtchk		*/
			    TOPT_XTIDFLTQOS,	/* xo_mgmtdef		*/
			    TOPT_OPTCONDATA,	/* xo_condata		*/
			    TOPT_XTICONOPTS,	/* xo_conopts		*/
			    TOPT_OPTDISDATA,	/* xo_discondata	*/
			    TOPT_DISREASON,	/* xo_disconreas	*/
			    TOPT_ACCEPT		/* xo_accept		*/
			};
#endif	/* ifdef OSI */
