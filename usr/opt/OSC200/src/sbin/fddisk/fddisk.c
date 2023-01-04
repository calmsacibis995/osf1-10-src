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
static char *rcsid = "@(#)$RCSfile: fddisk.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/10/13 13:12:35 $";
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
 * File:        fddisk.c
 * Date:        12-Aug-1991
 * Author:      Roger Morris
 *
 * Modification History:
 *    Version     Date          Who     Reason
 *
 *      1.00    12-Aug-91       rsm     Creation date.  
 *	2.00	27-Aug-91	rsm	Fixed problems with formatting.
 *	        30-Oct-91	rsm	Fixed file-system-install bug.
 *	        30-Oct-91	rsm	Update comments.
 *
 */

/* ************************************************************************* */
/* fddisk.c -- fd disk control program.                                      */
/* ************************************************************************* */

#define DEBUG 1

#define roger 0

#ifdef OSF
# include <io/dec/fdi/fdi.h>
# include <sys/ioctl.h>
# include "fd_fsdef.h"
# if DEBUG
#  include <io/dec/fdi/fdioctst.h>
# endif
#else
# if roger
#  include "../sys/io/fd/mips/fdi.h"
#  include <sys/ioctl.h>
#  include "../sys/io/fd/mips/fd_fsdef.h"
#  if DEBUG
#   include "../sys/io/fd/mips/fdioctst.h"
#  endif
# else
#  include <io/fd/mips/fdi.h>
#  include <sys/ioctl.h>
#  include <io/fd/mips/fd_fsdef.h>
#  if DEBUG
#   include <io/fd/mips/fdioctst.h>
#  endif
# endif
#endif

#include <stdio.h>
#include <strings.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>

#define SECTOR_SIZE 512

int do_silent = 0;
static void smsg( char* msg, char* opt )
    {
    if ( !do_silent )
	fprintf( stderr, msg, opt );
    }

/* ************************************************************************* */

static struct fsdat_struct* p;
unsigned long last = 0;

struct lbls { struct lbls* next; long sector; long offset; unsigned char c; };
struct lbls *first_lbls;

/* Return non-zero if bad label or labels not supported for requested        */
/*  file system.                                                             */

int init_fill_buf( struct fsdat_struct* gp, unsigned char* lbl )
    {
    struct lbls** lpp = &first_lbls;

    p = gp;
    last = 0;
    first_lbls = 0;

    while ( p->offset < -1 )
	{
	if ( p->offset == -2 )
	    {
	    unsigned char* cp = lbl;
	    int i;
	    for ( i = 0 ; i < p->data[2] ; i++ )
		{
		*lpp = malloc(sizeof(**lpp));
		(*lpp)->next = 0;
		(*lpp)->sector = p->data[0];
		(*lpp)->offset = p->data[1]+i;
                (*lpp)->c = (cp && *cp) ? *cp++ : p->data[3];
		lpp = &(*lpp)->next;
		}
	    if ( cp && *cp )
		return 1;
	    }
	p++;
	}
    if ( lbl && *lbl && !first_lbls )
	return 1;
    return 0;
    }

/* Return x if sector for-sure filled entirely with character x, return -1   */
/*  if sector probably filled with varying data.                             */

int fill_buf( unsigned char* buf, unsigned long sector )
    {
    int offset;
    int v = -1;

    for ( ; p->offset >= 0 && p->sector < sector ; p++ )
	if ( p->data[4] == p->data[3] )
	    last = p->data[3];

    while ( first_lbls && first_lbls->sector < sector )
	first_lbls = first_lbls->next;

    if ( sector<p->sector && (!first_lbls||sector<first_lbls->sector) )
	{
	for ( offset = 0 ; offset < SECTOR_SIZE ; offset++ )
	    *buf++ = last;
	return last;
	}
    else
	{
	for ( offset = 0 ; offset < SECTOR_SIZE ; offset += 16 )
	    {
	    int i, j;
	    if ( p->offset == offset && p->sector == sector )
		{
		for ( i = 0 ; i < 4 ; i++ )
		    for ( j = 0 ; j < 4 ; j++ )
			{
			if ( first_lbls && sector == first_lbls->sector
				&& offset+i*4+j == first_lbls->offset )
			    {
			    *buf++ = first_lbls->c;
			    first_lbls = first_lbls->next;
			    }
			else
			    *buf++ = p->data[i]>>24-j*8;
			}
		if ( p->data[4] == p->data[3] )
		    last = p->data[3];
		p++;
		}
	    else
		for ( i = 0 ; i < 4 ; i++ )
		    for ( j = 0 ; j < 4 ; j++ )
			{
			if ( first_lbls && sector == first_lbls->sector
				&& offset+i*4+j == first_lbls->offset )
			    {
			    *buf++ = first_lbls->c;
			    first_lbls = first_lbls->next;
			    }
			else
			    *buf++ = last;
			}
	    }
	return -1;
	}
    }

/* ************************************************************************* */

int please_wait
	(
	int fd,
	int wait_mode,                 /* 0 == fail if no media present.     */
                                       /* 1 == wait for new media if there   */
                                       /*       is no media or if the current*/
                                       /*       media is not new since the   */
                                       /*       previous open of the device. */
                                       /* 2 == wait for media present.       */
                                       /* 3 == wait for media present.  If   */
                                       /*       there is currently media in  */
                                       /*       the drive, first wait for it */
                                       /*       to be removed.               */
	long duration,
	struct fd_sense* fds
	)
    {
    long t;
    long endt;

    time(&endt);
    endt += duration;

    if ( fds->media_id != FD_MID_NO_MEDIA )
	{
	if ( wait_mode == 0 )
	    return 0;
	if ( wait_mode == 2 || wait_mode==1 && fds->new_media )
	    return smsg("Disk is now present.\n",0), 0;

	smsg("Waiting for media to be removed.\n",0);

	for(;;)
	    {
	    if ( ioctl(fd,FDIOTPRRST,fds) )
		return fprintf(stderr,"(error 4)\n"), 2;
	    if ( fds->tamper_possible )
		{
		smsg("\nDisk has been removed.\n",0);
		break;
		}
	    smsg(".",0);
	    time(&t);
	    if ( duration >= 0 && t > endt )
		return smsg("\nTimeout waiting for disk to be removed.\n",0),2;
	    sleep(1);
	    }
	}

    if ( wait_mode == 0 )
	return smsg("No media in the drive.\n",0), 1;

    for(;;)
	{
	if ( fds->media_id != FD_MID_NO_MEDIA )
	    return smsg("\nDisk has been inserted.\n",0), 0;
	smsg(".",0);
	time(&t);
	if ( duration >= 0 && t > endt )
	    return smsg("\nTimeout waiting for disk to be inserted.\n",0), 2;
	sleep(1);
	if ( ioctl(fd,FDIOTPRRST,fds) )
	    return fprintf(stderr,"(error 5)\n"), 2;
	}
    }


