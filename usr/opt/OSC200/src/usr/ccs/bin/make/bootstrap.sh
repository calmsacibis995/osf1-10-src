#!/bin/sh
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
#	@(#)bootstrap.sh	3.1	(ULTRIX/OSF)	2/26/91
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
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0
#
#
# ALPHA cross-development rebuild directions:
#
# 1. do a "workon" to get a lot of environment variables set-up
# 2. setenv COMP_HOST_ROOT /
# 3. setenv COMP_TARGET_ROOT /
# 4. setenv ARCHIVE_FORMAT COFF
# 5. setenv MACHINE alpha
# 6. sh bootstrap.sh
# 7. cp make <build environment tools directory>
#            normally /usr/sde/tools/pmax_ultrix/bin/.
#
if [ "${NO_VFPRINTF}" = "NO_VFPRINTF" ]
then
    MISC_DEFS=-DNO_VFPRINTF
else
    MISC_DEFS=
fi
LIBS_DEFS=-D${ARCHIVE_FORMAT}
MAIN_DEFS=-DMACHINE=\"${MACHINE}\"
RULES_DEFS="-Dtarget_MACHINE=\"TARGET_MACHINE=${TARGET_MACHINE}\" -Dtarget_machine=\"target_machine=${target_machine}\" -Dthis_CPUTYPE=\"CPUTYPE=${CPUTYPE}\" -Dthis_cputype=\"cputype=${cputype}\""
CFLAGS="${HOST_FLAGS} -D_BLD -D_BSD -O"
LDFLAGS="${HOST_LDFLAGS}"
${CC=cc} -c ${CFLAGS} ${MAIN_DEFS} main.c
${CC} -c ${CFLAGS} doname.c
${CC} -c ${CFLAGS} ${MISC_DEFS} misc.c
${CC} -c ${CFLAGS} files.c
${CC} -c ${CFLAGS} dirs.c
${CC} -c ${CFLAGS} ${LIBS_DEFS} libs.c
${CC} -c ${CFLAGS} ${RULES_DEFS} rules.c
${CC} -c ${CFLAGS} dosys.c
yacc gram.y
mv y.tab.c gram.c
${CC} -c ${CFLAGS} gram.c
${CC} -c ${CFLAGS} dyndep.c
${CC} -c ${CFLAGS} environment.c
${CC} -c ${CFLAGS} rewinddir.c
${CC} ${LDFLAGS} main.o doname.o misc.o files.o dirs.o libs.o rules.o dosys.o gram.o dyndep.o environment.o rewinddir.o ${LIBS} -o make
