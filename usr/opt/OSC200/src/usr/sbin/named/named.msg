 
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
$ @(#)$RCSfile: named.msg,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 06:14:32 $
$
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1989
$ All Rights Reserved
$ Licensed Materials - Property of IBM
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ messages for named
$
$len 
$quote "         define quote character.
$set MS_NAMED        Define initial set#

$ db_dump  
$ Do not translate %s, %d, %1$s, etc. 
CKFILNM              "%s.chk"
$ Do not translate .chk . 
DMPTIM               "; Dumped at %s."
LOWMIN               "named: %d root hints... (too low)"
$ Do not translate named.
ROOTDMP              "; ---- Root hint cache dump ----\n"
$ Do not translate root, hint, cache. 
CACHDMP              "; ---- Cache dump ----\n"
$ Do not translate Cache. 
CACHDR               "; --- Cache and data ---\n"
$ Do not translate Cache. 
HINTSHDR             "; --- Hints ---\n"
$ Do not translate Hints. 
ORGMSG               "$Origin %s.\n"
UNSPECDMP            "named: Dump T_UNSPEC: There is not enough memory available now: %m\n"
$ Do not translate T_UNSPEC, named. 
USPECOUT             "named: Dump T_UNSPEC: There is an output buffer overflow.\n"
$ Do not translate T_UNSPEC, named . 
USPECBUF             "%s\n"
$ Do not translate. 
UNK                  "???\n"
NUMBER               "%d"
$ Do not translate. 
PORTNUM              "%d"
$ Do not translate. 
CONVBUF              "%1$ld %2$lx %3$lx %4$lx"
$ Do not translate. 
CONVBUFIN            "x %1$ld %2$lx %3$lx %4$lx"
$ db_load (db_lookup has no messages)  
LOGFILERR            "%1$s: %m"
UNKTYP               "named: %2$s on line %1$d is not a known type.\n"
$ Do not translate named.
CPUTYPERR            "%1$s: The CPU type on line %2$d is too long."
$ Do not translate CPU. 
OSTYPERR             "%1$s: The OS type on line %2$d is too long."
$ Do not translate OS. 
INBUFERR             "named: Load T_UNSPEC: There is an input buffer overflow."
$ Do not translate T_UNSPEC, named . 
USPECDATERR          "named: Load T_UNSPEC: Data is in an atob format that is not valid."
$ Do not translate T_UNSPEC, atob, named.. 
DBFMTERR             "%1$s: There is a database format error %3$s on line %2$d."
UNKOPT               "named: Line %1$d: The $ option $%2$s is not valid.\n"
$ Do not translate named.
SEQERR               "named: Line %d: Specify a value."
$ Do not translate named.
EOFERR               "named: Line %d: Reached an end of file that was not expected."
$ Do not translate named.
UNKPROTO             "%1$s: Protocol %3$s on line %2$d is not known."
UNKSERV              "%1$s: Service %3$s on line %2$d is not known."
PORTNUMERR           "%1$s: Port number %2$d on line %3$d is too large.\n"
PARENSERR            "%1$s: There is a missing ) character on line %2$d.\n"
$ db_reload  
$ Do not translate ) . 
RELOAD               "Reloading the nameserver.\n"
$ Do not translate nameserver. 
$ db_save  
SVNAM                "savename: %m."
$ Do not translate savename. 
SVDATA               "savedata: %m."
$ Do not translate savedata. 
SVHASH               "savehash: %m."
$ Do not translate savehash. 
SVINVQ               "saveinv: %m."
$ Do not translate saveinv. 
SVSTR                "savestr: %m."
$ Do not translate savestr. 
$ db_update  
ILLDATA              "named: %s has CNAME and other data (not permitted).\n"
$ Do not translate CNAME, named. 
$ ns_forw  
FMEMERR              "forw: %m."
$ Do not translate forw. 
$ ns_init  
BOOTFILERR           "%1$s: %m."
DIRERR               "Directory %1$s: %m.\n"
UNKFLD               "%1$s: Field %3$s on line %2$d is not known.\n"
INITZONE             "named: Do not specify more than %d zones.\n"
$ Do not translate named.
NOORG                "%1$s: There is a missing origin on line %2$d.\n"
BADRFSH              "%1$s: Refresh time %3$s on line %2$d is not valid.  It is ignored.\n"
CACHIGN              "%1$s: Cache refresh on line %2$d is ignored.\n"
BADADDR              "named: %s is ignored.  It is not dotted quad."
$ Do not translate named.
NOSRCADDR            "%1$s: A source or an address on line %2$d is missing.\n"
$ ns_main  
RESTRT               "Started again.\n"
SOCKSTRM             "socket  (SOCK_STREAM): %m."
$ Do not translate SOCK_STREAM, socket.
BINDERR              "bind(vs, %1$s[%2$d]): %m."
$ Do not translate vs, bind.
IFCONF               "get interface configuration: %m."
IFFLGS               "get interface flags: %m."
IFADDR               "get interface address: %m."
MMEMERR              "main: There is not enough memory available now.\n"
NETMSK               "get netmask: %m."
$ Do not translate netmask.
DESTADDR             "get dst address: %m"
$ Do not translate dst.
DGRAMSOCK            "socket (SOCK_DGRAM): %m."
$ Do not translate socket, SOCK_DGRAM.
DGBIND               "bind(dfd %1$d, %2$s[%3$d]): %m."
$ Do not translate bind, dfd.
SETFDSZ              "named: The return from getdtablesize() is greater than FD_SETSIZE."
$ Do not translate getdtablesize, FD_SETSIZE, named.
SELERR               "select: %m."
RCVERR               "Received from: %m."
ACCERR               "accept: %m."
USAGE                "Usage: named [-d DebugLevel] [-p PortNumber] [-b BootFile]\n"
$ Do not translate named.
SQADDMEM             "sqadd: There is not enough memory available now.\n"
$ Do not translate sqadd.
PEERNMERR            "getpeername: %m."
$ Do not translate getpeername.
TODERR               "gettimeofday failed: %m."
$ Do not translate gettimeofday.
$ ns_maint  
MTZONE               "zoneref: socket: %m."
$ Do not translate zoneref, socket.
MKQERR               "zoneref: The res_mkquery system call failed."
$ Do not translate zoneref, res_mkquery.
MTNOSOA              "There is no SOA from server %1$s. zone %2$s. (len 0).\n"
$ Do not translate SOA, len.
MASTER2              "zoneref: Masters for secondary zone %s cannot be reached."
$ Do not translate zoneref.
$ ns_req  
RQMEMERR             "ns_req: There is not enough memory available now.\n"
$ Do not translate ns_req.
DOMNMERR             "named: The domain name is too long: %s ...\n"
$ Do not translate named.
NAMERR               "Name_Too_Long"
$ Do not translate Name_Too_Long.
ZNLOOKERR            "InitDynUpdate: A lookup failed on zone %s.\n"
$ Do not translate InitDynUpdate.
RQNOSOA              "InitDynUpdate: Cannot find an SOA record for %s.\n"
$ Do not translate InitDynUpdate, SOA.
$ ns_resp  
SERRETNXD            "A server returned NXDOMAIN && !aa."
$ Do not translate NXDOMAIN, aa.
MAXQRY               "named: MAXQUERIES limit is exceeded.  There is a possible data loop in resolving %s."
$ Do not translate MAXQUERIES, named.
RPLYERR              "named: Response from %s is not in a valid form.\n"
$ Do not translate named.
ROOTERR              "check_root: Cannot find root.\n"
$ Do not translate check_root, root.
MINROOTERR           "check_root: %d root servers after query to root server is less than minimum."
$ Do not translate check_root.
NOROOTCLS            "There are no root nameservers for class %s.\n"
$ Do not translate nameservers.
$ ns_stats  
STATOPEN             "named: Cannot open statistics file %s.\n"
$ Do not translate named.
