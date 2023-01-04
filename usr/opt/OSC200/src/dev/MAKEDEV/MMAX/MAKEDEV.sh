#! /bin/sh
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
#	@(#)MAKEDEV.sh	3.1	(ULTRIX/OSF)	2/26/91
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

umask 77

#standard devices

# console must be made in init to get single user shell.

echo -n "Making devices:  "
echo -n "standard "
#null and memory devices - don't change groups yet
mknod mem		c 3 0 ; chmod 640 mem ; chgrp kmem mem
mknod kmem		c 3 1 ; chmod 640 kmem ; chgrp kmem kmem
mknod null		c 3 2 ; chmod 666 null
mknod tty		c 2 0 ; chmod 666 tty
mknod bbram	c 14 0; chmod 644 bbram
mknod klog		c 15 0; chmod 600 klog 

umask 0
#tapes - There's only one tape drive, but the minor numbers are
#	used for magic stuff : 4 - Don't rewind after close 
#	8 - high density.  This was changed in mmaxio/ms_dev.h
#	from 1 and 2 respectively.
echo -n "tape "
mknod mt0		b 1 0
mknod mt4		b 1 4
mknod mt8		b 1 8
mknod mt12		b 1 12
mknod rmt0		c 8 0
mknod rmt4		c 8 4
mknod rmt8		c 8 8
mknod rmt12	c 8 12

umask 2
#disks - These seem to have 6 partitions.  The minor field is split 4 and
# 4, so the unit 1 minor numbers start at 16
echo -n "disks "
mknod md0a		b 0 0
mknod md0b		b 0 1
mknod md0c		b 0 2
mknod md0d		b 0 3
mknod md0e		b 0 4
mknod md0f		b 0 5
mknod md0g         b 0 6
mknod md0h         b 0 7
mknod md0i         b 0 8
mknod md0j         b 0 9
mknod md0k         b 0 10
mknod md0l         b 0 11
mknod md0m         b 0 12
mknod md0n         b 0 13
mknod md0o         b 0 14
mknod md0p         b 0 15

mknod md1a		b 0 16
mknod md1b		b 0 17
mknod md1c		b 0 18
mknod md1d		b 0 19
mknod md1e		b 0 20
mknod md1f		b 0 21
mknod md1g         b 0 22
mknod md1h         b 0 23
mknod md1i         b 0 24
mknod md1j         b 0 25
mknod md1k         b 0 26
mknod md1l         b 0 27
mknod md1m         b 0 28
mknod md1n         b 0 29
mknod md1o         b 0 30
mknod md1p         b 0 31

mknod md2a         b 0 32
mknod md2b         b 0 33
mknod md2c         b 0 34
mknod md2d         b 0 35
mknod md2e         b 0 36
mknod md2f         b 0 37
mknod md2g         b 0 38
mknod md2h         b 0 39
mknod md2i         b 0 40
mknod md2j         b 0 41
mknod md2k         b 0 42
mknod md2l         b 0 43
mknod md2m         b 0 44
mknod md2n         b 0 45
mknod md2o         b 0 46
mknod md2p         b 0 47

mknod md3a         b 0 48
mknod md3b         b 0 49
mknod md3c         b 0 50
mknod md3d         b 0 51
mknod md3e         b 0 52
mknod md3f         b 0 53
mknod md3g         b 0 54
mknod md3h         b 0 55
mknod md3i         b 0 56
mknod md3j         b 0 57
mknod md3k         b 0 58
mknod md3l         b 0 59
mknod md3m         b 0 60
mknod md3n         b 0 61
mknod md3o         b 0 62
mknod md3p         b 0 63

mknod rmd0a	c 4 0
mknod rmd0b	c 4 1
mknod rmd0c	c 4 2
mknod rmd0d	c 4 3
mknod rmd0e	c 4 4
mknod rmd0f	c 4 5
mknod rmd0g        c 4 6
mknod rmd0h        c 4 7
mknod rmd0i        c 4 8
mknod rmd0j        c 4 9
mknod rmd0k        c 4 10
mknod rmd0l        c 4 11
mknod rmd0m        c 4 12
mknod rmd0n        c 4 13
mknod rmd0o        c 4 14
mknod rmd0p        c 4 15