int please_new( int fd, struct fd_sense* fds )
    {
    if ( ioctl(fd,FDIOTPRRST,fds) )
	return fprintf(stderr,"(error 4.5)\n"), 2;
    else
	return 0;
    }


int please_check_not_in_use( struct fd_sense* fds )
    {
    if ( fds->drive_nopen > 1 )
	return smsg( "Device in use by another process.\n",0 ), 1;
    else
	return 0;
    }

int please_check_nowp( struct fd_sense* fds )
    {
    if ( fds->write_protected )
	return smsg( "Media is write protected.\n",0 ), 1;
    else
	return 0;
    }

int please_check_new( int ignored, struct fd_sense* fds )
    {
    if ( fds->new_media )
	return 0;
    else
	return smsg( "Media was not changed since last open.\n",0), 1;
    }


char* lookup_dt( enum fd_drive_type dt )
    {
    switch( dt )
	{
	case FD_DT_NO_DRIVE:
	    return "NO DRIVE";
	case FD_DT_RX26:
	    return "RX26";
	case FD_DT_RX23:
	    return "RX23";
	case FD_DT_RX33:
	    return "RX33";
	default:
	    return "CANNOT DETERMINE DRIVE TYPE";
	}
    }


char* lookup_id( enum fd_media_id mt )
    {
    switch( mt )
	{
	case FD_MID_NO_MEDIA:
	    return "NO MEDIA ID";
	case FD_MID_DD:
	    return "DD";
	case FD_MID_HD:
	    return "HD";
	case FD_MID_ED:
	    return "ED";
	default:
	case FD_MID_CANNOT_DETERMINE:
	    return "CANNOT DETERMINE MEDIA ID";
	}
    }


char* lookup_media( enum fd_media_type mt )
    {
    switch( mt )
	{
	case FD_MT_350DD:
	    return "3.50 inch, DD  (720KB)";
	case FD_MT_350HD:
	    return "3.50 inch, HD  (1.44MB)";
	case FD_MT_350ED:
	    return "3.50 inch, ED  (2.88MB)";
	case FD_MT_525DD:
	    return "5.25 inch, DD  (720KB)";
	case FD_MT_525HD:
	    return "5.25 inch, HD  (1.2MB)";
	case FD_MT_RX50:
	    return "5.25 inch, RX50";
	case FD_MT_NO_MEDIA:
	    return "NO MEDIA PRESENT";
	default:
	    return "CANNOT DETERMINE MEDIA TYPE";
	}
    }

static int qr( int fd, long sector )
    {
    char sector_buf[SECTOR_SIZE];

    if ( lseek(fd,sector*SECTOR_SIZE,SEEK_SET) < 0 )
	return smsg("ERROR 9.\n",0), 1;
    else if ( read(fd,sector_buf,SECTOR_SIZE) != SECTOR_SIZE )
	return smsg("Error while reading sector %d.\n", (char*)sector ), 1;
    else
	return 0;
    }

int please_format( int fd, enum fd_media_type format, int interleave, int iti,
	int do_force )
    {
    struct fd_fmt_spec arg;
    arg.disable_dlock = do_force ? 1 : 0;
    arg.interleave = interleave;
    arg.cylinder_interleave = iti;
    arg.fill = -1;
    arg.media = format;
    arg.media_error = 1;
    if ( do_silent )
	{
	ioctl(fd,FDIOFMTDSK,&arg);
	return arg.media_error;
	}
    else
	{
	ioctl(fd,FDIOFFBSETUP,&arg);
	fprintf( stderr, "Disk type: %s\n",lookup_media(arg.actual.media));
	fprintf( stderr, "Number of sectors per track: %ld\n",
		arg.actual.n_sectors );
	fprintf( stderr, "Number of surfaces: %3ld\n",
		arg.actual.n_surfaces );
	fprintf( stderr, "Number of cylinders:%3ld\n",
		arg.actual.n_cylinders );
	fprintf( stderr, "Sector size:  %ld\n", arg.actual.sector_size );
	if ( interleave > 1 )
	    fprintf(stderr,"interleave factor:%3ld:%d\n",(long)arg.interleave,
		    iti );
	fprintf( stderr, "Formatting disk...\n" );
	fprintf(stderr,"  Percentage complete:   0%\10\10\10\10");
        while ( !arg.is_complete && !arg.media_error )
	    {
	    ioctl(fd,FDIOFFBNEXT,&arg);
	    fprintf(stderr,"%3d%% (head=%1d, cyl=%3d)%s",
		    (int)arg.percent_complete, (int)arg.next_surface,
		    (int)arg.next_cylinder,
		    "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b" );
	    }
	if ( arg.media_error )
	    return smsg(" Media error.  Disk not formatted.\n",0), 1;
	else
	    {
            struct fd_sense sense;
	    ioctl(fd,FDIOTPRRST,&sense);

	    smsg(" Format complete, checking...\n",0);
	    if (       qr(fd,arg.actual.n_sectors*1-1)
		    || qr(fd,arg.actual.n_sectors*1)
		    || qr(fd,arg.actual.n_sectors*2-1)
		    || qr(fd,arg.actual.n_sectors*2)
		    || qr(fd,arg.actual.n_sectors*3-1)
		    || qr(fd,arg.actual.n_sectors*3)
		    || qr(fd,arg.actual.n_sectors*arg.actual.n_surfaces
		    *arg.actual.n_cylinders-1)
		    || qr(fd,0)  )
    	        return smsg(" Unable to read disk\n",0), 1;
	    else
    	        return smsg(" Quick check of disk passes OK.\n",0), 0;
	    }
	}
    }

