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
static char	*sccsid = "@(#)$RCSfile: lpstat.c,v $ $Revision: 4.2.2.3 $ (DEC) $Date: 1993/10/08 15:14:45 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0.1
 */
#if !defined(lint) && !defined(_NOIDENT)
#endif
/*
 * lpstat -- Give various status information about the spooling system 
 *
 * This interfaces just to lpq, and lpc the "official" programs.
 * And does some lookups in printcap and /usr/spool... about trivial requests
 */

#include <sys/types.h>
#include <stdio.h>
#include <ctype.h>
#include <strings.h>
#include <locale.h>
#include <errno.h>
#include "lp.h"

#define TRUE 1
#define FALSE 0

#define LPQ "lpstat"
#define LPQPATH "/usr/bin/lpq"
#define LPC "lpstat"
#define LPCPATH "/usr/sbin/lpc"

#define DASH  '-'
#define COMMA ','
#define SPACE ' '

#define NAMESIZE 20
#define ID 1
#define JOB 0
#define MAXREQ 64

struct list {			/* linked list of USER or JOB names */
   struct list *next;
   short type; 
   char name[NAMESIZE+1];
};
typedef struct list List;

struct option {			/* linked list of options & switches ... */
    struct option *next;
    char   optChar;
    List   *selectors;		/* ... with optional selector list */
};
typedef struct option Option;

Option *OptionList;

short printSchedulerStatus,	/* Flags/Options */
    printSystemDefault,
    printSummary,
    printAll;

char line[BUFSIZ], pbuf[BUFSIZ/2], *bp = pbuf;

char *DefaultDest;		/* The default printers name */
void fatal();

#ifdef DEBUG
void showOptions();
#endif

int getOptions(int argc, char **argv);

int main(int argc, char **argv)
{
   void getSystemDefault(),
        showOutputStatusForUsers(),
        showSchedulerStatus(),
        showDevices(),
        showOutputStatus(),
        showPrinterStatus(),
        showAcceptanceStatus();

   Option *thisOption;

   (void) setlocale( LC_ALL, "" );
   catd = catopen(MF_PRINTER,NL_CAT_LOCALE);
   getSystemDefault();

   if (!getOptions(argc, argv)) {	/* No options, info for this user */
      List thisUser;
      char *getlogin();
      char *loginName = getlogin();

      if (loginName == NULL)
         loginName = (getpwuid(getuid()))->pw_name;

      thisUser.type = ID;
      strncpy(thisUser.name, loginName, NAMESIZE);
      thisUser.next = NULL;
      showOutputStatusForUsers(&thisUser);
      exit(0);
   }
	    
   if (printSystemDefault && !printSummary && !printAll)
      printf(MSGSTR(LPSTAT_1, "System default destination: %s\n"), DefaultDest);

   if (printSchedulerStatus && !printSummary && !printAll)
      showSchedulerStatus();

   if (printSummary || printAll) {
      printf(MSGSTR(LPSTAT_1, "System default destination: %s\n"), DefaultDest);
      showSchedulerStatus();
      showDevices((List *)NULL);

      if (printAll) {
         showAcceptanceStatus((List *)NULL);
         showOutputStatus((List *)NULL);
      }

      exit(0);				/* Showed everything already */
   }

   for (thisOption = OptionList; thisOption; thisOption = thisOption->next)
      if (thisOption->optChar == 'a')
         showAcceptanceStatus(thisOption->selectors);
      else if (thisOption->optChar == 'o')
         showOutputStatus(thisOption->selectors);
      else if (thisOption->optChar == 'p')
         showPrinterStatus(thisOption->selectors);
      else if (thisOption->optChar == 'u')
         showOutputStatusForUsers(thisOption->selectors);
      else if (thisOption->optChar == 'v')
         showDevices(thisOption->selectors);
      else if (thisOption->optChar == 'j')
         showOutputStatus(thisOption->selectors);

   exit(0);

   /* Not Reached */
   return(0);
}


