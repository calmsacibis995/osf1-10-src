.\" (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
.\" ALL RIGHTS RESERVED
.\" OSF/1 Release 1.0
.\"- Log:	execution_model.t,v
.\" Revision 1.1.1.2  90/02/23  14:43:16  jeffc
.\" 	Place design documents under source control
.\" 
.H 1 "Logical Volume Manager Implementation"
The logical volume device driver processes all logical I/O.
It converts the logical address to a physical address, handling mirroring
and bad block relocation, and then sends the request to a physical
volume device driver.
.P
Logical volume device driver configuration is performed by the
LVM commands through the
.I "ioctl" (2)
system call which calls the 
.I "lv_ioctl"
entry point of the logical volume device driver.
.fS
.so exemodel.pic
.fC
.FG "LVM Execution Model." \n(H1-
.fE
.P
Figure \n(H1-\n(Fg describes the relationship between the LVM components.
.P
The Logical Volume Device Driver is responsible for
maintaining the on-disk data structures that describe the Volume Group.
In this discussion, it will simply be referred to as "the device driver."
.P
It provides character and block entry
points like the physical disk device driver, with compatible arguments.
Each volume group has a separate entry in the kernel device switch table that
contains the entry points for the device driver and a pointer
to the volume group data structure (described below).
The logical volumes of a volume group are distinguished by their minor numbers.
.fS
.so ioflow.pic
.fC
.FG "Flow of I/O through the Logical Volume Device Driver" \n(H1-
.fE
.P
.nr !! \n(Fg
Figure \n(H1-\n(!! describes the flow of character and block requests
through the device driver.
Raw I/O requests are performed by issuing a
.I read (2)
or
.I write (2)
system call on a character special file for the logical volume.
The read or write system call is processed by the
file system, which calls the driver read or write entry point.
The driver transforms the character request
into a block request building a buffer for the request and calling the
driver strategy entry point.
.P
Block I/O requests are performed by issuing a read or write system
call on a block special file for a logical volume.
These
requests go through the system call interface to the block special file
service routines,
which build buffers for the request and call the driver strategy entry point.
.P
The driver strategy entry point then translates the logical address to a
physical address (handling mirroring and bad block relocation) and
calls the appropriate physical disk device driver.
On completion of the I/O, the physical device driver calls the
.I biodone
routine.
.I biodone
then calls the logical volume driver I/O completion handling routine.
Once this is completed, the logical volume driver calls
.I biodone
to notify the requester that the I/O is completed.
.P
Like most device drivers, the LVM driver is logically split into top and
bottom halves.
The top half contains the
.I open ,
.I close ,
.I read ,
.I write ,
and
.I ioctl
entry points.
The bottom half contains the
.I strategy
entry point - block read and write code.
The top half runs in the context of a user process,
and may sleep, and access the user process address space.
The bottom half of the device driver (potentially) runs in interrupt
context, and is not permitted to assume that any process context exists.
.H 2 "Data Structures"
.H 3 "I/O Request Data Structures"
.H 4 "struct buf"
The
.I strategy
entry point has a single parameter, a pointer to one
or a list of
.Cw buf
structs.
The buf structure is defined in
.Cw "<sys/buf.h>"
and contains all the information about an I/O
request that is needed, including a pointer to the data buffer.
.P
The
.I strategy
routine will accept a list of
.Cw "struct bufs" ,
linked through the
.Cw "av_forw" 
pointer.
This IBM extension to the strategy interface is not currently used
by the file system.
Internally, the driver uses this field to maintain in-process queues.
.P
For requests initiated through
.I strategy ,
.Cw "av_back"
is used by the LVM to link the logical request onto the
.Cw work_Q .
.P
For logical requests initiated by the LVM, for example, mirror cache I/O, 
.Cw "av_back"
is unused?
.P
The logical volume manager uses the driver-defined fields
.Cw b_driver_un_1
and
.Cw b_driver_un_2
to track the state of the request.
It refers to these fields as
.Cw b_work
and
.Cw b_options
respectively.
.P
.Cw "b_work"
is used to hold a bitmask of all pages within the logical track
group affected by this I/O request.
It is initialized in
.Cw lv_workmask ,
and is valid as long as the request is in progress.
.P
.Cw "b_options"
is used to pass I/O option information from the character
device driver entries
.RI ( read ,
.I write,
.I ioctl )
to the strategy routine.
It is only considered valid if the
.Cb B_PRIVATE
flag is set, indicating that the I/O request came through the
driver top half routines.
.H 4 "struct pbuf"
The driver associates one or more physical buf structures
.Cw "(struct pbuf)"
with each
.Cw "struct buf"
and passes them to the appropriate physical device driver.
.P
The
.Cw "struct pbuf"
is defined in
.Cw <lvm/lvmd.h> .
It is a standard
.Cw buf
structure with some additional fields used by the logical volume driver to
track the status of the physical requests that correspond to each logical
I/O request.
.P
The following fields are defined in a
.Cw "struct pbuf" :
.DS
.Cw
.ta (8u*\w' 'u) +(8u*\w' 'u) +(8u*\w' 'u) +(8u*\w' 'u) +(8u*\w' 'u) +(8u*\w' 'u)
struct pbuf {	
	struct buf	pb;		// buf to pass to physical driver
	struct buf	*pb_lbuf;	// corresponding logical request
	void		(*pb_sched) ();	// scheduler policy I/O done
	struct volgrp	*pb_volgrp;	// volume group structure
	struct pvol	*pb_pvol;	// physical volume structure
	struct bad_blk	*pb_bad;	// defects directory entry
	daddr_t		pb_start;	// starting physical address
	uchar_t		pb_mirror;	// current mirror
	uchar_t		pb_miract;	// active mirror mask
	uchar_t		pb_mirbad;	// mask of broken mirrors
	uchar_t		pb_mirdone;	// mask of mirrors done
	uchar_t		pb_miravoid;	// mirror avoidance mask
	uchar_t		pb_swretry;	// number of sw relocation retries
	uchar_t		pb_op;		// operation in progress
	uchar_t		pb_type;	// pbuf type (status area manager)
	long		pb_seqnum;	// VGSA sequence number
};
.DE
.P
Each the fields is used as follows:
.P
.Cw "pb"
is a
.Cw "struct buf"
that is used to communicate with the physical device driver.
The 
.Cw "pb.b_flags" ,
.Cw "pb.b_dev" ,
.Cw "pb.b_blkno" ,
.Cw "pb.b_un.b_addr" ,
and
.Cw "pb.b_bcount" ,
are initialized with appropriate values for the physical I/O request.
The 
.Cw "pb.b_forw"
and
.Cw "pb.b_back"
in this structure are used to link related physical requests to
each other by the mirror scheduling policies.
.P
.Cw "pb.av_forw"
is used to link together requests prior to passing them to the
physical driver.
Since we do not support passing lists to the 
.I strategy
routine, the LVM driver pulls the list apart just prior to calling the
physical driver.
.P
.Cw "pb_lbuf"
is a pointer to the logical request that caused this
physical request.
.P
.Cw "pb_sched"
field contains a pointer to the scheduling policy completion
routine.
When the physical request is completed, the scheduling policy
completion routine is invoked.
This field is initialized by the scheduling policy.
.P
.Cw "pb_volgrp"
is a pointer to the structure describing the volume group that
this request belongs to.
.Cw "pb_pvol"
is a pointer to the structure describing the physical
volume that this request is intended for.
.P
.Cw "pb_bad"
is used to indicate that this request is undergoing bad block
processing.
.P
The driver allocates and manages a pool of
.Cw pbufs .
When a
.Cw pbuf
is in the free pool, the 
.Cw "pb.av_forw"
link is used to link together the available
.Cw pbufs .
.H 3 "Volume Group Data Structures"
The logical volume device driver uses the information in the volume group
descriptors to determine the LVM configuration.
When a volume group is brought on-line,
an in-memory copy of the information in these descriptors is allocated.
.P
This information is used to build data structures that describe
the LVM configuration and the current operating state.
.fS
.so datastruct.pic
.fC
.FG "Data structures describing Logical Volumes." \n(H1-
.fE
.P
.nr !! \n(Fg
Figure \n(H1-\n(!! displays the relationship between the major
Logical Volume Device Driver data structures that decribe logical
volumes:
.Cw volgrp ,
.Cw lvol ,
and
.Cw extent .
.P
There is also additional state information maintained concerning
each logical extent, to track resynchronization.
.P
In addition to viewing the volume group as a collection of 
logical volumes, it is also a collection of physical volumes.
Each physical volume has a
.Cw "struct pvol"
that describes the current state of that physical volume.
.H 4 "struct volgrp"
The volume group structure,
.Cw volgrp ,
is allocated when the volume group it describes is configured.
This structure contains general
information about a volume group, such as the extent size, number of
logical volumes, number of open logical volumes, and a pointer to
an array of logical volume structures.
.P
There is one device switch entry for each volume group defined on
the system.
Each volume group entry contains a pointer to the
.Cw volgrp
data structure describing it.
.H 4 "struct lvol"
The logical volume structure,
.Cw lvol ,
is allocated when the logical volume it describes is activated.
It contains general information about a logical volume needed to perform I/O.
One of
.Cw lvol's
fields is a pointer to a work in progress hash table that contains pointers
to buf stuctures for each I/O request in progress on this logical volume.
This is used to ensure that logical I/O requests to overlapping block
ranges must complete in FIFO order.
.Cw lvol
also contains a pointer to a
physical extent map for each mirror of the logical volume.
.P
The extent maps are allocated when the containing logical volume is opened.
The extent maps contain one entry for each physical extent.
Each entry contains the physical disk address of the
beginning of the extent and a pointer to a
.Cw "struct pvol"
that describes the physical volume that this extent resides on.
.H 4 "struct pvol"
The physical volume structure
.Cw pvol ,
is allocated when its volume group is opened?
It contains general information about one physical volume in a volume
group and a pointer to a bad block directory that describes the known
bad blocks on this physical volume.
.H 2 "Quorum"
In order to guarantee that the LVM is operating with accurate
descriptor and status area information, a quorum of Physical Volumes must
contain identical descriptor information.
Quorum is defined to be 1 more than half of the Physical Volumes
in the group. 
As long as the descriptor area is written to a quorum of Physical Volumes,
it is guaranteed that
.I any
set of more than half of the Physical Volumes in the Volume Group
will always contain at least 1 copy of the latest descriptor area.
.P
The procedure used to guarantee that quorum is maintained is as follows.
There is space reserved for 2 descriptor areas on each Physical Volume.
Any time the descriptor area for the group needs to be updated,
the desciptor area that is not part of the current quorum
is overwritten.
Once the new descriptor area has propogated to all possible physical
volumes, the new descriptor area becomes the quorum
descriptor, if a sufficient number of writes were successful.
This guarantees that if the update does not complete, the Physical
Volume will still contain the valid, up-to-date descriptor 
area, and the Volume Group will still contain the same descriptors 
in the quorum set.
This guarantees that an update to the descriptor area can never
cause the number of physical volumes in the quorum set to
.I decrease .
.P
Once a quorum of Physical Volumes has been written with the new
descriptor area, the operation is permanent: that is, even if 
the system crashes, or some Physical Volumes fail, no set of
greater than half of the Physical Volumes can be selected
that will
.I not
have a copy of the new descriptor area.
[Note: there is a builtin assumption here that the VGDA is
always written using WRITEV, or that it is acceptable to lose
an update if there is
.I exactly
one volume in common between the old and new set
.I and
that one volume developed a new uncorrectable defect in the
newer descriptor area. I think this qualifies as multiple
simultaneous failures.]
.P
If quorum is lost, any I/O that would cause a change to the 
descriptor area cannot proceed.
For example, mirror writes cannot be allowed to proceed,
because the VGSA and MWC would not be guaranteed to
to reflect current mirror state.
Additionally, system administration commands that would
modify the Descriptor Area cannot be executed.
.H 2 "Top Half of Logical Volume Device Driver"
The top half of the Logical Volume Device Driver contains the code
that runs in the context of a process address space.
It contains all the entry points to the device driver except
the strategy routine.
These routines are contained in the source file
.Cw lvm/lv_syscalls.c .
.H 3 "lv_open"
The open entry point 
.RI ( lv_open )
is called by the file system when a logical
volume is mounted, or the device is opened.
.H 3 "lv_close"
The close entry point
.RI ( lv_close )
is called by the file system when a logical
volume is unmounted or when the last
.I close (2)
occurs on the open file corresponding to the device.
.H 3 "lv_read"
The read entry point
. ( lv_read )
is called by the
.I read (2)
system call to translate character I/O requests to block I/O requests.
The read routine simply calls the 
.I "physio"
routine, which verifies that the request is legal, and invokes the 
strategy routine, which handles the logical block I/O requests.
The
.I "physio"
routine may break up the I/O into multiple requests before passing it on to
the strategy routine.
.H 3 "lv_write"
The write entry point
.RI ( lv_write )
is called by the
.I write (2)
system call to translate character I/O requests to block I/O requests.
The write routine simply calls the 
.I "physio"
routine, which verifies that the request is legal, and invokes the 
strategy routine, which handles the logical block I/O requests.
The
.I "physio"
routine may break up the I/O into multiple requests before passing it on to
the strategy routine.
.H 3 "lv_ioctl"
The ioctl entry point
.RI ( lv_ioctl )
implements the majority of the driver programming interface. 
Details of this interface are contained in the "Programming
Interface Specification" chapter.
.H 2 "Bottom Half of Logical Volume Device Driver"
The bottom half of the driver contains the strategy entry point - the
code that is called to processes all logical block requests.
It validates I/O requests, check requests for conflicts on any requests in
progress on overlapping block ranges, translates logical addresses to
physical addresses, handles mirroring and bad block relocation.
This portion of the device driver
is not permitted to access any user process context, or to block.
.P
The bottom half of the driver is organized as several layers:
.BL
.LI
strategy - performs logical request validation, initialization,
and termination; serializes logical requests when their block ranges
overlap.
.LI
mirror consistency manager - ensures that all physical extents
corresponding to a logical extent contain the same data.
.LI
scheduler - schedules physical requests for logical operations and
handles mirroring.
.LI
status area manager - tracks availability of physical volumes and
state of physical extents.
.LI
physical - performs physical request startup and termination.
Handles bad block relocation.
.LE 1
.P
Each logical I/O request goes through these layers
(strategy -> mirror consistency -> scheduler -> status area -> physical)
to the physical device driver.
Once the I/O is complete, the request returns back through each layer 
to handle I/O completion processing
before returning to the original requestor.
In addition, the completion of one request may cause the
initiation or release of other requests at each layer.
.H 3 "Strategy"
The driver strategy routine is called with a buf structure (defined in
.Cw <sys/buf.h> )
that contains information about a logical request to be performed.
See chapter 8 for more information on the strategy interface.
.DS
lv_strategy( list of logical requests )
{
	For each request {
		If the request is invalid,
			call biodone() with error,
			continue.

		compute hash function for request,

		<<Begin work queue critical section.>>

		lv_block( hash queue, pending_Q, request )

		<<End work queue critical section.>>

		if the request was rejected,
			call biodone with error.
	}

	If pendinq_Q is not empty,
		lv_initiate( pendinq_Q ).
}
.DE
.P
To ensure that logical requests to overlapping block ranges
complete in FIFO order, this layer keeps track of all outstanding
requests using a work-in-progress hash queue
.Cw
.R ( work_Q )
.R
for each logical volume.
Whenever a new request arrives, it is blocked until all earlier
requests to overlapping pages have completed.
When each request finishes, those that are blocked waiting for it will
be processed.
.P
Serializing requests at the logical layer is necessary to ensure
that bad block relocation and mirror synchronization is 
performed correctly.
.P
This serialization is performed by the
.I lv_block
and
.I lv_unblock
routines.
.DS
lv_block( hash queue anchor, pending_Q, request )
{
	if the request crosses a track group boundary,
		return an error.

	compute mask of pages affected by this request.

	link request onto work_Q through av_back.

	If no conflicts, and the volume is not paused,
		link request onto pending_Q through av_forw.
}
.DE
.DS
lv_unblock( hash queue anchor, pending_Q, request )
{
	Remove this request from hash chain.

	if logical volume is paused,
		return.

	For all requests on this hash chain after this request {
		If no more conflicts {
			mark as unblocked,
			link request onto pending_Q through av_forw.
		}
	}
}
.DE
.P
The strategy layer also serializes access to the logical volume
configuration data structures: any time an adminstrative command
needs to change information in the VGDA regarding this logical
volume (and hence, the
.Cw "struct lvol"
and it's subordinate structures), 
the logical volume is paused by the
.I lv_pause
routine.
In this way, the lower layers of the driver can always assume
that these configuration-related structures are static while
an I/O request is in progress.
.P
When administrative commands need to change these data structures, the
.I lv_pause
routine causes the
.I lv_block
and
.I lv_unblock
routines to block all requests.
After completion of the changes, the logical volume is restarted
by the
.I lv_continue
routine.
The logical volume is only allowed to be paused for brief periods
to change these data structures; an adminstrative
command cannot wait for I/O transactions to complete while the 
volume is paused.
.DS
lv_pause( )
lv_continue( )
.DE
.P
Once the a request has passed the
.I lv_block
serialization, it is then possible to begin processing the request.
I/O options are initialized from the
.Cw "struct lvol" ,
and resynchronization requests are begun.
.P
The requests that are not rejected at this point are passed
to the next layer - the mirror consistency manager.
.DS
lv_initiate( request_Q )
{
	initialize pending_Q.

	For each logical request on request_Q {

		set up LVM_VERIFY, LVM_NORELOC, and LVM_NOMWC options.

		if write to read-only volume,
			set error,
			lv_complete( lv, request_Q, request ),
			continue.

		if I/O is outside of the LV,
			set error or EOF as appropriate,
			lv_complete( lv, request_Q, request ),
			continue.

		if this operation is a resync request,
			set up all resync data items.

		append this request to the pending_Q
	}
	If requests on pending_Q,
		lv_mwcm( pendinq_Q )
}
.DE
The
.I lv_terminate
function is the entry from the lower layer I/O completion routines
to the strategy layer.
This routine undoes the work that was performed by the strategy
layer when the I/O was initiated.
.P
The completion of a request, either by an error in
.I lv_inititate 
or through completion in
.I lv_terminate ,
may cause other requests that were previously blocked to be able
to proceed.
The
.I lv_complete
routine (using
.I lv_unblock )
will link new entries onto the pending queue which will then be processed.
(If unblocked in
.I lv_terminate ,
they will be offloaded to a thread context to be initiated.)
.DS
lv_terminate( list of logical requests )
{
	For each request {

		if this was a resync operation {
			// perform resync completion processing
			if it was a successful read,
				turn it into a write,
				append to pending_Q,
				continue.
		}
		lv_complete( lvol, pendinq_Q, request )
	}
	if requests on pendinq_Q,
		lv_initiate( pendinq_Q )
}
.DE
.DS
lv_complete( lvol, queue, request )
{
	compute hash function for this request.

	<<Begin work queue critical section.>>

	lv_unblock( hash queue, queue, request )

	if logical volume is pausing,
		decrement completion count.
		if all I/O complete,
			wakeup lv_pause sleeper.

	<<End hash queue critical section.>>

	biodone( request )
}
.DE
.H 3 "Mirror Consistency Manager"
The mirror consistency manager maintains data in the Mirror Write
Cache that allows the LVM to determine whether all the mirror copies
of a given logical track group are identical, even across system
crashes.
.P
The mirror consistency manager guarantees that prior to writing to
a mirrored logical track group, the "in-transition" status of that
track group is recorded in the volume group.
The in-transition status is stored in the Mirror Write Consistency
record (MWC) in each physical volume.
Once the all writes to the track group have completed, the cache
entry is available to other logical track groups.
The entry remains in the cache until reused, so that additional cache
record writes are not needed if there is locality of reference in the
disk write activity.
.P
When a request requires an entry in the MWC, is is placed on a 
wait queue, and the mirror consistency manager writes the updated
MWC to a physical volume.
When the MWC write is complete, the request is released, and may
proceed to the scheduler layer.
.P
In order to initiate the mirror consistency record I/O, the mirror
consistency manager uses a per volume group
.Cw "struct buf"
that is reserved for this purpose.
Since this I/O is initiated below the
.I strategy
conflict resolution layer, it does not interlock with I/O through lvol0,
and can never block waiting for other logical I/O to complete.
.DS
lv_mwcm( list of logical requests )
{
	For each logical request {

		If mirror consistency is not needed {
			link request onto sched_Q through av_forw,
			continue.
		}
		if lv_check_cache( sched_Q, request ) fails {
			// pass failed request back to strategy layer
			lv_terminate( request )
		}
	}

	lv_cache_start( sched_Q ).

	If any requests on sched_Q {
		lv_schedule( sched_Q ).
	}
}
.DE
.DS
lv_mwc_done( list of logical requests )
{
	For each logical request {

		If this request is mwc lbuf {
			// this is a mirror cache write completion
			Mark cache as not inflight, cache changed state.
			For all requests on this pvol_wait_queue,
				mark all changing cache entries as dirty,
				move request from pvol_cache_wait to sched_Q.
			continue;
		}

		If mirror consistency used for this request {
			// this is a logical I/O completion.
			decrement the I/O count for this cache entry,
			If I/O count is zero,
				Mark cache changed state.
		}

		link request onto done_Q.
	}
	if cache changed state {
		Process the cache wait queue.
	}

	If any requests on done_Q,
		lv_terminate( done_Q ).

	lv_cache_start( sched_Q ).

	If any requests on sched_Q,
		lv_schedule( sched_Q ).
}
.DE
.DS
lv_checkcache( request, queue )
{
	If cache entry found for this request {

		lv_cache_use( entry, MRU )

		If this track group is already marked dirty {

			increment track group I/O count,
			link request onto queue through av_forw.

		} Else {
			// This entry is not yet on disk.
			link request onto pvol cache_wait queue
		} 
	} Else {
		If cache is inflight or full {
			link request onto cache_wait queue through av_forw.
		} Else {
			// found a free entry
			find a place to write this entry, possibly failing.
			// request is now on the pvol cache_wait queue.
			fill in the cache entry.
			lv_cache_use ( entry, MRU )
		}
	}
}
.DE
.DS
lv_cache_use( entry, flag )
{
	make this entry MRU or LRU according to flag.
}
.DE
.DS
lv_cache_start( queue )
{
	If the cache is not inflight,
			And it has changed,
		mark cache inflight,
		set up cache I/O to a pvol using mwc lbuf.
		link lbuf onto queue.
}
.DE
.H 3 "Scheduler"
.P
The scheduler layer converts logical requests into one or more physical
requests, and then schedules the physical requests through the physical layer.
.H 4 "Overview"
.P
Logical requests are passed to the scheduler layer through the function
\fIlv_schedule\fR.  This function simply places all of the logical requests on
the physical buffer pending queue (\fIlv_pbuf_pending_Q\fR).  This is the
queue of all logical requests waiting for physical buffers before they can be
scheduled.
.P
Once all of the new requests are placed at the end of the pbuf pending queue,
the function \fIlv_reschedule\fR is called.  This function removes logical
requests from the pending queue, and attempts to allocate physical buffers.
If enough physical buffers for the request can be allocated, then the physical
requests are scheduled.  If not enough buffers can be allocated, then the
logical request is replaced on the front of the pending queue, and processing
of the queue is stopped.  When new physical buffers become free, the queue is
again processed.
.P
After sufficient physical buffers have been allocated the logical request is
passed, through the scheduler switch, to the logical volume's scheduling
policy.  Currently three scheduling policies are supported: reserved,
sequential and parallel.  The reserved policy is used for all I/O operations
to logical volume 0.  All other logical volumes use either the sequential or
parallel policies.  The sequential policy will cause mirror operations to
proceed sequentially.  The parallel policy will cause mirror operations to
proceed in parallel.
.P
When all of the physical I/O corresponding to a logical operation are
completed, the mirror write cache is notified with a call to
\fIlv_mwc_done()\fR.
.H 4 "Allocating Physical Buffers"
.P
Each logical request will need one or more physical buffers in order to be
scheduled.  Mirrored write operations on logical volumes with parallel
scheduling will need one physical buffer allocated for each mirror.  All other
operations only require one physical buffer.
.P
Physical buffers are allocated with the function \fIlv_getpbufs()\fR.  This
function decides how many buffers are necessary for the logical request, and
attempts to allocate them.  The allocated physical buffers are chained
together in a single linked list using the \fIpb.b_forw\fR field of the pbuf
structure.
.P
If not enough physical buffers for a logical request can be allocated, then
the request must pend until physical buffers become available.  This pending
is done in a fair manner.  That is, if there are any pending logical requests,
no new logical requests can be initiated until the pending requests have been
statisfied.  This eliminates a potential starvation situation where some
requests require multiple physical buffers and some require only one physical
buffer.  If the requests that required only one physical buffer were allowed
to proceed ahead of the requests that required multiple physical buffers, then
the ones requiring multiple physical buffers may never be able to proceed.
.H 4 "Scheduling Policies"
Three different scheduling policies are implemented: reserved, sequential and
parallel.  The different scheduling policies are used to control the sequence
and parallelism of writes to different mirrors.  The reserved policy assumes
that there are no mirrors.  The sequential policy performs writes to mirrors
in a sequential mannor, each mirror must be complete before the next mirror is
written.  The parallel policy attempts to get the best performance by writting
all of the mirrors at the same time.
.P
The read algorithms in each policy are also slightly different.  Using the
sequential scheduling policy, reads are performed from mirrors in a predefined
order.  Using the parallel scheduling policy, an attempt is made to optimize
the reads.  This is done by performing reads from the physical volume with the
fewest outstanding I/O operations.
.P
Each scheduling function has a corresponding completion function.  The
completion function is invoked by the physical layer when the I/O operation is
complete.  This completion function allows the scheduler to clean up state, or
schedule further I/O.  For example, the completion function is used by the
sequential scheduler to determine when each mirror has been written, so that
it can start the write of the next mirror.
.P
In addition to performing the scheduling of the physical operations, each of
the scheduling policies is responsible for translating the logical request
into the appropriate physical requests and handling any errors that may be
returned.
.H 5 "Scheduling State"
.P
All of the scheduling policies use fields of the physical buffer structure
.I (struct pbuf)
to maintain the state of the scheduling operation.  This state is used by the
scheduling policies to maintain error information, and knowledge of which
mirrors have been read or written.
.P
The current mirror being accessed is maintained in the
.I pb_mirror
field.  This field will be set before each physical operation.
.P
The
.I pb_miract
field is a bitmask of all of the mirrors that have had a physical operation
initiated for the current logical operation.
.P
The
.I pb_mirbad
field is a bitmask of all of the mirrors that had an error during the actual
I/O operation.
.P
The
.I pb_miravoid
field contains the bitmask of all of the mirrors that were avoided.  Mirrors
are avoided either when the physical volume is missing or the physical extent
is stale.
.P
.I pb_mirdone
is the mask of all of the mirrors that were either avoided or activated.  This
should be the same as the number of mirrors in the logical extent.  It is
not strictly necessary to maintain all of these bitmasks, as some of them can
be generated from the others, it is simply more convenient to do so.
.H 5 "Reserved Policy"
.P
The reserved scheduling policy is used for operations to the volume group
reserved area though logical volume 0.  This is the simplest of all of the
scheduling policies as it is known that logical volume 0 can not have mirrors.
This scheduling policy is implemented by the function
.I lv_reserved().
.P
The logical request is translated into the appropriate physical request
using the
.I lv_xlate()
function, and the physical request is started using the
.I lv_begin()
function.  The completion function is
.I lv_finished().
.P
The only possible error that can occur through the scheduling function is an
attempt to write to a non-exist physical extent.  If this occurs, then an
error is set in the buffer and the error number is set to EIO.  If an error
occurs during the actual I/O, then the error is passed back in the logical
buffer.
.H 5 "Mirrored Sequential Scheduling Policy"
.P
This scheduling policy performs mirrored writes in the order: primary,
secondary, then tertiary.  As each I/O is handled sequentially, only one
physical buffer needs to be allocated regardless of the number of mirrors.
Reads from mirrored partitions are handled in the same order as writes.  The
secondary mirror is only ever read when the primary mirror has failed, and the
tertiary mirror is only read when both the primary and the secondary mirror
have failed.  This policy is implemented by the function
.I lv_sequential().
.P
The completion function for sequential operations is set to either
.I lv_seqwrite()
if the operation is a write, or
.I lv_seqread_done()
if the operation is a read.  The function
.I lv_nextmir()
is called during the scheduling operation to determine the first mirror.  If
there are no available or valid mirrors, then an error is returned.  If a
mirror is available, then the logical operation is translated into a physical
operation, and the physical operation is initiated.
.P
When read operations are completed,
.I lv_seqread_done()
is called by the physical layer.  If there was an error in the operation, then
the next mirror in sequence will be attempted, again with a call to
.I lv_nextmir().
If there were no errors in the operation, and there were errors in a prior
mirror, then the driver will attempt to fix the prior error using the function
.I lv_fixup().
This function will write the data that was successfully read to all mirrors
that failed.
When the read operation is complete, either successfully or unsuccessfully,
the logical operation will be completed by
.I lv_finished().
The only way that the read operation can be unsuccessful is if all of the reads
from all of the mirrors failed.
.P
When write operations are completed,
.I lv_seqwrite()
is called from the physical layer.  If an error occured in the write
operation, the mirror that had the error is remembered for later processing in
the
.I pb_mirbad
field of the physical buffer.  Regardless of whether there was an error or
not, the next mirror is written.  When all of the mirrors have been written,
the scheduler determines if any of the writes failed.  If any of the writes
did fail, then stale extent processing is done before the logical operation
is completed.  If there were no errors, then the logical operation is
completed.
.H 5 "Mirrored Parallel Scheduling Policy"
.P
This scheduling policy performs writes for all mirrors in parallel.  Reads are
done from the "best" available physical volume.  Currently, the best physical
volume is determined to be the one with the fewest number of outstanding I/O
operations.
.P
The first step in performing a parallel write is to initialize all of the
physical buffers.  Previously, there was one physical buffer allocated for
each mirror in the logical volume.  These buffers were chained together using
the
.I pb.b_forw
field of the pbuf structure.  It is possible that not all of the physical
buffers will be necessary for the I/O operation.  This is because some of the
mirrors may be missing or stale.  For each mirror that is valid, the logical
operation is translated into a physical operation for that mirror.  After all
of the physical operations are set up, any unused physical buffers are
deallocated.  If none of the mirrors were valid, then the logical operation is
terminated with an error.  The physical buffers that will be used for the
mirror writes are linked together in a doubly linked list.  This allows each
physical buffer to locate all of the other physical buffers involved in the
logical operation.  Lastly all of the mirror writes are started in parallel.
.P
The completion function for mirror writes is the
.I lv_parwrite_done.
This function will be called by the physical layer for each physical operation
that was initiated.  In other words, each mirror write will be completed by
this function.  When each physical operation completes, the doubly linked list
of physical buffers is checked.  If there are other buffers still on the list,
then there must be I/O still outstanding.  In this case, error information
from the completed operation is folded into the total operation, the physical
buffer is removed from the doubly linked list and is freed.
.P
When the last physical operation is completed (indicated by no other physical
buffers on the doule linked list), the logical operation is completed.  As in
the sequential scheduling policy any errors are first subject to stale extent
processing (see below for stale processing algorithms).
.P
The parallel read case is exactly like the sequential read case, with the
exception of how the mirrors are selected.  Instead of selecting the first
mirror, the best mirror is chosen.  This is determined by the function
.I lv_bestmir().
.H 4 "Selecting the Best Mirror"
.P
The current algorithm for selecting the best mirror is very simple - the one
with the fewest number of outstanding I/O operations.
.P
In the future, this algorithm should be improved to take into account the
location of the disk head and the seek scheduling algorithms of the physical
volumes.  This was not done in the current implementation because this
knowledge is not readily available to the logical volume manager.
.H 4 "Translating Logical Operation to Physical Operation"
.P
The function of translating a logical operation to a physical operation is
done by the function
.I lv_xlate().
This translation converts the logical address <volume group, logical volume,
logical block, mirror number> to a physical address <physical volume, physical
block>.  The key to the translation is the logical extent maps.  The mirror
number and the logical block are used to index into the logical volume's
extent map to find the appropriate extent structure.  The extent structure is
then used to determine the physical volume number, which is used to find the
physical volume.
.P
The determination of the physical block number is different for logical volume
0 and the other logical volumes.  This is because the physical extent numbers
stored in logical volume 0's extent map are meaningless.  The reserved areas
of physical volumes do not have extent numbers.  The conversion of logical
block number to physical block number for logical volume 0 then is simply the
start of the volume group reserved area plus the offset
into the area.  The conversion for logical volumes, other than logical volume
0, is:
.DS
	((physical extent number) X (physical extent size)) +
	    (the start of the user data area on the physical volume) +
	    (the offset within the extent)
.DE
.H 4 "Handling I/O Errors"
.P
The scheduler must handle two types of I/O error conditions when handling
mirrors: failed reads and failed writes.  When a read operations fails on a
mirrored extent, the scheduler attempts to re-write the failed operation from
a successfully read mirror.  When a write operation fails on a mirrored
extent, the failed extents must be marked as stale.
.H 5 "Failed Read Operations"
.P
The function
.I lv_fixup
is called by the scheduler if there are one or more failed read operations
from a 
mirrored extent and one of the mirrors was successfully read.  This
function will attempt to re-write the data obtained from the successful read
to the failed mirrors.  The purpose of this write is to either clear the
errors or cause a bad block revectoring.
.P
When all of the failed mirrors have been re-written, the original logical read
request is completed with no error indication.  This is because, from the user
point of view, the data was successfully read from one of the mirrors.
.H 5 "Failed Write Operations"
.P
Fail write operations must cause the failing extents to be marked stale.  This
is because the data on the failed extents will now be different from the
successful extents.  The function
.I lv_stalepp
performs this function.
.P
There is one special case that must be handled.  If all of the mirrors failed,
then all but one of them are marked stale.  It is not possible, or desirable,
to mark all of the mirrors stale.  This is because the stale bit only means
that mirrored extents are inconsistent - not that they are correct or
incorrect.  In the case where all of the mirrors have failed, one of the
mirrors is chosen to be the consistent version and the others are marked stale.
.P
The stale information is written to disk by the VGSA layer.  When the stale
information has been updated to all of the physical volumes, then the logical
write operation is completed.  The only time that an error is indicated to the
logical operation is if the stale information could not be updated to a
majority of the physical volumes in the volume group (a quorum).  In this case
an error is indicated to the user and 
the mirror consistency cache is informed that stale information could not be
updated.  The mirror consistency cache will not allow the mirror consistency
record for this operation to be freed or reallocated.  See the mirror
consistency cache for a more detailed discussion of how this type of error is
handled.
.H 4 "Re-sychronization Operations"
.P
There is one special case that effects the parallel scheduler, sequential
scheduler and stale extent handler, this is re-synchronization operations.
Re-synchronization operations are operations generated to update stale extents
and make the extents "fresh" again.
.P
See the discussion on re-synchronization operations for a more detailed
description of these special cases.
.H 3 "Status Area Manager"
.P
The status area manager maintains the Volume Group Status Area (VGSA) on disk.
The VGSA describes the current state of physical volumes
(missing or present) and the current state of physical extents (stale or
fresh).  This information is read during volume group initialization and
updated as a result of changes in the volume group configuration or as a
result of I/O errors.
.P
The VGSA works in concert with the mirror consistency
cache to maintain the state of the volume group.  The mirror consistency cache
keeps track of all I/O that has not yet completed.  The VGSA keeps track of
all errors and configuration changes.  In other words, the 
mirror consistency cache records I/O that might have errors and the VGSA
records the known errors.  The mirror consistency cache can not have a record
released until the operation has been successful or the VGSA has recorded
any error.  If, due to a write error, it is impossible to update the VGSA, 
then the mirror consistency record that records the failed transaction can not
be released.
.P
Each physical volume in a volume group contains two copies of the VGSA.
When the VGSA needs to be updated, it is written sequentially to both copies
on all of the physical volumes in the volume group.  The procedure used to
update the physical volumes is known as a "wheel turn".  This algorithm will
be covered in more detail later.
.H 4 "Creating the Volume Group Status Area"
.P
The in-memory copy of the volume group status area is created when the volume
group is created, this is done by the
.I lv_sa_createvgsa
function.  As physical volumes are added to the volume group, the current VGSA
is written to it.  This is done as a result of the configuration change in the
volume group by the function
.I lv_sa_config.
.H 4 "Volume Group Activation"
.P
The only time that the VGSA is read is during activation of a volume group.
The volume group status area manager will select the VGSA with the most recent
time stamp as the one to read.
If there is an error reading this VGSA, then the next most recent VGSA is
read.  This procedure is followed until a VGSA is successfully read, or none
of the VGSAs are left to read.  If no VGSAs can be read, then the volume group
activation will fail.
.P
The function
.I lv_sa_missing
is also called during volume group activation.  This function is used to mark
newly missing physical volumes as missing, and newly present physical volumes
as present.  After the physical volumes are marked either missing or present
the updated VGSA is written to disk.  It is possible, through write errors,
that quorum will be lost during this operation.  If it is, then the volume
group activation will fail.
.H 4 "VGSA Updates or The Wheel"
.P
There are two types of operations used to update the volume group status area.
The first type is a result of a change in the volume group configuration.  The
second type is a result of faile I/O to a mirrored extent.
.P
These two operations are different in the way that they communicate to the
status area manager and the synchronization that is used both internally to
the status area manager and externally.  Configuration operations are
synchronous to the caller - the call does not return until the VGSA has been
written to disk.  Operations as a result of I/O errors are asynchronous to the
caller but synchronous to the logical operation.  The logical operation that
caused in the I/O error will not be allowed to complete until the error has
be marked in the VGSA and written to disk.
.P
The function
.I lv_sa_start
is called to request a change to the VGSA.  This function takes, as arguments,
a physical buffer that represents the change and the type of the change.  The
physical buffer is put on a queue of pending operations - the hold list.
If the VGSA wheel is not already active, then it is activated.  This is done by
calling the function
.I lv_sa_continue.
.P
.I lv_sa_continue
removes requests from the hold list, makes the requested changes to the VGSA
and then places the requests on the active list.  When the hold list has been
emptied, then the next wheel address is calculated.  The wheel is set up as a
simple index into the combination of physical volume number and VGSA index.  A
complete turn of the wheel will cover both VGSAs on all of the physical
volumes.
.P
Each time the VGSA is changed, a sequence number is incremented.  Each wheel
index maintains the sequence number of the VGSA that was last written there.
In addition, each physical buffer on the active list records the sequence
number that contained its change.  After the next wheel index is calculated,
the sequence number saved at that index is compared to the sequence numbers on
the active list.  All requests on the active list that have a sequence number
less than or equal to the one at the wheel index can be marked complete.  This
is because it is known that the request has been written to all of the physical
volumes.
After the active list is checked for completed requests, the VGSA is written
to the next wheel index, and, on successful completion, the wheel index's
sequence number is updated.
.P
This process continues until there are no more
requests on either the hold list or the active list.  When this occurs, the
status area state is marked as inactive.
.H 4 "Configuration Requests to the VGSA"
.P
Any change to the state of a physical volume or a physical extent must be
written to the VGSA before they can be completed.  This includes adding or
deleting a physical volume to a volume group, adding new mirror extents, and
resynchronizing stale extents.  The status area manager is informed of these
changes through the
.I lv_sa_config
function.  This function takes, as arguments, the volume group, the type of
the change and an argument that specifies the physical volumes or physical
extents effected.  The types of configuration requests supported are:
.BL
.LI
.B CNFG_PVMISSING \(em
Mark the specified physical volumes as missing.  The physical volumes are
specified by a null terminated array of pointers to
.I pvol
structures.  It is possible that this command could cause loss of quorum.
.LI
.B CNFG_INSTALLPV \(em
Mark the specified physical volumes a present.  This command is issued when
new physical volumes are added to the volume group, or existing, missing
physical volumes are reattached.
.LI
.B CNFG_FRESHPX \(em
Mark the specified physical extents as fresh.  This command is issued as
the result of a successful resynchronization command.  The extents to be
marked are specified as an array of structures.  Each structure contains the
physical volume number and the physical extent number to be marked.  The array
is terminated by a physical volume number of -1.
.LI 
.B CNFG_STALEPX \(em
Mark the specified physical extents as stale.  This command is issued when new
mirrors are added to existing logical extents.  The extents are specified as
in the CNFG_FRESHPX command.
.LI 
.B CNFG_SYNCWRITE \(em
Synchronously write the VGSA to all of the physical volumes.  This is used by
the mirror consistency manager during mirror recovery operations.  The mirror
consistency manager marks stale extents in the VGSA structure using the
.I lv_sa_setstale
function.  When the appropriate extents have been marked stale in the
in-memory VGSA, it is written to disk using this configuration command.
.LE
.P
Configuration operations do not use the normal method of updating the VGSA.
This is because typical configuration operations require more than one change
to the status area.  Instead of relying on
.I lv_sa_continue
to make the appropriate changes, 
.I lv_sa_config
makes the changes itself.  Before the VGSA can be changed, however, it must be
insured that it is not in the process of being written to disk.  This is the
reason for the synchronization between 
.I lv_sa_config
and
.I lv_sa_continue.
If the VGSA is in the process of being written, the configuration change waits
until the update is complete.  When it is complete, it makes its changes and
restarts the wheel.
.H 3 "Physical Layer"
The bottom layer of the LVDD, the physical layer, handles physical
request startup and termination, including bad block relocation.
This layer calls the physical device driver strategy entry point with a
.Cw "buf"
structure, and is called by 
.I "biodone"
when each physical request is completed.
This layer also performs bad block relocation and
detection/correction of bad blocks when necessary, hiding these
details from the other two layers.
.H 4 "Bad Block Relocation"
The physical layer of the LVDD checks each physical request to see if
there are any known bad blocks in the request.
Each physical volume data structure, 
.Cw "pvol" ,
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
.I "biodone" ()
calls the LVDD physical layer termination routine (specified in 
.Cw "b_iodone"
in buf structure) which will initiate the request with the relocated
block, and then the remaining request.
Once the entire physical
operation is completed, the appropriate scheduler policy routine in
the second layer of the LVDD will be called to start the next phase of
the logical operation.
.H 5 "Detecting and Correcting Bad Blocks"
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
hardware relocation requested.
If the request is a read, an I/O error
will be returned to the original requestor.
.P
If the operation was for a mirrored read, a request to read one of the
mirrors is initiated.
If the second read is successful, then the read
is turned into a write request and the physical device driver is called with 
hardware relocation specified.
.P
If the hardware relocation succeeds, the bad block entry in the LVDD
defects directory and the defects directory on the physical volume are
deleted.
.P
If the hardware relocation fails or the device does not support
hardware relocation, the physical layer of the LVDD
attempts software relocation.
.H 2 "Logical Volume Manager Initialization"
.H 3 "Volume Group Descriptor Reconciliation"
The volume group descriptor area contains essential information for
the Logical Volume Manager.
A quorum set of descriptor areas is maintained, in order to
guarantee that the driver can accurately determine 
the state of the volume group following a system crash.
Reconciliation of the on-disk volume group descriptor
areas takes place when the volume group is brought on-line.
.P
This procedure must ensure that forward progress is always made,
and that once a decision concerning the current Volume Group state
is made, that it is propogated to sufficient Physical Volumes so 
that the decision can never be unmade.
.H 4 "Locate Current Descriptor"
The first step in determining the current Volume Group state is to 
locate the newest volume group descriptor from the attached Physical Volumes.
.DS
For all attached Physical Volumes,
	read primary VG header & VG trailer
	save timestamps in struct pvol.
	if no read errors and header time == trailer time,
		mark this descriptor as valid,
		if this time > newest valid time,
			save this time as the newest valid time.
	else,
		mark this descriptor as invalid.
	read secondary VG header & VG trailer
	save timestamps in struct pvol.
	if no read errors and header time == trailer time,
		mark this descriptor as valid,
		if this time > newest valid time,
			save this time as the newest valid time.
	else,
		mark this descriptor as invalid.
.DE
This procedure never writes to any Physical Volume, so may be repeated
without danger.
It may only fail if no valid descriptors are present on any attached
Physical Volume.
.H 4 "Determine if Quorum is present"
Once the newest Descriptor is chosen, it must be determined if the
attached Physical Volumes form a quorum.
.P
The first step is to set the current configuration of the volume
group to the configuration shown in the chosen descriptor.
This involves detaching physical volumes that are attached, 
but not shown as current members of the volume group.
This can occur if a physical volume is in the process of being 
installed into the group when the system crashes, or if the 
volume was deleted from the group when it was offline.
.DS
	read the newest VGDA into memory
	validate all physical volumes, detaching frauds.
	if number of attached PVS > 1/2 total PVS,
		declare quorum present.
	else,
		fail: no quorum.
.DE
This procedure never writes to any Physical Volume, so may be repeated
without danger.
.P
Failure at this point requires that the administrator
attach or bring online more physical volumes in order to achieve
quorum.
.H 4 "Establish the Quorum Set"
If some physical volumes are missing when the group is activated,
the driver must guarantee that a different configuration 
decision cannot be made if the system crashes and is reactivated
with a different set of physical volumes.
.P
To do this, the chosen descriptor area with a new timestamp must
be propogated to a quorum set of the Physical Volumes.
.DS
	set timestamp to current time
	for all attached Physical Volumes,
		overwrite the older VGDA with the current VGDA.
.DE
Since this step involves writing to the volume group descriptor 
areas, it is not performed until after the VGSA and MWC consistency
is established.
.H 3 "Volume Group Status Area Synchronization"
[To Be Provided.]
.H 3 "Mirror Consistency Recovery"
The Mirror Write Consistency Manager guarantees that prior to
allowing any write to a mirrored logical extent to begin, 
a record of this write is made in a Mirror Consistency
Record contained on one of the physical volumes holding a copy
of the logical extent.
.P
The MWC Record entry is not deleted until all outstanding 
writes have completed, and the new mirror state propogated via
the VGSA if any change occurred.
The most recent MWC Record in the volume group is the most accurate;
older MWC records may contain entries marking track groups as dirty
that are actually clean.
.P
The granularity of the in-transition record is a Logical Track Group.
Each Logical Track Group contains 32 'pages' of data.
The 'page' size used normally corresponds to the VM page size, but
may be smaller if the maximum physical disk driver I/O length
is less than the resulting Logical Track Group size.
.P
The LVM logic performing resynchronization and mirror recovery
using I/O transaction of exactly 1 Logical Track Group; this
allows the strategy layer to block all other I/O to the track group 
while the resync is in progress.
This is also the maximum I/O size allowed by the LVM
.I lv_minphys ()
routine.
.H 4 "Missing Physical Volumes"
Once the VGDA and VGSA have been read from the physical volumes,
the Mirror Write Consistency Manager uses this information to
determine the state of all physical volumes both prior to the system
going down and currently. 
Each phycical volume may be in one of 4 states:
.AL
.LI
Was available, and now available.
.LI
Was available, and now missing.
.LI
Was missing, and still missing.
.LI
Was missing, and now available
.LE
Physical volumes category "Was available, now missing" may have contained
the most recent MWC Record.
In order to guarantee that no mirror can become out of sync 
because the MWC could not be read,
.I all
physical extents that are contained on these newly
missing physical volumes must be considered to have been 
in dirty when the volume group went off line.
Due to the potentially large amount of I/O that can be caused be
marking this number of extents as stale, the Mirror Consistency
Manager will not activate the volume group if there are any 
physical volumes in this state, unless the administrator 
specifies an override flag to the
.Cw LVM_ACTIVATEVG
call.
.P
If activation is allowed with newly-missing physical volumes,
the driver marks all but 1 physical extent as stale for every
mirrored extent contained on the missing physical volume.
.H 4 "Mirror Consistency Record"
Once all newly-missing physical volumes are dealt with, the
Mirror Consistency Manager selects the newest readable MWC Record,
and recovers the logical track groups associated with all the
entries shown in this record.
Recovery is performed by reading any accessible copy of the logical
track group, and writing it to the remaining copies.
If the writes succeeds, all mirrors now contain the same data.
If one or more of the writes fail, the physical extents will be
marked as stale in the VGSA.
In either case, mirror consistency is re-established, and the
MWC cache entry is deleted.
.P
Once all entries in the MWC Record have recovered, the Mirror
Consistecy Manager initialization is complete.
