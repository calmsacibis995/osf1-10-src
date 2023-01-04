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
static char	*sccsid = "@(#)$RCSfile: strsetup.c,v $ $Revision: 4.2.12.11 $ (DEC) $Date: 1993/12/15 20:03:45 $";
#endif 
/*
 */
/*
 * STREAMS kernel information/manager tool
 *
 * Driven by a set of environment variables, including:
 *
 *	KINFO_SIZE	buffer size used in STREAMS requests
 *	KINFO_LOOPS	iterations of STREAMS requests
 *	KINFO_SLEEP	delay between iterative STREAMS requests
 *	KINFO_HOLDOPEN	delay I/O on Stream for KIINFO_SLEEP
 *			 seconds after open
 *	KINFO_DEVICE	device special file name used for open call
 *
 * 01-Jun-91	wca
 *	Creation date
 */

#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stream.h>
#include <stropts.h>
#include <poll.h>
#include <time.h>
#include <sys/strkinfo.h> 
#include <sys/mode.h>
#include <nlist.h>
#include <paths.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <machine/hal_sysinfo.h>

#define STRINFO_BUF_SIZE	256
#define DEV_DIR        "/dev/streams/"
#define SAD_DIR		"/dev/sad/"
#define XTISO_DEV_DIR  "/dev/streams/xtiso/"

char ebuf[STRINFO_BUF_SIZE];
char myName[STRINFO_BUF_SIZE];
char defaultName[] = "/dev/streams/kinfo";
char kinfoName[STRINFO_BUF_SIZE];
int  bufSize = MAX_KINFO_BUF;
int  loopCount = ~0;
int  holdOpenFlag = 0;
int  zzzz = 3;
dev_t find_kinfodev();
int first = TRUE;

char *configfile_name = "/etc/strsetup.conf";
FILE *cf;		/* configuration file */

int iflag, cflag, rflag, fflag, dflag;

main( argc, argv )
int argc;
char *argv[];
{
        int ch;
	int status = 1;

	/*
	 * Setup/initilize our run time environment
	 */
	Init( argv[0] );

	/*
	 * Expect 1 argument on command line; IOCTL
	 * function code; all other settable parameters
	 * are taken from environment.
	 */
	while ((ch = getopt(argc, argv, "icdrf:")) != EOF)
            switch ((char)ch) {
	        case 'i':
		    iflag++;
		    break;
	        case 'c':
		    cflag++;
		    break;
		case 'd':
		    dflag++;
		    break;
		case 'f':
		    configfile_name = optarg;
		    fflag++;
		    break;

		default :
		    status = DoUsage();
		    break;
	   }

	/* if installing or deleting files */
	if(iflag || dflag) {
	    cf = fopen(configfile_name,"r");	/* open the config file */
	    if(cf == NULL) {
		fprintf(stderr,"%s: can't open configuration file ",myName);
		perror(configfile_name);
		/* exit only if a user supplied config file is not present */
		if(fflag)
		    exit(1);
		fprintf(stderr,"%s: will make default Streams configuration",myName);
		
	    } else
		(void)config_parse();		/* parse the file */
	    /* DoStrSetup will scan the configuratio information for 
	     * comparisons if only the delete option has been selected.
	     * Otherwise, any needed files will be installed.
	     */
	    status = DoStrSetup();
	    first = TRUE;
	    if(dflag)
		(void)destroy();
	}
	if(cflag)
	    status = DoGetSTRCFG();


	exit(status);
}


/*
 * Get the current STREAMS modules and drivers
 * configuration information.
 */
int DoGetSTRCFG()
{
	int fd;
	int bits;
	int status;
	struct strioctl str;
	SAD *buf;
	int i;

	fd = OpenKinfo();
	if (fd < 0) {
		sprintf(ebuf, " %s - open of '%s'", myName, kinfoName);
		perror(ebuf);
		return (1);
	}

	str.ic_cmd = KINFO_GET_STR_CFG;
	str.ic_timout = 15;
	str.ic_len = bufSize;
	buf = (SAD *)malloc(str.ic_len);
	if (buf == (SAD *)0) {
		sprintf(ebuf, " %s - malloc", myName);
		errno = ENOBUFS;
		perror(ebuf);
		return(1);
	}
	str.ic_dp = (char *) buf;
	bzero(str.ic_dp, str.ic_len);
	status = ioctl(fd, I_STR, &str);
	if (status < 0) {
		sprintf(ebuf, " %s - ioctl", myName);
		perror(ebuf);
		return(1);
	}

	DumpSAD( buf );

	free(str.ic_dp);
	close(fd);
	return(0);
}