int getOptions(int argc, char **argv)
{
   int opcount = 0;			/* option count */
   int drstcnt = 0;			/* count of options from {d,r,s,t} */
   int c;				/* current option flag */
   Option *currentOption;		/* current option list element */

   List *getList();
   List *getJobs();

   opterr = 0;				/* getopt() should be silent */

   while ((c = getopt(argc, argv, ":a:cdo:p:rstu:v:")) != -1) {
      switch (c) {
         case 'd':			/* system default destination */
            printSystemDefault = TRUE;
            opcount++;
            drstcnt++;
            break;

         case 'r':			/* request scheduler status */
            printSchedulerStatus = TRUE;
            opcount++;
            drstcnt++;
            break;

         case 's':			/* status summary */
            printSummary = TRUE;
            opcount++;
            drstcnt++;
            break;

         case 't':			/* all status information */
            printAll = TRUE;
            opcount++;
            drstcnt++;
            break;

         default:
            if (strchr("aopuv", optopt) == NULL) {
               usage();

               /* Not Reached */
               break;
            }

            optarg = "";

         /* fall thru if {a,o,p,u,v} switches without argument */

         case 'a':			/* acceptance status */
         case 'o':			/* output request status */
         case 'p':			/* printer status */
         case 'u':			/* user request status */
         case 'v':			/* printers and paths */
            currentOption = (Option *)malloc(sizeof(Option));

            if (currentOption == NULL)
               fatal(MSGSTR(LPSTAT_2, "Out of memory"));

            opcount++;
            currentOption->optChar = optopt;

            if (*optarg == '-') {	/* flag with argument */
               optind--;		/* backup and ... */
               optarg = "";		/* ... give getList() a null string */
            }

            currentOption->selectors = getList(optarg);
            currentOption->next = OptionList;
            OptionList = currentOption;
            break;

         case 'c':			/* class names (not implemented) */
            opcount++;
            fprintf(stderr, MSGSTR(LPSTAT_3, "Printer classes currently not supported\n"));
            break;
      }
   }

   if (argv[optind]) {			/* excess arguments */
      currentOption = (Option *)malloc(sizeof(Option));
      if (currentOption == NULL)
         fatal(MSGSTR(LPSTAT_2, "Out of memory"));

      opcount++;
      currentOption->optChar = 'j';
      currentOption->selectors = getJobs(optind, argc, argv);
      currentOption->next = OptionList;
      OptionList = currentOption;
   }

   if (drstcnt > 1)			/* conflicting switches */
      usage();

#ifdef DEBUG
   (void)showOptions();
#endif

   return(opcount);
}    
	

/*
 * getList takes a list consisting of usernames, printernames or
 * jobnumbers separated by commas and/or space and puts it into
 * a List structure for further use.
 */        

List *getList(char *text)
{
   char *tp = text;
   List *head = (List *)NULL;
   List *tail = (List *)NULL;
   List *currentElement;
   int i;
   int elementType;

   while (*tp) {
      /*
       * process the entire text string
       */
      currentElement = (List *)malloc(sizeof(List));
      if (currentElement == NULL)
         fatal(MSGSTR(LPSTAT_2, "Out of memory"));

      i = 0;
      elementType = JOB;

      while(*tp && *tp != COMMA && *tp != SPACE) {
         /*
          * within one specifier
          */
         if (!isdigit(*tp))
            elementType = ID;

         currentElement->name[i++] = *tp++;

         if (i == NAMESIZE)		/* skip & drop the rest */
            while (*tp && *tp != COMMA && *tp != SPACE)
               tp++;
      }

      currentElement->name[i] = '\0';
      currentElement->type = elementType;
      currentElement->next = (List *)NULL;

      if (head)
         tail->next = currentElement;
      else
         head = currentElement;

      tail = currentElement;

      while (*tp && (*tp == COMMA || *tp == SPACE))	/* skip separators */
         tp++;
   }

   return head;
}


