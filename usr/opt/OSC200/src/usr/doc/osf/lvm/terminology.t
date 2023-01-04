.\" (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
.\" ALL RIGHTS RESERVED
.\" OSF/1 Release 1.0
.H 1 "Terminology"
This section explains the terminology used in this document.
.H 2 "Block"
In the Logical Volume Manager, the term
.IX "block"
.IX "disk" "block"
.B "block"
refers to a disk block, or disk sector.
It is the minimum unit of I/O to the disk drive.
A file system block (in either the Berkeley or System V File Systems)
usually consists of more than 1 disk block.
.H 3 "Physical Block"
.IX "physical block"
.IX "disk" "physical block"
A 
.B "physical block"
is a contiguous region of
a physical volume corresponding in size to a disk sector.
All physical volumes in the volume group must have the same 
physical block size.
.P
A physical block may be good or bad depending on whether
or not it is able to store a sector's worth of data without loss.
A good physical block may become bad and be incapable of storing data.
A bad physical block can be relocated and rewritten, in which case
it becomes good again.
.H 3 "Logical Block"
.IX "logical block"
.IX "disk" "logical block"
A 
.B "logical block"
is a contiguous single-sector sized
region of a logical volume.
A logical block corresponds to one, two, or
three physical blocks if the logical extent containing
the logical block is
.B "non-mirrored" ,
.B "singly-mirrored" ,
or
.B "doubly-mirrored" ,
respectively.
.H 2 "Track Group"
A
.B "track group"
is a contiguous group of disk blocks, usually corresponding to
32 pages of data.
The mirror consistency mechanism uses track groups to track in-flight
regions of the disk.
.H 2 "Extent"
.H 3 "Physical Extent"
.IX "disk" "physical extent"
.IX "physical extent"
A
.B "physical extent"
is a contiguous set of physical blocks
contained within a single physical volume.
Within a volume group, the physical extents are always the same
size: this is the basic unit of allocation used to assign space to
logical volumes.
.H 3 "Logical Extent"
.IX "logical extent"
.IX "disk" "logical extent"
A 
.B "logical extent"
is a logically contiguous
set of logical blocks within a single logical volume.
A logical extent corresponds to one, two, or three physical
extents if the logical extent is non-mirrored, singly-mirrored,
or doubly-mirrored, respectively.
.P
Within a volume group, logical extents are always the same size,
and always correspond to the size of the physical extents
for that volume group.
.H 2 "Volume"
.IX "volume"
A
.B "volume"
is the general term used throughout this document to describe
a block-oriented device.
A volume is capable of holding a file system, or serving as
backing store for paging, or may be used as direct access storage
by an application, such as a database manager.
In a traditional UNIX system, a volume corresponds to a disk
drive, or a disk partition.
.H 3 "Logical Volume"
.IX "" "logical volume"
.IX "disk" "logical volume"
A 
.B "logical volume"
is a volume that is implemented by the Logical Volume Manager.
.P
A logical volume
is a block device composed of an integral number of logical extents.
The number of logical extents within a logical volume is variable, and is
determined by the Logical Volume Manager, working in conjunction with a
system administrator.
.P
A logical volume is always contained completely within a single volume group.
.H 3 "Physical Volume"
.IX "physical volume"
The term
.B "physical volume" 
is used to distinguish a volume that the LVM uses for it's storage
from a volume that the LVM implements.
A given physical volume must be assigned to a volume group before that
physical volume may be used by the LVM.
.P
An LVM
.B "physical volume"
is composed of a physical volume descriptor
area, followed by a variable number of physical blocks that
serve as volume group descriptors, followed by the area
used by the LVM for storing the the Logical Volume data.
.H 2 "Volume Group"
.IX "volume group"
.IX "disk" "volume group"
A
.B "volume group"
is a collection of volumes that are managed by the LVM.
It may be viewed both as a collection of logical volumes (implemented
by the LVM) or as a collection of physical volumes (used by the LVM).
.P
There is no restriction on the types of disk devices (e.g. SCSI,
ESDI, IPI, ...) that may be placed in a given volume group.
.P
There is also no restriction on the sizes of physical volumes contained
within a given volume group.
All logical volume space allocation within the volume group is performed
in extent-size units.
This minimum allocatable unit affects the amount of unallocatable space
on each physical volume.
