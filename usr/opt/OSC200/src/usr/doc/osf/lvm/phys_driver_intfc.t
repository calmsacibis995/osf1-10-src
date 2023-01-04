.\" (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
.\" ALL RIGHTS RESERVED
.\" OSF/1 Release 1.0
.H 1 "Physical Disk Device Driver Interface"
The physical layer of the LVM device driver interfaces with the physical
device driver strategy entry points.
.H 2 "Request Format"
The physical device driver strategy entry point is called with a pointer to a
.Cw "buf"
structure (see
.Cw "<sys/buf.h>"
for the full definition of this structure).
The 
.Cw "buf"
structure contains all the information needed to specify an I/O request.
The fields shown below are the used by the device driver strategy routine.
.DS
.Cw
struct buf
{
	long	b_flags;	// command/operation/status
	struct	buf *av_forw;
	struct	buf *av_back;	// position on free list if not BUSY
	long	b_bcount;	// transfer byte count
	short	b_error;	// returned after I/O
	dev_t	b_dev;		// major+minor device name
	caddr_t b_un.b_addr;	// low order core address
	daddr_t	b_blkno;	// block # on device
	long	b_resid;	// words not transferred after error
	struct  proc *b_proc;	// proc doing physical or swap I/O
	int	(*b_iodone)();	// function called by iodone (ASYNC)
	union	{	// these fields reserved for device driver
		long	longvalue;
		void	*pointvalue;
		daddr_t	diskaddr;
		time_t	timevalue;
	} b_driver_un_1, b_driver_un_2;
};
.DE
.P
When the driver has completed the operation (either successfully
or unsuccessfully) it calls
.I "biodone"
to notify the original requester.
.H 3 "b_flags"
The 
.Cw "b_flags"
field in the 
.Cw "buf"
informs the device driver what action should be performed for this
request.
.Cw "b_flags"
is constructed by logical OR-ing zero or more of the
following values:
.TS
tab(;);
lf9 l.
B_WRITE;write to device (pseudo-flag)
B_READ;read from device
B_PHYS;physical IO - directly to/from User space
B_RAW;set by physio for raw transfers
B_WRITEV;perform write verification (may be ignored)
B_HWRELOC;perform hardware relocation (may be ignored)
.TE
The
.Cb "B_READ"
flag indicates that the data is to be read from the device to
memory.
Its absence (sometimes called
.Cb "B_WRITE" )
indicates that data is transferred from memory to the device.
.Cb "B_PHYS"
is used to flag whether the data transfer is to system space
or user space. 
This flag is normally used in setting up the data transfer
for I/O devices that must differentiate between these 2 destination
address spaces.
Not all systems need to pay attention to this bit.
The
.Cb "B_RAW"
flag indicates that this I/O request came through the raw block device
interface
.RI ( physio ).
.P
The
.Cb "B_WRITEV"
and
.Cb "B_HWRELOC"
bits are newly-defined in OSF/1 in order to implement write verification
and hardware defect forwarding.
To take advantage of these new features, a disk driver must be changed 
to respond to these commands.
In order to minimize
.I "required"
changes, the command protocols for these bits have been defined
in such a way that ignoring them is a valid response.
.P
The driver uses the following bits for return codes and internal status.
.TS
tab(;);
lf9 l.
B_ERROR;transaction aborted
B_BAD;bad block revectoring in progress
B_PRIVATE;buffer is not part of the buffer cache
.TE
The driver sets the
.Cb "B_ERROR"
bit prior to calling
.I "biodone"
in order to indicate that
.Cw "b_error"
field is valid, and that a problem was encountered during the I/O operation.
The
.Cb "B_BAD"
and
.Cb "B_PRIVATE"
bits will never be set by the buffer cache, and are reserved for use by
the driver.
.Cb "B_BAD"
is used by some drivers to track software bad sector forwarding
operations.
.Cb "B_PRIVATE"
??
.P
Other bits present in 
.Cw "b_flags"
are not generally used by the device driver
.I "strategy"
routine, though they are sometimes used by other driver routines that
must build their own I/O transactions.
.H 3 "b_bcount"
.Cw "b_bcount"
contains the byte count of the data to be transferred.
The driver should not normally modify this number.
.H 3 "b_dev"
.Cw "b_dev"
contains the device number for the block device that 
is to perform the I/O transaction. 
Usually, the minor number, obtained via
.Cw "minor(bp->b_dev)" ,
.R
is used to encode which controller, drive, and partition is involved
in the trabsaction.
.H 3 "b_un.b_addr"
This member of the "object to be transferred" union indicates the
memory location the driver should reference to transfer data.
.H 3 "b_blkno"
Block number on disk where the transfer is to start.
This block number is usually measured in terms of disk sectors.
For a Berkeley FFS, there is usually more than one sector 
per fragment (small filesystem block).
.H 3 "b_iodone"
This field contains a pointer to function that is invoked by
.I biodone 
on completion of the transaction.
The block driver normally does not examine or modify this value.
A requester may set this field in order to regain control on completion
of an I/O request.
.H 3 "av_forw, av_back"
These pointers are available for the driver to use for maintaining 
chains or lists of requests.
The contents should be ignored on entry, and are ignored by
.I "biodone" .
.H 3 "b_driver_un_1, b_driver_un_2"
These fields are reserved soley for the device driver. 
They should be considered uninitialized on entry to
.I "strategy" ,
and are always ignored by 
.I "biodone" .
They can be used to maintain information between the request initiation
and completion.
It is also common to use the return value fields
.Cw "b_resid"
and
.Cw "b_error"
during I/O processing
The only restriction is that they must be set to the proper
value prior to invoking
.I "biodone" .
.H 3 "b_error"
If an error occurs while processing a request, the driver sets the
.Cb "B_ERROR"
bit and indicates the specific error in the
.Cw "b_error"
field.
The value returned in
.Cw "b_error"
usually gets propogated to the 
.Cw "errno"
variable in the original requesting process.
Its value should be chosen from the definitions in
.Cw "<errno.h>" .
.H 4 "New Error Returns"
In order for higher-level portions of the operating system to use the
new features, more detailed feedback from the disk driver is necessary
concerning request status.
.P
The following new values for the
.Cw "b_error"
field allow drivers to inform upper levels of software of exceptional
conditions.
.TS
tab(;);
lf9 l.
EMEDIA;Hard (uncorrectable) media error
ERELOCATED;Hardware relocation successful
ESOFT;Soft (correctable) media error
.TE
These return codes are not propogated to the calling user through
.Cw "errno" ,
but instead are used to provide additional information for use by
the LVM, the filesystem, or the raw disk driver.
Since these codes are only used in the
.Cw "b_error"
field of the
.Cw "struct buf" ,
they are defined in
.Cw "<sys/buf.h>" .
.H 5 "EMEDIA"
This is an actual error, and is used to distinguish between
ECC or media errors and problems due to missing disks, drive offline,
I/O request out of range, and all of the other things that are now
lumped into
.Cb "EIO" .
.P
After taking whatever corrective action it wants to, the requester
should convert this to
.Cb "EIO"
before returning the request to the user.
.P
The driver is expected to have set the
.Cw "b_resid"
field to indicate how much data was successfully transferred.
The block immediately following the transferred data contains the hard error.
.H 5 "ERELOCATED"
This return code is actually a success indication. 
It is used by block drivers that support hardware defect relocation
to indicate that the requested block reassignment and rewrite was accomplished
successfully.
A device driver that does not support hardware relocation may simply
ignore the
.Cb "B_HWRELOC"
bit in the request, and by failing to supply an
.Cb "ERELOCATED"
return value, indicate it's unwillingness to perform the operation.
A slightly smarter driver can reject any request that contains the 
.Cb "B_HWRELOC"
command, thus saving the I/O traffic of performing a useless write.
Such a driver can simply return success, which will be properly
interpreted by the requester.
.H 5 "ESOFT"
This error indicates that a correctable media error occurred during
the operation.
As the operation actually completed successfully, the requestor
should clear any error indication, possibly after recording soft
error statistics, or taking corrective action.
.H 3 "b_resid"
The device driver sets this field to the number of bytes
.I not
transferred successfully prior to calling
.I "biodone" .
For a request that can be fully satisfied, this field is zero.
.P
If a request begins past the end of the device,
.Cw "b_resid"
is set to
.Cw "b_bcount"
and the error code is set to
.Cb "EOF" .
For reads past the end of device, the field would be set to the
amount of data that could not be read.
Note that this is
.I not
an error 
.Cb "(B_ERROR" " is not set)."
.H 3 "b_proc"
The
.Cw "b_proc"
field contains a pointer to the
.Cw "struct proc"
of the process performing raw I/O.
The driver uses this field to obtain access to the virtual address space of
the target process, since in this case the
.Cw "b_un.b_addr"
field will contain a user virtual address.
.H 2 "Driver Actions"
In general, if the disk driver can perform the requested
operation successfully, it should clear the
.Cw "b_resid"
and
.Cw "b_error"
fields, and the
.Cb "B_ERROR"
flag in the
.Cw "b_flags"
field.
Hardware relocation and write verification are handled somewhat
differently, in order to provide compatibility with
drivers that do not support these functions with minimal changes.
.H 2 "Write Verification"
The caller requests that the device driver perform write 
verification by setting the
.Cb "B_WRITEV"
bit, and clearing
.Cb "B_READ" .
The strategy routine may reject a request that contains both
.Cb "B_READ" 
and
.Cb "B_WRITEV" 
set, as this combination is invalid.
The driver may be able to perform the write, read and check in one
operation, given appropriate hardware support.
Or, it may have to write the data, then read it back to perform the
check as a separate operation.
In either case, the the driver invokes
.I "biodone" 
when the verification is complete.
.P
If the verification phase fails, the driver should return
.Cb "ESOFT"
or
.Cb "EMEDIA"
similar to a normal failure to read data.
.P
If the verification phase succeeds, the driver should clear the
.Cb "B_WRITEV"
bit in the
.Cw "b_flags"
word.
This indicates to the caller that the driver correctly interpreted the
write verification command.
If the
.Cb "B_WRITEV"
bit is still set when 
.I "biodone"
is invoked, the caller is informed that the underlying driver does not
support write verification.
.P
In order to simplify driver logic, the LVM serializes all
I/O to a given block range. 
That is, while an I/O request is pending to a given block, no other
I/O request is initiated.
This guarantees that from the start of the verify (the write phase)
to the completion of the read phase, no intervening write requests
will be processed.
.H 2 "Hardware Defect Relocation"
[ To be provided. ]
