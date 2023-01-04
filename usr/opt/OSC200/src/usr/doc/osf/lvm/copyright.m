.\"
.\" (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
.\" ALL RIGHTS RESERVED
.\"
.\"
.\" OSF/1 Release 1.0
.\"
.\" Macros defined:
.\" .CR
.\"    copyright page. Should be invoked after .CS to kick out the 
.\"    copyright/trademark page.
.\" .CS
.\"    redefined to cause the main body of the document to always
.\"    contain an even number of pages, for production of originals
.\"    for 2-sided copying. Invokes -mm .CS to produce actual 
.\"    cover sheet. Final macros in document should be:
.\"    .CS
.\"    .CR
.\"    .TC
.\"    *in that order*. 
.\" .TC 
.\"    redefined to force even-number of pages in front matter (for 2-sided
.\"    copying).
.\" .TX
.\"    -mm table-of-contents escape macro. Inserts 'front matter' (revision
.\"    history, etc. in front of the table of contents.)
.de TX
.di ]v
.nf
.rH
.br
.di
.rm ]v
.if \\n(dn \{\
.PF "''-\\\\\\\\n(;g-''"
.ce 1
.S +1
.B
\\*(dN
.S
.SP
.ce 1
.I
.S +1
Revision History
.R
.S
.nf
.rH
.bp
.rm rH\}
..
.de Re
.	ec
..
.de Rh
.	eo
.	de rH Re
..
.de CR
.bp
.rs
.SP 20
The information contained within this document is subject to change
without notice.
.SP
OSF MAKES NO WARRANTY OF ANY KIND WITH REGARD TO THIS MATERIAL,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
.SP
OSF shall not be liable for errors contained herein or for incidental
consequential damages in connection with the furnishing, performance,
or use of this material.
.SP
This document contains proprietary information which is protected by
copyright.  All rights are reserved.  No part of this document may be
photocopied, reproduced, or translated into another language without
the prior written consent of Open Software Foundation, Inc.
.SP
Copyright\(co 1990 Open Software Foundation
.SP
OSF is a trademark of Open Software Foundation, Inc.
.SP
OSF/1 is a trademark of Open Software Foundation, Inc.
.SP
UNIX is a registered trademark of AT&T Information Systems, Inc. 
in the U.S.A and other countries.
..
.rn CS cS
.de CS
.if \\nP%2 .bp
.cS "\\$1" "\\$2" "\\$3" "\\$4" "\\$5" "\\$6"
..
.rn TC tC
.de TC
.ie \\n(.$-8 .tC "\\$1" "\\$2" "\\$3" "\\$4" "\\$5" "\\$6" "\\$7" "\\$8" "\\$9"
.el \{.ie \\n(.$-7 .tC "\\$1" "\\$2" "\\$3" "\\$4" "\\$5" "\\$6" "\\$7" "\\$8"
.el \{.ie \\n(.$-6 .tC "\\$1" "\\$2" "\\$3" "\\$4" "\\$5" "\\$6" "\\$7"
.el \{.ie \\n(.$-5 .tC "\\$1" "\\$2" "\\$3" "\\$4" "\\$5" "\\$6"
.el \{.ie \\n(.$-4 .tC "\\$1" "\\$2" "\\$3" "\\$4" "\\$5"
.el \{.ie \\n(.$-3 .tC "\\$1" "\\$2" "\\$3" "\\$4"
.el \{.ie \\n(.$-2 .tC "\\$1" "\\$2" "\\$3"
.el \{.ie \\n(.$-1 .tC "\\$1" "\\$2"
.el \{.ie \\n(.$ .tC "\\$1"
.el .tC\}\}\}\}\}\}\}\}
.af ;g 1
.ie \\n(;g%2 \{.af ;g i
.bp\}
.el .af ;g i
..
