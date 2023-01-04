 
$ *****************************************************************
$ *                                                               *
$ *    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
$ *                                                               *
$ *   All Rights Reserved.  Unpublished rights  reserved  under   *
$ *   the copyright laws of the United States.                    *
$ *                                                               *
$ *   The software contained on this media  is  proprietary  to   *
$ *   and  embodies  the  confidential  technology  of  Digital   *
$ *   Equipment Corporation.  Possession, use,  duplication  or   *
$ *   dissemination of the software and media is authorized only  *
$ *   pursuant to a valid written license from Digital Equipment  *
$ *   Corporation.                                                *
$ *                                                               *
$ *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
$ *   by the U.S. Government is subject to restrictions  as  set  *
$ *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
$ *   or  in  FAR 52.227-19, as applicable.                       *
$ *                                                               *
$ *****************************************************************
$
$ HISTORY
 
$
$ (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
$ ALL RIGHTS RESERVED
$
$
$ OSF/1 Release 1.0

$
$ @(#)$RCSfile: gated.msg,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1994/01/04 16:19:29 $
$ 
$ COMPONENT_NAME: TCPIP gated.msg
$ 
$ FUNCTIONS: N/A
$
$ ORIGINS: 10  26  27  39  36 
$
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1989
$ All Rights Reserved
$ Licensed Materials - Property of IBM
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$
$ messages for gated
$
$	Note:	Messages whose message ids start with <name> are issued   
$               by <name>.c. The collection of all messages issued by <name.c>  
$               is grouped under the set MS_<name>. Messages containing only
$		printf format characters, or whose only text consists of words
$		such as "rip", "hello", "egp", etc. are not included in the
$		catalog.
$

$quote "

$set MS_DEFS
DEFS_1 "Unknown error number"
$ DEFS_1  An error code was returned which was outside the range
$ DEFS_1  provided for in sys_errlist[] (larger than sys_nerr).  So this
$ DEFS_1  message will be passed to the p_error function instead of the
$ DEFS_1  message which would have been provided from sys_errlist had
$ DEFS_1  the error code been in the proper range.


$set MS_AF
AF_1 "inet_output: sendto() error sending %d bytes to %s"
$ AF_1   The inet_output routine received an error return from the
$ AF_1   sendto system call.  %1 is the number of bytes and %2 is
$ AF_1   the target address.  This message is sent to the p_error
$ AF_1   routine.  WARNING this message may not exceed 150 characters
$ AF_1   (bytes) exclusive of %1 and %2 but including blanks and
$ AF_1   punctuation.


$set MS_EGP
EGP_1 "egpjob: EGP neighbor %s state change DOWN -> UP, %d/%d responses received"
$ EGP1   The state of the EGP neighbor whose address is %1 has changed
$ EGP1   from up to down based on getting %2 responses for %3 commands
$ EGP1   sent.

EGP_2 "egpjob: EGP neighbor %s state change DOWN -> UP, %d/%d responses received at %s"
$ EGP2   The state of the EGP neighbor whose address is %1 has changed
$ EGP2   from down to up based on getting %2 responses for %3 commands
$ EGP2   sent.

EGP_3 "egpjob: EGP neighbor %s state change UP -> DOWN, %d/%d responses received"
EGP_4 "egpjob: EGP neighbor %s state change UP -> DOWN, %d/%d responses received at %s"

EGP_5 "egpjob: no valid NR update"
EGP_6 " for %d successive new poll"
EGP_7 " id's\n"
$ EGP_5  This message indiacates that no updates have been received
$ EGP_5  after %d requests.

EGP_8 "egpjob: recv too many commands (%d) in "
EGP_9 "%d sec from %s\n"
$ EGP_8   We have received %1 commands in %2 seconds from %3 (internet
$ EGP_8   address) which is too many so we're going to mark 'em bad.


EGP_10 "egpin: recv fragmented pkt from %s\n"
$ EGP_10     A fragmented packet has been received from %s (an internet
$ EGP_10     address).  egpin returns.

EGP_11 "egpin: gw %s no interface?\n"
$ EGP_11     The local table has no interface entry for the address
$ EGP_11     (%s).  egpin returns.

EGP_12 "egpin: gw %s no interface?"
$ EGP_12     Same as above.

EGP_13 "egpin: bad EGP checksum from %s at %s"

EGP_14 "egpin: bad EGP checksum from %s"
$ EGP_14   A packet from the internet address %s produced a bad checksum.
$ EGP_14   egpin returns.

EGP_15 "egpin: bad pkt length %d from %s"
$ EGP_15    The egp packet from %2 (an internet address) is longer
$ EGP_15    than the packet that contains it.  %1 is the ip length.
$ EGP_15    egpin returns.

EGP_16 "egpin: bad pkt length %d from %s at %s"
EGP_17 "invalid destination address"
EGP_18 "unsupported Version"

EGP_19 "egpin: error packet (reason %s) from EGP neighbor %s"
$ EGP_19     An egp error packet has been received with a reason number
$ EGP_19     of %1 from a neighbor whose address is %2.

EGP_20 "egpin: error packet (reason %) from EGP neighbor %s at %s"

EGP_21 "egpin: EGP neighbor %s thinks we are down"
$ EGP_21     An egp error packet has been received from a neighbor whose
$ EGP_21     address is %s indicating that they think we are down.

EGP_22 "egpin: EGP neighbor %s thinks we are down at %s"

EGP_23 "invalid Type field"
EGP_24 "unsupported Version"

EGP_25 "egpin: recv above error packet\n"
$ EGP_25     This message is issued if tracing is on and the e and p
$ EGP_25     command command line flags are set.  It indicates that
$ EGP_25     egpin has received an error packet.  Otherwise the following
$ EGP_25     message is sent.

EGP_26 "egpin: recv error pkt: "
$ EGP_26    As above but this message is sent to the traceegp function
$ EGP_26    as the comment parameter.

EGP_27 "egpin: ACQUIRE packet from illegit neighbor %s\n"
$ EGP_27  The neighbor attempting to get routing information is not
$ EGP_27  on our list.

EGP_28 "egpin: ACQUIRE packet from illegit neighbor %s at %s"
$ EGP_28  The neighbor attempting to get routing information is not
$ EGP_28  on our list.

EGP_29 "egpacq: neighbor %s specified AS %d, we expected %d at %s"
$ EGP_29     We received a neighbor acquisition packet from %1 (inet
$ EGP_29     address) with an Autonomous System number of %2 when our
$ EGP_29     tables indicate that %1 should have an AS of %3.

EGP_30 "egpacq: neighbor %s specified AS %d, we expected %d"

EGP_33 "egpacq: Neighbor acquisition refuse from EGP neighbor %s"
$ EGP_33     The EGP neighbor %s has refused our acquire message.S

EGP_34 "egpacq: Neighbor acquisition refuse from EGP neighbor %s at %s"

EGP_35 "egpacq cease from EGP neighbor %s, status %s"
$ EGP_35     The EGP neighbor %s (inet address) has requested to terminate
$ EGP_35     our neighbor status.

EGP_36 "egpacq: cease from EGP neighbor %s, status %s at %s"

EGP_37 "egpacq cease-ack from EGP neighbor %s"
EGP_38 "egpacq: cease-ack from EGP neighbor %s at %s"

EGP_39 "egpacq: recv bad pkt: "
$ EGP_39         This message indicates that the neighbor acquisition packet
$ EGP_39         was received with an unknown code.  This message is the
$ EGP_39         comment field sent to traceeegp.

EGP_40 "egpacq: last recvd pkt bad, recvd in state %d\n"
$ EGP_40     This message is an alternative to the above depending on
$ EGP_40     the command line flags.  %d is the neighbor state from
$ EGP_40     our tables.

EGP_41 "egpsetint: Hello interval = %d or poll interval = %d too big from %s, code %d at %s"
$ EGP_41   The neighbor acquistion message of type %4 (type code)
$ EGP_41   received from %3 (inet address) at %5 (system time) had
$ EGP_41   either a hello interval of %1 or a poll interval of %2
$ EGP_41   which was too big.

EGP_42 "egpsetint: Hello interval = %d or poll interval = %d too big from %s, code %d"

EGP_43 "egpsetint: EGP neighbor %s specified hello/poll intervals %d/%d, we specified %d/%d"
$ EGP_43     The EGP neighbor whose inet address is %1 specified a hello
$ EGP_43     interval of %2 and a poll interval of %3, our system requires
$ EGP_43     a minimum hello interval of %4 and a minium poll interval
$ EGP_43     of %5.

EGP_44 "egpsetint: EGP neighbor %s specified hello/poll intervals %d/%d, we specified %d/%d at %s"

EGP_45 "egpsetint: Using actual intervals %d/%d with EGP neighbor %s"
$ EGP_45    After making certain adjustments, we are using a hello
$ EGP_45    interval of %1 and a poll interval of %2 with the EGP
$ EGP_45    neighbor whose inet address is %3.

EGP_46 "egpsetint: Using actual intervals %d/%d with EGP neighbor %s at %s\n"
EGP_47 "egpsetint: intervals hello %d, poll %d, egpsleep %d\n"
EGP_48 "egpstngh: acquired EGP neighbor %s at %s"
EGP_49 "egpstngh: acquired EGP neighbor %s"
EGP_50 "egpstime: bad sleep time %d\n"
EGP_51 "egphello: EGP neighbor %s thinks we are down"
EGP_52 "egphello: EGP neighbor %s thinks we are down at %s"
EGP_53 "egphello: recv bad pkt: "
EGP_54 "egphello: last recvd pkt bad, recvd in state %d\n"
EGP_55 "invalid Code field in NR Poll"
EGP_56 "IP Net Address field of NR Poll not expected"
EGP_58 "IP Net Address field of NR Poll not a directly connected network"
EGP_59 "NR Poll received while neighbor substate is down"
EGP_60 "too many NR Polls received"
EGP_61 "egppoll: recv bad pkt: "
EGP_62 "egppoll: last recvd pkt bad, recvd in state %d\n"
EGP_63 "invalid Code field in NR Response"
EGP_64 "NR Response IP Net Address field does not match command"
EGP_65 "too many unsolicited NR responses"
EGP_66 "invalid NR Response message format"
EGP_67 "internal error parsing NR Response"
EGP_68 "egpnr: recv bad pkt: "
EGP_69 "egpnr: last recvd pkt bad, recvd in state %d\n"
EGP_70 "egpserr: error packet to neighbor %s: %s at %s"
EGP_71 "egpserr: error packet to neighbor %s: %s"
EGP_72 "egpserr: error packet to non-neighbor %s: %s at %s"
EGP_73 "egpserr: error packet to non-neighbor %s: %s"
EGP_74 "egpserr: send error pkt:\n"
EGP_75 "egpserr: send error pkt "
EGP_76 "egpsnr: malloc: out of memory\n"
EGP_77 "egpsnr: malloc: out of memory at %s"
EGP_78 "egpsnr: NR message size (%d) larger than EGPMAXPACKETSIZE (%d)"
EGP_79 "egpsnr: NR message size (%d) larger than EGPMAXPACKETSIZE (%d) at %s"
EGP_80 "egpsnr: NR message not sent"
EGP_81 "egpsnr: NR message not sent at %s"
EGP_82 "egp_send: no interface for source address %s"
EGP_83 "egp_send: no interface for source address %s at %s"
EGP_84 "egp_send: improper route sending %d bytes to %s\n"
EGP_85 "egp_send: sendto error sending %d bytes to %s: %s\n"
EGP_86 "egpallcease: Terminate signal received"
EGP_87 "egpallcease: Terminate signal received at %s\n"
EGP_88 "egpallcease: Shutting down EGP"
EGP_89 "egpallcease: Shutting down EGP at %s\n"
EGP_90 "egpcheckas: lost all neighbors to AS %d\n"
EGP_91 "rt_NRupdate: above changes due to loss of neighbor %s at %s"

$set MS_HELLO
HELLO_1 "helloin: gw %s no shared net?\n"
HELLO_2 "HELLO: %s time delay %d %s\n"
HELLO_3 "latched"
HELLO_5 "helloin: ignoring invalid net %s from %s at %s"
HELLO_6 "helloin: invalid type %d\n"
HELLO_7 "hello_update: above %d routes from %s updated %s\n"
HELLO_8 "hellojob: Checking interface %s\n"
HELLO_9 "hellojob: No HELLO - %s is passive\n"
HELLO_10 "hellojob: No HELLO - %s is down\n"
HELLO_11 "hellojob: HELLO not allowed out %s\n"
HELLO_12 "hellojob: Sending HELLO packet to %s, interface %s\n"
HELLO_13 "Source HELLO gateway %s not on same net\n"
HELLO_14 "hellojob: Sending HELLO packet to %s, interface %s\n"
HELLO_15 "supply: Unknown protocol %d for net %s"
HELLO_16 "supply: Unknown protocol %d for net %s\n"
HELLO_17 "HELLO_SND: net %s time delay %d\n"
HELLO_18 "hello_send: no interface for source address %s\n"
HELLO_19 "hello_send: sendto() error sending %d bytes to %s"

$set MS_ICMP
ICMP_1 "ICMP: %s redirect"
ICMP_4 " from %s"
ICMP_6 " via %s: "
ICMP_7 "cannot redirect to myself"
ICMP_8 "not from router in use\n"
ICMP_9 "tables not changed\n"
ICMP_15 "ignoring martian net\n"
ICMP_16 "rt_redirect: ignoring invalid net %s at %s"
ICMP_18 "rt_redirect: above %s redirect from %s updated %s\n"

$set MS_IF
IF_2 "if_check: no info socket "
IF_9 "if_check: %s, address %s up"
IF_10 "if_check: %s, address %s up at %s"
IF_12 "if_check: %s, address %s down"
IF_13 "if_check: %s, address %s down at %s"
IF_14 "%s: interface %s: %s, addr %s, metric %d"
IF_15 "up"
IF_16 "down"
IF_17 ", broadaddr %s, "
IF_18 ", dstaddr %s, "
IF_19 "\n%s: interface %s: "
IF_20 "net %s, "
IF_21 "netmask %s, "
IF_22 "\n%s: interface %s: "
IF_23 "subnet %s, "
IF_24 "subnetmask %s\n"
IF_25 "if_updateactivegw: out of memory"

$set MS_INIT
INIT_1 "init_if: malloc: out of memory\n"
INIT_3 "init_if: more than 39 interfaces\n"
INIT_4 "init_if: malloc: out of memory\n"
INIT_14 "init_if: PointoPoint RIP supplier to: %s\n"
INIT_15 "init_if: PointoPoint HELLO supplier to: %s\n"
INIT_16 "init_if: Exit as multiple non PTP"
INIT_17 "interfaces to net of interface %s\n"
INIT_18 "if_init: Exit no interfaces\n"
INIT_19 "if_init: Acting as RIP supplier to our direct nets"
INIT_20 "if_init: Acting as RIP supplier to our direct nets\n"
INIT_21 "if_init: Acting as HELLO supplier to our direct nets"
INIT_22 "if_init: Acting as HELLO supplier to our direct nets\n"
INIT_32 "init_egpngh: EGP neighbors on different nets"
INIT_33 "init_egpngh: autonomous system # 0\n"
INIT_34 "init_egpngh: egpmaxacquire not specified, %d assumed\n"
INIT_35 "init_egpngh: egpmaxacquire not specified, %d assumed"
INIT_36 "init_egpngh: egpmaxacquire = %d > # neighbors = %d\n"
INIT_37 "init_egpngh: no EGP neighbors.\n"
INIT_38 "init_egpngh: %s: initialization error\n"
INIT_67 "parse_egpneighbor: missing neighbor name or address"
INIT_69 "parse_egpneighbor: invalid neighbor name or address %s\n"
INIT_70 "parse_egpneighbor: invalid parameter: %s"
INIT_77 "parse_egpneighbor: missing interface name or address"
INIT_79 "parse_egpneighbor: invalid interface name or address %s\n"
INIT_80 "parse_egpneighbor: invalid interface %s\n"
INIT_82 "parse_egpneighbor: missing source net name or address"
INIT_84 "parse_egpneighbor: invalid source net name or address %s\n"
INIT_85 "parse_egpneighbor: missing gateway name or address"
INIT_87 "parse_egpneighbor: invalid gateway name or address %s\n"
INIT_88 "parse_egpneighbor: invalid gateway %s\n"
INIT_90 "parse_egpneighbor: malloc: out of memory\n"
INIT_91 "%s: missing value for parameter %s"
INIT_92 "%s: missing value for parameter %s\n"
INIT_94 "%s: invalid value (%s) for parameter %s"
INIT_95 "%s: invalid value (%s) for parameter %s\n"
INIT_98 "init_egp: no direct net to neighbor %s\n"
INIT_99 "init_egp: no direct net to neighbor %s's gateway\n"
INIT_102 "No service for router available\n"
INIT_105 "get_rip_socket: RIP receive buffer size set to %dK\n"
INIT_109 "get_egp_socket: EGP receive buffer size set to %dK\n"
INIT_111 "get_egp_socket: EGP send buffer size set to %dK\n"
INIT_112 "init_if: alias found on interface %s\n"

$set MS_MAIN
MAIN_4 "Usage: %s [i][e][r][p][u][R][H][N]] [logfile]\n"
MAIN_5 "Cannot get internet socket\n"
MAIN_7 "SIOCGIFADDR error on lo0\n"
MAIN_9 "%s: ERROR: SRC not found, continuing without SRC support\n"
MAIN_10 "Start %s version %s"
MAIN_11 "Start %s[%d] version %s at %s\n"
MAIN_12 "main: can't open snmp socket"
MAIN_13 "No config file, one interface and a default route, gated exiting\n"
MAIN_14 "\n***Routes are being installed in kernel\n\n"
MAIN_15 "\n***Routes are not being installed in kernel\n\n"
MAIN_19 "%s version %s \n\tpid %d, started %s"
MAIN_29 "main: commence routing updates:\n\n"
MAIN_31 "%s: ERROR: '%d' recvfrom\n"
MAIN_32 "ERROR: gated does not support this option.\n"
MAIN_33 "setsockopt SO_DEBUG: %m"
MAIN_73 "Timer control for periodic route-age and interface processing"
MAIN_77 "Toggles trace on/off"
MAIN_81 "Sets dump flag so database can be dumped"
MAIN_85 "Terminates gated"
MAIN_91 "recvpkt: recvfrom"
MAIN_92 "recvpkt: fromlen %d invalid\n\n"
MAIN_93 "recvfrom: packet discarded, length %d > %d"
MAIN_94 ", from addr %s\n\n"
MAIN_95 "JOB BEGIN:\ttime: %u egp: %u rip: %u hello: %u\n\t\trt: %u if: %u stamp: %u\n"
MAIN_96 "TIMESTAMP: %s"
MAIN_97 "JOB CALL:\tegpjob()\n"
MAIN_98 "JOB RETURN:\tegpjob()\n"
MAIN_99 "JOB CALL:\ttoall()\n"
MAIN_100 "JOB RETURN:\ttoall()\n"
MAIN_101 "JOB CALL:\thellojob()\n"
MAIN_102 "JOB RETURN:\thellojob()\n"
MAIN_103 "JOB CALL:\trt_time()\n"
MAIN_104 "JOB RETURN:\trt_time()\n"
MAIN_105 "JOB CALL:\tdumpinfo()\n"
MAIN_106 "JOB RETURN:\tdumpinfo()\n"
MAIN_107 "JOB CALL:\tif_check()\n"
MAIN_108 "JOB RETURN:\tif_check()\n"
MAIN_109 "JOB END:\ttime: %u egp: %u rip: %u hello: %u\n\t\trt: %u if: %u stamp: %u\n"
MAIN_110 "JOB TIME:\tinterval: %u delta: %u\n"
MAIN_112 "DELETE"
MAIN_113 "Exit %s version %s"
MAIN_114 "\nExit %s[%d] version %s at %s\n"
MAIN_117 "setdumpflg: dump request received"
MAIN_118 "setdumpflg: dump request received at %s"
MAIN_119 "chgtrace: can not toggle tracing to console"
MAIN_120 "set_reinit_flag: re-init request received"
MAIN_121 "set_reinit_flag: re-init request received at %s"
MAIN_122 "main: initialization file %s missing"
MAIN_123 "main: initialization file %s missing at %s"
MAIN_124 "reinit: reinitializing from %s started"
MAIN_125 "reinit: reinitializing from %s started at %s"
MAIN_126 "reinit: reinitializing from %s done"
MAIN_127 "reinit: reinitializing from %s done at %s"
MAIN_129 "conf_open: initialization file %s missing - using defaults\n"
MAIN_133 "conf_open: error opening %s\n"

$set MS_MIB
MIB_1 "snmpin: AGENT packet type %d received from %s, "
MIB_2 "Size: %d bytes\n"
MIB_3 "snmpin: unexpected AGENT pkt type"
MIB_4 "snmpin: unexpected AGENT pkt type\n"
MIB_5 "snmpin: Route next %s "
MIB_6 " is %s"
MIB_7 " can not find next route..."
MIB_9 "snmpin: Route %s "
MIB_11 "snmpin: invalid AGENT pkt type"
MIB_12 "snmpin: invalid AGENT pket type\n"
MIB_13 "snmp_in: sendto() error"
MIB_14 "register_snmp_vars: sendto() error"
MIB_19 "No \"mib\" port in /etc/services\n"
MIB_23 "EGP neighbor %s loss\n"
MIB_24 "send_trap: sendto() error"

$set MS_RIP
RIP_1 "protocol not INET"
RIP_2 "ignoring version 0 packets"
RIP_3 "can not find interface for route"
RIP_4 "not on trustedripgateways list"
RIP_5 "not on same net"
RIP_6 "interface marked for no RIP in/out"
RIP_7 "not supplying RIP"
RIP_9 "not from a trusted port"
RIP_11 "not listening to RIP on this interface"
RIP_12 "TRACE packets not supported"
RIP_13 "(none)"
RIP_17 "interface marked for no RIP in"
RIP_18 "ripin: ignoring invalid net %s from %s at %s"
RIP_19 "invalid or not implemented command"
RIP_20 "rip_update: above routes supplied from %s updates %s\n"
RIP_22 "ripin: ignoring RIP %s packet from %s - %s\n"
RIP_24 "toall: Checking interface %s\n"
RIP_25 "toall: No RIP - %s is passive\n"
RIP_26 "toall: No RIP - %s is down\n"
RIP_27 "toall: RIP not allowed out %s\n"
RIP_28 "toall: Sending RIP packet to %s, flags %d, interface %s\n"
RIP_29 "toall: Source RIP gateway %s not on same net"
RIP_30 "toall: Source RIP gateway %s not on same net\n"
RIP_31 "toall: Sending RIP packet to %s, flags %d, interface %s\n"
RIP_32 "sendripmsg: bad route %s"
RIP_33 "sendripmsg: bad route %s\n"
RIP_34 "supply: Unknown protocol %d for net %s"
RIP_35 "supply: Unknown protocol %d for net %s\n"

$set MS_RIPQUERY
RIPQUERY_5 "Wait time set to %d\n"
RIPQUERY_6 "usage: ripquery [ -n ] [ -p ] [ -r ] [ -v ] [ -w time] hosts...\n"
RIPQUERY_9 "%s: unknown\n"
RIPQUERY_12 "No service for router available\n"
RIPQUERY_14 "from %s(%s):\n"
RIPQUERY_15 "**Non-zero port (%d) **"
RIPQUERY_19 "default"

$set MS_RT_CONTROL
RT_CONTROL_1 " control_add: malloc: out of memory\n"
RT_CONTROL_10 "rt_resinit: unexpected EOLN at line %d\n"
RT_CONTROL_17 "rt_resinit: invalid %s %s at line %d\n"
RT_CONTROL_22 "rt_resinit: 'gateway' expected at line %d\n"
RT_CONTROL_26 "rt_resinit: 'intf' expected at line %d\n"
RT_CONTROL_28 "rt_resinit: No interface list at line %d\n"
RT_CONTROL_30 "rt_resinit: 'all' unexpected at line %d\n"
RT_CONTROL_33 "rt_resinit: 'proto' expected at line %d\n"
RT_CONTROL_35 "rt_resinit: invalid host %s at line %d\n"
RT_CONTROL_38 "rt_resinit: invalid interface %s at line %d\n"
RT_CONTROL_39 "rt_resinit: max number of interfaces at line %d\n"
RT_CONTROL_42 "rt_resinit: announce and noannounce specified "
RT_CONTROL_43 "for interface %s at line %d\n"
RT_CONTROL_50 "rt_resinit: No protocol list at line %d\n"
RT_CONTROL_52 "rt_resinit: 'all' unexpected at line %d\n"
RT_CONTROL_55 "rt_resinit: 'egpmetric' expected at line %d\n"
RT_CONTROL_62 "rt_resinit: unknown protocol %s at line %d\n"
RT_CONTROL_66 "rt_resinit: invalid EGP metric %d at line %d\n"
RT_CONTROL_82 "The \"Do not announce\" list and \"announce\" list both\n"
RT_CONTROL_83 "initialized globally.  You can only have one or the other\n"
RT_CONTROL_84 "applied globally.  You can have both on different interfaces.\n"
RT_CONTROL_85 "rt_resinit: %s: initialization error\n"
RT_CONTROL_89 "rt_ASinit: out of memory\n"
RT_CONTROL_90 "rt_ASinit: out of memory"
RT_CONTROL_92 "rt_ASinit: duplicate %sannouncetoAS in line %d, %sannouncetoAS already specified for AS %d\n"
RT_CONTROL_97 "rt_ASinit: duplicate %sannouncetoAS in line %d, %sannouncetoAS already specified for AS %d"
RT_CONTROL_105 "rt_ASinit: invalid keyword: %s"
RT_CONTROL_106 "rt_ASinit: invalid keyword: %s\n"
RT_CONTROL_114 "rt_ASinit: %sannouncetoAS %d %srestrict ASlist"
RT_CONTROL_124 "rt_ASinit: unexpected EOLN at line %d\n"
RT_CONTROL_127 "rt_ASinit: invalid net %s at line %d\n"
RT_CONTROL_140 "rt_ASinit: %s: initialization error\n"
RT_CONTROL_141 "rt_ASinit: %s: reinitialization error - changes ignored\n"
RT_CONTROL_142 "martian_init: out of memory"
RT_CONTROL_144 "is_martian: invalid net %s in martian table\n"

$set MS_RT_EGP
RT_EGP_1 "No interface for egp, %s wanted\n"
RT_EGP_2 "No interface for egp, %s wanted"
RT_EGP_3 "rt_NRnets: malloc: out of memory"
RT_EGP_4 "rt_NRnets: malloc: out of memory\n"
RT_EGP_6 "not propogating default.\n"
RT_EGP_7 "not Class A, B or C\n"
RT_EGP_8 "rt_NRnets: net not class A, B or C: %s"
RT_EGP_9 "not on egpnetsreachable list\n"
RT_EGP_10 "shared net.\n"
RT_EGP_11 "direct routes not valid for this AS.\n"
RT_EGP_12 "restrictions prohibit announcement\n"
RT_EGP_13 "exterior routes not announced\n"
RT_EGP_14 "interior routes not valid for this AS.\n"
RT_EGP_16 "not originating default\n"
RT_EGP_17 "split horizon\n"
RT_EGP_18 "not valid for this AS\n"
RT_EGP_20 "no AS restrictions\n"
RT_EGP_21 "not sending kernel routes\n"
RT_EGP_22 "not sending redirected routes\n"
RT_EGP_23 "unknown protocol %d\n"
RT_EGP_24 " DEFAULT - "
RT_EGP_26 "added to update distance %3d gateway %s\n"
RT_EGP_27 "No interface for egp, %s wanted\n"
RT_EGP_28 "No interface for egp, %s wanted"
RT_EGP_29 "rt_NRupdate: net %-15s not class A, B or C from %-15s via %-15s at %s"
RT_EGP_30 "rt_NRupdate: net %s not class A, B or C from %s via %s\n"
RT_EGP_31 "rt_NRupdate: ignoring invalid net %-15s from %-15s via %-15s at %s"
RT_EGP_32 "rt_NRupdate: ignoring invalid net %s from %s via %s"
RT_EGP_33 "rt_NRupdate: ignoring net %-15s from %-15s via %-15s at %s"
RT_EGP_34 "rt_NRupdate: ignoring net %s from %s via %s"
RT_EGP_35 "rt_NRupdate: net %-15s not valid from AS %5d at %s"
RT_EGP_36 "rt_NRupdate: net %s not valid from AS %d"
RT_EGP_38 "rt_NRupdate: above %d routes from %s updated %s\n"

$set MS_RT_IF
RT_IF_1 "\nrt_ifinit: interior routes for direct interfaces:\n"
RT_IF_2 "rt_ifinit: interface %s: %s marked passive\n"
RT_IF_3 "\nrt_ifoptinit: interface options (if any):\n"
RT_IF_7 "rt_ifoptinit: syntax error, line %d\n"
RT_IF_9 "rt_ifoptinit: invalid interface address %s\n"
RT_IF_19 "rt_ifoptinit: metric %d invalid in line %d\n"
RT_IF_21 "rt_ifoptinit: metric %d invalid in line %d\n"
RT_IF_22 "rt_ifoptinit: unsupported protocol %s in line %d\n"
RT_IF_23 "fixed metric interface %s protocol %s metric %d"
RT_IF_25 "rt_ifoptinit: %s: initialization error\n"

$set MS_RT_INIT
RT_INIT_6 "rt_readkernel: rthost and rtnet not in namelist\n"
RT_INIT_12 "rt_readkernel: rthashsize not in namelist\n"
RT_INIT_13 "rt_readkernel: can't malloc %d bytes"
RT_INIT_14 "rt_readkernel: error reading kmem\n"
RT_INIT_15 "\nrt_readkernel: Initial routes read from kernel (if any):\n"
RT_INIT_16 "rt_readkernel: ignoring invalid net %s at %s"
RT_INIT_17 "rt_readkernel: error reading kmem\n"
RT_INIT_18 "rt_dumbinit: non-routing gateway routes (if any):\n"
RT_INIT_23 "rt_dumbinit: syntax error, line %d\n"
RT_INIT_25 "rt_dumbinit: invalid %s name or address %s\n"
RT_INIT_28 "rt_dumbinit: invalid gateway name or address %s\n"
RT_INIT_34 "unsupported protocol %s line %d\n"
RT_INIT_38 "rt_dumbinit: syntax error, line %d"
RT_INIT_44 "rt_dumbinit: invalid metric: %d, line %d"
RT_INIT_45 "rt_dumbinit: invalid metric: %d, line %d\n"
RT_INIT_47 "rt_dumbinit: invalid gateway name or address %s"
RT_INIT_48 "rt_dumbinit: invalid gateway name or address %s\n"
RT_INIT_52 "unsupported protocol %s line %d"
RT_INIT_55 "rt_dumbinit: error adding default route\n"
RT_INIT_57 "rt_dumbinit: out of memory"
RT_INIT_62 "unsupported default type %s line %d"
RT_INIT_63 "unsupported default type %s line %d\n"
RT_INIT_64 "rt_dumbinit: %s: initialization error\n"
RT_INIT_78 "ignored with announce or noannounce restrictions.\n"
RT_INIT_84 "NRadvise_init: invalid name or address %s"
RT_INIT_90 "bad interface in control list.\n"
RT_INIT_91 "out of memory\n"
RT_INIT_99 "rt_NRadvise_init: %s: initialization error\n"
RT_INIT_100 "init_options: Reading configuration protocol options:\n"
RT_INIT_105 "%s options: "
RT_INIT_113 "EGP conversion factor error - %d\n"
RT_INIT_121 "RIP default metric error - %d\n"
RT_INIT_123 "HELLO default metric error - %d\n"
RT_INIT_126 "init_options: %s: initialization error\n"

$set MS_RT_TABLE
RT_TABLE_1 "rt_add: malloc: out of memory\n"
RT_TABLE_2 "rt_add: interface not found for net %-15s gateway %s\n"
RT_TABLE_3 "invalid %s route %s not announced"
RT_TABLE_6 "no announcelist for %s route %s, may be restricted "
RT_TABLE_12 "noannouncelist for %s route %s"
RT_TABLE_21 "rt_default: no active default route\n"
RT_TABLE_22 "rt_time: Invalid exterior route to net %s via %s at %s"
RT_TABLE_23 "rt_time: Invalid interior route to net %s via protocol %s at %s"
RT_TABLE_24 "rt_time: above %d routes deleted and %d routes helddown %s\n"
RT_TABLE_25 "interface timeout - deleting route to %s\n"
RT_TABLE_28 "GW UNREACH: "
RT_TABLE_29 "rt_lookupnext malloc failed"
RT_TABLE_30 "rt_lookupnext n_routes = %d is too small"
RT_TABLE_31 "rt_lookupnext n_routes = %d, i = %d"

$set MS_STR
STR_1 "cap: rcvd \"%s\"\n"
STR_2 "cap: rtn \"%s\", string == \"%s\"\n"
STR_3 "next_white: rcvd \"%s\"\n"
STR_4 "next_white: rtn %d\n"
STR_5 "asubstr: rcvd \"%s\"(string) %d(start) %d(stop)\n"
STR_6 "asubstr: rtn \"%s\"\n"

$set MS_TRACE
TRACE_5 "gw %-15s metric %5d  proto "
TRACE_8 "  flags"
TRACE_11 "  state"
TRACE_14 "  hwin_min %d "
TRACE_15 "  fromproto "
TRACE_18 "  extmetric %d"
TRACE_19 "  as %d"
TRACE_25 "Tracing to %s suspended at %s"
TRACE_26 "tracing to %s suspended"
TRACE_28 "error opening %s: %m"
TRACE_33 "tracing to %s started"
TRACE_34 "\fTracing to %s started at %s"
TRACE_38 "traceflags: invalid trace flag '%s' ignored\n"
TRACE_40 "\nTracing flags enabled: "
TRACE_43 "none\n\n"
TRACE_45 "\nPacket received at time %s"
TRACE_46 "protocol %d, from addr %s"
TRACE_47 ", on interface addr %s\n"
TRACE_48 "Data octets (decimal):\n"
TRACE_52 "Invalid"
TRACE_63 " -> %s length %d at %s"
TRACE_68 "%s vers %d, type %s(%d), code %s(%d), status %s(%d)%s, AS# %d, id %d"
TRACE_69 " UNSOLICITED"
TRACE_71 ", hello int %d, poll int %d"
TRACE_72 ", src net %s"
TRACE_73 ", src net %s, #int %d, #ext %d\n"
TRACE_76 ", error %d (invalid)\nreturned header: "
TRACE_77 ", error: %s\nreturned header: "
TRACE_78 "vers %d, typ %d, code %d, status %d, AS# %d,"
TRACE_79 " id %d, %d, %d"
TRACE_82 "RIP %s %s.%d vers %d, cmd %s, length %d"
TRACE_83 " at %s"
TRACE_84 "\tnet %-15s  metric %2d  size %d\n"
TRACE_85 ", file %*s at %s"
TRACE_86 ", net %s at %s"
TRACE_90 "%s length %d"
TRACE_91 ", %d nets"
TRACE_92 " at %s\n"
TRACE_93 "Unknown"
TRACE_94 "%s %s.%d type %s(%d) length %d at %s"
TRACE_95 "%s\tinteger length %d value: %d 0x%x\n"
TRACE_96 "%s\tstring length %d value: "
TRACE_99 "%s\tIP address length %d value: %s 0x%x\n"
TRACE_100 "%s\tcounter length %d value: %u 0x%x\n"
TRACE_101 "%s\tgauge length %d value: %u 0x%x\n"
TRACE_102 "%s\ttimer length %d value: %u 0x%x\n"
TRACE_103 "%s\tlength %d variable: "
TRACE_109 "dumpinfo: processing dump to %s"
TRACE_110 "dumpinfo: processing dump to %s at %s"
TRACE_111 "\f\n\t%s[%d] version %s memory dump on %s at %s\n"
TRACE_113 "Interface Stats:\n\n"
TRACE_114 "\tInterface: %s\t\t\tMetric: %d\n"
TRACE_115 "\tAddress: %s\n"
TRACE_116 "\tUp-down transitions: %u\n"
TRACE_117 "\tBroadcast Address:   %s\n"
TRACE_118 "\tDestination Address: %s\n"
TRACE_119 "\tNet Number:    %s\t\tNet Mask:    %lx\n"
TRACE_120 "\tSubnet Number: %s\t\tSubnet Mask: %lx\n"
TRACE_121 "\tFlags: <"
TRACE_128 "NONE\n"
TRACE_132 "\tActive gateways on interface:"
TRACE_140 "EGP status:\n\tN_remote_nets: %d\t\tdefaultegpmetric: %d\n"
TRACE_141 "\tAutonomous System: %d\t\tacquired: %d/%d\n"
TRACE_143 "\tPackets In: %d\t\t\tErrors In: %d\n"
TRACE_144 "\tPackets Out: %d\t\t\tErrors Out: %d\n"
TRACE_145 "\t\t\t\t\tTotal Errors: %d\n"
TRACE_146 "\n\tNeighbor: %s"
TRACE_147 "\t\tInterface: %s\n"
TRACE_148 "\tHello time: %2d:%02d:%02d\t\tHello interval: %d\n"
TRACE_149 "\tPoll time: %2d:%02d:%02d\t\tPoll interval: %d\n"
TRACE_150 "\tState: <"
TRACE_151 ">\t\tReachability: <%s|%s>\n"
TRACE_156 "\tLast poll received: %s"
TRACE_157 "\tNet to poll: %s\n"
TRACE_158 "\tFlags: %X <"
TRACE_164 "N/A"
TRACE_184 "AS restriction lists:\n"
TRACE_186 "Does not receive"
TRACE_187 "receives"
TRACE_192 "AS validation:\n"
TRACE_195 "Martian networks:\n\t"
TRACE_200 "\n\nRouting Tables:\nEntries:\t%d\n"
TRACE_201 "\tUnits:\n\t\tInternal metric:\tmilliseconds\n\t\tAge:\t\t\tseconds\n\n"
TRACE_202 "\tDestination:       %s\n"
TRACE_203 "\tAutonomous System: %d\n"
TRACE_204 "\tGateway:           %s\n"
TRACE_205 "\tInterface:         %s\n"
TRACE_206 "\tAge:               %d\n"
TRACE_207 "Unknown"
TRACE_208 "\tProtocol:          %s\n"
TRACE_209 "\tMetric:            %d\n"
TRACE_210 "\tExterior Metric:   %d\n"
TRACE_211 "\tFlags:             <"
TRACE_215 "\tState:             <"
TRACE_219 "\tFrom Protocol:     <"
TRACE_223 "\tMinimum HELLO time delay in last %d updates: %d\n"
TRACE_224 "\tLast %d HELLO time delays:\n\t\t"
TRACE_227 "\tRestrictions:\n"
TRACE_228 "\t\tANNOUNCED via <"
TRACE_233 "OUT ALL INTERFACES\n"
TRACE_234 "OUT INTERFACES:\n"
TRACE_237 "\t\tNOT ANNOUNCED via <"
TRACE_246 "\t\tNOTLISTENEDTO via <"
TRACE_251 "FROM ALL INTERFACES\n"
TRACE_252 "FROM INTERFACES:\n"
TRACE_255 "\t\tLISTENEDTO via <"
TRACE_260 "FROM ALL GATEWAYS\n"
TRACE_261 "FROM GATEWAYS:\n"
TRACE_265 "dumpinfo: dump completed to %s"
TRACE_266 "dumpinfo: dump completed to %s at %s"
TRACE_267 "net %s (%c) - %d interior gateways, %d exterior gateways\n"
TRACE_268 "\t%s gateway %s, %d distances\n"
TRACE_269 "interior"
TRACE_270 "exterior"
TRACE_271 "\t\tdistance %d, %d networks\n"
TRACE_273 "\tpacket smashed\n"
