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
#	@(#)regress02.sh	3.1	(ULTRIX/OSF)	2/26/91
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
# Test 02:
#	Test the creation, access, and deletion of a mirrored
#	logical volume
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

# setup done
LVOL=/dev/${VOLGROUP}/rlvol${LVOLNO}

echo "Creating & initializing LVOL"
lvmk -mm -s 16 ${GROUP} ${LVOL}
( dudley -w -p 0xdeadbeef ${LVOL} || true )
lvquerymap ${LVOL}

vgdeactivate ${GROUP}
pvremove ${GROUP} 2

echo "Testing reactivate w/missing PVOL and resync"
vgactivate ${GROUP}
echo "Mirror 1 should be stale, Mirror 2 should be stale & missing"
lvquerymap ${LVOL}
resync ${LVOL}
echo "Mirror 2 should be stale & missing"
lvquerymap ${LVOL}
( dudley -w -p 0x0 ${LVOL} || true )

echo "Deactivating"
vgdeactivate ${GROUP}
pvremove ${GROUP} 1
pvremove ${GROUP} 0

echo "Reactivating volume group"
vgactivate ${GROUP} ${pvols}
echo "Mirror 2 should be stale, none missing"
lvquerymap ${LVOL}
resync -lx 5 ${LVOL}
echo "Extent 5 should be fresh"
lvquerymap ${LVOL}

echo "Deleting LVOL"
lvdelete ${GROUP} ${LVOLNO}
lvquery ${GROUP} ${LVOLNO}
pvdelete ${GROUP} 2
pvdelete ${GROUP} 1
pvdelete ${GROUP} 0

echo "Volume Group deleted"
vgquery ${GROUP}
