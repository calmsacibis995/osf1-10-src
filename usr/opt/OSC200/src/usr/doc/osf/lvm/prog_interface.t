.\" (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
.\" ALL RIGHTS RESERVED
.\" OSF/1 Release 1.0
.\"- Log:	execution_model.t,v
.\" Revision 1.1.1.2  90/02/23  14:43:16  jeffc
.\" 	Place design documents under source control
.\" 
.H 1 "Programming Interface Specification"
This section describes the LVM programming interface.  These interface
routines consist largely of
.I ioctl (2)
calls, made by a administrative program to pass
information to and obtain information from the driver.
.P
When managing logical volumes, the main line of division between the LVM
driver code and assistant utilities centers around whether the volumes in
question are active or not.
The LVM driver keeps information concerning the
state of all active logical volumes and volume groups in memory, and
dynamically updates this information as the need arises.
The in-memory copy
of this information is the final word as to the state of the LVM, so changes
made to on-disk structures without the knowledge of the LVM driver will not
be recognized.
.P
Because of this, the LVM must be involved whenever this information is to be
modified.
The driver would also need to verify the information provided to
the LVM from any outside source.
Because the effort on verifying this
information is comparable to generating the information directly, the driver
contains the code to do this directly, eliminating the need for substantial
amounts of library code.
.H 2 "System Interfaces"
All devices respond (some without much effect) to
.I open (),
.I close(),
.I read(),
.I write(),
and
.I ioctl()
system calls.
The LVM is no different; programs may open a logical volume and read
or write to it as a raw device, without any surprises.
The only place the system interface differs is in the device-specific
.I ioctl
commands.
.P
One difference between the LVM and most mass storage device drivers is that
the LVM has a control special file associated with every volume group.
The following section describes a possible implementation.
The actual directory structure is at the discretion of the system administrator.
.P
In the /dev directory, there is a subdirectory of the form vgnn, where nn is
the number of the volume group.
In vg00, for example, ls produces a listing something like this:
.P
.nf
.in +6
/dev/vg00:
brw-------  1 root      10,   0 Jan 15 15:20 group
brw-------  1 root      10,   1 Jan 15 15:25 lv10_01
brw-------  1 root      10,   2 Jan 15 15:26 lv10_02
brw-------  1 root      10,   3 Jan 15 15:27 lv10_03
brw-------  1 root      10,   4 Jan 15 15:28 lv10_04
.in -6
.fi
.P
The special file /dev/vg00/group is major device number 10, minor device
number 0.
The zero'th minor device is always the control device for the
volume group.
Devices 1 through n are the logical volumes in that volume
group.
Although the group device is a control construct only, all system
I/O calls are valid operations.
Logical volume 0 contains the
information about all the physical and logical volumes in the volume group.
It is a pseudo-device, whose contents are the concatenation of the volume group
reserved areas of the physical volumes that make up the volume group.
.P
The LVM must be told which physical volumes make up the volume group
prior to activation.
Each physical volume in the volume group may contain
a Volume Group Descriptor Area (VGDA) which describes, among other things,
which physical volumes are in the volume group.
It is the responsibility
of the Logical Volume management tools to maintain the correlation between
physical device names and physical volume unique ID's.
.H 2 "Ioctl Interface"
The manipulation of volume groups and logical and physical volumes
in the LVM is done through the use of ioctl's.
This is the method by which
commands are sent to the LVM to accomplish such things as adding physical
space to a logical volume.
Each ioctl is described below.
.P
All ioctl's take three arguments: the file descriptor of the device to which
the command refers, the command, and an argument.
.P
The file descriptor usually refers to the volume group control device.
This is a special
file ("group") located in the /dev/vgxx subdirectory whose main purpose is
to provide control access to a given volume group.
This file is opened prior to doing any further manipulation to the volume
group (i.e., adding physical disks, etc.)
The file descriptor returned is
the handle through which the ioctl's know which volume group is being
changed or queried, eliminating the need for a volume group name or ID in
structures passed in or out.
Once this file descriptor is obtained, it is
considered the sole identifier for the volume group in question.
.P
Some ioctl's may also be applied to logical volume devices.
These are noted in the descriptions.
A very small number of commands are
.I only
valid for logical volume devices, and are not valid with the control device.
.P
The command is a constant enumerating the function to be performed.
The command argument to the ioctl function has information encoded describing
the size of the argument pointed to by the third ioctl parameter, and
whether it is an input parameter, an output parameter, or both.
The final argument is either a pointer to a structure for the
specific command, or the data required by the command.
The commands and their arguments are detailed in the following section.
.P
The definitions of the commands, and the arguments passed to
the commands, are contained in the LVM include file,
.Cw /usr/include/lvm/lvm.h .
.P
NOTE: in the following descriptions, the typedefs below are assumed to
apply:
.DS
.Cw
.ta (8u*\w' 'u) +(8u*\w' 'u) +(8u*\w' 'u) +(8u*\w' 'u) +(8u*\w' 'u) +(8u*\w' 'u) +(8u*\w' 'u) +(8u*\w' 'u) +(8u*\w' 'u) +(8u*\w' 'u)
	typedef unsigned long	ulong_t;
	typedef unsigned short	ushort_t;
.DE
.P
An unrecognized command will return
.Cw EINVAL .
.bp
.H 3 "LVM_ACTIVATEVG - Activate the Volume Group"
This ioctl brings a given volume group on-line.
This involves reconciliation of the VGDA's on all attached physical volumes,
and recovery of active mirrors.
Depending on whether
.Cw LVM_ALL_PVS_REQUIRED
or
.Cw LVM_NONMISSING_PVS_REQUIRED
flags are set in the flags word, it may fail if some
of the physical volumes in the volume group are missing.
.DS
#include <lvm/lvm.h>

