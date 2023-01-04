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
#	@(#)regress01.sh	3.1	(ULTRIX/OSF)	2/26/91
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
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
# OSF/1 Release 1.0
#
# Regression tests.
# Test 01:
#	Test the creation of a volume group, and successive activates and
#	deactivates, with and without all PV's attached.
set -x
PATH=.:$PATH
VOLGROUP=vg0
DEVNO=16
PVOLS="rz2g rz3g rz4g"

pvols=
for pvol in ${PVOLS}
do
	mkpv /dev/r${pvol}
	pvols="${pvols} /dev/${pvol}"
done
GROUP=/dev/${VOLGROUP}/group
if [ ! -d /dev/${VOLGROUP} ]
then
	mkdir /dev/${VOLGROUP}
fi
rm -f ${GROUP}
mknod ${GROUP} c ${DEVNO} 0

set -e
echo "Testing Volume Group creation"
vgcreate ${GROUP} ${pvols}
vgquery ${GROUP}
vgdeactivate ${GROUP}

echo "Testing reactivate"
vgactivate ${GROUP}
vgdeactivate ${GROUP}

echo "Testing activation w/o 1 PVOL"
pvremove ${GROUP} 2
vgactivate ${GROUP}
vgdeactivate ${GROUP}

echo "Reactivating"
pvremove ${GROUP} 1
pvremove ${GROUP} 0
vgactivate ${GROUP} ${pvols}
vgquery ${GROUP}

echo "Deleting Volume Group"
pvdelete ${GROUP} 0
pvdelete ${GROUP} 1
pvdelete ${GROUP} 2
vgquery ${GROUP}
