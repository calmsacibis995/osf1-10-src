.\" (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
.\" ALL RIGHTS RESERVED
.\" OSF/1 Release 1.0
.H 1 "Limits"
The limits defined in the following two sections apply to
physical and logical volumes and their constituents.
Many of these limits are listed in the
.I "lvm.h"
include file.
.IX "disk" "limits"
.IX "logical volume" "limits"
.IX "physical volume" "limits"
.IX "limits" "architectural"
.H 2 "Architectural Limits"
This section describes architectural limits on disk entities and
their characteristics.
An architectural limit is a limit imposed by the on-disk data structures that
would cause incompatibilities with future versions if it were 
changed.
.P
The following limits apply, and the data structure items that cause
each limit are:
.BL
.LI
Physical volumes per volume group:  1 to 255.
.VL \n(Li
.LI
limiting factor: 8-bit pv_num field in the PV header.
The practical limit is much lower.
.LE 1
.LI
Physical extents per physical volume:  1 to 65535 (LVM_MAXPXS).
(0 - 64GB @ 1MB/extent).
.VL \n(Li
.LI
limiting factor: 16 bit px_count in PV Header.
.LE 1
.LI
Physical extent size:  2**n bytes, 17 <= n <= 28. (128K to 256 MB).
.VL \n(Li
.LI
limiting factor: unknown. This number is stored as the exponent
(17 to 28) in the VG Header, in a 16-bit field.
.LE 1
.LI
Physical block size: minimum 512 Bytes
.VL \n(Li
.LI
limiting factor: all data structures are defined to begin on 512-byte
boundaries.
It appears that this might also be a maximum.
.LE
.LI
Logical volumes per volume group:  0 to 65535.
.VL \n(Li
.LI
limiting factor: 16-bit unsigned numbers (maxlvs, numlvs) in the VG header,
16-bit unsigned number (lv_index) in the physical extent map entry, 
16-bit unsigned number (lv_minor) in the mirror consistency record.
.LE 1
.LI
Logical extents per logical volume:  0 to 65535.  (0 to 64GB @ 1MB/extent)
.VL \n(Li
.LI
limiting factor: 16 bit (lx_num) field in the physical extent map
entry, 16 bit (maxsize, num_lxs) fields in the LV Header.
.LE 1
.LI
Logical extent size:  Must equal physical extent size.
.LI
Logical block size:  Must equal physical block size.
.LI
Number of software-relocated defects per physical volume: 3519 sectors
.VL \n(Li
.LI
limiting factor: size of the bad block directory in the physical volume
reserved area.
.LE 1
.LE 1
.H 2 "Implementation Limits"
.IX "limits" "implementation"
An implementation limit is a limit imposed by the device driver interfaces,
in-memory data structures, or the operating system, that could
theoretically be changed without modifying the on-disk formats.
The above architecture limits are also the implementation limits
except in the following cases where the specified implementation
limits are imposed:
.BL
.LI
Physical volumes per volume group: 1 to 32*N, (variable), selected by
administrator.
.VL \n(Li
.LI
limiting factor: this comes from the Volume Group Status Area data
structures.
The status area must be sized by the system administrator during initial
volume group creation, and this places an upper bound on the maximum number
of physical volumes in the group.
.LE 1
.LI
Physical extents per physical volume: variable, selected by administrator.
.VL \n(Li
.LI
limiting factor: this comes from the Volume Group Status Area data
structures.
The typical limit is on the order of 1000's of physical extents per
physical volume.
The status area must be sized by the system administrator during initial
volume group creation, and this places an upper bound on the maximum size
of any physical volume in the group.
.LE 1
.LI
Physical extent size:  2**n bytes, 20 <= n <= 28.
.VL \n(Li
.LI
limiting factor: unknown.
.LE 1
.LI
Logical volumes per volume group:  0 to 255 (LVM_MAXLVS).
.VL \n(Li
.LI
limiting factor: number of bits in the minor device number of the
.B dev_t 
data type, currently 8 bits.
.LE 1
.LI
Logical volume size:  2GB (2**31 bytes)
.VL \n(Li
.LI
limiting factor: the
.B off_t
data type used to specify the seek location to the
.I lseek (2)
system call.
This only affects the raw device interface (read/write).
The block device (driver) interface uses
.B daddr_t
(32 bit disk sector number) to specify I/O requests.
This limit is (2**32)*(2**9) bytes (2TB).
.LE 1
.LI
Physical extents per logical extent: 0 to 3 (LVM_MAXCOPIES)
.VL \n(Li
.LI
limiting factor: internal algorithms and data structures depend
on encoding the mirror number in a 2-bit field.
.LE 1
.LE 1
