.\"	refer -e -n -p vis.refs -s vis.nr | eqn | pic | psroff -ms
.EQ
delim $$
.EN
.ds CH
.de Ip
.IP \(bu 3
..
.de Qp
.nr PS -2
.nr VS -2
.QP
..
.de Qe
.nr PS +2
.nr VS +2
..
.de RQ
.br
.di
.nr NF 0
.if \\n(dn-\\n(.t .nr NF 1
.if \\n(TC .nr NF 1
.if !\\n(NF .if \\n(TB .nr TB 0
.nf
.rs
.nr TC 5
.in 0
.ls 1
.if !\\n(TB \{\
.	ev
.	br
.	ev 2
.	KK
.\}
.ls
.ce 0
.if !\\n(TB .rm KK
.if \\n(TB .da KJ
.if \\n(TB \!.KD \\n(dn
.if \\n(TB .KK
.if \\n(TB .di
.nr TC \\n(TB
.if \\n(KN .fi
.in
.ev
..
.\"	These macros should select a typewriter font if you have one.
.de LS
.LP
.KS
.LD
.ft L
.ta .6i 1.2i 1.8i 2.4i 3i 3.6i 4.2i
..
.de LE
.ft P
.DE
.KE
..
.de Ls
.nr PS -4
.nr VS -6
.LS
..
.de Le
.LE
.nr PS +4
.nr VS +6
.LP
..
.nr PO 1.25i
.TL
Visualizing X11 Clients
.AU
David Lemke
David S. H. Rosenthal
.AI
Sun Microsystems
2550 Garcia Ave.
Mountain View CA 94043
.AB
The color model of version 11 of the X Window System
exposes a number of aspects of the underlying display.
The concept it uses to represent these device-dependencies is the
.I Visual ;
there may be several Visuals available on a given display.
We present example programs to illustrate the techniques required if
an X client is to operate correctly across the entire range of possible
combinations of Visuals.
.AE
.LP
.DS C
Copyright \(co 1988, 1991 by Sun Microsystems, Inc.\s-2\u*\d\s0
.DE
.FS
*  Permission to use,  copy,  modify and distribute
this document for any purpose and without fee is hereby
granted,  provided that the above copyright notice and this permission
notice appear in all copies,  and that the name of Sun Microsystems,  Inc. not be
used in advertising or publicity pertaining to distribution of the software
without specific,  written prior permission.  Sun Microsystems,  Inc. makes no
representations about the suitability of the software described herein for
any purpose.  It is provided "as is" without express or implied warranty.
.br
\(dg  The X Window System is a trademark of the Massachusetts Institute
of Technology.
.br
This paper was originally presented at the Summer 1988 Usenix Conference.
There were some errors in this version,
and subsequent developments have obsoleted parts of it,
so it has been updated.
.FE
.LP
.Qp
``The polymorphic visions of the eyes and the spirit are contained in
uniform lines of small or capital letters,  periods,  commas,  parentheses \-
pages of signs packed as closely together as grains of sand representing
the many-colored spectacle of the world on a surface that is always the
same and always different,  like dunes shifted by the desert wind.''
.sp
	Italo Calvino,  \fISix Memos for the Next Millennium\fP
.sp .25i
.Qe
.NH
Introduction
.LP
A design objective of Version 11 of the X Window System\s-2\u\(dg\d\s0
.[
scheifler gettys transactions graphics 1987
.]
is to make it
possible to write client programs that operate correctly across a wide
range of displays.
This objective is often referred to as ``device-independence'',
but this is something of a misnomer.
X11 abstracts the properties of popular display types into an object
called a
.I Visual ,
and exposes to the client the existence of a number of
Visuals.
Visuals are divided into classes;
each class representing the abstraction of a particular type of display hardware.
.LP
The implementor of an X11 server for a particular display must decide
on the appropriate Visual classes to export to its clients.
More than one Visual per screen may be exported
to allow access to the full set of hardware capabilities.
When a client connects to an X11 server,
it must determine the Visuals the server is exporting and their classes,
adapting its behavior to suit.
Normally,  this will involve the client choosing one of the Visuals,  and changing
appropriate initializations.
.LP
Simply adhering to the X11 protocol does not ensure that a client
will operate correctly on all possible combinations of Visuals.
The early X11 server implementations that were made available exported
only a single Visual per screen,
and these Visuals were of only two classes.
As a result,  many existing X11 clients operate correctly on single-Visual
screens of these two classes,
but not on others.
.LP
Among these not-really-device-independent clients are the
.B xwd
and
.B xwud
programs in the X11R3 distribution.
They ``dump'' to a file,  and ``undump'' from a file,
the image of a window,  but they don't work across all combinations of Visuals.
In particular,  they don't work:
.Ip
for TrueColor or DirectColor Visuals.
.Ip
if there are subwindows that have a different depth,  Visual,
or colormap.
.Ip
if the default colormap of the screen being dumped to doesn't
mimic the colormap of the window being dumped.
.LP
We first review the Visual concept,  and then present versions of
the screen dump and undump clients that do work correctly across
all Visuals.
These clients are designed as teaching aids;
they illustrate the techniques needed to make truly device-independent
X11 clients,  but they:
.Ip
are as simple as possible,  and thus too inefficient for production use,
.Ip
ignore all the issues of interacting with window managers and other
clients,
.[
rosenthal communication conventions 1988
.]
in the interests of clarity,
.Ip
and ignore our own advice about using a toolkit,
.[
rosenthal hello world 1988
.]
in order to focus on the underlying X11 mechanisms.
.NH
Visual Classes
.LP
An X client paints in a
.I Drawable ,
a rectangular array of numbers called
pixel values,  using operations that alter the numbers,
such as CopyArea and PolyLine.
The only difference that affects the drawing process between one
drawable and another is the
.I depth ,
the number of bits in each of the numbers.
The Visual concept is irrelevant to this process;
a useful model is to think of CPU access to a display bitmap,
with the monitor turned off (see Figure 1).
.KF
.DS
.PS 3.5i 2.5i
line from 5.237,4.825 to 6.175,6.325
line from 5.237,6.325 to 6.237,4.825
line from 2.275,5.788 to 2.175,5.763 to 2.275,5.737
line from 2.175,5.763 to 4.175,5.763
line from 4.075,5.737 to 4.175,5.763 to 4.075,5.788
line from 4.737,7.013 to 6.737,7.013 to 6.737,4.325 to 4.737,4.325 to 4.737,7.013
line from 4.175,8.012 to 7.175,8.012 to 7.175,3.325 to 4.175,3.325 to 4.175,8.012
line from 0.300,6.325 to 2.175,6.325 to 2.175,5.138 to 0.300,5.138 to 0.300,6.325
.ps 7
"Drawable" at 4.925,6.606 ljust
"Memory" at 4.487,7.668 ljust
"Pixel Values" at 2.387,5.918 ljust
"CPU" at 0.925,5.731 ljust
.PE
.DE
.DS C
Figure 1:  The X11 Drawing Process
.DE
.KE
.LP
Some Drawables are
.I Windows ,
and these can be made visible by
.I mapping
them to the screen.
When the client creates a Window,  it must be assigned one of the
possible Visuals for the screen;
this Visual will control the process by which the Window's pixels
become visible when it is mapped.
A useful model is to think of video refresh access to a display
bitmap,  with the CPU halted (see Figure 2).
.KF
.DS
.PS 3.5i 2.5i
line from 6.612,5.763 to 7.112,6.888
line from 6.612,6.888 to 7.112,5.763
line from 0.988,5.075 to 1.988,7.075
line from 0.988,7.075 to 1.988,5.075
line from 3.612,7.325 to 4.862,7.325 to 4.862,5.263 to 3.612,5.263 to 3.612,7.325
line from 2.550,6.325 to 3.425,6.325
line from 3.325,6.300 to 3.425,6.325 to 3.325,6.350
line from 0.800,7.763 to 2.300,7.763 to 2.300,4.763 to 0.800,4.763 to 0.800,7.763
line from 5.925,6.325 to 6.425,5.575
line from 5.925,6.325 to 6.425,7.263
line from 6.425,7.263 to 7.237,7.263 to 7.237,5.575 to 6.425,5.575 to 6.425,7.263
line from 4.987,6.325 to 5.925,6.325
line from 5.825,6.300 to 5.925,6.325 to 5.825,6.350
line from 3.487,7.763 to 4.987,7.763 to 4.987,4.825 to 3.487,4.825 to 3.487,7.763
line from 0.550,8.075 to 2.550,8.075 to 2.550,4.513 to 0.550,4.513 to 0.550,8.075
.ps 7
"Colormap" at 3.737,7.106 ljust
"Screen" at 6.512,7.106 ljust
"Values" at 2.675,6.106 ljust
"Pixel" at 2.675,6.481 ljust
"RGB" at 5.112,6.418 ljust
"Visual" at 3.550,7.543 ljust
"Window" at 1.113,7.543 ljust
"Memory" at 0.863,7.906 ljust
.PE
.DE
.DS C
Figure 2:  The X11 Display Process
.DE
.KE
.LP
Conceptually,
when a screen pixel is to be refreshed the top-most Window's
corresponding pixel value is read,  and used to index into a
.I Colormap ,
an object containing three arrays (red,  green and blue) of
intensity values that models a video look-up table.
The Visual for the window controls:
.Ip
How this indexing is done.
.Ip
Which of possibly many installed Colormaps is used.
.Ip
Whether and how the Colormap can be modified.
.LP
Visuals are divided into six classes,  modeling six different types
of display hardware.  The classes,  and their effects on the pixel value
to visible color mapping,  are:
.Ip
.I StaticGray .
The pixel value indexes a predefined,  read-only colormap.
For each colormap cell,  the red,  green and blue values
are the same,  producing a gray image.
.Ip
.I StaticColor .
The pixel values indexes a predefined,  read-only colormap.
The red,  green and blue values for each cell are server-dependent.
.Ip
.I TrueColor .
The pixel value is divided into sub-fields for red,  green and blue.
Each sub-field separately indexes the appropriate primary of a
predefined,  read-only colormap.
The red,  green and blue values for each cell are server-dependent,
and are selected to provide a nearly linear increasing ramp.
.Ip
.I GrayScale .
The pixel value indexes a colormap which the client can alter,  subject
to the restriction that the red,  green and blue values of each cell
must always be the same,  producing a gray image.
.Ip
.I PseudoColor .
The pixel value indexes a colormap which the client can alter.
The red,  green and blue values of each cell can be selected arbitrarily.
.Ip
.I DirectColor .
The pixel value is divided into sub-fields for red,  green and blue.
Each sub-field separately indexes the appropriate primary of a
colormap that the client can alter.
.LP
The early X11 server implementations provided two Visuals:
.Ip
Servers for monochrome displays exported a single StaticGray Visual of depth 1.
.Ip
Servers for color displays exported a single PseudoColor Visual of varying depth,
typically 4 or 8.
.LP
These are the natural choices for simple workstation hardware,  but they made
it easy for initial clients to ignore many of the problems of dealing
with Visuals.
.LP
Already,  servers for common hardware such as the MIT sample
server for the DEC QDSS display
and the X11/NeWS
.[
schaufler design overview 1988
.]
server on Sun color hardware are exporting multiple Visuals.
The QDSS server's default Visual is PseudoColor,
but it also exports a StaticColor Visual.
The X11/NeWS server's default is a StaticColor Visual,
and it also exports a PseudoColor Visual.
As X11 becomes available on more complex hardware,  multiple
Visuals will become increasingly common.
.LP
Many current X11 clients make unwarranted assumptions about their Visuals.
Among the most common such assumptions\s-2\u\(dg\d\s0
.FS
\(dg  The technical term for these erroneous assumptions is
.I bugs .
.FE
are:
.Ip
That the default Visual is the only available Visual.
Clients making this assumption may fail even though a
Visual that could have supported them was available.
.Ip
That all Visuals with more than two Colormap cells are
color.
Clients making this assumption will behave strangely on
some StaticGray and GrayScale Visuals.
.Ip
That all Visuals with more than two Colormap cells have
writable Colormaps.
Clients making this (very common) assumption will fail
with Xlib errors on StaticColor and TrueColor Visuals.
.Ip
That Colormaps (and especially the default Colormap) are
infinitely large,  so that attempts to allocate private cells
in them will always succeed.
.LP
In fact,  the only clients that can ignore the question of Visuals
are those that use the
.I BlackPixel()
and
.I WhitePixel()
macros to paint a black and white image in the default Visual;
this will work on all servers though the colors may not actually
be Black or White.
.I All
other clients must pay some attention to the details of the Visual(s)
they are using.
To demonstrate how to deal with these details,  we present versions
of the screen dump and undump clients.
.NH
The ``dump'' Client
.LP
The objective of the screen dump client is to record the appearance
of a specified rectangle of the screen so that it can be restored later,
most likely on some other display device.
Thus,  the stored representation of the rectangle must be device-independent.
The simplest possible device-independent representation is the red,  green and
blue (RGB) intensities of each pixel.
.LP
The only X11 protocol request that allows a client to read back the contents of
a window is GetImage,  so the dump client clearly must be based on it.
From the point of view of the screen dump client,  there are two problems
with the specification of GetImage:
.Ip
It returns pixel values,  not RGB values.
Pixel values are not device-independent,  and so cannot be used directly.
.Ip
Some of the values it returns may be garbage.
.NH 2
Converting pixel values to RGB
.LP
To transform the returned pixel values to RGB intensities,  the client must
use the QueryColors request to look up the index in a specified Colormap.
The Colormap to use will be given by the colormap attribute of the window
in question.
.LP
The Visual to be used for this lookup is implicit.
Since setting a window's colormap attribute to a Colormap that
does not match the Visual of the window is an error,
using the Colormap attribute of the window
for the pixel value lookup implies using the correct Visual.
.NH 2
Obtaining Correct Data from GetImage
.LP
The problem of GetImage returning garbage is described in the X11
protocol specification:
.Qp
``If the window has a backing store, 
then the backing-store contents are returned for regions of the window 
that are obscured by noninferior windows;
otherwise, the returned contents of such obscured regions are undefined.
Also undefined are the returned contents of visible
regions of inferiors of different depth than the specified window.''
.[
scheifler protocol 1988
.]
.Qe
.LP
There are therefore two reasons why some of the returned pixel values may be
wrong:
.Ip
They may be obscured by non-inferior windows,  and the server may not
be maintaining backing-store for the window.
The backing-store attribute of a window is a hint to the server;
there is no way for a client to discover whether the server is actually
maintaining backing-store for any particular window at any particular time.
.IP
Thus,  unless it can be determined that a pixel in a window is
.I not
obscured by a non-inferior window,  that pixel value must be regarded as
garbage.
.Ip
They may be in a visible region of an inferior of different depth.
.IP
Thus,  unless the pixel in question can be shown
.I not
to be in an inferior of different depth,  its value must be regarded as
garbage.
.LP
In both these cases,  at some cost,  we can:
.Ip
unambiguously determine that a pixel value is good,
.Ip
assume that all others are garbage,
.Ip
and use GetImage on other windows to discover the correct values
for these assumed-to-be-garbage pixels.
.LP
Unfortunately,  there is a third reason why the pixels may be garbage:
.Qp
``When no valid contents are available for regions of a window
and the regions are either visible or the server is maintaining backing store,
the server automatically tiles the regions with the window's background
unless the window has a background of 
.B None .
If the background is 
.B None , 
the previous screen contents from other windows of the same depth as the window
are simply left in place if the contents come from the parent of the window
or an inferior of the parent;
otherwise, the initial contents of the exposed regions are undefined.''
.[
scheifler protocol 1988
.]
.Qe
.LP
In other words,  the pixels in areas whose top window has background
.B None
may not have been overwritten by any pixels drawn to the top window,
they may simply have been left there when the window was mapped or
exposed.
.LP
There is no way for a client to determine which pixels of a window have
been updated by operations to that window (or its children).
Thus there is no way to discover which pixels of a top window with
background
.B None
are garbage,
and even if there were there is no way to discover the correct pixel
value or how to interpret it.
The screen dump client simply has to live with the problem;
its results cannot be guaranteed correct if any visible windows
have background
.B None .
.LP
There is one further problem.
The protocol specification describes how a Pixmap or a single pixel
value can be used as the border for a window.
It does not specify which Visual and which Colormap are to be used to
display these values.
It appears that this is an oversight,  and that the Visual and the
Colormap of the window are to be used to display its border.
The dump client assumes this.
.NH 2
Dump Algorithm
.LP
We use the painter's algorithm,  creating an array of pixels to hold the
output,  and painting the windows into it from the back forwards.
.LP
Each pixel in the output may be painted a number of times,  but the last
time it is painted will be by the window on top at that pixel.
Since this last pixel painted is not overlaid,
we are assured that the pixels returned by GetImage will be good
(subject to the background
.B None
problem).
.LP
Here is an outline of the algorithm;
the code itself is shown in Appendix A:
.Ip
Create a data structure with one entry per pixel in the specified rectangle,
and an empty list whose entries represent Colormaps in use.
Each entry in the list is itself the head of a list of pixel values in use
with that Colormap.
.Ip
GrabServer to prevent changes to the window tree while all this is going on.
.Ip
Start at the root,  and:
.RS
.Ip
Use GetGeometry and GetWindowAttributes to find the details of
the current window.
.Ip
If the current window is Viewable:
.RS
.Ip
If the window's Colormap isn't in the list,  create an entry
describing it,  and add it to the list.
.Ip
Use GetImage on the current window,  and for every pixel in
the returned array:
.RS
.Ip
Label every corresponding pixel in the data structure with the pixel value
returned by GetImage,  and with this window's Colormap.
.Ip
If this is the first time we've seen this pixel value for this Colormap,
use QueryColor to discover the RGB values that this window's Colormap
maps the pixel value into,  and store the pixel value and RGB in the
list of pixel values in use for this Colormap.
.RE
.Ip
Use QueryTree to find the list of children of the current window and
recurse,  starting with the head (bottom-most) of the list.
.RE
.RE
.Ip
UngrabServer,  we're finished with it.
.Ip
Write to the output the width and height of the rectangle,
and the number of pixel values we've added to the Colormap lists
(this is an upper bound on the number of the distinct colors
in the output).
.Ip
For each pixel in the data structure we built:
.RS
.Ip
Find the list entry for the Colormap the pixel is labelled with.
.Ip
In the list of pixel values in use for that Colormap,
find the pixel value the pixel is labelled with.
.Ip
Write to the output the RGB value stored for its pixel value
in the list of pixel values in use for the Colormap.
.RE
.NH 2
Assessment of the Algorithm
.LP
This algorithm is about as simple as it can be while still getting
the job done,  but it causes much more protocol traffic than is
strictly required.
.LP
By making the very pessimistic assumption that a pixel overlaid by
.I any
other window is potentially garbage,
it asks the server to return the value of each of these pixels
more often than is essential.
A more efficient algorithm would only regard pixels overlaid by non-inferiors
or inferiors of different depth as potential garbage.
.LP
Space consumption may also be a problem.
There are potentially up to $ 2 sup 32 $ different pixel values,
and we must retain a list of each pixel value in use for every
Colormap.
Fortunately,
the list of values in use cannot get longer than
the $ [ w ~ times ~ h ] $ of the specified rectangle
(in the case where every pixel in the rectangle is different),
and will normally be much less.
.NH
The ``undump'' Client
.LP
The objective of the screen undump client is to
restore the appearance of the rectangle of the screen that was
dumped earlier.
It may not in fact be possible to do this,  since the Visuals
available to the undump program may not be capable of displaying
all the colors in the original image.
.LP
The undump client has two main tasks:
.Ip
Choose an appropriate Visual and create a Colormap for it.
.Ip
Build an image in memory in which the RGB values have been
converted to pixel values using the Colormap.
.LP
For each of these tasks,  we review the general problem and
then focus on the specific features needed for the undump client.
.NH 2
Choosing a Visual
.LP
The server, during the connection handshake,  provides the following
information for each Visual on each screen:
.Ip
The depth.
.Ip
The Visual class.
.Ip
The masks that identify the red,  green and blue sub-fields of the pixel
value.
.Ip
The size (number of cells) of the Colormaps of the Visual.
.Ip
The number of bits in a red,  green or blue value that the server
will regard as significant.
.LP
There can be no general rules for choosing a Visual;
a particular application might regard any of these pieces of information
as the most important factor.
For example:
.Ip
Clients which want to play Colormap tricks,
such as colormap animation,
need writable Colormap cells,
and thus a dynamic Visual.
For them,  the class is the most important feature.
.Ip
Clients which require an exact color need a PseudoColor, DirectColor
or TrueColor Visual.
Other Visuals involve the server making an approximation to the requested
RGB value.
For these clients,  the class is the most important feature.
.Ip
Clients that
.I really
care about the exact color will also be interested in the number of
significant bits,  which will tell them how closely the display can
approximate the RGB value.
.Ip
Clients that require a certain number of colors to allow for contrast,
but don't care about the exact colors,
need a Visual with a Colormap large enough to hold the colors.
For them,  the Colormap size is the most important feature.
.LP
Xlib
.[
gettys xlib 1988
.]
provides two utility routines to make selecting a suitable Visual easier,
using the
.I XVisualInfo
structure shown in Figure 3.
.KF
.Ls
.ta 8n 16n 24n 32n 40n
	typedef struct {
		Visual		*visual;
		VisualID	visualid;
		int		screen;
		int		depth;
		int		class;
		unsigned long	red_mask;
		unsigned long	green_mask;
		unsigned long	blue_mask;
		int		colormap_size;
		int		bits_per_rgb;
	} XVisualInfo;
.Le
.DS C
Figure 3: The XVisualInfo Structure
.DE
.KE
.LP
A client that simply wants to select a Visual of a given depth and class
can use
.I XMatchVisualInfo() .
The client provides a depth and a Visual class,  and an
.I XVisualInfo
structure describing one of the possibly many Visuals of that class
and depth is returned.
.LP
A client that wants more detailed control over the selection of a Visual
can use
.I XGetVisualInfo() .
The client constructs a template
.I XVisualInfo
structure,  filling in the fields it is interested in.
An array of
.I XVisualInfo
structures is returned,
one for each Visual that matches the specified fields.
For example,  the code in Figure 4
will find all PseudoColor Visuals with 256-entry
Colormaps.
.KF
.Ls
.ta 8n 16n 24n 32n 40n 48n 56n
	{
		XVisualInfo	vinfo_template,
				*vinfo_list;
		int		num_matching_visuals;
	
		vinfo_template.class = PseudoColor;
		vinfo_template.colormap_size = 256;
	
		vinfo_list = XGetVisualInfo(dpy,
			VisualClassMask | VisualColormapSizeMask,
			&vinfo_template, &num_matching_visuals);
		if (vinfo_list == (VisualInfo *) 0) {
			/* No such Visuals */
		} else {
			/* vinfo_list is an array of matches */
		}
	}
.Le
.DS C
Figure 4:  Finding all 256-entry Colormap Visuals
.DE
.KE
.LP
In the case of the undump client,  we have a problem that is
more complex than either of these two simple cases.
We have a set of RGB values
that we wish to display as well as the available Visuals will let
us.
We have an upper bound
.I N
on the number of colors,  but no other
information about the colors\s-2\u\(dd\d\s0.
.FS
\(dd  Scanning the RGB values could generate other information,
such as that all the R, G and B values were equal in a gray image.
Doing so would make the code,  and the choice of a Visual too
complex for a paper like this.
.FE
.LP
Choosing a Visual capable of displaying
.I N
different RGB values takes four steps (the actual code is the
routine
.I FindVisual()
in Appendix B):
.Ip
Rank the Visual classes in descending order of usability for
our purposes:
.RS
.Ip
TrueColor.
A TrueColor Visual with large enough colormaps would be ideal
for the dump client,  since it would not merely represent the
required RGB values,  but it would not reserve private Colormap
cells to do so.
.Ip
DirectColor & PseudoColor.
These Visuals,  if their Colormaps are big enough,  can represent the
RGB values we need,  but to do so they have to reserve private
cells,  reducing the server resources available for other clients.
.Ip
StaticColor.
A color Visual,  even though we have no control over the color values,
is likely to make a better approximation than a GrayScale Visual which
cannot display colors at all.
.Ip
GrayScale.
This Visual,  if its Colormaps were big enough,  could make a monochrome
approximation to the image better than a StaticGray Visual in which
we would have no control over the shades of gray.
.Ip
StaticGray.
If there's nothing else,  we'll have to make do with this.
.RE
.Ip
Scan the list of Visuals returned by the server
during the connection handshake,  and for each Visual class
record the Visual with the largest Colormaps.
Of course,  in the normal case,  only a few of the classes will
exist,  and the others will be recorded as having a largest Colormap of
size zero.
.Ip
In the rank order,  look for the smallest Visual with
colormaps big enough to display the required number of colors.
If one is available,  use it.
.Ip
Otherwise,  use the Visual with the largest Colormaps.
Since none of the maps are big enough to display all the colors,
the server will doing some approximation,
and the bigger the map the better the approximation.
.LP
Note that we look for the
.I smallest
Visual that will do the job.
This will ensure that the client uses no more resources than
required to get its job done.
.NH 2
Converting RGB to pixel values
.LP
X provides three fundamentally different methods for converting RGB
values to pixel values.  A client can:
.Ip
Request the server to perform the conversion and return a pixel value.
The result is to associate a sharable,  read-only Colormap cell
containing an approximation to the RGB values with the pixel value.
.Ip
Request the server to assign a writable Colormap cell and corresponding
pixel value for the client's private use.
The client can then set the Colormap cell to the RGB value.
.Ip
The client can predict the pixel value that will correspond to the
RGB value,  using its knowledge of the contents of the Colormap.
.LP
The choice of a suitable method depends on the requirements of the
application and on the class of the Visual it is using.
.NH 3
Server does conversion
.LP
The AllocColor request takes an RGB triple and a Colormap as an
argument,  and returns a pixel index that points to a
Colormap cell containing the best approximation the server can
make to that RGB value,  and the actual RGB values that the cell
contains.
It does so by:
.Ip
If the Visual is static (StaticGray, StaticColor,  or TrueColor),
it returns the cell in the map that is closest to the request RGB
value.
.Ip
If the Visual is dynamic,  and the Colormap has free cells,  the
server will allocate an cell,  mark it read-only,  and set it to
the requested RGB value.
If there are no free cells,
the request will fail.
.LP
This technique can be used by applications except those for
which the
.I exact
representation of a color is of primary importance,
or which use very large numbers of colors.
The reasons for preferring this method are that it will work on
any Visual,  and that it makes the most efficient use of the
server's resources.
.LP
Note,  however,  that since AllocColor returns a value,  it
requires a round-trip to the server and is therefore slow.
Note also that AllocColor may fail,
if the Visual is dynamic and there are no free cells in the colormap.
If it does fail,
the normal response is to move the client's allocated cells from
the full colormap to a newly created one using CopyColormapAndFree.
.NH 3
Client sets color
.LP
If the Visual is dynamic,
clients can use AllocColorCells or AllocColorPlanes to reserve one
or more pixel values for their private use.
If the allocation succeeds,  the pixel values will point to private,
writable Colormap cells.
The client can then use StoreColors to set the RGB values of these
cells to exactly the values it desires.
.LP
This technique is more efficient than AllocColor,
because a round-trip to the server is needed only for the initial allocation
rather than every time a color is modified.
However,
it should be used only if it essential to the application because
.Ip
it will work only on a dynamic Visual,
.Ip
it consumes server resources that no other client can use until they
are released,
.Ip
it can fail and measures must be taken to cope with this possibility.
.LP
In general,  clients using this technique should create a private Colormap,
perhaps using CopyColormapAndFree to preserve any pre-allocated cells.
Ideally,
the default Colormap for each Visual should be left for clients using the
other techniques to maximize the sharing of resources.
Unfortunately,  the protocol defines a default Colormap only for the Visual
of the root window.
.NH 3
Client predicts pixel value
.LP
There are two circumstances in which a client can determine the pixel
value corresponding to a given RGB value
.I without
a round-trip to the server.
To do so,  it must have knowledge of the contents of the Colormap:
.Ip
If the Visual is TrueColor,
the Colormap is known to provide linear ramps in each primary color.
The R, G and B values can thus be adjusted to the match the corresponding
sub-field mask,  and or-ed together to make the pixel value.
.Ip
If the Colormap is one of the set of ``standard'' Colormaps
a similar calculation can be performed.
Applications wishing to use one of these Colormaps look for a property
describing it on the root window of the screen;
if the property is not found the application creates a suitable Colormap
and a property to describe it.
.LP
In particular,
clients should find a property called RGB_DEFAULT_MAP on the root
window of a multi-visual screen
containing an array of XStandardColormap structures describing a
default colormap for each visual,
and the linear RGB ramps within it.
They can use this to share colors with other applications on the screen;
this is the preferred method for most clients.
.NH 2
The Undump Algorithm
.LP
Here is an outline of the algorithm;
the code itself is shown in Appendix B:
.Ip
Read in the width,  height,  number of colors & RGB values.
.Ip
Choose an appropriate Visual
.Ip
Create a Colormap for the chosen Visual.
.Ip
Create an image in memory to hold the pixel values that will
eventually appear in the window.
.Ip
For each pixel in the data:
.RS
.Ip
Convert the RGB values for the pixel into a pixel value,
using the AllocColor request.
.Ip
Store the pixel value into the corresponding location in the
image.
.RE
.Ip
Create and map a window which uses the Colormap.
.Ip
Paint any exposed part of the window with the pixel values
from the image.
.NH 2
Assessment of the Algorithm
.LP
Once again,  this algorithm is about as simple as it can be
and still get the job done.
As a result,  it is grossly inefficient.
It makes the pessimistic assumption that every pixel in the
image is a different color,  and calls AllocColor to convert its RGB
values into a pixel value.
Of course,  it is likely that many pixels are the same color,
and calling AllocColor only for every distinct color would reduce the
traffic considerably.
.LP
One problem is that this client creates its own Colormap.
If the default Colormap mechanism worked for multiple Visuals,
we could have used the appropriate default Colormap.
Ideally,  clients should share Colormaps to improve the chance they
will appear in their correct colors.
We should have used the RGB_DEFAULT_MAP or RGB_BEST_MAP properties
to share a Colormap with other similar clients.
.LP
The client will work on every Visual type,  in most cases providing the
best match available for the colors in the image.
There is,  however,  one case in which the match will be less than
optimal.
If the Visual is PseudoColor and is too small to accommodate the number
of colors in the image,
the first colors encountered in the right-to-left within top-to-bottom
scan of the image will get exact colors as new read-only cells
are allocated by AllocColor.
At some point,
the map will fill up,
and the next AllocColor call will fail.
Since we found the largest visual,
there is nothing to do but give up.
.LP
The Visual selection process has two problems.
For the TrueColor and DirectColor cases,
it pessimistically assumes that all the distinct colors in the image differ
only in one component,  and will thus overestimate the size of the Colormap
required.
For example,  suppose we have a blue image with 24 distinct colors;
all 24 have the same Red and Green values,  differing only in their Blue value.
We would need a Colormap with 24 entries for Blue,  but only 1-entry
maps for Red and Green.
In practice,  it is likely that the color distribution will be more even and
smaller maps would suffice.
.LP
Further,  it is complicated by the fact that the number of distinct colors
is only an upper bound.
Consider a dump of a screen with an 8-bit PseudoColor Visual and a 1-bit
StaticGray Visual.
Assume that the image in the PseudoColor Visual had 256 different colors,
two of which were white and black.
The dump client would not realize that the white and black of the StaticGray
Visual were the same as the white and black of the PseudoColor Visual
and would record the number of colors as 258.
Undumping to the same display,  it would regard the PseudoColor Visual as
inadequate to represent the 258 colors.
.LP
Better analysis of the set of RGB values to be displayed would solve both
problems,  but would make the program too complex for this paper.
.NH
Visuals and the Toolkit
.LP
The attention paid by the X Toolkit Intrinsics to the problem
of specifying Visuals is evident from the fact that the word Visual
does not appear in the index to the first version of the Intrinsics manual.
.[
toolkit intrinsics 1988
.]
In R4,
this was remedied.
.[
asente swick 1990
.]
.LP
The Window for a Widget is created during the process of 
.I realizing
the Widget,
and the Visual to use has to be selected as part of this process.
Different Widget classes will differ as to how they choose a Visual.
All Athena Widgets use CopyFromParent as their Visual selection,
deferring the choice to their parent.
The root of the Widget tree is a so-called Shell Widget,
which in the Intrinsics implementation itself by default inherits its Visual
from its parent,  in this case the root.
From R4 on,
Shell widgets have a XtNvisual resource than can be used to specify the
visual to be used by widgets within the shell's tree.
.LP
The core information,  which all Widgets possess,  includes a Colormap.
For the Athena Widgets,  this had better be a Colormap from the root's
Visual,  or Xlib errors will occur.
This Colormap is used to convert RGB values to pixels for the Widget,
using the AllocColor technique.
One might think that it would be possible to realize a Widget in a Visual
other than the root's by giving it a Colormap from that Visual,  but
this will not work.
.LP
A Widget class that needed some Visual other than its Shell's
would have to implement its own realize procedure,  and make the choice there.
The choice algorithm would be the same for all instances of the Widget,
but this does not mean that all instances would have to have the same Visual.
This sounds onerous,  but it is in fact the correct approach.
As we showed above,
the algorithm for choosing an appropriate Visual is application-specific;
it would not be possible to wire-in to the Intrinsics a single algorithm
that would satisfy everyone.
.NH
Conclusion
.LP
The X11 color model provides powerful access to the capabilities
of different types of display hardware.
Used carelessly,
this access can lead to programs which run only in the
environment in which they were developed.
.LP
All but the simplest X11 clients must take care to choose the most
suitable Visual for their purposes,
and to adapt their behavior to its capabilities.
We have presented the techniques for doing so for a simple
application,  but more complex applications will require a wider
range of techniques.
More work is needed to develop these.
.SH
Acknowledgements
.LP
Thanks are due to James Gosling,  who helped to develop our understanding
of these issues,  and who designed the multi-Visual and Colormap capabilities
of the X11/NeWS server,
to Jeff Vroom,  who spotted the problem of the default colormap,
to Bob Scheifler,  who clarified several obscure points,
and to the Usenix reviewers.
.[
$LIST$
.]
.SH
Appendix A: Dump Program
.LP
.Ls
.ta 8n 16n 24n 32n 40n 48n 56n 64n
.so xsd.c
.Le
.SH
Appendix B: Undump Program
.LP
.Ls
.ta 8n 16n 24n 32n 40n 48n 56n 64n
.so xsud.c
.Le
