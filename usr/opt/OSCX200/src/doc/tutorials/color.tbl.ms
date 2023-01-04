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
.de Su  \" if $1 = +, this gives a superscript; $1 = - gives a subscript.  
.       \" If 2 args, $1=super/subscripted text with post-space
.       \" If 3 args, $2=regular text; $3=super/subscripted text w/pre-space
.if "\\$1"+" \{\
.       if \\n(.$=3 \\$2\v'-4p'\s-3\\$3\v'4p'\s0
.       if \\n(.$=2 \v'-4p'\s-3\\$2\v'4p'\s0\}
.if "\\$1"-" \{\
.       if \\n(.$=3 \\$2\v'3p'\s-3\h'2p'\\$3\v'-3p'\s0
.       if \\n(.$=2 \v'3p'\s-3\\$2\v'-3p'\s0\h'2p'\}
..
.\ "MIT header page and copyright notice
.\" MIT page header and footers
.ps 10
.nr PS 10
.nr LL 5.75i
.nr PO .75i
.EH ''''
.OH ''''
.EF ''''
.OF ''''
\.".EQ
\."delim !!
\.".EN
.ad b
.sp 8
.ce 4
\s+1\fBColor\fP\s-1
.sp
\s+1\fBA Chapter from O'Reilly and Associates\fP\s-1
.br
\s+1\fBXlib Programming Manual\fP\s-1
.sp 6
.ce 2
\s-1Adrian Nye
.sp 6p
\s-1O'Reilly and Associates, Inc.\s+1
.sp 2
.sp 8
.sp .5
The X Window System is a trademark of MIT.
.sp .5
Copyright \(co 1990  O'Reilly and Associates, Inc.
.sp .5
Permission to use, copy, modify and distribute this documentation for any
purpose and without fee is hereby granted, provided that the above copyright
notice appears in all copies and that both that copyright notice and this
permission notice appear in supporting documentation, and that the name of
O'Reilly and Associates, Inc. not be used in in advertising or publicity pertaining
to distribution of the software without specific, written prior permission.
O'Reilly and Associates, Inc makes no representations about the 
suitability of the software described herein for any purpose.
It is provided ``as is'' without express or implied warranty.
.bp 
.sp 1
.ce
\s+1\fBAcknowledgments\fP\s-1
.sp 2
We'd like to thank Robert Scheifler for reviewing this document.
His comments on an earlier draft are represented here, but he has
not seen this draft, and it is not certain that his view
is fully and correctly represented.
.sp .5
Al Tabayoyon, Paul Shearer, and others at Tektronix also provided
valuable comments on earlier drafts.
.pn +170
.bp
.nr HM .75i
.nr FM 3i
.ad b
.af PN 1
.EH ''''
.OH ''''
.EF '%''\f(HIXlib Programming Manual\fR'
.OF '\f(HIColor\fR''%'
.sp 3
.ti 5.65i
\s24\f(HB7\fR\s10
.sp 
.ti 5i
\s20\f(HBColor\fR\s10
.sp 4
.IP "" 6n
.Ti "Xlib Programming Manual"
.ds u: \o'u\(..'
.Se "7" "Color"
.St "Color"
A typical X application allows the user to specify colors for the 
background and border of each of its windows, colors for the cursor, and 
foreground and background colors to be set in GCs for drawing text and 
graphics.  More complex applications (such as Computer Aided Design (CAD) 
applications) might use color to distinguish physical or logical 
layers.  Still more complex applications, such as in imaging, might use 
fine gradations of color to represent real-world data.  Yet in discussing 
the background and border window attributes and how to set the foreground 
and background members of the GC, we have spoken only of pixel values.
.sp .5
.\" I don't like this PP now
How are these pixel values translated to colors?  And how must an X 
client manage color if it is to run successfully on the wide variety 
of screen hardware available in the X environment?
.sp .5
Because X must support a wide variety of systems with differing screen 
hardware, the Xlib color-handling mechanisms are fairly complex.  Even 
programmers who have previously written color graphics applications will 
find there are some new concepts to learn.
.sp .5
This chapter starts out by describing the different types of screens 
that an X application may run on and the mechanisms Xlib provides for 
determining the screen type.  It then describes the simplest color-allocation 
mechanisms, which could be used by applications whose principal use of 
color is for decoration.  It proceeds to discuss more complex color 
applications and concludes with a section on writing applications that 
will be portable across different types of color and monochrome screens.
.IP \s+4\f(HB7.1\fR\s-4  6n
\s+4\f(HBBasic Color Terms and Concepts\fR\s-4
.sp
Most color screens on the market today are based on the RGB color 
model.  Each pixel on the screen is actually made up of three 
phosphors:  one red, one green, and one blue.  Each of these three 
phosphors is sensitive to a separate electron beam.  When all three 
phosphors are fully illuminated, the pixel appears white to the human 
eye.  When all three are dark, the pixel appears black.  When the 
illumination of each primary color varies, the three phosphors generate 
an additive color that might seem surprising.  For example, equal 
portions of red and green, with no admixture of blue, make yellow.  
Most people are more familiar with subtractive color mixing, used 
in paints, where red, yellow, and blue are the three primary colors 
from which all other colors (except white and shades of gray) can be made.
.sp .5
You, no doubt, know that a color screen uses multiple bits per pixel 
(also referred to as multiple planes) to specify colors.  A \fIcolormap\fP 
is used to translate each pixel's value into the visible colors you see 
on the screen.
.sp .5
A colormap is no more than a lookup table stored in the server.  Any 
given pixel value is used as an index into this table\(emfor example, a 
pixel value of 16 will select the sixteenth element, or \fIcolorcell\fP.
.sp .5
On the most common type of color system, each colorcell contains 
separate 16-bit intensity values for each of the three primary colors.  
.sp .5
As shown in Figure 1, a pixel value uniquely identifies a 
particular colorcell.  Each pixel value in the visible portions of a 
window is continuously read out of screen memory and looked up in the 
colormap.  The RGB values in the specified colorcell control the intensity 
of the three primary colors and thus determine the color that is displayed 
at that point on the screen.
.sp
Note: Figure 1 "Pixel value to RGB mapping with the colormap on a color 
screen" here.
.sp
The range of colors possible on the screen is a function of the 
number of bits available in the colormap for RGB specification.  
If eight bits is available for each primary, then the range of 
possible colors is
.Su + 256 3
(about 16 million colors).
.sp .5
However, the number of different colors that can be displayed 
on the screen at any one time is a function of the number of 
planes.  A 4-plane system could index
.Su + 2 4
.hw distinct
colorcells (16\ distinct colors); an 8-plane system could index
.Su + 2 8
colorcells (256 distinct colors); and a 24-plane system could index
.Su + 2 24
colorcells (over 16 million distinct colors).
.sp .5
A client attempting to use color does not \fIspecify\fP a pixel value 
.\" (read-only or read/write) 
and the color to be put in that cell in order to draw in a given color.  
Instead, it requests access to a colorcell in a colormap (managed by 
the server) and is \fIreturned\fP a pixel value.  This is called
\fIallocating\fR a color.  When a client allocates a color, it asks 
the server, "Which colorcell can I use?" and the server responds by 
saying, "You can use the colorcell specified by this pixel value."  
There are three functions that allocate colors, which are described 
in detail and demonstrated in later sections in this chapter.
.sp .5
.IP \s+2\f(HB7.1.1\fR\s-2 6n
\s+2\f(HBThe Color Name Database\fR\s-2
.sp .5
In order to simplify color specification and to promote sharing
of colors, X provides a color database that translates string color 
names into RGB values.  As described above, sharing of colorcells
can happen only if two clients allocate a read-only cell with the 
exact same RGB values.  If both clients allocate a color specified
by one of the 300-odd string names, there is a much better chance of 
them selecting the exact same RGB values and thereby sharing a cell 
than if they use one of the
.Su + 2 48
possible combinations of RGB values.
.sp .5
Because of differences in screen hardware, the same RGB values may
generate quite different colors on different hardware.  Therefore, 
server implementors will change the RGB values corresponding to each 
color name to make sure that the appropriate color appears on their 
screen.  This is called \fIgamma correction\fR.  By using names 
from this database, you are more sure of getting a color close to 
the one you request.  If the server implementor has not provided a 
gamma-corrected color database, there is no way a program can tell 
exactly what color is being displayed even when it knows the RGB values.
.sp .5
It is also important to note that the color names are not specified 
by the X11 protocol or Xlib.  Therefore, server implementors may 
change them, but more often, they will simply add to the list.  
(Note that some servers allow users to customize this file.  
For more information, see VOL3.)
.sp .5
Table 1 shows the color names and corresponding RGB values 
in the default color database for R3.  The R4 database is much more 
extensive.  The text 
version of this database in the standard distribution on a UNIX-based 
system is in the file \fI/usr/lib/X11/rgb.txt\fP.  The location of this 
file may vary.  
.sp .5
The color names in the color database are strings in which each
character uses the ISO Latin-1 encoding.  The ISO (International
Standards Organization) encoding is used by virtually all workstations 
manufacturers.  What this means is that the first 127 character codes 
correspond to 7-bit ASCII and are the normal English characters that 
appear on U.S. keyboards.  But ISO characters are 8-bit, and the 
characters from 128 to 255 are used for characters with accents and 
other variations, necessary for other Western languages.  
.sp .5
Server vendors should be able to supply a color database file for 
each foreign language.  The RGB values would be the same, but the 
names would be different.  In the English file, the entry for green 
is encoded with the ISO character codes 103\ (g), 114\ (r), 101\ (e), 
101\ (e), 110\ (n).  In German, the same entry would be for \fIgr\*(u:n\fR, 
encoded with the ISO codes 103\ (g), 114\ (r), 252\ (\*(u:), 110\ (n).
In a workstation configured for German, there will be an easy way to 
type \fI\*(u:\fR.
.sp .5
Note that keysyms also use the ISO Latin standard.
.sp
"The R3 Color Database*"
.sp
.TS H
linesize(2);
l | l | l | l
l | n | n | n.
English Words	Red	Green	Blue
.sp 2p
_
.sp 2p
.TH
aquamarine	112	219	147
black	0	0	0
blue	0	0	255
blue violet	159	95	159
brown	165	42	42
cadet blue	95	159	159
coral	255	127	0
cornflower blue	66	66	111
cyan	0	255	255
dark green	47	79	47
dark olive green	79	79	47
dark orchid	153	50	204
dark slate blue	107	35	142
dark slate gray	47	79	79
dark slate grey	47	79	79
dark turquoise	112	147	219
dim gray	84	84	84
dim grey	84	84	84
firebrick	142	35	35
forest green	35	142	35
gold	204	127	50
goldenrod	219	219	112
gray	192	192	192
green	0	255	0
green yellow	147	219	112
grey	192	192	192
indian red	79	47	47
khaki	159	159	95
light blue	191	216	216
light gray	168	168	168
light grey	168	168	168
light steel blue	143	143	188
lime green	50	204	50
magenta	255	0	255
maroon	142	35	107
medium aquamarine	50	204	153
.sp 5p
_
.TE
.in 2.85i
.rt 1.295i
.TS H
linesize(2);
l | l | l | l
l | n | n | n.
English Words	Red	Green	Blue	
.sp 2p
_
.sp 2p
.TH
medium blue	50	50	204
medium forest green	107	142	35
medium goldenrod	234	234	173
medium orchid	147	112	219
medium sea green	66	111	66
medium slate blue	127	0	255
medium spring green	127	255	0
medium turquoise	112	219	219
medium violet red	219	112	147
midnight blue	47	47	79
navy	35	35	142
navy blue	35	35	142
orange	204	50	50
orange red	255	0	127
orchid	219	112	219
pale green	143	188	143
pink	188	143	143
plum	234	173	234
purple	176	0	255               
red	255	0	0
salmon	111	66	66
sea green	35	142	107
sienna	142	107	35
sky blue	50	153	204
slate blue	0	127	255
spring green	0	255	127
steel blue	35	107	142
tan	219	147	112
thistle	216	191	216
turquoise	173	234	234
violet	79	47	79
violet red	204	50	153
wheat	216	216	191
white	252	252	252
yellow	255	255	0
yellow green	153	204	50
.sp 5p
_
.TE
.in -2.75i
.sp 10p
_______________
.br
.vs 10
\s-2*Also defined are the color names "gray0" through "gray100", 
spelled with an "e" or an "a".  "gray0" is black and "gray100" is 
white.
.in 0 
.vs
.sp .5
.IP \s+2\f(HB7.1.2\fR\s-2 6n
\s+2\f(HBHexadecimal Color Specification\fR\s-2
.sp .5
It is also possible to specify colors using a hexadecimal string.
.sp .5
The hexadecimal form of color specification is necessary, since you may 
want the user to be able to specify an exact color, not just the rough
approximation allowed by an string name.  The hexadecimal specification 
must be in one of the following formats:
.sp .5
\s-1\f(CW
.nf
#RGB			(\fI4 bits each of red, green, and blue\fP)
#RRGGBB		(\fI8 bits each of red, green, and blue\fP)
#RRRGGGBBB	(\fI12 bits each of red, green, and blue\fP)
#RRRRGGGGBBBB	(\fI16 bits each of red, green, and blue\fP)
\fR\s0
.fi
.sp .5
Each of the letters represents a hexadecimal digit.  In the shorter
formats, the specified values are interpreted as the most significant
bits of a 16-bit value.  For example, \f(CW#3a7\fP and 
\f(CW#3000a0007000\fP are equivalent.
.sp .5
Use of hexadecimal color specifications does not preclude colorcell 
sharing, since the user could specify the same hexadecimal value for 
the color for two or more clients.  However, it probably tends to make
sharing less likely, since a window manager might allocate all the
colors in the color database as read-only cells, and then any client 
that uses hexadecimal specifications will probably be allocating a 
separate cell instead of sharing.
.IP \s+4\f(HB7.2\fR\s-4  6n
\s+4\f(HBDifferences in Display Hardware\fR\s-4
.sp
The description of color mapping given in the previous section was
actually somewhat over-simplified.  There are significant differences
in how the colormap is used on mid-range color screens, monochrome
and gray-scale screens, and high performance color screens.  Color 
handling in X was designed to work with any of these hardware types.
.sp .5
.IP \s+2\f(HB7.2.1\fR\s-2 6n
\s+2\f(HBMid-range Color Displays\fR\s-2
.sp .5
The most common type of color screen has between four and eight planes
and uses the colormap indexing technique described above.  This type 
of screen is so widespread because it provides a flexible color system 
while being moderately priced.  The mapping of pixel values to colorcells, 
with arbitrary RGB values stored in each colorcell, allows a very large 
range of possible colors, even though a more limited number can be shown 
.ne 2
on the screen at any one time.
.sp .5
Mid-range color screens usually have only one hardware colormap.
In other words, the pixel values in all the windows on the screen 
are mapped to colors using the same colormap.  On most of these
systems, however, the color in any colorcell in the hardware colormap 
can be individually changed, and therefore, the entire colormap can 
be replaced with a new set of values.  X provides the concept of the 
\fIvirtual colormap\fR, so that more than one set of colorcells can 
be maintained, even though only one of them can be in use at a time.  
Virtual colormaps are swapped in and out of the hardware colormap by 
the window manager.  This makes it possible for an application that 
has special color needs to create its own virtual colormap, which 
the window manager will load into the hardware colormap when that 
application is in use.  However, since only one hardware colormap is
available and all applications share it, when any one application 
creates a new virtual colormap and the window manager installs it, 
all other applications will screen in false colors, since the pixel
values they use now point to cells in the other client's colormap. 
This is acceptable, since the window manager always installs the 
correct colormap for the application in use, but it is obviously 
not ideal.  On high performance systems, described below, this 
problem is solved by having multiple hardware colormaps.
.sp .5
.IP \s+2\f(HB7.2.2\fR\s-2 6n
\s+2\f(HBMonochrome and Gray Scale\fR\s-2
.sp .5
Monochrome (black and white) screens have only a single plane of 
screen memory.  Each pixel is made up of a single phosphor, which 
can be either on or off.
.sp .5
Gray-scale screens are sometimes used for publishing applications, 
since pixels made up of a single phosphor are smaller than those 
made up of three phosphors and the resolution is, therefore, better.
As shown in Figure 2, a gray-scale screen works by looking 
up the intensity of the pixel in the colormap, which, for this screen 
type, contains only a single value.  This controls the intensity of a 
single electron beam.  Gray scale can be simulated on a color screen 
by making the red, green, and blue values equal in a given colorcell 
to determine the brightness of gray pixels on the screen.
.sp
Note: Figure 2 "Pixel value to RGB mapping \(em gray scale and 
monochrome screens" here.
.sp
A gray-scale screen might have a read-only colormap, so that 
the gray levels in each cell could not be changed.  A monochrome 
screen is an example of this type; it is a single-plane screen 
with a two-element read-only colormap.
.sp .5
.IP \s+2\f(HB7.2.3\fR\s-2 6n
\s+2\f(HBHigh Performance Color Displays\fR\s-2
.sp .5
As memory has become cheaper and applications more advanced,  
workstations with 24\ planes and more have become more common.  With 
24 bits per pixel, it is possible to screen every discernable color
at the same time.  This makes it possible to do smooth shading and 
other applications that use a large number of closely spaced colors.
.sp .5
The problem with having so many planes is that a colormap of the 
style used in mid-range color screens would be impossibly large:  it 
would contain over 16 million entries.  Instead, the available bits 
per pixel are broken down into three separate colormap indices, one 
for each primary color, as shown in Figure 3.  This 
approach still allows the full range of colors to be generated but 
makes the job of loading the colormap much more manageable.  This 
scheme requires three primary colormaps of only 256 entries each
to specify all 16 million colors on a 24-plane system.
.sp
Note: Figure 3 "Pixel value to RGB mapping \(em high performance color 
screens" here.
.sp
In high performance screens, having a read-only colormap makes just 
as much sense as having it read/write, because nearly every color 
imaginable can be simultaneously available.  With a read-only 
colormap, there is a fixed relationship between the pixel values used 
to select a color and the actual RGB values generated.  This makes 
possible applications that want to calculate pixel values directly 
instead of having to calculate colors and then determine which pixel 
value represents that color, as is necessary when the colormap is read/write.
.sp .5
In reality, most screens in this class let you use the color resources
in either fashion, using virtual colormaps.  There can be one read-only
virtual colormap and one read/write virtual colormap.  However, unlike
on mid-range color screen hardware, most high performance color systems 
have multiple hardware colormaps, so that both virtual colormaps can be 
installed and used at the same time.  In fact, on many of these systems, 
each window can have its own virtual colormap installed in the hardware 
at the same time.
.sp .5
.IP \s+2\f(HB7.2.4\fR\s-2 6n
\s+2\f(HBHow X Describes Color Support with Visuals\fR\s-2
.sp .5
A \fIvisual\fR describes the characteristics of a virtual colormap 
that has been or can be created for use on a particular screen.  As 
used by Xlib, a visual is actually a pointer to a structure (of type 
\f(CWVisual\fP) containing information about one way of using a particular 
screen.  A visual must be specified when creating a colormap or a 
window, and the same visual must be used in creating a window as is 
used to create the colormap to be used in that window.
.sp .5
Most windows inherit their parent's visual, and windows will often 
share the root window's visual, which is known as the default visual.  
The default visual describes, naturally, the default colormap.  If 
you create all your windows with \f(CWXCreateSimpleWindow\fP, you will 
be using the default visual and colormap.
.\"If you need to get a pointer to the default visual, you can do so 
.\"with the \f(CWDefaultVisual\fP macro.
.sp .5
The \f(CWVisual\fP structure is intended to be opaque; programs 
are not supposed to access its contents.  This is so that Xlib 
implementors can change the structure without breaking existing 
clients.  The procedure used to avoid accessing its members is
not all that cumbersome but is just beginning to come into use 
by application writers.  Up to this point, most programmers have 
broken this rule.  We will show you only the correct method here,
since it adds only a few lines to the application.
.sp .5
Even more existing applications have avoided visuals altogether and 
used only the \f(CWDefaultDepth\fP or \f(CWDisplayPlanes\fP macros 
to attempt to determine whether the screen is monochrome or color.
However, this does not work in general, because it does not distinguish
between gray-scale screens and color screens (both have more than
one plane).  The only way to make this distinction is to get 
information about visuals.
.sp .5
Remember that a visual is only one way to use color on a particular
screen.  There may be a list of supported visuals on a screen, with 
.hw write-ability
each visual describing a different depth and writeability of the 
colormap.  On a color system, there may be both monochrome and 
color visuals available.
.sp .5
The correct method to get information about the visuals supported 
on a particular screen is to use \f(CWXMatchVisualInfo\fP or 
\f(CWXGetVisualInfo\fP.  These functions return \f(CWXVisualInfo\fP 
structures that contain information about the available visuals and 
are public so their fields can be safely accessed.  
.Nd 8
.sp .5
The \f(CWclass\fP member of \f(CWXVisualInfo\fP contains a constant 
specifying one of six different visual classes,* 
.FS 
*Do not confuse \fIvisual\fR class with \fIwindow\fR class.  While 
both are represented in certain structures as the \f(CWclass\fR member 
and both are set when a window is created and cannot be changed, they 
are quite different.  The window class is \f(CWInputOutput\fP or 
\f(CWInputOnly\fP.  The visual class is only part of the overall 
visual, which is the way color is represented for a window.
.FE
corresponding to the basic ways of using a screen:  \f(CWDirectColor\fP, 
\f(CWGrayScale\fP, \f(CWPseudoColor\fP, \f(CWStaticColor\fP,
\f(CWStaticGray\fP, or \f(CWTrueColor\fP.
.sp .5
As summarized in Table 2, the visual classes distinguish between 
color or monochrome, whether the colormap is read/write or read-only, 
and whether a pixel value provides a single index to the colormap or 
is decomposed into separate indices for red, green, and blue values.
.sp
Table 2: "Comparison of Visual Classes" 
.sp
.TS
linesize(2), tab(@);
l | l | l
l | lp9fCW | lp9fCW.
Colormap Type@Read/Write@Read-only
.sp 2p
_
.sp 2p
Monochrome/Gray@GrayScale@StaticGray
Single Index for RG&B@PseudoColor@StaticColor
Decomposed Index for RG&B@DirectColor@TrueColor
.sp 5p
_
.TE
There may be more than one way of using color on a particular screen,
and therefore, there may be more than one supported visual.  This is 
usually true of high-end workstations.  There are ways to search 
through the available visuals to select the one that most closely 
meets the needs of your application, as will be described later.
Several visuals of the same class may be provided but at different
depths.  On high performance screens, it is possible to create the 
colormap as read/write or as read-only.  Both methods have certain 
advantages and would be used for different applications.  There would 
be a separate visual for each of these ways of using the screen hardware.  
One of these visuals would be \f(CWTrueColor\fP class and the other 
\f(CWDirectColor\fP class.  Some 24-plane screens allow the screen 
to be treated as two separate 12-plane \f(CWPseudoColor\fP visuals.  
(This allows for "double-buffering," a technique useful for animation, 
or for storing distance data to simplify hidden line and plane 
calculations in 3-D applications.)  In fact, on some advanced 
workstations, you can use a different visual in each window.
.sp .5

Figure 4 schematically represents the visual classes that 
can theoretically be supported by each type of screen hardware.  
A screen that supports the \f(CWDirectColor\fP class can 
theoretically support any of the six visual classes.  A screen 
that supports the \f(CWPseudoColor\fP visual class can support 
\f(CWGrayScale\fP, \f(CWPseudoColor\fP, \f(CWStaticColor\fP, 
or \f(CWStaticGray\fP visual classes.  A screen that supports the 
\f(CWGrayScale\fP visual class can also support \f(CWStaticGray\fP 
visual classes.  The three types of screen with read-only colormaps 
can only support visuals of their own class.  But remember that 
just because a certain visual class can theoretically be supported 
by a certain screen hardware does not mean that the server 
implementors will decide to support that class.  
.sp
Note: Figure 4 "Hierarchy of visual classes" here.
.sp
.IP \s+2\f(HB7.2.5\fR\s-2 6n
\s+2\f(HBShareability vs. Changeability\fR\s-2
.sp .5
Notice that \f(CWDirectColor\fP, \f(CWGrayScale\fP, and 
\f(CWPseudoColor\fP visuals have changeable colormaps, but
\f(CWStaticColor\fP, \f(CWStaticGray\fP, and \f(CWTrueColor\fP
have immutable colormaps.  Within the changeable colormaps, it 
is possible to have two types of colorcells:  read-only and read/write.
The color in a read-only cell is set once by one client and from then
on can be shared by any client but not changed.  A read/write cell
can have its color changed at any time by the client that allocated 
it but cannot be shared by other clients.  In immutable colormaps, 
you are limited to only read-only cells.  
.sp .5
One advantage of immutable colormaps is that all the cells are 
read-only and can be shared between clients, so all the cells 
are available to every client.  Immutable colormaps also make 
it possible to calculate pixel values from the colors desired 
without querying the server, since the mapping between pixel 
values and colors is predictable.  This technique is necessary 
for smooth shading and 3-D rendering algorithms.  As you will 
see, this is usually not possible with changeable colormaps.
The disadvantages of immutable colormaps are that there may not
be the exact color you desire (if there are a small number of 
planes) and you cannot allocate read/write cells, so you cannot 
change a colorcell to change the color of existing pixels on the 
screen.  To change a color, you have to redraw the graphics with 
a new pixel value.
.sp .5
In general, the advantage of changeable colormaps is that you 
can have both private read/write cells and shareable read-only 
cells.  That is why \f(CWPseudoColor\fP and \f(CWDirectColor\fP
are the most useful visuals, when a screen supports them.
\f(CWPseudoColor\fP and \f(CWDirectColor\fP allow you to decide
whether your client really needs read/write cells or whether it 
can use read-only cells.  Read-only usage is preferred, since 
these cells can be shared by all clients, which means that the 
colormap is less likely to run out of free cells.
.Nd 2
.sp .5
Try not to confuse the writeability of colormaps with the writeability 
of colorcells.  A colorcell in a read/write colormap can be allocated 
read/write or read-only.  A colorcell in a read-only colormap can only 
be allocated read-only.  A changeable colormap could be made entirely 
read-only if the window manager or any other client allocates all 
available colorcells read-only.  
.sp .5
The advantages of read/write colorcells, available only in 
changeable colormaps, are that your program can select exactly 
the color you want (as long as it is physically possible on the 
screen) and you can change the color at will, which instantly 
changes the visible color of everything drawn with that pixel 
value if the colormap is currently installed.  Although any other 
client can also change the values in a read/write cell, it is a 
convention that only the client that allocated the cell should 
change its contents.  You \fIown\fR that pixel value.  Since most 
clients cannot be satisfied with having no control over their 
displayed colors, this pixel value is not shareable.  That means 
that if several clients that use read/write colorcells are running, 
all the colorcells might be used.  Then some client will be forced 
to create its own colormap, with the negative consequences described 
in Section 7.7.
.IP \s+4\f(HB7.3\fR\s-4  6n
\s+4\f(HBAllocating Shared Colors\fR\s-4
.sp
Since free colorcells can quickly become a scarce resource when 
clients store private color values, simple clients that mainly use 
color for decoration are encouraged always to allocate read-only 
colors, so that these colorcells can be shared by other clients 
that allocate the same colors read-only.  
.sp .5
The returned pixel value can be used to set the 
\f(CWbackground_pixel\fP or \f(CWborder_pixel\fP attribute of a window 
or to set the \f(CWforeground\fP or \f(CWbackground\fP member of a GC, 
which are used by drawing requests.  (See Chapters 4 and 5
for more information.)
.sp .5
Read-only colorcells can be allocated with the following routines:
.IP "\f(CWXAllocColor\fP" 22n
Returns the index of the colorcell (a pixel value) that contains 
the RGB values requested or that contains the closest RGB values 
physically possible on the screen.
.IP "\f(CWXAllocNamedColor\fP" 22n
Returns the index of the colorcell that contains the RGB values
associated with a specified color name from the string color name 
database or the closest RGB values physically possible on the screen.
.sp .5
By convention, clients allow the user to specify colors on the command 
line or in the resource database using a color name.  When the RGB 
values are chosen from the color database by specifying color name 
strings, sharing of read-only colorcells is much more likely than if 
colors are specified as raw RGB values or using hexadecimal specifications.
.sp .5
\f(CWXParseColor\fP parses a color name string or a hexadecimal color 
specification string and returns RGB values.  It can be used with 
\f(CWXAllocColor\fP or the routines that allocate read/write cells, which 
will be described later.  For color names, it gets the RGB values from 
the server's color database just like \f(CWXAllocNamedColor\fP.  You may 
have noticed that \f(CWXAllocNamedColor\fP is very similar to the 
combination of \f(CWXParseColor\fP and \f(CWXAllocColor\fP.  The 
difference is slight:  \f(CWXAllocNamedColor\fP can interpret color 
names but not hexadecimal specifications\(embut hexadecimal 
specifications are rarely made by users anyway.  The two-routine 
combination is more often used because it allows you to separately 
report errors in parsing the color specified and allocating the colorcell.
.sp .5
Using \f(CWXQueryColor\fP and \f(CWXQueryColors\fP you can find
out what RGB values are in each colorcell.  But there is no way 
to determine whether a given cell is read-only or read/write or 
how many cells are currently unallocated.
.sp .5
A request to allocate a color may fail because there are no free
colormap cells and, for read-only colorcells, because no existing 
colorcell contains the closest color possible on the hardware to 
the exact color requested.  Applications must allocate colors by 
trial and error.  The routines that allocate colorcells all have 
\f(CWStatus\fP return values.  If the call to allocate colorcells 
returns \f(CWFalse\fP, the client may modify the arguments and try 
again.  If repeated attempts fail, the client can settle with 
\f(CWBlackPixel\fP and \f(CWWhitePixel\fP or, if these colors are 
inadequate, create a new virtual colormap.  An application with 
picky color needs that cannot be satisfied can simply report to 
the user that its color needs cannot be met and exit.  
.sp .5
.IP \s+2\f(HB7.3.1\fR\s-2 6n
\s+2\f(HBThe XColor Structure\fR\s-2
.sp .5
Both \f(CWXAllocColor\fP and \f(CWXAllocNamedColor\fP (as well as 
other functions that manipulate colorcells) take as an argument an 
\f(CWXColor\fP structure.  This structure is used to specify the 
desired RGB values, as well as to return the pixel value.  
.sp .5
The \f(CWXColor\fP structure is shown in Example 1.  
The information it contains closely matches the information
in each cell of the colormap.
.sp .5
\s-1\f(CW
.nf
.ta 4.5n 3i
typedef struct {
    unsigned long pixel;    /* Pixel value */
    unsigned short red, green, blue;    /* RGB values */
    char flags;    /* DoRed,  DoGreen, and/or 
         * DoBlue */
    char pad;    /* Unused; pads structure 
         * to even word boundary */
} XColor;
\fR\s0
.fi
.sp .5
Example 1: "The XColor structure"
.sp
In \f(CWXAllocColor\fP and \f(CWXAllocNamedColor\fP, the 
\f(CWpixel\fP member returns the pixel value that will be 
used to set the foreground or background pixel value in 
the GC or window attributes.  In \f(CWXStoreColor\fP and 
\f(CWXQueryColor\fP, which you will see later, the \f(CWpixel\fP 
.ne 2
member indicates which cell in the colormap is having its color 
set (read/write cells only) or is having its RGB values queried.  
.sp .5
The \f(CWred\fP, \f(CWgreen\fP, and \f(CWblue\fP members are 16-bit 
values.  Full brightness in a color is a value of 65535, half brightness 
is 32767, and off is 0.  (The server automatically scales these values 
if the hardware colormap includes fewer bits for RGB values.)  
.sp .5
.hw DoGreen
The \f(CWflags\fP member of the \f(CWXColor\fP structure is a bitwise 
OR of the symbols \f(CWDoRed\fP, \f(CWDoGreen\fP, and \f(CWDoBlue\fP.  
These flags are used to specify which of the red, green, and blue 
values should be read while changing the RGB values in a read/write 
colorcell.
.sp .5
.IP \s+2\f(HB7.3.2\fR\s-2 6n
\s+2\f(HBCode to Allocate Read-only Colors\fR\s-2
.sp .5
As we have said, applications that have basic color needs should
allocate read-only, shareable color cells.  Example 2 shows 
code to allocate a color specified using a name from the color name 
database.  In this case, we have simply hardcoded the color name strings.  
In a real application, you would hardcode the default color but allow 
user specification of the string, as is done in \fIbasecalc\fR, 
described in Chapter 12.
.sp .5
This routine uses \f(CWXMatchVisualInfo\fP to determine whether
color is supported on the screen.  If any of the four color visual 
classes are supported, it proceeds to attempt to allocate read-only 
colors.  Whenever anything fails or if color is not supported, the 
routine uses black and white.  For some applications, this could be
modified to allocate levels of gray on GrayScale visual class screens.
.sp .5
The code for all the examples in this chapter is in the example
source in the directory \fI/basicwin/color/\fR.  This example
is called \fIbasic.ro\fR.
.sp .5
\s-1\f(CW
.nf
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <stdio.h>

extern Display *display;
extern int screen_num;
extern Screen *screen_ptr;
extern unsigned long foreground_pixel, background_pixel, 
        border_pixel;
extern char *progname;

#define MAX_COLORS 3

/* This is just so we can print the visual class intelligibly */
static char *visual_class[] = {
    "StaticGray",
    "GrayScale",
    "StaticColor",
    "PseudoColor",
    "TrueColor",
    "DirectColor"
};

get_colors()
{
    int default_depth;
    Visual *default_visual;
    static char *name[] = {"Red", "Yellow", "Green"};
    XColor exact_def;
    Colormap default_cmap;
    int ncolors = 0;
    int colors[MAX_COLORS];
    int i = 5;
    XVisualInfo visual_info;
    
    /* Try to allocate colors for PseudoColor, TrueColor, 
     * DirectColor, and StaticColor; use black and white
     * for StaticGray and GrayScale */

    default_depth = DefaultDepth(display, screen_num);
    default_visual = DefaultVisual(display, screen_num);
    default_cmap   = DefaultColormap(display, screen_num);
    if (default_depth == 1) {
        /* Must be StaticGray, use black and white */
        border_pixel = BlackPixel(display, screen_num);
        background_pixel = WhitePixel(display, screen_num);
        foreground_pixel = BlackPixel(display, screen_num);
        return(0);
    }

    while (!XMatchVisualInfo(display, screen_num, default_depth, 
            /* visual class */i--, &visual_info))
        ;
    printf("%s: found a %s class visual at default depth.\en", 
            progname, visual_class[++i]);
    
    if (i < StaticColor) { /* Color visual classes are 2 to 5 */
        /* No color visual available at default depth;
         * some applications might call XMatchVisualInfo
         * here to try for a GrayScale visual if they
         * can use gray to advantage, before giving up
         * and using black and white */
        border_pixel = BlackPixel(display, screen_num);
        background_pixel = WhitePixel(display, screen_num);
        foreground_pixel = BlackPixel(display, screen_num);
        return(0);
    }

    /* Otherwise, got a color visual at default depth */

    /* The visual we found is not necessarily the default
     * visual, and therefore it is not necessarily the one
     * we used to create our window; however, we now know
     * for sure that color is supported, so the following
     * code will work (or fail in a controlled way) */
    /* Let's check just out of curiosity: */
    if (visual_info.visual != default_visual)
    {
        printf("%s: %s class visual at default depth\en", 
                progname, visual_class[i]);
         printf("is not default visual! Continuing anyway...\en");
    }

    for (i = 0; i < MAX_COLORS; i++) {
        printf("allocating %s\en", name[i]);
        if (!XParseColor (display, default_cmap, name[i], 
                &exact_def)) {
            fprintf(stderr, "%s: color name %s not in database", 
                    progname, name[i]);
            exit(0);
        }
        printf("The RGB values from the database are %d, %d, %d\en", 
                exact_def.red, exact_def.green, exact_def.blue);
           if (!XAllocColor(display, default_cmap, &exact_def)) {
            fprintf(stderr, "%s: can't allocate color:\en", 
                    progname);
            fprintf(stderr, "All colorcells allocated and\en");
            fprintf(stderr, "no matching cell found.\en");
            exit(0);
        }
        printf("The RGB values actually allocated are %d, %d, %d\en", 
                exact_def.red, exact_def.green, 
                exact_def.blue);
        colors[i] = exact_def.pixel;
        ncolors++;
    }

    printf("%s: allocated %d read-only color cells\en", 
            progname, ncolors);

    border_pixel = colors[0];
    background_pixel = colors[1];
    foreground_pixel = colors[2];
    return(1);
}
\fR\s0
.fi
.sp .5
Example 2: "Allocating read-only colorcells"
.sp
This code begins by setting variables to the default depth, visual, 
and colormap for later use.  If the default depth is one, then the 
application is displaying on a monochrome screen, and black and 
white are returned.  Then the code calls \f(CWXMatchVisualInfo\fP 
in a loop to look for a color visual at the default depth\(emit is 
called up to four times, until a color visual is found.  If none 
is found, it again returns black and white, since this screen must 
support only a \f(CWGrayScale\fP visual (at this depth, anyway).  
Some applications may wish to allocate grays in this branch.  The 
rest of the code loops through the list of color names to be 
allocated, looks them up in the color database, and then allocates 
them.  If either the lookup stage or the allocation stage fails for 
any color, the routine prints an error and exits.  It could instead 
simply fall back on black and white again; your choice.
.sp .5
As noted in the code, the visual found might not necessarily be
the default visual.  This does not always matter, because if any 
color visual is available, it is a good bet that the default visual
is also color, and so colors can be allocated without doing any 
further research.  With \f(CWXMatchVisualInfo\fP, it is difficult 
to develop an algorithm that is guaranteed to find the default 
visual.  This is much easier with \f(CWXGetVisualInfo\fP, which 
returns a list of available visual structures that match a set of 
criteria you specify.  If you pass no criteria, it simply returns 
the entire list of available visuals.  You can then search through 
the list matching the \f(CWvisual\fP member of the \f(CWXVisualInfo\fP 
structures to the default visual.
.sp .5
The \f(CWXParseColor\fP call specifies a color name, and the RGB values
corresponding to that name are returned from the color database in the
passed \f(CWXColor\fP structure.  This structure is then passed to
\f(CWXAllocColor\fP, and the pixel value allocated is returned in the
\f(CWpixel\fP field of the structure.
.sp .5
The same calls would be used to parse a hexadecimal color string.  
Pink could be specified in the call to \f(CWXParseColor\fP as 
"\f(CW#bc8f8f\fP" instead of "\f(CWpink\fP".  But, as we have said 
before, color names are preferred, because there is a better chance 
that they will specify a color already allocated or later to be 
allocated by another client.
.sp .5
It is also possible to specify the desired RGB values explicitly.
This is good for default colors because it saves a call to 
\f(CWXParseColor\fP, but on the other hand, you might not get a
consistent color on all systems because you are bypassing the 
gamma correction implemented through the color database.  Simply 
declare an \f(CWXColor\fP structure and set its \f(CWred\fP, 
\f(CWgreen\fP, and \f(CWblue\fP members to the desired RGB values.  
Of course, these values can be specified as integers, hexadecimal 
values, or any other way that the C language allows.  Then pass 
this structure to \f(CWXAllocColor\fP.  But remember, as we have 
said, it is better to use color names when allocating read-only 
colorcells than to use any of these explicit RGB values.
.sp .5
.IP \s+2\f(HB7.3.3\fR\s-2 6n
\s+2\f(HBHighlighting in Two Colors\fR\s-2
.sp .5
It is easy to highlight graphics on a monochrome system.  The simplest 
way is to set the GC to the \f(CWGXxor\fP logical function and draw 
your graphics once to draw them and again to undraw them.  You must 
grab the server between the drawing and undrawing so that no other 
client changes the same pixels in between (by, for example, covering 
part of the area with another window).  On a monochrome system, this 
always changes white to black and black to white if you set the 
foreground in the GC to \f(CW1\fP (setting it to \f(CWBlackPixel\fP 
or \f(CWWhitePixel\fP is not guaranteed to work on all systems, 
because either may be \f(CW0\fP).
.sp .5
When drawing in \f(CWBlackPixel\fP and \f(CWWhitePixel\fP on a 
color system, the color drawn by the \f(CWGXxor\fP operation is
random if \f(CWBlackPixel\fP or \f(CWWhitePixel\fP are used for
the foreground pixel value in the GC.  This is because there is 
no restriction on which pixel value \f(CWBlackPixel\fP and 
\f(CWWhitePixel\fP can be on a server\(emthey are not necessarily 
\f(CW1\fP and \f(CW0\fP and not necessarily different by just 
one bit.  For example, the pixel value drawn if the foreground 
pixel value in the GC is \f(CWBlackPixel\fP and the pixel value 
on the screen is \f(CWWhitePixel\fP is \f(CWBlackPixel\fP XOR 
\f(CWWhitePixel\fP, which, unless \f(CWBlackPixel\fP and WhitePixel 
are different by only one bit, is a third pixel value not allocated
by this client.  The colorcell identified by this pixel value might 
contain black, in which case the operation would not change the screen.
.sp .5
The solution to this problem, which works on monochrome and color 
systems, is to set the foreground pixel value in the GC used in 
drawing with \f(CWGXxor\fP to the exclusive OR of \f(CWBlackPixel\fP 
and \f(CWWhitePixel\fP or by setting the logical function to 
\f(CWGXinvert\fP and using a plane mask which is the exclusive 
OR of \f(CWWhitePixel\fP and \f(CWBlackPixel\fP.  All applications 
that highlight graphics drawn in \f(CWBlackPixel\fP and 
\f(CWWhitePixel\fP on a color system should use one of these two 
methods.  The following example illustrates how this works using 
two arbitrarily chosen pixel values (which could be \f(CWBlackPixel\fP 
and \f(CWWhitePixel\fP or could be any two colors).
.sp .5
Let's assume that we draw in two pixel values, which we will call 
\f(CWcolor1\fP and \f(CWcolor2\fP.  The pixel values for these could be:
.sp .5
\s-1\f(CW
.nf
color1 = 11111111111111110000000000000000
color2 = 00000000111111111111111100000000
.fi
\fR\s0
.fi
.sp .5
The pixel value we will use to draw is generated by taking the 
exclusive OR of \f(CWcolor1\fP and \f(CWcolor2\fP:
.sp .5
\s-1\f(CW
color1 XOR color2 = 11111111000000001111111100000000
\fR\s0
.sp .5
Now we set the \f(CWforeground\fP in the GC to this pixel value
and the \f(CWfunction\fP in the GC to \f(CWGXxor\fP and draw.
This changes existing pixels that contained \f(CWcolor1\fP to 
\f(CWcolor2\fP and existing pixels that were \f(CWcolor2\fP to 
\f(CWcolor1\fP.
.sp .5
\s-1\f(CW
.nf
foreground =               11111111000000001111111100000000
existing pixel (color1) =  11111111111111110000000000000000
resulting pixel (color2) = 00000000111111111111111100000000
.fi
\fR\s0
.sp .5
The other way to do this is to set the \f(CWplane_mask\fP in the
GC to (\f(CWcolor1\fP\ \f(CW^\fP\ \f(CWcolor2\fP) and then use a 
logical function of \f(CWGXinvert\fP.  This is equally effective.
.sp .5
.IP \s+2\f(HB7.3.4\fR\s-2 6n
\s+2\f(HBChoosing Default Colors\fR\s-2
.sp .5
A client that uses color should allow the user to specify the 
colors either on the command line or in the resource database, 
or both.  The resource manager (described 
in Chapter 11) can be used to merge these 
preferences with the defaults of the program.  However, the 
client needs to have reasonable default colors in case the 
user does not specify any preferences.
.sp .5
Follow these guidelines for your application's default colors:*  
.sp
.FS 
*Courtesy Oliver Jones, Apollo Computer.
.FE
.RS
.IP \(bu 5
Use string color names for read-only colorcells if possible,
since this maximizes the chance of sharing cells.
.IP \(bu 5
Use colors with large contributions from two or all three 
primary colors\(emthey light the screen more brightly.
.IP \(bu 5
Avoid shades of pure blue\(emthe human eye is relatively 
insensitive to and unable to focus on images made of pure blue 
light.  Mix blue shades with white (white contains equal parts 
of all three primary colors).
.IP \(bu 5
Remember that some users are color blind.  Do not use the same 
intensity of green and red for "safe" and "danger"\(emuse colors 
with differing intensity.
.RE
.sp 2
.IP \s+4\f(HB7.4\fR\s-4  6n
\s+4\f(HBAllocating Private Colors\fR\s-4
.sp
In colormaps of the \f(CWPseudoColor\fP or \f(CWTrueColor\fP 
visual classes, a client can allocate read/write cells.  
Read/write colorcells should be allocated when: 
.sp
.RS
.IP \(bu 5
The application draws something whose color must be changed dynamically 
without redrawing it.  For example, in a color mixing program, the 
palette must be drawn in colors that change frequently.  If this 
were done with read-only colors, cells would have to be allocated 
and freed frequently and the palette area redrawn with each new 
color.  However, with read/write colorcells, the steps of 
allocation and color setting are separate, so that the color of 
an already allocated cell can be changed at will.  Anything drawn
using the pixel value of this colorcell will change color immediately
when the RGB values in the colorcell are changed.
.IP \(bu 5
The application needs to overlay graphics on top of other graphics
in such a way that the overlayed graphics can be erased without
disturbing the underlying graphics.  For example, in a Computer Aided 
Design (CAD) package for chip design, it is often useful to overlay
the various layers of a chip in different colors on the screen.  When
one of the layers is removed, you want to avoid having to redraw all
the underlying layers.  How to do this by allocating read/write cells
will be described.
.IP \(bu 5
The system has a huge colormap, and the application needs to set a
large number of colorcells.  The calls for manipulating read/write
colorcells allow you to manipulate multiple cells per call, whereas
with read-only cells, you are limited to one cell per call.
.RE
.sp 2
Note that read/write colorcell allocation never works on \f(CWTrueColor\fP 
or \f(CWStaticColor\fP visuals.  Therefore, on systems that only support 
these visuals, an application that uses read/write colorcells cannot work.  
Read/write colorcells should only be used when really needed.
.sp .5
\f(CWXAllocColorCells\fP allocates read/write colorcells.  At its 
simplest, it allows you to allocate read/write cells so you can 
change the RGB values dynamically.
.sp .5
But to simply allocate just a few cells, you set the \f(CIncolors\fP 
argument to the number of colorcells desired and \f(CInplanes\fP to 
\f(CW0\fP, and all the pixel values you need will be returned in the 
\f(CIpixels\fP array.  The real reason for the \f(CInplanes\fP and 
\f(CIplane_masks\fP arguments will become clear in Section 7.5.2.
The RGB values of the allocated cells are set with \f(CWXStoreColor\fP, 
\f(CWXStoreColors\fP, or \f(CWXStoreNamedColor\fP.
.sp .5
\f(CWXAllocColorPlanes\fP, on the other hand, is only used when you 
want to be able to vary a primary color component of graphics already 
drawn without redrawing them.  It allocates read/write cells, so that 
a preset number of bits are reserved for each primary color.  Primarily 
for \f(CWDirectColor\fP, it also allows you to simulate a small 
\f(CWDirectColor\fP colormap on a \f(CWPseudoColor\fP visual but uses 
up colorcells quickly.  It treats the colormap as three separate lookup 
tables, allocating \f(CIncolors\fR\ \f(CW*\fP 
.Su + \f(CW2\fR \f(CInreds\fR 
entries in the red lookup table, \f(CIncolors\fR\ \f(CW*\fP 
.Su + \f(CW2\fR \f(CIngreens\fR 
entries in the green lookup table, and \f(CIncolors\fR\ \f(CW*\fP 
.Su + \f(CW2\fR \f(CInblues\fR 
entries in the blue lookup table.
.Nd 4
.sp .5
The following routines are used to actually store colors into read/write 
colorcells once they are allocated:
.IP "\f(CWXStoreColor\fP" 22n
Changes the read/write colormap cell corresponding to the specified 
pixel value to the hardware color that most closely matches the RGB 
values specified.*  
.FS
*Even when storing explicit RGB values, you may not get the precise 
color you specify.  For example, if the hardware colormap supports 
only four bits of intensity in each primary and you specify 
eight-bit values, the server will scale the values you provide to 
the closest possible equivalent on the hardware.
.FE
The flags \f(CWDoRed\fP, \f(CWDoGreen\fP, and \f(CWDoBlue\fP in the 
\f(CWXColor\fP structure indicate which primary colors in the cell 
are to be changed.
.IP "\f(CWXStoreColors\fP" 22n
Like \f(CWXStoreColor\fP, except it does multiple cells per call.
Changes the read/write colormap cell corresponding to the specified 
pixel value to the hardware color that most closely matches the RGB 
values specified.  The flags \f(CWDoRed\fP, \f(CWDoGreen\fP, and 
\f(CWDoBlue\fP in each \f(CWXColor\fP structure indicate which primary 
colors in each cell are to be changed.
.IP "\f(CWXStoreNamedColor\fP" 22n
Performs the same function as \f(CWStoreColor\fP, except that it stores 
the RGB values associated with a string color name in the RGB database.  
This call would be useful for loading a private colormap with each of 
the default named colors.
.sp .5
.IP \s+2\f(HB7.4.1\fR\s-2 6n
\s+2\f(HBAllocating Read/Write Colorcells for Dynamic Colors\fR\s-2
.sp .5
As described above, the simplest use of read/write colors is to 
allocate colorcells whose colors can by changed at any time.  
Example 3 is analogous to the code just shown to 
allocate read-only colors, except that it allocates read/write 
colors instead.  Note that it calls \f(CWXAllocColorCells\fP with 
the \f(CIncolors\fP argument set to the number of colorcells 
desired and \f(CInplanes\fP set to zero.
.sp .5
\s-1\f(CW
.nf
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <stdio.h>

extern Display *display;
extern int screen_num;
extern unsigned long foreground_pixel, background_pixel, 
        border_pixel;

#define MAX_COLORS 3

get_colors()
{
    int default_depth;
    Visual *default_visual;
    static char *name[] = {"Red", "Yellow", "Green"};
    XColor exact_defs[MAX_COLORS];
    Colormap default_cmap;
    int ncolors = MAX_COLORS;
    int plane_masks[1];
    int colors[MAX_COLORS];
    int i;
    XVisualInfo visual_info;
    int class;

    class = PseudoColor;
    default_depth = DefaultDepth(display, screen_num);
    default_visual = DefaultVisual(display, screen_num);
    default_cmap   = DefaultColormap(display, screen_num);
    if (default_depth == 1) {
        /* Must be StaticGray, use black and white */
        border_pixel = BlackPixel(display, screen_num);
        background_pixel = WhitePixel(display, screen_num);
        foreground_pixel = BlackPixel(display, screen_num);
        return(0);
    }

    if (!XMatchVisualInfo(display, screen_num, default_depth, 
            PseudoColor, &visual_info)) {
        if (!XMatchVisualInfo(display, screen_num, default_depth, 
                DirectColor, &visual_info)) {
            /* No PseudoColor visual available at default_depth;
              * some applications might try for a GrayScale 
              * visual here if they can use gray to advantage, 
              * before giving up and using black and white */
            border_pixel = BlackPixel(display, screen_num);
            background_pixel = WhitePixel(display, screen_num);
            foreground_pixel = BlackPixel(display, screen_num);
            return(0);
        }
    }

    /* Got PseudoColor or DirectColor visual at default_depth */

    /* The visual we found is not necessarily the default
     * visual, and therefore it is not necessarily the one
     * we used to create our window; however, we now know
     * for sure that color is supported, so the following 
     * code will work (or fail in a controlled way) */

    /* Allocate as many cells as we can */
    ncolors = MAX_COLORS;
    while (1) {
           if (XAllocColorCells (display, default_cmap, False, 
                plane_masks, /* nplanes */0, colors, ncolors))
            break;
    ncolors--;
    if (ncolors = 0)
        fprintf(stderr, "basic: couldn't allocate read/write \
                colors\en");
        exit(0);
    }

    printf("basic: allocated %d read/write color cells\en", ncolors);

    for (i = 0; i < ncolors; i++) {
        if (!XParseColor (display, default_cmap, name[i], 
                &exact_defs[i])) {
            fprintf(stderr, "basic: color name %s not in database", 
                    name[i]);
            exit(0);
        }

        /* Set pixel value in struct to the allocated one */
        exact_defs[i].pixel = colors[i];
             exact_defs[i].flags = DoRed | DoGreen | DoBlue;
    }

    /* This sets the color of read/write cell */
    XStoreColors (display, default_cmap, exact_defs, ncolors);
    border_pixel = colors[0];
    background_pixel = colors[1];
    foreground_pixel = colors[2];
}
\fR\s0
.fi
.sp .5
Example 3: "Allocating read/write colorcells for dynamic colors"
.sp
The \f(CWmain\fP that calls this \f(CWget_colors\fP function, shown 
in Example 4 contains an \f(CWXQueryColor\fP call that 
gets the current RGB values in the colorcell (necessary because 
\f(CWmain\fP and \f(CWget_colors\fP are in separate source files and 
the RGB values used in \f(CWget_colors\fP are not global variables) 
and an \f(CWXStoreColor\fP call that changes the color of what is 
drawn in the foreground pixel value every time you press a button 
in the window.  In the example source, this application is in 
the directory \fIbasicwin/color/\fR and is called \fIbasic.rw\fR.
.sp .5
\s-1\f(CW
.nf
  .
  .
  .

void main(argc, argv)
int argc;
char **argv;
{
      .
      .
      .

    XColor color;
    unsigned short red, green, blue;
      .
      .
      .
    /* Open display, etc. */

    color.pixel = foreground_pixel;
    XQueryColor(display, DefaultColormap(display, screen_num), 
            &color);
    printf("red is %d, green is %d, blue is %d\en", color.red, 
            color.green, color.blue);

    while (1)  {
        XNextEvent(display, &report);
        switch  (report.type) {
          .
          .
          .
        case ButtonPress:
            color.red += 5000;
            color.green -= 5000;
            color.blue += 3000;
            printf("red is %d, green is %d, blue is %d\en", 
                    color.red, 
            color.green, color.blue);
            XStoreColor(display, DefaultColormap(display, 
                    screen_num), &color);
            break;
          .
          .
          .
    }
}
\fR\s0
.fi
.sp .5
Example 4: "Main of basic.rw \(em changing colors of dynamic colorcells"
.sp
.IP \s+2\f(HB7.4.2\fR\s-2 6n
\s+2\f(HBAllocating Read/Write Colorcells for Overlays\fR\s-2
.sp .5
\f(CWXAllocColorCells\fP has another use:  it allows you to 
nondestructively overlay one set of graphics over another.  
The underlying graphics will not be visible where the overlay 
is drawn, but they can be refreshed by simply setting or clearing
one or more complete planes in the drawable.  This technique can 
improve the performance of a client by reducing the amount of 
complicated graphics that have to be redrawn.  It can be useful 
for highlighting graphics for selection.  However, as noted earlier, 
read/write colorcells can only be allocated in \f(CWPseudoColor\fP 
and \f(CWDirectColor\fP visuals, so any application that attempts 
to use this technique should also provide a fallback technique for 
use on other visuals or in case of failure.
.sp .5
The trick that allows drawing without destroying what is already drawn 
relies on the fact that we can draw in one plane of the drawable, 
changing the pixel values and therefore the color, without changing 
any other plane.  This is possible using the \f(CWplane_mask\fP 
component of the GC.  It is these other planes that contain the 
information about the drawing that was already there.  The 
disadvantage of this approach is that we have to allocate more
colorcells than we would normally need.  Some of the colorcells 
will need to be loaded with duplicate RGB values.  Because of this 
waste of colorcells, this technique should be used only when the 
graphics being preserved are slow for the client or the server to redraw.
.Nd 20
.sp .5
To illustrate this trick, we are going to draw in one color (the
foreground in the GC), set the \f(CWbackground_pixel\fP attribute 
of the window to a second color, and then draw something temporary 
over the top with a third color.*  
.FS
*Note that the background of a window is redrawn by the server when
\f(CWExpose\fP events occur, but this does not effect the process of drawing
and removing overlays, because no \f(CWExpose\fP event will be triggered in
this process.  In other words, even though the background color is
set as a window attribute and drawn by the server, the response to
other graphics drawn on top is the same as if the background were 
drawn by the application.  The background counts as a color that
must be preserved.
.FE
To do this, we need to allocate four colorcells with 
\f(CWXAllocColorCells\fP.  The pixel values allocated
will look something like this:
.sp .5
.sp .5
\s-1\f(CW 3n
.nf
\fRColor\v'1p'\h'-\w'Color'u'\l'\w'Color'u\(ul'\v'-1p'                            Important Bits\v'1p'\h'-\w'Important Bits'u'\l'\w'Important Bits'u\(ul'\v'-1p'                                                               Remaining bits\v'1p'\h'-\w'Remaining bits'u'\l'\w'Remaining bits'u\(ul'\v'-1p'\f(CW

foreground:    ----0--0------------------------  \fIall other bits don't matter\fP

background:    ----0--1------------------------  \fIall other bits don't matter\fP

highlight1:    ----1--0------------------------  \fIall other bits don't matter\fP

highlight2:    ----1--1------------------------  \fIall other bits don't matter\fP
\fR\s0
.fi
.sp .5
.sp .5
The bits indicated could have been any bits, but it is significant
that only two bits distinguish the four pixel values.  The first 
pixel value is used for the foreground, and the second for the 
background.  We draw overlays in the third or fourth pixel value.
Since we do not want to erase what was drawn in the foreground and
background pixel values, we use a plane mask to restrict the drawing
of the highlighting pixel value to a single plane, the one where 
bits in the highlighting pixel values are set to 1.  When this 
entire plane (indicated by the 1 in pixel values \f(CWhighlight1\fP 
and \f(CWhighlight2\fP) is cleared, anything drawn in \f(CWhighlight1\fP 
or \f(CWhighlight2\fP disappears, and anything that was drawn in the 
foreground or background will reappear.  The color in the colorcell 
indicated by \f(CWhighlight2\fP must be the same as the color of 
colorcell \f(CWhighlight1\fP so that the same highlighting color 
appears regardless of the bit already in the drawable that 
distinguishes the foreground and background pixel values.
.sp .5
\f(CWXAllocColorCells\fP does not return these four pixel values 
directly.  Instead it returns the arrays \f(CIcolors\fP and
\f(CIplane_masks\fP that are more convenient for actually using 
the overlays than a single array of pixel values.  (Each of these 
arrays has the number of members that was specified in the 
\f(CIncolors\fP and \f(CInplanes\fP arguments.)  Both arrays 
consist of unsigned long values like pixel values.  One array 
contains the plane masks of the overlay planes, and the other 
contains the pixel values that can be used for drawing 
independent of the overlay planes.  Here are the values returned 
in each array after we call \f(CWXAllocColorCells\fP with 
\f(CIncolors\f(CW\ =\ 2\fR and \f(CInplanes\f(CW\ =\ 1\fR.
These values are then used to generate the pixel values shown above.
.sp .5
.sp .5
\s-1\f(CW 3n
.nf
\fRArray Members\v'1p'\h'-\w'Array Members'u'\l'\w'Array Members'u\(ul'\v'-1p'                   Important Bits\v'1p'\h'-\w'Important Bits'u'\l'\w'Important Bits'u\(ul'\v'-1p'                                                               Remaining Bits\v'1p'\h'-\w'Remaining Bits'u'\l'\w'Remaining Bits'u\(ul'\v'-1p'\f(CW

colors[0]      =  ----0--0------------------------  \fIother bits don't matter\fP
colors[1]      =  ----0--1------------------------   

plane_masks[0] =  ----1---------------------------  \fIall other bits 0\fR
\fR\s0
.fi
.sp .5
.sp .5
The two members of the \f(CIcolors\fP array are used for the 
foreground and background.  Pixel values \f(CWhighlight1\fP and 
\f(CWhighlight2\fP are composed by combining with a bitwise OR 
each item in the \f(CIcolors\fP array with each item in the 
\f(CIplane_masks\fP array.  In this case, \f(CWhighlight1\fP
is (\f(CIcolors\f(CW[0]\ |\ \f(CIplane_masks\f(CW[0]\fR).  The 
\f(CWplane_mask\fP in the GC used when highlighting should be set 
to the OR of the members of \f(CIplane_masks\fP used to make the 
highlighting pixel value.  In this simplest case, highlighting 
should be done with the \f(CWplane_mask\fP in the GC set to 
\f(CIplane_masks\f(CW[0]\fR.
.sp .5
Note that \f(CWhighlight2\fP, generated with 
(\f(CIcolors\f(CW[1]\ |\ \f(CIplane_masks\f(CW[0]\fR), can be 
useful.  As mentioned earlier, \f(CWhighlight2\fP can be used 
.hw highlight
interchangeably with \f(CWhighlight1\fP, as long as the 
\f(CWplane_mask\fP in the GC is set to \f(CIplane_masks\f(CW[0]\fR.  
But \f(CWhighlight2\fP has another use.  With a GC that does not have 
its \f(CWplane_mask\fP set to \f(CIplane_masks\f(CW[0]\fR (the GC used 
for drawing with the foreground or background), this fourth pixel value 
can be used for drawing in the highlighting color while wiping out 
the underlying graphics, so that when the highlight is removed, the 
background color appears regardless of the contents of the drawable 
before the highlighting.
.sp .5
We have been hinting at the fact that this overlay technique
can be used with more than two colors and more than one plane.  
\f(CIncolors\fP specifies the number of colors than can be drawn
and preserved while drawing in the overlays.  \f(CInplanes\fP 
specifies how many separate one-color overlays you may have or how 
many bits of color are available in a single overlay.  The pixel 
values in the \f(CIcolors\fP array are the ones that will be 
preserved through overlays.  By ORing together each \f(CIcolors\fP 
with any combination of \f(CIplane_masks\fP, you get the pixel 
values that are used for drawing the overlays.  Note, however, that 
the plane mask of the GC used for the overlaying must be the OR of 
the same combination of members of the \f(CIplane_masks\fP array 
as were used to generate the pixel value.
.sp .5
The total number of pixel values (colorcells) allocated by 
\f(CWXAllocColorCells\fP is \f(CIncolors\fR\ \f(CW*\fR
.Su + \f(CW2\fR \f(CInplanes\fR 
.\".Ns R
.\"Remove this note if 2 ^ nplanes (superscript) appears above it.
.\".Ne
\&.  Note that the more planes you try to allocate, the less likely this
request is to succeed, particularly on \f(CWPseudoColor\fP visuals.
Therefore, if you are trying for multiple overlays or one multicolor
overlay, this will probably work reliably only on \f(CWDirectColor\fP 
visuals, so make sure you have a backup plan for more common systems.
In most cases, the underlying graphics can be redrawn if the overlays 
that would preserve them cannot be allocated.  It is also possible to 
use backing store (which can save selective planes) or to manage your 
own off-screen pixmaps for use in fast redrawing of complicated graphics.
.sp .5
\f(CWXAllocColorCells\fP takes a \f(CIcontig\fP argument that specifies 
whether the planes returned in \f(CIplane_masks\fP must be contiguous.  
The \f(CIcontig\fP argument is normally set to \f(CWFalse\fP, specifying 
that the allocated planes need not be contiguous, because then the chances 
of success of the \f(CWXAllocColorCells\fP call are greater.  There are 
more likely to be a number of noncontiguous planes available than the same
number of contiguous planes.  The \f(CIcontig\fP argument may have to be
set to \f(CWTrue\fP for imaging applications that want to be able to perform
mathematical operations on the pixel values.  It is easier to perform 
operations by shifting bits with contiguous planes than to achieve the 
same effect with random planes. 
.sp .5
Each plane mask has one bit for \f(CWGrayScale\fP and \f(CWPseudoColor\fP 
or three bits for \f(CWDirectColor\fP or \f(CWTrueColor\fP, and none of 
the masks have bits in common.
.sp .5
Example 5 demonstrates allocating the read/write cells for 
a single overlay plane.  It implements the overlay scheme described
above.  If this overlay plan fails, it allocates three colors so 
that a highlight can still be implemented even though the underlying
graphics will have to be redrawn.  If the color allocation fails 
completely, it uses black and white, which can be highlighted
using the \f(CWGXxor\fP logical function to invert the color,
as described in Section\ 7.3.3.
.sp .5
\s-1\f(CW
.nf
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <stdio.h>

extern Display *display;
extern int screen_num;
extern unsigned long foreground, background_pixel, overlay_pixel_1, 
        overlay_pixel_2;
extern unsigned long overlay_plane_mask;

#define MAX_COLORS 2
#define MAX_PLANES 1
#define MAX_CELLS 4       /* MAX_COLORS * 2 ^ MAX_PLANES */
#define CANNOT_OVERLAY 0
#define CAN_OVERLAY 1

int
get_colors()
{
    int default_depth;
    static char *name[] = {"Red", "Yellow", "Green", "Green"};
    XColor exact_defs[MAX_CELLS];
    Colormap default_cmap;
    int ncolors = 4;
    int plane_masks[MAX_PLANES];
    int colors[MAX_COLORS];
    int i;
    XVisualInfo visual_info;
    int class;

    default_depth = DefaultDepth(display, screen_num);
    default_cmap   = DefaultColormap(display, screen_num);
    if (default_depth == 1) {
        /* Must be StaticGray, use black and white */
        background_pixel = WhitePixel(display, screen_num);
        foreground = BlackPixel(display, screen_num);
        printf("using black and white\en");
        return(CANNOT_OVERLAY);
    }

    if (!XMatchVisualInfo(display, screen_num, default_depth, 
            PseudoColor, &visual_info)) {
        if (!XMatchVisualInfo(display, screen_num, default_depth, 
                DirectColor, &visual_info)) {
            /* No PseudoColor or TrueColor visual available 
             * at default_depth; some applications might try 
             * for a GrayScale visual here if they can use 
             * gray to advantage, before giving up and using 
             * black and white */
            background_pixel = WhitePixel(display, screen_num);
            foreground = BlackPixel(display, screen_num);
            printf("using black and white\en");
            return(CANNOT_OVERLAY);
        }
    }

    /* Got PseudoColor or TrueColor visual at default depth */

    /* The visual we found is not necessarily the default 
     * visual, and therefore it is not necessarily the one
     * we used to create our window; however, we now know
     * for sure that color is supported, so the following
     * code will work (or fail in a controlled way) */

       if (XAllocColorCells (display, default_cmap, False, plane_masks, 
            1, colors, 2) == 0) {
        /* Can't get enough read/write cells to overlay;
         * try at least to get three colors */
           if (XAllocColorCells (display, default_cmap, False, 
                plane_masks, 0, colors, 3) == 0) {
            /* Can't even get that; give up and use
             * black and white */
                       background_pixel = WhitePixel(display, 
                            screen_num);
                       foreground = BlackPixel(display, screen_num);
            printf("using black and white\en");
            return(CANNOT_OVERLAY);
        }
        else
            ncolors = 3;
    }
          
    /* Allocated three or four colorcells succesfully,
     * now set their colors -- three and four are set
     * to the same RGB values */
    for (i = 0; i < ncolors; i++)
    {
        if (!XParseColor (display, default_cmap, name[i], 
                &exact_defs[i])) {
            fprintf(stderr, "basic: color name %s not in database", 
                    name[i]);
            exit(0);
        }
        /* This needed before calling XStoreColors */
        exact_defs[i].flags = DoRed | DoGreen | DoBlue;
    }
    printf("got RGB values\en");

    /* Set pixel value in struct to the allocated ones */
    exact_defs[0].pixel = colors[0];
    exact_defs[1].pixel = colors[1];
    exact_defs[2].pixel = colors[0] | plane_masks[0];
    exact_defs[3].pixel = colors[1] | plane_masks[0];

    /* This sets the color of the read/write cells */
    XStoreColors (display, default_cmap, exact_defs, ncolors);
    printf("stored colors\en");

    background_pixel = exact_defs[0].pixel;
    foreground = exact_defs[1].pixel;
    if (ncolors == 4) {
        overlay_pixel_1 = exact_defs[2].pixel;
        overlay_pixel_2 = exact_defs[3].pixel;
        overlay_plane_mask = plane_masks[0];
        printf("set can\en");
        return(CAN_OVERLAY);
    }
    else {
        /* This must be used as a normal color, not overlay */
        overlay_pixel_1 = exact_defs[2].pixel;
        printf("set can't\en");
        return(CANNOT_OVERLAY);
    }
}
\fR\s0
.fi
.sp .5
Example 5: "Using XAllocColorCells to allocate read/write colorcells for overlay plane"
.sp
.IP \s+2\f(HB7.4.3\fR\s-2 6n
\s+2\f(HBUsing XAllocColorPlanes\fR\s-2
.sp .5
\f(CWXAllocColorPlanes\fP also allocates read/write colorcells 
but in a different way than \f(CWXAllocColorCells\fP.
\f(CWXAllocColorPlanes\fP is used when you want to be able to 
change the amount of a primary color in graphics without having 
to redraw them.  In other words, perhaps you are looking at an 
image and would like to increase the redness of it.  The best 
way to do this is to increase the amount of red in every pixel 
value.  \f(CWXAllocColorPlanes\fP would be the way to allocate 
colors to allow this.  It is rarely used except in imaging 
applications and 3-D graphics and will rarely work except on 
24-plane workstations with a \f(CWDirectColor\fP visual.
.sp .5
Note that for applications like a paint mixing program, in which 
you have three bars for the three primary colors and a palette that 
shows the mixed color, you would not use \f(CWXAllocColorPlanes\fP.  
The correct way to implement this is to allocate a single read/write 
color for the palette and to change it dynamically.  (If the primary 
colors are displayed, they should be allocated using read-only colors.)
.sp .5
The piece of code shown in Example 6 is similar to 
Example 5 but it uses \f(CWXAllocColorPlanes\fP.  
It is somewhat sketchy, because real applications that use 
\f(CWXAllocColorPlanes\fP are complicated.
.sp .5
After allocating colors with \f(CWXAllocColorPlanes\fP, you can 
then use \f(CWXStoreColors\fP to set the colors.  When \f(CInred\fP, 
\f(CIngreen\fP, and \f(CInblue\fP are each \f(CW8\fP, only one call 
to \f(CWXAllocColorPlanes\fP and one call to \f(CWXStoreColors\fP 
are necessary to allocate and set all 16\ million colors of an 
entire 24-plane colormap.
.Nd 10
.sp .5
\s-1\f(CW
.nf
#define PIXELS 256

Display *display;
int screen_num;
int contig = False;            /* Noncontiguous planes */
unsigned long pixels[PIXELS];    /* Return of pixel values */

/* Number of independent pixel values allocate */
unsigned int ncolors = PIXELS;    

/* Need PIXELS * 2 ^ maxplanes defs, where maxplanes
 * is the largest of nred, ngreen, and nblue */
XColor defs[2048];    

/* Number of planes to allocate for each primary */
unsigned int nreds = 3, ngreens = 3, nblues = 2;    

/* Returned masks, which bits of pixel value for each primary */
unsigned long \f(CWred_mask\fP, \f(CWgreen_mask\fP, \f(CWblue_mask\fP;    

Colormap colormap;
Status status;

/* Open display, etc. */
/* Get or create large DirectColor colormap */

while (status = \f(CWXAllocColorPlanes\fP(display, colormap, 
        contig, pixels, ncolors, nreds, ngreens, nblues, 
        &red_mask, &green_mask, &blue_mask) == 0) {
    /* Make contig False if it was True; reduce value of
     * ncolors; reduce value of nreds, ngreens, and/or 
     * nblues; or try allocating new map; break when 
     * you give up */
    break;
}
if (status == 0) {
    fprintf(stderr, "%s: couldn't allocate requested colorcells", 
            argv[0]);
    exit(-1);
}

/* Define desired colors in defs */

while (status = XStoreColors(display, colormap, defs, 
        ncolors) == 0) {
    fprintf(stderr, "%s: can't store colors", argv[0]);
    /* Try to fix problem here, exit or break */
    exit(-1);
}
    
/* Draw your shaded stuff! */
\fR\s0
.fi
.sp .5
Example 6: "Using XAllocColorPlanes to allocate colorcells for DirectColor"
.sp
.IP \s+4\f(HB7.5\fR\s-4  6n
\s+4\f(HBGetting Complete Visual Information\fR\s-4
.sp
As mentioned earlier, some systems define more than one visual.
The default visual might not be the most appropriate for your 
application.  Moreover, the visual found using the technique 
described in Section 7.4.2 using \f(CWXMatchVisualInfo\fP 
is fine for applications with routine color needs but is not 
necessarily the best.  As you may recall, \f(CWXMatchVisualInfo\fP 
returns a single visual arbitrarily selected from the list that 
matches the passed visual class and depth.  The most thorough 
method is to get a complete list of visual information for every 
available visual using \f(CWXGetVisualInfo\fP and then choose from these.
.sp .5
\f(CWXGetVisualInfo\fP returns a list of visual structures that match 
the attributes specified by template and mask arguments.  The template
is an \f(CWXVisualInfo\fP structure with members set to the required values, 
and the mask indicates which members are matched with the list of available 
.ne 2
visuals.  By passing an empty template structure, you can get a complete 
list of \f(CWXVisualInfo\fP structures.
.sp .5
.IP \s+2\f(HB7.5.1\fR\s-2 6n
\s+2\f(HBThe XVisualInfo Structure\fR\s-2
.sp .5
The \f(CWXVisualInfo\fP structure returns information about the 
available visuals.  It is used both to select a visual type from those 
available and as a source of information while using a particular visual.
.sp .5
The \f(CWXVisualInfo\fP structure is shown in Example 7.
.sp .5
\s-1\f(CW
.nf
typedef struct {
    Visual *visual;
    VisualID visualid;
    int screen_num;
    unsigned int depth;
    int class;
    unsigned long red_mask;
    unsigned long green_mask;
    unsigned long blue_mask;
    int colormap_size;      /* Same as map_entries member of Visual */
    int bits_per_rgb;
} XVisualInfo;
\fR\s0
.fi
.sp .5
Example 7: "The XVisualInfo structure"
.sp
The \f(CWvisual\fP member is a pointer to the internal \f(CWVisual\fP
structure.  This pointer is used as the \f(CIvisual\fP argument of 
\f(CWXCreateWindow\fP and \f(CWXCreateColormap\fP. 
.sp .5
The \f(CWvisualid\fP member is not normally needed by applications.
.sp .5
As discussed earlier, the \f(CWclass\fP member specifies whether the 
screen is to be considered color or monochrome and changeable or 
immutable.  The \f(CWclass\fP member can be one of the constants
\f(CWDirectColor\fP, \f(CWGrayScale\fP, \f(CWPseudoColor\fP, 
\f(CWStaticColor\fP, \f(CWStaticGray\fP, or \f(CWTrueColor\fP. 
.sp .5
The \f(CWred_mask\fP, \f(CWgreen_mask\fP, and \f(CWblue_mask\fP
members are used only for the \f(CWDirectColor\fP and 
\f(CWTrueColor\fP visual classes, where there is a separate map 
.hw primary
for each primary color.  They define which bits of the pixel 
value index into the colormap for each primary color.  Each 
mask has one contiguous set of bits, with no bits in common with 
the other masks.  These values are zero for monochrome and most 
four- to eight-plane color systems.  
.sp .5
The \f(CWcolormap_size\fP member of the structure tells you how
many different pixel values are valid with this visual.  For a 
monochrome screen, this value is two.  For the default visual 
of an eight-plane color system, this value is typically 254 or 
256 (two colors are often reserved for the cursor).  For 
\f(CWDirectColor\fP and \f(CWTrueColor\fP, \f(CWcolormap_size\fP 
will be the number of cells for the biggest individual pixel 
subfield.  The \f(CWcolormap_size\fP member is the same as the
\f(CWmap_entries\fP member of the visual structure.
.sp .5
The \f(CWbits_per_rgb\fP member specifies how many bits in each of
the red, green, and blue values in a colorcell are used to drive the 
RGB gun in the screen.  For a monochrome screen, this value is one.
For the default visual of an eight-plane color system, this value is 
typically eight.  The pixel subfields (the red, green, and blue values 
in each colorcell) are 16-bit unsigned short values, but only the
highest \f(CWbits_per_rgb\fP bits are used to drive the RGB gun in 
the screen.  This number corresponds the number of bits of resolution 
in the Digital to Analog Converter (DAC) in the screen hardware.
.sp .5
.IP \s+2\f(HB7.5.2\fR\s-2 6n
\s+2\f(HBExample of Choosing a Visual\fR\s-2
.sp .5
Example 8 shows a routine that uses \f(CWXGetVisualInfo\fP 
to get all the visuals of depth 8 on the current screen, as defined 
by the X server, and then creates a colormap and window.
.sp .5
\s-1\f(CW
.nf
#include <X11/Xlib.h>
#include <X11/Xutil.h>

visual()
{
Display *display;
Colormap colormap;
Window window;
XSetWindowAttributes attributes;
unsigned long valuemask;
int screen_num;
    .
    .
    .
XVisualInfo vTemplate;    /* Template of the visual we want */
XVisualInfo *visualList;  /* List of XVisualInfo structs that 
                           * match */
int visualsMatched;       /* Number of visuals that match */
    .
    .
    .
/* Set up the XVisualInfo template so that it returns a list
 * of all the visuals of depth 8 defined on the current screen
 * by the X server */
vTemplate.screen = screen_num;
vTemplate.depth = 8; 
visualList = XGetVisualInfo (display, VisualScreenMask | 
        VisualDepthMask, &vTemplate, &visualsMatched);
if ( visualsMatched == 0 )
    fatalError ("No matching visuals\en");

/* Create a colormap for a window using the first of the 
 * visuals in the list ov XVisualInfo structs returned by 
 * XGetVisualInfo */
colormap = XCreateColormap (display, RootWindow(display, screen_num), 
    visualList[0].visual, AllocNone);

/* Must specify colormap attribute if using nondefault visual */
attributes.colormap = colormap;
valuemask |= CWColormap;
    .
    .
    .
window = XCreateWindow (display, RootWindow(display, screen_num),
    x, y, width, height, border_width, vTemplate.depth,
    InputOutput, visualList[0].visual, valuemask, &attributes);
XSetWindowColormap(display, window, colormap);

/* All done with visual information; free it */

XFree(visualList);
    .
    .
    .
} /* End routine */
\fR\s0
.fi
.sp .5
Example 8: "Code to match visuals"
.sp
Notice that the list of \f(CWXVisualInfo\fP structures is freed 
with \f(CWXFree\fP after use.
.IP \s+4\f(HB7.6\fR\s-4  6n
\s+4\f(HBThe GrayScale Visual\fR\s-4
.sp
On a gray-scale workstation or a \f(CWGrayScale\fP visual on a 
color workstation, a color application should still work correctly.  
The only problem might be that when colors are allocated, the closest 
physically possible colors (returned by \f(CWXAllocColor\fP) will
result in shades of gray that provide insufficient contrast.  The 
best way to avoid this is to explicitly check for the \f(CWStaticGray\fP
visual.  For true bulletproof operation, it is a good idea to check 
any user-specified colors to make sure they contrast.
.sp .5
The color names "gray0" through "gray100", spelled with an "e" or 
an "a", can be used with \f(CWXParseColor\fP to get RGB values for 
various grays.
.sp .5
You must set the red, green, and blue values to be equal.  Some servers 
only use one of the values, and others combine all three according to 
the NTSC standard that makes color television signals work on 
black-and-white televisions:
.sp .5
\s-1\f(CW
intensity = (.30 * red) + (.59 * green) + (.11 * blue)
\fR\s0
.fi
.sp .5
.Nd 3
MIT's implementations use a least-squares algorithm that determines
the closest RGB values in the (gray) colormap to the RGB values specified.  
Exactly what algorithm is used is up to the server implementor.
.IP \s+4\f(HB7.7\fR\s-4  6n
\s+4\f(HBCreating and Installing Colormaps\fR\s-4
.sp
.\" remove have
In discussing colormaps earlier in this chapter, we mentioned 
that there are hardware colormaps and virtual colormaps, but we 
did not discuss the ramifications of this fact.
.sp .5
A hardware colormap is a physical register from which the screen 
hardware reads the RGB intensity values that generate the colors 
on the screen.  Most workstations have only one hardware colormap, 
in which case all windows on the screen are interpreted using the 
same colormap.  Some high performance workstations have multiple 
hardware colormaps, in which case separate windows may have their 
own independent colormaps.
.sp .5
.\"  ATTENTION NEEDED HERE
If the hardware colormap cannot be changed, it is termed \fIimmutable\fR.  
Monochrome systems normally have an immutable colormap, since it does 
little good to swap the two entries or make them both black or white.  
Some low-cost color systems and X terminals have immutable hardware 
colormaps.  The \f(CWStaticColor\fP, \f(CWStaticGray\fP, and 
\f(CWTrueColor\fP visuals are the only visuals that can possibly 
work on systems that have immutable hardware colormaps.  In immutable 
colormaps, no client can allocate private colorcells and all RGB 
values are preset.  On these systems, \f(CWXCreateColormap\fP succeeds, 
but it just gives you another copy of the default colormap (or one of 
the default colormaps if there are multiple immutable colormaps).  The 
application should check for this when creating colormaps.
.sp .5
On most color workstations, you can write new values into the hardware 
colormap or colormaps to change that mapping.  These hardware colormaps 
are termed \fIchangeable\fR.  The \f(CWDirectColor\fP, \f(CWGrayScale\fP, 
and \f(CWPseudoColor\fP visuals are available only on systems that have 
changeable colormaps.
.sp .5
X manages multiple colormaps by keeping \fIvirtual colormaps\fR in 
memory and installing them as instructed by the window manager.
\fIInstalling\fR a colormap is the process of moving a virtual 
colormap into the hardware colormap.  Only installed colormaps
are used to determine the colors appearing on the screen.  When 
there is only one hardware colormap and a new virtual colormap 
is installed, the virtual colormap that was previously installed 
becomes \fIuninstalled\fR.
.sp .5
Up to this point in this chapter, we have been allocating colors
out of the default colormap, which is created and installed when 
the server starts up.  On the most common color workstations, with 
four to eight planes, it is quite easy for clients that require 
precise colors to allocate all the available colorcells.  Virtual 
colormaps are a response to this problem.  When a client cannot get 
the colorcells it needs from the installed colormap, it can create 
a new virtual one.  The window manager will then install this virtual 
colormap when this application is in use.
.sp .5
When a virtual colormap is installed and there is only one hardware 
colormap, all the clients that used the old colormap will be displayed 
in false colors, since the pixel values in their windows will be 
interpreted according to the new colormap.
.sp .5
When an application creates a virtual colormap, it must set the
colormap window attribute of its top-level window so that the window 
manager can find out what colormap to install.  By default, this 
attribute indicates the default colormap.  If its subwindows use 
different colormaps from the main window, there is a property that 
can be set to tell this to the window manager, as described 
in Chapter 10.
.sp .5
Several of the older window managers do not install colormaps properly.  
Under these, an application that creates its own colormap also has to 
install it.  However, as these window managers are revised or go out 
of use, it becomes a hard rule that application should never install 
their own colormaps.  This is required by the current conventions
described in Chapter 10.
.sp .5
By now you should be getting the idea that it is much better to
arrange to share the default colormap with the other applications 
than to try to create one of your own.  The only time when you 
should really need to create a special colormap is when you are 
doing smooth shading or similar applications that need many 
strangely distributed colors.  On the other hand, creating a 
virtual colormap might be the only way to make your application 
that has demanding color needs work on a system that provides 
only a \f(CWPseudoColor\fP visual.  On systems with multiple 
hardware colormaps, you can create your own colormap and have 
it installed without affecting other applications.  You can use 
\f(CWXListInstalledColormaps\fP to get information about how 
many colormaps are installed into the hardware.
.sp .5
.IP \s+2\f(HB7.7.1\fR\s-2 6n
\s+2\f(HBFunctions for Manipulating Colormaps\fR\s-2
.sp .5
The following functions should be used by applications only if 
they need a special purpose colormap:
.IP "\f(CWXCreateColormap\fP" 25n
Creates a virtual colormap resource, either with no allocated 
entries or with all allocated read/write, that matches the passed 
visual.  If no entries are allocated, they can be allocated 
either as read/write or as read-only cells.  If all entries are 
allocated read/write, the colormap is completely private and just 
needs its colors set with \f(CWXStoreColors\fP.
.IP "\f(CWXFreeColormap\fP" 25n
Uninstalls the specified virtual colormap and frees the resources
associated with the colormap.  Applications are allowed to use this.
Sends a \f(CWColormapNotify\fP event to any windows that were using 
the colormap.
.IP "\f(CWXListInstalledColormaps\fP" 25n
.br
Lists the installed colormaps.
.IP "\f(CWXCopyColormapAndFree\fP" 25n
.br
Moves all the client's existing colormap entries to a new colormap 
and frees those entries of the old colormap.  This is used when 
colorcell allocation fails and some cells have already been allocated.  
It saves needing to create a colormap and start from the beginning 
allocating colors.  For applications with special color needs that 
can't make do, they can call \f(CWXCopyColormapAndFree\fP, set their 
colormap window attribute, and continue allocating colors in the new 
colormap where they left off.
.IP "\f(CWXSetWindowColormap\fP" 25n
Sets the colormap window attribute of a window.
.sp .5
The following functions are use by the window manager to install and
uninstall colormaps:
.IP "\f(CWXInstallColormap\fP" 25n
A function only to be used by window managers to install a 
colormap.  Any window using that colormap ID as its colormap 
attribute receives a \f(CWColormapNotify\fP event.
.IP "\f(CWXUninstallColormap\fP" 25n
A function only to be used by window managers to uninstall a 
colormap.  Removes a virtual colormap from the set of installed 
hardware colormaps.  On systems with only one hardware colormap, 
the default colormap is reinstalled.  Sends \f(CWColormapNotify\fP 
event to windows that are using the specified map.
.sp .5
.IP \s+2\f(HB7.7.2\fR\s-2 6n
\s+2\f(HBThe ColormapNotify Event\fR\s-2
.sp .5
\f(CWColormapNotify\fP events notify an application when the 
colormap specified in the colormap attribute for a particular
window has been installed, uninstalled, or freed or when the 
attribute itself has been changed.  The former is used by 
applications, and the latter by window managers. 
.sp .5
If your application wants to know when your colormap is installed
or uninstalled, it should watch for these events and act accordingly.  
To receive \f(CWColormapNotify\fP events, pass \f(CWColormapChangeMask\fP 
(ORed with the other masks you need) to \f(CWXSelectInput\fP.
Example 9 shows the \f(CWXColormapEvent\fP structure.
.sp .5
\s-1\f(CW
.nf
.ta 4.5n 2i
typedef struct {
    int type;
    unsigned long serial;    /* # of last request processed by server */
    Bool send_event;    /* True if this came from SendEvent 
         * request */
    Display *display;    /* Display the event was read from */
    Window window;
    Colormap colormap;    /* Colormap or None */
    Bool new;
    int state;    /* ColormapInstalled, ColormapUninstalled */
} XColormapEvent;
\fR\s0
.fi
.sp .5
Example 9: "The ColormapEvent structure"
.sp
.Nd 10
Here is a brief explanation of each member of the 
\f(CWXColormapEvent\fP structure:
.IP "\f(CWwindow\fP" 15n
The window for which this event was selected, whose colormap 
attribute was changed or whose colormap specified in that 
attribute was installed, uninstalled, or freed.
.Nd 4
.IP "\f(CWcolormap\fP" 15n
The colormap associated with the window, either a colormap ID or 
the constant \f(CWNone\fP.  It will be \f(CWNone\fP only if this 
event was in response to an \f(CWXFreeColormap\fP call.
.IP "\f(CWnew\fP" 15n
\f(CWTrue\fP when the colormap attribute has been changed, or
\f(CWFalse\fP when the colormap is installed or uninstalled.
.IP "\f(CWstate\fP" 15n
Either \f(CWColormapInstalled\fP or \f(CWColormapUninstalled\fP;
it indicates whether the colormap is installed or uninstalled.
.sp .5
\f(CWXFreeColormap\fR, \f(CWXInstallColormap\fR, and 
\f(CWXUninstallColormap\fR generate this event for windows that 
have their colormap attribute set to the colormap that was affected.  
\f(CWXSetWindowColormap\fP and \f(CWXChangeWindowAttributes\fP can 
also generate this event.  From the information in the structure, 
you can tell which of these calls generated the event and what 
the current status of the colormap is.  
See Chapter 10 in this manual and the ICCCM
for an additional description of the conventions regarding colormaps.
.sp .5
.IP \s+2\f(HB7.7.3\fR\s-2 6n
\s+2\f(HBThe Required Colormap List\fR\s-2
.sp .5
The X protocol specifies that each server can specify a required list
of colormaps, which affects what happens when other colormaps are
installed or uninstalled.  Here is what the protocol specification
says about the required list (translated into Xlib terms):
.QP
At any time, there is a subset of the installed maps, viewed as an
ordered list, called the required list.  The length of the required
list is at most \f(CWmin_maps\fP, where \f(CWmin_maps\fP is a member
of the \f(CWDisplay\fP structure.  The required list is maintained as
follows.  When a colormap is an explicit argument to \f(CWXInstallColormap\fP,
it is added to the head of the list, and the list is truncated at the
tail if necessary to keep the length of the list to at most \f(CWmin_maps\fP.  
When a colormap is an explicit argument to \f(CWXUninstallColormap\fP 
and it is in the required list, it is removed from the list.  A colormap 
is not added to the required list when it is installed implicitly by the 
server, and the server cannot implicitly uninstall a colormap that is 
in the required list.
.sp .5
In less precise words, the \f(CWmin_maps\fP most recently installed maps
are guaranteed to be installed.  This number will often be one; clients
needing multiple colormaps should beware.
.IP \s+4\f(HB7.8\fR\s-4  6n
\s+4\f(HBStandard Colormaps\fR\s-4
.sp
A \fIstandard colormap\fR is one in which the mapping between
pixel values and colors is predictable.  The purpose of standard
colormaps is to encourage sharing of entire colormaps (not just 
individual cells) between applications that have too demanding 
color needs to be able to allocate read-only colors out of the 
default colormaps.
.sp .5
X defines a set of properties that contain information describing
commonly used colormaps.  An application reads these properties
by calling \f(CWXGetRGBColormaps\fP (\f(CWXGetStandardColormap\fP 
prior to R4).  This call returns an \f(CWXStandardColormap\fP
structure that contains enough information so that the application
can calculate the colors in every colormap cell (or a certain range 
within the colormap).  This structure may also include the ID of a
colormap matching this description that was created by the window
manager or another client.  Otherwise, the application then creates 
a new colormap and uses this information to allocate and set the 
colors according to the information in the property.
.sp .5
But how does the sharing work?  After creating this colormap, the
application sets the ID of the created colormap into the \f(CWcolormap\fP
field of the \f(CWXStandardColormap\fP structure and then calls
\f(CWXSetRGBColormaps\fP (\f(CWXSetStandardColormap\fP prior to R4).
This resets the property, so that the next time another client calls
\f(CWXGetRGBColormaps\fP, the \f(CWcolormap\fP field of the returned
structure will actually contain the ID of the appropriate colormap.
.sp .5
Therefore, although an application must have the code to create, 
allocate, and set colors in a standard colormap, in some cases this
code will not be executed because some other client will have already
done the work.  After calling \f(CWXGetRGBColormaps\fP, if the 
\f(CWcolormap\fP field is zero, the application must create the 
colormap.  Otherwise, the \f(CWcolormap\fP field holds the ID of 
an appropriate colormap.
.sp .5
When an application uses standard colormaps, two (or more) instances
of the application can run at the same time without increasing the
load on the system caused by creating multiple copies of the same 
colormap.  Applications that do not use standard colormaps will end 
up creating separate but identical colormaps.  The window manager 
will switch these in and out of the hardware colormap whenever a 
different instance is in use.  Although nothing on the screen will 
change color because both the colormaps are identical, the server 
will be performing unnecessary installing and uninstalling, and the 
extra colormaps will waste server memory.
.sp .5
In some cases, the window manager or even the server will create one 
or more standard colormaps.  This does not change how applications 
work at all.  Applications do not care whether it was the window 
manager, the server, or some other client that created a standard 
colormap.
.sp .5
If your application does not create or use a custom colormap,
you can skip this section if pressed for time.
.sp .5
Applications can also use the knowledge about a standard colormap 
to optimize the process of figuring out which existing pixel values 
correspond to required colors and which colors must be allocated 
and set from scratch.
.sp .5
.IP \s+2\f(HB7.8.1\fR\s-2 6n
\s+2\f(HBThe Standard Colormap Properties\fR\s-2
.sp .5
The standard colormap properties contain information about a few
commonly used colormaps.  However, note that even if an application 
creates a custom colormap unlike any of these, it should still use
the standard property mechanism by creating its own standard 
colormap structure.
.sp .5
Properties were introduced in Section 2.1.4.  For a more complete 
description of properties, see Section 10.1.
.sp .5
In the call to \f(CWXGetRGBColormaps\fP (or \f(CWXGetStandardColormap\fP),
you specify one of these atoms like \f(CWXA_RGB_BEST_MAP\fP (or, if 
necessary, one unique to your application).
.sp .5
The following list names the atoms and describes the colormap
associated with each one:
.IP "\f(CWXA_RGB_DEFAULT_MAP\fP" 10n
.br
This property defines part of the system default colormap.  This 
colormap may be initially completely unallocated, or it may contain 
a selection of read-only colorcells with the RGB values from the 
color database and a few unallocated cells for use by applications 
that need read/write cells.  A typical allocation of the 
\f(CWXA_RGB_DEFAULT_MAP\fP on eight-plane screens is all the colors 
produced from any combination of six reds, six greens, and six blues.  
This gives 216 uniformly distributed colors and leaves 40 for other 
programs or for special purpose colors for text, borders, and so on.
A typical allocation for the \f(CWXA_RGB_DEFAULT_MAP\fP on 24-plane 
screens is 64\ reds, 64 greens, and 64 blues.  This gives about 
one\ million uniformly distributed colors (64 intensities of 4096 
different hues) and leaves lots of colorcells available for other purposes.
.IP "\f(CWXA_RGB_BEST_MAP\fP" 10n
.br
This property defines the "best" RGB colormap available on the screen.  
Of course, this is a subjective evaluation.  Many image-processing and 
3-D programs need to use all available colormap cells and to distribute 
as many perceptually distinct colors as possible over those cells.  In 
this case, there may be more green values available than red and more 
green or red than blue.
.sp .5
On an eight-plane \f(CWDirectColor\fP visual, \f(CWXA_RGB_BEST_MAP\fP 
is usually a 3/3/2 allocation.  On a 24-plane \f(CWDirectColor\fP visual, 
\f(CWXA_RGB_BEST_MAP\fP is usually an 8/8/8 allocation.  On other screens, 
\f(CWXA_RGB_BEST_MAP\fP is purely up to the implementor of the server.
.IP "\f(CWXA_RGB_RED_MAP\fP,\ \f(CWXA_RGB_GREEN_MAP\fP,\ \f(CWXA_RGB_BLUE_MAP\fP" 10n
.br
These properties define all-red, all-green, and all-blue colormaps, 
respectively.  These maps are used by programs that make color-separated 
images.  For example, a user might generate a full color image on an 
eight-plane screen by rendering an image once with high color resolution 
in red, once with green, and once with blue and exposing a single 
frame in a camera with three images.
.IP "\f(CWXA_RGB_GRAY_MAP\fP" 10n
.br
This property describes the "best" gray-scale colormap
available on the screen.  
.sp .5
.IP \s+2\f(HB7.8.2\fR\s-2 6n
\s+2\f(HBThe XStandardColormap Structure\fR\s-2
.sp .5
As described above, an application that wants to use a standard 
colormap must get the structure that contains the specification 
for the colormap using \f(CWXGetRGBColormaps\fP 
(\f(CWXGetStandardColormap\fP prior to R4).  Some servers and 
window managers, particularly on high performance workstations, 
create some or all of the standard colormaps when they initialize.
If the desired colormap has already been created, it is returned 
in the \f(CWcolormap\fP member of the \f(CWXStandardColormap\fP 
structure shown in Example 10.  If the colormap does not 
yet exist, the \f(CWcolormap\fP member will be zero.  In that case, 
the application can create a colormap and allocate entries to match 
the specification in the members of \f(CWXStandardColormap\fP, 
then call \f(CWXSetRGBColormaps\fP (\f(CWXSetStandardColormap\fP 
prior to R4) to allow other clients to share this colormap.
.sp .5
\s-1\f(CW
.nf
typedef struct _XStandardColormap {
    Colormap colormap;
    unsigned long red_max, green_max, blue_max;
    unsigned long red_mult, green_mult, blue_mult;
    unsigned long base_pixel;
    VisualID visualid;         /* Added in R4:  ICCCM version 1 */
    XID killid;                /* Added in R4:  ICCCM version 1 */
} XStandardColormap;
\fR\s0
.fi
.sp .5
Example 10: "The XStandardColormap structure"
.sp
The members of the \f(CWXStandardColormap\fP structure are as follows:
.sp
.RS
.IP \(bu 5
The \f(CWcolormap\fR member is the ID of a colormap created by the
\f(CWXCreateColormap\fP function or the default colormap.  This ID
can be used to install a virtual colormap into the hardware colormap.
.IP \(bu 5
The \f(CWred_max\fP, \f(CWgreen_max\fP, and \f(CWblue_max\fP fields 
give the maximum red, green, and blue values, respectively.  A 
typical allocation that provides 
.Su 6 3 
=\ 216 read-only, shareable colors in a \f(CWPseudoColor\fP colormap 
on a standard eight-plane workstation is \f(CWred_max\fP\ =\ 5, 
\f(CWgreen_max\fP\ =\ 5, and \f(CWblue_max\fP\ =\ 5.  This leaves 
40 cells available for special colors and private, nonshareable purposes.
.\"On the \f(CWStellarStation\fP, 
On a 24-plane workstation, there would be eight bits available for 
each color in a \f(CWTrueColor\fP visual, which would allow 256 shades 
of each primary color.  In this case, \f(CWred_max\fP\ =\ 255, 
\f(CWgreen_max\fP\ =\ 255, and \f(CWblue_max\fP\ =\ 255.  This map 
would include 
.Su 256 3 
=\ 16.38 million total colors.
.IP \(bu 5
The \f(CWred_mult\fP, \f(CWgreen_mult\fP, and \f(CWblue_mult\fP fields 
scale each pixel subfield into the proper range in the 16-bit RGB value in 
the colorcell with the range 0 to 65535.  The red pixel subfield is moved 
\f(CWred_mult\fP bits toward the most significant bit of the pixel value.
.sp .5
For a 3/3/2 \f(CWDirectColor\fP allocation (eight reds, eight greens, 
four blues), \f(CWred_mult\fP might be 32, \f(CWgreen_mult\fP might 
be 4, and \f(CWblue_mult\fP might be 1 (as shown in Figure 5).  
These effectively move the red value into the most significant bits 
of the RGB value in the colorcell, the green into the middle, and the 
blue into the least significant bits.  This arrangement is arbitrary 
but useful.  For a six-colors-each allocation, which must be 
\f(CWPseudoColor\fP since the planes cannot be evenly allocated to 
separate primaries, \f(CWred_mult\fP might be 36, \f(CWgreen_mult\fP 
might be 6, and \f(CWblue_mult\fP might be\ 1.
.IP \(bu 5
The \f(CWbase_pixel\fP field gives the base value that is added to 
the pixel value calculated from the RGB values and scale factors.  
Usually the \f(CWbase_pixel\fP is obtained from a call to the 
\f(CWXAllocColorPlanes\fP function.  
.IP \(bu 5
The \f(CWvisualid\fP field is the ID of a server resource associated 
with each visual, of type \f(CWVisualID\fP.  You will need this ID only 
if you intend to use standard colormaps.  This field was added in R4 
because only with this information can standard colormaps be used with 
other than the default visual.  Prior to R4, standard colormaps were not 
in wide use, partly because they could only be used with the default visual.
.IP \(bu 5
The \f(CWkillid\fP field returns a number that is used if the
application needs to free the colormap for some reason.  If 
\f(CWkillid\fP is greater than one, then the resources should be 
freed by calling \f(CWXKillClient\fP with the \f(CWkillid\fP field 
as the argument.  If \f(CWkillid\fP is one, then the resources should 
be freed by calling \f(CWXFreeColormap\fP with the \f(CWcolormap\fP 
field as the argument.  If \f(CWkillid\fP is zero, then no attempt 
should be made to free the resources.
.RE
.sp 2
Only the \f(CWcolormap\fP, \f(CWred_max\fP, \f(CWred_mult\fP, 
and \f(CWbase_pixel\fP fields of the \f(CWXStandardColormap\fP 
structure are use for \f(CWGrayScale\fP colormaps. 
.sp .5
.IP \s+2\f(HB7.8.3\fR\s-2 6n
\s+2\f(HBThe 3/3/2 Standard Colormap\fR\s-2
.sp .5
Now let's look at a typical standard colormap.  The following example 
describes the 3/3/2 \f(CWDirectColor\fP standard colormap used on 
eight-plane screens.  Three planes are used for red, three planes for 
green, and two planes for blue.  This 3/3/2 allocation allows values 
in the range of:
.sp .5
\s-1\f(CW
.nf
red    0-7    \fR\s+1thus\s-1\fP    red_max    =  7  
green  0-7           green_max  =  7  
blue   0-3           blue_max   =  3  
\fR\s0
.fi
.sp .5
To obtain the pixel value, these RGB values must be shifted to their
corresponding planes.  If the red value is contained in the three 
most significant planes or bits, the green values in the three next
most significant planes or bits, and the blue value in the two least
significant planes or bits, then the pixel can be constructed as
shown in Figure 5.
.sp
Note: Figure 5 "Shifting pixel subfields into pixel value" here.
.sp
In a \f(CWDirectColor\fP system like this, the multiples are equal to 
.Su + 2 \fIn\fP 
, where \fIn\fR is their lowest plane or bit position.  If the red, 
green, and blue were stored in a different order, the multiples would 
not be 32, 4, 1 but would still be calculated from the above description 
and formula.  The 3/3/2 standard colormap allocation is fairly standard.
.sp .5
.IP \s+2\f(HB7.8.4\fR\s-2 6n
\s+2\f(HBCreating and Using a Standard Colormap\fR\s-2
.sp .5
Two members were added to the \f(CWXStandardColormap\fP structure 
in Release 4 to comply with the ICCCM (interclient communication 
conventions, described in Chapter 10).  Because of this, there 
are two different sets of routines that manage standard colormaps, 
one for use with R3 (and earlier) and the other for use with R4 
(and later).  The R3 routines will also work under R4, but an 
application using them will not comply with current conventions.  
The R3 routines are \f(CWXGetStandardColormap\fP and 
\f(CWXSetStandardColormap\fP, and the R4 routines are 
\f(CWXGetRGBColormaps\fP and \f(CWXSetRGBColormaps\fP.  The reason 
for the plural form of the R4 routine \f(CWXGetRGBColormaps\fP is 
that it returns a list of colormaps; it also has a \f(CIcount\fP 
argument not present in the R3 routine.  According to the ICCCM, 
only queries of the \f(CWXA_RGB_DEFAULT_MAP\fP standard colormap 
can return more than one structure.
.sp .5
Example 11: gets information about the \f(CWXA_RGB_BEST_MAP\fP 
standard colormap, creates it if no other client already has, calculates 
pixel values from it, and sets the colormap window attribute of the 
window.  Note that under R3, the created colormap can be used only 
with windows created using the default visual, because the standard 
colormap information under R3 does not include a visual.  This example 
gives up and falls back on read-only colorcell allocation if the 
standard colormap property is not defined by the server or if creating 
a colormap returns the default colormap (which happens on systems with 
an immutable hardware colormap).
.Nd 10
.sp .5
\s-1\f(CW
.nf
  .
  .
  .

#define USE_DEFAULT_COLORMAP 1
#define USE_STANDARD_COLORMAP 0

void main(argc, argv)
int argc;
char **argv;
{
    XStandardColormap best_map_info;
    XColor *exact_defs;
    XSetWindowAttributes attrib;
    unsigned long attribmask;
    int i, j, k, l;
    int ncells;
    XVisualInfo *vlist, vinfo_template, *v;
    int num_vis;
    int count;
    Visual *visual;
    int strategy = USE_STANDARD_COLORMAP;
      .
      .
      .
    /* Open display */

    visual = DefaultVisual(display, screen_num);

#ifdef X11R3
        if (XGetStandardColormap(display, RootWindow(display,
                        screen_num), &best_map_info,
            XA_RGB_BEST_MAP) == 0) {
#else /* R4 or later */
        if (XGetRGBColormaps(display, RootWindow(display,
                        screen_num), &best_map_info, &count, 
            XA_RGB_BEST_MAP) == 0) {
#endif /* X11R3 */
                printf("%s: RGB_BEST_MAP colormap property not set.\en",
                argv[0]);
        /* Give up standard colormaps; use one of the 
         * basic color strategies */
        get_colors();
        strategy = USE_DEFAULT_COLORMAP;
    }
        else if (best_map_info.colormap) {
        /* Someone else created the map we need; make sure 
         * it's valid, then we'll use it below */
        if (best_map_info.red_max == 0) {
            printf("%s: RGB_BEST_MAP colormap property is set\en", 
                    argv[0]);
            printf("but is missing data.\en");
            strategy = USE_DEFAULT_COLORMAP;
        }
                attrib.colormap = best_map_info.colormap;
        }
    else if (best_map_info.visualid == 0) {
        printf("%s: Standard colormap property is set\en", argv[0]);
           printf("but is missing data.");
        /* Some systems define the properties but don't 
         * place any data in them; this is a server bug, 
         * but we'll check for it anyway */
        /* Fall back on a basic color strategy */
        strategy = USE_DEFAULT_COLORMAP;
    }
    else {
        /* Got information, but the described colormap 
         * has not been created yet; create it and 
         * allocate all cells read/write */

        /* XCreateColormap requires a visual argument 
          * (pointer to a Visual structure); however, the 
          * XStandardColormap structure returns a VisualID, 
         * which might not be the default visual; 
         * Converting between these two is painful */
        vlist = XGetVisualInfo(display, VisualNoMask, 
                &vinfo_template, &num_vis);
        for (v = vlist; v < vlist + num_vis; v++) {
            if (v->visualid == best_map_info.visualid) {
                visual = v->visual;
                break;
            }
        }

        best_map_info.colormap = XCreateColormap(display, 
                RootWindow(display, screen_num), 
                visual, AllocAll);

        if (best_map_info.colormap == 
                DefaultColormap(display, screen_num)) {
            printf("%s: hardware colormap is immutable:\en", 
                    argv[0]);
            printf("cannot create new colormap.\en");
        }

        attrib.colormap = best_map_info.colormap;

        ncells = best_map_info.base_pixel + 
                ((best_map_info.red_max + 1) * 
                   (best_map_info.green_max + 1) * 
                (best_map_info.blue_max + 1));

        exact_defs = (XColor *) calloc(sizeof(XColor), ncells); 

        /* Permute the levels of red, green, and blue */
        l = best_map_info.base_pixel;
        for (i = 0; i < best_map_info.blue_max; i++) {
            exact_defs[l].blue = 65536 * i / best_map_info.blue_max;
            for (j = 0; j < best_map_info.green_max; j++) {
                exact_defs[l].green = 65536 * j / 
                        best_map_info.green_max;
                for (k = 0; k < best_map_info.red_max; k++) {
                    exact_defs[l].red = 65536 * k / 
                            best_map_info.red_max;
                    l++;
                }
            }
        }

        XStoreColors (display, best_map_info.colormap, exact_defs, 
                ncells);
        
        /* If to be used in a window not created with the
         * default visual, must create the window first and
         * use instead of RootWindow in this call; here we
         * assume the default visual */
#ifdef X11R3
        XSetStandardColormap(display, RootWindow(display, 
                screen_num), &best_map_info, XA_RGB_BEST_MAP);
#else /* R4 or later */
        XSetRGBColormaps(display, RootWindow(display, screen_num), 
                count, &best_map_info, XA_RGB_BEST_MAP);
#endif /* X11R3 */
    }

    if (strategy == USE_STANDARD_COLORMAP) {
        /* We must not have called get_colors above, 
         * must be using standard colormaps strategy */

        /* Note that we act like we have already allocated pixel
          * pixel values, even though actually another client did */
        background_pixel = best_map_info.base_pixel  +
                (best_map_info.red_max * best_map_info.red_mult) +
                (best_map_info.green_max * best_map_info.green_mult) +
                (best_map_info.blue_max * best_map_info.blue_mult);

        attribmask = CWBackPixel | CWColormap;

        foreground_pixel = (best_map_info.green_max * 
                best_map_info.green_mult / 2) + 
                best_map_info.base_pixel;
        
        border_pixel = (best_map_info.blue_max * 
                best_map_info.blue_mult / 2) + 
                best_map_info.base_pixel;
    }

    /* Create opaque window */
    win = XCreateWindow(display, RootWindow(display,screen_num), x,  
            y, width, height, borderwidth, 
            DefaultDepth(display, screen_num), 
            InputOutput, visual, attribmask, &attrib);
      .
      .
      .
}
\fR\s0
.fi
.sp .5
Example 11: "Code to create and use XA_RGB_BEST_MAP"
.sp
.hw XGetRGB-Colormaps
This code begins by reading the \f(CWXA_RGB_BEST_MAP\fP property 
using the \f(CWXGetRGBColormaps\fP or \f(CWXGetStandardColormap\fP 
call.  (The name \f(CWXGetRGBColormaps\fP suggests that the function 
returns a description of multiple colormaps\(embut this is true only 
for \f(CWXA_RGB_DEFAULT_MAP\fP.  Therefore, the R3 and R4 functions 
are truly equivalent as used here.)  If one of these calls succeeds, 
the property is defined and its contents have been placed in the 
\f(CWbest_map_info\fP structure.  Since any other, perhaps buggy, 
client might have set this property (like your own application while 
you are debugging it), it is a good idea not to trust its contents 
any more than necessary.  (Properties set on the root window remain 
defined even after the client that set them has exited.)  The code checks 
to make sure that the fields contain reasonable values before using them.
.sp .5
If the \f(CWcolormap\fP field of \f(CWbest_map_info\fP is nonzero, it 
should be the ID of a standard colormap that another client has created.  
Your application can immediately proceed to use the pixel values in this 
colormap as though your application had already allocated them read-only, 
even though in reality some other client allocated them read/write.
.sp .5
On the other hand, if the \f(CWcolormap\fP field is zero, your application 
needs to create, allocate, and set the values of the standard colormap 
itself.  You allocate the cells read/write, because this allows you to 
explicitly set the RGB values of each pixel value.  Even though you 
allocate the cells read/write, you should use them as if they were 
read-only, so that other applications can share them after you reset 
the \f(CWXA_RGB_BEST_MAP\fP property to include the new colormap ID.
As this suggests, a read/write cell, even though described earlier as 
being private and changeable by that one client, can be public if all 
the applications agree not to change its RGB values.
.sp .5
The algorithm used to store RGB values into the cells in the colormap is
somewhat arbitrary.  Conventions for it will probably be adopted by the 
X Consortium when there is more interest in standard colormaps.  Any 
algorithm is good enough to allow two instances of the same application 
to share a colormap.  But for two different applications to share the 
colormap, each must know exactly what RGB values the other would place 
in the colormap if the other were run before the colormap was created.
.sp .5
The \f(CWXGetVisualInfo\fP call is described in Section 7.5.2.
.sp .5
If you pass \f(CWAllocAll\fP to \f(CWXCreateColormap\fP, you do not 
need to make an \f(CWXAllocColorCells\fP call to allocate all the 
cells read/write.  However, you can use \f(CWAllocAll\fP only if you 
intend the entire colormap to be read-only to all clients.  Some 
clients want a few cells preserved to be rewriteable for dynamically 
changing colors.  If yours is that way, you must use \f(CWAllocNone\fP 
and then call \f(CWXAllocColorCells\fP once to create the standard 
portion of the colormap and again to allocate the cells your 
application will treat as private.
.sp .5
Once the colormap window attribute of a window is set, the window
manager will take care of installing the colormap.  When there is
only one hardware colormap, the window manager usually installs an
application's colormap when that application contains the pointer 
(for real-estate type window managers) or is given the keyboard 
focus (for click-to-type style window managers).
.sp .5
When a window manager creates a standard colormap, it can use a 
slightly different technique to make sure that the standard colormap
remains defined even after the window manager exits.  Assuming that it 
has already checked to see whether some other client has created a 
standard colormap and none has, it performs the following sequence of steps:
.sp
.RS
.IP \(bu 5
Create a new connection to the same server. 
.IP \(bu 5
Determine the color capabilities of the screen.  Choose a visual.
.IP \(bu 5
Create a colormap (not required for \f(CWXA_RGB_DEFAULT_MAP\fP).
.IP \(bu 5
Call \f(CWXAllocColorPlanes\fP or \f(CWXAllocColorCells\fP to 
allocate cells in the colormap (if did not use \f(CWAllocAll\fP 
flag when creating the colormap).
.IP \(bu 5
Call \f(CWXStoreColors\fP to store appropriate color values in 
the colormap.
.IP \(bu 5
Fill in the descriptive fields in the \f(CWXStandardColormap\fP 
structure, including the ID of the created colormap.
.IP \(bu 5
Call \f(CWXSetRGBColormaps\fP or \f(CWXSetStandardColormap\fP to set 
the property on the root window.  Under R4, the \f(CWkillid\fP field 
should be set to the colormap ID.
.Nd 4
.IP \(bu 5
Use \f(CWXSetCloseDownMode\fR to make the resource permanent.
.IP \(bu 5
Close the new connection to the server.
.RE
.sp 2
.IP \s+2\f(HB7.8.5\fR\s-2 6n
\s+2\f(HBRGB-to-Pixel Conversion\fR\s-2
.sp .5
The standard colormaps such as \f(CWXA_RGB_BEST_MAP\fP are useful 
when you want to calculate pixel values from RGB values.  
.sp .5
Consider a 3-D display program that draws a smoothly shaded sphere.  
At each pixel in the image of the sphere, the program computes the 
intensity and color of light reflected to the viewer.  The result 
of each computation is a triple of red, green, and blue coefficients 
in the range 0.0 to 1.0.  To draw the sphere, the program needs a 
colormap that provides a large range of uniformly distributed 
colors.  The colormap must be arranged so that the program can
convert its RGB triples into pixel values very quickly, because 
drawing the entire sphere will require many such conversions.  
An example of one such calculation is shown in Example 12.  
Example 10 demonstrated how to do this for integral RGB values.
.sp .5
\s-1\f(CW
.nf
XStandardColormap best_map_info;
float red, green, blue;
unsigned long pixelvalue;

pixelvalue = best_map_info.base_pixel +
   ((unsigned long)(0.5 + (red * best_map_info.red_max)) *
        best_map_info.red_mult) + 
   ((unsigned long)(0.5 + (green * best_map_info.green_max)) * 
        best_map_info.green_mult) +
   ((unsigned long)(0.5 + (blue * best_map_info.blue_max)) *
        best_map_info.blue_mult);
\fR\s0
.fi
.sp .5
Example 12: "Calculating pixel values from floating point RGB values"
.sp
For gray scale colormaps, only the \f(CWcolormap\fP, 
\f(CWred_max\fP, \f(CWred_mult\fP, and \f(CWbase_pixel\fP fields 
of the \f(CWXStandardColormap\fP structure are defined.  The other 
fields are ignored.  Pixel values for a \f(CWStaticGray\fP or 
\f(CWGrayScale\fP visual must be in the range:
.sp .5
\s-1\f(CW
.nf
base_pixel <= pixel_value < (red_max * red_mult) + base_pixel
\fR\s0
.fi
.sp .5
To compute a gray pixel value, use the following expression:
.sp .5
\s-1\f(CW
.nf
pixel_value = gray * red_mult + base_pixel;
\fR\s0
.fi
.sp .5
where:
.in +5n
.TS
linesize(2), tab(@);
lfCW l l.
gray@\=@the gray value you desire (0 to \f(CWred_max\fP)
red_mult@\=@value from \f(CWXStandardColormap\fP structure
base_pixel@\=@value from \f(CWXStandardColormap\fP structure
.TE
.in -5n
.IP \s+4\f(HB7.9\fR\s-4  6n
\s+4\f(HBMiscellaneous Color-handling Functions\fR\s-4
.sp
The following miscellaneous functions provide additional ways to 
use the color database, to find out the RGB values in a colormap 
cell, and to free cells that are no longer needed:
.IP "\f(CWXLookupColor\fP" 18n
Looks up a string color name in the color database and returns 
separate color structures containing the exact RGB values specified 
in the database for that name and the closest RGB values available 
on the hardware.  This function does not look at any cells in the 
colormap, even though it has a \f(CIcolormap\fP argument!  This 
argument specifies which screen the color should be looked up on.  
The difference between \f(CWXLookupColor\fP and \f(CWXParseColor\fP
is that \f(CWXParseColor\fP accepts the hexadecimal color specification 
(which \f(CWXLookupColor\fP does not), while \f(CWXLookupColor\fP 
returns the closest colors available on the hardware (which 
\f(CWXParseColor\fP does not).  \f(CWXLookupColor\fP might be 
useful for making sure that user-specified colors are contrasting.
.Nd 3
.IP "\f(CWXQueryColor\fP"
Fills an \f(CWXColor\fP structure with the RGB values corresponding to 
the colormap cell indicated by a pixel value.  Also sets the \f(CWflags\fP 
member of the structure to (\f(CWDoRed\ |\ DoGreen\ |\ DoBlue\fR).
.IP "\f(CWXQueryColors\fR"
Fills multiple \f(CWXColor\fP structures with the RGB values and flags 
corresponding to the colormap cells indicated by a pixel values.  Also 
sets each \f(CWflags\fP member to (\f(CWDoRed\ |\ DoGreen\ | DoBlue\fR).
.IP "\f(CWXFreeColors\fR" 18n
Frees the colormap cells associated with the given pixel values and/or 
frees the given planes.  Since all the colorcells an application allocates 
are freed when the application exits, this routine is needed only when an 
application is finished with cells before it exits.  Freeing a read/write 
colorcell makes that cell available to other applications.  Freeing a 
read-only cell may make the cell unallocated, but only if no other 
application is sharing that cell.