ioctl(fd, LVM_ACTIVATEVG, &flags)
int fd;
int flags;
.DE
.TS
box, tab(;);
lw(1i) | lw(1.35i) | lw(2.95i).
Field;Allowed Values;Description
=
flags;LVM_ACTIVATE_LVS;Allow logical volume opens
;LVM_ALL_PVS_REQUIRED;Activate fails if any physical volumes are missing
;LVM_NONMISSING_PVS_REQUIRED;T{
Activate fails if any physical volumes are missing which were
not previously known as missing
T}
.TE
.TS
tab(;), box;
c s
lw(1i) | lw(5.08i).
Errors returned
_
Error Code;Meaning
=
ENODEV;No valid VGDAs were found on any physical volume.
ENODEV;Could not find a valid volume group status area.
ENOENT;T{
Quorum was lost while attempting to update the volume group status area.
T}
EEXIST;T{
LVM_ALL_PVS_REQUIRED was specified and at leat one physical volume was missing.
T}
ENOMEM;Insufficient kernel memory to complete request.
ENXIO;Quorum does not exist.
EIO;I/O error while reading the bad block directory.
EINVAL;There is an invalid physical extent in the VGDA's extent map.
ENOTDIR;T{
LVM_NONMISSING_PVS_REQUIRED was specified and a "non-missing" PV has not been
attached.
T}
.TE
.bp
.H 3 "LVM_ATTACHPV - Attach Physical Volume"
This ioctl attaches the physical volume to the volume group.
This operation is analogous to a mount - the named device is opened, and the LVM
maintains a reference to it.
The LVM record is read to determine the vg_id and the pvnum.
The ioctl fails if the volume is a member of another volume group.
.DS
#include <lvm/lvm.h>

ioctl(fd, LVM_ATTACHPV, path)
int fd;
char *path;
.DE
.TS
box, tab(;);
lw(1i) | lw(1.35i) | lw(3.5i).
Field;Allowed Values;Description
=
path;PATH_MAX chars max;T{
NULL terminated physical volume pathname.
T}
.TE
.TS
tab(;), box;
c s
lw(1i) | lw(5.08i).
Errors returned
_
Error Code;Meaning
=
EFAULT;The path does not refer to a valid memory address.
ENXIO;The physical volume is a member of another volume group.
ENOENT;A component of the path does not exist.
ENOTDIR;A component of the path prefix is not a directory.
ENXIO;T{
The path refers to a device that does not exist, or is
not configured into the kernel.
T}
ENOTBLK;The path designates a file that is not a block device.
EACCESS;A component of the path was not accessible.
ELOOP;Too many symlinks were encountered while looking up path.
ENAMETOOLONG;T{
The path is too long, or a component exceeds the
maximum allowable size.
T}
EEXIST;T{
A physical volume with the same physical volume number is
already attached to this volume group.
T}
ENOTTY;T{
Inappropriate ioctl for device - the command was attempted
on a logical volume device rather than the control device.
T}
ENODEV;The physical volume not a member of any volume group.
EXDEV;T{
The physical volume is not a member of the specified volume group.
T}
ENOMEM;Insufficient kernel memory to complete request.
EIO;T{
I/O error while reading the bad block directory or the volume group descriptor
area.
T}
.TE
.bp
.H 3 "LVM_CHANGELV - Change Logical Volume"
This ioctl changes the attributes of a logical volume in a
particular volume group.
It updates the specified logical volume's LVM data
structures and logical volume entry in the descriptor area.
.P
This ioctl may be applied to a logical volume device.
In this case, the minor_num is ignored, and the command
applies to that device.
.DS
#include <lvm/lvm.h>

ioctl(fd, LVM_CHANGELV, &lv_statuslv)
int fd;
struct lv_statuslv {
   ushort_t  minor_num;		/* Device minor number 		     */
   ushort_t  maxlxs;		/* Maximum number of logical extents */
   ushort_t  lv_flags;		/* Logical Volume flag word 	     */
   ushort_t  sched_strat;	/* Mirror write scheduling strategy  */
   ushort_t  max_mirrors;	/* Maximum number of mirrors allowed */
} lv_statuslv;
.DE
.TS
box, tab(;);
lw(1i) | lw(1.35i) | lw(3.5i).
Field;Allowed Values;Description
=
minor_num;1 to LVM_MAXLVS;Logical volume minor number
_
maxlxs;0 to LVM_MAXLXS;T{
New maximum size for logical volume, count of logical extents.
T}
_
.T&
l | l s.
lv_flags;Logical OR of the following constants:
.T&
l | l l.
;LVM_DISABLED;Logical volume unavailable for use
;LVM_NORELOC;New bad blocks are not relocated
;LVM_RDONLY;Read-only logical volume
;LVM_STRICT;T{
Allocate mirrors on distinct physical volumes
T}
;LVM_VERIFY;T{
Verify all writes to the logical volume
T}
;LVM_NOMWC;T{
Do not perform mirror write consistency for this logical volume.
T}
.T&
l | l | l.
_
sched_strat;LVM_SEQUENTIAL;T{
Write mirror copies sequentially
T}
;LVM_PARALLEL;Write mirror copies in parallel
_
max_mirrors;< LVM_MAXCOPIES;T{
Maximum number of mirrors allowed for this logical volume
T}
.TE
.TS
box, tab(;);
c s
lw(1i) | lw(5.08i).
Errors returned
_
Error Code;Meaning
=
EINVAL;The minor_num is invalid.
EINVAL;Max_mirrors was not in the range (0, LVM_MAXCOPIES-1)
EINVAL;The lv_flags contains an unrecognized flag.
EROFS;The volume group not activated.
ENODEV;The minor_num refers to a non-existent logical volume.
EINVAL;T{
The sched_strat argument was not one of LVM_PARALLEL or LVM_SEQUENTIAL.
T}
EBUSY;T{
The maxlxs or maxmirrors argument is smaller than the current allocation for
the logical volume. Must deallocate before changing the logical size.
T}
ENOMEM;Insufficient kernel memory to complete request.
EFAULT;The argument does not refer to a valid memory address
.TE
.bp
.H 3 "LVM_CHANGEPV - Change Physical Volume"
This ioctl changes the attributes of a physical volume.
This ioctl can be used to change the number of defects relocated on this
physical volume, and also, to
disallow or re-allow allocation of extents on the physical volume.
Allocation from a physical volume should be disallowed if the physical
extents of that physical volume are to be migrated to another physical
volume.
.DS
#include <lvm/lvm.h>