int please_dl( int fd )
    {
    unsigned char sector_buf[SECTOR_SIZE];
    long psn;
    int i;
    struct fd_mt mtype;
    struct fd_chs_psn trans;
    long tot_size;

    for ( i = 0 ; i < SECTOR_SIZE ; i++ )
	sector_buf[i] = i;

    ioctl(fd,FDIOGETMTYP,&mtype);
    tot_size = (long)mtype.n_surfaces*mtype.n_cylinders*mtype.n_sectors;

    if ( mtype.sector_size != SECTOR_SIZE )
	return smsg("ERROR 2.\n",0),1;

    smsg( "Installing diagnostic label on each sector...\n", 0 );

    for( psn = 0 ; psn < tot_size ; psn++ )
	{
	trans.psn = psn;
	if ( ioctl(fd,FDIOMKCHS,&trans) )
	    return smsg("ERROR 9.\n",0),1;
	sprintf( (char*)(sector_buf+ 0), "PSN = %7ld  \n", psn );
	sprintf( (char*)(sector_buf+16), "Cyl = %7ld  \n", trans.cylinder );
	sprintf( (char*)(sector_buf+32), "Head = %6ld  \n", trans.head );
	sprintf( (char*)(sector_buf+48), "Sector = %4ld  \n", trans.sector );
	sprintf( (char*)(sector_buf+64), "***************\n" );
	if ( lseek(fd,psn*SECTOR_SIZE,SEEK_SET) < 0 )
	    smsg( "Unable to seek to sector %ld.\n", (char*)psn );
	else
	    {
	    if ( write(fd,sector_buf,SECTOR_SIZE) != SECTOR_SIZE )
		smsg( "Unable to write to sector %ld.\n", (char*)psn );
	    }
	}

    smsg("diagnostic labels installed.\n",0);

    return 0;
    }

enum install_fs
    {
    IFS_NONE,
    IFS_UFS,
    IFS_MSDOS,
    IFS_SCO,
    IFS_DIAGNOSTIC
    };

int please_install( int fd, enum install_fs fs_type, unsigned char* label )
    {
    unsigned char sector_buf[SECTOR_SIZE];
    struct fsdat_struct* dip = 0;
    long n_sectors;
    long psn;
    char* cp = "&";
    struct fd_mt mtype;
    long tot_size;

    if ( fs_type == IFS_DIAGNOSTIC )
	return please_dl(fd);

    ioctl(fd,FDIOGETMTYP,&mtype);

    tot_size = (long)mtype.n_surfaces*mtype.n_cylinders*mtype.n_sectors;

    if ( mtype.sector_size != SECTOR_SIZE )
	return smsg("ERROR 2.\n",0),1;

    switch( fs_type )
	{
	case IFS_UFS:
	    cp = "Ultrix";
	    dip = mtype.media==FD_MT_350DD ? dd_ufs :
		  mtype.media==FD_MT_350HD ? hd_ufs :
		  mtype.media==FD_MT_350ED ? ed_ufs :
		  dip;
	    break;
	case IFS_MSDOS:
	    cp = "MS-DOS";
	    dip = mtype.media==FD_MT_350DD ? dd_msdos :
		  mtype.media==FD_MT_350HD ? hd_msdos :
		  mtype.media==FD_MT_350ED ? ed_msdos :
		  dip;
	    break;
	case IFS_SCO:
	    cp = "SCO";
	    dip = mtype.media==FD_MT_350DD ? dip /* dd_sco */ :
		  mtype.media==FD_MT_350HD ? dip /* hd_sco */ :
		  mtype.media==FD_MT_350ED ? dip /* ed_sco */ :
		  dip;
	    break;
	default:
	case IFS_NONE:
	    /* do nothing */;
	}
    if ( !dip )
	return smsg("Invalid file-system spec.\n",0), 1;
    smsg("Installing %s file system...\n", cp);

    /* arg.user_buffer = sector_buffer; */

    if ( init_fill_buf( dip, label ) )
	return smsg("Invalid label for file system.\n",0), 1;
	
    for( psn = 0 ;; psn++ )
	{
	fill_buf( sector_buf, psn );
	if ( lseek(fd,psn*SECTOR_SIZE,SEEK_SET) < 0 )
	    break;
	if ( write(fd,sector_buf,SECTOR_SIZE) != SECTOR_SIZE )
	    break;
	}

    if ( psn < tot_size ) 
	return smsg("Error, file system not installed on all sectors.\n",0), 1;

    smsg("File system installed.\n",0);

    return 0;
    }

/* The following is a high-speed combination of please_format and            */
/*  please_install.                                                          */

