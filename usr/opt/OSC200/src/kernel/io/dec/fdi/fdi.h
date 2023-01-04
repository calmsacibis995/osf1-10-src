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
/*
static char *rcsid = "@(#)$RCSfile: fdi.h,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/10/13 13:02:35 $";
*/
#endif

/************************************************************************
 *									*
 *			Copyright (c) 1991 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any  other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

/************************************************************************
 *
 * File:        fdi.h
 * Date:        14-Aug-1991
 * Author:      Roger Morris
 *
 * Description:
 *      fdi.h -- Header file for use with fd device driver.
 *
 * Modification History:
 *    Version     Date          Who     Reason
 *
 *      1.00    14-Aug-91       rsm     Creation date.  
 *
 *              15-Oct-91       rsm     Added ``fill'' element to struct
 *                                       fd_fmt_spec to allow a more efficient
 *                                       -fsdos parameter to be added to 
 *                                       fddisk in the future.
 *
 *              15-Oct-91       rsm     Added more elements to struct
 *                                       fd_sense and fd_mt to allow fddisk
 *                                       to report more information.
 *
 *              15-Oct-91       rsm     Changed magic number because of above
 *                                       changes.
 */

#ifndef FDI_INCLUDE
#define FDI_INCLUDE   1


/* ************************************************************************* */
/* The names of all symbols defined or declared here begin with the string   */
/* "fd_" or "FDIO".  In addition, all external symbols resulting from the    */
/* fd driver code begin with either "fd_" or "fdcam_".                       */
/* ************************************************************************* */



/* ************************************************************************* */

enum fd_drive_type                     /* Specifies type of drive            */
    {
    FD_DT_NO_DRIVE,
    FD_DT_RX26,
    FD_DT_RX23,
    FD_DT_RX33
    };

enum fd_media_type                     /* Specifies type of media in drive   */
    {
    FD_MT_AUTOSENSE,                   /* Not really a type.  This value     */
                                       /*  tells the various format commands */
                                       /*  to determine the media type.      */
    FD_MT_NO_MEDIA,
    FD_MT_350DD,
    FD_MT_350HD,
    FD_MT_350ED,
    FD_MT_525DD,
    FD_MT_525HD,
    FD_MT_RX50
    };

struct fd_mt
    {
    long n_sectors;
    short sector_size;
    short n_surfaces;
    short n_cylinders;
    enum fd_media_type media;
    short partition_start;             /* Starting PSN for current partition.*/
    short partition_end;               /* Ending PSN + 1 for current part.   */
                                       /*  If partition_end<partition_start  */
                                       /*  then partition data is unknown.   */
    short spare1;
    };

enum fd_media_id
    {
    FD_MID_NO_MEDIA,
    FD_MID_CANNOT_DETERMINE,
    FD_MID_DD,
    FD_MID_HD,
    FD_MID_ED
    };

struct fd_fmt_spec
    {
    struct fd_mt actual;               /* Written by FDIOFMTDSK,FDIOFFBSETUP.*/
    enum fd_media_type media;          /* Read by FDIOFMTDSK, FDIOFFBSETUP.  */
    char is_complete;                  /* Written by FDIOFMTDSK, FDIOFFBNEXT.*/
                                       /*  Init to zero by FDIOFFBSETUP.     */
    char percent_complete;             /*  "                                 */
    char media_error;                  /* Written by FDIOFMTDSK, FDIOFFBNEXT,*/
                                       /*  FDIOFFBSETUP.                     */
    char disable_dlock;                /* This should typically be set to    */
                                       /*  zero.  If zero, the current       */
                                       /*  drive is locked until the format  */
                                       /*  is complete.  If zero, however,   */
                                       /*  the FDIOFFBSETUP will fail if     */
                                       /*  the drive is in use.   This       */
                                       /*  parameter is only used by the     */
                                       /*  FDIOFFBSETUP and FDIOFFBNEXT      */
                                       /*  commands.                         */
    short next_cylinder;
    short next_surface;
    short interleave;                  /* <= 1 for no interleave.            */
    short cylinder_interleave;         /* cylinder-to-cylinder additional    */
                                       /*  interleave.  typ: 0.              */
    short start_position;              /* Sector slot number in which to     */
                                       /*  place the first logical sector.   */
                                       /*  typ: 0.                           */

    short fill;                        /* <0 to use default fill.            */
    };

