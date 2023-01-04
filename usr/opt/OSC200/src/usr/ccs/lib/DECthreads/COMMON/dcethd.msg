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
$ @(#)$RCSfile: dcethd.msg,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/04/13 21:35:52 $
$ 
$ ++
$ ! @OSF_COPYRIGHT@
$ !
$ !  FACILITY:
$ !
$ !      DECthreads Message Catalog
$ !
$ !  FILENAME:
$ !
$ !      dcethreads.msg
$ !
$ !  ABSTRACT:
$ !
$ !	 DCE threads messages.
$ !
$ !  AUTHORS:
$ !
$ !	 Dave Butenhof
$ !
$ !  CREATION DATE:
$ !
$ !	 06 June 1991
$ !
$ !  MODIFICATION HISTORY:
$ !
$ !	001	Dave Butenhof	20 November 1991
$ !		Remove arguments from exceptions 01 and 02: the arguments
$ !		weren't used, and these are really placeholders anyway, since
$ !		those statuses aren't even used except on VMS.
$ !	002	Dave Butenhof	08 January 1992
$ !		Remove "Exception: " prefix from messages; instead,
$ !		exc_report will add it. That way it'll show on all exceptions
$ !		reported by DECthreads, not just DECthreads facility codes.
$ !	003	Dave Butenhof	22 March 1993
$ !		Change address exception format to lX rather than 08X.
$ --
$ set 1 
01 Address exception raised, 0x%1$lx
02 Status exception raised
03 Uninitialized exception raised
05 Invalid memory address
06 Operation failed due to insufficient quota
07 Insufficient virtual memory for requested operation
08 Insufficent privilege for requested operation
09 Normal successful completion
10 Illegal instruction
11 Reserved addressing error
12 Privileged instruction error
13 Reserved operand error
14 Trace or breakpoint trap
15 IOT trap
16 EMT trap
17 Arithmetic error
18 Bad system call
19 Broken pipe
20 CPU-time limit exceeded
21 File-size limit exceeded
22 Integer overflow
23 Integer divide by zero
24 Floating point overflow
25 Floating point/decimal divide by zero
26 Floating point underflow
27 Decimal overflow
28 Subscript out of range
48 Thread has been canceled
49 A thread assertion check has failed
50 Invalid parameter to operation
51 Internal error detected in thread library
52 Current thread has been requested to exit
53 Object referenced does not currently exist
54 Object referenced is already in use
55 Operation inappropriate for specified object
56 Wrong mutex specified in condition wait
57 Attempted stack overflow was detected
58 No space is currently available to create a new stack
59 The current stack was not allocated by thread library
60 Timed condition wait expired
61 Unimplemented feature
62 Thread library initialization is already in progress
63 No space currently available to process interrupt request
64 Unable to process condition signal from interrupt level
65 Improper nesting of alert scope
128 Unknown status code 0x%1$08X
