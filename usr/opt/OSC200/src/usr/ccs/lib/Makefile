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
# 	@(#)$RCSfile: Makefile,v $ $Revision: 4.2.20.17 $ (DEC) $Date: 1993/11/22 21:50:13 $
#
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

# This next line is a list of subdirs which are exported during pass2 (the
#   pass to export libc); This is a list of all items which must be
#   exported ahead of (or with) libc:
EXPLIBC_SUBDIRS		= libexc libc libg libm 

# Don't need to build library to access rose objects.
#EXPLIBLD_SUBDIRS	= libld
EXPLIBLD_SUBDIRS	=
EXPLIB_CMPLRS_SUBDIRS	= crt libexc libmld libfe libmisc libu \
			  libxmalloc libp libprof libus \
			  libdpi
EXPLIB_ECA_SUBDIRS      = libmir libmoi libmoss
EXPLIB_SUBDIRS		= libcapsar libPW libbsd libaio libsys5 libsys5_r \
			  libcurses libdbm libl liblmf libretired_malloc \
			  libmach libmp libntp DECthreads libcmalib \
			  libresolv librpc librpcsvc librt libsb \
                          libsetld libtermlib libtli libaud \
			  libutil libxti liby libsecurity \
			  libAF libc_r libcda_fe libcda_be libdvs \
			  libids_nox libbinlog libplot \
			  libchf libips libimg libcda libdnet \
			  libiconv libcdrom libstor \
			  ${EXPLIB_CMPLRS_SUBDIRS} migcom \
			  ${EXPLIB_ECA_SUBDIRS}

SUBDIRS			= libc libm ${EXPLIB_SUBDIRS} libchf libids_nox \
			  libips libimg \
			  libcda libplot cflow migcom xpass


SYMLINKS		= libAF.a libPW.a libaio.a libbsd.a libc.a libc_r.a \
			  libcmalib.a libdnet_stub.a libcurses.a libdbm.a \
			  libl.a liblmf.a libln.a libm.a libmach.a \
			  libmp.a libpthreads.a libresolv.a \
			  librpc.a librpcsvc.a libtermcap.a libtermlib.a \
			  libtli.a libutil.a libxproc.a libxti.a liby.a \
			  yaccpar libsys5.a libsys5.so libaud.a crt0.o \
			  libsecurity.a libsys5_r.a libstor.a \
			  libretired_malloc.a libcdrom.a libmld.a

SYMLINKDIR1		= ../ccs/lib
SYMLINKDIR2		= /usr/lib

crt0.o_SYMLINKDIR1	= cmplrs/cc
crt0.o_SYMLINKDIR2	= /usr/ccs/lib

libsys5.a_SYMLINKDIR1	= ../opt/s5/usr/lib
libsys5.so_SYMLINKDIR1	= ../opt/s5/usr/shlib
libsys5.so_SYMLINKDIR2	= /usr/shlib

libsys5_r.a_SYMLINKDIR1	= ../opt/s5/usr/lib

libmld.a_SYMLINKDIR1	= ../ccs/lib/cmplrs/cc${REL}

DATAFILES		= ncform nrform yaccpar

ILIST			= ncform nrform yaccpar
IDIR			= /usr/ccs/lib/
ncform_IMODE		= 644
nrform_IMODE		= 644
yaccpar_IMODE		= 644

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/subdirs.mk
include ${MAKEFILEPATH}/datafiles.mk