/*
 * getJobs takes the arguments remaining after all of the
 * option flags have been parsed and puts them into a List
 * structure for further use.
 */

List *getJobs(int lastop, int argc, char **argv)
{
   char *ap;
   List *head = (List *)NULL;
   List *tail = (List *)NULL;
   List *currentElement;
   int i, k;

   for (i = lastop; argc > i; i++) {
      currentElement = (List *)malloc(sizeof(List));
      if (currentElement == NULL)
         fatal(MSGSTR(LPSTAT_2, "Out of memory"));

      ap = argv[i];
      k = 0;

      while (*ap) {
         currentElement->name[k++] = *ap++;

         if (k == NAMESIZE)
            while (*ap)
               ap++;
      }

      currentElement->name[k] = '\0';
      currentElement->type = JOB;
      currentElement->next = (List *)NULL;

      if (head)
         tail->next = currentElement;
      else
         head = currentElement;

      tail = currentElement;
   }

   return head;
}


void getSystemDefault()
{
   char *getenv();
      
   DefaultDest = getenv("LPDEST");
   if (DefaultDest == NULL || strlen(DefaultDest) == 0)
      DefaultDest = getenv("PRINTER");

   if (DefaultDest == NULL || strlen(DefaultDest) == 0)
      DefaultDest = DEFLP;
}


void showOutputStatusForUsers(List *users)
{
   char **args;
   char *Pflag;
   int i;

   if (users == NULL) {
      void showOutputStatus();
      showOutputStatus((List *)NULL);
      return;
   }

   args = (char **)malloc((3 + len(users)) * sizeof(char *));

   if (args == NULL)
      fatal(MSGSTR(LPSTAT_2, "Out of memory"));

   args[0] = LPQ;

   Pflag = (char *)malloc(3 + strlen(DefaultDest));
   if (!Pflag)
      fatal(MSGSTR(LPSTAT_2, "Out of memory"));

   sprintf(Pflag, "-P%s", DefaultDest);

   args[1] = Pflag;

   i = 2;
   while (users) {
      args[i++] = users->name;
      users = users->next;
   }

   args[i] = NULL;

   if (sFork())
      wait(NULL);			/* let the output finish */
   else
      execv(LPQPATH, args);
}


void showSchedulerStatus()
{
   int lfd;
 
   (void) umask(0);
   lfd = open(MASTERLOCK, O_RDONLY);
   errno = 0;

   if (lfd < 0 || flock(lfd, LOCK_SH|LOCK_NB) == 0)
      printf(MSGSTR(LPSTAT_4, "Scheduler is not running\n"));
   else if (errno == EWOULDBLOCK)
      printf(MSGSTR(LPSTAT_5, "Scheduler is running\n"));
   else
      printf(MSGSTR(LPSTAT_27, "Cannot lock %s\n"), MASTERLOCK);

   close(lfd);
}


/*
 * showDevices interrogates the /etc/printcap file and shows the
 * device for each printer specified in the argument list.  If no
 * arguments are specified, all printers listed in the printcap
 * file are shown sequentially.
 */

