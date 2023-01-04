$ 
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
$ @(#)$RCSfile: ping.msg,v $ $Revision: 4.1.2.2 $ (DEC) $Date: 1993/07/23 18:36:22 $
$ 
$ (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
$ ALL RIGHTS RESERVED
$
$ OSF/1 Release 1.0
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$ OBJECT CODE ONLY SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1988, 1989
$ All Rights Reserved
$ 
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$ 
$ ping.msg	4.1 16:48:44 7/15/90 SecureWare
$ messages for ping
$
$ Do not translate ping.
$ len
$ 
$quote "         define quote character.
$set MS_PING        Define initial set#

PKTSZERR "ping: The packet size is too large.\n"
$	  The packet size entered on the command line was too big.
DATBYTS1 "PING %s: (%s): %d data bytes\n"
$	informational.  
DATBYTS2 "PING %s: %d data bytes\n"
$ 	  informational.  Prints the hostname and the packet size to be used.
WRTBYTS "ping: wrote %s %d chars, ret=%d\n"
$	informational.  "chars" is an abbreviation for "characters", 
$	"ret" is an abbreviation for "return value".  these abbreviations
$	should be translated into abbreviations appropriate for the 
$	target language.
$ 	On sento() error, prints echo request packet information;
$ 	host name, bytes sent, return value. If return < 0, the
$ 	sento() error message above is printed. This message is
$ 	printed both when sento() returns -1 or the return value
$ 	doesn't equal the bytes written.
SHRTPKT "ping: The packet of %1$d bytes from %2$s is too short.\n"
$ 	The packet is shorter than the minimum required.
$ 	The packet size followed by the from address is printed.
ICMPSEQ "icmp_seq=%d. "
$	Do not translate
$ 	informational.  Prints the icmp sequence number of the packet.
PINGHDR "\n----%s PING Statistics----\n"
$ 	informational.  Header for statistcs listing
XMTPKTS "%d packets transmitted, "
$	informational.  reports number of packets transmitted.
RECPKTS "%d packets received, "
$	informational.  reports number of packets received.
LOSTPKTS "%d%% packet loss."
$ 	informational.  Calculted percentage of packets lost. xmt/rec*100.
RDTRIPTIM "round-trip (ms)  min/avg/max = %ld/%ld/%ld\n"
$       informational.  reports the round-trip time statistics of packets sent.
$       ms is abbreviation for milliseconds.  min is abbreviation for
$       minimum, avg is abbreviation for average, and max is abbreviation
$       for maximum.  abbreviation should be translated to corresponding
$       abbreviations in target language.
HOSTNOANS "ping: The host %s is not responding.\n"
$ 	Host never answered.
PING_AUTH    "%s: need ping authorization\n"
PING_PRIV    "%s: insufficient privileges\n"
PING_BAD_NUM	"ping: bad number of packets to transmit.\n"
PING_ROOT	"ping: you must be root to use either -f or -l option.\n"
PING_TIMING	"ping: bad timing interval.\n"
PING_BAD_PRELOAD	"ping: bad preload value.\n"
PING_ILLEGAL_PACK	"ping: illegal packet size.\n"
PING_UNK_HOST	"ping: unknown host %s\n", target
PING_INCOMP_OPTIONS	"ping: -f and -i incompatible options.\n"
PING_NO_MEM	"ping: out of memory.\n"
PING_UNKN_PROTO	"ping: unknown protocol icmp.\n"
PING_RECORD_RTE	"ping: record route not available in this implementation.\n"
PING_F_FLOOD	"%d bytes from %s: icmp_seq=%u"
PING_TTL	" ttl=%d"
PING_TIME	" time=%ld ms"
PING_DUP	" (DUP!)"
PING_WRONG_DB	"\nwrong data byte #%d should be 0x%x but was 0x%x"
PING_NL_TAB	"\n\t"
PING_X		"%x "
PING_BYTES_FROM	"%d bytes from %s: "
PING_LSRR	"\nLSRR: "
PING_ZERO	"\t0.0.0.0"
PING_FMT1	"\t%s"
PING_SAME_RTE	"\t(same route)"
PING_RR		"\nRR: "
PING_NOP	"\nNOP"
PING_UNKN_OPT	"\nunknown option %x"
PING_DUPLICATE	"+%ld duplicates, "
PING_SOMEONE	"-- somebody's printing up packets!"
PING_ECHO_REPLY	"Echo Reply\n"
PING_NET_UNREACH "Destination Net Unreachable\n"
PING_HOST_UNR	"Destination Host Unreachable\n"
PING_PROTO_UNR	"Destination Protocol Unreachable\n"
PING_PORT_UNR	"Destination Port Unreachable\n"
PING_FRAG_DF	"frag needed and DF set\n"
PING_RTE_FAIL	"Source Route Failed\n"
PING_DEST_BCODE	"Dest Unreachable, Bad Code: %d\n"
PING_SRC_QUENCH	"Source Quench\n"
PING_REDIRECT_NET	"Redirect Network"
PING_REDIRECT_HST	"Redirect Host"
PING_SERV_NET	"Redirect Type of Service and Network"
PING_SERV_HOST	"Redirect Type of Service and Host"
PING_RED_BCODE	"Redirect, Bad Code: %d"
PING_NEW_ADDR	"(New addr: 0x%08lx)\n"
PING_ECHO_REQ	"Echo Request\n"
PING_TTLIVE	"Time to live exceeded\n"
PING_FRAG_TIMEOUT	"Frag reassembly time exceeded\n"
PING_TIME_BCODE	"Time exceeded, Bad Code: %d\n"
PING_PARAM_PROB	"Parameter problem: pointer = 0x%02x\n"
PING_TIMESTAMP	"Timestamp\n"
PING_TIMESTAMP_REPLY	"Timestamp Reply\n"
PING_INFO_REQ	"Information Request\n"
PING_INFO_REPLY	"Information Reply\n"
PING_ADDR_MASK	"Address Mask Request\n"
PING_ADDR_REPLY	"Address Mask Reply\n"
PING_BAD_ICMP	"Bad ICMP type: %d\n"
PING_IP_HDR	"Vr HL TOS  Len   ID Flg  off TTL Pro  cks      Src      Dst Data\n"
PING_FMT2	" %1x  %1x  %02x %04x %04x"
PING_FMT3	"   %1x %04x"
PING_FMT4	"  %02x  %02x %04x"
PING_FMT5	" %s "
PING_FMT6	"%02x"
PING_TCP_DUMP	"TCP: from port %u, to port %u (decimal)\n"
PING_UDP_DUMP	"UDP: from port %u, to port %u (decimal)\n"
PING_PATTERNS	"ping: patterns must be specified as hex digits.\n"
PING_PATTERN	"PATTERN: 0x"
PING_NEWLINE	"\n"
PING_USAGE	"usage: ping [-Rdfnqrv] [-c count] [-i wait] [-l preload]\n\t[-p pattern] [-s packetsize] host\n"