ioctl(fd, LVM_CHANGEPV, &lv_changepv)
int fd;
struct lv_changepv {
   ushort_t pv_key;		/* Physical volume identifier 	   */
   ushort_t pv_flags;		/* Logical OR of flags (see chart) */
   ushort_t pv_maxdefects;	/* Maximum number of defects       */
				/* to relocate.			   */
} lv_changepv;
.DE
.TS
box, tab(;);
lw(1i) | lw(1.35i) | lw(3.34i).
Field;Allowed Values;Description
=
pv_key;internally defined;T{
Physical volume identifier assigned by driver
T}
_
.T&
l | l s.
pv_flags;Logical OR of the following constants:
.T&
l | l l.
;LVM_PVNOALLOC;T{
No extent allocation allowed from this physical volume.
T}
;LVM_PVRORELOC;T{
No new defects relocated on this physical volume.
T}
.T&
l | l | l.
_
pv_maxdefects; 0 to bbpool_len;T{
Maximum number of software-relocated defects.
T}
.TE
.TS
box, tab(;);
c s
lw(1i) | lw(5.08i).
Errors returned
_
Error Code;Meaning
=
EINVAL;The pv_flags argument contains unrecognized flags.
ENXIO;The pv_key argument references a non-existing physical volume.
EBUSY;T{
There are more existing defects than could be supported with the max_defects
argument.
T}
EFAULT;The argument does not refer to a valid memory address
ENOTTY;T{
Inappropriate ioctl for device - the command was attempted
on a logical volume device rather than the control device.
T}
.TE
.bp
.H 3 "LVM_CREATELV - Create Logical Volume"
This ioctl creates a logical volume in a particular volume group.
This function uses the supplied information to update a previously unused
entry in the logical volume list.
The index into the list of logical volume
entries corresponds to the minor number of the logical volume.
This ioctl does not do extent allocation.
The
.Cw LVM_EXTENDLV
ioctl must be used to allocate extents for the new logical volume.
.DS
#include <lvm/lvm.h>

ioctl(fd, LVM_CREATELV, &lv_statuslv)
int fd;
struct lv_statuslv {
   ushort_t minor_num;		/* Device minor number		     */
   ushort_t maxlxs;		/* Maximum number of logical extents */
   ushort_t lv_flags;		/* Logical Volume flag word 	     */
   ushort_t sched_strat;	/* Mirror write scheduling strategy  */
   ushort_t max_mirrors;	/* Maximum number of mirrors allowed */
} lv_statuslv;
.DE
.TS
box, tab(;);
lw(1i) | lw(1.35i) | lw(3.5i).
Field;Allowed Values;Description
=
minor_num;1 to LVM_MAXLVS;Logical volume minor number
_
maxlxs;0 to LVM_MAXLXS;New maximum size for logical volume
_
.T&
l | l s.
lv_flags;Logical OR of the following constants:
.T&
l | l l.
;LVM_DISABLED;Logical volume unavailable for use
;LVM_NORELOC;New bad blocks are not relocated
;LVM_RDONLY;Read-only logical volume
;LVM_STRICT;T{
Allocate mirrors on distinct physical volumes
T}
;LVM_VERIFY;T{
Verify all writes to the logical volume
T}
;LVM_NOMWC;T{
Do not perform mirror write consistency for this logical volume.
T}
.T&
l | l | l.
_
sched_strat;LVM_SEQUENTIAL;Write mirror copies sequentially
;LVM_PARALLEL;Write mirror copies in parallel
_
max_mirrors;< LVM_MAXCOPIES;T{
Maximum number of mirrors allowed for this logical volume
T}
.TE
.TS
box, tab(;);
c s
lw(1i) | lw(5.08i).
Errors returned
_
Error Code;Meaning
=
EINVAL;The minor_num is 0.
EDOM;T{
The minor_num is greater than the maximum number of logical volumes in the
volume group.
T}
EEXIST;The minor_num refers to an already existing logical volume.
ENOMEM;Insufficient kernel memory to satisfy the request.
EROFS;The volume group not activated.
ENODEV;The minor_num refers to a non-existent logical volume.
EINVAL;T{
The sched_strat argument was not one of LVM_PARALLEL or LVM_SEQUENTIAL.
T}
EBUSY;T{
The maxlxs or maxmirrors argument is smaller than the current allocation for
the logical volume. Must deallocate before changing the logical size.
T}
ENOTTY;T{
Inappropriate ioctl for device - the command was attempted
on a logical volume device rather than the control device.
T}
.TE
.bp
.H 3 "LVM_CREATEVG - Create Volume Group"
This ioctl creates a volume group and installs the first physical volume.
It initializes the in-memory VGDA for the volume group.
.DS
#include <lvm/lvm.h>

ioctl(fd, LVM_CREATEVG, &lv_createvg)
int fd;
struct lv_createvg {
   char     *path;	/* Pathname of 1st physical volume     */
   lv_uniqueID_t vg_id;	/* Volume group ID to set 	       */
   ushort_t pv_flags;	/* Logical OR of flags (see chart)     */
   ushort_t maxlvs;	/* Max number logical volumes allowed  */
   ushort_t maxpvs;	/* Max number physical volumes allowed */
   ushort_t maxpxs;	/* Max number physical extents allowed */
   ulong_t  pxsize;	/* Physical extent size 	       */
   ulong_t  pxspace;	/* Space allocated for each extent     */
   ushort_t maxdefects;	/* Maximum number of defects	       */
			/* to relocate.			       */
} lv_createvg;

struct lv_uniqueID {
   ulong_t id1;
   ulong_t id2;
};
typedef struct lv_uniqueID lv_uniqueID_t;
.DE
.TS
box, tab(;);
lw(1i) | lw(1.35i) | lw(3.5i).
Field;Allowed Values;Description
=
path;PATH_MAX chars max;T{
NULL terminated physical volume pathname.
T}
_
vg_id;Valid unique ID;Volume group unique ID
_
.T&
l | l s.
pv_flags;Logical OR of the following constants:
.T&
l | l l.
;LVM_PVNOALLOC;T{
No extent allocation allowed from this physical volume.
T}
;LVM_PVRORELOC;T{
No new defects relocated on this physical volume.
T}
.T&
l | l | l.
_
maxlvs;0 to LVM_MAXLVS;T{
Maximum number of logical volumes this volume group will contain.
T}
_
maxpvs;0 to LVM_MAXPVS;T{
Maximum number of physical volumes this volume group will contain.
T}
_
maxpxs;0 to LVM_MAXPXS;T{
Maximum number of physical extents any physical volumes in this volume
group will contain.
T}
_
pxsize;1MB to 256MB;T{
Physical extent size for all extents in this volume group (in bytes).
Must be a power of 2.
T}
_
pxspace;1MB to 256MB;T{
Actual space allocated for each extent (in bytes).
This must be the same or larger than pxsize.
T}
_
maxdefects;0 to bbpool_len;T{
Maximum number of software-relocated defects.
T}
.TE
.TS
box, tab(;);
c s
lw(1i) | lw(5.08i).
Errors returned
_
Error Code;Meaning
=
EINVAL;T{
Invalid argument structure - some field within the
structure contained an invalid value.  Specific checks are made for; zero
volume group ID, maxlvs greater than LVM_MAXLVS, maxpvs greater than MAXPVS,
maxpxs greater than LVM_MAXPXS, 1MB <= pxsize <= 256MB, pxsize <= pxspace,
pxspace is a multiple DEV_BSIZE, pv_flags is valid.
T}
EEXIST;The volume group already exists.
ENOMEM;Insufficient kernel memory to complete request.
ENOSPC;Insufficient space on the volume for the VGRA
ENOENT;The file specified by path does not exist
ENODEV;path does not specify a valid physical volume
EPERM;Permission denied on open of path
EIO;Unable to read the physical volume
ENOTBLK;path designates a file that is not a block device.
ENXIO;The physical volume has no driver configured
EFAULT;The argument does not refer to a valid memory address
ENOTTY;T{
Inappropriate ioctl for device - the command was attempted
on a logical volume device rather than the control device.
T}
.TE
.bp
.H 3 "LVM_DEACTIVATEVG - Deactivate Volume Group"
This ioctl takes a given volume group off-line.
All logical volumes in this volume group must be closed.
The argument is ignored.
.DS
#include <lvm/lvm.h>