int please_make( int fd, enum fd_media_type format, int interleave, int iti,
	int do_force, enum install_fs fs_type, unsigned char* label )
    {
    unsigned char* trk_buf;
    struct fsdat_struct* dip = 0;
    long psn;
    char* cp = "&";
    long spt;
    struct fd_sense sense;

    struct fd_fmt_spec arg;
    arg.disable_dlock = 1;             /* Necessary because we are going     */
                                       /*  to write to the disk while we     */
                                       /*  format it.                        */
    arg.interleave = interleave;
    arg.cylinder_interleave = iti;
    arg.fill = -1;
    arg.media = format;
    arg.media_error = 1;

    ioctl(fd,FDIOTPRRST,&sense);
    ioctl(fd,FDIOFFBSETUP,&arg);

    spt = arg.actual.n_sectors;
    
    fprintf( stderr, "COMBINED FORMAT / FILE-SYSTEM INSTALL\n");
    fprintf( stderr, "Disk type: %s\n",lookup_media(arg.actual.media));
    fprintf( stderr, "Number of sectors per track: %ld\n", spt );
    fprintf( stderr, "Number of surfaces: %3ld\n",
	    arg.actual.n_surfaces );
    fprintf( stderr, "Number of cylinders:%3ld\n",
	    arg.actual.n_cylinders );
    fprintf( stderr, "Sector size:  %ld\n", arg.actual.sector_size );
    if ( interleave > 1 )
	fprintf(stderr,"interleave factor:%3ld:%d\n",(long)arg.interleave,
		iti );

    if ( arg.actual.sector_size != SECTOR_SIZE )
	return smsg("ERROR 2.\n",0),1;

    trk_buf = malloc( spt*SECTOR_SIZE );

    switch( fs_type )
	{
	case IFS_UFS:
	    cp = "Ultrix";
	    dip = arg.actual.media==FD_MT_350DD ? dd_ufs:
		  arg.actual.media==FD_MT_350HD ? hd_ufs:
		  arg.actual.media==FD_MT_350ED ? ed_ufs:
		  dip;
	    break;
	case IFS_MSDOS:
	    cp = "MS-DOS";
	    dip = arg.actual.media==FD_MT_350DD ? dd_msdos :
		  arg.actual.media==FD_MT_350HD ? hd_msdos :
		  arg.actual.media==FD_MT_350ED ? ed_msdos :
		  dip;
	    break;
	case IFS_SCO:
	    cp = "SCO";
	    dip = arg.actual.media==FD_MT_350DD ? dip /* dd_sco */ :
		  arg.actual.media==FD_MT_350HD ? dip /* hd_sco */ :
		  arg.actual.media==FD_MT_350ED ? dip /* ed_sco */ :
		  dip;
	    break;
	default:
	case IFS_NONE:
	    /* do nothing */;
	}
    if ( !dip )
	return smsg("Invalid file-system spec.\n",0), 1;

    if ( init_fill_buf( dip, label ) )
	return smsg("Invalid label for file system.\n",0), 1;
	
    fprintf( stderr, "Formatting disk and installing %s file system...\n",
	    cp );

    fprintf(stderr,"  Percentage complete:   0%\10\10\10\10");

    psn = 0;
    while ( !arg.is_complete && !arg.media_error )
	{
	int i;
	int k;

	arg.fill = fill_buf( trk_buf, psn );
	for ( i = 1 ; i < spt ; i++ )
	    {
	    if ( arg.fill != fill_buf( trk_buf+i*SECTOR_SIZE, psn+i ) )
		arg.fill = -1;
            }

	if ( arg.fill < 0 &&  lseek(fd,psn*SECTOR_SIZE,SEEK_SET) < 0 )
	    break;

	if ( ioctl(fd,FDIOFFBNEXT,&arg) )
	    break;

	if ( arg.fill<0 && write(fd,trk_buf,spt*SECTOR_SIZE)!=spt*SECTOR_SIZE )
	    break;

                                       /* Print head and cyl number for track*/
                                       /*  just formatted.                   */
	fprintf(stderr,"%3d%% %chead=%1d, cyl=%3d%c%s",
		(int)arg.percent_complete,
		arg.fill>=0?'(':'{',
		(int)arg.next_surface,
		(int)arg.next_cylinder,
		arg.fill>=0 ?')':'}',
		"\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");

	psn += spt;
	}

    if ( psn < arg.actual.n_sectors*arg.actual.n_surfaces
	    *arg.actual.n_cylinders )
	return smsg("Error, format/file-system-install halted at s:%ld\n",
		(char*)psn), 1;

    smsg(" Disk formatted and file system installed.  Checking...\n",0);

    ioctl(fd,FDIOTPRRST,&sense);

    if (       qr(fd,arg.actual.n_sectors*1-1)
	    || qr(fd,arg.actual.n_sectors*1)
	    || qr(fd,arg.actual.n_sectors*2-1)
	    || qr(fd,arg.actual.n_sectors*2)
	    || qr(fd,arg.actual.n_sectors*3-1)
	    || qr(fd,arg.actual.n_sectors*3)
	    || qr(fd,arg.actual.n_sectors*arg.actual.n_surfaces
	    *arg.actual.n_cylinders-1)
	    || qr(fd,0)  )
	return smsg(" Unable to read disk\n",0), 1;
    else
	return smsg(" Quick check of disk passes OK.\n",0), 0;

    }

int please_deposit( int fd, long sector, long end_sector )
    {
    unsigned char sector_buf[SECTOR_SIZE];
    int i, j;

    for ( ; sector <= end_sector ; sector++ )
	{
	for ( i = 0 ; i < SECTOR_SIZE ; i++ )
	    {
	    if ( (j = getchar()) == EOF )
		break;
	    sector_buf[i] = j;
	    }
	while ( i < SECTOR_SIZE )
	    sector_buf[i++] = 0;

	if ( lseek(fd,sector*SECTOR_SIZE,SEEK_SET) < 0 )
	    return smsg("ERROR 4.\n",0),1;
	if ( write(fd,sector_buf,SECTOR_SIZE) != SECTOR_SIZE )
	    return smsg("ERROR 4b.\n",0),1;
	}
    return smsg("Sector(s) written.\n",0),0;
    }

int please_examine( int fd, long sector, long end_sector, int is_hex )
    {
    unsigned char sector_buf[SECTOR_SIZE];
    int i, j, k, l;
    for ( ; sector <= end_sector ; sector++ )
	{
	if ( lseek(fd,sector*SECTOR_SIZE,SEEK_SET) < 0 )
	    return smsg("ERROR 5.\n",0),1;
	else if ( read(fd,sector_buf,SECTOR_SIZE) != SECTOR_SIZE )
	    return smsg("Error while reading.\n",0),1;

	if ( is_hex )
	    {
	    unsigned char* sp = sector_buf;
	    char ascii[20];
	    printf("Sector %ld ...\n",sector);
	    for ( i = 0 ; i < 32 ; i++ )
		{
		char* ap = ascii;
		printf("0x%04x: ",i*16);
		for ( j = 0 ; j < 4 ; j++ )
		    {
		    printf("  ");
		    for ( k = 0 ; k < 4 ; k++ )
			{
			*ap++ = (*sp<' '||*sp>'~')?'.':*sp;
			printf( "%02x", *sp++ );
			}
		    }
		*ap = 0;
		printf("  %s\n",ascii);
		}
	    }
	else
	    for ( i = 0 ; i < SECTOR_SIZE ; i++ )
		putchar(sector_buf[i]);
	}
    }

