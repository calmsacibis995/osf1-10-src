.\"
.\" (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
.\" ALL RIGHTS RESERVED
.\"
.\"
.\" OSF/1 Release 1.0
.H 2 "Logical Volume Manager Execution Model"
The Logical Volume Manager is made up of several components:
.BL
.LI
System management subroutines (liblvm.a)
.P
The 
.B "liblvm"
library contains LVM subroutines that perform system
management functions such as create a logical volume, query a volume
group, resync a physical volume, etc.
These subroutines update and
manage the volume group descriptor area defined at the beginning of
each physical volume in a volume group.
The system management
commands that deal with logical volumes are the primary users of these
subroutines.
.LI
Logical Volume Device Driver
.P
The logical volume device driver processes all logical I/O.
It
converts the logical address to a physical address, handling mirroring
and bad block relocation, and then sends the request to a physical
volume device driver.
.P
Logical volume device driver configuration is performed by the
.B "liblvm"
subroutines via the
.B "ioctl"
system call which
calls the 
.B "lv_ioctl"
entry point of the logical volume device
driver.
.LE 1
\l'\n(.lu-\n(.iu'
.so exemodel.pic
.FG "LVM Execution Model." \n(H1-
\l'\n(.lu-\n(.iu'
.P
Figure \n(H1-\n(Fg describes the relationship between the
LVM components.
.P
The Logical Volume Device Driver (LVDD) has some data structures in
the kernel that describe the volume groups that are initialized.
When these volume groups are modified by the system management subroutines
the user-level code makes LVM system calls to update the LVDD
kernel data structures.
In some cases, the Logical Volume Device
Driver must update the on-disk descriptor area or the Bad Block
Directory on cylinder 0 of a physical volume.
.H 3 "LVM Programming Interfaces"
While the LVM subroutines are not part of the kerenel, we document
them briefly here because the LVM is a new part of the OSC (and, truth
to tell, because we have the information already).
.P
The 
.B "liblvm"
library contains LVM subroutines that perform Logical
Volume Manager system management functions.
The basic function of these routines is to maintain the LVM data
structures so that LVM recovery procedures can repair any descriptor
information that becomes out-of-sync due to power failures or missing
physical volumes.
Anytime a change is made to a volume group, both
the LVDD data structures and the on-disk copy of the volume group
descriptor area are updated.
These 
.B "liblvm"
subroutines are called by the system management commands.
A brief descripton of the subroutines follow.
The interface for these subroutines is described in (?? WHAT FILE??)
Heading id'liblv' unknown -- \&.
.H 4 "lvm_createlv - Create Logical Volume"
The 
.B "lvm_createlv"
subroutine is called by a system management
command to create a logical volume in an existing volume group.
This function uses the supplied information to update a previously null
entry in the logical volume list.
The index into the list of logical
volume entries corresponds to the minor number of the logical volume.
This function does not do partition allocation.
The 
.B "lvm_extendlv"
subroutine must be used to allocate partitions for
the new logical volume.
.H 4 "lvm_changelv - Change Logical Volume"
The 
.B "lvm_changelv"
function is called by a system management
command to change attributes of an existing logical volume.
The function updates the specified logical volume's LVM data structures
and logical volume entry in the descriptor area.
.H 4 "lvm_deletelv - Delete Logical Volume"
The 
.B "lvm_deletelv"
function is called by a system management
command to delete a logical volume from its volume group.
The logical volume must not be opened when this function is called.
All logical partitions belonging to this logical volume are deallocated.
The LVM data structures and the descriptor area are updated.
.H 4 "lvm_querylv - Query Logical Volume"
The 
.B "lvm_querylv"
function is called by a system management
command to return information about a the logical volume.
It verifies
that the logical volume is valid and writes the information from the
mapped file for its volume group to the buffer supplied.
.H 4 "lvm_createvg - Create a Volume Group"
The 
.B "lvm_createvg"
function is called by a system management
command to create a new volume group and to install its first physical
volume.
It first verifies that the physical volume does not exist in
another volume group.
The LVM data structures are then updated with
new entries for the new volume group as if the volume group had been validated.
A volume group descriptor area is created and initialized
for the new volume group, and two copies of it are written to the
physical volume which is installed.
.H 4 "lvm_installpv - Install Physical Volume"
The 
.B "lvm_installpv"
function is called by a system management
command to install a physical volume into the volume group specified.
It first verifies that the physical volume does not exist in another
volume group.
A new entry is then added in the LVM data structures
and in the volume group descriptor area for this physical volume, and
a copy of the descriptor area is written to the physical volume.
.H 4 "lvm_deletepv - Delete Physical Volume"
The 
.B "lvm_deletepv"
function is called by a system management
command to delete the physical volume specified from its volume group.
The physical volume must not contain any partitions of a logical
volume for it to be deleted.
If the physical volume contains any
partitions of a logical volume, an error code is returned.
In this case, the user must delete logical volumes or relocate the partitions
that reside on this physical volume with the
.B "lvm_migratepv"
function.
For an empty physical volume, 
.B "lvm_deletepv"
removes the entries for this physical volume from the LVM data structures and
from the descriptor area, and it initializes the descriptor area on
the physical volume being deleted.
.H 4 "lvm_changepv - Change a Physical Volume"
The 
.B "lvm_changepv"
function is called by a system management
command to change certain characteristics of a physical volume.
This command can be used to change the physical volume state to temporarily
remove (and later return) a physical volume from its volume group and,
also, to disallow or re-allow allocation of partitions on the physical
volume.
Allocation from a physical volume should be disallowed if the
physical partitions of that physical volume are to be migrated to
another physical volume.
.H 4 "lvm_querypv - Query Physical Volume"
The 
.B "lvm_querypv"
function is called by a system management
command to return information on the physical volume specified.
It verifies that the physical volume is valid and writes the information
from its volume group mapped file to the buffer supplied.
.H 4 "lvm_deletevg - Delete Volume Group"
The 
.B "lvm_deletevg"
function is called by a system management
command to delete a volume group from the system.
It first verifies
that the specified volume group is varied off, and that all logical
and physical volumes in the volume group have been deleted.
If any of these are not true, an error code is returned.
Otherwise, the descriptors on all physical volumes in the volume group are
initialized.
.H 4 "lvm_queryvg - Query Volume Group"
The 
.B "lvm_queryvg"
function is called by a system management
command to return information on a volume group.
It verifies that the
specified volume group is valid and writes the information from the
volume group's mapped file to the buffer supplied.
.H 4 "lvm_queryvgs - Query All Volume Groups"
The 
.B "lvm_queryvgs"
function is called by a system managment
command to return the IDs of all volume groups on the system.
It verifies that the volume group is valid and writes the volume group
IDs from all the mapped files to the buffer supplied.
.H 4 "lvm_varyonvg - Vary On Volume Group"
The 
.B "lvm_varyonvg"
function varies a volume group on-line, that is
make the volume group accessible through the LVM device driver (??
TRUE??).
At IPL this function is called to vary on the ROOT volume
group (the volume group that contains the root filesystem.)  Once the
system is IPLed, the varyon command calls lvm_varyonvg to vary on any
other volume group.
The 
.B "lvm_varyonvg"
subroutine first tries to contact all physical volumes in the volume group.
When a volume is
declared unavailable, the fact is recorded in the descriptor area of
remaining volumes in the volume group (HOW?).
The timestamp value in the
descriptor area's header and trailer are used during recovery to
detect the fact that the descriptor area of a previously missing
volume may be stale (HOW?).
Once the physical volumes are varied on,
.B "lvm_varyonvg"
verifies the consistency of the descriptor area on
each physical volume in the volume group and any mirrored data, and
then corrects any inconsistencies, if possible (HOW?).
It also validates the bad block directory on cylinder 0 of each physical volume.
.P
After all volumes in the volume group are validated, the LVM data
structures (mapped file and the LVDD kernel data structures) are
created from the descriptor area on the physical volumes.
The device switch entry for this volume group is then initialized with the
pointer to its LVDD volume group structure and the LVDD initialization
routine is called.
.H 4 "lvm_varyoffvg - Vary Off Volume Group"
The 
.B "lvm_varyoffvg"
function varies the specified volume group
off-line so that its logical volumes can no longer be accessed.
Once the volume group has been varried off, any attempt to open a logical
volume in the volume group results in and error return.
.H 4 "lvm_extendlv - Extend Logical Volume"
The
.B "lvm_extendlv"
function is called by a system management
command to extend a logical volume by a specified number of partitions
or to add mirror copies for a specified number of logical partitions.
It allocates logical partitions for the specified logical volume at
the locations specified in the array of physical volume and partition
locations given as input.
It updates the LVM data structures and the descriptor area.
.H 4 "lvm_reducelv - Reduce Logical Volume"
The 
.B "lvm_reducelv"
function is called by a system management
command to reduce a logical volume by a specified number of partitions
or to delete mirror copies for a specified number of logical
partitions.
It deallocates logical partitions for the specified
logical volume at the locations specified in the array of physical
volume and partition locations.
The LVM data structures and the descriptor area are updated.
.H 4 "lvm_resynclp - Resynchronize Logical Partition"
The
.B "lvm_resynclp"
function is called by 
.B "lvm_resynclv" .
It initiates mirror resynchronization for all physical partitions in this
logical partition that are in the LVM_STALE state.
When resynchronization is complete, these partitions will be in the
LVM_ACTIVE state.
The LVM data structures and the descriptor area are updated.
.H 4 "lvm_resynclv - Resynchronize Logical Volume"
The 
.B "lvm_resynclv"
function is called by a system management
command to synchronize a logical volume.
It calls 
.B "lvm_resynclp"
for each logical partition in the logical volume that has a physical
partition in the LVM_STALE state.
The LVM data structures and the descriptor area are updated.
.H 4 "lvm_resyncpv - Resynchronize Physical Volume"
The 
.B "lvm_resyncpv"
function is called by a system management
command to synchronize a physical volume.
For each physical partition
on the physical volume that is in the LVM_STALE state, it
resynchronizes the partition.
The LVM data structures and the descriptor area are updated.
.H 4 "lvm_migratepp - Migrate Physical Partition"
The 
.B "lvm_migratepp"
function is called to move a physical partition.
If the logical partition is triply mirrored, the function
first calls 
.B "lvm_dealloclp"
to delete the specified partition and then calls 
.B "lvm_alloclp"
to allocate a new partition, and the new physical partition is resynchronized.
If the logical partition is not triply mirrored, the function calls 
.B "lvm_alloclp"
to allocate a
new partition, the new physical partition is resynchronized, and it
calls 
.B "lvm_dealloclp"
to delete the old partition.
.H 3 "LVM System Calls"
(?? THE LVM DOES NOT HAVE ANY SPECIFIC SYSTEM CALLS; INSTEAD THE LVM
DEVICE DRIVER IS ACCESSED THROUGH NORMAL DEVICE CALLS, INCLUDING
open, close, ETC.??  IF THIS IS TRUE WHAT ABOUT THE
FOLLOWING??)
.P
The LVM system calls are used by the LVM components above the kernel
to update the LVDD kernel data structures.
The LVDD has data
structures describing each volume group that is initialized.
These data structures are built when a volume group is varied on-line from
the data in the on-disk LVM descriptor area at the beginning of each
physical volume in the volume group.
When the descriptor area is
updated by the system management functions above the kernel, the LVDD
data structures must be updated to reflect the changes in the volume
groups that are varied on.
This is done via LVM system calls - one for each of the LVDD data structures.
These system calls have
parameters that specify the specific LVDD data structure entry to be
updated and a pointer to a new data structure entry.
(See "Logical Volume Device Driver - /dev/hdn" for more 
information on the LVDD data structures.)
.H 3 "Logical Volume Device Driver - /dev/hdn"
The Logical Volume Device Driver (LVDD) is a pseudo-device driver that
implements logical volumes.
It provides character and block entry
points like the physical disk device driver, with compatible arguments.
Each volume group has a separate entry in the kernel device switch
table that contains the entry points for the device driver and a pointer
to the volume group data structure (described below).
The logical
volumes of a volume group are distinguished by their minor numbers.
\l'\n(.lu-\n(.iu'
.so ioflow.pic
.FG "Flow of I/O through the Logical Volume Device Driver" \n(H1-
\l'\n(.lu-\n(.iu'
.P
.nr !! \n(Fg
Figure \n(H1-\n(!! describes the flow of character and block requests
through the LVDD.
Character I/O requests are performed by issuing a
read or write system call on a character special file (/dev/rhd*) for
a logical volume.
The read or write system call is processed by the
file system, which calls the LVDD read or write entry point.
The read/write entry point transforms the character request
into a block request building a buffer for the request and calling the
LVDD strategy entry point.
.P
Block I/O requests are performed by issuing a read or write system
call on a block special file (/dev/hd*) for a logical volume.
These
requests go through the systemc call interface to BREAD/BWRITE block I/O
services which build buffers for the request and call the LVDD
strategy entry point.
.P
The LVDD strategy entry point then translates the logical address to a
physical address (handling mirroring and bad block relocation) and
calls the appropriate physical disk device driver.
On completion of the I/O, the physical device driver calls IODONE on the device
interrupt level.
IODONE then calls the LVDD I/O completion handling routine.
Once this is completed, the LVDD calls IODONE to notify the
requester that the I/O is completed.
.P
The LVDD is logically split into top and bottom halves.
The top half contains the open, close, read, write, ioctl,
and init entry points.
The bottom half contains the strategy entry point - block read and
write code.
This was done to isolate the code that has no access to user process context.
The bottom half of
the device driver runs on interrupt levels, and is not permitted to
assume that any process context exists.
The top half runs in the context of a process address
space.
(?IF BIO.C CALLS THE BOTTOM HALF DIRECTLY, WHO CALLS THE TOP HALF;
ONLY THOSE WHO ACTUALLY TREAT THE LVDD AS A "REGULAR DEVICE"? IN ANY
CASE, WHO?  IF ONLY FILE SYS MAYBE IT SHOULD BE IN THE PICTURE)
.H 4 "Data Structures"
.H 5 "Physical Buf structure"
The the strategy entry point has a single parameter, a pointer to one
or a list of logical buf structures.
The logical buf structure is
defined in <sys/buf.h> and contains all the information about an I/O
request that is needed, including a pointer to the data buffer.
The strategy entry point associates one or more (if mirrored) physical buf
structures (pbuf) with each logical buf structure (buf) and passes
them to the appropriate physical device driver.
The physical buf structure is defined in <sys/disk.h>.
It is a standard buf structure
with some additional fields used by the LVDD to track the status of
the physical requests that correspond to each logical I/O request.
The LVDD allocates and manages a pool of pbufs.
.P
.B "Volume Group Data Structures"
.P
The Logical Volume Device Driver uses the information in the disk
descriptors contained in a hidden logical volume (/dev/vg<major#>)
that only the LVM knows about.
When a volume group is varied on-line,
an in memory copy of the information on these descriptors, necessary
for the Logical Volume Device Driver, is allocated.
\l'\n(.lu-\n(.iu'
.so datastruct.pic
.FG "LV Device Driver data structures describing a volume group." \n(H1-
\l'\n(.lu-\n(.iu'
.P
.nr !! \n(Fg
Figure \n(H1-\n(!! displays the relationship between the major
Logical Volume Device Driver data structures:  volgrp, lvol, part.
There is one device switch entry for each volume group defined on
the system.
Each volume group entry contains a pointer to the volume
group data structure describing it.
.P
The volume group structure, volgrp, is allocated when the volume
group it describes is opened.
Volume groups are implicitly opened when
any of their logical volumes are opened.
This structure contains general
information on a volume group, such as the partition size, number of
logical volumes, number of open logical volumes, and a pointer to
an array of logical volume structures, with one entry for each logical
volume in the volume group.
.P
The logical volume structure, lvol, is allocated when the logical volume
it describes is opened.
It contains general information on a logical
volume needed to perform logical I/O on it.
One of lvol's fields is a
pointer to a work in progress hash table that contains pointers to buf
stuctures for each I/O request in progress on this logical volume.
This is used to ensure that logical I/O requests to overlapping block
ranges must complete in FIFO order.
lvol also contains a pointer to a
physical partition map for each mirror of the logical volume.
.P
The partition maps are allocated when the containing logical volume is
opened.
The partition maps contain one entry, part, for each physical
partition.
Each entry contains the physical disk address of the
beginning of the partition and a pointer to a structure, pvol, that
describes the physical volume that this partition resides on.
The physical volume structure is allocated when its volume group is opened.
It contains general information about one physical volume in a volume
group and a pointer to a bad block directory that describes the known
bad blocks on this physical volume.
.H 4 "Top Half of Logical Volume Device Driver"
The top half of the Logical Volume Device Driver contains the code
that runs in the context of a process address space.
It contains all the entry points to the device driver except strategy:
.H 5 "open"
The open entry point is called by the file system when a logical
volume is mounted, to open the logical volume specified.
.H 5 "close"
The close entry point is called by the file system when a logical
volume is unmounted, to close the logical volume specified.
.H 5 "read"
The read entry point is called by the read system call to translate
character I/O requests to block I/O requests.
The read entry point
simply calls the 
.B "uphysio"
routine which verifies that the request
is legal, and invokes the 
LVDD strategy entry point which handles logical block I/O requests.
The
.B "uphysio"
routine may
break up the I/O into multiple requests before passing it on to
the strategy routine.
.H 5 "write"
The write entry point is called by the write system call to
translate character I/O requests to block I/O requests.
The LVDD write
performs the same function to write requests as the LVDD read does to
read requests.
.H 5 "ioctl"
See  -- Heading id 'hd' unknown --  for more information. (SEE WHERE?)
.H 4 "Bottom Half of Logical Volume Device Driver"
The bottom half of the LVDD contains the strategy entry point - the
code that is called to processes all logical block requests.
It validates I/O requests, check requests for conflicts on any requests in
progress on overlapping block ranges, translates logical addresses to
physical addresses, handles mirroring and bad block relocation.
This half of the device driver runs on interrupt levels,
and therefore is not permitted to access any user process context, or to
block.
.P
The bottom half of the LVDD is broken down into three layers:
.BL
.LI
strategy - performs logical requests validation, initialization
and termination; serializes logical requests when their block ranges
overlap.
.LI
scheduler - schedules physical requests for logical operations and
handles mirroring.
.LI
physical - performs physical request startup and termination.
Handles bad block relocation.
.LE 1
.P
Each logical I/O request goes through these three layers
(strategy -> scheduler -> physical) to the physical device driver.
Then once the I/O is complete, the request
returns back through each layer (physical -> scheduler -> strategy)
to handle the I/O completion processing at each layer
before returning to the original requestor.
.H 5 "Strategy"
The LVDD strategy entry point is the first layer.
It deals only with logical requests.
It is called with a logical buf
structure (defined in <sys/buf.h>) that contains information about
the request and a pointer to the data buffer.
.P
Strategy first validates each request, returning immediately any
invalid requests:
.BL
.LI
I/O past end of logical volume
.LI
invalid length (not a multiple of blocks size)
.LE 1
Then, to ensure that logical requests to overlapping block ranges must
complete in FIFO order, this layer keeps track of all outstanding
requests using a work-in-progress queue for each logical volume.
Whenever a new request arrives, it is blocked until all earlier
conflicting requests in the same logical track group have completed.
When each request finishes, those that are blocked waiting for it will
be scheduled.
Serializing requests at the logical layer makes
processing much simpler for the lower level physical operations of the LVDD.
Bad block relocation and mirror retries can be scheduled
without fear that an overlapping request has slipped into the queue
out of order and changed the data.
.P
.B Note:
This "exclusive lock" on the block range of each request is sufficient
to ensure FIFO operation, but does not provide maximum overlap for
concurrent reads.
This is not a problem, because most users of this
service (file system, VMM, etc) are combining concurrent reads of the
same block at a higher level.
In practice, the oportunity to schedule
overlapping reads concurrently will be rare, so the simplicity of
exclusive locks for both reads and writes is attractive.
.P
A list of buf structures for requests that are not blocked is passed
to the second layer - the scheduler.
.H 5 "Scheduler"
The scheduler layer schedules one or more physical requests for each
logical request.
This involves:
.BL
.LI
translating logical addresses to physical addresses
.LI
handling mirroring
.LI
calling the LVDD physical layer with a list of physical requests.
.LE
When a physical I/O operation is complete, the scheduler initiates the
next phase of the corresponding logical requests, or calls the
strategy termination routine to notify the originator that the
operation has been completed.
.H 6 "Translating Logical Address to Physical Address"
The scheduler handles the translation of logical address <volume
group, logical volume, logical block> to a physical address <physical
volume type, physical volume, physical block>.
The first two values
of the address correspond to the major number and minor number of the device.
For a logical address, the major number specifies the volume
group and the minor number specifies the logical volume in the volume
group (this pair is refered to as a logical dev_t).
For a physical
address, the major number specifies the physical disk device driver
and the minor number specifies physical volume that the request is for
(this is refered to as a physical dev_t).
The logical buf structure
passed to the LVDD strategy entry point contains the logical dev_t and
the logical block number of the start of the request.
The logical block may map to from one to three physical blocks, corresponding to
the primary, secondary, or tertiary copy of the logical block.
.P
To map a logical address to a physical address, the major number from
the logical dev_t specifies the volgrp structure to use.
The minor
number in the logical dev_t is used to index into the lvol array to find
the specific logical volume entry.
Then index into the logical volume's
partition array (parts) with the mirror number to be used (primary=0,
secondary=1, tertiary=2) and add the logical partiton number
to get the physical partition entry in that mirrors partiton array.
This entry contains the
physical dev_t (major and minor number of the physical volume this
partiton resides on) and the starting address of the physical partition.
.H 6 "Scheduling Policies"
Different scheduling policies seem to be best for different
applications depending on the recovery strategy they employ.
Therefore, the LVDD scheduler is state-driven and the initial state
determines the policy used for a given logical volume.
This way additional scheduling policies can be easily added.
There are three scheduling policies:
.BL
.LI
non-mirrored
[ NOTE: This may be eliminated unless there are unusual performance
implications. ]
.LI
write mirrors sequentially
.LI
write mirrors in parallel
.LE 1
.H 6 "Non-Mirrored Scheduling Policy"
The non-mirrored scheduling policy is the most common and most simple
case.
One physical 
.B "buf"
structure is allocated for a logical request.
The logical address is translated to a physical address (see discussion
above) and the physical layer is called to check for bad blocks
and send the request to the appropriate physical device driver.
.H 6 "Mirrored Sequential Scheduling Policy"
This scheduling policy performs mirrored writes in order: primary,
secondary, then tertiary.
A physical 
.B "buf"
structure is allocated for
the logical request, the logical address is translated to a physical
address, and the last layer of the LVDD is called to check for bad blocks
and call the appropriate physical device driver.
Once the I/O is
completed, these operations are repeated for the secondary and then
the tertiary copy using the same physical buf structure.
.P
For read operations, the primary mirror is read.
If the read fails,
the next mirrored copy is read and then the primary copy is "fixed" by
turning the read operation into a write, specifying hardware
relocation in the call to the physical device driver (see "Detecting and
Correcting Bad Blocks" for more information on hardware relocation).
.H 6 "Mirrored Parallel Scheduling Policy"
This scheduling policy performs writes for all mirrors in parallel.
A physical  
.B "buf"
structure is allocated for
.I each
copy.
If there are not enough physical 
.B "buf"
structures available
to perform the entire operation, the logical 
.B "buf"
structure is
put on a buffer wait queue until more 
.B "pbuf" s
are available.
If enough 
.B "pbuf"
structures are available, the address is translated
and the physical layer of the LVDD is called for each copy to check
for bad blocks and call the appropriate physical device driver.
On
I/O completion of each copy, the physical 
.B "buf"
structure is freed.
Once the last copy is completed, the strategy termination
routine is called to notify the original requestor of the logical I/O
completion.
.P
For read operations, we take advantage of mirroring to increase the
I/O performance.
The mirror whose physical volume arm is closest to
the current request is selected to be read.
This is done by keeping
track of the last requested arm position for each physical volume in the
.B "pvol"
data structure, and comparing these with the physical
address of each mirror to find the mirror whose arm would have to move
the shortest distance.
If the read operation fails, one of the other
copies is read, and on successful completion, the first copy is
"fixed" by turning the read operation into a write with hardware
relocation specified on the call to the physical device driver. (See
Detecting and Correcting Bad Blocks for more information on hardware
relocation.)
.H 5 "Physical Layer"
The third layer of the LVDD, the physical layer, handles physical
request startup and termination.
This layer calls the physical device driver strategy entry point with a
.B "buf"
structure, and is called by 
.B "iodone"
when each physical request is completed.
This layer also performs bad block relocation and
detection/correction of bad blocks when necessary, hiding these
details from the other two layers.
.H 6 "Interface to Physical Device Drivers"
The physical layer of the LVDD interfaces with the different physical
device driver strategy entry points.
The physical device driver
strategy entry points are called with a
.B "buf"
structure (see
.B "<sys/buf.h>"
for definition of standard buf structure).
The 
.B "buf"
structure contains all the
information needed for an I/O request.
The 
.B "b_options"
field in the 
.B "buf"
informs the physical device driver if write verification
and/or hardware relocation should be performed on the request.
.B "b_options"
is constructued by logical OR-ing zero or more of the
following values:
.DS L
    
.B " WRITEV"
	- perform write verification
    
.B "HWRELOC"
	- perform hardware relocation only if safe
    
.B "UNSAFEREL"
	- perform hardware relocation even if unsafe
??EXPLAIN SAFE/UNSAFE??)
[ got me. ]
.DE
If hardware relocation or write verification is not supported by the
physical disk, the physical device driver should set the 
.B "EINVAL"
value in the 
.B "b_error"
field in the 
.B "buf"
structure.
.P
On I/O completion, 
.B "iodone" () 
calls the LVDD completion routine in the physical layer (specified in the 
.B "b_done"
field in the buf structure).
This routine expects these fields to be set in the buf
structure by the physical device driver if an error occurred:
.DS L
.B "b_flags"
- should have the 
.B "B_ERROR"
value set
     
.B "b_error"
- should be set to 
.B "EMEDIA"
     
.B "b_resid"
- should be set to the amount of data not transfered after the error.
.DE
.H 6 "Bad Block Relocation"
The physical layer of the LVDD checks each physical request to see if
there are any known bad blocks in the request.
Each physical volume data structure, 
.B "pvol" ,
has a pointer to the defect directory hash
anchor table for that physical volume.
This table contains entries for
all known bad blocks that have not been relocated by the hardware.
.P
To determine if the request contains any such blocks, the physical
address is hashed and its hash chain is searched to see if any bad
block entries are in the address range of the request.
If there is, the physical request is split up into multiple separate physical
requests: the first containing any blocks up to the bad block; the
second contains the relocated block (relocated address is specified in
the bad block entry); the third contains any blocks after the bad
block to the end of the request.
These separate requests are processed sequentially.
.P
Once the I/O of the first request is completed,
.B "iodone" ()
calls the LVDD physical layer termination routine (specified in 
.B "b_done"
in buf structure) which will initiate the request with the relocated
block, and then the remaining request.
Once the entire physical
operation is completed, the appropriate scheduler policy routine in
the second layer of the LVDD will be called to start the next phase of
the logical operation.
.P
Detecting and Correcting Bad Blocks
.P
The physical layer of the LVDD initiates all bad block processing
and isolates all of the decision making from the physical device driver.
This is done so that the physical device driver does not need to know
anything about mirroring.
If a logical volume is mirrored, a newly
detected bad block can actually be "fixed" by relocating the block,
reading the mirror and then writing the contents of the good mirror to
the relocated block.
With mirroring the user need not even know when
bad blocks are found, but the physical device driver will log permanent
I/O errors so the user can determine the rate of media surface errors.
.P
When a bad block is detected during I/O, the physical device driver
sets the error fields in the buf structure to indicate that there was
a media surface error.
The physical layer of the LVDD then initiates
any bad block processing that must be done.
.P
If the operation was a non-mirrored read, the block is not relocated.
This is done because the relocated block will contain garbage until a
write is performed to the block.
To handle this, an entry is put in
the LVDD defects directory and the defects directory on disk for the
bad block with no relocated block address and the status set to
"relocation desired".
On each I/O request the physical layer checks
to see if there are any bad blocks in the request.
If the request is
a write and it contains a block that is in a "relocate desired" state,
the request will be sent to the physical device driver with 
.B "safe"
hardware relocation requested.
If the request is a read, an I/O error
will be returned to the original requestor.
.P
If the operation was for a mirrored read, a request to read one of the
mirrors is initiated.
If the second read is successful, then the read
is turned into a write request and the physical device driver is called with 
.B "safe"
hardware relocation specified.
.P
If the hardware relocation succeeds, the bad block entry in the LVDD
defects directory and the defects directory on the physical volume are
deleted.
.P
If the hardware relocation fails or the device does not support
.B "safe"
hardware relocation, the physical layer of the LVDD
attempts software relocation.
??YES, AND HOW DOES IT DO THAT??)
.H 3 "Logical Volume Manager Initialization"
The Logical Volume Manager initialization which must occur
consists of the following:
.AL
.LI
Load the logical volume device driver (if necessary)
.LI
Vary the each volume group on-line.
.P
The 
.B "lvm_varyonvg"
subroutine performs the recovery necessary to
keep the LVM descriptors and any mirrored data consistent, build the
LVM data structures that describe the root volume group, and call the
LVDD configuration entry point.
.LE 1
.H 3 "Disk Recovery"
Physical disks exhibit several significant failure modes which must be
handled by our recovery techniques:
.BL
.LI
Surface defects
.LI
Catastrophic media failure (head crash, etc.)
.LI
Drive inaccessible due to adapter, bus, power supply or drive
electronics failures
.LE 1
.P
This section mostly concerns itself with the last type of failure.
Surface defects are addressed by mirroring and by bad block relocation
above.
.P
Drive accessibility failures cause the most trouble for the logical
volume manager.
Volume descriptors can become inconsistent and mirrored
partitions can lose synchronization.
Considerable system overhead must be
incurred during recovery, when a missing drive becomes accessible again.
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
For high-availability configurations, you should use mirroring.
Partitions should be allocated so that mirrors are accessed via
separate adapters and buses and run off separate DC power supplies.
Because mirroring improves the availability, larger volume groups
become practical in these configurations.
For these systems, on-line
mirror resynchronization is an important requirement; the machine
should never reboot because of any single failure.
.H 4 "Physical drive failure detection"
There are two reasons why a physical drive may be declared missing
from a volume group:
.BL
.LI
The physical volume cannot be located when the volume group is varied
on-line.
.LI
A drive that had been varied on-line successfully cannot be written
after repeated attempts.
.LE 1
.P
Each of these failure criteria must be applied very cautiously.
.P
Since drives must power-on in sequence, a fairly lengthy timeout is
needed before deciding that a drive is unavailable.
Timeouts as large
as a minute or two might be reasonable in this situation.
.P
Write errors can cause mirrors to lose synchronization.
Surface defects are normally not detected on writes, but sometimes this may
happen.
When surface defects are encountered on writes, and the
attempted relocation fails, we must assume that the drive has somehow
become unavailable, and declare it missing from the volume group.
With mirroring, read errors can also result in relocation, if some
other mirror can be read successfully.
When rewriting the relocated
bad block in this case, we may discover that the defective drive
cannot be written.
Declaring the drive missing in this case is
probably optional, but may be a good idea.
.P
(??WHO IS WE, ARE THERE THINGS THAT A VENDOR PROGRAMMER WOULD DO OR IS
THIS JUST A THINK PIECE THAT WAS USED IN IBM PLANNING??  SHOULD I CUT
ALL OF THIS OUT, OR WERE SOME OF THESE QUESTIONS RESOLVED, AND WHAT IS
THE RESOLUTION ??)
.P
Since the recovery cost of re-integrating lost mirrors is high, we
need to be conservative about declaring drives unavailable.
We may wish to run a few simple diagnostic tests before giving up - read the
minidisk directory, for example, or write the CE cylinder.
Finally, we may wish to retry the operation after a sizeable time delay before
giving up.
.P
Once a physical drive has been declared missing from the volume group,
that fact is recorded on all copies of the volume group descriptor
area before any other processing can continue.
This is done by updating the the volume
state field of the missing physical volume's physical volume header in
the volume group descriptor area.
Before the updated descriptor area
is written to disk, the timestamp value in the volume group header and
in the volume group trailer is updated with the current value from the
processor-time-of-day clock.
.H 4 "Physical drive recovery"
When a missing physical drive is returned to its volume group, any
information it contains is carefully reintegrated with the current
state of the volume group.
If the returned physical drive contains a
copy of the volume group descriptor area, then the descriptor area on
this physical volume is updated, if necessary, with the contents of
the most current volume group descriptor area for this volume group.
If the physical volume contains any partitions that have become stale
while it was missing (??WHICH MEANS?? HOW IS IT DETERMINED??), then,
depending on the flag options (??WHAT OPTIONS??) from the varyon,
these stale partitions will either be resynchronized (??HOW??), or
some type of message (WHAT MESSAGE??) is logged to notify the user
that this physical volume is in need of resynchronization.
.H 5 "Volume Group Descriptor Recovery"
The volume group descriptor area contains essential information for
the Logical Volume Manager.
The consistency of the volume group
descriptor area on the individual physical volumes which contain a
copy must be maintained in case of a system failure while the copies
are being updated.
Recovery of the on-disk volume group descriptor
areas takes place when the volume group is varied on-line.
(??WHAT DOES
THIS MEAN FOR AN LVDD THAT IS ON A PHYS VOLUME THAT BECOMES UNAVAIL
AND THEN AVAIL WHILE THE VG IS STILL VARIED ON?? THIS SEEMS TO
CONTRADICT THE PREVIOUS PARAGRAPH??)
.P
Whenever an LVM system management function is called to make changes
to a logical volume or a physical volume in the volume group (?WHAT
DOES THAT MEAN??), then the following method is used to update the
volume group descriptor area in memory and to record it on all
physical volumes in the group which contain on-disk copies.
Note that this is not a complete implementation algorithm.
The purpose of its
inclusion is merely to convey how the on-disk copies of the volume
group descriptor area are updated in order to keep the data in those
descriptor areas consistent.
Many implementation details have purposely been left out.
(??SHOULD WE PUT THEM IN??)
.br
[one wonders what purpose is served by leaving them out]
.AL
.LI
Read the processor-time-of-day clock into memory.
.LI
For the in-memory copy of the volume group descriptor area, do
the following:
.AL a
.LI
Copy the processor-time-of-day clock into the volume group
header timestamp.
.LI
Copy the processor-time-of-day clock into the volume group
trailer timestamp.
.LI
Update the desired parts of the volume group descriptor area.
(??FROM WHERE, HOW??)
.LE 1
.LI
For each physical volume containing a copy of the descriptor area,
update the on-disk copy in the following manner:
.BL
.LI
Write the 512-byte block (??) containing the volume group header.
.LI
Write all blocks that have been updated in the logical volume
list, the physical volume list, and the name list area.
(?? AS
DETERMINED BY SOME TIMESTAMP? OR WHAT??)
.LI
Write the 512-byte (??) block containing the volume group trailer.
.LE 1
.P
.B Note:
If a significant number of blocks have been updated, then the
alternative of writing the entire volume group descriptor area
with one write may be used.
(??WELL WAS IT??)
.LE 1
.P
When a volume group is varied on-line, the following method is be used
to check and update the on-disk copies of the volume group descriptor area.
Note that this is not a complete implementation algorithm.
The purpose of its inclusion is merely to convey how the on-disk copies of
the volume group descriptor area are used during the vary-on of the
volume group to perform recovery for the volume group descriptor area.
Many implementation details have purposely been left out. (??SAME
COMMENTS AS ABOVE??)
.AL 1
.LI
Initialize the timestamp_save variable to 0.
.LI
In order to find the most current copy of the on-disk volume
group descriptor area, do the following for each on-disk copy:
.AL a
.LI
Read the volume group header into memory.
.LI
Read the volume group trailer into memory.
.LI
If 
.B "timestamp_vg_header"
equals 
.B "timestamp_vg_trailer" ,
then
.AL i
.LI
If 
.B "timestamp_vg_header"
is greater than 
.B "timestamp_save" ,
then
.BL
.LI
Set 
.B "pv_ptr"
to save identification of the physical volume which
contains this copy of the volume group descriptor area.
.LI
Set 
.B "timestamp_save"
equal to 
.B "timestamp_vg_header" .
.LE 1
.LE 1
.LE 1
.LI
Read the entire volume group descriptor area from the physical
volume pointed to by 
.B "pv_ptr"
into 
.B "mem_vg_desc_area" .
.LI
In order to update the on-disk copies, do the following for each on-disk copy:
.AL
.LI a
Read the volume group header into memory.
.LI
Read the volume group trailer into memory.
.LI
If 
.B "timestamp_vg_header"
not equal to 
.B "timestamp_vg_trailer" ,
then
.AL i
.LI
Set 
.B "timestamp_vg_trailer"
to 0.
.LI
Write the volume group trailer from memory to on-disk copy.
.LI
Write 
.B "mem_vg_desc_area"
to on-disk copy of the volume group descriptor area.
.LE 1
.LI
Else if 
.B "timestamp_vg_header"
equals 
.B "timestamp_vg_trailer" ,
then
.AL i
.LI
If 
.B "timestamp_save"
is greater than 
.B "timestamp_vg_header" ,
then
Write 
.B "mem_vg_desc_area"
to the on-disk copy of the volume group
descriptor area.
.LE 1
.LE 1
.LE 1
.P
The following list describes some of the information being kept to
ensure that the volume group descriptor area can be resynchronized to
maintain consistency in the case of a system failure while its copies
are being updated and to ensure that stale partitions of a previously
missing physical volume can be resynchronized.
.BL
.LI
During execution of the 
.B "lvm_varyonvg" ()
library function, the volume group
descriptor area information for any physical volumes in the volume
group which cannot be powered up and are declared missing is updated
to indicate that the physical volumes are missing.
The user will be
notified in some manner of any missing physical volumes.
(??HOW, BY WHAT?)
[ This will be determined by the command that does this.]
.LI
The 
.B "lvm_varyonvg" ()
of a volume group will not be successful
unless there are enough copies of the volume group descriptor area
contained on the physical volumes which can be powered-on to ensure
maintaining the consistency of the volume group descriptor area.
(??HOW MANY, A MAJORITY OF THE TOTAL I THINK BUT WHY?? )
.LI
Once the 
.B "lvmvaryon_vg" ()
function has determined that the
Logical Volume Manager (LVM) can maintain consistency of the volume
group descriptor area, then all copies of the volume group descriptor
area on the physical volumes that became active will be resynchronized
and updated, if they are not currently consistent.
.LI
If a physical volume which has been an active member of the volume
group is declared missing by the Logical Volume Device Driver (LVDD),
then it will update information in the volume group
descriptor area to indicate the physical volume state of this physical
volume is missing.
.LI
When a previously active volume becomes missing, the LVM device driver
ensures that the remaining copies of the volume group descriptor
area are enough to maintain the consistency of the descriptor area.
If this cannot be done, the volume group will be declared inactive.
.LI
When the LVDD first writes to a logical partition where one of the
physical partitions is missing, this information is recorded in 
the Volume Group Descriptor Area
to indicate that the missing physical partition is
now stale.
.LI
If an I/O request is made to a logical partition whose only active
mirror partitions (i.e., those that are currently on-line) are in the
stale state, the LVDD returns an 
.B "errno"
value of 
.B "EIO" .
The Logical Volume Manager must ensure that all physical copies for a
logical partition cannot become stale.
.LI
When a previously missing physical volume again becomes active, the
on-disk volume group descriptor area, if present on that physical
volume, will be checked and updated (??BY WHOM, WEHN??), if necessary,
(??WHEN IS IT NOT??) to maintain consistency.
Stale physical
partitions will either be resynchronized, or some type of notification
will be given if the physical volume needs to be resynchronized.
.LE 1
