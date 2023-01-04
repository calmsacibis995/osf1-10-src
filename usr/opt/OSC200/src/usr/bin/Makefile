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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.3.14.15 $ (DEC) $Date: 1993/11/23 22:00:44 $
#
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0
#
# Modification History
# --------------------
# 001	Piyanai		(1/17/91)
#     - Add ksh
# 002	ech		(4/22/91)
#     - Add newinv and kits
# 003	ech
#     - Add gendisk, gentapes, invsync, genufi, updmerge, updmv
# 004	lindner		(6/3/91)
#     - Add ic
# 005   terry		(6/3/91)
#     - Add domainname, ypcat, ypmatch, ypwhich, yppasswd
# 006	woodburn	(6/3/91)
#     - Add text
# 007   terry           (6/5/91)
#     - Add ntpq, ntpdc, xntpdc, ntp for Network Time Protocol
#
# 008	Peter Derr	(6/6/91)
#     -	Add cdoc, dtoc, ctod, ddifanls, dtifddif, ddiftext, textddif,
#	ddifps, and vdoc, all CDA commands (Digital Proprietary software)
# 009	gws		(6/6/91)
#     - removed eqn from SUBDIRS it is not supported.  neqn Makefile
#	already has the backpointers to the needed eqn sources so descent into
#	eqn subdir is not necessary for building neqn.
# 010	gws		(6/6/91)
#     - added tbl to SUBDIRS (tbl relocated from usr/local/nosupport)
# 011	gws		(6/6/91)
#     - added nroff to SUBDIRS (nroff relocated from usr/local/nosupport)
#	nroff Makefile already has the backpointers to the needed troff
#	sources so descent into troff subdir is not necessary.  troff is NOT
#	to be built.
# 012	haslock		(6/10/91)
#	- added devbook autoxref mancomp and conind to SUBDIRS
# 013	lebel		(6/25/91)
#	- added rpcgen
# 014 - dike
#	- added krash
# 015 - gaudet 		(7/2/91)
#	- added capsar
# 016 - lambert 	(8/9/91)
#	- added ltf to SUBDIRS
# 017 - gaudet Tue Oct  8 10:03:46 EDT 1991
#	- deleted msgs
# 018   aju             (10/09/91)
#       - added mhdecode, needed by dxmail to display DDIF/CDA files
#
#
SEC_SUBDIRS		= at binmail chgrp chown cpio date ex finger \
			  from hostname ipcs kill login mesg mt newgrp \
			  nice passwd ps su tar w
CDA_SUBDIRS		= cdoc ctod ddifanls ddiftext ddifps dtifddif \
			  dtoc textddif vdoc
SUBDIRS			= ${SEC_SUBDIRS} ${CDA_SUBDIRS} \
			  Mail adb apply asa awk banner basename bc bdiff \
			  bfs biff cal calendar capsar cat cdsuf checkeq checknr \
			  chmod cksum clear cmp col colcrt colrm comm \
			  compress cp crashdc crypt csh csplit ct cu cut \
			  dc dd deroff  df diction diff diff3 diffmk \
			  dircmp dirname domainname du echo ed env \
			  eucset expand explain expr false find fold ftp \
			  gawk gendisk gentapes genufi genxlt getconf getopt graph grep \
			  groups head help iconv id install invsync iostat \
			  ipcrm join kits kdbx ksh last lastcomm learn \
			  leave line ln locale localedef lock logger logname look ls ltf \
			  machine man mh mhdecode \
			  mkdep mkdir mkfifo more msg mv mxr nawk neqn newinv news \
			  nfsstat nl nohup nroff nslookup ntp ntpdc ntpq \
			  od pack pagesize \
			  paste pathchk patch pax pcat pfstat pg pr printf \
			  pwd rcp rcs rdist renice rev rlogin \
			  rm rmail rpcgen rsh ruptime rusers rup rwall rwho sccs script sdiff \
			  sed sh showmount sleep soelim sort spell \
			  spellin split spray stty style sum tabs tail \
			  talk tbl tcopy tee telnet test text tftp tic \
			  time tip touch tput tr trbsd true tset tty \
			  uac ul uname unexpand unifdef uniq units unpack updmerge \
			  updmv users uucp vacation \
			  vmstat wc whereis which who whoami whois \
			  window write xargs xntpdc xsend yes \
			  ypcat ypmatch yppasswd ypwhich fdetach strchg strconf

SYMLINKS	= cb cflow cxref error indent lex lint lorder m4 make mig \
		  mkstr ranlib strings tsort xstr yacc
SYMLINKDIR1	= ../ccs/bin
SYMLINKDIR2	= /usr/bin

# notyet - 

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/subdirs.mk
