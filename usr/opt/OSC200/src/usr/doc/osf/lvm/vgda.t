.\" (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
.\" ALL RIGHTS RESERVED
.\" OSF/1 Release 1.0
.H 2 "Layout of the Volume Group Reserved Area"
.H 3 "Volume Group Descriptor Area"
.IX "logical volume" "group descriptor area"
.IX "disk" "volume group descriptor area"
.IX "volume group descriptor area"
The Logical Volume Manager
.B "volume group descriptor area"
for a volume group contains:
.AL 1
.LI
The volume group header
.LI
A list of logical volume entries
.LI
A list of physical volumes, where each entry contains a physical
volume header and a map of physical extents
.LI
The volume group trailer
.LE
.P
.nr !! \n(Fg+1
Figure \n(H1-\n(!! indicates the layout of the volume group descriptor
area on a physical volume.
.fS
.so VGDA.pic
.fC
.FG "Layout of the Volume Group Descriptor Area." \n(H1-
.fE
.P
The size of the area needed to describe a physical volume in the
physical volume list is variable because the number of entries in the
physical extent map depends on the size of the physical volume.
For example, a 200 megabyte physical volume with a physical extent size
of 1 megabyte will have 200 physical extent map entries.
.P
The space required (in DEV_BSIZE sectors) is given by the formula:
.DS
.ta .5i
.Cw
size = 1 + ((16 * maxlvs) + (DEV_BSIZE-1))/DEV_BSIZE +
	maxpvs * (16 + (4 * maxpxs) + (DEV_BSIZE-1))/DEV_BSIZE + 1
.DE
.P
For maxlvs = 255, maxpvs = 32, and maxpxs = 1016, this is a total
of 266 blocks, or 133 KB per volume group descriptor area.
.P
Each Physical Volume contains space for 2 volume group descriptor areas.
.P
For each on-disk copy of the descriptor area, the timestamp values in
its volume group header and its volume group trailer are compared for
equality to determine if the last write to disk of that descriptor area
was completed successfully.
If the timestamps are identical, then the descriptor area is considered
valid.
Since the writing of one copy is
completed before another is started, at most one descriptor
area is in transition at any given time.
.P
The volume group must always have at least 2 descriptor areas that
are kept up to date. 
This is to guarantee that if either a bad block develops in
one of the descriptor areas, or if the system crashes during a write to
one of the descriptor
areas, that there is always at least 1 valid copy available.
If both of these errors occur simultaneously, of course, then
data may be lost.
In normal operation every physical volume contains
at least 1 valid copy of the descriptor area, so the likelihood
of destroying all of them simultaneously is low.
If both descriptor areas are valid, then the newest is considered the
current descriptor area for that physical volume.
.P
In small volume groups (1 physical volume), this means that
both descriptor areas are always kept up to date on the disk.
For larger volume groups (2 or more physical volumes) the 
descriptor areas on each disk are written alternately.
.P
The value of the timestamps among the valid on-disk descriptor
areas is used to determine which of the on-disk descriptor areas contains the
most up-to-date information for the volume group.
The timestamp values among copies could differ because of previously missing
physical volumes or because of a system failure during the updating
of on-disk descriptor areas.
.H 4 "Volume Group Header"
.IX "volume group" "header"
.IX "disk" "volume group header"
The 
.B "volume group header"
contains global information about the
volume group and has the following format:
.fS
.so VG_header.pic
.fC
.FG "Volume Group Header Format." \n(H1-
.fE
.B "Note:"
Byte offsets are hexadecimal.
Slashes ("//.../") indicate a reserved field.
.P
The volume group header begins at offset zero within the volume group
descriptor area, and occupies an entire 512-byte block.
The volume group header contains the following information:
.TS
tab(;);
lBw(5m) lBw(5i).
Bytes;Description
.T&
lf7 l.
0x00-0x07;T{
The 
.B "vg_timestamp"
field contains a copy of the contents of the
Processor Time-Of-Day Clock, indicating when the descriptor area was
most recently updated.
If and only if the descriptor area has never
been updated, or the physical volume has been deallocated (removed
from a volume group), vg_timestamp contains a 64-bit string of binary
zeroes.
This field is used to verify the consistency of the descriptor area.
T}
0x08-0x0F;T{
The 
.B "vg_id"
field contains a 64-bit number uniquely and
universally identifying a volume group.
This unique identifier is assigned by the Logical Volume Manager.
T}
0x10-0x11;T{
The 
.B "maxlvs"
field specifies the maximum number of logical volumes
allowed in the volume group.
This value is specified when the volume group is created.
The 
.B "maxlvs"
is an integer in the range of 1 to
LVM_MAXLVS.
T}
0x12-0x13;T{
The 
.B "numpvs"
field indicates the number of physical volumes
currently installed in the volume group.
T}
0x14-0x15;T{
The
.B "maxpxs"
field indicates how much space is to be reserved in each
physical volume entry for physical extent data.
This places an upper limit on the size of any physical volume
in the volume group.
T}
0x16-0x1F; These bytes are reserved, and set to zero.
.TE
.H 4 "Logical Volume List"
.IX "logical volume" "list"
.IX "disk" "logical volume list"
The 
.B "logical volume list"
contains entries for the maximum number
of logical volumes allowed in the volume group.
This number is specified when a volume group is created.
This number is represented by the 
.B "maxlvs"
value stored in the volume group header.
An entry's index into the list is the minor number of the
logical volume.
Any or all entries in the logical volume list may be null.
.P
The logical volume list begins on a 512-byte block boundary.
.H 4 "Logical Volume Entry"
.IX "logical volume" "entries"
Each 
.B "logical volume entry"
describes one logical volume within
the volume group and has the following format:
.fS
.so LV_entry.pic
.fC
.FG "Logical Volume Entry Format." \n(H1-
.fE
.B "Note:"
Byte offsets are hexadecimal.
Slashes ("//.../") indicate a reserved field.
.P
Each logical volume entry contains the following information:
.TS
tab(;);
lBw(5m) lBw(5i).
Bytes;Description
.T&
lf7 l.
0x00-0x01;T{
The 
.B "maxlxs"
field indicates the maximum size (in logical
extents) up to which this logical volume may grow.
Its value may range from 0 to 
.B "LVM_MAXLXS" .
T}
0x02-0x03;T{
The
.B "lv_flags"
field indicates the current state of this logical volume.
It contains the following bits which are OR'ed together:
.VL 20n 0
.LI "LVM_DISABLED"
If set, this logical volume is not available for use.
.LI "LVM_LVDEFINED"
If set, this logical volume entry is defined.
.LI "LVM_NORELOC"
If set, defect records within
this logical volume are ignored.
If clear, known defects are relocated.
.LI "LVM_RDONLY"
Only read access only is permitted for this logical volume.
.LI "LVM_STRICT"
If set, mirror allocation must be strictly enforced.
[But where is the policy that is being enforced?]
[Policy field: neither used nor referenced by the driver.]
.LI "LVM_VERIFY"
If set, logical volume writes will be verified.
If clear, writes will not be verified.
.LI "LVM_NOMWC"
If set, mirror write consistency is not performed for this
logical volume.
This means that mirror consistency is not guaranteed after
a crash.
.LE 1
T}
0x04;T{
The 
.B "sched_strat"
field indicates the strategy used to write copies of the logical extent.
It may be one of the following:
.VL 20n 0
.LI "LVM_PARALLEL"
Write mirror copies in parallel.
.LI "LVM_SEQUENTIAL"
Write mirror copies sequentially.
.LE 1
T}
0x05;T{
The
.B "maxmirrors"
field defines the maximum number of mirror copies that are to 
be allocated for any logical extent in this logical volume.
Valid values range from 0 (non-mirrored) to 2 (doubly-mirrored).
T}
0x06-0x0F;T{
Reserved area, set to zero.
T}
.TE
.H 4 "Physical Volume List"
.IX "physical volume" "list"
.IX "disk" "physical volume list"
The 
.B "physical volume list"
consists of a physical volume entry for
each physical volume in the volume group.
A physical volume entry
consists of a physical volume header, which contains global
information about the physical volume, and a map of physical extent
entries, each of which describes one physical extent on the
physical volume.
At any given time, the number of entries in the
physical volume list (i.e., the number of physical volumes currently
installed in the volume group) is given by
.B "numpvs" ,
a variable which is stored in the volume group header and whose value may range
from 1 to LVM_MAXPVS.
.P
Each physical volume entry begins on a 512-byte block boundary.
.H 4 "Physical Volume Header"
.IX "physical volume" header"
The 
.B "physical volume header"
contains information about the
physical volume and about the contents of the physical extent map.
It has the following format:
.fS
.so PV_header.pic
.fC
.FG "Physical Volume Header Format." \n(H1-
.fE
.B "Note:"
Byte offsets are hexadecimal.
Slashes ("//.../") indicate a reserved field.
.P
The physical volume header fields contain the following information:
.TS
tab(;);
lBw(5m) lBw(5i).
Bytes;Description
.T&
lf7 l.
0x00-0x07;T{
The 
.B "pv_id"
field is a 64-bit number uniquely and universally
identifying a physical volume.
This unique identifier should be
thought of as having been assigned by the manufacturer of the physical
volume, although it may be assigned by the Logical Volume Manager.
.P
If the value contained in the "Unique Identifier" field of the
LVM Record (bytes 8-15) is nonzero when the physical volume
is first installed in a system, the value is copied into the
.B "pv_id"
field of the physical volume header, and is used from that
point on by the Logical Volume Manager as the unique identifier for
the physical volume.
.P
If this field is zero, it indicates that the Physical Volume Entry
is not valid, with the exception of the
.B px_count
field.
T}
0x08-0x09;T{
The 
.B "px_count"
field indicates the number of physical
extents contained in this Physical Volume.
The
.B "px_count"
is computed by the Logical Volume Manager from the
number of physical blocks in the physical volume and the
size of each physical extent.
This field also indicates the size of the Physical Volume Entry
in the Physical Volume List.
T}
0x0A-0x0B;T{
The 
.B "pv_flags"
field indicates the current state of the physical
volume and may have one or more of the following values (logically
ORed):
.VL 20n 0
.LI "LVM_PVDEFINED"
If set, this physical volume (actually, this physical volume entry)
is defined.
.LI "LVM_PVNOALLOC"
Allocation of extents on this physical volume is not allowed
at this time.
This state must be set when this physical volume is being migrated
to another physical volume.
.LI "LVM_NOVGDA"
A volume group descriptor area is not maintained on this disk.
.LI "LVM_PVRORELOC"
Do not relocate new defects discovered on this physical volume:
the bad block directory is treated as read-only.
.LE 1
T}
0x0C-0x0D;T{
The
.B "pv_msize"
field is the size of the physical extent map, in entries.
It indicates the amount of disk space this physical volume
entry occupies.
T}
0x0E-0x0F;T{
The pv_defectlim field indicates the maximum number of
software-relocated defects allowed on this physical volume.
T}
.TE
.H 4 "Physical Extent Map Entry"
.IX "physical volume" "physical extent map"
The 
.B "physical extent map"
contains a variable
.IX "" "physical extent map entries"
number of
.B "physical extent map entries" ,
depending on the physical volume size.
Each entry describes one physical extent within the physical volume,
and has the following format:
.fS
.so PP_map_entry.pic
.fC
.FG "Physical Extent Map Entry Format." \n(H1-
.fE
.B "Note: "
Byte offsets are hexadecimal.
.P
The physical extent map entry contains the following
information:
.TS
tab(;);
lBw(5m) lBw(5i).
Bytes;Description
.T&
lf7 l.
0x00-0x01;T{
The 
.B "lv_index"
field is an index into the logical volume list.
It specifies the logical volume that this extent is allocated to.
This value is also the minor number of the logical volume.
A value of zero indicates an unallocated Physical Extent.
T}
0x02-0x03;T{
The 
.B "lx_num"
field indicates the logical extent, within the
logical volume that contains this physical extent, represented by
this physical extent.
The 
.B "lx_num"
is an integer in the range
of 0 to LVM_MAXLXS.
T}
.TE
.H 4 "Volume Group Trailer"
.IX "logical volume" "volume group trailer"
.IX "disk" "volume group trailer"
.IX "volume group trailer"
The 
.B "volume group trailer"
contains a timestamp value and has the following format:
.fS
.so VG_trailer.pic
.fC
.FG "Volume Group Trailer Format." \n(H1-
.fE
.B "Note:"
Byte offsets are hexadecimal.
Slashes ("//.../") indicate a reserved field.
.P
The volume group trailer occupies a 512-byte block.
.TS
tab(;);
lBw(5m) lBw(5i).
Bytes;Description
.T&
lf7 l.
0x00-0x07;T{
The 
.B "vg_timestamp"
contains a copy of the contents of the
Processor Time-Of-Day Clock, indicating when the descriptor area was
most recently updated.
If and only if the descriptor area has never
been updated, or the physical volume has been deallocated (removed
from a volume group), 
.B "vg_timestamp"
contains a 64-bit string of binary zeroes.
This field is used to verify the consistency of the descriptor area.
T}
0x00-0x1F;These bytes are reserved, and are set to zero.
.TE
.H 3 "Volume Group Status Area"
The
.B "Volume Group Status Area"
(VGSA)
.IX "disk" "volume group status area"
.IX "volume group status area"
tracks the validity and availability of the physical volumes.
It contains 1 bit of state for each physical volume in the volume group, plus 1
bit of state for each physical extent in each physical volume.
The VGSA can be thought of as an extension of the volume group descriptor area.
For any physical volume containing a VGDA, there is space for 2 copies of
the VGSA.
Just like the VGDA, the VGSA contains head and tail
timestamps to indicate validity, and the 2 available locations are
updated alternately.
.P
The size of the status area, (and hence the location of the trailing timestamp)
is contained in the LVM Record.
.fS
.so VGSA.pic
.fC
.FG "Volume Group Status Area" \n(H1-
.fE
The number of longwords allocated to the PV missing bits is variable,
and is determined by the 
.Cw maxpvs
field: 1 longword is allocated for each 32 physical volumes (or fraction
thereof) allowed in the volume group.
The index into this bit array is the same as the index
into the physical volume list.
.P
The number of bytes allocated to the physical extent stale bits in
also variable.
Each physical volume is allocated an equal amount of space for tracking
stale extents: 1 byte for each 8 physical extents allowed in a
physical volume in this volume group, obtained from the
.Cw maxpxs
field.
Each stale bit array begins on a byte boundary.
.P
The status area is padded in order to locate the trailing 
timestamp at the end of the blocks allocated for the status area.
.H 3 "Mirror Consistency Record"
The mirror consistency cache is used to minimize the amount
of I/O that must be performed to guarantee that all mirrors
are consistent following a system crash.
.P
Mirrors may become inconsistent if a physical volume is unavailable
when a write is performed, or if the system crashes prior to the 
completion of all writes to a mirrored area.
Mirrors can never become inconsistent due to reads, or I/O to
a non-mirrored extent.
Before allowing a mirrored write to begin, the LVM allocates an
entry in the mirror consistency cache, and writes it to one
of the physical volumes containing one of the mirror copies.
The cache entry is not available for reuse until all outstanding
writes to the mirrored region are completed.
.P
Mirror consistency entries are allocated on a Logical Track Group basis
(1 logical track group == 32 pages).
.P
When performing recovery after a crash, only the most recent valid mirror
consistency cache is used.
All logical track groups marked as in transition are assumed to be
inconsistent, and are recovered by copying one of the versions to the
remainder.
A more complete algorithm for the recovery mechanism is given in the
implementation chapter.
.fS
.so MWC.pic
.fC
.FG "Mirror Consistency Record." \n(H1-
.fE
