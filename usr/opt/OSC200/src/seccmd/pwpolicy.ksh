#!/usr/bin/ksh -p
# 
# *****************************************************************
# *                                                               *
# *    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
# *                                                               *
# *   All Rights Reserved.  Unpublished rights  reserved  under   *
# *   the copyright laws of the United States.                    *
# *                                                               *
# *   The software contained on this media  is  proprietary  to   *
# *   and  embodies  the  confidential  technology  of  Digital   *
# *   Equipment Corporation.  Possession, use,  duplication  or   *
# *   dissemination of the software and media is authorized only  *
# *   pursuant to a valid written license from Digital Equipment  *
# *   Corporation.                                                *
# *                                                               *
# *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
# *   by the U.S. Government is subject to restrictions  as  set  *
# *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
# *   or  in  FAR 52.227-19, as applicable.                       *
# *                                                               *
# *****************************************************************
#
# HISTORY
# 
# @(#)$RCSfile: pwpolicy.ksh,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/04/01 20:16:50 $
# 

:
: This is a null site password policy callout.
:

exit 0

: # The protocol exchanged between the passwd-changing programs and this
: # executable is briefly described here, and uses symbolic constants
: # defined in <prot.h>.
:
: # The pwpolicy program is currently expected to product no output on stdout
: # or stderr.  It receives its instructions on stdin.  Its only allowable
: # form of answer is 0/non-0 exit code.
:
: # It receives a function to perform as an ASCII representation of a
: # decimal integer, followed by a newline.  The subsequent parameters
: # are supposedly function-specific, but there is presently only one
: # function (accept/no-accept on validity of a password).  The input
: # parameters for this function are (again, on separate lines, alone on
: # their lines):  The username for whom we're trying to set a password;
: # and the proposed password.
: # (The username is so that the program can make calls to getpwnam and/or
: # getprpwnam to check on any other data which it considers relevant.)
:
: # If this file is missing, and there are any users whose protected
: # profiles require that their passwords be checked by this executable,
: # then those users will be unable to change their passwords, and an audit
: # record will be attempted to flag the condition.  A LOG_CRITICAL call to
: # syslog will also be made.

: exit 0 # did this above to get out faster if called by mistake