void showDevices(List *lst)
{
   char *lp, *rm, *rp;

   if (lst == NULL) {
      /*
       * No selectors, show all printers
       */
      while (getprent(line) > 0) {
         /*
          * for all printers in the printcap file
          */
         char *cp = line;		/* grab printer's first name */
         bp = pbuf;

         while (*cp != '|' && *cp != ':' && *cp != '\0')
            *bp++ = *cp++;

         *bp = '\0';
         printf(MSGSTR(LPSTAT_6, "Output for printer %s is sent to "), pbuf);

         bp = pbuf;			/* extract the device info */
         lp = pgetstr("lp", &bp);
         rm = pgetstr("rm", &bp);
         rp = pgetstr("rp", &bp);

         if (rm && strlen(rm) != 0)
            printf(MSGSTR(LPSTAT_7, "remote printer %s on %s\n"), rp, rm);
         else if (lp && strlen(lp) != 0)
            printf("%s\n", lp);
         else
            printf(MSGSTR(LPSTAT_8, "NOWHERE. Error in printcap file\n"));
      }

      endprent();			/* close the printcap file */
   }
   else
      while (lst) {
         /*
          * for each printer in the argument list
          */
         if (pgetent(line, lst->name) > 0) {
            printf(MSGSTR(LPSTAT_6, "Output for printer %s is sent to "), lst->name);

            bp = pbuf;			/* extract the device info */
            lp = pgetstr("lp", &bp);
            rm = pgetstr("rm", &bp);
            rp = pgetstr("rp", &bp);

            if (rm && strlen(rm) != 0)
               printf(MSGSTR(LPSTAT_7, "remote printer %s on %s\n"), rp, rm);
            else if (lp && strlen(lp) != 0)
               printf("%s\n", lp);
            else
               printf(MSGSTR(LPSTAT_8, "NOWHERE. Error in printcap file\n"));
         }
         else
            fprintf(stderr, MSGSTR(LPSTAT_9, "unknown printer: %s\n"), lst->name);

         lst = lst->next;
      }
}


void showAcceptanceStatus(List *lst)
{
   if (lst == NULL)
      /*
       * No selectors, show acceptance status of all printers
       */
      if (sFork())
         wait(NULL);			/* let the output finish */
      else
         execl(LPCPATH, LPC, "status", NULL);
   else {
      /*
       * show acceptance status of all printers in the argument list
       */
      int i = 2;

      char **args = (char **)malloc((3+len(lst)) * sizeof(char *));
      if (args == NULL)
         fatal(MSGSTR(LPSTAT_2, "Out of memory"));

      args[0] = LPC;
      args[1] = "status";

      while (lst) {
         args[i++] = lst->name;
         lst = lst->next;
      }

      args[i] = NULL;

      if (sFork())
         wait(NULL);			/* let the output finish */
      else
         execv(LPCPATH, args);
   }
}


void showOutputStatus(List *lst)
{
   char Pflag[BUFSIZ/2 + 3];		/* -P flag argument buffer */
   char *pname;
   int i;

   if (lst == NULL) {
      /*
       * No selectors, show output status of all printers
       */
      while (getprent(line) > 0) {
         /*
          * for each printer in the printcap file
          */
         char *cp = line;		/* grab printer's first name */
         bp = pbuf;

         while (*cp != '|' && *cp != ':' && *cp != '\0')
            *bp++ = *cp++;

         *bp = '\0';
         printf(MSGSTR(LPSTAT_10, "Requests on %s:\n"), pbuf);

         if (sFork())
            wait(NULL);			/* let the output finish */
         else {
            sprintf(Pflag, "-P%s", pbuf);
            execl(LPQPATH, LPQ, Pflag, NULL);
         }

         printf("\n");
      }

      endprent();			/* close the printcap file */
   }
   else {
      char **args = (char **)malloc((MAXREQ + 3) * sizeof(char *));
      if (args == NULL)
         fatal(MSGSTR(LPSTAT_2, "Out of memory"));

      while (lst) {
         /*
          * process the entire argument list
          */
         i = 0;
         args[i++] = LPQ;

         if (lst->type == ID) {			/* printer name */
            pname = lst->name;
            lst = lst->next;
         }
         else					/* job# */
            pname = DefaultDest;

         sprintf(Pflag, "-P%s", pname);
         args[i++] = Pflag;

         while (lst) {
            /*
             * get all job#'s for this printer
             */
            if (lst->type != JOB)
               break;

            args[i++] = lst->name;
            lst = lst->next;
         }

         args[i++] = NULL;

         printf(MSGSTR(LPSTAT_10, "Requests on %s:\n"), pname);

         if (sFork())
            wait(NULL);			/* let the output finish */
         else
            execv(LPQPATH, args);
      }

      free(args);			/* release memory */
   }
}