ioctl(fd, LVM_DEACTIVATEVG, 0)
int fd;
.DE
.H 3 "LVM_DELETELV - Delete Logical Volume"
This ioctl deletes a logical volume from a particular volume
group.
The logical volume must not be open when this ioctl is called.
All logical extents belonging to this logical volume will be deallocated.
The LVM data structures and the descriptor area are updated.
.DS
#include <lvm/lvm.h>

ioctl(fd, LVM_DELETELV, minor_num)
int fd;
int minor_num;
.DE
.TS
box, tab(;);
lw(1i) | lw(1.35i) | lw(3.5i).
Field;Allowed Values;Description
=
minor_num;1 to LVM_MAXLVS;Logical volume minor number
.TE
.TS
box, tab(;);
c s
lw(1i) | lw(5.08i).
Errors returned
_
Error Code;Meaning
=
EINVAL;minor_num was less than or equal to zero
EROFS;volume group not activated
ENODEV;minor_num refers to a non-existent logical volume
EBUSY;The indicated logical volume is open
ENOTTY;T{
Inappropriate ioctl for device - the command was attempted
on a logical volume device rather than the control device.
T}
.TE
.bp
.H 3 "LVM_DELETEPV - Delete Physical Volume"
This ioctl deletes a physical volume from a specified volume
group.  The physical volume must not contain any extents of a logical
volume for it to be deleted.  If the physical volume contains any extents
of a logical volume, an error code is returned.  In this case, the user must
delete logical volumes or relocate the extents that reside on this
physical volume.  For an empty physical volume, LVM_DELETEPV removes the
entries for this physical volume from the LVM data structures and from the
descriptor area, and it initializes the descriptor area on the physical
volume being deleted.
.DS
#include <lvm/lvm.h>

ioctl(fd, LVM_DELETEPV, pv_key)
int fd;
int pv_key;
.DE
.TS
box, tab(;);
lw(1i) | lw(1.35i) | lw(3.5i).
Field;Allowed Values;Description
=
pv_key;internally defined;Physical volume identifier assigned by driver
.TE
.TS
box, tab(;);
c s
lw(1i) | lw(5.08i).
Errors returned
_
Error Code;Meaning
=
ENOTTY;T{
Inappropriate ioctl for device - the command was attempted
on a logical volume device rather than the control device.
T}
.TE
.bp
.H 3 "LVM_EXTENDLV - Extend Logical Volume"
This ioctl adds extents to a given logical volume.  It allocates
physical extents for the specified logical volume at the physical volume
and physical extent specified as input via the extent list pointer.
It updates the LVM data structures and the descriptor area.
.P
The
.Cw LVM_EXTENDLV
command may mark a newly-allocated extent as stale as it is being
assigned to a logical volume.
This will occur if the indicated logical extent already has at least
one physical extent allocated: since the new copy does not
contain the same data as the existing copy, it is immediately
marked as out of date.
When assigning one or more physical extents to a logical extent
that has no previous allocation, the extents are
.I not
marked as stale.
The assumption is that all the copies of the logical extent
(at this point) contain garbage, so that is unimportant which garbage
the LVM returns when the data is read.
.P
Because of this behavior, establishing a doubly-mirrored logical volume
should be performed in one operation (or one operation per logical
extent) rather than one operation per mirror copy.
In the first case, the newly-allocated logical volume will not
require resynchronization (though reads from blocks that have never
been written will be unpredictable).
Otherwise, the logical volume must be synchronized through
.Cw LVM_RESYNCLX
or
.CW LVM_RESYNCLV
prior to the mirrors being effective.
.P
This command may be applied to a file descriptor corresponding to
a logical volume device.
In this case, the minor_num field is ignored.
.DS
#include <lvm/lvm.h>

ioctl(fd, LVM_EXTENDLV, &lv_lvsize)
int fd;
struct lv_lvsize {
   ushort_t minor_num;	/* Logical Volume minor number	 */
   ushort_t size;	/* Number of extents to allocate */
   lxmap    *extents;	/* Extent list			 */
} lv_lvsize;

struct lxmap {
   ushort_t lx_num;	/* Logical extent number   */
   ushort_t pv_key;	/* Physical volume number  */
   ushort_t px_num;	/* Physical extent number  */
   ushort_t status;	/* Extent status (IGNORED) */
};
typedef struct lxmap lxmap_t;
.DE
.TS
box, tab(;);
lw(1i) | lw(1.35i) | lw(3.5i).
Field;Allowed Values;Description
=
minor_num;1 to LVM_MAXLVS;Logical Volume minor number
_
size;1 to LVM_MAXPXS;Number of extents to add
_
extents;;Pointer to the extent array
=
lx_num;0 to LVM_MAXLXS;Logical extent number to add
_
pv_key;internally defined;Physical volume identifier assigned by driver
_
px_num;0 to LVM_MAXPXS;Physical extent number to add
_
status;any;Ignored
.TE
.TS
box, tab(;);
c s
lw(1i) | lw(5.08i).
Errors returned
_
Error Code;Meaning
=
EFAULT;The argument does not refer to a valid memory address.
EBUSY;An extent described by the extent array is already in use.
ENODEV;The specified logical volume does not exist.
.TE
.bp
.H 3 "LVM_INSTALLPV - Install Physical Volume"
This call installs a physical volume into a specified volume
group.  This involves adding it to the in-memory VGDA for the volume group,
and then updating all active physical volumes in the volume group.  It will
fail if the physical volume is already a member of another volume group.
.DS
#include <lvm/lvm.h>

