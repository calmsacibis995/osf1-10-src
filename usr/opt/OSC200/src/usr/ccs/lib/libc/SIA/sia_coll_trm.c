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
static char *rcsid = "@(#)$RCSfile: sia_coll_trm.c,v $ $Revision: 1.1.14.4 $ (DEC) $Date: 1993/08/04 21:20:39 $";
#endif
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if defined(_SHARED_LIBRARIES) || (!defined(_SHARED_LIBRARIES) && !defined(_THREAD_SAFE))
#pragma weak sia_collect_trm = __sia_collect_trm
#endif
#if !defined(_THREAD_SAFE)
#pragma weak sia_sig = __sia_sig
#pragma weak sia_timeout = __sia_timeout
#endif
#endif
#ifdef getpass
#undef getpass
#endif

#include <termios.h>
#include "siad.h"
#include "siad_bsd.h"
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#define TRUE 1
#define FALSE 0
#define MAX_BUFF 100
#define SIG_INTR 1
#define SIG_TIME 2

typedef enum { False, True } Boolean;
static char *next_token (char *ptr, int *len, char **next_ret);
static int debug=0;

#ifdef _THREAD_SAFE
extern int sia_sig;
extern int sia_timeout;
#else
int 	sia_sig=0;
int	sia_timeout=0;
#endif
sig_t	sia_sighupsav;
sig_t	sia_sigintsav;
sig_t	sia_sigquitsav;
sig_t	sia_sigtimesav;

/*
 * Internal function to safely read a complete line and truncate to proper
 * size.  DAL003.
 */
static char *getline(const char *string, int size, FILE *stream)
{
	extern int errno;
	int c;
	char *cp;

	errno = 0;
	if(!fgets(string, size, stream) || ferror(stream) || errno == EINTR)
		return NULL;
	else
		if(cp=index(string, '\n'))
			*cp = '\0';
		else
			while((c=getc(stdin)) != '\n' && c != EOF && errno != EINTR)
				;
	return string;
}

/*******  sia_collect_trm is used to collect from terminals ********/
/*******  sia_collect_wks is used to collect from workstations *****/
int
  sia_collect_trm (int timeout, int rendition, unsigned char *title, int num_prompts, prompt_t *prompts)
{
  
  if (check_params (timeout, rendition, title, num_prompts, prompts) == -1)
    return (SIACOLPARAMS);
	
  sia_timeout=timeout;
  setsig();
  switch (rendition) 
    {
    case SIAMENUONE: 
      if (sia_menu_any (1, title, num_prompts, prompts) == -1) {
	restorsig();
	if (sia_sig == 0 )
		return (SIACOLTIMOUT);
	else	return (SIACOLABORT);
      }
      break;
    case SIAMENUANY:
      if (sia_menu_any (0, title, num_prompts, prompts) == -1){
	restorsig();
	if (sia_sig == 0 )
		return (SIACOLTIMOUT);
	else	return (SIACOLABORT);
      }
      break;
    case SIAFORM:
      if (sia_form (title, num_prompts, prompts) == -1){
	restorsig();
	if (sia_sig == 0 )
		return (SIACOLTIMOUT);
	else	return (SIACOLABORT);
      }
      break;
    case SIAONELINER:
      if (sia_one_liner (title, prompts) == -1){
	restorsig();
	if (sia_sig == 0 )
		return (SIACOLTIMOUT);
	else	return (SIACOLABORT);
      }
      break;
    case SIAINFO: case SIAWARNING:
      sia_info (title, prompts);
      restorsig();
      break;
    default:
      restorsig();
      return (SIACOLPARAMS);
    }
  restorsig();
  return (SIACOLSUCCESS);
}

static
check_params (int timeout, int rendition, char *title, int num_prompts, 
	      prompt_t *prompts)
{
  int i;

  if (timeout < 0) {
	if (debug) printf("timeout too small\n");
	return (-1);
  }
  if (rendition < 1 || rendition > LAST_RENDITION) {
	if (debug) printf("rendition out of bounds\n");
	return (-1);
  }
  if (rendition > 0 && rendition <= SIAONELINER) {
    if (num_prompts < 1 || num_prompts > MAX_PROMPTS) {
	if (debug) printf ("num_prompts out of bounds\n");
        return (-1);
    }
    for (i=0; i<num_prompts; i++) {
      if (prompts[i].prompt == (unsigned char *) NULL) {
	if (debug) printf ("prompt is a NULL pointer, i=%d\n",i);
	return (-1);
	}
    }
  }

  if (rendition == SIAONELINER || rendition == SIAFORM) {
    for (i=0; i<num_prompts; i++) {

      if (prompts[i].result == (unsigned char *) NULL) {
	if (debug) printf ("result is a NULL pointer, i=%d\n",i);
	return(-1);
      }

      if (prompts[i].min_result_length >= prompts[i].max_result_length) {
	if (debug) printf ("min result length not less than max result length, i=%d\n", i); 
	return(-1);
      }
      
      if (prompts[i].control_flags < 0  
	  || prompts[i].control_flags > FLAG_MAX) {
	if (debug) printf ("control flags out of bounds\n");
        return(-1);
      }
    }
  }
  return(1);
}