void showPrinterStatus(List *lst)
{
   void showOne();

   if (lst == NULL) {
      /*
       * No selectors, show the status of all printers
       */
      while (getprent(line) > 0) {
         /*
          * for each printer in the printcap file
          */
         char *cp = line;		/* grab printer's first name */
         bp = pbuf;

         while (*cp != '|' && *cp != ':' && *cp != '\0')
            *bp++ = *cp++;
         *bp = '\0';

         printf("%s: \n", pbuf);
         showOne();
      }

      endprent();
   }
   else
      while (lst) {
         /*
          * for each printer in the argument list
          */
         if (pgetent(line, lst->name) > 0) {
            printf("%s: \n", lst->name);
            showOne();
         }
         else
            fprintf(stderr, MSGSTR(LPSTAT_9, "unknown printer: %s\n"), lst->name);

         lst = lst->next;
      }
}
	

void showOne()
{
   struct stat stbuf;
   char *sd, *lo;

   bp = pbuf;

   if ((sd = pgetstr("sd", &bp)) == NULL)
      sd = DEFSPOOL;

   if ((lo = pgetstr("lo", &bp)) == NULL)
      lo = DEFLOCK;
    
   sprintf(line, "%s/%s", sd, lo);
   if (stat(line, &stbuf) >= 0) {
      printf(MSGSTR(LPSTAT_11, "Queuing is %s\n"),
             (stbuf.st_mode & 010) ? MSGSTR(LPSTAT_12, "disabled") : MSGSTR(LPSTAT_13, "enabled"));

      printf(MSGSTR(LPSTAT_14, "Printing is %s\n"),
             (stbuf.st_mode & 0100) ? MSGSTR(LPSTAT_12, "disabled") : MSGSTR(LPSTAT_13, "enabled"));
    }
    else
	fprintf(stderr, MSGSTR(LPSTAT_15, "Cannot stat %s\n"), line);
}


int len(List *lst)
{
   int i = 0;

   while (lst) {
      i++;
      lst = lst->next;
   }

   return i;
}


void fatal(char *s)
{
   fputs(s, stderr);
   fputc('\n', stderr);
   exit(1);
}

#ifdef DEBUG /* ------------------------------------------------------------- */

void printList(List *lst)
{
   while (lst) {
      printf(MSGSTR(LPSTAT_16, "Type: %s\tText: %s\n"), 
             lst->type ? MSGSTR(LPSTAT_17, "ID") : MSGSTR(LPSTAT_18, "JOB"),
             lst->name);
      lst = lst->next;
   }
   printf("\n\n");
}

void showOptions()
{
   Option *ol = OptionList;

   if (printSchedulerStatus)
      printf(MSGSTR(LPSTAT_19, "SchedulerStatus\n"));
   if (printSystemDefault)
      printf(MSGSTR(LPSTAT_20, "SystemDefault\n"));
   if (printSummary)
      printf(MSGSTR(LPSTAT_21, "Summary\n"));
   if (printAll)
      printf(MSGSTR(LPSTAT_22, "All\n"));

   while (ol) {
      printf(MSGSTR(LPSTAT_23, "Optionchar = %c\n"), ol->optChar);
      printList(ol->selectors);
      ol = ol->next;
   }
}
#endif /* DEBUG ------------------------------------------------------------- */

int sFork()
{
   int fstat = fork();

   if (fstat == -1) {
      fatal(MSGSTR(LPSTAT_25, "Can't fork!"));
      exit(1);
   }
   return fstat;
}

usage()
{
   fprintf(stderr, MSGSTR(LPSTAT_26, "Usage: lpstat [-a[list]] [-d | -r | -s | -t] [-o[list]]\n\t [-p[list]] [-u[list]] [-v[list]] [ID...]\n"));
   exit(1);
}
