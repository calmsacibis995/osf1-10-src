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
/*
 * @(#)$RCSfile: scu.h,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/11/23 23:08:57 $
 */
/************************************************************************
 *									*
 *			Copyright (c) 1991 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any	other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   This software is  derived	from  software	received  from	the	*
 *   University    of	California,   Berkeley,   and	from   Bell	*
 *   Laboratories.  Use, duplication, or disclosure is	subject  to	*
 *   restrictions  under  license  agreements  with  University  of	*
 *   California and with AT&T.						*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or	reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

/*
 * File:	scu.h
 * Author:	Robin T. Miller
 * Date:	August 9, 1991
 *
 * Description:
 *	Common include file used by SCSI Utility Program.
 *
 * Modification History:
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/file.h>
#include <sys/types.h>

/*
 * I'm not sure which predefined name to key off of for prototype
 * functions, so I'll define my own here.
 */
#if defined(__STDC__) || defined(__DECC) || defined(__mips)
#define PROTOTYPE
#endif

#include "parser.h"
#include "scu_global.h"

/*
 * Local Defines:
 */
#define DEFAULT_DEVICE	"/dev/cam"		/* Default device name.	*/
#define DEV_UAGT	"UAGT"			/* CAM User Agent driver*/

#define SUCCESS		PC$_SUCCESS		/* Success status code. */
#define FAILURE		PC$_FAILURE		/* Failure status code. */
#define WARNING		PC$_WARNING		/* Warning status code.	*/
#define CONTINUE	PC$_WARNING		/* Continue w/ SUCCESS.	*/
#define END_OF_FILE	PC$_WARNING		/* End of file status.	*/
#define TRUE		1			/* Boolean TRUE value.	*/
#define DNL		0			/* Disable newline.	*/
#define PNL		1			/* Print newline.	*/
#define FALSE		0			/* Boolean FALSE value.	*/
#define HARD		1			/* For ResetDeviceEntry */
#define SOFT		0			/* level to perform	*/
#define FATAL_ERROR	255			/* Fatal error code.	*/
#define	equal(s1,s2)	(strcmp(s1, s2) == 0)	/* Compare two strings.	*/
#define CD_BLOCK_SIZE	2048			/* CD-ROM block size.	*/
#define LEAD_OUT_TRACK	0xAA			/* Lead-out track number*/

#define MAX_PCF_TYPES		4		/* Maximum PCF types.	*/
#define MAX_MODE_PAGES		0x3f		/* Maximum mode pages.	*/

#define NEXUS_NOT_SET		0xff		/* Nexus field not set.	*/
#if defined(__alpha)
#define MAX_SCSI_BUS		15		/* Max SCSI bus number.	*/
#else /* !defined(__alpha) */
#define MAX_SCSI_BUS		3		/* Max SCSI bus number.	*/
#endif /* defined(__alpha) */
#define MAX_SCSI_TARGET		7		/* Max SCSI target #.	*/
#define MAX_SCSI_LUN		7		/* Max SCSI lun number.	*/

#define ARGS_BUFFER_SIZE	32768		/* Input buffer size.	*/
#define ARGV_BUFFER_SIZE	4096		/* Maximum arguments.	*/
#define BUFFER_SIZE		4096		/* Other buffer sizes.	*/
#define DATA_BUFFER_SIZE	4096		/* Data buffer size.	*/
#define IOP_BUFFER_SIZE		256		/* I/O parameters size.	*/
#define STRING_BUFFER_SIZE	256		/* String buffer size.	*/
#define DEC_BLOCK_SIZE		512		/* DEC block size.	*/
#define SCU_BUFFER_SIZE		2048		/* Work buffer size.	*/
#define MODE_PAGES_SIZE		255		/* Mode pages buf size.	*/

#define ABSOLUTE_ADDRESS	0		/* Absolute address.	*/
#define RELATIVE_ADDRESS	1		/* Track relative addr.	*/