enum fd_seek_stat
    {
    FD_SEEK_NORMAL,                    /* All is OK.  It can be assumed that */
                                       /*  this entry will always evaluate   */
                                       /*  to zero.                          */
    FD_SEEK_SID_MISMATCH,              /* Sector-ID mismatch.  This does not */
                                       /*  apply to mode FD_SEEK_PCPHS.      */
    FD_SEEK_NO_SECTOR,                 /* Requested sector not found at      */
                                       /*  specified location.               */
    FD_SEEK_DATA_ERROR,                /* A data error was encountered.      */
    FD_SEEK_OOR                        /* Requested location out of range.   */
    };

#define FD_MAGIC 0x49543348L

struct fd_sense
    {
    long magic;
    enum fd_drive_type drive_type;
    enum fd_media_id media_id;
    char tamper_possible;
    char new_media;
    char write_protected;
    int  drive_nopen;
    long version;                      /* Internal driver version code.      */
    int  spare1;                       /* Initialized to zero for now.       */
    };

enum fd_seek_mode
    {
    FD_SEEK_NOT_USED,                  /* Special code, used internally.     */
    FD_SEEK_PSN,                       /* Address sector via PSN (Physical   */
                                       /*  Sector Number).                   */
    FD_SEEK_LBN,                       /* Address sector via LBN (Logical    */
                                       /*  Block Number).  In this implemen- */
                                       /*  tation, this is the same as PSN.  */
    FD_SEEK_CHS,                       /* Address sector via Cylinder/Head/  */
                                       /*  Sector numbers.                   */
    FD_SEEK_PCPHS                      /* This is the same as FD_SEEK_CHS,   */
                                       /*  but that the data in the sector   */
                                       /*  ID field need not match except    */
                                       /*  for the sector number.            */
    };

struct fd_chs_psn
    {
    unsigned long psn;                 /* Written by FDIOMKPSN, Read by      */
                                       /*  FDIOMKCHS                         */
    unsigned short cylinder;           /* Written by FDIOMKCHS, Read by      */
                                       /*  FDIOMKPSN                         */
    unsigned short head;               /*  "                                 */
    unsigned short sector;             /*  "                                 */
    };

struct fd_seek
    {
    struct fd_chs_psn where;           /* Which sector to seek to.           */
    enum fd_seek_mode mode;            /* Indicates how the sector is        */
                                       /*  addressed.                        */
    char disable_plock;                /* This should typically be set to    */
                                       /*  zero.  If zero, the current       */
                                       /*  partition is locked until the     */
                                       /*  indicated sector is read or       */
                                       /*  written.  If zero, however, the   */
                                       /*  FDIOSEEK command will fail if     */
                                       /*  the partition is in use.          */
    };


/* ************************************************************************* */
/* fd i/o controls                                                           */
/* ************************************************************************* */

#define FDIO_X 'y'
#define FDIOSENSE    _IOR ( FDIO_X,  1, struct fd_sense )
#define FDIOGETMTYP  _IOR ( FDIO_X,  2, struct fd_mt )
#define FDIOFMTDSK   _IOWR( FDIO_X,  3, struct fd_fmt_spec )
#define FDIOFFBSETUP _IOWR( FDIO_X,  4, struct fd_fmt_spec )
#define FDIOFFBNEXT  _IOWR( FDIO_X,  5, struct fd_fmt_spec )
#define FDIOFMTTRK   _IOWR( FDIO_X, 14, struct fd_fmt_spec )
#define FDIOTPRRST   _IOR ( FDIO_X,  6, struct fd_sense )
#define FDIOCHKMED   _IOR ( FDIO_X,  7, long )   /* -1 ==good */
#define FDIOSEEK     _IOWR( FDIO_X,  8, struct fd_seek )
#define FDIOMKCHS    _IOWR( FDIO_X, 10, struct fd_chs_psn )
#define FDIOMKPSN    _IOWR( FDIO_X, 11, struct fd_chs_psn )
/* #define FDIOCTST     _IOWR( FDIO_X, 13, struct fd_ioctst_struct )         */
/*      DEVIOCGET    is defined in ioctl.h                                   */
/*      DEVGETGEIOM  is defined in ioctl.h                                   */


#endif /* FDI_INCLUDE */

