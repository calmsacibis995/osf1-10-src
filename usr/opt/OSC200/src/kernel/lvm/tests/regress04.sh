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
#	@(#)regress04.sh	3.1	(ULTRIX/OSF)	2/26/91
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
# Test 04:
#	Verify that attaching a PVOL to an active volume group works
#	correctly.
set -x
PATH=.:$PATH
VOLGROUP=vg0
DEVNO=16
PVOLS="rz2g rz3g rz4g"
LVOLNO=1

npvols=0
pvols=
pvolnums=
for pvol in ${PVOLS}
do
	mkpv /dev/r${pvol}
	lastpvol=/dev/${pvol}
	pvols="${pvols} ${lastpvol}"
	pvolnums="${pvolnums} ${npvols}"
	npvols=`expr ${npvols} + 1`
done
GROUP=/dev/${VOLGROUP}/group
if [ ! -d /dev/${VOLGROUP} ]
then
	mkdir /dev/${VOLGROUP}
fi
rm -f ${GROUP}
mknod ${GROUP} c ${DEVNO} 0

set -e
vgcreate ${GROUP} ${pvols}
vgquery ${GROUP}
vgdeactivate ${GROUP}
pvremove ${GROUP} `expr ${npvols} - 1`
vgactivate ${GROUP}

# setup done
LVOL=/dev/${VOLGROUP}/rlvol${LVOLNO}

echo "Creating & initializing LVOL"
lvmk -mm -s 16 ${GROUP} ${LVOL}
echo "Mirror 2 should be missing"
lvquerymap ${LVOL}

echo "The hard part: write to lvol & reattach the missing pvol"

echo "Initiating writes"
( dudley -w -p 0x0 ${LVOL} || true ) &
sleep 20; pvattach ${GROUP} ${lastpvol}
wait

echo "Mirror 2 should be stale to some point, and then fresh beyond"
msck -v ${LVOL}

echo "Cleaning up"
lvdelete ${GROUP} ${LVOLNO}
for pvnum in ${pvolnums}
do
	pvdelete ${GROUP} ${pvnum}
done
vgquery ${GROUP}
