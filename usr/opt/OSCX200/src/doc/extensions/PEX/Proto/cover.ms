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
\
.nr PS 10
.nr VS 12
.nr LL 6.25i
.nr PD 0.12i
.nr PO 1.0i
.fi
.LP
.DS I 4.5i
30 September, 1990
.DE
.LP
Dear X3D Member:
.LP
Version 5.0 of the PEX specification is now available to the public.
If your organization has a representative to the PEX activity,
contact him or her to obtain the updated PEX documents.  If not, the documents
may be obtained by anonymous ftp from the machine extract.lcs.mit.edu
(Internet number 18.30.0.245).  Please access extract during off-peak hours
if at all possible.
.LP
The following documents are available in (compressed) PostScript form:
.DS
.ta 2.5i
~pub/PEX/doc/v5p_cover.ps	This cover letter
~pub/PEX/doc/v5p_proto.ps.Z	PEX Protocol Specification
~pub/PEX/doc/v5p_proto_toc.ps.Z	PEX Protocol Specification table of contents
~pub/PEX/doc/v5p_encoding.ps.Z	PEX Protocol Encoding document
~pub/PEX/doc/v5p_encoding_toc.ps.Z	PEX Protocol Encoding table of contents
.DE
.LP
We are releasing these documents for public review and comment.
To report minor problems with
the documents, send mail to Sally Barry, the PEX document editor, at
"barry@3d.enet.dec.com".  More significant questions or comments may be
sent to the mailing list "pex-spec@expo.lcs.mit.edu".
.LP
The purpose of this document release is to form a basis for the PEX
Sample Implementation effort.
Most of the changes in this release are in order to fix minor problems with
the Consortium Review draft.  In addition, error handling for output commands is
more fully described.
.LP
I'd like to thank all those who took the time to report problems and errors
with the previous draft of the specifications, helping to create even better
versions this time around.
.sp 2
.ti 3.75i
.nf
Sally C. Barry

.ti 3.75i 
.nf
PEX Document Editor
.sp 2
.bp
.sp 2
.fi
Please use ftp access to extract only at nights or on weekends (Eastern Standard
Time), as extract is a well-used machine at M.I.T.
The PEX documents are also available via ftp from the machine
gatekeeper.dec.com (Internet number 16.1.0.2) in the directory
~pub/X11/PEX.
Again, please access this machine only during off-hours as it is heavily
used during normal (Pacific Standard Time) working hours.