mknod rmd1a	c 4 16
mknod rmd1b	c 4 17
mknod rmd1c	c 4 18
mknod rmd1d	c 4 19
mknod rmd1e	c 4 20
mknod rmd1f	c 4 21
mknod rmd1g        c 4 22
mknod rmd1h        c 4 23
mknod rmd1i        c 4 24
mknod rmd1j        c 4 25
mknod rmd1k        c 4 26
mknod rmd1l        c 4 27
mknod rmd1m        c 4 28
mknod rmd1n        c 4 29
mknod rmd1o        c 4 30
mknod rmd1p        c 4 31

mknod rmd2a        c 4 32
mknod rmd2b        c 4 33
mknod rmd2c        c 4 34
mknod rmd2d        c 4 35
mknod rmd2e        c 4 36
mknod rmd2f        c 4 37
mknod rmd2g        c 4 38
mknod rmd2h        c 4 39
mknod rmd2i        c 4 40
mknod rmd2j        c 4 41
mknod rmd2k        c 4 42
mknod rmd2l        c 4 43
mknod rmd2m        c 4 44
mknod rmd2n        c 4 45
mknod rmd2o        c 4 46
mknod rmd2p        c 4 47

mknod rmd3a        c 4 48
mknod rmd3b        c 4 49
mknod rmd3c        c 4 50
mknod rmd3d        c 4 51
mknod rmd3e        c 4 52
mknod rmd3f        c 4 53
mknod rmd3g        c 4 54
mknod rmd3h        c 4 55
mknod rmd3i        c 4 56
mknod rmd3j        c 4 57
mknod rmd3k        c 4 58
mknod rmd3l        c 4 59
mknod rmd3m        c 4 60
mknod rmd3n        c 4 61
mknod rmd3o        c 4 62
mknod rmd3p        c 4 63

chmod 640 md0[a-p] md1[a-p] md2[a-p] md3[a-p] rmd0[a-p] rmd1[a-p] rmd2[a-p] rmd3[a-p] 
chgrp kmem md0[a-p] md1[a-p] md2[a-p] md3[a-p] rmd0[a-p] rmd1[a-p] rmd2[a-p] rmd3[a-p] 
chown sys md0[a-p] md1[a-p] md2[a-p] md3[a-p] rmd0[a-p] rmd1[a-p] rmd2[a-p] rmd3[a-p] 
umask 77

# real ttys -- the scc has 4 serial lines, note that tty00 is the console
echo -n "ttys "
mknod tty00	c 1 0
mknod tty01	c 1 1
mknod tty02	c 1 2
mknod tty03	c 1 3

umask 0
#ptys -- There are 80 of these, in two groups, each with master and slave
echo -n "ptys "
mknod ttyp0	c 6 0
mknod ptyp0	c 7 0
mknod ttyp1	c 6 1
mknod ptyp1	c 7 1
mknod ttyp2	c 6 2
mknod ptyp2	c 7 2
mknod ttyp3	c 6 3
mknod ptyp3	c 7 3
mknod ttyp4	c 6 4
mknod ptyp4	c 7 4
mknod ttyp5	c 6 5
mknod ptyp5	c 7 5
mknod ttyp6	c 6 6
mknod ptyp6	c 7 6
mknod ttyp7	c 6 7
mknod ptyp7	c 7 7
mknod ttyp8	c 6 8
mknod ptyp8	c 7 8
mknod ttyp9	c 6 9
mknod ptyp9	c 7 9
mknod ttypa	c 6 10
mknod ptypa	c 7 10
mknod ttypb	c 6 11
mknod ptypb	c 7 11
mknod ttypc	c 6 12
mknod ptypc	c 7 12
mknod ttypd	c 6 13
mknod ptypd	c 7 13
mknod ttype	c 6 14
mknod ptype	c 7 14
mknod ttypf	c 6 15
mknod ptypf	c 7 15

