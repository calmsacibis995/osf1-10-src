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
#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader= "$Id: getcons.c,v 1.1.2.5 92/11/05 15:03:38 Peter_Derr Exp $";	/* BuildSystemHeader */
#endif		/* BuildSystemHeader */
#include <stdio.h>
#include <signal.h>
#include <utmp.h>
#include <pwd.h>
#include <sys/time.h>
#include <grp.h>

#ifdef __osf__
#  include <sys/fcntl.h>
#else
#  include <ttyent.h>
#  include <sys/file.h>
#  define SCPYN(a, b)	strncpy(a, b, sizeof(a))
#  define SCMPN(a, b)	strncmp(a, b, sizeof(a))
#  ifndef TRUE
#    define TRUE (1)
#    define FALSE (0)
#  endif
#endif

static char *devname;

/*
 * cons_available()
 * returns boolean of whether or not someone is logged in on console
 */
static int cons_available()
{
#ifdef __osf__
	struct utmp utmp;
	struct utmp *static_utmp;

	setutent();
	strcpy(utmp.ut_line, "console");
	while ((static_utmp = getutline(&utmp)) != NULL) {
		if ((*static_utmp).ut_user[0] != '\0') {
			endutent();
			return(FALSE);
		};
		strcpy((*static_utmp).ut_line, "\0"); /* Force to non-static info. */
	};
	endutent();
	return(TRUE);
#else
	FILE *fp;
	struct utmp utmp;
	struct ttyent *tent;

	setttyent();
	if((tent = getttynam("console")) != NULL) 
		if(tent->ty_status & TTY_ON) return(FALSE);

	if((fp = fopen("/etc/utmp", "r")) == NULL)  {
		perror("/etc/utmp");
		exit(1);
	}

	while(fread(&utmp, sizeof(utmp), 1, fp) == 1) {
		if(utmp.ut_name[0] == '\0') continue;
		if(strcmp(utmp.ut_line, "console") == 0)  {
			fclose(fp);
			return(FALSE);
		}
	}
	return(TRUE);
#endif
}

/*
 * set_utmp()
 * make an entry in the utmp file for this login
 */
#ifdef __osf__
static void set_utmp(name) 
char *name;
{
   /*
    * utmp logging disabled for DEC OSF/1 since xdm logs to utmp.
    */
   /* ========= */
	return;
   /* ========= */
   /*
	struct passwd *pent;
	struct utmp utmp;

	zero_utmp(&utmp);
	name++;
        
	pent = getpwuid(getuid());
	strcpy(utmp.ut_user, pent->pw_name);
	strncpy(utmp.ut_id, name, 2);
	strcpy(utmp.ut_line, name);
	utmp.ut_type = USER_PROCESS;
	utmp.ut_pid = getppid();
	time(&utmp.ut_time);

	pututline(&utmp);
    */
}
#else
static void set_utmp(oldname, newname) 
char *oldname;
char *newname;
{
	struct ttyent *tent;
	int t;
	int foundit = FALSE;
	FILE *fp;
	struct utmp utmp;

	newname++;

	setttyent();
	for(t = 1; (tent = getttyent()) != NULL; t++ ) {
		if ((tent->ty_status & TTY_ON)
		    && strcmp(tent->ty_name, oldname) == 0) {
			foundit = TRUE;
			break;
		}
	}
	endttyent();
	if(! foundit ) {
		fprintf(stdout, "/etc/ttys\n");
		fflush(stdout);
		exit(-1);
	}
 
	if((fp = fopen("/etc/utmp", "r+")) == NULL) {
		perror("/etc/utmp");
		exit(-1);
	}
	fseek(fp, (long)(t*sizeof(utmp)), 0);
	if(fread(&utmp, sizeof(utmp), 1, fp) != 1) {
		fprintf(stdout, "t = %d, size = %d\n",t, sizeof(utmp));
		fflush(stdout);
		perror("utmp read");
		exit(-1);
	}
	if(strcmp(oldname, utmp.ut_line) != 0) {
		fprintf(stdout, "utmp entry %d mismatch %s != %s\n",
			t, oldname, utmp.ut_line);
		fflush(stdout);
		exit(-1);
	}
	fseek(fp, (long)(t*sizeof(utmp)), 0);
	strcpy(utmp.ut_line, newname);
	if(fwrite(&utmp, sizeof(utmp), 1, fp) != 1) {
		perror("writing utmp");
		exit(-1);
	}
	fclose(fp);
}
#endif

/* 
 * clear_utmp()
 * clear out this sessions utmp file entry
 */
