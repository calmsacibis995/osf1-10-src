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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.6.17.6 $ (DEC) $Date: 1993/06/08 13:25:16 $
#
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

VPATH			= ${MAKETOP}/usr/include:${MAKETOP}/usr/include/${target_machine}
INCFILES		= standards.h dirent.h binsem.h \
			  mach_o_format.h mach_o_header.h mach_o_vals.h
MACH_O_INCFILES		= mach_o_types.h mach_o_header_md.h
ALPHA_INCFILES		= syms.h va_list.h varargs.h symconst.h sym.h \
			  stamp.h ${MACH_O_INCFILES}
MIPS_INCFILES		= syms.h varargs.h symconst.h sym.h \
			  stamp.h ${MACH_O_INCFILES}
MMAX_INCFILES		= syms.h varargs.h ${MACH_O_INCFILES}
AT386_INCFILES		= a.out.h stab.h varargs.h ${MACH_O_INCFILES}
HP_M68K_INCFILES        = a.out.h varargs.h aouthdr.h filehdr.h scnhdr.h

DATAFILES		= ${INCFILES} ${${TARGET_MACHINE}_INCFILES}
INCLUDES		= ${DATAFILES}

SUBDIRS			= mach arch io dec kern net netinet s5fs sys ufs \
			  vm lvm builtin cdfs dli mach_debug netns nfs \
			  streams streamsm svr4 sysV rpc rpcsvc klm procfs sec \
			  ffm  tty

OTHERS			= lnk_machine hal

ILIST	= ${INCLUDES}
IDIR	= /usr/sys/include/
IMODE	= 444
SYMLINKS = hal ${target_machine}
SYMLINKDIR1 = arch/${target_machine}
SYMLINKDIR2 = /usr/sys/include
${target_machine}_SYMLINK = machine
${target_machine}_SYMLINKDIR1 = arch


include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/datafiles.mk
include ${MAKEFILEPATH}/subdirs.mk
include ${MAKEFILEPATH}/others.mk

lnk_machine: ${ALWAYS}
	${RM} ${_RMFLAGS_} machine
	ln -s arch/${target_machine} machine

hal: ${ALWAYS}
	${RM} ${_RMFLAGS_} hal
	ln -s arch/${target_machine}/hal hal
