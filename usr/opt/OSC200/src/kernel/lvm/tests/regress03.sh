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
#	@(#)regress03.sh	3.1	(ULTRIX/OSF)	2/26/91
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
# Test 03:
#	Test the creation, access, and deletion of a mirrored
#	logical volume with a missing physical volume.
set -x
PATH=.:$PATH
VOLGROUP=vg0
DEVNO=16
PVOLS="rz2g rz3g rz4g"
LVOLNO=1

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
vgcreate ${GROUP} ${pvols}
vgquery ${GROUP}
vgdeactivate ${GROUP}
pvremove ${GROUP} 2
vgactivate ${GROUP}

# setup done
LVOL=/dev/${VOLGROUP}/rlvol${LVOLNO}

echo "Creating & initializing LVOL"
lvmk -mm -s 16 ${GROUP} ${LVOL}
echo "Mirror 2 should be missing"
lvquerymap ${LVOL}

( dudley -w -p 0xdeadbeef ${LVOL} || true )
echo "Mirror 2 should be missing & stale"
lvquerymap ${LVOL}

echo "Verifying that resync of missing PVOL does nothing"
resync ${LVOL}
echo "Mirror 2 should still be missing & stale"
lvquerymap ${LVOL}

vgdeactivate ${GROUP}
pvremove ${GROUP} 1
pvremove ${GROUP} 0
vgactivate ${GROUP} ${pvols}
echo "Mirror 2 should be stale, none missing"
lvquerymap ${LVOL}
resync -lx 7 ${LVOL}
echo "Extent 7 should be completely fresh"
lvquerymap ${LVOL}

echo "Delete of active pvol should fail"
( pvdelete ${GROUP} 2 || true )

echo "Deleting LVOL and Volume Group"
lvdelete ${GROUP} ${LVOLNO}
pvdelete ${GROUP} 2
pvdelete ${GROUP} 1
pvdelete ${GROUP} 0
vgquery ${GROUP}