static void clear_utmp(name) 
char *name;
{
#ifdef __osf__

   /*
    * utmp logging disabled for DEC OSF/1 since xdm logs to utmp.
    */
   /* ========= */
	return;
   /* ========= */
   /*
	struct utmp utmp;
	struct utmp *uptr;
	int i;

	name++;

	zero_utmp(&utmp);
	strcpy(utmp.ut_line, name);
	if ((uptr=getutline(&utmp)) == NULL) {
		perror("can't find utmp entry");
		return;
	}
	
	for (i=0; i<32; i++) uptr->ut_user[i] = '\0';
	for (i=0; i<64; i++) uptr->ut_host[i] = '\0';
	uptr->ut_type = DEAD_PROCESS;
	time(&uptr->ut_time);
	pututline(uptr);
    */
#else
	register f;
	int found = 0;
	struct utmp wtmp;

	name++;

	f = open("/etc/utmp", O_RDWR);
	if (f >= 0) {
		while (read(f, (char *)&wtmp, sizeof(wtmp)) == sizeof(wtmp)) {
			if (SCMPN(wtmp.ut_line, name) || wtmp.ut_name[0]==0)
				continue;
			lseek(f, -(long)sizeof(wtmp), 1);
			SCPYN(wtmp.ut_name, "");
			SCPYN(wtmp.ut_host, "");
			time(&wtmp.ut_time);
			write(f, (char *)&wtmp, sizeof(wtmp));
			found++;
		}
		close(f);
	}
	if (found) {
		f = open("/usr/adm/wtmp", O_WRONLY|O_APPEND);
		if (f >= 0) {
			SCPYN(wtmp.ut_line, name);
			SCPYN(wtmp.ut_name, "");
			SCPYN(wtmp.ut_host, "");
			time(&wtmp.ut_time);
			write(f, (char *)&wtmp, sizeof(wtmp));
			close(f);
		}
	}
#endif
}

/* this doesn't make any sense... */
static char *getpty()
{
	static char ttydev[] = "/dev/ttyxx";
	static char ptydev[] = "/dev/ptyxx";
	int devindex, letter = 0;
	int pty, tty;

	while (letter < 11) {
	    ttydev [8] = ptydev [8] = "pqrstuvwxyz" [letter++];
	    devindex = 0;

	    while (devindex < 16) {
		ttydev [9] = ptydev [9] = "0123456789abcdef" [devindex++];
		if ((pty = open (ptydev, O_RDWR)) < 0)
			continue;
		if ((tty = open (ttydev, O_RDWR)) < 0) {
			close(pty);
			continue;
		}
		close(tty);
		close(pty);
		return(ttydev);
	    }
	}
	perror("ptys");
	return(NULL);
}

main(argc, argv)
char *argv[];
int argc;
{
	char *ptr;
        struct group *terminal_or_system_group;

	/* don't want xsession quitting to kill us off */
	signal(SIGTERM, SIG_IGN);
	signal(SIGHUP, SIG_IGN);

	if(argc < 2) {
		fprintf(stdout, "usage:\n");
		fflush(stdout);
		exit(-1);
	}
	if (argc == 2) {
		if(argv[1][0] != ':') {
			fprintf(stdout, "invalid argument\n");
			fflush(stdout);
			exit(-1);
		}
		if(XOpenDisplay(argv[1]) == NULL) {
			fprintf(stdout, "can't open display\n");
			fflush(stdout);
			exit(-1);
		}
		if(!atoi(argv[1]+1) && cons_available()) {
			devname = "/dev/console";
			chown("/dev/xcons",  getuid(), getgid());
			chmod("/dev/xcons", 0666);
		} else devname = getpty();
#ifdef __osf__
		if (devname) 
			set_utmp(rindex(devname, '/'));
#else
		if (devname) 
			set_utmp(argv[1], rindex(devname, '/'));
#endif

                setgrent();
                if ((terminal_or_system_group = getgrnam("terminal")) != 0)
                        chown(devname, getuid(), terminal_or_system_group->gr_gid);
                else
                        chown(devname, getuid(), getgid()); /* Don't know what to do if above fails. */
                endgrent();
		chmod(devname, 0620);
		/* old code:
		  chown(devname, getuid(), getgid());
		  chmod(devname, 0600);
		*/
		
#ifdef RETURN_PTY
		ptr = rindex(devname, '/'); ptr++;
		if (*ptr == 't') *ptr = 'p';
#endif
		fprintf(stdout, "%s\n", devname); fflush(stdout);
		exit(0);
	} else {

                setgrent();
                if ((terminal_or_system_group = getgrnam("system")) != 0)
                        chown("/dev/console", 0, (*terminal_or_system_group).gr_gid);
                endgrent();
		chmod("/dev/console", 0622);

#ifdef RETURN_PTY
		ptr = rindex(argv[1], '/');  ptr++;
		if (*ptr == 'p') *ptr = 't';
#endif
		clear_utmp(rindex(argv[1], '/'));
	}
}

#ifdef __osf__
zero_utmp(uptr)
struct utmp *uptr;
{
	int i;

	for (i=0; i<32; i++) uptr->ut_user[i] = '\0';
	for (i=0; i<14; i++) uptr->ut_id[i] = '\0';
	for (i=0; i<32; i++) uptr->ut_line[i] = '\0';

	uptr->ut_type = uptr->ut_pid
	  = uptr->ut_exit.e_termination = uptr->ut_exit.e_exit = 0;

	uptr->ut_time = 0;
	for (i=0; i<64; i++) uptr->ut_host[i] = '\0';
}
#endif

logErr(str)
char *str;
{
    FILE *errorfile;

    if ((errorfile = fopen("/tmp/getcons.log", "a")) != NULL) {
    	fprintf(errorfile, "getcons:  %s", str);
    	fflush(errorfile);
    	fclose(errorfile);
    }
}
