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
.de Ip
.IP \(bu 3
..
.\"	These macros should select a typewriter font if you have one.
.de LS
.KS
.DS
.ft C
..
.de LE
.ft P
.DE
.KE
..
.TL
Godzilla's Guide to Porting the X V11 Sample Server
.sp
March 1, 1988
.sp
Updated for R4: April 12, 1990
.AU
David S. H. Rosenthal
.AI
Sun Microsystems
.AU
Adam R. de Boor
.AI
University of California at Berkeley
.AU
edits by Bob Scheifler
.AI
Massachusetts Institute of Technology
.AU
Updated for R4 by Keith Packard
.AI
MIT X Consortium
.AB
.LP
For hackers in a hurry,  here's how to do a quick-and-dirty port of the
X V11 sample server to memory-mapped monochrome & color frame buffers.
The authors disclaim responsibility for damage to the environment during
this process.
.AE
.QP
``Did I \- ?''
.QP
``I told you you could,''  [Obi-Wan] Kenobi informed him with pleasure.
``Once you start to trust your inner self,  there'll be no stopping you.''
.DS C
Star Wars,  \fIGeorge Lucas\fP
.DE
.sp 2
.DS C
Copyright \(co 1987 by Sun Microsystems, Inc.\s-2\u1\d\s0
.DE
.DS C
Copyright \(co 1990 by the Massachusetts Institute of Technology\s-2\u1\d\s0
.DE
.FS
1.  Permission to use, copy, modify, distribute and sell this document for
any purpose and without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies, and that
the names of Sun Microsystems, Inc.  and M.I.T. not be used in advertising
or publicity pertaining to distribution of the software without specific,
written prior permission.  Sun Microsystems, Inc. and M.I.T. make no
representations about the suitability of the software described herein for
any purpose.  It is provided "as is" without express or implied warranty.
.FE
.sp 2
.NH
Introduction
.LP
In two independent efforts,  we ported the first alpha release of X V11
to monochrome Sun hardware.  The results of these efforts were merged together,
incorporated into the second alpha release,  and delivered to MIT for
distribution.
We went on to port the second alpha release to a number of Sun 8-bit
memory-mapped color displays,  and we contributed this code to the beta
releases.
.LP
With future releases, the internal structure of the server changed, and
porting efforts have to be modified as well.  No attempt has been made to
isolate the porting efforts from the changes between various versions of the
server.  This document focuses on Release 4 of the MIT Sample Server.
.LP
Based on this experience, we present a guide for others attempting to port X
V11.  Our goal is to enable them to get the server up and functional as
expeditiously as possible.  We address only ``initial'' ports, using the
\fIserver/ddx/mfb\fP, \fIserver/ddx/cfb\fP and \fIserver/ddx/mi\fP code to
drive a dumb memory frame buffer.
.LP
It is possible to port X V11 to dumb frame buffers very quickly,
even without a detailed understanding of the internals of the server.  We
give instructions on how to do this,  and then go on to cover some of the
internals that will be important in doing a more complex port,  and in
performance tuning.
We do \fInot\fP cover the following areas in detail:
.Ip
Porting to operating systems other than 4.2 or 4.3 BSD.
.Ip
Driving ``intelligent'' display hardware.
.Ip
Supporting input devices other than the mouse and keyboard.
.Ip
Performance tuning the system.
.LP
This document refers to the fourth full X V11 distribution from the MIT X
Consortium.
.NH
Raw Materials
.LP
The server source code is organised into a number of directories:
.IP ddx 8
The code for driving displays,  keyboards,  mice,  and other peripherals.
.IP dix 8
The machine independent code implementing the protocol interpreter and
resource managers.
.IP include 8
The \fI.h\fP files defining the machine independent data structures for the
server.
.IP os 8
The code implementing the server's file and inter-process I/O,  which
depends on the operating system.
.LP
In the \fBddx\fP directory,  there are the following directories:
.IP mfb 8
The ``monochrome frame buffer'' driver.  This is intended to be a machine independent
driver for 1-bit deep memory mapped displays.
.IP mi 8
The ``machine independent'' driver.  This is intended to be a machine
independent (but slightly incomplete) driver for \fIany\fP frame buffer.
.IP cfb 8
The ``color frame buffer'' driver.  This is intended to be a machine
independent driver for \fIn\fP-bit deep (\fIn\fP <= 32) Z-Format memory
mapped displays.  While much of this code is portable to arbitrary depth
displays, release 4 provides a fast implementation only for the 8-bit case.
.IP ...
There are several directories which are specific to a particular vendor and
contain minimal code for talking to that hardware.
In the \fBos\fP directory,  there is at present only the following directory:
.LP
The only subdirectory in the \fBos\fP directory is:
.IP 4.2bsd 8
An implementation of the file and interprocess primitives for the server
using the facilities of 4.2 and 4.3 BSD.  This code also works on various
SYSV-BSD blends (A/UX, HPUX etc) but no attempt has been made to get it
working in a generic SYSV environment.
.LP
The intent is that one should port the sample server to new hardware by
using the \fImi\fP code,  implementing at first only the lowest level routines
such as \fBFillSpans(\|)\fP.
We did not do so for our initial monochrome ports (the \fImi\fP code was too
incomplete), instead we used the \fImfb\fP code.  Our subsequent color
ports were based on \fImi\fP and we describe both.  The current release
relies on \fIcfb\fP heavily as much of \fImi\fP is only interesting from an
academic viewpoint.
.LP
The details of server internals you need to know to do this, and more
advanced ports,  are described in a document (the \fIPorting
Layer\fP) written by the DEC team that developed it.\s-2\u2\d\s0
.FS
2.  \fIThe Porting Layer for the X V11 Sample Server\fP,  by Susan Angebranndt,
Raymond Drewry,  Phil Karlton and Todd Newman.  See the file
\fIdoc/Server/ddx.doc.tbl.ms\fP.
.FE
You should read this document even for a simple monochrome or color port, a
complete understanding isn't important, but a better understanding of the
basic porting process will result from even a cursory perusal.  We refer to
it frequently when more details are needed.
.NH
Road Map
.LP
In order to get the sample server running on your hardware,  you will need
to deal with four distinct areas:
.IP a)
Painting the Bits \- you need to make the code work for the mapping
between pixel coordinates and byte/bit addresses defined by your hardware.
.IP b)
Pounding the Keys \- you need to map between the keystrokes and mouse
movements your hardware provides,  and the canonical forms required by the
server.
.IP c)
Dragging the Cursor \- you need to move a cursor image around the screen,
and change the image on demand.
.IP d)
Starting Up & Shutting Down \- you need to initialize and close down the
connections between the server and your hardware.
.NH
Doing the Port (Monochrome and 8-bit color)
.LP
Lets suppose you want to port X V11 to the Generic Workstation Company's
(GWC) hardware.  You can do most of the work in one fell swoop:
.LS
#!/bin/sh
cd server/ddx/sun
mkdir ../gwc
sed -e 's/sun/gwc/g' Makefile >../gwc/Makefile
for A in *.[hc]
do
	sed -e 's/sun/gwc/g' $A >../gwc/gwc`expr $A : 'sun\e(.*\e)'`
