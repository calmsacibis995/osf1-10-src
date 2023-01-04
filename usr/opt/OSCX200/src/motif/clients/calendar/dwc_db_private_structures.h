#ifndef _dwc_db_private_structures_h_
#define _dwc_db_private_structures_h_
/* $Header$ */
/* DWC_DB_PRIVATE_STRUCTURES.H "V3.0-002" */
/*
**  Copyright (c) Digital Equipment Corporation, 1990
**  All Rights Reserved.  Unpublished rights reserved
**  under the copyright laws of the United States.
**  
**  The software contained on this media is proprietary
**  to and embodies the confidential technology of 
**  Digital Equipment Corporation.  Possession, use,
**  duplication or dissemination of the software and
**  media is authorized only pursuant to a valid written
**  license from Digital Equipment Corporation.
**
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or 
**  disclosure by the U.S. Government is subject to
**  restrictions as set forth in Subparagraph (c)(1)(ii)
**  of DFARS 252.227-7013, or in FAR 52.227-19, as
**  applicable.
**++
**  FACILITY:
**
**	DECwindows Calendar; database access routines
**
**  AUTHOR:
**
**	Per Hamnqvist, September 1988
**
**  ABSTRACT:
**	This module includes the other main structure modules used to
**	build the datastructures that are internal to the DECwindows
**	Calendar database.
**
**  ENVIRONMENT:
**
**	User mode, executable image.
**
**
**  MODIFICATION HISTORY:
**
** V3.0-002 Paul Ferwerda					21-Nov-1990
**		Got rid of the sdl file. Maintainers have to edit this directly
**		now. This was done to make is easier to build Ultrix versions
**		while nfsed to a cms reference copy.
**
**	V1-000	Per Hamnqvist					07-Sep-1988
**		Module created.
**--
*/

/*
**  Include Files
*/
#include "dwc_db_record_structures.h"
#include "dwc_db_work_structures.h"

#endif /* end of _dwc_db_private_structures_h_ */
