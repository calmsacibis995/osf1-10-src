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
#       @(#)$RCSfile: Makefile,v $ $Revision: 4.3.4.6 $ (DEC) $Date: 1993/01/12 22:49:40 $ */
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

DATAFILES		= allocator.h as1atof.h binasm.h ddinfo.h \
				dec.h dpi.h exit.h \
			  f_errno.h feedback.h fio.h gptable.h host.h \
			  mednat.h mipspascal.h newinst.h prof_header.h \
			  psearch.h rcodes.h rmt_ptrace.h stext.h stinfc.h \
			  stsupport.h synonyms.h u_outformat.h ucode.h uini.h \
			  uoptions.h uread.h uscan.h usys.h uwri.h \
			  demangle_string.h definitions.h \
			  internal_symbol.h mangled_name.h utilities.h

INCLUDES		= ${DATAFILES}

ILIST			= ${INCLUDES}
IDIR			= /usr/include/cmplrs/
IMODE			= 444

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/datafiles.mk