done
.LE
.LP
You now have a directory in the right place with a first approximation to the
source files in it.
It will avoid confusion if you remove all code in this directory between
.LS
#ifdef	SUN_WINDOWS
#endif	SUN_WINDOWS
.LE 
This deals with running X V11 ``on top of'' the SunWindows window system,
and is of no interest here.
.LP
The next steps tackle the four major areas in which
GWC's workstations are different from Sun's.
.NH 2
Painting the Bits
.LP
Assuming that you have a machine  which has a simple memory bitmap,
all you have to do to ensure that the pixels are painted correctly is to
deal with the potential differences between simple memory bitmaps.  The files
\fIserver/include/servermd.h\fP and \fIfonts/bdftosnf/bdftosnf.h\fP contain
the definitions for the various CPUs which already support the sample
server.  You will need to add #ifdef sections after the others
in each file which contains the following definitions.
.Ip
Byte order.  Both the \fImfb\fP and \fIcfb\fP code can handle either byte
order.  If you have a 68000 CPU (Big Endian), the file
\fIserver/include/servermd.h\fP should contain:
.LS
#define IMAGE_BYTE_ORDER	MSBFirst
.LE
If you have a 80386 GWC,  \fIserver/include/servermd.h\fP should contain:
.LS
#define IMAGE_BYTE_ORDER	LSBFirst
.LE
.Ip
Pixel order.  Again, both the \fImfb\fP and \fIcfb\fP code can handle either
the 68000 style, where the most significant bit is to the left on the scan
line, or the 80386 style, where the least significant bit is to the left on
the scan line.  For the 68000 GWC, the file \fIserver/include/servermd.h\fP
should contain:
.LS
#define BITMAP_BIT_ORDER	MSBFirst
.LE
.IP
And the file \fIfonts/bdftosnf/bdftosnf.h\fP should contain:
.LS
#define	DEFAULTBITORDER	MSBFirst
.LE
For the 80386 GWC,  the file \fIserver/include/servermd.h\fP should contain:
.LS
#define BITMAP_BIT_ORDER	LSBFirst
.LE
.IP
And the file \fIfonts/bdftosnf/bdftosnf.h\fP should contain:
.LS
#define	DEFAULTBITORDER	LSBFirst
.LE
.Ip
Pixel meaning.
Some machines have monochrome displays where a 1 bit means black, while
others ones have displays where a 1 means white.  Check the manual, and set
the appropriate values in the file \fIserver/ddx/gwc/gwcBW2.c\fP:
.LS
pScreen->whitePixel = 0 or 1;
pScreen->blackPixel = 1 or 0;
.LE
.Ip
Alignment restrictions.
CPUs vary in the alignment restrictions they place on memory accesses.  For
example, some can do 4-byte accesses at every byte address, and some only at
4-byte boundaries.  In general, the \fImfb\fP and \fIcfb\fP code take the
conservative viewpoint that the framebuffer and memory bitmaps are accessed
only 4 bytes wide at 4-byte boundaries.
.IP
However, this isn't true of the font code.  For Sun hardware, we decided to
extend the same restriction to the font code, and changed the padding rules
used for the fonts to avoid those cases in which the \fImfb\fP and \fIcfb\fP
code would use addresses that weren't 4-byte aligned.  We will cover the
padding rules in some detail later; for the moment all you need to do is to
include in the file \fIfonts/bdftosnf/bdftosnf.h\fP:
.LS
#define	DEFAULTGLPAD	4
.LE
while \fIserver/include/servermd.h\fP should have
.LS
#define GLYPHPADBYTES	4
.LE
.IP
.LP
The \fImfb\fP, \fIcfb\fP and \fImi\fP directories are now set up for your
machine.  The next steps are more work.
.NH 2
Pounding the Keys
.LP
More frequently than you would believe possible,  the DIX layer will call
the function \fBProcessInputEvents(\|)\fP.
This function lives in \fIgwcIo.c\fP,  and you will be able to use it
almost unchanged.  It has to:
.Ip
Obtain the device-specific events from the keyboard and the mouse,  by
calling through their private descriptor structures to a device-specific
\fBGetEvents\fP routine.
.Ip
In time-stamp order, hand each event to the appropriate \fBProcessEvents\fP
routine, again calling through the private descriptor structure for each
device.
.Ip
Maintain the time of the last event.
.Ip
Restore the screen if it is currently saved.
.LP
Only the first of these requires modifications to the code you now have,
and these take place in the files \fIgwcKbd.c\fP and \fIgwcMouse.c\fP
.LP
Each device has a \fBGetEvents\fP routine;  it needs to return an array
containing the device-specific events that are immediately available.  The
precise structure of the events is not important,  but they need to be
time-stamped by the kernel.  The declarations in \fIgwcIo.c\fP,
\fIgwcKbd.c\fP and \fIgwcMouse.c\fP of things as pointers to
\fBFirm_events\fP should be changed to \fBgwc_events\fP,  the name of the
structures the GWC kernel returns.
.LP
To obtain the events,  you can either do a non-blocking \fBread(\|)\fP or use
a shared-memory circular queue of events, if your kernel
supports it.  The \fIserver/ddx/sun\fP
code supports only non-blocking reads,  since current
Sun kernels do not have a shared-memory event queue.  You are strongly urged
to add support for a shared-memory event queue to the GWC kernel,  since doing
the non-blocking \fBread(\|)\fPs is a serious performance problem.
.LP
For the present,  we assume that the GWC kernel has separate
\fI/dev/mouse\fP and \fI/dev/kbd\fP files,  which:
.Ip
Support the \fBfcntl(FNDELAY)\fP call.
.Ip
Provide a stream of \fBgwc_event\fP structures containing position, keycode,
and timestamp information.
.LP
In \fIgwcMouse.c\fP,  you will need to change only the names of the fields
in the device-specific event structures,  from those of the \fIFirm_event\fP
to those of the \fIgwc_event\fP,  and the values of the ID codes (such as
MS_LEFT) used for the button and motion events.  If you have one of the GWC
mice that reports absolute positions rather than X and Y deltas,  you will
need to add another case to the following statement.
.LS
switch (fe->id) {
    case MS_LEFT:
    case MS_MIDDLE:
    case MS_RIGHT:
    case LOC_X_DELTA:
    case LOC_Y_DELTA:
    default:
}
.LE
Beware of the Sun convention that motion up gives a positive Y
delta.
.LP
In \fIgwcKbd.c\fP,  you will need to make similar changes to the names of
the event fields.  Then,
replace the various \fBioctl(KIOC<foo>)\fP calls with their GWC equivalents
Their meanings are:
.TS
center, box;
c c
l l.
Name	Operation
_
KIOCTYPE	T{
Get int indicating keyboard type
T}
KIOCGTRANS	T{
Get/set keycode translation.  We want ASCII events.
T}
KIOCSDIRECT	T{
Switch keystrokes between \fI/dev/kbd\fP and \fI/dev/console\fP.  We want
\fI/dev/kbd\fP.
T}
.TE
.LP
Now,  you have to establish a mapping between the codes your keyboard
sends and the names X V11 uses for keys (the so-called \fIkeysyms\fP).
Read the files \fIserver/ddx/gwc/gwcKeyMap.c\fP and \fIX11/keysym.h\fP.
Look at your keyboard manual,  and find the lowest keycode it sends (\fIkcmin\fP)
and the highest keycode it sends (\fIkcmax\fP).
Look at the keyboard itself,  and find the key with the largest number of
symbols on it,  counting strings like ``Return'' or ``F9'' as one
symbol.
The number of symbols on this key is the \fIwidth\fP of the keyboard.
Now,  for each of your keyboard types,  you will need two things in
\fIgwcKeyMap.c\fP:
.Ip
A \fIkeymap\fP,  which is an array with (\fIkcmax\fP-\fIkcmin\fP+1)
rows and \fIwidth\fP
columns.
Each row in the array should contain the keysyms corresponding to the
symbols on the keycap,  with the first column containing the symbol
generated when the key is un-shifted,  the second the symbol generated when
the key is shifted,  and the other columns the other symbols in no
special order.
So,  for example,  a key with the symbols ``1'' and ``!'' on it would
have the keysyms \fIXK_1\fP and \fIXK_exclam\fP in the table.
.Ip
A \fImodifier map\fP,  which is an array with one entry per keycode.
Each entry should contain the modifier bits that are set when that key
is down.  So,  for example,  a ``Shift'' key would have an entry
\fIShiftMask\fP.
.NH 2
Dragging the (Software) Cursor
.LP
Suns use a software cursor,  as there is no cursor hardware.
The problem with
is that it must be removed from the display before painting operations that
might affect the pixels it is using.  And,  of course,  put back again at
some time later.
.LP
Fortunately, a machine-independent software cursor layer is included in
\fIserver/ddx/mi\fP which is easily connected to the mouse device.  It will
work on any display, but it is a little slow, and you will want to use the
hardware cursor on machines that have it.  The changes needed to do so are
fairly extensive, so we cover them later also.
.NH 2
Starting Up & Shutting Down
.LP
When the server is started,  it initializes its output and input devices by
calling their initialization procedures.
This is where things get really device-specific,  and the code for the GWC
will differ significantly from the Sun code.  The overall structure will be
preserved,  however.
.LP
It is important to observe that,  once the last client has closed its
connection and everything has been shut down,
the server will re-initialize everything by repeating the process.
Although there is a close-down procedure,  it is generally better to avoid
closing the device,  instead simply reset it to its initial state.
So if,  for
example,  initializing a display or a mouse involves opening a file,  the
descriptor should be remembered in a static structure and not re-opened if
it is already open.\s-2\u4\d\s0
.FS
4.  See section 2.4.3 of the \fIPorting Layer\fP.
.FE
.NH 3
Output
.LP
Output devices are initialized in a two-step process:
.Ip
The server calls \fBInitOutput(\|)\fP,  a routine in \fIgwcInit.c\fP which you
can re-use untouched.  It calls each potentially available display's probe
routine,  finding the probe routine and the probable file name for it in the
\fBgwcFbData[\|]\fP array.
.IP
The dumb monochrome device,  whose driver is in \fIgwcBW2.c\fP,  has a probe
routine called \fBgwcBW2Probe(\|)\fP.  This has to attempt to initialize the
monochrome display and,  if it succeeds,  fill out the \fBfbFd\fP
structure describing it.  In the Sun case,  most of the work is done in a
routine \fBsunOpenFrameBuffer(\|)\fP in \fIsunInit.c\fP,  because it is common
among all Sun framebuffers.  This routine scans the command-line arguments,
the environment,  and the \fI/dev\fP directory to find a frame-buffer of
the required type,  opens it,  and returns the file descriptor.
.IP
The dumb color device, whose driver is in \fIgwcCG3.c\fP, is similarly
structured, the probe is called \fIgwcCG3Probe(\|)\fP.
.IP
The probe routines then have to map this descriptor into the server's address
space, and install a pointer to the pixels in the \fBfbFd\fP structure.
Note that the Sun monochrome code has to deal with two different sizes of
monchrome framebuffers.  The \fBfbFd\fP structure is the static structure we
mentioned earlier, needed to preserve internal driver information across
server re-initializations.  Finally, it calls \fBAddScreen(\|)\fP, giving it
the address of the BW2/CG3 initialization routine.
.Ip
\fBAddScreen(\|)\fP fills out the screen information, and calls the
initialization routine.  In the monochrome case, \fBmfbScreenInit(\|)\fP is
called to initialize the \fImfb\fP part of the code.  This fills in the
\fBScreen\fP's operations vector with the routines the
\fImfb\fP code supports.  The remaining entries are filled out by the
initialization routine itself.  This code can be reused unchanged.  The
color case simply uses \fBcfbScreenInit(\|)\fP.  Note that in the color case
the values for whitePixel and blackPixel are left unspecified.  \fIcfb\fP
will fill in the values 0 for white and 1 for black.  If this is not
satisfactory, replace them with your own values after calling
\fBcfbScreenInit\fP and before calling \fBcfbCreateDefColormap\fP.
.LP
When the server is shutting down prior to re-initialization,  it will call
the \fBCloseScreen\fP function in the \fBScreen\fP structure.
You can also use this routine unchanged.
.LP
One other area which needs attention are the \fBgwcBW2SaveScreen(\|)\fP and
\fBgwcCG3SaveScreen(\|\)\fP routines.  These need to enable and disable the
video for the GWC framebuffer.
.NH 3
Input
.LP
A similar two-step process is used to initialize the input devices.  First,
the server calls \fBInitInput(\|)\fP, in \fIgwcInit.c\fP.  It registers the
keyboard and mouse devices by calling \fBAddInputDevice(\|)\fP,  among the
arguments to which are the appropriate initialization/closedown routines
\fBgwcMouseProc\fP and \fBgwcKbdProc\fP.
.LP
Then,  the server calls each of the device initialization/closedown routines
twice,  once with command \fBDEVICE_INIT\fP and once with command \fBDEVICE_ON\fP.
These must:
.Ip
Open the necessary devices.
.Ip
Set up the appropriate keymaps.
.LP
When the server is shutting down prior to re-initialization,  it will call
the device's initialization/closedown routine with command \fBDEVICE_OFF\fP.
.LP
Except for the details of how the devices are opened and coerced to supply
ASCII events,  and the changes made earlier to the \fBioctl(\|)\fPs,  the rest
of this code can be used unchanged.
.NH 2
Tidying Up
.LP
You have now made all the major changes needed.  All that remains is to make
suitable changes to the makefiles:
.Ip
In \fIserver/Imakefile\fP,  make the following changes:
.KF
.LS
     ALLDDXDIRS = \fIwhatever is already there\fP ddx/gwc
	    GWC = ddx/gwc/libgwc.a
    ALLPOSSIBLE = \fIwhatever is already there\fP Xgwc