int please_pchd( int fd, int noascii, int noverbose,
	int nosdef, char* name, char* struct_name )
    {
    int do_verbose = !noverbose;
    long array_size = 0;
    long dat[4];
    long last = 0x00000000;
    unsigned char ascii[17];
    unsigned char sector_buf[SECTOR_SIZE];
    long sector = 0;

    if ( !nosdef )
	{
	printf("\n");
	printf("struct %s\n",struct_name);
	printf("    {\n");
	printf("    unsigned short sector;  /* Sector count if offset==-1*/\n");
	printf("    short offset;           /* -1 == EOL                 */\n");
	printf("    unsigned long data[4];  /* MSB first                 */\n");
	printf("    };\n");
	}
    printf("\n");
    printf("/* The following array of the above-declared structure     */\n");
    printf("/* represents the data on the entire disk, in order of     */\n");
    printf("/* increasing sector number and offset within that sector. */\n");
    printf("/* Where data is repeated, elements are omitted from the   */\n");
    printf("/* array.  The repeated data is equal to the data[3] of    */\n");
    printf("/* the most-recent element for which data[3]==data[2], or  */\n");
    printf("/* to 0x00000000 if no such entry was yet found.           */\n");
    printf("struct %s %s[] =\n",struct_name,name);
    printf("  {\n");

    for(;;)
	{
	int i, j, k, l;
	unsigned char* cp = sector_buf;

	if ( lseek(fd,sector*SECTOR_SIZE,SEEK_SET) < 0 )
	    return smsg("ERROR 7.\n",0),1;
	else if ( read(fd,sector_buf,SECTOR_SIZE) != SECTOR_SIZE )
	    {
	    printf( "    /*** Done at %ld sectors. ***/\n", sector );
	    break;
	    }

	for ( i = 0 ; i < 1 ; i++,sector++ )
	    {
	    for ( j = 0 ; j < 32 ; j++ )
		{
		unsigned char* ap = ascii;
		for ( k = 0 ; k < 4 ; k++ )
		    {
		    dat[k] = 0;
		    for ( l = 0 ; l < 4 ; l++ )
			{
			if(*cp<' '||*cp>'~'||(ap>ascii&&*cp=='/'&&cp[-1]=='*'))
			    *ap++ = '.';
			else
			    *ap++ = *cp;
			dat[k] = (dat[k]<<8) + *cp++;
			}
		    }
		*ap = 0;
		if ( dat[0]==last&&dat[1]==last&&dat[2]==last&&dat[3]==last )
		    {
		    if ( do_verbose == 1 )
			{
			printf(
 "    /* repeat:  %08lx   %08lx   %08lx   %08lx  ", last,last,last,last );
			if ( noascii )
			    printf("*/\n");
			else
			    printf("  %s*/\n",ascii);
			do_verbose = 2;
			}
		    }
		else
		    {
		    do_verbose = do_verbose!=0;
		    printf( "    {%4ld,%3ld,0x%08lx,0x%08lx,0x%08lx,0x%08lx},",
			    sector,(long)j*16,dat[0],dat[1],dat[2],dat[3]);
		    if ( noascii )
			printf("\n");
		    else
			printf("/*%s*/\n",ascii);
		    if ( dat[2]==dat[3] )
			last = dat[3];
		    array_size++;
		    }
		}
	    }
	}
    printf(
	"    {%4ld,%3ld,         0,         0,         0,         0}/*EOL*/\n",
        sector,-1L );
    printf(
        "  };  /* End of array %s[%ld], occupying at least %ld bytes */\n",
	    name, array_size, array_size*20 );
    printf("\n");
    }

int please_describe( int fd, struct fd_sense* fds, int formatted )
    {
    struct fd_mt mtype;

    if ( fds->drive_type == FD_DT_NO_DRIVE )
	{
	if ( formatted )
	    printf("0::0:0:0:0:0:0:0:::0:0:0\n" );
	else
	    printf( "NO DRIVE PRESENT.\n" );
	return 1;
	}
    else 
	{
	if ( formatted )
	    printf( "1:%s:", lookup_dt(fds->drive_type) );
	else
	    printf( "Drive type: %s\n", lookup_dt(fds->drive_type) );

	if ( fds->media_id == FD_MID_NO_MEDIA )
	    {
	    if ( formatted )
		printf("0:0:0:0:0:0:0:::0:0:0\n" );
	    else
		printf( "NO MEDIA PRESENT IN DRIVE.\n" );
	    return 1;
	    }
	else
	    {
	    long tmp;
	    if ( ioctl(fd,FDIOGETMTYP,&mtype) )
		return smsg("(error 8)\n",0), 1;

	    tmp = (long)mtype.n_surfaces*mtype.n_cylinders*mtype.n_sectors;

	    if ( formatted )
		{
		printf("1:%ld:%ld:%ld:%ld:%ld:%ld:%s:%s:%d:%d:%d\n",
			(long)mtype.n_sectors,
			(long)mtype.n_cylinders,
			(long)mtype.n_surfaces,
			(long)mtype.sector_size,
			tmp*SECTOR_SIZE,
			tmp,
			lookup_media(mtype.media),
			lookup_id(fds->media_id),
			fds->write_protected?1:0,
			fds->drive_nopen,
			mtype.spare1&3
			);
		}
	    else
		{
		printf("Number of sectors/track: %3ld\n",
			(long)mtype.n_sectors);
		printf("Number of cylinders/disk:%3ld\n",
			(long)mtype.n_cylinders );
		printf("Number of surfaces/disk: %3ld\n",
			(long)mtype.n_surfaces );
		printf("Sector size:            %4ld\n",
			(long)mtype.sector_size );
		if ( mtype.spare1&3 )
		    printf("Interleave:             %s\n",
			    ((mtype.spare1&3)==1)?"   1":">= 2" );
		printf("Total capacity:    %9ld bytes  (%ld sectors)\n",
			tmp*SECTOR_SIZE, tmp );
		if ( mtype.partition_start <= mtype.partition_end )
		    {
		    printf("Partition size:    %9ld bytes  (%ld sectors)\n",
			    (mtype.partition_end-mtype.partition_start)
			    *SECTOR_SIZE,
			    (mtype.partition_end-mtype.partition_start) );
		    printf("Partition offset:  %9ld bytes  (%ld sectors)\n",
			    mtype.partition_start*SECTOR_SIZE,
			    mtype.partition_start );
		    }
		printf("Media type:          %s\n",
			lookup_media(mtype.media) );
		printf("Media id:            %s\n",
			lookup_id(fds->media_id) );
		printf("Media write protect: %s\n",
			fds->write_protected?"READ ONLY":"READ/WRITE" );
		printf(
"(drive open count=%d, tamper=%d/%d, driver ver=0x%08lx)\n",
			fds->drive_nopen, fds->tamper_possible, fds->new_media,
			fds->version );
		}
	    return 0;
	    }
	}
    }

int please_test( int fd )
    {
    /*
    if ( ioctl(fd,FDIOCHKMED,&result) )
	return smsg("ERORR 6.\n",0),1;
    */

    long result = -1;
    struct fd_mt mtype;
    long limit;
    long i;

    if ( ioctl(fd,FDIOGETMTYP,&mtype) )
	return smsg("(error 8)\n",0), 1;
    
    limit = mtype.n_sectors*mtype.n_cylinders*mtype.n_surfaces;

    if ( do_silent )
	{
	for ( i = 0 ; i < limit ; i++ )
	    if ( qr(fd,i) )
		{
		result = i;
		break;
		}
	}
    else
	{
	int cyl;
	int head;
	fprintf( stderr, "Checking disk...\n" );
	fprintf(stderr,"  Percentage complete:   0%\10\10\10\10");
	for ( cyl = 0 ; cyl < mtype.n_cylinders && result<0 ; cyl++ )
	    for ( head = 0 ; head < mtype.n_surfaces && result<0 ; head++ )
		{
		long s = head*mtype.n_sectors
			+ cyl*mtype.n_surfaces*mtype.n_sectors;

		fprintf( stderr,
			"%3d%% (head=%1d, cyl=%3d)%s",
			(int)(s*100/limit),
			(int)head,
			(int)cyl,
			"\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b" );

		for ( i = 0 ; i < mtype.n_sectors ; i++ )
		    if ( qr(fd,i+s) )
			{
			result = i+s;
			break;
			}
		}

	if ( result < 0 )
	    fprintf(stderr,"Entire media reads OK.\n");
	else
	    fprintf(stderr,
		    "\nRead error encountered at sector %ld, Test aborted.\n",
		    result );
	}
    return result>=0;
    }

static int do_io( int fd, int io_req, int io_reg, int io_val )
    {
    struct fd_ioctst_struct xdat;

    xdat.d1 = io_reg;
    xdat.d2 = io_val;

    if ( io_req == 'w' )
	xdat.cmd = WREG;
    else if ( io_req == 's' )
	xdat.cmd = SREG;
    else if ( io_req == 'c' )
	xdat.cmd = CREG;
    else if ( io_req == 'r' )
	xdat.cmd = RREG;
    else
	return;
    
    if ( ioctl( fd, FDIOCTST, &xdat ) )
	return printf( "<ioctl(,FDIOCTST,{%d,%d,%d}) ERROR>\n", xdat.cmd,
		xdat.d1, xdat.d2 ), 1;
    else if ( io_req == 'r' )
	return printf( "OK, <0x%08lx>\n", xdat.d2 ), 0;
    else
	return printf( "OK\n" ), 0;
    }


int usage( char* my_name, char* problem )
    {
#   define E stderr

    /* Suggest 'strings -40 /usr/bin/fddisk' to get full help message.      */

    fprintf(E,"\0                                                        \n");
    if ( problem )
	fprintf(E,"Command-line syntax error at \"%s\"\n", problem);
    fprintf(E,"Usage: %s [-fmt [-i<nnn>[:<ccc>]] [-f]]          \n", my_name );
    fprintf(E,"            [-what] [-test] [-hexd<nnn>] [-s] [-wait] <dev>\n");
    fprintf(E,"  -fmt       Format, auto-sense density                    \n");
    fprintf(E,"\0-fmtdd     Format at double density ( 720k)              \n");
    fprintf(E,"\0-fmthd     Format at high density   (1.44m)              \n");
    fprintf(E,"\0-fmted     Format at extra density  (2.88m)              \n");
    fprintf(E,"\0-fsufs     Install Ultrix file system                    \n");
    fprintf(E,"\0-fsdos     Install MS-DOS file system                    \n");
    fprintf(E,"\0-fssco     Install SCO file system                       \n");
    fprintf(E,"\0-fsdiagnostic  Install diagnostic label on each sector.  \n");
    fprintf(E,"\0-mkdos     Combined format and MSDOS file-system install.\n");
    fprintf(E,"\0-mkufs     Combined format and UFS file-system install.  \n");
    fprintf(E,"\0    Note, -mkxxx should only be used on the disabled-    \n");
    fprintf(E,"\0    partition device to avoid re-reading of the partition\n");
    fprintf(E,"\0    table after formating of eavh track.                 \n");
    fprintf(E,"\0-lbl<xxx>  Set volume label to <xxx> (only with -fs, -mk)\n");
    fprintf(E,"  -f         Force format or file-system install even if   \n");
    fprintf(E,"              media not just inserted and even if drive in \n");
    fprintf(E,"              use by another process.                      \n");
    fprintf(E,"  -what      Print disk information.                       \n");
    fprintf(E,"\0-fwhat     Print disk information, formatted.            \n");
    fprintf(E,"  -i<nnn>[:<ccc>  Use interleave factor <nnn> for format.  \n");
    fprintf(E,"              If specified, <ccc> indicates additional     \n");
    fprintf(E,"              inter-cylinder interleave.                   \n");
    fprintf(E,"\0-new       Ignore previous disk change.                  \n");
    fprintf(E,"  -test      Check entire media for readability.           \n");
    fprintf(E,"  -hexd<nnn>[:<mmm>] Hex-dump sector <nnn>:<mmm> to stdout.\n");
    fprintf(E,"  -pchd      Packed-C hex dump of entire disk to stdout.   \n");
    fprintf(E,"\0-pcan<xx>  Packed-C hex dump array name is <xx>.         \n");
    fprintf(E,"\0-pcsn<xx>  Packed-C hex dump structure name is <xx>.     \n");
    fprintf(E,"\0-pcnosd    No structure definition in packed-c hex dump. \n");
    fprintf(E,"\0-pcnoa     No ascii column in packed-c hex dump.         \n");
    fprintf(E,"\0-pcnov     No verbose in packed-c hex dump.              \n");
    fprintf(E,"\0-exam<nnn>[:<mmm>] Copy sectors <nnn>:<mmm> to stdout.   \n");
    fprintf(E,"\0-dep<nnn>[:<mmm>]  Copy stdin to sectors <nnn>:<mmm>.    \n");
    fprintf(E,"  -s         Silent mode.                                  \n");
    fprintf(E,"  -wait      If none, wait for media in drive.             \n");
    fprintf(E,"  -waitn     Wait for NEW (since last open) media.         \n");
    fprintf(E,"\0-waiti     Wait for user to insert new media.            \n");
    fprintf(E,"  <dev>      is the device, such as /dev/fd                \n");
    fprintf(E,"\0The following are special debugging switches and cannot  \n");
    fprintf(E,"\0 be used in combination with other switches:             \n");
    fprintf(E,"\0-w:<reg>:<val> Write <val> to variable <reg>.            \n");
    fprintf(E,"\0-s:<reg>:<val> OR <val> into variable <reg>.             \n");
    fprintf(E,"\0-c:<reg>:<val> AND ~<val> into variable <reg>.           \n");
    fprintf(E,"\0-r:<reg>   Read variable <reg>, where <reg> one of:      \n");
    fprintf(E,"\0       fd_trigger, fdcam_debug_mask, fd_cbuf, fd_foo.    \n");
    fprintf(E,"\0                                                         \n");
    return 2;
    }




int main( int argc, char** argv )
    {
    char* my_name = "fddisk";
    enum fd_media_type do_format = FD_MT_NO_MEDIA;
    int do_make = 0;
    int interleave = 0;
    int iti = 0;
    enum install_fs do_install = IFS_NONE;

    int io_req = 0;                    /* 'w' for write,                     */
                                       /* 'r' for read,                      */
                                       /* 's' for OR into,                   */
                                       /* 'c' for AND~ into                  */
                                       /* 0   for no io request              */
    int io_reg;
    int io_val;

    int do_force = 0;
    int do_what = 0;
    int do_new = 0;
    int do_test = 0;
    long hexd = -1;   /* -1 == none,  else == sector */
    long hexd_end;
    long exam = -1;
    long exam_end;
    long do_pchd = 0;
    int do_pchd_noascii = 0;
    int do_pchd_noverbose = 0;
    int do_pchd_nosdef = 0;
    char* pchd_struct_name = "fsdat_struct";
    char* pchd_name = "fred";
    long deposit = -1;
    long deposit_end;
    long wait_duration = -1;
    int do_wait = 0;
    char* device = "";
    unsigned char* label = 0;
    int fd = -1;
    struct fd_sense fds;

    if ( *argv )
	my_name = *argv++;

    for ( ; *argv && **argv=='-' ; argv++ )
	{
	if ( !strcmp(*argv,"-fmt") && do_format==FD_MT_NO_MEDIA )
	    do_format = FD_MT_AUTOSENSE;
/**
	else if ( !strcmp(*argv,"-fmtdd") && do_format==FD_MT_NO_MEDIA )
	    do_format = FD_MT_350DD;
	else if ( !strcmp(*argv,"-fmthd") && do_format==FD_MT_NO_MEDIA )
	    do_format = FD_MT_350HD;
	else if ( !strcmp(*argv,"-fmted") && do_format==FD_MT_NO_MEDIA )
	    do_format = FD_MT_350ED;
**/
	else if ( !strcmp(*argv,"-fsufs") && do_install==IFS_NONE )
	    do_install = IFS_UFS;
	else if ( !strcmp(*argv,"-fsdos") && do_install==IFS_NONE )
	    do_install = IFS_MSDOS;
	else if ( !strcmp(*argv,"-fssco") && do_install==IFS_NONE )
	    do_install = IFS_SCO;
	else if ( !strcmp(*argv,"-fsdiagnostic") && do_install==IFS_NONE )
	    do_install = IFS_DIAGNOSTIC;
	else if ( !strcmp(*argv,"-mkdos") && do_install==IFS_NONE
		&& do_format==FD_MT_NO_MEDIA )
	    {
	    do_format = FD_MT_AUTOSENSE;
	    do_install = IFS_MSDOS;
	    do_make = 1;
	    }
	else if ( !strcmp(*argv,"-mkufs") && do_install==IFS_NONE
		&& do_format==FD_MT_NO_MEDIA )
	    {
	    do_format = FD_MT_AUTOSENSE;
	    do_install = IFS_UFS;
	    do_make = 1;
	    }
	/* Note, others could be added, such as "-mksco". */
	else if ( !strncmp(*argv,"-lbl",4) && !label )
	    label = (unsigned char*)*argv+4;
	else if ( !strncmp(*argv,"-label",6) && !label )
	    label = (unsigned char*)*argv+6;
	else if ( !strcmp(*argv,"-f") && do_force == 0 )
	    do_force = 1;
	else if ( !strcmp(*argv,"-what") && do_what == 0 )
	    do_what = 1;
	else if ( !strcmp(*argv,"-fwhat") && do_what == 0 )
	    do_what = 2;
	else if ( !strcmp(*argv,"-new") && do_new == 0 )
	    do_new = 1;
	else if ( !strcmp(*argv,"-test") && do_test == 0 )
	    do_test = 1;
	else if ( !strncmp(*argv,"-waiti",6) && do_wait == 0 )
	    {
	    do_wait = 3;
	    if ( *(*argv+6) )
		wait_duration = atol(*argv+6);
	    }
	else if ( !strncmp(*argv,"-waitn",6) && do_wait == 0 )
	    {
	    do_wait = 1;
	    if ( *(*argv+6) )
		wait_duration = atol(*argv+6);
	    }
	else if ( !strncmp(*argv,"-wait",5) && do_wait == 0 )
	    {
	    do_wait = 2;
	    if ( *(*argv+5) )
		wait_duration = atol(*argv+5);
	    }
	else if ( !strcmp(*argv,"-s") && do_silent == 0 )
	    do_silent = 1;
	else if ( !strncmp(*argv,"-hexd",5) && hexd<0 && exam<0 && !do_pchd )
	    {
	    char* xp = strchr(*argv+5,':');
	    if ( xp )
		{
		*xp = 0;
		hexd = atoi(*argv+5);
		hexd_end = atoi(xp+1);
		}
	    else
		{
		hexd = atoi(*argv+5);
		hexd_end = hexd;
		}
	    }
	else if ( !strncmp(*argv,"-exam",5) && exam<0 && hexd<0 && !do_pchd )
	    {
	    char* xp = strchr(*argv+5,':');
	    if ( xp )
		{
		*xp = 0;
		exam = atoi(*argv+5);
		exam_end = atoi(xp+1);
		}
	    else
		{
		exam = atoi(*argv+5);
		exam_end = exam;
		}
	    }
	else if ( !strncmp(*argv,"-dep",4) && deposit<0 )
	    {
	    char* xp = strchr(*argv+4,':');
	    if ( xp )
		{
		*xp = 0;
		deposit = atoi(*argv+4);
		deposit_end = atoi(xp+1);
		}
	    else
		{
		deposit = atoi(*argv+4);
		deposit_end = deposit;
		}
	    }
	else if ( !strcmp(*argv,"-pchd") && do_pchd == 0 && hexd<0 && exam<0 )
	    do_pchd = 1;
	else if ( !strcmp(*argv,"-pcnoa") )
	    do_pchd_noascii = 1;
	else if ( !strcmp(*argv,"-pcnov") )
	    do_pchd_noverbose = 1;
	else if ( !strcmp(*argv,"-pcnosd") )
	    do_pchd_nosdef = 1;
	else if ( !strncmp(*argv,"-pcan",5) )
	    pchd_name = *argv+5;
	else if ( !strncmp(*argv,"-pcsn",5) )
	    pchd_struct_name = *argv+5;
	else if ( !strncmp(*argv,"-i",2) && interleave == 0 )
	    {
	    char* xp = strchr(*argv+2,':');
	    if ( xp )
		{
		*xp = 0;
		interleave = atoi(*argv+2);
		iti = atoi(xp+1);
		}
	    else
		{
		interleave = atoi(*argv+2);
		iti = 0;
		}
	    }
	else if (  ( !strncmp(*argv,"-w:",3) || !strncmp(*argv,"-r:",3)
		|| !strncmp(*argv,"-c:",3) || !strncmp(*argv,"-s:",3) )
		&& !io_req )
	    {
	    char* xp = strchr(*argv+3,':');
	    char* var_name = *argv+3;
	    io_req = (*argv)[1];

	    if ( xp )
		{
		*xp = 0;
		io_val = atoi(xp+1);
		}
	    else if ( (*argv)[1] == 'r' )
		{
		io_val = 0;
		}
	    else
		return usage(my_name,*argv);

	    if ( !strcmp(var_name,"fd_trigger") )
		io_reg = 13;
	    else if ( !strcmp(var_name,"fdcam_debug_mask") )
		io_reg = 14;
	    else if ( !strcmp(var_name,"fd_cbuf") )
		io_reg = 15;
	    else if ( !strcmp(var_name,"fd_foo") )
		io_reg = 16;
	    else
		return usage(my_name,*argv);
	    }
	else
	    return usage(my_name,*argv);
	}

    if ( *argv )
	device = *argv++;

    if ( !device || !*device )
	return usage(my_name,"NO DEVICE SPECIFIED");
    else if ( *argv )
	return usage(my_name,*argv);
    else if ( label && do_install==IFS_NONE )
	return usage(my_name,"-lbl");
    else if ( do_force && do_format==FD_MT_NO_MEDIA )
	return usage(my_name,"-f");
    else if ( interleave && do_format==FD_MT_NO_MEDIA )
	return usage(my_name,"-i");
                                                                                
/* ************************************************************************* */
/*  Perform steps in following order:                                        */
/*    0)  Open the device                                                    */
/*    1)  wait for floppy if -wait, otherwise abort if no floppy.            */
/*    2)  if no -f, abort if floppy opened since insertion.                  */
/*    3)  format if supposed to.                                             */
/*    4)  install file system if supposed to, then change label if -lbl      */
/*    5)  if -dep, do it.                                                    */
/*    6)  if -hexd, -exam, or -pchd, do it.                                  */
/*    7)  if -what, do it.                                                   */
/*    8)  if -test, do it.                                                   */
/* ************************************************************************* */

    /*
    if ( do_format!=FD_MT_NO_MEDIA || do_wait )
    */
	fd = open( device, O_RDWR|O_NDELAY, 0 );
    /*
    else
	fd = open( device, O_RDWR, 0 );
    */

    if ( fd < 0 )
	{
	if ( errno == EACCES )
	    fprintf(stderr,"Required permissions missing to open \"%s\".\n",
		    device);
	else if ( errno == ENXIO )
	    fprintf(stderr,"Device driver for \"%s\" missing.\n",device);
	else
	    fprintf(stderr,"Unable to open \"%s\".\n",device);
	return 2;
	}

    if ( io_req )
	{
	do_io( fd, io_req, io_reg, io_val );
	return;
	}

    if ( ioctl(fd,FDIOSENSE,&fds) )
	return fprintf(stderr,"(1) Device \"%s\" not type fd.\n",device),2;
    if ( fds.magic != FD_MAGIC )
	return fprintf(stderr," Device \"%s\" not type fd, magic=0x%08lx.\n",
		device,fds.magic),2;

    if ( do_wait && please_wait(fd,do_wait,wait_duration,&fds) )
	return 2;
    
    if ( do_format!=FD_MT_NO_MEDIA && !do_force
	    && please_check_not_in_use(&fds) )
	return 2;

    if ( do_format!=FD_MT_NO_MEDIA && please_wait(fd,0,0,&fds) )
	return 2;

    if ( do_format!=FD_MT_NO_MEDIA && !do_force && please_check_new(fd,&fds) )
	return 2;

    if ( do_format!=FD_MT_NO_MEDIA && please_check_nowp(&fds) )
	return 2;
    
    if ( do_new && please_new(fd,&fds) )
	return 2;

    if ( interleave == 0 && do_format != FD_MT_NO_MEDIA )
	{
	interleave = 2;
	iti = 4;
	fprintf(stderr,"NOTE:  Setting interleave factor to ``-i2:4''.\n");
	/*
	fprintf(stderr,"NOTE:  Setting interleave factor to ``2'' due to \n");
	fprintf(stderr,"       system's lack of multi-sector floppy DMA.\n");
	*/
	fprintf(stderr,"       Use ``-i<nnn>[:<ccc>]'' option to override.\n");
	}
    
    if ( do_make )
	{
	if (please_make(fd,do_format,interleave,iti,do_force,do_install,label))
	    return 2;
	}
    else
	{
	if ( do_format!=FD_MT_NO_MEDIA &&
		please_format(fd,do_format,interleave,iti,do_force) )
	    return 2;
	
	if ( do_install != IFS_NONE && please_install(fd,do_install,label) )
	    return 2;
	}
    
    if ( deposit >= 0 && please_deposit(fd,deposit,deposit_end) )
	return 2;

    if ( exam >= 0 && please_examine(fd,exam,exam_end,0) )
	return 2;

    if ( hexd >= 0 && please_examine(fd,hexd,hexd_end,1) )
	return 2;

    if ( do_pchd && please_pchd(fd, do_pchd_noascii, do_pchd_noverbose,
	    do_pchd_nosdef, pchd_name, pchd_struct_name) )
	return 2;

    if ( do_what && please_describe(fd,&fds,do_what==2) )
	return 2;

    if ( do_test && please_test(fd) )
	return 2;

    if ( close(fd) )
	return fprintf(stderr,"Error during file close.\n"), 2;

    return 0;
    }




