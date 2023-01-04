.\"
.\" *****************************************************************
.\" *                                                               *
.\" *    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
.\" *                                                               *
.\" *   All Rights Reserved.  Unpublished rights  reserved  under   *
.\" *   the copyright laws of the United States.                    *
.\" *                                                               *
.\" *   The software contained on this media  is  proprietary  to   *
.\" *   and  embodies  the  confidential  technology  of  Digital   *
.\" *   Equipment Corporation.  Possession, use,  duplication  or   *
.\" *   dissemination of the software and media is authorized only  *
.\" *   pursuant to a valid written license from Digital Equipment  *
.\" *   Corporation.                                                *
.\" *                                                               *
.\" *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
.\" *   by the U.S. Government is subject to restrictions  as  set  *
.\" *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
.\" *   or  in  FAR 52.227-19, as applicable.                       *
.\" *                                                               *
.\" *****************************************************************
.\"
.\"
.\" HISTORY
.\"
.\" Use eqn, tbl, and -ms
.\" $XConsortium: xdmcp.ms,v 1.18 91/07/28 15:48:25 rws Exp $
.EQ
delim @@
define oc % "\\fR{\\fP" %
define cc % "\\fR}\\fP" %
.EN
.de PT
..
.de BT
..
.fp 6 I
.fp 7 C
.fp 8 CB
.ps 10
.nr PS 10
\&
.TL
\s+2\fBX Display Manager Control Protocol\fP\s-2
.sp
Version 1.0
.sp
MIT X Consortium Standard
.sp
X Version 11, Release 5
.AU
Keith Packard
.AI
X Consortium
Laboratory for Computer Science
Massachusetts Institute of Technology
.AB
.LP
Since the X Display Manager (xdm) may be used to manage remote displays (such
as X terminals), a protocol for requesting service over the network is
needed.  Since the display manager is a scarce resource, the X Display
Manager Control Protocol (also called XDMCP) is designed to use unreliable
datagrams and places the bulk of the burden for sequencing and retransmission
on the display.  A standard byte order and synchronous responses reduces the
complexity of the protocol.
.AE
.LP
.DS C
Copyright \(co 1989 by the Massachusetts Institute of Technology
.DE
.sp 3
.LP
Permission to use, copy, modify, and distribute this documentation for any
purpose and without fee is hereby granted, provided that the above copyright
notice and this permission notice appear in all copies.  MIT makes no
representations about the suitability for any purpose of the information in
this document.  This documentation is provided ``as is'' without express or
implied warranty.
.de PT
.ie o .tl 'XDMCP''X Display Manager Control Protocol '
.el .tl 'X Display Manager Control Protocol ''XDMCP'
..
.bp 1
.de BT
.tl ''\fB % \fP''
..
.NH 1
Purpose and Goals
.LP
The purpose of XDMCP is to provide a uniform mechanism for an autonomous
display to request login service from a remote host.  By "autonomous" we mean
the display consists of hardware and processes that are independent of any
particular host where login service is desired.  (For example, the server
cannot simply be started by a fork/exec sequence on the host.)  An "X terminal"
(screen, keyboard, mouse, processor, network interface) is a prime example of
an autonomous display.
.LP
From the point of view of the end user, it is very important to make
autonomous displays as easy to use as traditional hardwired character
terminals.  Specifically, you can typically just power on a hardwired
terminal, and be greeted with a login prompt.  The same should be possible
with autonomous displays.  However, in a network environment with multiple
hosts, the end user may want to choose which host(s) to connect to.  In an
environment with many displays and many hosts, a site administrator may want
to associate particular collections of hosts with particular displays.  We
would like to support the following options:
.RS
.LP
The display has a "hardwired" host that it should connect to.  It should be
possible to "power on" the display and receive a login prompt, without user
intervention.
.LP
Any one of several hosts on a (sub)net may be acceptable for accepting login
from the display.  (For example, the user's file systems can be mounted onto
any such host, providing comparable environments.)  It should be possible
for the display to "broadcast" to find such hosts, and to have the display
either automatically choose a host, or present the possible hosts to the
user for selection.
.LP
The display has a fixed set of hosts that it can connect to.  It should be
possible for the display to have that set "wired in", but it should also be
possible for a site administrator to be able to maintain host sets for a
large number of displays using a centralized facility, without having to
interact (physically or electronically) with each individual display.
Particular hosts should be allowed to refuse login service, based on
whatever local criteria are desired.
.RE
.LP
The control protocol should be designed in such a way that it can be used over
a reasonable variety of communication transport layers.  In fact, it is quite
desirable if every major network protocol family that supports the standard X
protocol is also capable of supporting XDMCP, since the end result of XDMCP
negotiation will be standard X protocol connections to the display.  However,
since the number of displays per host may be large, a connection-based protocol
appears less desirable than a connection-less protocol.
.LP
In order to keep the burden on displays at a minimum (since display cost is not
a factor that can be ignored), it is desirable that displays not be required to
maintain permanent state (across power cycles) for the purposes of the control
protocol, and it is desirable to keep required state at a minimum while the
display is powered on.
.LP
Security is an important consideration, and must be an integral part of the
design.  The important of security goals in the context of XDMCP are:
.RS
.LP
It should be possible for the display to verify that it is communicating
with a legitimate host login service.  Since the user will present
credentials (e.g. password) to this service, it is important to avoid spoof
attacks.
.LP
It should be possible for the display and the login service to negotiate the
authorization mechanism to be used for the standard X protocol.
.LP
It should be possible to provide the same level of security in verifying the
login service as is provided by the negotiated authorization mechanism.
.LP
Since there are no firm standards yet in the area of security, XDMCP must be
flexible enough to accomodate a variety of security mechanisms.
.RE
.NH 1
Overview of the protocol
.LP
XDMCP is designed to provide authenticated access to display management
services for remote displays.  A new network server, called a \fBDisplay
Manager\fP will use XDMCP to communicate with displays to negotiate the
startup of X sessions.  The protocol allows the display to authenticate the
manager.  It also allows most of the configuration information to be
centralized with the manager, to ease the burden of system administration in
a large network of displays.  The essential goal is to provide plug-and-play
services similar to those provided in the familiar mainframe/terminal world.
.LP
Displays may be turned off by the user at any time.  Any existing session
running on a display which has been turned off must be identifiable.  This
is made possible by requiring a three-way handshake to start a session.  If
the handshake succeeds, any existing session is terminated immediately and a
new session started.  There is the problem (at least with TCP) that
connections may not be closed when the display is turned off.  In most
environments, the manager should reduce this problem by periodically XSync'ing
on its own connection, perhaps every five to ten minutes, and terminating the
session if its own connection ever closes.
.LP
Displays should not be required to retain permanent state for purposes of
the control protocol.  One solution to packets received out of sequence
would be to use monotonically-increasing message identifiers in each message
to allow both sides to ignore messages which arrive out-of-sequence.  For
this to work, displays would at a minimum have to increment a stable "crash
count" each time they are powered on, and use that number as part of a
larger sequence number.  But if displays cannot retain permanent state this
cannot work.  Instead, the manager assumes the responsibility for permanent
state by generating unique numbers which identify a particular session and
the protocol simply ignores packets which correspond to an invalid session.
.LP
The Manager must not be responsible for packet reception.  To prevent the
Manager from becoming stuck because of a hostile display, no portion of the
protocol requires the Manager to retransmit a packet.  Part of this means
that any valid packet which the Manager does receive \fImust\fP be
acknowledged in some way, to prevent the display from continuously resending
packets.  The display can keep the protocol running as it will always know
when the Manager has received (at least one copy of) a packet.  On the
Manager side, this means that any packet may be received more than once (if
the response was lost) and duplicates must be ignored.
.NH 1
Data Types
.LP
XDMCP packets contain several types of data.  Integer values are always
stored most significant byte first in the packet (``Big Endian'' order).
As XDMCP will not be used to transport large quantities of data, this
restriction will not substantially hamper the efficiency of any
implementation.  Also, no padding of any sort will occur within the packets.
.TS
expand;
c c c
c c c
l l l.
Type Name	Length	Description
	(in bytes)
CARD8	1	A single byte unsigned integer
CARD16	2	Two byte unsigned integer
CARD32	4	Four byte unsigned integer
ARRAY8	n+2	This is actually a CARD16 followed by
		a collection of CARD8.  The value of the CARD16
		field (n) specifies the number of CARD8 values to
		follow
ARRAY16	2*m+1	This is a CARD8 (m) which specifies the
		number of CARD16 values to follow
ARRAY32	4*l+1	This is a CARD8 (l) which specifies the
		number of CARD32 values to follow
ARRAYofARRAY8	?	This is a CARD8 which specifies the
		number of ARRAY8 values to follow.
		
.TE
.NH 1
Packet Format
.LP
All XDMCP packets have the following information:
.TS
expand;
c c c c
c c c c
_ _ _
| c l l | c
| c l l | c
| c l l | c
_ _ _
c l l c.
Length in	Field	Description of field
Bytes	Type	

2	CARD16	version number
2	CARD16	opcode	packet header
2	CARD16	n = length of remaining data in bytes

n	???	packet-specific data
.TE
.LP
The fields are as follows:
.LP
Version Number
.RS
This specifies the version of XDMCP that generated this packet in
case changes in this protocol are required.  Displays and
managers may choose to support older versions for compatibility.
This field will initially be 1.
.RE
.LP
Opcode
.RS
This specifies what step of the protocol this packet represents and should
contain one of the following values (encoding provided in section below):
\f8BroadcastQuery\fP, \f8Query\fP, \f8IndirectQuery\fP, \f8ForwardQuery\fP,
\f8Willing\fP, \f8Unwilling\fP, \f8Request\fP, \f8Accept\fP, \f8Decline\fP,
\f8Manage\fP, \f8Refuse\fP, \f8Failed\fP, \f8KeepAlive\fP, \f8Alive\fP.
.RE
.LP
Length of data in bytes
.RS
This specifies the length of the information following the first 6 bytes.
Each packet-type has a different format, and will need to be separately
length-checked against this value.  As every data item has either an
explicit length, or an implicit length, this can be easily accomplished.
Packets that have too little or too much data should be ignored.
.RE
.LP	
Packets should be checked to make sure that they satisfy the following
conditions:
.RS
.IP 1
They must contain valid opcodes.
.IP 2
The length of the remaining data should correspond to the sum of the 
lengths of the individual remaining data items.
.IP 3
The
\f7opcode\fP
should be expected (a finite state diagram is given
in a later section).
.IP 4
If the packet is of type \f8Manage\fP or \f8Refuse\fP, the \f7Session ID\fP
should match the value sent in the preceding \f8Accept\fP packet.
.RE
.NH 1
Protocol
.LP
Each of the opcodes is described below.  Since a given packet type is only
ever sent one way, each packet description below indicates the direction.
Most of the packets have additional information included beyond the
description above.  The additional information is appended to the packet
header in the order described without padding, and the length field is
computed accordingly.
.LP
\f8Query\fP
.br
\f8BroadcastQuery\fP
.br
\f8IndirectQuery\fP
.RS
Display \(-> Manager
.br
Additional Fields:
.RS
\f7Authentication Names\fP:
ARRAYofARRAY8
.RS
A list of authentication names which the display supports.  The manager will
choose one of these and return it in the \f8Willing\fP packet.
.RE
.RE
Semantics:
.RS
A \f8Query\fP packet is sent from the display to a specific host to ask if
that host is willing to provide management services to this display.  The
host should respond with \f8Willing\fP if it is willing to service the
display or \f8Unwilling\fP if it is not.
.LP
A \f8BroadcastQuery\fP packet is similar to the \f8Query\fP packet except
that it is intended to be received by all hosts on the network (or
sub-network).  However, unlike \f8Query\fP requests, hosts that are not
willing to service the display should simply ignore \f8BroadcastQuery\fP
requests.
.LP
An \f8IndirectQuery\fP packet is sent to a well-known manager which forwards
the request to a larger collection of secondary managers using
\f8ForwardQuery\fP packets.  In this way, the collection of managers which
respond can be grouped on other than network boundaries; the use of a
central manager reduces system administrative overhead.  The primary manager
may also send a \f8Willing\fP packet in response to this packet.
.LP
Each packet type has slightly different semantics:
.IP
The \f8Query\fP packet is destined only for a single host.  If the display
is instructed to \f8Query\fP multiple managers, it will send multiple
\f8Query\fP packets.  The \f8Query\fP packet also demands a response from
the manager, either \f8Willing\fP or \f8Unwilling\fP.
.IP
The \f8BroadcastQuery\fP packet is sent to many hosts.  Each manager which
receives this packet will not respond with an \f8Unwilling\fP packet.
.IP
The \f8IndirectQuery\fP packet is sent to only one manager, with the request
that the request be forwarded to a larger list of managers using
\f8ForwardQuery\fP packets.  This list is expected to be maintained at one
central site to reduce administrative overhead.  The function of this packet
type is similar to \f8BroadcastQuery\fP except that \f8BroadcastQuery\fP is
not forwarded.
.RE
Valid Responses:
.RS
\f8Willing\fP, \f8Unwilling\fP
.RE
Problems/Solutions:
.RS
Problem:
.RS
Not all managers receive the query packet.
.RE
.RS
Indication:
.RS
none if \f8BroadcastQuery\fP or \f8IndirectQuery\fP was sent, else failure
to receive \f8Willing\fP.
.RE
Solution:
.RS
Repeatedly send the packet while waiting for user to choose a manager.
.RE
.RE
.RE
Timeout/Retransmission policy:
.RS
An exponential backoff algorithm should be used here to reduce network load
for long-standing idle displays.  Start at 2 seconds, back off by factors of
2 to 32 seconds and discontinue retransmit after 126 seconds.  The display
should reset the timeout when user-input is detected.  In this way, the
display will ``wakeup'' when touched by the user.
.RE
.RE
.LP
\f8ForwardQuery\fP
.RS
Primary Manager \(-> Secondary Manager
.br
Additional Fields:
.RS
\f7Client Address\fP:
ARRAY8
.RS
The network address of the client display.
.RE
\f7Client Port\fP:
ARRAY8
.RS
An identification of the client task on the client display.
.RE
\f7Authentication Names\fP:
ARRAYofARRAY8
.RS
This is a duplicate of \f7Authentication Names\fP array which was received
in the \f8IndirectQuery\fP
packet.
.RE
.RE
Semantics:
.RS
When primary manager receives a \f8IndirectQuery\fP packet, it is
responsible for sending \f8ForwardQuery\fP packets to an appropriate list of
managers which can provide service to the display using the same network
type as the one the original \f8IndirectQuery\fP packet was received from.
The \f7Client Address\fP and \f7Client Port\fP fields must contain an
address which the secondary manager can use to reach the display also using
this same network.  Each secondary manager sends a \f8Willing\fP packet to
the display if it is willing to provide service.
.LP
\f8ForwardQuery\fP packets are similar to \f8BroadcastQuery\fP packets in
that managers which are not willing to service particular displays should
not send a \f8Unwilling\fP packet.
.RE
Valid Responses:
.RS
\f8Willing\fP
.RE
Problems/Solutions:
.RS
Identical to \f8BroadcastQuery\fP
.RE
Timeout/Retransmission policy:
.RS
Like all packets sent from a manager, this packet should never be
retransmitted.
.RE
.RE
.RE
.LP
\f8Willing\fP
.RS
Manager \(-> Display
.br
Additional Fields:
.RS
\f7Authentication Name\fP:
ARRAY8
.RS
This specifies the authentication method, selected from the list offered in
the \f8Query\fP, \f8BroadcastQuery\fP or \f8IndirectQuery\fP packet that the
manger expects the display to use in the subsequent \f8Request\fP packet.
This choice should remain as constant as feasible so that displays which
send multiple \f8Query\fP packets can use the \f7Authentication Name\fP from
any \f8Willing\fP packet which arrives.
.LP
The display is free to ignore managers which request an insufficient level
of authentication.
.RE
\f7Hostname\fP:
ARRAY8
.RS
A human readable string describing the host from which the packet was sent.
The protocol specifies no interpretation of the data in this field.
.RE
\f7Status\fP:
ARRAY8
.RS
A human readable string describing the ``status'' of the host.  This could
include load average/number of users connected or other information.  The
protocol specifies no interpretation of the data in this field.
.RE
.RE
Semantics:
.RS
A \f8Willing\fP packet is sent by managers that may service connections from
this display.  It is sent in response to either a \f8Query\fP,
\f8BroadcastQuery\fP or \f8ForwardQuery\fP but does not imply a commitment
to provide service (e.g. it may later decide that it has accepted enough
connections already).
.RE
Problems/Solutions:
.RS
Problem:
.RS
\f8Willing\fP not received by the display.
.br
Indication:
.RS
none if \f8BroadcastQuery\fP or \f8IndirectQuery\fP was sent, else failure to
receive \f8Willing\fP.
.RE
Solution:
.RS
The display should continue to send the query until a response is received.
.RE
.RE
.RE
Timeout/Retransmission policy:
.RS
Like all packets sent from the manager to the display, this packet should
never be retransmitted.
.RE
.RE
.LP
\f8Unwilling\fP
.RS
Manager \(-> Display
.br
Additional Fields:
.RS
The same fields as in the \f8Willing\fP packet.  The \f7Status\fP field
should indicate to the user a reason for the refusal of service.
.RE
Semantics:
.RS
An \f8Unwilling\fP packet is sent by managers in response to direct
\f8Query\fP requests (as opposed to \f8BroadcastQuery\fP or
\f8IndirectQuery\fP requests) if the manager will not accept requests for
management.  This is typically sent by managers that wish to only service
particular displays or which handle a limited number of displays at once.
.RE
Problems/Solutions:
.RS
Problem:
.RS
\f8Unwilling\fP not received by the display.
.br
Indication:
.RS
Display fails to receive \f8Unwilling\fP.
.RE
Solution:
.RS
The display should continue to send \f8Query\fP messages until a response is
received.
.RE
.RE
.RE
Timeout/Retransmission policy:
.RS
Like all packets sent from the manager to the display, this packet should
never be retransmitted.
.RE
.RE
.LP
\f8Request\fP
.br
.RS
Display \(-> Manager
.br
Additional Fields:
.RS
\f7Display Number\fP:
CARD16
.RS
The index of this particular server for the host on which the display is
resident.  This value will be zero for most autonomous displays.
.RE
\f7Connection Types\fP:
ARRAY16
.RS
An array indicating the stream services accepted by the display.  If the
high-order byte in a particular entry is zero, the low-order byte
corresponds to an X-protocol host family type.
.RE
\f7Connection Addresses\fP:
ARRAYofARRAY8
.RS
For each connection type in the previous array, the corresponding entry in
this array indicates the network address of the display device.
.RE
\f7Authentication Name\fP:
ARRAY8
.br
\f7Authentication Data\fP:
ARRAY8
.RS
This specifies the authentication protocol that the display expects
the manager to validate itself with.  The Authentication Data is
expected to contain data which the manager will interpret, modify
and use to authenticate itself.
.RE
\f7Authorization Names\fP:
ARRAYofARRAY8
.RS
This array specifies which types of authorization the display supports.  The
manager may decide to reject displays with which it cannot perform
authorization.
.RE
\f7Manufacturer Display ID\fP:
ARRAY8
.RS
This field can be used by the manager to determine how to decrypt the
Authentication Data field in this packet.  See the section below on
Manufacturer Display ID Format.
.RE
.RE
Semantics:
.RS
A \f8Request\fP packet is sent by a display to a specific host to request a
session id in preparation for a establishing a connection.  If the manager
is willing to service a connection to this display, it should return an
\f8Accept\fP packet with a valid session id and should be ready for a
subsequent Manage request.  Otherwise, it should return a \f8Decline\fP
packet.
.RE
Valid Responses:
.RS
\f8Accept\fP, \f8Decline\fP
.RE
Problems/Solutions:
.RS
Problem:
.RS
Request not received by manager.
.br
Indication:
.RS
Display timeout waiting for response.
.RE
Solution:
.RS
Display resends \f8Request\fP message.
.RE
.RE
Problem:
.RS
Message received out of order by manager.
.br
Indication:
.RS
none
.RE
Solution:
.RS
Each time a \f8Request\fP is sent, the manager sends the \f7Session ID\fP
associated with the next session in the \f8Acknowledge\fP.  If that next
session is not yet started, the manager will simply resend with the same
\f7Session ID\fP.  If the session is in progress, the manager will reply
with a new \f7Session ID\fP; in which case, the \f8Acknowledge\fP will be
discarded by the display.
.RE
.RE
.RE
Timeout/Retransmission policy:
.RS
Timeout after 2 seconds, exponential backoff to 32 seconds.  After no more
than 126 seconds, give up and report an error to the user.
.RE
.RE
.LP
\f8Accept\fP
.RS
Manager \(-> Display
.br
Additional Fields:
.RS
\f7Session ID\fP:
CARD32
.RS
This identifies the session which can be started by the manager.
.RE
\f7Authentication Name\fP:
ARRAY8
.br
\f7Authentication Data\fP:
ARRAY8
.RS
This data is sent back to the display to authenticate the manager.
If the Authentication Data is not the value expected by the display, it
should terminate the protocol at this point and display an error to the user.
.RE
\f7Authorization Name\fP:
ARRAY8
.br
\f7Authorization Data\fP:
ARRAY8
.RS
This data is sent to the display to indicate the type of authorization the
manager will be using in the first XOpenDisplay request after the
Manage packet is received.
.RE
.RE
Semantics:
.RS
An \f8Accept\fP packet is sent by a manager in response to a \f8Request\fP
packet if the manager is willing to establish a connection for the display.
The \f7Session ID\fP is used to identify this connection from any preceding
ones and will be used by the display in its subsequent \f8Manage\fP packet.
The \f7Session ID\fP is a 32 bit number which is incremented each time an
\f8Accept\fP packet is sent as it must be reasonably unique over a long
period of time.
.LP
If the authentication information is invalid, a \f8Decline\fP packet will be
returned with an appropriate \f7Status\fP message.
.RE
Problems/Solutions:
.RS
Problem:
.RS
\f8Accept\fP or \f8Decline\fP not received by display.
.br
Indication:
.RS
Display timeout waiting for response to \f8Request\fP.
.RE
Solution:
.RS
Display resends \f8Request\fP message.
.RE
.RE
Problem:
.RS
Message received out of order by display.
.br
Indication:
.RS
Display receives \f8Accept\fP after \f8Manage\fP has been sent.
.RE
Solution:
.RS
Display discards \f8Accept\fP messages after it has sent a \f8Manage\fP
message.
.RE
.RE
.RE
Timeout/Retransmission policy:
.RS
Like all packets sent from the manager to the display, this packet should
never be retransmitted.
.RE
.RE
.LP
\f8Decline\fP
.RS
Manager \(-> Display
.br
Additional Fields:
.RS
\f7Status\fP:
ARRAY8
.RS
This is a human readable string indicating the reason for refusal of
service.
.RE
\f7Authentication Name\fP:
ARRAY8
.br
\f7Authentication Data\fP:
ARRAY8
.RS
This data is sent back to the display to authenticate the manager.  If the
\f7Authentication Data\fP is not the value expected by the display, it
should terminate the protocol at this point and display an error to the user.
.RE
.RE
Semantics:
.RS
A \f8Decline\fP packet is sent by a manager in response to a \f8Request\fP
packet if the manager is unwilling to establish a connection for the
display.  This is allowed even if the manager had responded \f8Willing\fP to
a previous query.
.RE
Problems/Solutions:
.RS
same as for
\f8Accept\fP.
.RE
Timeout/Retransmission policy:
.RS
Like all packets sent from a manager to a display, this packet should never
be retransmitted.
.RE
.RE
.LP
\f8Manage\fP
.RS
Display \(-> Manager
.br
Additional Fields:
.RS
\f7Session ID\fP:
CARD32
.RS
This field should contain the non-zero session id returned
in the
\f8Accept\fP
packet.
.RE
\f7Display Number\fP:
CARD16
.RS
This field must match the value sent in the previous
\f8Request\fP
packet.
.RE
\f7Display Class\fP:
ARRAY8
.RS
This array specifies the class of the display.  Please refer to the section
below (Display Class Format) which discusses the format of this field.
.RE
.RE
Semantics:
.RS
A \f8Manage\fP packet is sent by a display to ask the manager to begin a
session on the display.  If the \f7Session ID\fP is correct the manager
should open a connection, otherwise it should respond with a \f8Refuse\fP or
\f8Failed\fP packet, unless the \f7Session ID\fP matches a currently
running session, or a session which has not yet successfully opened the
display but has not given up the attempt.  In this latter case, the
\f8Manage\fP packet should be ignored.  This will work as stream connections
give positive success indication to both halves of the stream, and positive
failure indication to the connection initiator (which will eventually
generate a \f8Failed\fP packet).
.RE
Valid Responses:
.RS
X connection with correct auth info,
\f8Refuse\fP,
\f8Failed\fP.
.RE
Problems/Solutions:
.RS
Problem:
.RS
\f8Manage\fP
not received by manager.
.br
Indication:
.RS
Display timeout waiting for response.
.RE
Solution:
.RS
Display resends
\f8Manage\fP
message.
.RE
.RE
Problem:
.RS
\f8Manage\fP received out of order by manager.
.br
Indication:
.RS
session already in progress with matching \f7Session ID\fP.
.RE
Solution:
.RS
\f8Manage\fP packet ignored.
.RE
Indication:
.RS
\f7Session ID\fP doesn't match next \f7Session ID\fP
.RE
Solution:
.RS
\f8Refuse\fP message is sent.
.RE
.RE
Problem:
.RS
Display cannot be opened on selected stream.
.br
Indication:
.RS
open display fails.
.RE
Solution:
.RS
\f8Failed\fP message is sent including a human readable reason.
.RE
.RE
Problem:
.RS
Display open does not succeed before a second manage packet is received
because of a timeout occuring in the display.
.br
Indication:
.RS
\f8Manage\fP packet received with \f7Session ID\fP matching the session
attempting to connect to the display.
.RE
Solution:
.RS
\f8Manage\fP packet is ignored.  As the stream connection will either
succeed, which will result in an active session, or the stream will
eventually give up hope of connecting and send a \f7Failed\fP packet, no
response to this \f8Manage\fP packet is necessary.
.RE
.RE
.RE
Timeout/Retransmission policy:
.RS
Timeout after 2 seconds, exponential backoff to 32 seconds.  After no more
than 126 seconds, give up and report an error to the user.
.RE
.RE
.LP
\f8Refuse\fP
.RS
Manager \(-> Display
.br
Additional Fields:
.RS
\f7Session ID\fP:
.RS
This field should be set to the
\f7Session ID\fP
received in the
\f8Manage\fP
packet.
.RE
.RE
Semantics:
.RS
A \f8Refuse\fP packet is sent by a manager when the \f7Session ID\fP
received in the \f8Manage\fP packet does not match the current \f7Session
ID\fP.  The display should assume that it received an old \f8Accept\fP
packet and should resend its \f8Request\fP packet.
.RE
Problems/Solutions:
.RS
Problem:
.RS
Error message is lost.
.br
Indication:
.RS
display times out waiting for OpenDisplay, \f8Refuse\fP or \f8Failed\fP.
.RE
Solution:
.RS
display resends \f8Manage\fP message.
.RE
.RE
.RE
Timeout/Retransmission policy:
.RS
Like all packets sent from a manager to a display, this packet should never be
retransmitted.
.RE
.RE
.LP
\f8Failed\fP
.RS
Manager \(-> Display
.br
Additional Fields:
.RS
\f7Session ID\fP:
CARD32
.RS
This field should be set to the \f7Session ID\fP received in the
\f8Manage\fP packet.
.RE
\f7Status\fP:
ARRAY8
.RS
A human readable string indicating the reason for failure.
.RE
.RE
Semantics:
.RS
A \f8Failed\fP packet is sent by a manager when it has problems establishing
the initial X connection in response to the \f8Manage\fP packet.
.RE
Problems/Solutions
.RS
Same as for \f8Refuse\fP.
.RE
.RE
.LP
\f8KeepAlive\fP
.RS
Display \(-> Manager
.br
Additional Fields:
.RS
\f7Display Number\fP:
CARD16
.RS
Set to the display index for the display host.
.RE
.RE
.RS
\f7Session ID\fP:
CARD32
.RS
This field should be set to the \f7Session ID\fP received in the
\f8Manage\fP packet during the negotiation for the current session.
.RE
.RE
Sematics:
.RS
A \f8KeepAlive\fP packet can be sent at any time during the session by a
display to discover if the manager is running.  The manager should respond
with \f8Alive\fP whenever it receives this type of packet.
.LP
This allows the display to discover when the manager host is no longer running.
A display is not required to send \f8KeepAlive\fP packets, and, upon lack of
receipt of \f8Alive\fP packets, is not required to perform any specific action.
.LP
The expected use of this packet is to terminate an active session when the
manager host or network link fails.  The display should keep track of the
time since any packet has been received from the manager host and
use \f8KeepAlive\fP packets when a substantial time has elapsed since the
most recent packet.
.RE
Valid Responses:
.RS
\f8Alive\fP
.RE
Problems/Solutions:
.RS
Problem:
.RS
Manager doesn't receive the packet or display doesn't receive the response.
.RE
.RS
Indication:
.RS
No \f8Alive\fP packet returned
.RE
Solution:
.RS
Retransmit the packet with an exponential backoff; start at 2 seconds and
assume the host is not up after no less than 30 seconds.
.RE
.RE
.RE
.RE
.LP
\f8Alive\fP
.RS
Manager \(-> Display
.br
Additional Fields:
.RS
\f7Session Running\fP:
CARD8
.RS
This field indicates that the session indicated by \f7Session ID\fP is
currently active.  The value is 0 if no session is active, 1 if a session
is active.
.RE
\f7Session ID\fP:
CARD32
.RS
The ID of the currently running session; if any.  When no session is active
this field should be zero.
.RE
.RE
Semantics:
.RS
An \f8Alive\fP packet is sent in response to a \f8KeepAlive\fP request.  If
a session is currently active on the display, the manager includes the
\f7Session ID\fP in the packet.  The display can use this information to
determine the status of the manager.
.RE
.RE
.RE
.NH 1
Session Termination
.LP
When the session is over, the initial connection with the display (the one
which ack's the \f8Manage\fP packet) will be closed by the manager.  If only
a single session was active on the display, all other connections should be
closed by the display and the display should be reset.  If multiple sessions
are active simultaneously, and the display can identify which connections
belong to the terminated sesssion, those connections should be closed.
Otherwise all connections should be closed and the display reset only when
all sessions have been terminated (i.e. all initial connections closed).
.LP
The session may also be terminated at any time by the display if the
managing host no longer responds to \f8KeepAlive\fP packets.  The exact
time-outs for sending \f8KeepAlive\fP packets is not specified in this
protocol as the trade off should not be fixed between loading an otherwise
idle system with spurious \f8KeepAlive\fP packets and not noticing that the
manager host is down for a long time.
.NH 1
State Diagrams
.LP
These state diagrams are designed to cover all actions of both
the display and the manager.  Any packet which is received out-of-sequence
will be ignored.
.LP
Display:

.RS
.LP
\f6start\fP:
.RS
user-requested connect to one host \(-> \f6query\fP

.br
user-requested connect to some host \(-> \f6broadcast\fP

.br
user-requested connect to site host-list \(-> \f6indirect\fP
.RE

.LP
\f6query\fP:
.RS
Send \f8Query\fP packet
.br
\(-> \f6collect-query\fP
.RE

.LP
\f6collect-query\fP:
.RS
receive \f8Willing\fP \(-> \f6start-connection\fP

.br
receive \f8Unwilling\fP \(-> \f6stop-connection\fP

.br
timeout \(-> \f6query\fP
.RE	

.LP
\f6broadcast\fP:
.RS
Send \f8BroadcastQuery\fP packet
.br
\(-> \f6collect-broadcast-query\fP
.RE

.LP
\f6collect-broadcast-query\fP:
.RS
receive \f8Willing\fP \(-> \f6update-broadcast-willing\fP

.br
user-requested connect to one host \(-> \f6start-connection\fP

.br
timeout \(-> \f6broadcast\fP
.RE

.LP
\f6update-broadcast-willing\fP:
.RS
Add new host to the host list presented to the user.
.br
\(-> \f6collect-broadcast-query\fP
.RE

.LP
\f6indirect\fP:
.RS
Send \f8IndirectQuery\fP packet
.br
\(-> \f6collect-indirect-query\fP
.RE

.LP
\f6collect-indirect-query\fP:
.RS
receive \f8Willing\fP \(-> \f6update-indirect-willing\fP

.br
user-requested connect to one host \(-> \f6start-connection\fP

.br
timeout \(-> \f6indirect\fP
.RE

.LP
\f6update-indirect-willing\fP:
.RS
Add new host to the host list presented to the user.
.br
\(-> \f6collect-indirect-query\fP
.RE

.LP
\f6start-connection\fP:
.RS
Send \f8Request\fP packet
.br
\(-> \f6await-request-response\fP
.RE

.LP
\f6await-request-response\fP:
.RS
receive \f8Accept\fP \(-> \f6manage\fP

.br
receive \f8Decline\fP \(-> \f6stop-connection\fP

.br
timeout \(-> \f6start-connection\fP
.RE

.LP
\f6manage\fP:
.RS
Save \f7Session ID\fP
.br
Send \f8Manage\fP packet with \f7Session ID\fP
.br
\(-> \f6await-manage-response\fP
.RE	

.LP
\f6await-manage-response\fP:
.RS
receive XOpenDisplay: \(-> \f6run-session\fP

.br
receive \f8Refuse\fP with matching \f7Session ID\fP \(-> \f6start-connection\fP

.br
receive \f8Failed\fP with matching \f7Session ID\fP \(-> \f6stop-connection\fP

.br
timeout \(-> \f6manage\fP
.RE

.LP
\f6stop-connection\fP:
.RS
Display cause of termination to user
.br
\(-> \f6start\fP

.RE	

\f6run-session\fP:
.RS
Decide to send \f8KeepAlive\fP packet \(-> \f6keep-alive\fP

await close of first display connection

.br
\(-> \f6reset-display\fP
.RE	

.LP
\f6keep-alive\fP:
.RS
send \f8KeepAlive\fP packet with current \f7Session ID\fP

.br
\(-> \f6await-alive\fP
.RE

.LP
\f6await-alive\fP:
.RS
Receive \f8Alive\fP with matching \f7Session ID\fP \(-> \f6run-session\fP

.br
Receive \f8Alive\fP with non-matching \f7Session ID\fP or
FALSE \f7Session Running\fP \(-> \f6reset-display\fP

.br
Final Timeout without receiving \f8Alive\fP packet \(-> \f6reset-display\fP

.br
timeout \(-> \f6keep-alive\fP
.RE

.LP
\f6reset-display\fP:
.RS
(if possible) \(-> close all display connections associated with this session

.br
last session \(-> close all display connections

.br
\(-> \f6start\fP
.RE

.RE
.LP
Manager:
.RS

.LP
\f6idle\fP:
.RS
receive \f8Query\fP \(-> \f6query-respond\fP

.br
receive \f8BroadcastQuery\fP \(-> \f6broadcast-respond\fP

.br
receive \f8IndirectQuery\fP \(-> \f6indirect-respond\fP

.br
receive \f8ForwardQuery\fP \(-> \f6forward-respond\fP

.br
receive \f8Request\fP \(-> \f6request-respond\fP

.br
receive \f8Manage\fP \(-> \f6manage\fP

.br
an active session terminates \(-> \f6finish-session\fP

.br
receive \f8KeepAlive\fP \(-> \f6send-alive\fP

.br
\(-> \f6idle\fP
.RE	

.LP
\f6query-respond\fP:
.RS
if willing to manage \(-> \f6send-willing\fP

.br
\(-> \f6send-unwilling\fP
.RE

.LP
\f6broadcast-respond\fP:
.RS
if willing to manage \(-> \f6send-willing\fP

.br
\(-> \f6idle\fP
.RE

.LP
\f6indirect-respond\fP:
.RS
Send \f8ForwardQuery\fP packets to all managers on redirect list.
.br
if willing to manage \(-> \f6send-willing\fP

.br
\(-> \f6idle\fP
.RE

.LP
\f6forward-respond\fP:
.RS
Decode destination address, if willing to manage \(-> \f6send-willing\fP

.br
\(-> \f6idle\fP
.RE

.LP
\f6send-willing\fP:
.RS
Send \f8Willing\fP packet
.br
\(-> \f6idle\fP
.RE

.LP
\f6send-unwilling\fP:
.RS
Send \f8Unwilling\fP packet
.br
\(-> \f6idle\fP
.RE

.LP
\f6request-respond\fP:
.RS
if manager is willing to allow a session on display \(-> \f6accept-session\fP

.br
\(-> \f6decline-session\fP
.RE

.LP
\f6accept-session\fP:
.RS
Generate \f7Session ID\fP.  Save \f7Session ID\fP, display address and
display number somewhere
.br
Send \f8Accept\fP packet
.br
\(-> \f6idle\fP
.RE		

.LP
\f6decline-session\fP:
.RS
Send \f8Decline\fP packet
.br
\(-> \f6idle\fP
.RE	

.LP
\f6manage\fP:
.RS
If \f7Session ID\fP matches saved \f7Session ID\fP \(-> \f6run-session\fP
.br
If \f7Session ID\fP matches \f7Session ID\fP of session in process of
starting up, or currently active session \(-> \f6idle\fP

.br
\(-> \f6refuse\fP

.RE	
.LP

\f6refuse\fP:
.RS
Send
\f8Refuse\fP
packet
.br
\(-> 
\f6idle\fP

.RE	

.LP
\f6run-session\fP:
.RS
Terminate any session in progress
.br
XOpenDisplay
.br
open display succeeds \(-> 
\f6start-session\fP

.br
\(-> 
\f6failed\fP
.RE

.LP
\f6failed\fP:
.RS
send \f8Failed\fP packet
.br
\(-> \f6idle\fP
.RE	

.LP
\f6start-session\fP:
.RS
Start a new session
.br
\(-> \f6idle\fP
.RE

.LP
\f6finish-session\fP:
.RS
XCloseDisplay

.br
\(-> \f6idle\fP
.RE

.LP
\f6send-alive\fP:
.RS
Send \f8Alive\fP packet containing current status.

.br
\(-> \f6idle\fP
.RE

.RE
.NH 1
Protocol Encoding
.LP
When XDMCP is implemented on top of UDP (the Internet User Datagram Protocol),
port number 177 is to be used.
.LP
The version number in all packets will be 1.
.LP
Packet opcodes are 16 bit integers.
.RS
.TS
c c
l l.
Packet Name	Encoding
_
BroadcastQuery	1
Query	2
IndirectQuery	3
ForwardQuery	4
Willing	5
Unwilling	6
Request	7
Accept	8
Decline	9
Manage	10
Refuse	11
Failed	12
Alive	13
KeepAlive	14
.TE
.RE
.LP
Per packet information follows:
.LP
\f8Query\fP
.br
\f8BroadcastQuery\fP
.br
\f8IndirectQuery\fP
.RS
These packets are identical except for the opcode field.
.TS
c c c
c l l.
Length	Type	Description
_
2	CARD16	version number (always 1)
2	CARD16	opcode (always \f8Query\fP, \f8BroadcastQuery\fP or \f8IndirectQuery\fP)
2	CARD16	length
1	CARD8	number of \f7Authentication Names\fP sent (m)
2	CARD16	length of first \f7Authentication Name\fP (m\d\s-21\s+2\u)
m\d\s-21\s+2\u	CARD8	first \f7Authentication Name\fP
\&...		Other \f7Authentication Names\fP
.TE
.RE
.LP
\f8ForwardQuery\fP
.RS
.TS
c c c
c l l.
Length	Type	Description
_
2	CARD16	version number (always 1)
2	CARD16	opcode (always \f8ForwardQuery\fP)
2	CARD16	length
2	CARD16	length of \f7Client Address\fP (m)
m	CARD8	\f7Client Address\fP
2	CARD16	length of \f7Client Port\fP (n)
n	CARD8	\f7Client Port\fP
1	CARD8	number of \f7Authentication Names\fP sent (o)
2	CARD16	length of first \f7Authentication Name\fP (o\d\s-21\s+2\u)
o\d\s-21\s+2\u	CARD8	first \f7Authentication Name\fP
\&...		Other \f7Authentication Names\fP
.TE
.RE
.LP
\f8Willing\fP
.RS
.TS
c c c
c l l.
Length	Type	Description
_
2	CARD16	version number (always 1)
2	CARD16	opcode (always \f8Willing\fP)
2	CARD16	length (6 + m + n + o)
2	CARD16	Length of \f7Authentication Name\fP (m)
m	CARD8	\f7Authentication Name\fP
2	CARD16	\f7Hostname\fP length (n)
n	CARD8	\f7Hostname\fP
2	CARD16	\f7Status\fP length (o)
o	CARD8	\f7Status\fP
.TE
.RE
.LP
\f8Unwilling\fP
.RS
.TS
c c c
c l l.
Length	Type	Description
_
2	CARD16	version number (always 1)
2	CARD16	opcode (always \f8Unwilling\fP)
2	CARD16	length (4 + m + n)
2	CARD16	\f7Hostname\fP length (m)
m	CARD8	\f7Hostname\fP
2	CARD16	\f7Status\fP length (n)
n	CARD8	\f7Status\fP
.TE
.RE
.LP
\f8Request\fP
.RS
.TS
c c c
c l l.
Length	Type	Description
_
2	CARD16	version number (always 1)
2	CARD16	opcode (always \f8Request\fP)
2	CARD16	length
2	CARD16	\f7Display Number\fP
1	CARD8	Count of \f7Connection Types\fP (m)
2 \(mu m	CARD16	\f7Connection Types\fP
1	CARD8	Count of \f7Connection Addresses\fP (n)
2	CARD16	Length of first \f7Connection Address\fP (n\s-2\d1\u\s+2)
n\s-2\d1\u\s+2	CARD8	First \f7Connection Address\fP
\&...		Other connection addresses
2	CARD16	Length of \f7Authentication Name\fP (o)
o	CARD8	\f7Authentication Name\fP
2	CARD16	Length of \f7Authentication Data\fP (p)
p	CARD8	\f7Authentication Data\fP
1	CARD8	Count of \f7Authorization Names\fP (q)
2	CARD16	Length of first \f7Authorization Name\fP (q\s-2\d1\u\s+2)
q\s-2\d1\u\s+2	CARD8	First \f7Authorization Name\fP
\&...		Other authorization names
2	CARD16	Length of \f7Manufacturer Display ID\fP (r)
r	CARD8	\f7Manufacturer Display ID\fP
.TE
.RE
.LP
\f8Accept\fP
.RS
.TS
c c c
c l l.
Length	Type	Description
_
2	CARD16	version number (always 1)
2	CARD16	opcode (always \f8Accept\fP)
2	CARD16	length (12 + n + m + o + p)
4	CARD32	\f7Session ID\fP
2	CARD16	Length of \f7Authentication Name\fP (n)
n	CARD8	\f7Authentication Name\fP
2	CARD16	Length of \f7Authentication Data\fP (m)
m	CARD8	\f7Authentication Data\fP
2	CARD16	Length of \f7Authorization Name\fP (o)
o	CARD8	\f7Authorization Name\fP
2	CARD16	Length of \f7Authorization Data\fP (p)
p	CARD8	\f7Authorization Data\fP
.TE
.RE
.LP
\f8Decline\fP
.RS
.TS
c c c
c l l.
Length	Type	Description
_
2	CARD16	version number (always 1)
2	CARD16	opcode (always \f8Decline\fP)
2	CARD16	length (6 + m + n + o)
2	CARD16	Length of \f7Status\fP (m)
m	CARD8	\f7Status\fP
2	CARD16	Length of \f7Authentication Name\fP (n)
n	CARD8	\f7Authentication Name\fP
2	CARD16	Length of \f7Authentication Data\fP (o)
o	CARD8	\f7Authentication Data\fP
.TE
.RE
.LP
\f8Manage\fP
.RS
.TS
c c c
c l l.
Length	Type	Description
_
2	CARD16	version number (always 1)
2	CARD16	opcode (always \f8Manage\fP)
2	CARD16	length (8 + m)
4	CARD32	\f7Session ID\fP
2	CARD16	\f7Display Number\fP
2	CARD16	Length of \f7Display Class\fP (m)
m	CARD8	\f7Display Class\fP
.TE
.RE
.LP
\f8Refuse\fP
.RS
.TS
c c c
c l l.
Length	Type	Description
_
2	CARD16	version number (always 1)
2	CARD16	opcode (always \f8Refuse\fP)
2	CARD16	length (4)
4	CARD32	\f7Session ID\fP
.TE
.RE
.LP
\f8Failed\fP
.RS
.TS
c c c
c l l.
Length	Type	Description
_
2	CARD16	version number (always 1)
2	CARD16	opcode (always \f8Failed\fP)
2	CARD16	length (6 + m)
4	CARD32	\f7Session ID\fP
2	CARD16	Length of \f7Status\fP (m)
m	CARD8	\f7Status\fP
.TE
.RE
.LP
\f8KeepAlive\fP
.RS
.TS
c c c
c l l.
Length	Type	Description
_
2	CARD16	version number (always 1)
2	CARD16	opcode (always \f8KeepAlive\fP)
2	CARD16	length (6)
2	CARD16	\f7Display Number\fP
4	CARD32	\f7Session ID\fP
.TE
.RE
.LP
\f8Alive\fP
.RS
.TS
c c c
c l l.
Length	Type	Description
_
2	CARD16	version number (always 1)
2	CARD16	opcode (always \f8Alive\fP)
2	CARD16	length (5)
1	CARD8	\f7Session Running\fP (0: not running 1: running)
4	CARD32	\f7Session ID\fP (0: not running)
.TE
.RE
.NH 1
Display Class Format
.LP
The \f7Display Class\fP field of the \f8Manage\fP packet is used by the
display manager to collect common sorts of
displays into manageable groups.  This field is a string encoded of
ISO-LATIN-1 characters in the following format:
.nf
.sp
.ta 1i
	ManufacturerID-ModelNumber
.fi
.sp
.LP
Both elements of this string must exclude characters of the set { \fB-\fP,
\&\fB.\fP, \fB:\fP, \fB*\fP, \fB?\fP, \fI<space>\fP }.  The ManufacturerID is a
string which should be registered with the X Consortium.  The ModelNumber is
designed to identify characteristics of the display within the manufacturer's
product line.  This string should be documented in the users manual for the
particular device.  This string should probably not be specifiable by the
display user to avoid unexpected configuration errors.
.NH 1
Manufacturer Display ID Format
.LP
To authenticate the manager, the display and manager will share a private
key.  The manager, then, must be able to discover which key to use for a
particular device.
The \f7Manufacturer Display ID\fP field of the \f8Request\fP packet is
intended for this purpose.  Typically, the manager host will
contain a map between this number and the key.  This field is intended to be
unique per display, possibly the ethernet address of the display in the form:
.nf
.sp
.ta 1i
	-Ethernet-8:0:2b:a:f:d2
.sp
.fi
or string
of the form:
.nf
.sp
.ta 1i
	ManufacturerID-ModelNumber-SerialNumber
.sp
.fi
where ManufacturerID, ModelNumber and SerialNumber are encoded using
ISO-LATIN-1 characters, excluding  { \fB-\fP,
\&\fB.\fP, \fB*\fP, \fB?\fP, \fI<space>\fP }
.LP
When the display is shipped to a customer, it should include both the
\f7Manufacturer Display ID\fP
and the private key in the documentation set.  This information should not
be modifiable by the display user.
.NH 1
Authentication
.LP
In an environment where authentication is not needed, XDMCP can disable
authentication by having the display send empty \f7Authentication Name\fP
and \f7Authentication Data\fP fields in the \f8Request\fP packet.  In this
case, the manager will not attempt to authenticate itself.  Other
authentication protocols may be developed, depending on local needs.
.LP
In an unsecure environment, the display must be able to verify that the
source of the various packets is a trusted manager.  These packets will
contain authentication information.  As an example of such a system, the
following discussion describes the "XDM-AUTHENTICATION-1" authentication
system.  This system uses a 56 bit shared private key, and 64 bits of
authentication data.  An associated example X authorization protocol
"XDM-AUTHORIZATION-1" will also be discussed.  The 56 bit key is represented
as a 64 bit number in network order (big endian).  This means that the first
octet in the representation will be zero.  When incrementing a 64 bit value,
the 8 octets of data will be interpreted in network order (big endian). I.e.
the last octet will be incremented, subsequent carries propogate towards the
first octet.
.LP
Assumptions:
.IP
The display and manager share a private key.  This key could be programmed
into the display by the manufacturer and shipped with the unit.  It must not
be available from the display itself, but should allow the value to be
modified in some way.  The system administrator would be responsible for
managing a database of terminal keys.
.IP
The display can generate random authentication numbers.
.LP
Some definitions first:
.EQ
oc D cc sup kappa mark = "encryption of plain text " D " by key " kappa
.EN C
.EQ
oc DELTA cc * sup kappa lineup = "decryption of crypto text " DELTA " with key " kappa
.EN C
.EQ
{ tau } lineup = "private key shared by display and manager"
.EN C
.EQ
rho lineup = "64 bit random number generated by display"
.EN C
.EQ
alpha lineup = "authentication data in XDMCP packets"
.EN C
.EQ
sigma lineup = "per-session private key, generated by manager"
.EN C
.EQ
beta lineup = "authorization data"
.EN
.LP
Encryption will use the DES; blocks shorter than 64 bits will be zero-filled
on the right to 64 bits.  Blocks longer than 64 bits will use block chaining:
.EQ
oc { D } cc sup kappa lineup = oc { D sub 1 } cc sup kappa " "
oc { D sub 2 } " " xor " " oc { D sub 1 } cc sup kappa cc sup kappa
.EN
.LP
The display generates the first authentication data in the
\f8Request\fP
packet:
.EQ
alpha sub roman Request mark = oc rho cc sup tau
.EN
.LP
For the
\f8Accept\fP
packet, the manager decrypts the initial message and returns
@alpha sub roman Accept@:
.EQ
rho lineup = oc alpha sub roman Request cc * sup tau
.EN C
.EQ
alpha sub roman Accept lineup = oc rho + 1 cc sup tau
.EN
.LP
The \f8Accept\fP packet also contains the authorization intended for use by
the X server.  A description of authorization type ``XDM-AUTHORIZATION-1''
follows:
.LP
The \f8Accept\fP packet contains the authorization name
``XDM-AUTHORIZATION-1''.  The authorization data is the string:
.EQ
beta sub Accept mark = oc sigma cc sup tau
.EN
.LP
To create authorization information for connection setup with the X server
using the XDM-AUTHORIZATION-1 authorization protocol, the client computes the
following:
.EQ
N mark = "X client identifier"
.EN C
.EQ
T lineup = "Current time in seconds on client host (32 bits)"
.EN
.EQ C
beta lineup = oc rho N T cc sup sigma
.EN
.LP
For TCP connections @N@ is 48 bits long and contains the 32 bit IP address of
the client host followed by the 16 bit port number of the client socket.
Formats for other connections must be registered.
The resulting value, @beta@, is 192 bits of authorization data which is sent
in the connection setup to the server.  The server receives the packet,
decrypts the contents.  To accept the connection, the following must hold:
.IP 1
@rho@ must match the value generated for the most recent XDMCP negotiation.
.IP 2
@T@ must be within 1200 seconds of the internally stored time.  If no time
been received before, the current time is set to @T@.
.IP 3
No packet containing the same pair (@N@, @T@) can have been received
in the last 1200 seconds (20 minutes).
