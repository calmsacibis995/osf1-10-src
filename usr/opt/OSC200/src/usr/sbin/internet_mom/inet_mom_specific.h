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
 * @(#)$RCSfile: inet_mom_specific.h,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/13 19:41:49 $
 */
/*
**  inet_mom_specific.h
*/

#include <sys/fcntl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/stream.h>
#include <sys/protosw.h>
#include <sys/strkinfo.h>
#include <stropts.h>
#include <stdio.h>
#include <sys/time.h>
#include <setjmp.h>


/*
 *  Information for the attributes syscontact and syslocation.  These are read
 *  out of the Internet configuration file.  The default Link Polling Timer
 *  value is "read-only" for the Common Agent (to change this value, you
 *  must manually edit the file; you can't do it via the Common Agent).
 */

#define SYSTEM_LOCATION         1    
#define SYSTEM_CONTACT          2 
#define LINK_POLLING_INTERVAL   3

#if defined(__osf__)
# define CONFIG_FILE "/etc/eca/internet_mom.conf"
#else
# if defined(ultrix) || defined(__ultrix)
#  define CONFIG_FILE "/usr/var/kits/ecalocal/internet_mom.conf"
# else
#  define CONFIG_FILE "/etc/eca/internet_mom.conf"
# endif
#endif



/* Miscellaneous definitions */

#define MOM_C_ROUTER_PORT       520
#define MOM_C_VAN_JACOBSONS     4       /* As defined in MIB-II */
#define MOM_C_TCP_MAXCONN_VALUE -1      /* As defined in MIB-II */

#define MOM_C_ROUTE_INVALID     2
#define MOM_C_MAX_BUFLEN        1024


/*  Type of packets that can be sent from mom to gated */

#define MOM_C_AGENT_REG         0x01  /* variable registration          */
#define MOM_C_AGENT_REQ         0x02  /* request for a variable value   */
#define MOM_C_AGENT_ERR         0x03  /* error                          */
#define MOM_C_AGENT_RSP         0x04  /* a response to a request        */
#define MOM_C_AGENT_REQN        0X05  /* request for next variable value*/


/*
 * To indicate the status of an interface.
 */

#define MOM_C_IF_UP             1
#define MOM_C_IF_DOWN           2
#define MOM_C_IF_TESTING        3

#define MOM_C_IP_NETENTRY_OTHER         1
#define MOM_C_IP_NETENTRY_INVALID       2
#define MOM_C_IP_NETENTRY_DYNAMIC       3
#define MOM_C_IP_NETENTRY_STATIC        4

#define MOM_C_LESS_THAN         -1
#define MOM_C_EQUAL_TO           0
#define MOM_C_GREATER_THAN       1

#define COMPARE( x, y ) \
    ( ((x) > (y)) ? MOM_C_GREATER_THAN               \
                  : ((x) < (y)) ? MOM_C_LESS_THAN    \
                                : MOM_C_EQUAL_TO )

#define CLOSED        1
#define LISTEN        2
#define SYN_SENT      3
#define SYN_RECEIVED  4
#define ESTABLISHED   5
#define FIN_WAIT_1    6
#define FIN_WAIT_2    7
#define CLOSE_WAIT    8
#define LAST_ACK      9
#define CLOSING       10
#define TIME_WAIT     11


static struct sockaddr_in  local_addr;

struct sockaddr_in  to_router;
int rtsock;

/*
 * Global - needed for all the kernel reads
 */

int inetfd ;

/*  Size of the atEntry table index in octets, atIfIndex (1) + atDummy (1) + atNetAddress (4) */
#define atEntry_INDEX_SIZE 6