ioctl(fd, LVM_INSTALLPV, &lv_installpv)
int fd;
struct lv_installpv {
   char     *path;	/* Physical volume pathname	   */
   ulong_t  pxspace;	/* Space allocated for each extent */
   ushort_t pv_flags;	/* Logical OR of flags (see chart) */
   ushort_t maxdefects;	/* Maximum number of defects       */
			/* to relocate.			   */
} lv_installpv;
.DE
.TS
box, tab(;);
lw(1i) | lw(1.35i) | lw(3.5i).
Field;Allowed Values;Description
=
path;PATH_MAX chars max;T{
NULL terminated physical volume pathname.
T}
_
.T&
l | l s.
pv_flags;Logical OR of the following constants:
.T&
l | l l.
;LVM_PVNOALLOC;T{
No extent allocation allowed from this physical volume.
T}
;LVM_PVRORELOC;T{
No new defects relocated on this physical volume.
T}
;LVM_NOVGDA;T{
Physical volume does not contain a Volume Group Descriptor Area
T}
.T&
l | l | l.
_
maxdefects;0 to bbpool_len;T{
Maximum number of software-relocated defects.
T}
.TE
.TS
box,tab(;);
c s
lw(1i) | lw(5.08i).
Errors returned
_
Error Code;Meaning
=
EROFS;The volume group is not active
ENOMEM;Unable to allocate memory
ENODEV;The device is not a valid physical volume
EPERM;Write permission denied on the device
EACCESS;A component of the path was not accessible
EIO;Unable to read the physical volume
ENOTBLK;path designates a file that is not a block device.
ENXIO;The physical volume has no driver configured
EFAULT;The argument does not refer to a valid memory address
ENOTTY;T{
Inappropriate ioctl for device - the command was attempted
on a logical volume device rather than the control device.
T}
.TE
.bp
.H 3 "LVM_OPTIONSET/LVM_OPTIONGET - Raw I/O options"
.Cw LVM_OPTIONSET
sets the I/O options for the raw logical volume device.
The raw device is capable of avoiding specified mirrors on read
operations, set through the
.Cw opt_avoid
field.
This allows a program to access a specific copy of a mirrored logical
volume.
.P
The
.Cw opt_options
field allows the program to temporarily (until the device is closed)
specify that all writes are to be verified
.Cw
.R ( LVM_VERIFY )
.R
or that defect relocation is not to be performed
.Cw
.R ( LVM_NORELOC )
.R
.P
To setthese options permanently, or for the block device, see
.Cw "LVM_CHANGELV" .
The raw I/O options are cleared when the raw device is first opened, and never
have an effect on block device operations.
.P
.Cw LVM_OPTIONGET
obtains the current raw device I/O options, as set by the
.Cw LVM_OPTIONSET
command.
These functions are only valid against the logical volume devices, not the
control device, since it only applies to an open device.
.DS
#include <lvm/lvm.h>

struct lv_option {
    ushort_t opt_avoid;         /* Raw device mirror avoidance mask  */
    ushort_t opt_options;       /* I/O options. see below.           */
};

ioctl(fd, LVM_OPTIONGET, &lv_option)
int fd;
struct lv_option lv_option;

ioctl(fd, LVM_OPTIONSET, &lv_option)
int fd;
struct lv_option lv_option;
.DE
.TS
box, tab(;);
lw(1i) | lw(1.35i) | lw(3.5i).
Field;Allowed Values;Description
=
opt_avoid;0 to LVM_MIRAVOID;Mirrors avoided during raw reads.
_
.T&
l | l s.
opt_options;Logical OR of the following constants:
.T&
l | l l.
;LVM_NORELOC;No bad block relocation performed
;LVM_VERIFY;Verify all writes
.TE
.TS
box, tab(;);
c s
lw(1i) | lw(5.08i).
Errors returned
_
Error Code;Meaning
=
EINVAL;opt_avoid out of range (LVM_OPTIONSET only)
EINVAL;opt_options included invalid bit values (LVM_OPTIONSET only)
EFAULT;The argument does not refer to a valid memory address
ENOTTY;T{
Inappropriate ioctl for device - the command was attempted
on the control device.
T}
.TE
.bp
.H 3 "LVM_QUERYLV - Query Logical Volume"
This ioctl obtains information about a particular logical volume
from the specified volume group.
It verifies that the logical volume is
valid and returns the information requested for its volume group to the
buffer supplied.
.P
This command may be applied to a file descriptor corresponding to
a logical volume device.
In this case, the minor_num field is ignored.
.P
Structure fields are output fields unless marked otherwise.
.DS
#include <lvm/lvm.h>

ioctl(fd, LVM_QUERYLV, &lv_querylv)
int fd;
struct lv_querylv {
   ushort_t minor_num;		/* Device minor number (INPUT)		 */
   ulong_t  numpxs;		/* Current number of physical extents	 */
   ushort_t numlxs;		/* Current number of logical extents	 */
   ushort_t maxlxs;		/* Maximum number of logical extents	 */
   ushort_t lv_flags;		/* Logical Volume flag word		 */
   ushort_t sched_strat;	/* Mirror write scheduling strategy	 */
   ushort_t max_mirrors;	/* Maximum number of mirrors allowed	 */
} lv_querylv;
.DE
.TS
box, tab(;);
lw(1i) | lw(1.35i) | lw(3.5i).
Field;Allowed Values;Description
=
minor_num;1 to LVM_MAXLVS;Logical volume minor number
_
numpxs;0 to LVM_MAXPXS;Current number of physical extents
_
numlxs;0 to LVM_MAXLXS;Current number of logical extents
_
maxlxs;0 to LVM_MAXLXS;Maximum number of logical extents
_
.T&
l | l s.
lv_flags;Logical OR of the following constants:
.T&
l | l l.
;LVM_LVDEFINED;Logical volume entry defined
;LVM_DISABLED;Logical volume unavailable for use
;LVM_NORELOC;New bad blocks are not relocated
;LVM_RDONLY;Read-only logical volume - no writes permitted
;LVM_STRICT;Allocate mirrors on different physical volumes
;LVM_VERIFY;Verify all writes to the logical volume
;LVM_NOMWC;T{
Mirror Consistency not performed on this logical volume
T}
.T&
l | l | l.
_
sched_strat;LVM_SEQUENTIAL;Write mirror copies sequentially
;LVM_PARALLEL;Write mirror copies in parallel
_
max_mirrors;< LVM_MAXCOPIES;T{
Maximum number of mirrors allowed for this logical volume
T}
.TE
.TS
box, tab(;);
c s
lw(1i) | lw(5.08i).
Errors returned
_
Error Code;Meaning
=
EINVAL;minor_num is zero
ENXIO;The volume group is not activated
EFAULT;The argument does not refer to a valid memory address
.TE
.bp
.H 3 "LVM_QUERYLVMAP - Query Logical Volume Physical Extent Map"
This ioctl obtains information from the specified volume group
about the space and extents allocated to a particular logical volume.
It verifies that the logical volume is valid and returns the information
requested for its volume group to the buffer supplied.
The allocation map
must be large enough to accommodate the extent map from the logical
volume.
This information is available from LVM_QUERYLV.
.P
This command may be applied to a file descriptor corresponding to
a logical volume device.
In this case, the minor_num field is ignored.
.DS
#include <lvm/lvm.h>

