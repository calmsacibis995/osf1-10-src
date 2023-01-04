/* #module DT_version "T3.0" */
/*
 *  Title:	DT_version
 *
 *
 *  +------------------------------------------------------------------------+
 *  | Copyright © Digital Equipment Corporation, 1988, 1993 All Rights       |
 *  | Reserved.  Unpublished rights reserved under the copyright laws of     |
 *  | the United States.                                                     |
 *  |                                                                        |
 *  | The software contained on this media is proprietary to and embodies    |
 *  | the confidential technology of Digital Equipment Corporation.          |
 *  | Possession, use, duplication or dissemination of the software and      |
 *  | media is authorized only pursuant to a valid written license from      |
 *  | Digital Equipment Corporation.                                         |
 *  |                                                                        |
 *  | RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure by the      |
 *  | U.S. Government is subject to restrictions as set forth in             |
 *  | Subparagraph (c)(1)(ii) of DFARS 252.227-7013, or in FAR 52.227-19,    |
 *  | as applicable.                                                         |
 *  |                                                                        |
 *  | The information in this software is subject to change  without  notice |
 *  | and  should  not  be  construed  as  a commitment by Digital Equipment |
 *  | Corporation.                                                           |
 *  |                                                                        |
 *  | DIGITAL assumes no responsibility for the use or  reliability  of  its |
 *  | software on equipment which is not supplied by DIGITAL.                |
 *  +------------------------------------------------------------------------+
 *  
 *  
 *  Module Abstract:
 *
 *	This module provides a singular global string indicating the DECterm
 *	version number.  Any other modules that need the version number
 *	string should "globalref char decterm_version[];"
 *
 *
 *  Author:	Eric Osman
 *
 *  Modification history:
 *
 *  Eric Osman		30-Jul-1993	BL-D
 *	- Merge vxt and vms decterm sources.
 *
 *  Aston Chan		23-Apr-1993	V1.2/BL2
 *	- Update the major version to 1.2
 * 
 *  Eric Osman		11-June-1992	Sun
 *	No __DATE__ and __TIME__ defined on Sun.
 *
 *  Aston Chan		07-May-1992	Post V3.1/SSB
 *	Include compile date time to the version number.  The Version number
 *	has to be updated for the next major release.
 *
 *  Aston Chan		30-Apr-1992	V3.1/SSB
 *	Update version for VMS DECwindows Motif V1.1 SSB
 *
 *  Aston Chan		09-Mar-1992	V3.1/BL6
 *	Update version for VMS DECwindows Motif V1.1 BL6 (SSB??)
 *  Aston Chan		11-Feb-1992	V3.1/BL5
 *	Update version for VMS DECwindows Motif V1.1 Bl5 (EFT)
 *
 *  Aston Chan		15-Jan-1992	V3.1
 *	Update version for VMS DECwindows Motif V1.1 BL4 (EFT)
 *
 *  Alfred von Campe    19-Dec-1991     V3.1
 *	Update version for VMS DECwindows Motif V1.1 BL3 (IFT)
 *
 *  Alfred von Campe    09-May-91       V3.0
 *	Prepare for final (SSB) release.
 *
 *  Alfred von Campe    09-Apr-91       T3.0-EFT2
 *	Prepare for (almost) final release.
 *
 *  Jim Bay		11-Jan-1991	T3.0
 *	First external field test T3.0
 *
 *  Jim Bay		20-Nov-1990	X3.0-8
 *	Converted widgets to gadgets
 *
 *  Mark Granoff	14-Jun-1990	X3.0-4
 *
 *  Mark Woodbury 	25-May-1990	X3.0-3M
 *
 *  Mark Granoff	15-May-1990	X3.0-3
 *
 *  Mark Granoff	29-Jan-1990	X3.0-1
 *
 *  Eric Osman		14-Oct-1987	X0.2
 *	First release.
 *
 *  <modifier's name>	<date>		<ident of revised code>
 *	<description of change and purpose of change>
 *
 *  Peter Sichel    28-Dec-1987
 *  Remove non-portable "globaldef".  Storage is defined in this module.
 *  All other references to decterm_version must use "extern"
 */
#ifdef VAXC
#define external globalref
#define externaldef globaldef
#else
#define external extern
#define externaldef
#endif

externaldef char decterm_version[] = "V2.0";


#ifdef sun   /* On Sun, __DATE__ and __TIME__ seem to not be defined (why?) */
externaldef char build_date[] = "June 1992";
externaldef char build_time[] = "";
#else
externaldef char build_date[] = __DATE__;
externaldef char build_time[] = __TIME__;
#endif
