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
static char     *sccsid = "@(#)$RCSfile: ntp_util.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/05/04 16:58:47 $";
#endif
/*
 */

/*
 * ntp_util.c - stuff I didn't have any other place for
 */
#include <stdio.h>
#include <errno.h>
#include <strings.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <sys/time.h>
#ifdef convex
#include "/sys/sync/queue.h"
#include "/sys/sync/sema.h"
#endif
#include <net/if.h>
#include <netinet/in.h>

#include <ntp/ntp_syslog.h>
#include <ntp/ntp_fp.h>
#include <ntp/ntp.h>


/*
 * This contains odds and ends.  Right now the only thing you'll find
 * in here is the hourly stats printer and some code to support rereading
 * the keys file, but I may eventually put other things in here such as
 * code to do something with the leap bits.
 */

/*
 * Name of the keys file
 */
char *key_file_name;

/*
 * The name of the drift_comp file and the temporary.
 */
char *stats_drift_file;
char *stats_temp_file;

/*
 * We query the errno to see what kind of error occured
 * when opening the drift file.
 */
extern int errno;

#ifdef DEBUG
extern int debug;
#endif

/*
 * init_util - initialize the utilities
 */
void
init_util()
{
	stats_drift_file = 0;
	stats_temp_file = 0;
	key_file_name = 0;
}


/*
 * hourly_stats - print some interesting stats
 */
void
hourly_stats()
{
	int fd;
	char *val;
	int vallen;
	extern l_fp drift_comp;
	extern int compliance;
	extern char *lfptoa();
	extern char *mfptoa();
	extern int no_log;
	
	if (!no_log){
	syslog(LOG_INFO, "hourly check: drift %s compliance %s",
		lfptoa(&drift_comp, 8),
		mfptoa((compliance<0)?(-1):0, compliance, 8));}
	
	if (stats_drift_file != 0) {
		fd = open(stats_temp_file, O_WRONLY|O_TRUNC|O_CREAT, 0666);
		if (fd == -1) {
			syslog(LOG_ERR, "can't open %s: %m", stats_temp_file);
			return;
		}

		val = lfptoa(&drift_comp, 9);
		vallen = strlen(val);
		/*
		 * Hack here.  Turn the trailing \0 into a \n and write it.
		 */
		val[vallen] = '\n';
		if (write(fd, val, vallen+1) == -1) {
			syslog(LOG_ERR, "write to %s failed: %m",
			    stats_temp_file);
			(void) close(fd);
			(void) unlink(stats_temp_file);
		} else {
			(void) close(fd);
			/* atomic */
			(void) rename(stats_temp_file, stats_drift_file);
		}
	}
}


/*
 * stats_config - configure the stats operation
 */
void
stats_config(item, value)
	int item;
	char *value;	/* only one type so far */
{
	register char *cp;
	FILE *fp;
	int len;
	char buf[128];
	l_fp old_drift;
	extern char *emalloc();
	extern void loop_config();
	extern char *lfptoa();

	switch(item) {
	case STATS_FREQ_FILE:
		if (stats_drift_file != 0) {
			(void) free(stats_drift_file);
			(void) free(stats_temp_file);
			stats_drift_file = 0;
			stats_temp_file = 0;
		}

		if (value == 0 || (len = strlen(value)) == 0)
			break;

		stats_drift_file = emalloc((u_int)(len + 1));
		stats_temp_file = emalloc((u_int)(len + sizeof(".TEMP")));
		bcopy(value, stats_drift_file, len+1);
		bcopy(value, stats_temp_file, len);
		bcopy(".TEMP", stats_temp_file + len, sizeof(".TEMP"));
#ifdef DEBUG
		if (debug > 1) {
			printf("stats drift file %s\n", stats_drift_file);
			printf("stats temp file %s\n", stats_temp_file);
		}
#endif

		if ((fp = fopen(stats_drift_file, "r")) == NULL) {
			if (errno != ENOENT)
				syslog(LOG_ERR, "can't open %s: %m",
				    stats_drift_file);
			break;
		}

		if (fgets(buf, sizeof buf, fp) == NULL) {
			syslog(LOG_ERR, "can't read %s: %m",
			    stats_drift_file);
			(void) fclose(fp);
			break;
		}

		(void) fclose(fp);

		/*
		 * We allow leading spaces, then the number.  Terminate
		 * at any trailing space or string terminator.
		 */
		cp = buf;
		while (isspace(*cp))
			cp++;
		while (*cp != '\0' && !isspace(*cp))
			cp++;
		*cp = '\0';

		if (!atolfp(buf, &old_drift)) {
			syslog(LOG_ERR, "drift value %s invalid", buf);
			break;
		}

		/*
		 * Finally!  Give value to the loop filter.
		 */
#ifdef DEBUG
		if (debug > 1) {
			printf("loop_config finds old drift of %s\n",
			    lfptoa(&old_drift, 9));
		}
#endif
		loop_config(LOOP_DRIFTCOMP, &old_drift);
		break;
	
	default:
		/* oh well */
		break;
	}
}


/*
 * getauthkeys - read the authentication keys from the specified file
 */
void
getauthkeys(keyfile)
	char *keyfile;
{
#ifdef DES_OK
	int len;
	extern char *emalloc();

	len = strlen(keyfile);
	if (len == 0)
		return;
	
	if (key_file_name != 0) {
		if (len > strlen(key_file_name)) {
			(void) free(key_file_name);
			key_file_name = 0;
		}
	}
	if (key_file_name == 0)
		key_file_name = emalloc((u_int)(len + 1));
	
	bcopy(keyfile, key_file_name, len+1);
	authreadkeys(key_file_name);
#endif /* DES_OK */
}


/*
 * rereadkeys - read the authentication key file over again.
 */
void
rereadkeys()
{

#ifdef DES_OK
	if (key_file_name != 0)
		authreadkeys(key_file_name);
#endif /* DES_OK */

}



