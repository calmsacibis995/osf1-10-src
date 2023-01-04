.\" (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
.\" ALL RIGHTS RESERVED
.\" OSF/1 Release 1.0
.H 1 "Functional Overview"
The Logical Volume Manager is an optional software component that is
available to manage disk storage in the OSF Operating System.
.P
The LVM provides block-oriented storage to any
other software in the system that might require it.
As a block device, the programming interface to the LVM for ordinary
applications is identical to any other block device:
a filesystem may be mounted on it, or read and write requests
can be issued directly to the device.
The block devices implemented by the LVM are called Logical Volumes.
The block devices used by the LVM to store data are called Physical
Volumes.
A collection of Physical Volumes managed together is referred to
as a Volume Group.
.so lvm_layering.pic
The storage provided by the volume group may be used by the virtual memory
system, a filesystem,
or an application that accesses a "raw" block device.
.P
Using the LVM to manage disk storage has the following advantages:
.BL
.LI
Logical volumes may be larger than the underlying disk drives.
.LI
Logical volumes may expand or contract under control of the system
administrator.
.LI
Data within a logical volume may be replicated for reliability.
.LI
Bad blocks may be detected, relocated, and optionally repaired.
.LE 1
The LVM is not intended to address replicated 
copies of data in a networked or distributed environment.
In order for such a system to function properly, issues of coherency
and simultaneous write access through multiple channels must be
dealt with that do not exist in the non-distributed case.
For distributed replication of data, algorithms at a semantic
level (file system, or database manager) seem more appropriate.
.P
From an administrative program standpoint, the LVM has a programming
interface definition that allows the administrator to manage the
storage.
.H 2 "Disk Drive Spanning"
Logical volumes defined by the LVM are not constrained to
reside on a single disk drive.
.P
This allows filesystems (and hence, files) to be created that
are larger than the underlying disk drives used for the
storage.
Administrators should be aware that if a logical volume
spans more than one drive, the probability of failure
.I increases .
Mirroring, discussed below, is one technique used to counteract
this increased risk.
.H 2 "Variable Size Volumes"
A given logical volume may be extended or reduced under control 
of the system administrator. 
This allows the admistrator to more flexibly allocate 
disk resources.
.P
Note that reducing a logical volume necessarily involves
discarding some amount of data.
It is unlikely that a Berkeley or System V filesystem will
be able to be reduced without dumping and restoring the data.
.P
Extending a logical volume will not have an immediate effect on
an unmodified UNIX filesystem.
For filesystems that are used for paging or as temporary storage,
the administrator can perform a
.I newfs (8)
during the next reboot to make the space available.
.P
Conceptually, modifying the Berkeley Fast File System to expand
dynamically is not difficult.
The LVM provides the underlying capability, but it up to
the filesystem code to take advantage of it.
.H 2 "Mirroring"
.IX "disk" "mirroring"
Data stored in a logical block may be
.B "mirrored"
(physically replicated) for high-availability, high performance, or both.
If the system maintains two copies of the data, it is said to be
.B "singly-mirrored" ;
if there is a total of three copies, it is
.B "doubly mirrored" .
.P
The replication of data within a logical volume is an
optional capability under the control of the system administrator.
This should not be confused with the replication of the internal volume group 
descriptor data, which is controlled completely by the Logical Volume
Manager.
.H 3 "Mirroring Disks for High Availability"
Data stored on on a disk drive may become unavailable for one
of several reasons
.BL 
.LI
Media defects
.LI
Catastrophic drive failure (head crash or similar)
.LI
Power supply failure
.LI
Controller failure
.LE 1
By replicating data, the LVM is able to transparently recover from the loss of
one copy of the data.
When the LVM is unable to access one copy, it
redirects I/O intended for the missing data to a secondary or tertiary copy.
Doubly mirroring data allows
data to be continuously available and modifiable even in
the face of a failure of one copy of that data.
.P
Media defects are handled though a combination of bad block relocation
and mirroring.
.P
Drive and power supply failures are handled by mirroring.
In order for this to be most effective, care must be excercised by the
adminstrator when configuring the volume group.
.P
Controller failures can be handled to the extent that they do not
operate incorrectly, but simply fail to operate.
For example, if a controller's EDC (error detection and correction) logic
reports errors that do not exist, or if it writes data to the drive
that then cannot be read, then the LVM will react as with media failures.
If the controller fails in a way that corrupts memory, writes data to
the wrong disk block, returns incorrect data on read without indicating
an error, or any of a long list of other failures, there is little 
chance that the LVM will be able to recover.
.P
For non-mirrored configurations, a reasonable policy is to keep volume
groups small and not split them across busses, adapters and power
supplies.
This reduces the number of failure modes that can cause
partitioning of the volume group.
By keeping the group small, the
probablilty of a drive electronics failure remains fairly low.
The probability that one of N drives fails is approximately N times the
failure probability of a single drive.
A volume group containing
three disks that are each 99.9% available will have about 99.7%
availability.
.P
For high-availability configurations, mirroring should be used.
Extents should be allocated so that mirrors are accessed via
separate adapters and buses and run off separate DC power supplies.
Because mirroring improves the availability, larger volume groups
become practical in these configurations.
For these systems, on-line
mirror resynchronization is an important requirement; the machine
should never reboot because of any single failure.
.H 3 "Mirroring Disks for High Performance"
Current time-sharing computing systems tend to have
a significantly higher proportion of disk I/O for reads rather than for writes.
In this case, keeping multiple copies of a block may allow for a
higher-performance computing system than would be attainable without
keeping multiple copies of a data item.
.P
Higher performance may be achieved as follows:
.BL
.LI
Read Block:
.P
The copy of the mirrored block that costs the least to retrieve is
scheduled to be read.
(The "cost" function for disks is typically a
function of seek time and rotational latency).
.LI
Write Block:
.P
All copies of the mirrored block are scheduled to be written whenever
practical.
The logical block is not considered to be written until the
last copy of the logical block has been written.
At worst, the longest write will take the worst case time for writing
a single block.
.LE 1
If the frequency of reading a logical block
outweighs the frequency of writing a logical block, mirroring data can
provide (statistically) higher performance than not mirroring data.
.P
The advantage gained from having multiple copies of data to be read
is highly application dependent.
For example, the anonymous paging space (swap partition or paging
files) probably does not have a significantly higher percentage of I/O
for read than write: in this case, mirroring will not result in
a performance gain.
It might even result in a performance loss due to the increased I/O
traffic caused by mirroring.
A filesystem that is used for system executables might show a significant
performance gain: nearly 100% of the I/O transactions are reads.
.H 2 "Bad Block Relocation"
The LVM manages the replacement of defective disk blocks within
the volume group.
The techniques used are under control of the system administrator.
.P
When the LVM detects a soft (correctable) read error, it can rewrite the
data to the the physical drive, potentially correcting the error.
Recurring soft errors will be treated as a hard error.
.P
When a hard (uncorrectable) error occurs, the LVM
can relocate the block using a pool of data blocks that it
maintains for this purpose.
Future I/O is directed to the new block.
If mirroring is in use, then the LVM redirects the failed read to another
copy of the data, and then rewrites the relocated bad block.
This procedure causes the failed block to be completely invisible
to the application layer, and restores full mirror replication.
If no alternate copy of the data is available, then the LVM 
returns a read error.
.P
The LVM will also cooperate with smart disk controllers and drivers
to perform hardware bad block relocation: prior to performing software 
relocation, the physical disk drive is asked to attempt to relocate
and rewrite the defective block. 
If this procedure succeeds, the disk drive again appears defect-free,
and the LVM removes it's copy of the defect record.