static 
setsig ()
{
  void *collect_timeout(),
  *collect_interrupt();

  sia_sig=0;	/* set and save signal handlers */
  sia_sighupsav = signal (SIGHUP, collect_interrupt);
  sia_sigintsav = signal (SIGINT, collect_interrupt);
  sia_sigquitsav = signal (SIGQUIT, collect_interrupt);
  sia_sigtimesav = signal (SIGALRM, collect_timeout);
  if(sia_timeout)
	alarm ((unsigned) sia_timeout);
}

static
restorsig ()
{
  if(sia_timeout)
        alarm(0);
  (void) signal (SIGALRM, (void (*)(int))sia_sigtimesav);
  (void) signal(SIGHUP, (void (*)(int))sia_sighupsav);
  (void) signal(SIGINT, (void (*)(int))sia_sigintsav);
  (void) signal(SIGQUIT, (void (*)(int))sia_sigquitsav);
}

static void *
collect_timeout ()
{
  sia_sig = SIG_TIME;
  if (debug) printf ("\nSIA collect timed out.\n");
}

static void *
collect_interrupt ()
{
  if(sia_timeout) 
	alarm(0);
  sia_sig = SIG_INTR;	/* restore signal handlers */
  if (debug) printf ("\nSIA collect aborted.\n");
}

/* getpass function adapted from libc.  -DAL001 */
static char *
getpass(prompt, result, maxlen)
char	*prompt, *result;
int maxlen;
{
	struct termios ttyb;
	char     savel;
	tcflag_t flags;
	register char *p;
	register int c;
	FILE	*fi;

	if((fi = fopen("/dev/tty", "r+")) == NULL)
		return((char*)NULL);
	else
		setbuf(fi, (char*)NULL);
	(void) tcgetattr(fileno(fi), &ttyb);
	flags = ttyb.c_lflag;
	ttyb.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL);
	(void) tcsetattr(fileno(fi), TCSAFLUSH, &ttyb);
	(void) fputs(prompt, stderr);

	for(p=result; sia_sig == 0 && (c = getc(fi)) != '\n' && c != EOF; ) {
		if(p < &result[maxlen])
			*p++ = c;
	}
	*p = '\0';
	(void) putc('\n', stderr);
	ttyb.c_lflag = flags;
	(void) tcsetattr(fileno(fi), TCSADRAIN, &ttyb);
	if(fi != stdin)
		(void) fclose(fi);
	if(sia_sig == 0)
		return result;
	else
		return (char *) 0;
}

static int 
sia_menu_any (int type, char *title, int num_prompts, prompt_t *prompt)
{
  int item [MAX_PROMPTS],
      len[MAX_PROMPTS],
      i, j,
      not_done = TRUE,
      num_sel = 1,
      more_tokens,
      first_illegal,
      illegal,
      this_one_ok;
  long selptr;

  char selection [MAX_BUFF],
       *next,
       *token,
       *ptr,
       sel[MAX_PROMPTS][MAX_BUFF];
       
  while (not_done) {
    printf ("\n%s\n\n", title);         

    for ( i = 0; i < num_prompts; i++) {
      printf ("%d  %s\n", i+1, prompt[i].prompt);  /* print prompt */
      prompt[i].result = (unsigned char *) NULL; /* initialize result */
    }

    if (type == 1)
	printf("\nSelect ONE item by number: ");
    else
	if (num_prompts > 1)
	  printf("\nSelect items by number [Example: 1 %d]: ", num_prompts);
        else
          printf("\nSelect item by number [Example: 1]: ");

	if(getline(selection, sizeof selection, stdin) == (char *) NULL)	/*DAL003*/
	{
	sia_sig=-1;
        return (-1);
	}
    
    /* Initialize */
    for (i=0; i<MAX_PROMPTS; i++) {        
      strcpy (sel[i], "0");
      item[i] = len[i] = 0;
    }
    illegal=FALSE;
    first_illegal=TRUE;
    more_tokens=TRUE;
    num_sel = 1;

   /* Gather tokens into sel[] array; set num_sel */
    while (more_tokens) {
      token = next_token (selection, &len[num_sel-1], &next);
      if (token == NULL) {
	num_sel--;
	more_tokens=FALSE;
      } else {
	strncpy (sel[num_sel-1], token, len[num_sel-1]);
	strcpy (selection, next);
	num_sel++;
      }
    }

    if (type == 1 && num_sel != 1)
       printf ("\nPlease select exactly ONE item\n");
    else { 
      /* Validate tokens */
      for (i=0; i < num_sel ; i++) {
        this_one_ok = TRUE;
        item[i]=strtoul (sel[i], &ptr, 0);
	selptr = (long) sel + ((i) * MAX_BUFF);
	if (selptr + (long) len[i] != (long) ptr) {
	  print_illegal (len[i], sel[i], &illegal, &first_illegal, type, 
			 &this_one_ok);
	} else if ((item[i] < 1) || (item[i] > num_prompts)) {
	  print_illegal (len[i], sel[i], &illegal, &first_illegal, type, 
			 &this_one_ok);
	} else {
	  for (j=0; j<i; j++) {
	    if (item[i] == item[j]) {
	      print_illegal (len[i], sel[i], &illegal, &first_illegal, type, 
			     &this_one_ok);
	      break;
	    }
          }	
	}
	if (this_one_ok) prompt[item[i]-1].result = (unsigned char *) "1";
      }
    if (!illegal) {
      not_done = FALSE;
    } else {
      printf("\n");
      for (i=0; i< num_prompts; i++)
       prompt[i].result = (unsigned char *) NULL;
    }
  }

  }
  if (type == 0 && num_sel == 1)
     printf ("\nYou have selected 1 item:\n");
  else if (type == 0)
     printf("\nYou have selected %d items:\n", num_sel);
  else
     printf("\nYou have selected:\n");

  for (i = 0; i < num_sel; i++) {
    printf ("%s\n", prompt[item[i]-1].prompt);
  }
  printf("\n");
  return(1);
}