#define BLOCK_SIZE		512		/* Bytes in block.	*/
#define KBYTE_SIZE		1024		/* Kilobytes value.	*/
#define MBYTE_SIZE		1048576L	/* Megabytes value.	*/
#define GBYTE_SIZE		1073741824L	/* Gigabytes value.	*/
#define DEFAULT_BLOCK_SIZE	512		/* Default block size.	*/
#define DEFAULT_ERROR_LIMIT	10		/* Default error limit.	*/
#define DEFAULT_PASS_LIMIT	1		/* Default pass count.	*/
#define DEFAULT_RETRY_LIMIT	25		/* Default retry limit.	*/
#define DEFAULT_PATTERN		0x39c39c39L	/* Default data pattern	*/

/*
 * Definitions for Direct Access Verify Command:
 */
#define MIN_VERIFY_LENGTH	1		/* Minimum block length	*/
#define MAX_VERIFY_LENGTH	0xffff		/* Maximum block length	*/
#define VERIFY_LENGTH       MAX_VERIFY_LENGTH	/* Default length.	*/

/*
 * Definitions for Sequential Access Read/Write Commands:
 */
#define MAX_SEQ_DATA_LENGTH	0xffffff	/* Maximum data length.	*/

/*
 * Definitions for Sequential Access Verify Command:
 */
#define MIN_VERIFY_TAPE_LENGTH	1		/* Minimum tape length.	*/
#define MAX_VERIFY_TAPE_LENGTH	0xffffff	/* Maximum tape length.	*/
#define VERIFY_TAPE_LENGTH  MAX_VERIFY_TAPE_LENGTH /* Default length.	*/

/*
 * Definitions for Read & Write Buffer Commands:
 */
#define DEFAULT_BUFFER_ID	0		/* The buffer ID.	*/
#define DEFAULT_BUFFER_MODE	0		/* The buffer mode.	*/
					 /* 0=ALL_RDBUF_COMBINED_HEADER	*/
#define DEFAULT_BUFFER_OFFSET	0		/* The buffer offset.	*/
#define DEFAULT_SEGMENT_SIZE   (8 * KBYTE_SIZE)	/* The download size.	*/

#define EQ(x,y)		(strcmp(x,y) == 0)	/* String EQ compare.	*/
#define EQL(x,y,n)	(strncmp(x,y,n) == 0)	/* Compare w/length.	*/
#define EQC(x,y)	(strcasecmp(x,y) == 0)	/* Case insensitive.	*/
#define EQLC(x,y,n)	(strncasecmp(x,y,n) == 0) /* Compare w/length.	*/

#define NE(x,y)		(strcmp(x,y) != 0)	/* String NE compare.	*/
#define NEL(x,y,n)	(strncmp(x,y,n) != 0)	/* Compare w/length.	*/
#define NEC(x,y)	(strcasecmp(x,y) != 0)	/* Case insensitive.	*/
#define NELC(x,y,n)	(strncasecmp(x,y,n) != 0) /* Compare w/length.	*/

#define EQS(x,y)	(strstr(x,y) != NULL)	/* Sub-string equal.	*/
#define EQSC(x,y)	(strcasestr(x,y) != NULL) /* Case insensitive.	*/

#define NES(x,y)	(strstr(x,y) == NULL)	/* Sub-string not equal	*/
#define NESC(x,y)	(strcasestr(x,y) == NULL) /* Case insensitive.	*/

/*
 * Strings used by Common Printing Functions. --> Make User Setable <--
 */
#define ASCII_FIELD	"%33.33s: %s"
#define EMPTY_FIELD	"%35.35s%s"
#define NUMERIC_FIELD	"%33.33s: %d"
#define HEX_FIELD	"%33.33s: %#x"

#define BIN_RADIX	0		/* Binary radix.		*/
#define DEC_RADIX	1		/* Decimal radix.		*/
#define HEX_RADIX	2		/* Hexidecimal radix.		*/
#define OCT_RADIX	3		/* Octal radix.			*/