ioctl(fd, LVM_QUERYLVMAP, &lv_lvsize)
int fd;
struct lv_lvsize {
   ushort_t minor_num;	/* Device minor number		       */
   ulong_t size;	/* size of *extents in logical extents */
   lxmap_t *extents;	/* Extent map for logical volume       */
} lv_lvsize;

struct lxmap {
   ushort_t lx_num;	/* Logical extent number  */
   ushort_t pv_key;	/* Physical volume number */
   ushort_t px_num;	/* Physical extent number */
   ushort_t status;	/* Extent status	  */
};
typedef struct lxmap lxmap_t;
.DE
.TS
box, tab(;);
lw(1i) | lw(1.35i) | lw(3.5i).
Field;Allowed Values;Description
=
minor_num;1 to LVM_MAXLVS;Logical volume minor number
_
currentsize;0 to LVM_MAXLXS;Current size for logical volume
_
allocmap;;Allocation map for logical volume
=
lx_num;0 to LVM_MAXLXS;Logical extent number to add
_
pv_key;internally defined;Physical volume identifier assigned by driver
_
px_num;0 to LVM_MAXPXS;Physical extent number to add
_
.T&
l | l s.
status;Logical OR of the following constants:
.T&
l | l l.
;LVM_PXSTALE;T{
Physical extent is stale (does not contain valid data).
T}
;LVM_PXMISSING;T{
Physical extent is on a missing physical volume.
T}
.TE
.TS
box, tab(;);
c s
lw(1i) | lw(5.08i).
Errors returned
_
Error Code;Meaning
=
EFAULT;The argument does not refer to a valid memory address
.TE
.bp
.H 3 "LVM_QUERYPV - Query Physical Volume"
This ioctl retrieves information about a given physical volume.
It verifies that the physical volume is valid and writes the information
requested to the buffer supplied.
.DS
#include <lvm/lvm.h>

ioctl(fd, LVM_QUERYPV, &lv_querypv)
int fd;
struct lv_querypv {
   ushort_t pv_key;	/* Physical volume identifier (INPUT)	 */
   ushort_t pv_flags;	/* Logical OR of flags (see chart)	 */
   ushort_t px_count;	/* Number of physical extents		 */
   ushort_t px_free;	/* Number of free physical extents	 */
   ulong_t  pxspace;	/* Space allocated for each extent	 */
   dev_t    pv_rdev;	/* current devno assigned to pvol	 */
   ushort_t maxdefects;	/* Current defect limit			 */
   ushort_t bbpool_len;	/* Hard defect limit			 */
} lv_querypv;
.DE
.TS
box, tab(;);
lw(1i) | lw(1.35i) | lw(3.5i).
Field;Allowed Values;Description
=
pv_key;internally defined;Physical volume identifier assigned by driver
_
.T&
l | l s.
pv_flags;Logical OR of the following constants:
.T&
l | l l.
;LVM_PVMISSING;T{
Physical volume is missing from the volume group
T}
;LVM_NOTATTACHED;T{
Physical volume is not attached to a volume group
T}
;LVM_NOVGDA;T{
Physical volume does not contain a Volume Group Descriptor Area
T}
;LVM_PVNOALLOC;T{
No extent allocation allowed from this physical volume.
T}
;LVM_PVRORELOC;T{
No new defects relocated on this physical volume.
T}
.T&
l | l | l.
_
px_count;0 to LVM_MAXPXS;T{
Maximum number of physical extents this physical volume will ever contain
T}
_
px_free;0 to LVM_MAXPXS;T{
Current number of free physical extents on this physical volume
T}
_
pv_rdev;;T{
Device number (major,minor) currently used to access this physical
volume.
Not valid if physical volume is not attached.
T}
.TE
.TS
box, tab(;);
c s
lw(1i) | lw(5.08i).
Errors returned
_
Error Code;Meaning
=
EFAULT;The argument does not refer to a valid memory address
ENODEV;T{
The specified pv_key does not correspond to physical volume
attached to this volume group, i.e., no such device.
T}
ENOTTY;T{
Inappropriate ioctl for device - the command was attempted
on the control device.
T}
.TE
.bp
.H 3 "LVM_QUERYPVMAP - Query Physical Volume Extent Map"
This ioctl returns the map of physical extents on the physical
volume, indicating the logical volume and logical extent to which each
corresponds.
A physical extent which is not currently assigned to a logical
volume will be indicated by an lv_minor value of 0.
.DS
#include <lvm/lvm.h>

ioctl(fd, LVM_QUERYPVMAP, &lv_querypvmap)
int fd;
struct lv_querypvmap {
   ushort_t pv_key;	/* Physical volume identifier	    */
   ushort_t numpxs;	/* Number of physical extents	    */
   pxmap_t  *map;	/* Map of volume's physical extents */
} lv_querypvmap;

struct pxmap {
   ushort_t lv_minor;	/* Logical volume minor number */
   ushort_t lv_extent;	/* Logical extent on volume    */
   ushort_t status;	/* Extent status	       */
};
typedef struct pxmap pxmap_t;
.DE
.TS
box, tab(;);
lw(1i) | lw(1.35i) | lw(3.5i).
Field;Allowed Values;Description
=
pv_key;internally defined;Physical volume identifier assigned by driver
_
numpxs;0 to LVM_MAXPXS;T{
Total number of physical extents on this physical volume
T}
_
map;;Pointer to the physical extent map
=
lv_minor;0 to LVM_MAXLVS;Logical volume minor number
_
lv_extent;0 to MAXLXS;Logical extent number on volume
_
status;LVM_PXSTALE;Physical extent is stale
.TE
.TS
box, tab(;);
c s
lw(1i) | lw(5.08i).
Errors returned
_
Error Code;Meaning
=
EFAULT;The argument does not refer to a valid memory address
ENODEV;T{
The specified pv_key does not correspond to physical volume
attached to this volume group, i.e., no such device.
T}
ENOTTY;T{
Inappropriate ioctl for device - the command was attempted
on the control device.
T}
.TE
.bp
.H 3 "LVM_QUERYPVPATH - Query Physical Volume Using Path"
This ioctl is identical to LVM_QUERYPV, except that it takes a
pathname as the physical volume identifier.  It returns the pv_key rather
than taking it as input.
.DS
#include <lvm/lvm.h>

