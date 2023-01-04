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
# @(#)$RCSfile: Makefile,v $ $Revision: 4.3.7.5 $ (DEC) $Date: 1993/10/29 19:03:24 $
#
# (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
# ALL RIGHTS RESERVED 
#
#
# OSF/1 1.2
#

#
# Makefile to build locales on native system
#
# As root, run the command:  
#
# 	make all
#
# to build and install all locales.
#
# To build a specific local, run the command:
#
#	make <locale-name>
#
# where locale name is something like en_US.ISO8859-1
#
# NOTE: localedef *will* generate warnings for these locales.
#	The exit status of localedef is 1 when there are warnings.
#

SUBDIRS		= charmaps \
		  src \
		  iconv

LOCALES		= da_DK.ISO8859-1 \
		de_CH.ISO8859-1 \
		de_DE.ISO8859-1 \
		el_GR.ISO8859-7 \
		en_GB.ISO8859-1 \
		en_US.ISO8859-1 \
		es_ES.ISO8859-1 \
		fi_FI.ISO8859-1 \
		fr_BE.ISO8859-1 \
		fr_CA.ISO8859-1 \
		fr_CH.ISO8859-1 \
		fr_FR.ISO8859-1 \
		is_IS.ISO8859-1 \
		it_IT.ISO8859-1 \
		nl_BE.ISO8859-1 \
		nl_NL.ISO8859-1 \
		no_NO.ISO8859-1 \
		pt_PT.ISO8859-1 \
		sv_SE.ISO8859-1 \
		tr_TR.ISO8859-9

DATAFILES		= C.en Cextend.en da_DK.88591.en de_CH.88591.en \
			  de_DE.88591.en en_GB.88591.en en_US.88591.en \
			  es_ES.88591.en fi_FI.88591.en fr_BE.88591.en \
			  fr_CA.88591.en fr_CH.88591.en fr_FR.88591.en \
			  it_IT.88591.en nl_BE.88591.en \
			  nl_NL.88591.en no_NO.88591.en pt_PT.88591.en \
			  sv_SE.88591.en \
			  en_GB.88591@DEC.en \
			  fr_FR.88591@DEC.en \
			  de_DE.88591@DEC.en

CTABFILES		= C Cextend da_DK.88591 de_DE.88591 en_GB.88591 \
			  en_US.88591 es_ES.88591 fi_FI.88591 \
			  fr_CA.88591 fr_FR.88591 it_IT.88591 \
			  nl_NL.88591 no_NO.88591 \
			  pt_PT.88591 sv_SE.88591 \
			  en_GB.88591@DEC \
			  fr_FR.88591@DEC \
			  de_DE.88591@DEC

OTHERS		= ${LOCALES} ${SYMLINKS} ${CTABFILES}
IDIR		= /usr/lib/nls/loc/
ILIST		= ${LOCALES} ${SYMLINKS} ${DATAFILES} ${CTABFILES}

de_DE.88591_ILINKS	= ${IDIR}de_CH.88591
fr_FR.88591_ILINKS	= ${IDIR}fr_BE.88591 ${IDIR}fr_CH.88591
nl_NL.88591_ILINKS	= ${IDIR}nl_BE.88591

IMODE		= 644
NOSTRIP		=

CCFLAGS		= -C "-Wf,-wchar32" -C "-Wf,-unsigned_wchar" -C "${INCDIRS}"
LDFLAGS		= -L "${SHLIBDIRS}"
LOCALEDEF	= localedef -v ${CCFLAGS} ${LDFLAGS} -P "${_CC_EXEC_PREFIX_}"
_SYMLINK_	= ${${@}_SYMLINK}

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/datafiles.mk
include ${MAKEFILEPATH}/locales.mk
include ${MAKEFILEPATH}/subdirs.mk
include ${MAKEFILEPATH}/others.mk

${SYMLINKS}:	$${_SYMLINK_}
	${RM} ${_RMFLAGS_} $@
	umask 022 ; ln -s ${_SYMLINK_} $@

da_DK.ISO8859-1:	src/da_DK.src
	${LOCALEDEF} -c -f charmaps/ISO8859-1.cm -i $> $@

de_CH.ISO8859-1:	src/de_CH.src
	${LOCALEDEF} -c -f charmaps/ISO8859-1.cm -i $> $@

de_DE.ISO8859-1:	src/de_DE.src
	${LOCALEDEF} -c -f charmaps/ISO8859-1.cm -i $> $@

el_GR.ISO8859-7:	src/el_GR.src
	${LOCALEDEF} -c -f charmaps/ISO8859-7.cm -i $> $@

en_GB.ISO8859-1:	src/en_GB.src
	${LOCALEDEF} -c -f charmaps/ISO8859-1.cm -i $> $@

en_US.ISO8859-1:	src/en_US.src
	${LOCALEDEF} -c -f charmaps/ISO8859-1.cm -i $> $@

es_ES.ISO8859-1:	src/es_ES.src
	${LOCALEDEF} -c -f charmaps/ISO8859-1.cm -i $> $@

fi_FI.ISO8859-1:	src/fi_FI.src
	${LOCALEDEF} -c -f charmaps/ISO8859-1.cm -i $> $@

fr_BE.ISO8859-1:	src/fr_BE.src
	${LOCALEDEF} -c -f charmaps/ISO8859-1.cm -i $> $@

fr_CA.ISO8859-1:	src/fr_CA.src
	${LOCALEDEF} -c -f charmaps/ISO8859-1.cm -i $> $@

fr_CH.ISO8859-1:	src/fr_CH.src
	${LOCALEDEF} -c -f charmaps/ISO8859-1.cm -i $> $@

fr_FR.ISO8859-1:	src/fr_FR.src
	${LOCALEDEF} -c -f charmaps/ISO8859-1.cm -i $> $@

is_IS.ISO8859-1:	src/is_IS.src
	${LOCALEDEF} -c -f charmaps/ISO8859-1.cm -i $> $@

it_IT.ISO8859-1:	src/it_IT.src
	${LOCALEDEF} -c -f charmaps/ISO8859-1.cm -i $> $@

nl_BE.ISO8859-1:	src/nl_BE.src
	${LOCALEDEF} -c -f charmaps/ISO8859-1.cm -i $> $@

nl_NL.ISO8859-1:	src/nl_NL.src
	${LOCALEDEF} -c -f charmaps/ISO8859-1.cm -i $> $@

no_NO.ISO8859-1:	src/no_NO.src
	${LOCALEDEF} -c -f charmaps/ISO8859-1.cm -i $> $@

pt_PT.ISO8859-1:	src/pt_PT.src
	${LOCALEDEF} -c -f charmaps/ISO8859-1.cm -i $> $@

sv_SE.ISO8859-1:	src/sv_SE.src
	${LOCALEDEF} -c -f charmaps/ISO8859-1.cm -i $> $@

tr_TR.ISO8859-9:	src/tr_TR.src
	${LOCALEDEF} -c -f charmaps/ISO8859-9.cm -i $> $@