mknod ttyq0	c 6 16
mknod ptyq0	c 7 16
mknod ttyq1	c 6 17
mknod ptyq1	c 7 17
mknod ttyq2	c 6 18
mknod ptyq2	c 7 18
mknod ttyq3	c 6 19
mknod ptyq3	c 7 19
mknod ttyq4	c 6 20
mknod ptyq4	c 7 20
mknod ttyq5	c 6 21
mknod ptyq5	c 7 21
mknod ttyq6	c 6 22
mknod ptyq6	c 7 22
mknod ttyq7	c 6 23
mknod ptyq7	c 7 23
mknod ttyq8	c 6 24
mknod ptyq8	c 7 24
mknod ttyq9	c 6 25
mknod ptyq9	c 7 25
mknod ttyqa	c 6 26
mknod ptyqa	c 7 26
mknod ttyqb	c 6 27
mknod ptyqb	c 7 27
mknod ttyqc	c 6 28
mknod ptyqc	c 7 28
mknod ttyqd	c 6 29
mknod ptyqd	c 7 29
mknod ttyqe	c 6 30
mknod ptyqe	c 7 30
mknod ttyqf	c 6 31
mknod ptyqf	c 7 31

mknod ttyr0	c 6 32
mknod ptyr0	c 7 32
mknod ttyr1	c 6 33
mknod ptyr1	c 7 33
mknod ttyr2	c 6 34
mknod ptyr2	c 7 34
mknod ttyr3	c 6 35
mknod ptyr3	c 7 35
mknod ttyr4	c 6 36
mknod ptyr4	c 7 36
mknod ttyr5	c 6 37
mknod ptyr5	c 7 37
mknod ttyr6	c 6 38
mknod ptyr6	c 7 38
mknod ttyr7	c 6 39
mknod ptyr7	c 7 39
mknod ttyr8	c 6 40
mknod ptyr8	c 7 40
mknod ttyr9	c 6 41
mknod ptyr9	c 7 41
mknod ttyra	c 6 42
mknod ptyra	c 7 42
mknod ttyrb	c 6 43
mknod ptyrb	c 7 43
mknod ttyrc	c 6 44
mknod ptyrc	c 7 44
mknod ttyrd	c 6 45
mknod ptyrd	c 7 45
mknod ttyre	c 6 46
mknod ptyre	c 7 46
mknod ttyrf	c 6 47
mknod ptyrf	c 7 47

mknod ttys0	c 6 48
mknod ptys0	c 7 48
mknod ttys1	c 6 49
mknod ptys1	c 7 49
mknod ttys2	c 6 50
mknod ptys2	c 7 50
mknod ttys3	c 6 51
mknod ptys3	c 7 51
mknod ttys4	c 6 52
mknod ptys4	c 7 52
mknod ttys5	c 6 53
mknod ptys5	c 7 53
mknod ttys6	c 6 54
mknod ptys6	c 7 54
mknod ttys7	c 6 55
mknod ptys7	c 7 55
mknod ttys8	c 6 56
mknod ptys8	c 7 56
mknod ttys9	c 6 57
mknod ptys9	c 7 57
mknod ttysa	c 6 58
mknod ptysa	c 7 58
mknod ttysb	c 6 59
mknod ptysb	c 7 59
mknod ttysc	c 6 60
mknod ptysc	c 7 60
mknod ttysd	c 6 61
mknod ptysd	c 7 61
mknod ttyse	c 6 62
mknod ptyse	c 7 62
mknod ttysf	c 6 63
mknod ptysf	c 7 63

mknod ttyt0	c 6 64
mknod ptyt0	c 7 64
mknod ttyt1	c 6 65
mknod ptyt1	c 7 65
mknod ttyt2	c 6 66
mknod ptyt2	c 7 66
mknod ttyt3	c 6 67
mknod ptyt3	c 7 67
mknod ttyt4	c 6 68
mknod ptyt4	c 7 68
mknod ttyt5	c 6 69
mknod ptyt5	c 7 69
mknod ttyt6	c 6 70
mknod ptyt6	c 7 70
mknod ttyt7	c 6 71
mknod ptyt7	c 7 71
mknod ttyt8	c 6 72
mknod ptyt8	c 7 72
mknod ttyt9	c 6 73
mknod ptyt9	c 7 73
mknod ttyta	c 6 74
mknod ptyta	c 7 74
mknod ttytb	c 6 75
mknod ptytb	c 7 75
mknod ttytc	c 6 76
mknod ptytc	c 7 76
mknod ttytd	c 6 77
mknod ptytd	c 7 77
mknod ttyte	c 6 78
mknod ptyte	c 7 78
mknod ttytf	c 6 79
mknod ptytf	c 7 79
umask 77
echo "."
echo "DONE making devices"