ioctl(fd, LVM_QUERYPVPATH, &lv_querypvpath)
int fd;
struct lv_querypvpath {
   char     *path;	/* Physical volume pathname (INPUT)	 */
   ushort_t pv_key;	/* Physical volume identifier		 */
   ushort_t pv_flags;	/* Logical OR of flags (see chart)	 */
   ushort_t px_count;	/* Number of physical extents		 */
   ushort_t px_free;	/* Number of free physical extents	 */
   ulong_t  pxspace;	/* Space allocated for each extent	 */
   dev_t    pv_rdev;	/* current devno assigned to pvol	 */
   ushort_t maxdefects;	/* Current defect limit			 */
   ushort_t bbpool_len;	/* Hard defect limit			 */
} lv_querypvpath;
.DE
.TS
box, tab(;);
lw(1i) | lw(1.35i) | lw(3.5i).
Field;Allowed Values;Description
=
path;PATH_MAX chars max;Null-terminated physical volume pathname
_
pv_key;internally defined;Physical volume identifier assigned by driver
_
.T&
l | l s.
pv_flags;Logical OR of the following constants:
.T&
l | l l.
;LVM_PVMISSING;T{
Physical volume is missing from the volume group
T}
;LVM_NOTATTACHED;T{
Physical volume is not attached to a volume group
T}
;LVM_NOVGDA;T{
Physical volume does not contain a Volume Group Descriptor Area
T}
;LVM_PVNOALLOC;T{
No extent allocation allowed on this physical volume
T}
;LVM_PVRORELOC;T{
No new defects relocated on this physical volume.
T}
.T&
l | l | l.
_
px_count;0 to LVM_MAXPXS;T{
Maximum number of physical extents this physical volume will ever contain
T}
_
px_free;0 to LVM_MAXPXS;T{
Current number of free physical extents on this physical volume
T}
pv_rdev;;T{
Device number (major,minor) currently used to access this physical
volume.
Not valid if physical volume is not attached.
T}
.TE
.TS
box, tab(;);
c s
lw(1i) | lw(5.08i).
Errors returned
_
Error Code;Meaning
=
EFAULT;The argument does not refer to a valid memory address
ENOENT;A component of the path does not exist.
ENOTDIR;A component of the path prefix is not a directory.
ENXIO;T{
The path refers to a device that does not exist, or is
not configured into the kernel.
T}
ENOTBLK;path designates a file that is not a block device.
EACCESS;
ELOOP;Too many symlinks were encountered while looking up path.
ENAMETOOLONG;T{
The path is too long, or a component exceeds the
maximum allowable size.
T}
ENODEV;T{
The specified path does not correspond to physical volume
attached to this volume group, i.e., no such device.
T}
ENOTTY;T{
Inappropriate ioctl for device - the command was attempted
on a logical volume device.
T}
.TE
.bp
.H 3 "LVM_QUERYPVS - Query Physical Volumes"
This ioctl retrieves the physical volume list from the volume
group.
It requires the number of volumes in the volume group as input (as
obtained from
.Cw LVM_QUERYVG )
and returns the pv_key for each.
.DS
#include <lvm/lvm.h>

ioctl(fd, LVM_QUERYPVS, &lv_querypvs)
int fd;
struct lv_querypvs {
   ushort_t numpvs;	/* Current physical volume count */
   ushort_t *pv_keys;	/* Pointer to the key list	 */
} lv_querypvs;
.DE
.TS
box, tab(;);
lw(1i) | lw(1.35i) | lw(3.5i).
Field;Allowed Values;Description
=
numpvs;0 to LVM_MAXPVS;T{
Current number of physical volumes in this volume group
T}
_
pv_keys;;Pointer to list of returned keys
.TE
.TS
box, tab(;);
c s
lw(1i) | lw(5.08i).
Errors returned
_
Error Code;Meaning
=
EFAULT;The argument does not refer to a valid memory address
ENOTTY;T{
Inappropriate ioctl for device - the command was attempted
on a logical volume device.
T}
.TE
.bp
.H 3 "LVM_QUERYVG - Query Volume Group"
This ioctl retrieves information about a given volume group.  It
verifies that the specified volume group is valid and writes the information
requested to the buffer supplied.
.DS
#include <lvm/lvm.h>

ioctl(fd, LVM_QUERYVG, &lv_queryvg)
int fd;
struct lv_queryvg {
   ushort_t maxlvs;	/* Max number logical volumes allowed	 */
   ushort_t maxpvs;	/* Max number physical volumes allowed	 */
   ulong_t  pxsize;	/* Physical extent size			 */
   ushort_t freepxs;	/* Number of free extents		 */
   ushort_t cur_lvs;	/* Current logical volume count		 */
   ushort_t cur_pvs;	/* Current physical volume count	 */
   ushort_t vgstate;	/* State of the volume group		 */
} lv_queryvg;
.DE
.TS
box, tab(;);
lw(1i) | lw(1.35i) | lw(3.38i).
Field;Allowed Values;Description
=
vg_id;valid unique ID;Valid volume group unique ID
_
maxlvs;0 to LVM_MAXLVS;T{
Maximum number of logical volumes this volume group will contain.
T}
_
maxpvs;0 to LVM_MAXPVS;T{
Maximum number of physical volumes this volume group will contain.
T}
_
pxsize;1MB to 256MB;T{
Physical extent size for all extents in this volume group.
T}
_
pxspace;1MB to 256MB;T{
Actual space allocated for each extent.  This may be the same as pxsize.
T}
_
freepxs;0 to LVM_MAXPXS;Current number of free extents.
_
cur_lvs;0 to 255;T{
Current number of logical volumes in this volume group.
T}
_
cur_pvs;0 to LVM_MAXPVS;T{
Current number of physical volumes in this volume group.
T}
_
.T&
l | l s.
vg_state;Logical OR of the following constants:
.T&
l | l l.
;LVM_VGACTIVATED;Volume Group is activated.
;LVM_LVSACTIVATED;Logical Volumes are activated.
.TE
.TS
box, tab(;);
c s
lw(1i) | lw(5.08i).
Errors returned
_
Error Code;Meaning
=
EFAULT;The argument does not refer to a valid memory address
.TE
.bp
.H 3 "LVM_REALLOCLV - Move physical extents between logical volumes"
This command atomically removes physical extents from one logical volume
and assigns them to another.
The logical extent number of each physical extent is preserved.
If the destination logical volume already has space allocated for the
indicated logical extents, the new extents will be marked as stale by
the reallocation.
.DS
#include <lvm/lvm.h>

