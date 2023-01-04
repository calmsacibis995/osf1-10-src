.\" (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
.\" ALL RIGHTS RESERVED
.\" OSF/1 Release 1.0
.H 1 "Physical Volume Layout"
.IX "physical volume" "layout"
.IX "disk" "physical volume layout"
This section describes the layout of a physical volume and the data
structures used to represent the physical-level characteristics of a
physical volume.
.P
A physical volume consists of a logically contiguous string of
physical sectors.
The physical sectors are numbered 0 through LPSN
(i.e., 0, 1, 2, 3, ... , LPSN), where LPSN is the last physical sector
number on the volume.
The total number of physical sectors on the volume is LPSN + 1.
.P
The following descriptions indicate the amount of space in 
each data item, but do not specify the byte order or
bit order with a given item.
The byte and bit order is determined by the system implementor, and
should be selected to be the "natural" order for the target
machine.
In the diagrams and descriptions, bits are numbered from 0 as the
Most Significant Bit to 31 as the Least Significant Bit within
a 32-bit word.
Volume Groups are not intended to be portable between implementations 
with different data representations.
.P
.nr !! \n(Fg+1
Figure \n(H1-\n(!! indicates the organization of the data areas on
the physical volume.
.fS
.so physvol.pic
.fC
.FG "Physical Volume Organization" \n(H1-
.fE
.H 2 "Layout of the Physical Volume Reserved Area"
Each physical volume uses the first 128 sectors
to store various types of disk configuration and operation information.
This information is placed on the disk by the Logical Volume Manager.
This section defines the structure and contents of the first 128 sectors of the
physical volume.
.P
.IX "disk" "reserved sectors"
.IX "physical volume" "reserved sectors"
The following table describes the information stored on the first
128 sectors of the LVM physical volume.
The locations of the respective items are expressed as physical sector number.
.P
Note that any reserved blocks are not read or written by the
LVM, and may be used, at the implementor's option, to store disk labeling
information.
.fS
.TS
center,tab(~),allbox;
lw(2.25i) lw(.65i).
Information~Phys. Sector
.T&
l lf7.
Reserved~0-7
LVM Record~8
Bad Block Directory~9-63
Reserved~64-71
Backup LVM Record~72
Backup Bad Block Directory~73-127
.TE
.fC
.FG "Layout of the Physical Volume Reserved Area" \n(H1-
.fE
.P
Each of the above fields, with the exception of the backup fields, is
discussed on the following pages.
The formats of the backup fields
are the same as that of their primary counterparts.
.H 3 "LVM Record"
.IX "disk" "LVM record"
.IX "physical volume" "LVM record"
.IX "logical volume" "record"
The LVM Record associates a physical volume with a volume
group.
.nr !! \n(Fg+1
Figure \n(H1-\n(!! indicates the layout of the LVM Record.
.fS
.so LVM_rec.pic
.fC
.FG "LVM Record Format" \n(H1-
.fE
.ne 10
The LVM Record contains the following information:
.TS
tab(;);
lBw lBw(5i).
Bytes;Description
.T&
lf7 l.
0x00-0x07;T{
.I "LVM Identification Field."
This contains an identifier which signifies that the LVM Record
is present and valid.
This field contains the ASCII characters
.B "LVMREC01"
T}
0x08-0x0F;T{
.I "Unique ID of the Physical Volume"
This field contains the unique id assigned to this
physical volume.
If this field contains 0, or the LVM Record has not
been initialized, then the LVM will generate a new unique id.
T}
0x10-0x17;T{
.I "Unique ID of the Volume Group."
This contains the unique id of the volume group into which this
physical volume has been installed.
This field contains 0 if the physical volume is not
currently a member of a volume group.
T}
0x18-0x1B;T{
.I "Physical Volume Number"
The index of this physical volume within the volume group.
Only valid if the
.I "Unique ID of the Volume Group"
is non-zero.
T}
0x1C-0x1F;T{
.I "Last Physical Sector Number"
The physical sector number of the last sector in the physical volume.
The amount of space available in the physical volume.
T}
0x20-0x23;T{
.I "Length of Volume Group Reserved Area."
This contains the length of the reserved area for
volume group information measured in number of sectors.
This field is set to 0 if the physical volume is not a 
member of a volume group.
The volume group reserved area is always begins immediately
after the physical volume reserved area (that is, at sector 128).
T}
0x24-0x27;T{
.I "Length of the Volume Group Descriptor Area."
This contains the length of the volume group descriptor area measured
in number of sectors or 0 if there is not a copy of the volume group
descriptor area on this disk.
T}
0x28-0x2B;T{
.I "Length of the Volume Group Status Area."
This contains the length of the volume group status area measured
in number of sectors or 0 if there is not a copy of the volume group
descriptor area on this disk.
The descriptor area and status area are always linked: a given physical
volume has neither or both, but never just one.
T}
0x2C-0x2F;T{
.I "Physical Sector Number of Primary Descriptor and Status Area."
This contains the physical sector number of where the volume group
descriptor area begins on this physical disk or 0 if there is not a
copy of the volume group descriptor and status areas on this disk.
T}
0x30-0x33;T{
.I "Physical Sector Number of Secondary Descriptor and Status Area."
This contains the physical sector number of where a secondary copy of
the volume group descriptor area begins on this physical disk or
0 if there is no secondary copy.
T}
0x34-0x37;T{
.I "Length of the Mirror Consistency Record."
This contains the length of the mirror consistency record measured in 
number of sectors.
The mirror consistency record is required on every physical volume
in the volume group.
T}
0x38-0x3B;T{
.I "Physical Sector Number of Primary Mirror Consistency Record."
This contains the physical sector number of the primary mirror
consistency record.
T}
0x3C-0x3F;T{
.I "Physical Sector Number of Secondary Mirror Consistency Record."
This contains the physical sector number of the secondary mirror
consistency record.
T}
0x40-0x43;T{
.I "Physical Sector Number of the LVM User Data Area."
This contains the location of the beginning of the data area used by
the LVM to store physical extents.
T}
0x44-0x47;T{
.I "Physical Extent Size."
The size of each of the physical extents in this physical volume.
This is expressed as an exponent: each extent is 2**size sectors
long.
Must be identical for all volumes in the volume group.
T}
0x48-0x4B;T{
.I "Physical Extent Space."
The space allocated, measured in number of sectors, for each of the
physical extents in this physical volume.
This must be at least as large as the physical extent size.
If larger than the physical extent size, the remaining space is
available for bad block relocation. [Note that the current version
of the driver does not use this space for bad block relocation, so 
setting these 2 sizes to be different does not actually cause bad blocks
to be redirected to this space.]
T}
0x4C-0x4F;T{
.I "Length of the Bad Block Relocation Pool."
Number of sectors available in the common bad block relocation pool.
[In this version of the driver, this is the only bad block relocation
area available.]
T}
0x50-0x53;T{
.I "Physical Sector Number of the Bad Block Relocation Pool."
Location of the common bad block relocation pool.
T}
0x54-0x1FF;T{
These bytes are reserved. Set to 0.
T}
.TE
.H 3 "Bad Block Directory"
.IX "disk" "bad block directory"
.IX "bad blocks" "directory"
The bad block directory keeps a record of the blocks on the physical
volume that have been diagnosed as unusable.
.P
Each entry in the bad block directory is 8 bytes long. The first entry
shows that this is the bad block directory. It contains the ASCII
letters 
.B "DEFECT01"
in the first 8 bytes.
.P
If the Bad Block Directory is in the process of being updated, 
and is internally inconsistent,
the first 8 bytes contain the ASCII letters
.B "UPDATE01" .
The remaining entries identify specific bad blocks.
The defect list is terminated by a sector consisting of all 
unused entries.
Each entry is structured as shown below:
.fS
.so BB_entry.pic
.fC
.FG "Bad Block Directory Entry Format" \n(H1-
.fE
.TS
tab(;);
lB lBw(5i).
Bytes;Description
.T&
lf7 l.
0x00-0x03;T{
Reason the block was marked as unusable and physical sector number
of the bad block.
.VL 5m 0
.LI "Bits\ 0-3"
Reason the block was marked unusable.
.VL 5m 0 1
.LI "0x0\ ="
This bad block entry is unused.
.LI "0x1\ ="
The physical volume manufacturer found a defect.
.LI "0xA\ ="
The surface verification diagnostic test found a defect.
.LI "0xB\ ="
The system found a defect.
.LI "0xC\ ="
The manufacturing test found a defect.
.LE 1
.LI "Bits\ 4-31"
Physical sector number of the bad block.
This is a 28-bit unsigned number.
Unused entries have the the bad block number set to 0.
.LE 1
T}
0x04-0x07;T{
Relocation status, and the physical sector number of the relocated
block.
.VL 5m 0
.LI "Bits\ 0-3"
Relocation status.
Not meaningful until the relocation sector is non-zero.
Defined as follows:
.VL 5m 0 1
.LI "0x0\ ="
Relocation has been performed.
.LI "0x1\ ="
Software relocation in progress.
.LI "0x2\ ="
Hardware relocation has been requested.
.LI "0x8\ ="
The relocated block needs to be written to.
.LE 1
.LI "\fRBits\ 4-31\fR"
Physical sector number of the relocated block.
This is a 28-bit unsigned number.
If this field is 0, the bad block has not been relocated.
.LE 1
T}
.TE
.so vgda.t
.H 2 "LVM User Data Area"
The
.B "LVM user data area"
is the region of the disk used to store the user's data blocks.
The user, in this case, might be a filesystem or a virtual memory
system.
The user data area is divided into fixed-size extents; all
allocation of physical disk space to logical volumes is performed in units
of these extents.
The extent size within a volume group is chosen at volume group
creation time.
Extent size is always a power of 2 bytes, and may range from 1MB to
256 MB.
Usually, small extents are best, since this determines the amount of
space wasted on each physical volume.
For extremely large disks, the limit on total number of physical extents
per physical volume, or the resulting VGDA size, may require a larger extent
size to be necessary.
.P
The LVM allows more space to be allocated to a physical extent than
is actually used (see the 
.I "Physical Extent Size"
and
.I "Physical Extent Space"
entries in the LVM Record).
The additional space may then be used for relocation of bad blocks without
necessarily causing a long seek.
This feature is optional, and is probably not useful with disk controllers
and drives that perform their own bad block reassignment.
The driver will honor redirects to the additional space, but will not
(in the current version) redirect newly found bad blocks to this space.
.H 2 "Common Bad Block Relocation Pool"
The
.B "common bad block relocation pool"
is used by the LVM to redirect I/O intended for disk blocks that
have developed defects.
Before allocating a block in the bad block pool, the LVM attempts
to have the disk controller perform hardware redirection.
Only when this fails, either due to the disk not implementing hardware
revectoring, or because it has exceeded it's capacity to redirect
failed blocks, will the LVM perform software redirection.