static int 
   sia_form(char *title, int num_prompts, prompt_t *prompt)
{
  int i;

  if ((title != (char *) NULL) && (strlen(title) != 0))
        printf ("\n%s\n\n", title);                

  for (i=0; i<num_prompts; i++) {
    if (sia_one_liner ((char *) NULL, &prompt[i]) == -1) 
	return (-1);
  }
  return (1);
}

static int 
sia_one_liner (char *title, prompt_t *prompt)
{
  char result[MAX_BUFF];
  int not_done = TRUE,
	invisible = FALSE,
	i;

  if ((title != (char *) NULL) && (strlen(title) != 0))
	printf ("\n%s\n\n", title);                

  if ((prompt[0].control_flags & SIARESINVIS) == SIARESINVIS) {
    invisible = TRUE;
  }
    
  while (not_done) {
    not_done = FALSE;
	if(invisible) {
/* Use getpass instead of hardcoded termio calls. -DAL001 */
		if(!getpass(prompt[0].prompt, result, sizeof result)) {
			sia_sig = -1;
			return -1;
		}
	} else {
		fputs(prompt[0].prompt, stdout);
		if(getline(result,sizeof result,stdin) == (char *) NULL)	/*DAL003*/
		{
			sia_sig=-1;
			return (-1);
		}
	}

    if (strlen(result) > prompt[0].max_result_length) {
      if (debug) printf ("Response too long.\n");
/*
      not_done = TRUE;
DAL002 */
	result[prompt[0].max_result_length] = '\0';	/* DAL002 */
    } else if (strlen(result) < prompt[0].min_result_length) {
      if (debug) printf ("Response too short.\n");
      not_done = TRUE;
    } else if ((prompt[0].control_flags & SIAALPHA) == SIAALPHA) {
      for (i=0; i < strlen(result); i++) {
	if (!isalpha(*(result + i))) {
	  if (debug) printf("Response must contain only letters.\n");
	  not_done = TRUE;
	  break;
	}
      }
   } else if ((prompt[0].control_flags & SIANUMBER) == SIANUMBER) {
      for (i=0; i < strlen(result); i++) {
	if (!isdigit(*(result + i))) {
	  if (debug) printf("Response must contain only numbers.\n");
	  not_done = TRUE;
	  break;
	}
      }
    } else if ((prompt[0].control_flags & SIAALPHANUM) == SIAALPHANUM) {
      for (i=0; i < strlen(result); i++) {
	if (!isalnum(*(result + i))) {
	  if (debug) printf("Response must contain only alphanumeric characters.\n");
	  not_done = TRUE;
	  break;
	}
      }
    } else if ((prompt[0].control_flags & SIAPRINTABLE) == SIAPRINTABLE) {
      for (i=0; i < strlen(result); i++) {
	if (!isprint(*(result + i))) {
	  if (debug) printf("Response must contain only printable characters.\n");
	  not_done = TRUE;
	  break;
        }
      } 
    }
  }
 strcpy (prompt[0].result, result); 
 
  return(1);
}

static int 
sia_info (char *title, prompt_t *prompt)
{
  if (title && *title)
    printf ("\n%s\n\n", title);               

  printf ("%s\n", prompt->prompt);
  
  return(1);
}


static char *next_token (char *ptr, int *len, char **next_ret)
{
  char *ret;

  ret = ptr;
  while (isspace(*ret) && (*ret != '\0')) ret++;
  if (*ret == '\0') return (NULL);
  ptr = ret;
  while (!isspace(*ptr) && (*ptr != '\0')) ptr++;
  if (len) *len = ptr - ret;
  if (next_ret) *next_ret = ptr;
  return(ret);
}

static int print_illegal (int len, char *sel, int *illeg, int *first, int type,
			  int *ok)
{
  if (*first == TRUE) {
	    *first = FALSE;
            if (type == 1)
	      printf ("\n** Invalid choice: %.*s ", len, sel);
            else
	      printf ("\n** Invalid choice(s): %.*s ", len, sel);
	  } else {
	    printf ("%.*s ", len, sel);
	  }
	  *illeg=TRUE;
          *ok = FALSE;
}
