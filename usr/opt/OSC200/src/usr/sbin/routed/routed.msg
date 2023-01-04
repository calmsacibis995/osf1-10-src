$
$ (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
$ ALL RIGHTS RESERVED
$
$
$ OSF/1 Release 1.0
$
$ ORIGINS: 10  26  27
$
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1989
$ All Rights Reserved
$ Licensed Materials - Property of IBM
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ messages for routed
$

$set MS_ROUTED

$quote "

SNDERR          "sendto"
FAMERR          "\"from\" address in unsupported address family (%d), cmd %d\n"
NOPACKET 	"RIP version 0 packet received from %s! (cmd %d)"
UNKRTR          "trace command from unknown router, %s"
BOGUS 		"bogus input (from passive interface, %s)"
URTRPKT         "packet from unknown router, %s"
RTEERR          "route in unsupported address family (%d), from %s (af %d)\n"
RTEHOST         "bad host in route from %s (af %d)\n"
BADMETRIC 	"bad metric (%d) from %s\n"
SENDUPDATE	"send dynamic update\n"
DELAYUPDATE	"delay dynamic update\n"
SENDDELAY	"send delayed dynamic update\n"
INHIBUPDATE	"inhibit dynamic update for %d usec\n"
SELECTUDT	"select until dynamic update %d/%d sec/usec\n"
UNKSERV         "routed: router/udp: unknown service\n"
USAGE           "usage: routed [ -s ] [ -q ] [ -t ] [ -g ]\n"
FORMAT		"s %d, ibits %x index %d, mod %d, sh %x, or %x &ibits %x\n"
SETTIME		"setitimer: %m\n"
RECERR          "recvfrom"
SOCKET          "socket"
SOCKERR         "socket: %m"
SKTOPT          "setsockopt SO_BROADCAST: %m"
SKTOPT1         "setsockopt SO_REVBUF: %m"
REVBUF		"recv buf %d\n"
BINDERR         "bind"
BINDERR2        "bind: %m"
SOCKERR2        "socket: %m"
IOCTLGIF        "ioctl (get interface configuration)"
IOCTLGIFL       "%s: ioctl (get interface flags)"
IOCTLGADDR      "%s: ioctl (get dstaddr)"
IOCTLBDCST      "%s: ioctl (get broadaddr)"
IOCTLMET        "%s: ioctl (get metric)"
IOCTLNMSK       "%s: ioctl (get netmask)"
MEMERR          "routed: out of memory\n"
MEMERR2         "routed: ifinit: out of memory\n"
REIF            "re-installing interface %s"
ADDROUTE	"adding route to net/host %s through gateway %s: %m\n"
ADDRTE          "SIOCADDRT"
CHGRTE          "changing route from interface %s (timed out)"
DELRTE          "SIOCDELRT"
DELIF           "deleting route to interface %s (timed out)"
TRINI           "traceinit: can't init %s\n"
FNDELAYERR	"fcntl O_NDELAY: %m\n"