#ifndef	XgwcServer
#define	XgwcServer /* as nothing */
#endif
            ALL = \fIwhatever is already there\fP XgwcServer

#
# GWC server
#
GWCDIRS = dix ddx/snf ddx/mi ddx/mfb ddx/cfb ddx/gwc os/4.2bsd
GWCOBJS = ddx/gwc/gwcInit.o
GWCLIBS = $(GWC) $(CFB) $(DIX) $(SNF) $(UNIX) $(MFB) $(MI) $(EXTENSIONS)
GWCSYSLIBS = $(SYSLIBS)
XgwcDIRS = $(GWCDIRS)

ServerTarget(Xgwc,$(EXTDIR) $(GWCDIRS),$(GWCOBJS),$(GWCLIBS),$(GWCSYSLIBS))
.LE
.DS C
.I
Figure 1: Top-level Imakefile Changes
.R
.DE
.KE
.Ip
Now go ahead,  make everything,  and enjoy!
.LP
There are probably some details we've omitted \- we no longer have our GWCs
so we're writing this from memory.  If you find either the details or the
hardware,  please let us know.
.NH
Additional work needed for doing a Color Port on non 8-bit displays.
.LP
After the \fImfb\fP-based monochrome drivers were shipped to MIT,  we went
on to develop drivers for Sun's color hardware.
Our initial attempt was based on the
\fImi\fP code.  Our goals were:
.Ip
To get the server functional on color hardware as expeditiously as possible.
.Ip
To test the color code in \fIserver/dix\fP,  and as much as possible of the
code in \fIserver/ddx/mi\fP.
.Ip
To test the recommended porting strategy,  using \fImi\fP and implementing
only \fBGetSpans(\|)\fP,  \fBSetSpans(\|)\fP and \fBFillSpans(\|)\fP.
.Ip
To provide a highly portable implementation of the DDX layer for color hardware
to enable others to get the server running as effortlessly as possible.
.LP
Performance was explicitly not a goal.
Just as \fImfb\fP assumes that the display it is driving has a 1-bit deep memory
framebuffer accessed 32 bits wide at 32-bit boundaries,  \fIcfb\fP assumes a
\fIn\fP-bit deep (\fIn\fP\ <=\ 32) memory framebuffer accessed 32 bits wide
at 32-bit boundaries.  While these restrictions are fairly onerous,  they
make the code highly portable in both cases.  In the monochrome case, 
the DEC team managed to provide relatively good performance.  In the color
case,  there is \fIn\fP times more work to do,  and we have not attempted
all of the optimizations that \fImfb\fP uses.
.NH 2
Porting cfb to non-8 bit framebuffers
.LP
By default,  \fIcfb\fP is set up to drive a framebuffer that is 8 bits deep
with the pixel order defined by \fBBITMAP_BIT_ORDER\fP.  If your framebuffer
is different,  you will need to change some parameters in
\fIserver/ddx/cfb/cfbmskbits.h\fP:
.TS
center, box;
c s s s
c c c c
l n n l.
Parameters in cfb
_
Parameter	mfb	cfb	Comment
_
PPW	32	4	pixels per word
PLST	31	3	last pixel in a word (should be PPW-1)
PIM	0x1f	0x03	pixel index mask (index within a word)
PWSH	5	2	pixel-to-word shift
PSZ	1	8	pixel size (bits)
PMSK	0x01	0xFF	single-pixel mask
.TE
.LP
Read the comments in \fIcfbmskbits.[hc]\fP carefully before changing these
parameters.  You will also need to change the mask values in
\fIserver/ddx/cfb/cfbmskbits.c\fP.
.LP
Changing these parameters and masks is all you should need,  but we cannot
be sure.
The code has been used on several types of color hardware,  but in each case
there are four pixels to the word (and,  therefore,  the masks don't need
changing).
.NH 2
Using cfb
.LP
The code in \fIserver/ddx/sun/sunCG4C.c\fP illustrates how to use \fIcfb\fP
to drive an 8-bit deep memory frambuffer (the usage on non 8-bit deep frame
buffers is identcal but no example code exists).  You call
\fBcfbscrinit(\|)\fP to fill out the screen operation vector, and
\fBcfbCreateGC(\|)\fP to create a GC.  In both cases, reading the code will
show that most of the procedures to be used are from \fImi\fP.
.LP
Unlike \fImfb\fP, which effectively supports only a StaticGray visual, the
\fIcfb\fP code supports all 6 possible visual types if your hardware has
writeable color maps.  Otherwise, (or for testing purposes) you can define
\fBSTATIC_COLOR\fP and support a StaticColor visual (which also limits the
supported visual types to StaticColor and TrueColor).
.NH
Details
.LP
This section is not intended to be a complete survey of the details of the
server.  We wouldn't claim enough knowledge to write that (yet).   It is
rather a collection of comments on the areas we have had to deal with in
detail,  in the hope that we can save others from wasting their time on
problems that are either already solved or insoluble.
.NH 2
Software & Hardware Cursors
.LP
The details of the DIX interface to the cursor support are described in
section 2.5 of the \fIPorting Layer\fP.
.NH 3
Software Cursors
.LP
The Machine Independent software cursor code is very easy to use and it
provides reasonable performance.  It works by "wrapping" every potential
drawing operation to the screen and checking for overlap with the cursor
during these operations.  The \fIPorting Layer\fP describes this
"wrapping" notion in greater detail.
.NH 3
Hardware Cursors
.LP
To switch to using a hardware cursor,  you will need to study the QVSS code,\s-2\u12\d\s0
.FS
12.  See the files
\fIserver/ddx/sun/sunCursor.c\fP
and
\fIserver/ddx/dec/qvss/qvss_io.c\fP,
and section 2.5 of the \fIPorting Layer\fP.
.FE
and to remove the following code from the \fIserver/ddx/sun\fP files:
.Ip
\fIsunInit.c\fP:  the code that calls miDCInitialize
.Ip
\fIsunMouse.c\fP:  the code in \fBsunMouseProcessEvent(\|)\fP that positions
the software cursor.
.NH 2
Fonts & Padding
.LP
The \fIPorting Layer\fP describes the layout of the glyph information in
memory as follows:\s-2\u13\d\s0
.FS
13.  See section 5.3.
.FE
.QP
``Each scanline of each glyph is padded  to  a  byte  boundary
with  zero  bits.  Bit and byte order is whatever is natural
for the server.  (Note: the current BDF to SNF font compiler
handles  either  bit  order  within a byte as a compile time
option. It does not deal with byte order.) The glyph  for  a
character whose XCHARINFO is ci begins at cg[ci.byteOffset].
Glyphs may begin at arbitrary offsets within the array.''
.LP
The section on ``Alignment'' describes the problem:\s-2\u14\d\s0
.FS
14.  See section 4.5.2.
.FE
.QP
``The mfb text code might access mis-aligned longwords;  this is not a problem
on VAXes,  680x0 (x != 0),  or Intel architectures,  but might be on some as
yet unknown processors,  and is definitely bad on a 68000.  An easy fix is
to have the font compiler generate longword padded glyphs instead of
byte-padded ones.''
.LP
We followed this advice,  and the results are incorporated in the Sun
code in the release.
The trade-offs to consider when deciding what to do are:
.Ip
Many processors cannot access mis-aligned longwords.\s-2\u15\d\s0  In these
cases,  there is no real choice.
.FS
15.  Try it on a 68010 sometime,  or on a PC/RT,  or on a SPARC,  or ....
.FE
.Ip
Even processors which can access longwords at byte boundaries do so more
slowly than aligned accesses,  and these accesses are made in a
performance-critical area (painting characters).
Check your hardware manual, or write a small benchmark.  Furthermore, the
font code for big-endian machines is always slower when using non-longword
padded fonts as it must shift things around.
.Ip
On the other hand,  padding the glyphs to longwords wastes a significant
amount of space.
.LP
If you'd like to pack fonts even on machines which can't access longwords on
non-longword boundaries, then, in the file \fIserver/include/servermd.h\fP,
choose the alignment restriction (in bytes) for those accesses:
.LS
#define GETLEFTBITS_ALIGNMENT	1	/* arbitrary byte alignment */
#define GETLEFTBITS_ALIGNMENT	2	/* halfword alignment */
#define GETLEFTBITS_ALIGNMENT	4	/* longword alignment */
.LE
.LP
If you are using GLYPHPADBYTES 4, make sure you set GETLEFTBITS_ALIGNMENT to
1 even if your CPU can't access longwords on non-longword boundaries.  This
is because the text code would use extra instructions to check the alignment
of the accesses even though the fonts are guaranteed to be longword aligned.
.NH 2
Shared Event Queue
.LP
By default,  the DIX layer calls \fBProcessInputEvents(\|)\fP before it it
waits for something to happen,  and also before performing each client
request.
When using non-blocking \fBread(\|)\fP calls this is a substantial overhead,
and a facility for reducing this has been provided.  \fBSetInputCheck(\|)\fP
can be called with the addresses of two locations,  and
\fBProcessInputEvents(\|)\fP will only be called when they differ.  There are
two ways of using this:
.Ip
If your mouse and keyboard drivers support \fBSIGIO\fP,  enable this mode
when you open them,  and register a handler that increments a location.
Give \fBSetInputCheck(\|)\fP the address of this and a zero location,  and add
code to re-zero the location to \fBProcessInputEvents(\|)\fP.
The Sun code does this.
.Ip
If you have a kernel event queue that can be mapped into a user process,
give \fBSetInputCheck(\|)\fP the addresses of the head and tail pointers.\s-2\u17\d\s0
.FS
17.  See the file \fIserver/ddx/dec/qvss/qvss_io.c\fP.
.FE
This is the best alternative,  because it eliminates
the \fBread(\|)\fP system calls as well.
.LP
In both cases,  there is an interaction with the software cursor code.  When
the cursor is out of the bitmap,  this mechanism must be disabled.  We need
to ensure that \fBProcessInputEvents(\|)\fP will be called at some time soon
after the cursor is removed in order to put it back.
.Ip
If you are using \fBSIGIO\fP,  you can simply call the SIGIO handler from
the cursor removal code.
.Ip
If you are using a shared event queue,  you have to call
\fBSetEventCheck(\|)\fP in the cursor removal code,  giving it the addresses
of two locations that are always different,  and again in the code that
paints the cursor,  giving it the head and tail pointers.
.NH 2
Multiple Screens
.LP
The \fIsun\fP code supports multiple screens (``Zaphod'' mode).\s-2\u18\d\s0
.FS
18.  See \fIThe Hitch-Hiker's Guide to the Galaxy\fP by Douglas Adams.
.FE
There are two possible versions of Zaphod mode:
.Ip
``Passive'',  in which the cursor stays on one screen until some client
actively warps it to another screen.
.IP
``Active'',  in which the server warps the cursor between screens itself.
.LP
The \fIwm\fP window manager implements passize Zaphod mode,  clicking on the
background warps the pointer to the next screen in sequence.
.LP
The \fIsun\fP code implements active Zaphod mode by using the mi Cursor
code.  In \fBsunCursorOffScreen(\|)\fP, when the pointer gets to the right
(left) edge of the current screen, it is warped to the next (previous)
screen in sequence.  This is done by simply modifying the input paramenters
and returning the appropriate value.  If you aren't using the mi software
cursor code, examine it for the details on how to get the cursor to the new
screen.
.if 0 \{
	how do you deal with deciding what pixmap formats are supported,
and which ones not, in the following three cases:

	1 head, chosen from among several

	1 < n < m heads chosen from m

	n heads chosen from n

do you do table lookup here too?  this sounds like it could be
strange, especially if you have two screens with different bit or byte
order.  right now, i do it in init, by looking at some gloabls set by
each screen, which seems quite wrong.
	--raymond
There does not seem to be a problem here.

The protocol defines that there is a single format per server per
depth.  (I thought this was strange,  too,  but I got flamed at by RWS
for mentioning it,  so I guess that really is what the protocol says)

We simply use a static table that defines this format for all
depths it is possible to configure the server to support.  This table
is used irrespective of whether a screen supporting that depth was
found during auto-configuration.  Thus,  if you start the server on a
monochrome Sun,  the screenInfo struct contains:

	numPixmapFormats = 2
	formats[] = {
		1, 1, BITMAP_SCANLINE_PAD,
		8, 8, BITMAP_SCANLINE_PAD,
		}

Since the formats array is always searched with a particular depth
(derived from elsewhere and presumably valid) in mind,  these extra
entries do no harm.
\}
.if 0 \{
GCperDepth[] - entry 0 is always for depth 1.
\}
.NH
Conclusions
.LP
During the alpha & beta testing periods of X V11,  many interpid porters
attempted to adapt the sample server to their hardware.
It is to their credit,  and especially to the spirit of cooperation in
which the implementors received the enormous volume of comments and
suggestions that resulted,  that the X V11 sample server is now
remarkably easy to get running on new hardware.
A few bold strokes of the keyboard,  and the task is done.  Well,
almost,  but
not quite.  What remains is to tune the server to give of its best
on your particular hardware.  We leave this as an exercise for the reader.
.NH
Acknowledgements
.LP
Thanks to John Ousterhout and Andrew Cherenson for various suggestions
in the initial UCB port.
.LP
The Sun port of the alpha server was done by David Rosenthal,  Mike
Schwartz,  Stuart Marks, Robin Schaufler, and Alok Singhania.  It was made
much easier by the extent to which we could steal from the Sun X.10 server,
now the result of too many people's work to acknowledge individually.
However,  Paul Borman of Cray Research did particularly useful work on
keyboard support.
.LP
The \fIcfb\fP driver was originally the work of Stuart Marks (from a vague
idea by David Rosenthal), with help from Jack Palevitch (now at Apple) and
Bob Leichner of H-P Labs.  The current \fIcfb\fP release was written at the
MIT X Consortium for release 4 and shares a few file names in common with
the original code.
.LP
The version of the Sun code in the MIT release is the work of 
Adam de Boor,
David Rosenthal,
Stuart Marks,
Robin Schaufler,
Frances Ho,
Mark Opperman
and Geoff Lee.
Integration of the Sun code into the MIT release would have been impossible
without
the generous help of the Statistics Center at MIT,  who allowed us to
monopolise their Suns at all hours of the day and night,  and Todd Brunhoff
of Tektronix.
.LP
Special thanks to the Generic Workstation Company for the loan of their
hardware.
