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
# @(#)$RCSfile: Makefile,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/04/14 17:47:15 $
#
EXECUTABLES	= XIsso XSysAdmin redirect
SEC_DATAFILES 	= isso.msg sysadmin.msg XIsso.uid XSysAdmin.uid ${EXECUTABLES}

DATAFILES	= ${SEC_DATAFILES}

SEC_ILIST	= ${SEC_DATAFILES}
ILIST		= ${SEC_ILIST}

isso.msg_IMODE		= 444
isso.msg_IDIR   	= /usr/share/lib/sechelp/XIsso/
isso.msg_ILINKS 	= /usr/share/lib/sechelp/aif/isso.msg

sysadmin.msg_IMODE 	= 444
sysadmin.msg_IDIR      	= /usr/share/lib/sechelp/XSysAdmin/
sysadmin.msg_ILINKS    	= /usr/share/lib/sechelp/aif/sysadmin.msg


XIsso.uid_IMODE		= 444
XIsso.uid_IDIR		= /usr/lib/X11/uid/

XSysAdmin.uid_IMODE	= 444
XSysAdmin.uid_IDIR	= /usr/lib/X11/uid/

XSysAdmin_IMODE		= 2111
XSysAdmin_IDIR		= /var/tcb/bin/

XIsso_IMODE		= 2111
XIsso_IDIR		= /var/tcb/bin/

redirect_IMODE		= 111
redirect_IDIR		= /var/tcb/lib/

SYMLINKS		= ${EXECUTABLES}
SYMLINKDIR1		= ../../var/tcb/bin
SYMLINKDIR2		= /tcb/bin
SEC_IMODE		= 2111
SEC_IGROUP		= sec


redirect_SYMLINKDIR1	= ../../var/tcb/lib
redirect_SYMLINKDIR2	= /tcb/lib

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/others.mk

