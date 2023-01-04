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
$set MS_LIBTLI
$quote "
$ The following  are from libtli/terrnolst.c
1		"Incorrect address format"
2		"Incorrect option format"
3		"Incorrect permissions"
4		"Illegal transport fd"
5		"Could not allocate address"
6		"Out of state"
7		"Bad call sequence number"
8		"System error"
9		"Event requires attention"
10		"Illegal amount of data"
11		"Buffer not large enough"
12		"Flow control"
13		"No data"
14		"No disconnect indication available"
15		"No unitdata error indication available"
16		"Bad flags"
17		"No orderly release indication available"
18		"Primitive not supported"
19		"State is in process of changing"
20		"Unsupported struct-type requested"
21		"Invalid transport provider name"
22		"Qlen is zero"
23		"Address in use"
$ gencat allocates 24, 25, 26 for these
TERROR_MSG	"t_error"
TERROR_ERR	"%s: Error %d occured\n"
TERROR_STRERR	"%d: error unknown"
$ XPG4 additions - terror and tstrerror shift by 3
27		"Outstanding connection indications"
28		"Transport provider mismatch"
29		"Resfd specified to accept with qlen > 0"
30		"Resfd not bound to same address as fd"
31		"Incoming connection queue is full"
32		"XTI protocol error"