ioctl(fd, LVM_REALLOCLV, &lv_realloclv)
int fd;
struct lv_realloclv {
   ushort_t sourcelv;
   ushort_t destlv;
   ushort_t size;	/* Number of extents to move */
   lxmap_t *extents;
} lv_realloclv;
.DE
.bp
.H 3 "LVM_REDUCELV - Reduce Logical Volume"
This ioctl removes extents from a given logical volume
It deallocates a logical extent for the specified logical volume at the
physical volume, physical extent specified as input via
the extent list pointer.
It updates the LVM data structures and the descriptor area.
.P
This command may be applied to a file descriptor corresponding to
a logical volume device.
In this case, the minor_num field is ignored.
.DS
#include <lvm/lvm.h>

ioctl(fd, LVM_REDUCELV, &lv_lvsize)
int fd;
struct lv_lvsize {
   ushort_t minor_num;	/* Logical Volume minor number	   */
   ushort_t size;	/* Number of extents to deallocate */
   lxmap_t  *extents;	/* Extent list			   */
} lv_lvsize;

struct lxmap {
   ushort_t lx_num;	/* Logical extent number   */
   ushort_t pv_key;	/* Physical volume number  */
   ushort_t px_num;	/* Physical extent number  */
   ushort_t status;	/* Extent status (IGNORED) */
};
typedef struct lxmap lxmap_t;
.DE
.TS
box;
lw(1i) | lw(1.35i) | lw(3.5i).
Field	Allowed Values	Description
=
minor_num	1 to 255	Logical Volume minor number
_
size	1 to LVM_MAXPXS	Number of extents to remove
_
extents		Pointer to the extent array
=
lx_num	0 to LVM_MAXLXS	Logical extent number to remove
_
pv_key	internally defined	Physical volume identifier assigned by driver
_
px_num	0 to LVM_MAXPXS	Physical extent number to remove
_
status	any	Ignored
.TE
.TS
box, tab(;);
c s
lw(1i) | lw(5.08i).
Errors returned
_
Error Code;Meaning
=
EFAULT;The argument does not refer to a valid memory address
.TE
.bp
.H 3 "LVM_REMOVEPV - Remove Physical Volume"
This command temporarily removes a physical volume from the
volume group.
If the volume group is active, the physical volume state is changed to missing.
The physical volume device is closed.
This command is the inverse (effectively) of
.Cw LVM_ATTACHPV .
.DS
#include <lvm/lvm.h>

ioctl(fd, LVM_REMOVEPV, &pv_key)
int fd;
int pv_key;
.DE
.bp
.H 3 "LVM_RESYNCLV - Resynchronize Logical Volume"
This ioctl resynchronizes a logical volume.
Essentially, for every logical extent in the logical volume that has a
physical extent in the LVM_PXSTALE state, the extent is updated from a
mirror copy.
If successful, the the LVM_PXSTALE state is cleared.
.P
This command may be applied to a file descriptor corresponding to
a logical volume device.
In this case, the minor_num argument is ignored.
.DS
#include <lvm/lvm.h>

ioctl(fd, LVM_RESYNCLV, &minor_num)
int fd;
int minor_num;	/* Logical volume minor number	 */
.DE
.TS
box;
lw(1i) | lw(1.35i) | lw(3.5i).
Field	Allowed Values	Description
=
minor_num	1 to 255	Logical Volume minor number
.TE
.TS
box, tab(;);
c s
lw(1i) | lw(5.08i).
Errors returned
_
Error Code;Meaning
=
.TE
.bp
.H 3 "LVM_RESYNCLX - Resynchronize Logical Extent"
This ioctl initiates mirror resynchronization for all physical
extents in a logical extent that are in the LVM_PXSTALE state.
When resynchronization is complete, these extents will be in the LVM_ACTIVE
state.
.DS
#include <lvm/lvm.h>

ioctl(fd, LVM_RESYNCLX, &lv_resynclx)
int fd;
struct lv_resynclx {
   ushort_t minor_num;	/* Logical volume minor number	 */
   ushort_t lx_num;	/* Logical extent to resync	 */
} lv_resynclx;
.DE
.TS
box;
lw(1i) | lw(1.35i) | lw(3.5i).
Field	Allowed Values	Description
=
minor_num	1 to 255	Logical Volume minor number
_
lx_num	0 to LVM_MAXLXS	Logical extent number to resync
.TE
.TS
box, tab(;);
c s
lw(1i) | lw(5.08i).
Errors returned
_
Error Code;Meaning
=
EFAULT;The argument does not refer to a valid memory address
.TE
.bp
.H 3 "LVM_RESYNCPV - Resynchronize Physical Volume"
This ioctl resynchronizes a physical volume.
For each physical
extent on the physical volume that is in the LVM_PXSTALE state, it
resynchronizes the logical extent.
.DS
#include <lvm/lvm.h>

ioctl(fd, LVM_RESYNCPV, &pv_key)
int fd;
int pv_key;	/* Physical volume identifier	 */
.DE
.TS
box;
lw(1i) | lw(1.35i) | lw(3.5i).
Field	Allowed Values	Description
=
pv_key	internally defined	Physical volume identifier assigned by driver
.TE
.TS
box, tab(;);
c s
lw(1i) | lw(5.08i).
Errors returned
_
Error Code;Meaning
=
ENOTTY;T{
Inappropriate ioctl for device - the command was attempted
on a logical volume device.
T}
.TE
.bp
.H 3 "LVM_SETVGID - Set the Volume Group ID"
This routine sets the volume group ID for the volume group
implied by the file descriptor.  It fails if the volume group already has a
volume group ID and attached physical volumes.  It is a necessary precursor
to the LVM_ATTACHPV ioctl.  If the unique ID passed in is zero, it is stored.
.DS
#include <lvm/lvm.h>

ioctl(fd, LVM_SETVGID, &lv_setvgid)
int fd;
struct lv_setvgid {
   lv_uniqueID_t vg_id;	/* Volume group unique identifier */
} lv_setvgid;

struct lv_uniqueID {
   ulong_t id1;
   ulong_t id2;
};
typedef struct lv_uniqueID lv_uniqueID_t;
.DE
.P
The unique ID should be set to a globally unique number.
.TS
box, tab(;);
c s
lw(1i) | lw(5.08i).
Errors returned
_
Error Code;Meaning
=
EFAULT;The argument does not refer to a valid memory address
ENOTTY;T{
Inappropriate ioctl for device - the command was attempted
on a logical volume device rather than the control device.
T}
.TE
