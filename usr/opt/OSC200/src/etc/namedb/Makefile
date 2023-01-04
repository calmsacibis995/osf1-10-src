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
#
# HISTORY
#
#
# @(#)$RCSfile: Makefile,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1992/07/14 09:29:35 $
# 
#
# Modification History:
#
# 31-May-90	bbrown
#	Ported to ULTRIX/OSF.  Removed all but hosts processing.
#
# 28-Feb-90	sue
#	Added depend lines at end to continue make if a data file is
#	missing.
#
# 31-Jan-90	sue
#	Cleaned up the makefile.  Don't use time files.  Push after
#	all databases have been made.
#
# 16-Aug-89	sue
#	Explicitly state the location of the *.time files to
#	be the BIND directory /var/dss/namedb/src.
#
# 12-Jun-89	logcher
#	Created Makefile to make Hesiod files from etc files.
#

AWK =/bin/awk
TOUCH=/usr/bin/touch
ECHO=/bin/echo

NAMEDSRCDIR=/etc/namedb/src
NAMEDBINDIR=/etc/namedb/bin
NOPUSH=""
RESTART=/sbin/init.d/named restart
MAKE_HO=make_hosts -v

all: hosts.db push

push:
	-@if [ ! $(NOPUSH) ]; then \
		$(RESTART); \
		$(ECHO) "pushed"; \
	else \
		: ; \
	fi

hosts.db: $(NAMEDSRCDIR)/hosts
	-@if [ -f $(NAMEDSRCDIR)/hosts ]; then \
		$(NAMEDBINDIR)/$(MAKE_HO); \
		$(ECHO) "updated hosts"; \
	else \
		$(ECHO) "couldn't find $(NAMEDSRCDIR)/hosts"; \
	fi

hosts:		hosts.db push
$(NAMEDSRCDIR)/hosts:
