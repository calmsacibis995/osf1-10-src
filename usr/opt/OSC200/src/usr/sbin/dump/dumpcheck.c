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
static char	*sccsid = "@(#)$RCSfile: dumpcheck.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 06:04:31 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * dumpcheck.c
 *
 *	Modification History:
 *
 * 25-Jul-91	Sam Lambert
 *	Changed operation of at_load_point() to be less device
 *	dependent.  See comments around #ifndef OSF_REFERENCE_SOURCE.
 *
 */
#if !defined( lint) && !defined(_NOIDENT)

#endif

#include	"dump.h"
#include	<sys/file.h>
#include	<sys/ioctl.h>
#include	<sys/mtio.h>
#include	<sys/stat.h>

static int	read_header();
static int	at_load_point();
static int	check_checksum();
static void	reposition_tape();

/*
 * check the tape for a valid date. return TRUE on success.
 */

int
check_tape()
{
	struct s_spcl  *tape_header;
	int		status;
	int		tape_fd;

	if ( medium_flag == REGULAR_FILE )
		return( TRUE );

	while ((tape_fd = open(tape_file_name, O_RDONLY)) < 0)
	{
		msg(MSGSTR(CNOTFD, "Cannot open tape for date check\n"));
		if (query(MSGSTR(DYWTR, "Do you want to retry")) == NO)
		{
			abort_dump();
		}
	}

	status = TRUE;

	if (at_load_point(tape_fd) == TRUE)
	{
		tape_header = (struct s_spcl *) malloc((unsigned) max(blocks_per_write, HIGHDENSITYTREC) * TP_BSIZE);

		if (read_header(tape_fd, tape_header) == FALSE)
		{
			msg(MSGSTR(BWTDT1, "Cannot get the previous write date of tape\n"));
			msg(MSGSTR(BWTDT2, "Tape may be either 1) not a dump tape, 2) a new,\n"));
			msg(MSGSTR(BWTDT3, "unused tape, or 3) not at the load-point\n"));
			if (query(MSGSTR(DYWTOT, "Do you want to overwrite this tape")) == NO)
			{
				status = FALSE;
			}
		}
		else if (spcl.c_date - tape_header->c_date < (int) (DAY * 6.5))
		{

			/* less than 6.5 days old */

			msg(MSGSTR(RUIN1, "This tape was written last on %s\n"), ctime(&tape_header->c_date));
			msg(MSGSTR(RUIN2, "That was less than a week ago!\n"));
			msg(MSGSTR(RUIN3, "You may be RUINING a good tape!  Be careful!\n"));
			if (query(MSGSTR(DYRWTO, "Do you really wish to overwrite this tape")) == NO)
			{
				status = FALSE;
			}
		}
		else
		{
			msg(MSGSTR(VLDDT, "Mounted tape has a valid date\n"));
		}

		reposition_tape(tape_fd);

		free(tape_header);
	}

	(void) close(tape_fd);

	return(status);
}

static int
at_load_point(tape_fd)
	int		tape_fd;
{
#ifndef	OSF_REFERENCE_SOURCE
	/* DEC extension to allow tape drives to be recognized.
	 * Unfortunately, the SCSI drivers do not support/set the 
	 * DEV_BOM flag so there's really no way to determine if
	 * the device is really at BOT.  This means this routine
	 * will always return FALSE and the tape date check in
	 * check_tape() will never be performed.  There is no way
	 * around this problem though (until the drivers are made
	 * to properly set DEV_BOM) because the original OSF/1 way
	 * of getting this information (MTIOCGET and checking xxx_BOT
	 * bits) is not supported either...
	 */

	struct devget	devget;

	if (ioctl(tape_fd, DEVIOCGET, &devget) < 0)
	{
		msg(MSGSTR(UTCDS, "Unable to check drive status\n"));
		dump_perror("at_load_point(): ioctl()");
		if (query(MSGSTR(DYWTCA, "Do you want to continue anyway")) == NO)
		{
			abort_dump();
		}
		return(FALSE);
	}

#ifdef	DEBUG
	fprintf(stderr, "dump(debug): at_load_point(): devget.stat & DEV_BOM = %d\n", 
		(devget.stat & DEV_BOM));
	fflush(stderr);
#endif	/* DEBUG */

	if (devget.stat & DEV_BOM) {
		return(TRUE);
	}
	return(FALSE);

#else	/* if OSF_REFERENCE_SOURCE 
	 * This is the way the OSF source originally "worked".
	 * There's really no need to check the "known devices"
	 * at this level of detail, so don't bother.  Not doing
	 * so also eliminates the "Unrecognized tape drive type"
	 * message which users found disconcerting.
	 * The main problems are that these "xxx_BOT" flags
	 * aren't defined anywhere, and there was no "#if mips" 
	 * entry, so the user would ALWAYS get the above error.
	 * So now we use a DEVIOCGET and check the generic DEV_BOM 
	 * flag (see above), which is sufficient for the purposes of 
	 * the routines in this module.
 	 */

	struct mtget	mtget;

	if (ioctl(tape_fd, MTIOCGET, &mtget) < 0)
	{
		msg(MSGSTR(UTCDS, "Unable to check drive status\n"));
		dump_perror("at_load_point(): ioctl()");
		if (query(MSGSTR(DYWTCA, "Do you want to continue anyway")) == NO)
		{
			abort_dump();
		}
		return(FALSE);
	}

	switch (mtget.mt_type) {

#if	vax

	case MT_ISHT: 
		if (mtget.mt_dsreg & HTDS_BOT)
		{
			return(TRUE);
		}
		return(FALSE);

	case MT_ISUT: 
		if (mtget.mt_dsreg & UTDS_BOT)
		{
			return(TRUE);
		}
		return(FALSE);

	case MT_ISTS: 
		if (mtget.mt_dsreg & TS_BOT)
		{
			return(TRUE);
		}
		return(FALSE);

	case MT_ISTM: 
		if (mtget.mt_dsreg & TMER_BOT)
		{
			return(TRUE);
		}
		return(FALSE);

	/* NB: If, for some reason, it is desired to go back
	 * to the "MTIOCGET" way of doing things, the following
	 * "mt_type" of devices should be recognized and the 
	 * appropriate "xxx_BOT" flags defined (somewhere).
	 */
	case MT_ISMT:
		if (mtget.mt_dsreg & ???_BOT)
		{
			return(TRUE);
		}
		return(FALSE);

	case MT_ISTMSCP:
		if (mtget.mt_dsreg & ???_BOT)
		{
			return(TRUE);
		}
		return(FALSE);

	case MT_ISST:
		if (mtget.mt_dsreg & ???_BOT)
		{
			return(TRUE);
		}
		return(FALSE);

	case MT_ISSCSI:
		if (mtget.mt_dsreg & ???_BOT)
		{
			return(TRUE);
		}
		return(FALSE);

}
#endif	vax

#if	sun

	case MT_ISXY:
		if (mtget.mt_dsreg & XTS_BOT)
		{
			return(TRUE);
		}
		return(FALSE);

#endif sun

	default: 
		msg(MSGSTR(UTDT, "Unknown tape drive type: %d (0x%x)\n"), mtget.mt_type, mtget.mt_dsreg);
		msg(MSGSTR(SDCHK, "Skipping date check\n"));
		return(FALSE);
	}

#endif	/* ! OSF_REFERENCE_SOURCE */
	/*NOTREACHED*/

}

/*
 *	Examine the header to see that it is OK. That is, is it a header
 *	Return FALSE on error.
 */

static int
read_header(tape_fd, tape_header)
	int		tape_fd;
	struct s_spcl  *tape_header;
{
	if (read(tape_fd, (char *) tape_header, blocks_per_write * TP_BSIZE) < 0)
	{
		msg(MSGSTR(TREWTH, "Tape read error while trying to read tape header\n"));
		dump_perror("read_header(): read()");
		return(FALSE);
	}
	if (tape_header->c_magic != NFS_MAGIC && tape_header->c_magic != OFS_MAGIC)
	{
		msg(MSGSTR(BDMGK, "Bad magic number in tape header\n"));
		return(FALSE);
	}
	if (check_checksum((int *) tape_header) == FALSE)
	{
		msg(MSGSTR(CKSMERR, "Checksum error in tape header\n"));
		return(FALSE);
	}
	if (tape_header->c_type != TS_TAPE)
	{
		msg(MSGSTR(THNTTH, "Tape header record not of type tape-header\n"));
		return(FALSE);
	}
	return(TRUE);
}

static int
check_checksum(buffer)
	register int	*buffer;
{
	register int	i, j;

	j = sizeof(union u_spcl) / sizeof(int);
	i = 0;

	do
	{
	    i += *buffer++;
	}
	while (--j > 0);

	if (i != CHECKSUM)
	{
		return(FALSE);
	}
	return(TRUE);
}

/*
 * Reposition the tape at load_point
 */

static void
reposition_tape(tape_fd)
	int		tape_fd;
{
	struct mtop	mtop;

	mtop.mt_op = MTREW;
	mtop.mt_count = 1;

	if (ioctl(tape_fd, MTIOCTOP, &mtop) < 0)
	{
		if (ioctl(tape_fd, MTIOCTOP, &mtop) < 0)
		{
			msg(MSGSTR(TRRF, "Tape repositioning (rewind) failed\n"));
			dump_perror("reposition_tape(): ioctl()");
			msg(MSGSTR(CKPTBC, "Check position of tape before continuing\n"));
		}
	}
}

#ifdef	NOTYET

int
check_tape_status(tape_fd)
	int	tape_fd;
{
	int error =0;
	struct devget	devget;

	if (ioctl(tape_fd, DEVIOCGET, &devget) < 0)
	{
		msg(MSGSTR(UTCDS, "Unable to check drive status\n"));
		dump_perror("check_tape_status(): ioctl()");
		if (query(MSGSTR(DYWTCA, "Do you want to continue anyway")) == NO)
		{
			abort_dump();
		}
		return(TRUE);
	}

	if (devget.stat & DEV_OFFLINE) { 
		msg(MSGSTR(DOFLOLC, "Device offline; place online to continue\n");
		error++;
	}

	if (devget.stat & DEV_WRTLCK) {
		msg(MSGSTR(DWRTLK, "Device write locked; write enable to continue\n");
		error++;
	}

	if (error) {
		return(FALSE);
	}
	return(TRUE);
}
#endif	/* NOTYET */