#define DLB_MODE (S_IFCHR | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)
#define OTHER_MODE (S_IFCHR | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)

int DoStrSetup()
{
      dev_t devno;
      int i, j;
      int fd;
      struct strioctl str;
      SAD *buf;
      id_blk *id;
      STR_CFG *info;
      int status;
      struct stat sstat;
      int printit;
      mode_t oumask;
      int newkinfo = 0;

      if (stat(DEV_DIR , &sstat) < 0)
	  mkdir(DEV_DIR, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
      if (stat(XTISO_DEV_DIR , &sstat) < 0)
	  mkdir(XTISO_DEV_DIR, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
      if (stat(SAD_DIR , &sstat) < 0)
	  mkdir(SAD_DIR, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);

      str.ic_timout = 15;
      str.ic_len = bufSize;
      buf = (SAD *)malloc(str.ic_len);
      if (buf == (SAD *)0) {
	   sprintf(ebuf, " %s - malloc", myName);
	   errno = ENOBUFS;
	   perror(ebuf);
	   return(1);
       }
      str.ic_dp = (char *) buf;
      id = (id_blk *)buf;
      bzero(str.ic_dp, str.ic_len);
      str.ic_cmd = KINFO_GET_ID;

      /*
       * determine the device # for kinfo
       */
      devno = find_kinfodev();

      /*
       *  if the device for kinfo does not exist - create it.
       */
      status = stat(kinfoName, &sstat);
      if (status == -1 || (sstat.st_rdev != devno)) {
	      newkinfo = 1;
	      unlink(kinfoName);
	      oumask = umask(0);
	      if (mknod(kinfoName, OTHER_MODE, devno)) {
		  sprintf(ebuf, "%s: unable to mknod kinfo", myName);
		  perror(ebuf);
		  oumask = umask(oumask);
		  return(1);
	      }
	      oumask = umask(oumask);
      }

      /*
       * open kinfo and verify that it's the right version
       */
      if ((fd = OpenKinfo()) < 0) {
	  sprintf(ebuf, "%s: unable to open kinfo", myName);
  	  perror(ebuf);
	  return(1);
      }
      if (ioctl(fd, I_STR, &str) < 0) {
	  sprintf(ebuf, "%s: unable to find kinfo", myName);
	  perror(ebuf);
	  return(1);
      }
      if ((strncmp(id->name, KINFO_NAME, strlen(KINFO_NAME)) != 0) ||
	  (id->version != KINFO_VERSION)) {
  	  fprintf(stderr, "%s: kinfo has wrong name or version", myName);
	  return(1);
      }

      /*
       * get the list of streams modules
       */
      str.ic_cmd = KINFO_GET_STR_CFG;
      bzero(str.ic_dp, str.ic_len);
      if (ioctl(fd, I_STR, &str) < 0) {
	  sprintf(ebuf, "%s: Unable to get streams configuration info", myName);
	  perror(ebuf);
	  return(1);
      }
      close(fd);
      if (newkinfo)
	  unlink(kinfoName);


      /*
       * create a device for each module in the list
       */
      info = &buf->sad_cfg[0];
      oumask = umask(0);
      for (i= 0; i < buf->sad_blkhdr.sad_cnt; i++) {
	   char name[256];
	   int mode;
 
	   if(config(info->scb_name,major(info->scb_device),
			major(buf->sad_blkhdr.sad_clonedev)) == TRUE ||
			!iflag) {
		info++;
		continue;
	   }

 	   if (!strcmp(info->scb_name,"xtisoUDG") ||
 	       !strcmp(info->scb_name,"xtisoUST") ||
 	       !strcmp(info->scb_name,"xtisoIDP") ||
 	       !strcmp(info->scb_name,"xtisoSPP")) {
 		   info++;
 		   continue;
 	   }   

	   printit = TRUE;

	   if (!(info->scb_flags & SADF_DEVICE))
	       continue;
	   devno =  buf->sad_blkhdr.sad_clonedev | major(info->scb_device);
	   if (strcmp(info->scb_name, "dlb") == 0)
 		   mode = DLB_MODE;
	   else 
	       mode = OTHER_MODE;
	   if (strncmp(info->scb_name, "xtiso", 5) == 0) {
	       /* 
		* hack to change names like "xtisoTCP" to "tcp"
		*/
	       char ptr[256];
	       strcpy(ptr, info->scb_name + strlen("xtiso"));
	       for (j = 0; j < strlen(ptr); j++) 
		   ptr[j] = _tolower(ptr[j]);
	       sprintf(name, "%s%s",XTISO_DEV_DIR, ptr);
	   } else
	       if (strncmp(info->scb_name, "ptm", 3) == 0) {
		   /*
		    * hack to create /dev/ptmx
		    */
		   sprintf(name,"/dev/ptmx");
	    } else
	       if (strncmp(info->scb_name, "sad", 3) == 0) {
		   /*
		    * hack to create /dev/sad/admin
		    */
		   sprintf(name,"/dev/sad/admin");
	       } else
		   sprintf(name, "%s%s", DEV_DIR, info->scb_name);
	   if (mknod(name, mode, devno)) {
	       if (errno == EEXIST) {
		   stat(name, &sstat);
		   if (sstat.st_rdev != devno) {
		       unlink(name);
		       mknod(name, mode, devno);
		   } else {
		       printit = FALSE;
		   }
	       } else {
		   sprintf(ebuf, "%s: Unable to mknod on %s",
			   myName, info->scb_name);
		   perror(ebuf);
		   printit = FALSE;
	       }
	   }
	   if (printit == TRUE) {
	       if (first) {
		   printf("\tThe following STREAMS devices were created:\n");
		   printf("%30s %10s %10s\n", "Name", "Major", "Minor");
		   printf("%30s %10s %10s\n", "----", "-----", "-----");
		   first = FALSE;
	       }
	       printf("%30s %10d %10d\n", name,
	              major(buf->sad_blkhdr.sad_clonedev), major(info->scb_device));
	   }
           info++;
      }
      oumask = umask(oumask);
      return (0);

}

int DoNoSupport( func )
int func;
{
	sprintf(ebuf, " %s - function %d", myName, func);
	errno = ENOSYS;
	perror(ebuf);

	return(1);
}


/*
 * Generic open of device special file
 */
int OpenKinfo()
{
	int status;

	status = open(kinfoName, O_RDWR, 0);
	HoldOpen();
	return(status);
}

/*
 * Delay subsequent I/O.  Used
 * to debug multiple, simultaneous
 * opens on the STREAMS driver. 
 */
int HoldOpen()
{
	if (holdOpenFlag) {
		sleep(zzzz);
	}

	return(0);
}

/*
 * Give some help information.
 */
int DoUsage()
{
printf(" usage: %s [-cdi] [-f configfile]\n", myName);
	return(1);
}

/*
 * Display the current STREAMS modules and driver
 * configuration information.
 */
int DumpSAD( p )
SAD *p;
{
	if (p) {
		int modules, devices;
		STR_CFG *scfg;
		int i;
		int startTime;
		
		startTime = time( (time_t *)0 );
		printf("\nSTREAMS Configuration Information...%s\n", ctime((time_t *)&startTime)); 
		printf("%15s%11s%8s%11s\n",
				"Name", "Type", "Major", "Module ID");
		printf("%15s%11s%8s%11s\n",
				"----", "----", "-----", "---------");

		printf("%15s %10s %7d %10d\n",
		        "clone", "",
			major(p->sad_blkhdr.sad_clonedev),
			minor(p->sad_blkhdr.sad_clonedev) );
		for (i = modules = devices = 0, scfg = &p->sad_cfg[0]; 
			i < p->sad_blkhdr.sad_cnt; 
			i++, scfg++) {

			if (scfg->scb_flags & SADF_DEVICE) {
 				if (!strcmp(scfg->scb_name,"xtisoUDG") ||
 				    !strcmp(scfg->scb_name,"xtisoUST") ||
 				    !strcmp(scfg->scb_name,"xtisoIDP") ||
 				    !strcmp(scfg->scb_name,"xtisoSPP"))
 					continue; 

				printf("%15s %10s %7d %10d\n", 
					scfg->scb_name,
					"device",
					major(scfg->scb_device),
					scfg->scb_mid );
					devices++;
			}
			else if (scfg->scb_flags & SADF_MODULE) {
				printf("%15s %10s %18d \n", 
					scfg->scb_name,
				        "module",
					scfg->scb_mid);
				modules++;
			} else {
				printf(" Entry %d, ???\n", i);
			}
		}
		printf("\n\tConfigured devices = %d, modules = %d\n\n", devices, modules);
	}

	return;
}

int Init( np )
char *np;
{
	char *p;

	/*
	 * Save a global copy of our proc name
	 */
	p = strrchr(np, '/');
	if (p == (char *)0) {
		p = np;
	} else {
		p++;
	}
	strcpy(myName, p);

	p = (char *)getenv("KINFO_DEVICE");
	if (p) {
		strcpy(&kinfoName[0], p);
	} else {
		strcpy(&kinfoName[0], &defaultName[0]);
	}
	
	p = (char *)getenv("KINFO_SIZE");
	if (p) {
		bufSize = atoi(p);
	}

	p = (char *)getenv("KINFO_LOOPS");
	if (p) {
		loopCount = atoi(p);
	}

	p = (char *)getenv("KINFO_SLEEP");
	if (p) {
		zzzz = atoi(p);
	}

	p = (char *)getenv("KINFO_HOLDOPEN");
	if (p) {
		holdOpenFlag = atoi(p);
	}

	return(0);
}

dev_t
find_kinfodev()
{
    int kmem;
    dev_t devno, clonedev;
    struct nlist nl[] = {
#define N_KINFODEV     0
#define N_CLONEDEV     1
      {"_kinfodev"},
      {"_clonedev"},
      "",
    };
#define BOOTEDFILELEN 80
    char bootedfile[BOOTEDFILELEN];

    /* get the name of the running kernel */
    bootedfile[0] = '/';
    getsysinfo(GSI_BOOTEDFILE, &bootedfile[1], BOOTEDFILELEN, 0, 0);
    if (nlist(bootedfile, nl) < 0 || nl[N_KINFODEV].n_type == 0) {
        sprintf(ebuf,"%s: can't get name list - make sure kernel is not stripped, also make sure that STRKINFO is built into the kernel", myName);
	perror(ebuf);
        exit(1);
    }
    if ((kmem = open(_PATH_KMEM, O_RDONLY)) < 0) {
        sprintf(ebuf, "%s: error on open %s", myName, _PATH_KMEM);
        perror(ebuf);
        exit(1);
    }
    lseek(kmem, nl[N_KINFODEV].n_value, L_SET);
    read(kmem, &devno, sizeof (dev_t));
    lseek(kmem, nl[N_CLONEDEV].n_value, L_SET);
    read(kmem, &clonedev, sizeof (dev_t));
    close(kmem);
    devno = makedev(major(clonedev), major(devno));
    return(devno);
}

struct conf_entry {
	struct conf_entry *next;	/* pointer to next one on list */
	char 	*name;			/* device name */
	char 	*filename;		/* name of file(s) to create */
	int	mode;			/* file access modes */
	int	major;			/* major device number assigned */
	int	clone;			/* clone device number */
	int	minorl;			/* lowest minor number */
	int 	minorh;			/* highest minor number */
	int	lineno;			/* line number in config file */
	int	found;			/* the device in configured */
	int     printed;                /* true if already printed out */
} *ce_list;


/*
 * Search the config file for a driver name matching the one passed.
 * If a config line for the driver is found, create all the needed
 * special device nodes.  Allow multiple config lines per device.
 */

config(driver,majordev,clonedevno)
char *driver;
int majordev,clonedevno;
{
    register struct conf_entry *ce;
    int status = FALSE;

    for(ce = ce_list; ce; ce = ce->next) {

	/* if the names match, then try to create the file(s) */
	if(strcmp(driver,ce->name))
	    continue;

	ce->found = 1;		/* mark it as configured */

	if(!iflag)
	    continue;

	/* make major/minor device numbers right for clones */
	if(ce->clone) {
	    ce->minorl = majordev;
	    ce->minorh = majordev;
	    ce->major = clonedevno;
	    ce->clone = clonedevno;
	} else {
	    ce->major = majordev;
	}

	/* make the file(s) */
	(void)docentry(ce,FALSE);
	status = TRUE;
    }
    return status;
}

/* Make sure a number string contains only valid chars */

checknum(c,s,line)
register char *c;
char *s;
int line;
{
    while(*c && *c != '\n')
	if(!isdigit(*c)) {
	    fprintf(stderr,"strsetup: %s on line %d\n",s,line);
	    return FALSE;
	}
	else
	    c++;
    return TRUE;
}

/* check to determine if the end of a line has been reached too early */

checkbad(c,s,line)
register char *c;
char *s;
int line;
{
    if(!*c) {
	fprintf(stderr,"strsetup: %s on line %d\n",s,line);
	return FALSE;
    }
    return TRUE;
}

/* information for converting a minor number to a Berkeley style tty name.
 * Borrowed from ttyname.c in libc.
 */
static struct ttymap {
	int  tm_minorl;		/* lowest minor number in range */
	int  tm_minorh;		/* highest minor number in range */
        char tm_base;           /* 9th character of ttyname     */
        char tm_len;            /* no. of 10th char variations  */
        char tm_last_chr;       /* 10th character of ttyname    */
} ttymap[] = {
           0,  175,  'p',  16, '0',  /* ttyp0 - ttyzf minors 1-175     */
         176,  223,  'a',  16, '0',  /* ttya0 - ttycf minors 176-223   */
         224,  399,  'e',  16, '0',  /* ttye0 - ttyof minors 224-399   */
         400,  815,  'A',  16, '0',  /* ttyA0 - ttyZf minors 400-815   */
         816, 1321,  'p',  46, 'g',  /* ttypg - ttyzZ minors 816-1321  */
        1322, 1459,  'a',  46, 'g',  /* ttyag - ttycZ minors 1322-1459 */
        1460, 1965,  'e',  46, 'g',  /* ttyeg - ttyoZ minors 1460-1965 */
        1966, 3161,  'A',  46, 'g',  /* ttyAg - ttyZZ minors 1966-3161 */
	   0,	 0,    0,   0,	 0,
};

#define MAXBSDPTY	3161

/* create the needed nodes.  All the nodes for each specified minor device
 * is created here.  If the file already exists, make sure it has the
 * currently correct major/minor.  If not, remove it and re-create it.
 * Make all needed directories in the path.
 */
docentry(ce,iord)
register struct conf_entry *ce;
int iord;			/* true to delete the files */
{
    register int i;
    register int bsdptyflag = 0;
    register char *c;
    dev_t  devno;
    struct stat sstat;
    char filespec[256];
    char filename[256];
    char fileprint[256];

    /* check to see if there is a format specification in the filename */
    c = (char *)strchr(ce->filename,'%');
    if(c) {
	c++;
	if(*c == 'B') {
	    /* make sure the minor numbers are in range */
	    if(ce->minorh > MAXBSDPTY) {
		fprintf(stderr,"%s: minor number out of range (0-%d) on line %d\n",
			myName,MAXBSDPTY,ce->lineno);
		return FALSE;
	    }
	    /* use BSD pty slave node name conversion */
	    *c = 0;
	    strcpy(filespec,ce->filename);
	    strcat(filespec,"c%c");
	    c++;
	    strcat(filespec,c);
	    bsdptyflag = 1;
	} else
	    strcpy(filespec,ce->filename);
    } else
	strcpy(filespec,ce->filename);
    for(i = ce->minorl; i <= ce->minorh; i++) {
	if(bsdptyflag) {
	    register struct ttymap *tm;
	    extern char bsd_tty_char();

	    tm = ttymap;
	    for(tm = ttymap; tm->tm_len; tm++)
		if(i >= tm->tm_minorl && i <= tm->tm_minorh)
			break;
	    sprintf(filename,filespec,
		bsd_tty_char(tm->tm_base, ((i - tm->tm_minorl) / tm->tm_len)),
		bsd_tty_char(tm->tm_last_chr, ((i - tm->tm_minorl) % tm->tm_len)));
	}
	else
	    sprintf(filename,filespec,i);

        if(iord == TRUE)
	    deletefile(filename);
	else
	    createfile(filename,ce,i);
    }
}

createfile(filename,ce,minor)
register char *filename;
register struct conf_entry *ce;
int minor;
{
    int printit = TRUE;
    register dev_t devno;
    struct stat sstat;
    char *c;
    char printfile[256];

    if(makedirs(filename) == FALSE)
	return FALSE;
    /* now create the node */
    devno = makedev(ce->major,minor);
    if (mknod(filename, ce->mode, devno)) {
       if (errno == EEXIST) {
	   stat(filename, &sstat);
	   if (sstat.st_rdev != devno) {
	       unlink(filename);
	       mknod(filename, ce->mode, devno);
	   } else {
	       printit = FALSE;
	   }
       } else {
	   sprintf(ebuf, "%s: Unable to mknod on %s",
		   myName, filename);
	   perror(ebuf);
	   printit = FALSE;
	   return FALSE;
       }
    }
    /*
     * print out the device -
     * If this is a range of devices only print one line for the
     * whole range.  This is so we don't fill up the screen when
     * creating a whole range of devices. 
     * example:
     *        For the range: /dev/pts/%d 
     *        only print one line:  /dev/pts/*
     */
    if ((printit == TRUE) && (!ce->printed)) {
       if (first) {
	   printf("\tThe following STREAMS devices were created:\n");
	   printf("%30s %10s %10s\n", "Name", "Major", "Minor");
	   printf("%30s %10s %10s\n", "----", "-----", "-----");
	   first = FALSE;
       }
       c = strchr(ce->filename, '%');
       if (c) {
	   strncpy(printfile, ce->filename, strlen(ce->filename) - strlen(c));
           printf("%30s %10d          *\n", printfile, ce->major);
       } else {
	   printf("%30s %10d %10d\n", filename,ce->major,minor);
       }
       ce->printed = 1;
   }
}

/* make sure all directories in the files path are created before trying
 * to create the file.
 */

makedirs(path)
char *path;
{
    register char *c;
    struct stat sb;

    c = path;
    if(*c == '/')
	c++;
    while(c = (char *)strchr(c,'/')) {
	*c = 0;
	if(stat(path,&sb) < 0) {
	     if(mkdir(path,S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH) == -1){
		   sprintf(ebuf, "%s: Unable to mkdir  %s",
			   myName, path);
		   perror(ebuf);
		   return FALSE;
	     }
	}
	*c = '/';
	c++;
    }
    return TRUE;
}

config_parse()
{
    register struct conf_entry *ce;
    register char *c;
    char *name, *file, *cmode, *cminor;
    int minorl, minorh, mode, majorno;
    int lineno;
    char line[256];
    int cloneit;
    int formats;

    /* must have an opened config file */
    if(cf == NULL)
	return FALSE;

    rewind(cf);
    for(lineno = 1; fgets(line,256,cf) != NULL; lineno++) {
	cloneit = 0;
	c = line;

	/* find first non-space character */
	while(*c && isspace(*c)) c++;
	if(*c == '#')	/* comment line */
		continue;
	if(!*c)		/* blank line */
		continue;

	/* find end of name string */
	name = c;
	while(*c && !isspace(*c)) c++;
	if(checkbad(c,"only name field found",lineno) == FALSE) continue;
	*c = 0;
	c++;
	if(checkbad(c,"only name field found",lineno) == FALSE) continue;

	/* find start of file name */
	while(*c && isspace(*c)) c++;
	if(checkbad(c,"no filename found",lineno) == FALSE) continue;

	/* find end of filename string */
	file = c;
	while(*c && !isspace(*c)) c++;
	if(checkbad(c,"only name and filename found",lineno) == FALSE) continue;
	*c = 0;
	c++;
	if(checkbad(c,"only name and filename found",lineno) == FALSE) continue;

	/* find start of mode string */
	while(*c && isspace(*c)) c++;
	if(checkbad(c,"no file mode found",lineno) == FALSE) continue;
	cmode = c;

	/* find end of mode string */
	while(*c && !isspace(*c)) c++;
	if(checkbad(c,"no minor number(s) found",lineno) == FALSE) continue;
	*c = 0;
	c++;
	if(checkbad(c,"no minor number(s) found",lineno) == FALSE) continue;
	sscanf(cmode,"%o",&mode);

	/* find start of minor number string */
	while(*c && isspace(*c)) c++;
	if(checkbad(c,"no minor number(s) found",lineno) == FALSE) continue;
	cminor = c;

	/* For clone devices use the clone major */
	if(strncmp(cminor,"clone",5) == 0) {
	    cloneit = 1;
	    minorl = minorh = 0;
	}
	else if(c = (char *)strchr(c,'-')) {
	    *c = 0;
	    /* sanity check */
	    if(checknum(cminor,"bad lower minor number",lineno) == FALSE)
		continue;
	    minorl = atoi(cminor);
	    c++;
	    if(checknum(c,"bad upper minor number",lineno) == FALSE)
		continue;
	    minorh = atoi(c);
	    if(minorh < minorl) {
		fprintf(stderr,"strsetup: bad minor range on line %d\n",lineno);
		continue;
	    }
	} else {
	    if(checknum(cminor,"bad minor number",lineno) == FALSE)
		continue;
	    minorl = atoi(cminor);
	    minorh = minorl;
	}

	/* do some sanity checking on any format specifications in the
	 * file name.
	 */
	formats = 0;
	c = (char *)strchr(file,'%');
	if(c) {
	    do {
		if(check_format(c) == TRUE)
		    formats++;
		else
		    formats = -10000;		/* invalid format */
		c++;
		if(*c == '%')
		    formats--;
	    } while(c = (char *)strchr(c,'%'));
	}
	if(formats < 0)
	    continue;
	if(formats > 1) {
	    fprintf(stderr,"%s: too many filename conversions on line %d\n",
		myName,lineno);
	    continue;
	}
	if(formats == 0 && (minorh - minorl) > 1) {
	    fprintf(stderr,"%s: minor range requires filename conversion specification at line %d\n",myName,lineno);
	    continue;
	}

	ce = (struct conf_entry *)malloc(sizeof(struct conf_entry) +
			strlen(name) + strlen(file) + 2);
	if(!ce) {
	    sprintf(ebuf, "%s: Unable to malloc",myName);
	    perror(ebuf);
	    while(ce = ce_list) {
		ce_list = ce->next;
		free(ce);
	    }
	    return FALSE;
	}
	ce->name = (char *)ce + sizeof(*ce);
	ce->filename = ce->name + strlen(name) + 1;
	strcpy(ce->name,name);
	strcpy(ce->filename,file);
	ce->minorl = minorl;
	ce->minorh = minorh;
	ce->mode = mode | S_IFCHR;
	ce->major = 0;
	ce->clone = cloneit;
	ce->next = ce_list;
	ce_list = ce;
	ce->lineno = lineno;
	ce->found = 0;
	ce->printed = 0;
    }
    return TRUE;
}

/* check for valid format specifiers.  Only accept those that
 * specify the printing of an integer.
 */

check_format(s)
register char *s;
{
    register char c;

    c = *s;
    s++;
    switch(*s) {
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	    if(c == '%') {
		fprintf(stderr,"%s: filename format must contain a '0' after the '%%'\n",myName);
		return FALSE;
	    }
	/* FALLTHROUGH */
	case '0':
	    return check_format(s);
	    break;
	case 'd':
	case 'x':
	case 'X':
	case 'o':
	case 'u':
	case 'i':
	case 'B':
	case '%':
	    return TRUE;
	    break;
	default:
	    fprintf(stderr,"%s: invalid filename format at char '%c'\n",
		    myName,*s);
	    return FALSE;
	    break;
	}
    return TRUE;
}

char
bsd_tty_char(base,offset)
char base;
int offset;
{
    if(base == '0')
	return "0123456789abcdef"[offset];
    if((base + offset) > 'z')
	return '@' + offset - ('z' - base);
    return base + offset;
}

/* Scan the list of configuration information to find any configurations
 * that have no currently associated driver.  For those found, remove
 * any files that the entry specifies.
 */
destroy()
{
    register struct conf_entry *ce;
    int status = FALSE;

    for(ce = ce_list; ce; ce = ce->next) {
	if(ce->found)		/* the driver is configured, keep looking */
	    continue;

	/* remove all files associated with the device */
	(void)docentry(ce,TRUE);
	status = TRUE;
    }
    return status;
}

/* Delete the file(s) described by the configuration entry. */
deletefile(filename)
register char *filename;
{
    int printit = TRUE;
    struct stat sstat;

    /* stat the file and then unlink it.  The stat is just to extract the
     * information needed to print the table of deleted devices.
     */
    if(stat(filename,&sstat) == -1)
	return;
    if(unlink(filename) == -1)
	return;
    if (first) {
       if(iflag)
	   printf("\n");
       printf("\tThe following devices were DELETED:\n");
       printf("%30s %10s %10s\n", "Name", "Major", "Minor");
       printf("%30s %10s %10s\n", "----", "-----", "-----");
       first = FALSE;
   }
   printf("%30s %10d %10d\n", filename,major(sstat.st_rdev),
		minor(sstat.st_rdev));
}
