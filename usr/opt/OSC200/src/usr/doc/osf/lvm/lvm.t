.\" (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
.\" ALL RIGHTS RESERVED
.\" OSF/1 Release 1.0
.if t .nr O .6i
.if t .nr W 6.5i
.if t .nr S 11
.if t .nr N 4
.if t .nr L 10.8i
'so /usr/lib/tmac/tmac.m
.so fontmacros.m
.so picmacros.m
.so copyright.m
.nr Cl 3
.nr Ej 1
.nr Hb 6
.nr Hc 0
.nr Hs 4
.nr Hu 5
.ds HF  3  3  3  3  3  2  2
.ds HP 14 13 13 12 11 11 11
.ND "August 8, 1990"
.PH "'Revision 1.0'Logical Volume Manager Design'\\\\*(DT'"
.PF "''-\\\\nP-''"
.TL
Logical Volume Manager Design
.AF "Open Software Foundation"
.\" .AU "LVM Design Team"
.AU "Jeff Carter"
.AU "Jeff Collins"
.AU "Scott Hankin"
.AS 1 10
This document describes the scheme that
the OSF/1 Operating System uses to represent the characteristics of and
manage disk storage devices.
This subsystem is referred to as the Logical Volume Manager (LVM).
.AE
.MT 4
.IX "Disk" "Management"
.IX "Logical Volume" "Management"
.IX "Physical Volume" "Management"
.so overview.t
.so terminology.t
.so limits.t
.so physvol.t
.so prog_interface.t
.so operations.t
.so implementation.t
.so phys_driver_intfc.t
.CS
.CR
.TC
