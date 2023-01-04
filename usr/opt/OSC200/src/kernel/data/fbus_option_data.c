/*
 * *****************************************************************
 * *                                                               *
 * *    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
 * *                                                               *
 * *   All Rights Reserved.  Unpublished rights  reserved  under   *
 * *   the copyright laws of the United States.                    *
 * *                                                               *
 * *   The software contained on this media  is  proprietary  to   *
 * *   and  embodies  the  confidential  technology  of  Digital   *
 * *   Equipment Corporation.  Possession, use,  duplication  or   *
 * *   dissemination of the software and media is authorized only  *
 * *   pursuant to a valid written license from Digital Equipment  *
 * *   Corporation.                                                *
 * *                                                               *
 * *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
 * *   by the U.S. Government is subject to restrictions  as  set  *
 * *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
 * *   or  in  FAR 52.227-19, as applicable.                       *
 * *                                                               *
 * *****************************************************************
 */
/*
 * HISTORY
 */
#ifndef lint
static char *rcsid = "@(#)$RCSfile: fbus_option_data.c,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/04/23 20:43:18 $";
#endif
#include <io/dec/fbus/fbusreg.h>

     /*
	Fbus_option table fields:
	Fields 1, 2, and 3 are used to recognize fbus_options.
	Field 1: Module_Vendor_Id
	Field 2: Module_Hw_Version
	Field 3: Module_Sw_Version, or Node_Sw_Version, or Unit_Sw_Version
	         then it is used in matching the fbus_option
	         to an entry in this table.
	Field 4: Module Name (not used in matching)
	Field 5: driver name (not used in matching)
	         This is the same name as used in 'config' file.
	Field 6: 'C' for controller or device
		 'A' for adapter
	Field 7: Adapter config function
		 Called when field 6 is 'A'.
     */

struct fbus_option Fbus_option[] =
{
    { 0x08002b, 0x08002b, 0x000000, "B2102-AA", "fbe", 'C', 0 },
    { 0x08002b, 0x0186b0, 0x0186b0, "B2006-AA", "faa", 'C', 0 },
    /*
     * Do not delete any table entries above this line or your system
     * will not configure properly.
     *
     * Add any new controllers or devices here.
     * Remember, the module name must be blank padded to 8 bytes.
     */

    /*
     * Do not delete this null entry, which terminates the table or your
     * system will not configure properly.
     */
    {  0, 0, 0, "", "", '\0', 0},
};
