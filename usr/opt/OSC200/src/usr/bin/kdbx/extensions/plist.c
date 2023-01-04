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
static char *rcsid = "@(#)$RCSfile: plist.c,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/03/16 19:02:37 $";
#endif


# include "krash.h"

/*********** FROM kern/task.h ***************/
/* task swap_state values */
#define TASK_INSWAPPED          0x01
#define TASK_OUTSWAPPED         0x02
#define TASK_COMMING_IN         0x03
#define TASK_GOING_OUT          0x04

/* task swap_request values */
#define TASK_ALL_SET            0x01
#define TASK_WANTS_IN           0x02

/*********** FROM kern/thread.h **************/
/*
 *      Thread states [bits or'ed]
 */
#define TH_WAIT                 0x01    /* thread is queued for waiting */
#define TH_SUSP                 0x02    /* thread has been asked to stop */
#define TH_RUN                  0x04    /* thread is running or on runq */
#define TH_SWAPPED              0x08    /* thread is swapped out */
#define TH_IDLE                 0x10    /* thread is an idle thread */

/*********** FROM kern/thread_swap.h *********/
#define TH_SW_UNSWAPPABLE       1       /* not swappable */
#define TH_SW_IN                2       /* swapped in */
#define TH_SW_GOING_OUT         3       /* being swapped out */
#define TH_SW_WANT_IN           4       /* being swapped out, but should
                                           immediately be swapped in */
#define TH_SW_OUT               5       /* swapped out */
#define TH_SW_COMING_IN         6       /* queued for swapin, or being
                                           swapped in */
/*************************************************************/

static char *help_string =
"plist - print a list of processes with status information \\\n\
    Usage : plist [your favorite set of options] \\\n\
        -all        selects tasks on ALL queues \\\n\
        -in         selects tasks on the in-swapped queue \\\n\
        -inwork     selects tasks on the in-swap-work queue \\\n\
        -out        selects tasks on the out-swapped queue \\\n\
        -outwork    selects tasks on the out-swap-work queue \\\n\
        -stable     selects tasks on the in/out-swapped queues \\\n\
        -work       selects tasks on the in/out-swap-work queues \\\n\
\\\n\
        -full       includes FULL output \\\n\
        -mappings   includes address space information \\\n\
        -summary    includes a summary of the various swap queues \\\n\
        -threads    includes all threads \\\n\
";

FieldRec task_fields[] = {
  {".ref_count",     			NUMBER, NULL, NULL},
  {".map",           			NUMBER, NULL, NULL},
  {".thread_list.next",   		NUMBER, NULL, NULL},
  {".thread_count",  			NUMBER, NULL, NULL},
  {".proc_index",    			NUMBER, NULL, NULL},
  {".task_link.next", 			NUMBER, NULL, NULL},
  {".swap_state",    			NUMBER, NULL, NULL},
  {".swap_request",  			NUMBER, NULL, NULL},
  {".outswap_stamp", 			NUMBER, NULL, NULL},
  {".inswap_stamp",  			NUMBER, NULL, NULL},
  {".working_set",   			NUMBER, NULL, NULL},
  {".u_address",     			NUMBER, NULL, NULL}};
#define NUM_TASK_FIELDS (sizeof(task_fields)/sizeof(task_fields[0]))

FieldRec thread_fields[] = {
  {".thread_list.next",    		NUMBER, NULL, NULL},
  {".ref_count",     			NUMBER, NULL, NULL},
  {".kernel_stack",  			NUMBER, NULL, NULL},
  {".swap_state",    			NUMBER, NULL, NULL},
  {".state",         			NUMBER, NULL, NULL}};
#define NUM_THREAD_FIELDS (sizeof(thread_fields)/sizeof(thread_fields[0]))

FieldRec utask_fields[] = {
  {".uu_comm",       			STRING, NULL, NULL},
  {".uu_logname",    			STRING, NULL, NULL},
  {".uu_tsize",      			NUMBER, NULL, NULL},
  {".uu_dsize",      			NUMBER, NULL, NULL},
  {".uu_ssize",      			NUMBER, NULL, NULL},
  {".uu_text_start", 			NUMBER, NULL, NULL},
  {".uu_data_start", 			NUMBER, NULL, NULL},
  {".uu_stack_start", 			NUMBER, NULL, NULL},
  {".uu_stack_end",   			NUMBER, NULL, NULL}};
#define NUM_UTASK_FIELDS  (sizeof(utask_fields)/sizeof(utask_fields[0]))
 
FieldRec vm_map_fields[] = {
  {".vm_nentries",    			NUMBER, NULL, NULL},
  {".vm_ops",         			NUMBER, NULL, NULL},
  {".vm_size",        			NUMBER, NULL, NULL},
  {".vm_pmap",        			NUMBER, NULL, NULL},
  {".vm_ref_count",   			NUMBER, NULL, NULL},
  {".vm_hint",        			NUMBER, NULL, NULL},
  {".vm_first_free",  			NUMBER, NULL, NULL},
  {".vm_private",     			NUMBER, NULL, NULL},
  {".vm_res_count",   			NUMBER, NULL, NULL},
  {".vm_fault_rate",  			NUMBER, NULL, NULL},
  {".vm_pagefaults",  			NUMBER, NULL, NULL},
  {".vm_faultrate_time", 		NUMBER, NULL, NULL}};
#define NUM_VM_MAP_FIELDS (sizeof(vm_map_fields)/sizeof(vm_map_fields[0]))


/*
** plist -- display process(task)/thread/mapping information of processes
**          on the various swap lists.
*/
main(int argc, char **argv)
{
  char  buf[256];
  char *error;
  DataStruct header_address;
  int   i;
  int   inswapped_desired    = 0;
  int   inswap_work_desired  = 0;
  int   mappings_desired     = 0;
  int   outswapped_desired   = 0;
  int   outswap_work_desired = 0;
  long  pointer;
  int   summary_desired      = 0;
  int   threads_desired      = 0;
  long   _task_inswapped_queue_count;
  long  _task_inswap_work_queue_count;
  long  _task_outswapped_queue_count;
  long  _task_outswap_work_queue_count;

  /*
  ** Check the arguments. `check_args()' will scan for `-help'.  The
  ** remaining options are checked to ensure that they are unique up
  ** to at least three characters (there ARE exceptions).
  */
  check_args(argc, argv, help_string);
  for (i = 1; i < argc; i++)
           if (!strncmp(argv[i], "-all",  4)) inswapped_desired    =
                                              inswap_work_desired  =
                                              outswapped_desired   =
                                              outswap_work_desired = 1;
      else if (!strncmp(argv[i], "-ful",  4)) mappings_desired     =
                                              summary_desired      =
                                              threads_desired      = 1;
      else if (!strcmp( argv[i], "-in"     )) inswapped_desired    = 1;
      else if (!strncmp(argv[i], "-inw",  4)) inswap_work_desired  = 1;
      else if (!strncmp(argv[i], "-map",  4)) mappings_desired     = 1;
      else if (!strcmp( argv[i], "-out"    )) outswapped_desired   = 1;
      else if (!strncmp(argv[i], "-outw", 5)) outswap_work_desired = 1;
      else if (!strncmp(argv[i], "-sta",  4)) inswapped_desired    =
                                              outswapped_desired   = 1;
      else if (!strncmp(argv[i], "-sum",  4)) summary_desired      = 1;
      else if (!strncmp(argv[i], "-thr",  4)) threads_desired      = 1;
      else if (!strncmp(argv[i], "-wor",  4)) inswap_work_desired  =
                                              outswap_work_desired = 1;
      else {
        fprintf(stderr, "%s: invalid option, `%s'\n",
		argv[0],
		argv[i]);
        quit(1);
	}
  
  /*
  ** If no options were set, establish the default.
  ** (The default is to show all queues, just the task information).
  */
  if (argc == 1)
    inswapped_desired    =
    inswap_work_desired  =
    outswapped_desired   =
    outswap_work_desired = 1;

  /*
  ** Display the basic set of counters.
  */
  if (summary_desired) {
    /*
    ** Get the basic information from the system on the number os
    ** tasks on each swap queue.
    */
    if (!read_sym_val("task_inswapped_queue_count",    NUMBER, &_task_inswapped_queue_count,    &error) ||
        !read_sym_val("task_outswapped_queue_count",   NUMBER, &_task_outswapped_queue_count,   &error) ||
        !read_sym_val("task_inswap_work_queue_count",  NUMBER, &_task_inswap_work_queue_count,  &error) ||
        !read_sym_val("task_outswap_work_queue_count", NUMBER, &_task_outswap_work_queue_count, &error)) {
      fprintf(stderr, "Couldn't read task_swap_counter(s):\n");
      fprintf(stderr, "%s\n", error);
      quit(1);
      }  

    /*
    ** Display a header (with total task counts), followed by the individual
    ** task queue tallies.
    */
    print("");
    sprintf(buf, "Task swap queue count summary (total of %d tasks):",
		_task_inswapped_queue_count   +
		_task_inswap_work_queue_count +
		_task_outswapped_queue_count  +
		_task_outswap_work_queue_count);
    print(buf);
    sprintf(buf, "  In-swapped queue:          %4d tasks", _task_inswapped_queue_count);
    print(buf);
    sprintf(buf, "  In-swap work queue:        %4d tasks", _task_inswap_work_queue_count);
    print(buf);
    sprintf(buf, "  Out-swapped queue:         %4d tasks", _task_outswapped_queue_count);
    print(buf);
    sprintf(buf, "  Out-swap work queue:       %4d tasks", _task_outswap_work_queue_count);
    print(buf);
    print("");
    }

  /*
  ** Obtain the format of a task header.
  */
  if (!check_fields("struct task", task_fields, NUM_TASK_FIELDS, NULL)) {
    field_errors(task_fields, NUM_TASK_FIELDS);
    quit(1);
    }

  /*
  ** Check the format of the utask (needed for the u area).
  */
  if (!check_fields("struct utask", utask_fields, NUM_UTASK_FIELDS, NULL)) {
    field_errors(utask_fields, NUM_UTASK_FIELDS);
    quit(1);
    }

  /*
  ** If we are displaying information on threads, then we will need the
  ** thread definitions as well.
  */
  if (threads_desired) 
    if (!check_fields("struct thread", thread_fields, NUM_THREAD_FIELDS, NULL)) {
      field_errors(thread_fields, NUM_THREAD_FIELDS);
      quit(1);
      }

  /*
  ** If we are displaying mapping information, then we will need the
  ** vm_map definitions as well.
  */
  if (mappings_desired) 
    if (!check_fields("struct vm_map", vm_map_fields, NUM_VM_MAP_FIELDS, NULL)) {
      field_errors(vm_map_fields, NUM_VM_MAP_FIELDS);
      quit(1);
      }

  /*
  ** Loop four times (one for each queue).  A `switch()' is used to determine
  ** which queue is processed.  We start with the resident queues first, then
  ** to the in-transition queues (swapin/swapout).
  */
  for (i = 0; i < 4; i++) {
    long first;
    long last;

    switch (i) {
      case 0:	/* Display in-swapped queue */
		if (inswapped_desired) {
		  if (!read_sym_val("task_inswapped_queue.next",  NUMBER, &first, &error) ||
		      !read_sym_val("&task_inswapped_queue.next", NUMBER, &last,  &error)) {
			fprintf(stderr, "Couldn't read task_inswapped_queue.next:\n");
			fprintf(stderr, "%s\n", error);
			quit(1);
			}
		  }
		else
		  continue;
		break;
      case 1:	/* Display in-swap-work queue */
		if (inswap_work_desired) {
		  if (!read_sym_val("task_inswap_work_queue.next",  NUMBER, &first, &error) ||
		      !read_sym_val("&task_inswap_work_queue.next", NUMBER, &last,  &error)) {
			fprintf(stderr, "Couldn't read task_inswap_work_queue.next:\n");
			fprintf(stderr, "%s\n", error);
			quit(1);
			}
		  }
		else
		  continue;
		break;

      case 2:	/* Display out-swapped queue */
		if (outswapped_desired) {
		  if (!read_sym_val("task_outswapped_queue.next",  NUMBER, &first, &error) ||
		      !read_sym_val("&task_outswapped_queue.next", NUMBER, &last,  &error)) {
			fprintf(stderr, "Couldn't read task_outswapped_queue.next:\n");
			fprintf(stderr, "%s\n", error);
			quit(1);
			}
		  }
		else
		  continue;
		break;

      case 3:	/* Display out-swap-work queue */
		if (outswap_work_desired) {
		  if (!read_sym_val("task_outswap_work_queue.next",  NUMBER, &first, &error) ||
		      !read_sym_val("&task_outswap_work_queue.next", NUMBER, &last,  &error)) {
			fprintf(stderr, "Couldn't read task_outswap_work_queue.next:\n");
			fprintf(stderr, "%s\n", error);
			quit(1);
			}
		  }
		else
		  continue;
		break;
      } /* switch() */

    /*
    ** We have the start and end of the list.
    ** Retrieve the information now, each task at a time.
    */
    for (pointer = first; pointer != last;) {
      int utask_pointer;

      /*
      ** Establish a type for the pointer (it will follow through the data
      ** structures in the swap list).
      */
      if (!cast(pointer, "struct task", &header_address, &error)) {
        fprintf(stderr, "Couldn't cast pointer to struct task:\n");
        fprintf(stderr, "%s\n", error);
        quit(1);
        }

      /*
      ** Read the task structure pointed to by `pointer'.
      */
      if (!read_field_vals(header_address, task_fields, NUM_TASK_FIELDS)) {
	fprintf(stderr, "Unable to read struct task fields through pointer 0x%08x:\n", pointer);
	field_errors(task_fields, NUM_TASK_FIELDS);
        quit(1);
	}

      /*
      ** Display information about the task.
      ** (Leave a blank line between tasks.)
      */
      if (pointer != first)
	print("");
      sprintf(buf, "Task %08x: Ref %d, Map %08x, Threads %d, Pidx %d, Wset %d, SwState ",
			pointer,
			task_fields[0].data,
			task_fields[1].data,
			task_fields[3].data,
			task_fields[4].data,
			task_fields[10].data);
      switch ((int) task_fields[6].data) {
        case TASK_INSWAPPED:	strcat(buf, "Inswapped");
				break;
        case TASK_OUTSWAPPED:	strcat(buf, "Outswapped");
				break;
        case TASK_COMMING_IN:	strcat(buf, "SwappingIn");
				break;
        case TASK_GOING_OUT:	strcat(buf, "SwappingOut");
				break;
        default:		strcat(buf, "Unknown");
				break;
        }

      /*
      ** Show the swap request.
      */
      switch ((int) task_fields[7].data) {
        case TASK_ALL_SET:	strcat(buf, ", SwapReq AllSet ");
				break;
        case TASK_WANTS_IN:	strcat(buf, ", SwapReq WantsIn");
				break;
        default:		strcat(buf, ", SwapReq Unknown");
				break;
        }
      print(buf);

      /*
      ** Get the utask and proceed to obtain the command name for display.
      */
      if (!cast((long) task_fields[11].data, "struct utask", &header_address,
		&error)) {
        fprintf(stderr, "Couldn't cast pointer to struct utask:\n");
        fprintf(stderr, "%s\n", error);
        quit(1);
        }

      /*
      ** Read the utask structure pointed to by `pointer->u_address'.
      */
      if (!read_field_vals(header_address, utask_fields, NUM_UTASK_FIELDS)) {
	fprintf(stderr, "Unable to read struct utask fields through pointer 0x%08x:\n", task_fields[11].data);
	field_errors(utask_fields, NUM_UTASK_FIELDS);
        quit(1);
	}

      /*
      ** Display the additional utask information about the task.
      ** Second line of task information shows the size of text, data and stack.
      */
      sprintf(buf, "               Textsize %08x (%d), DataSize %08x (%d), StackSize %08x (%d)",
		utask_fields[2].data,
		utask_fields[2].data,
		utask_fields[3].data,
		utask_fields[3].data,
		utask_fields[4].data,
		utask_fields[4].data);
      print(buf);

      /*
      ** The third line of task information shows address ranges.
      */
      sprintf(buf, "               TextStart %08x, DataStart %08x, StackStart/End %08x/%08x",
		utask_fields[5].data,
		utask_fields[6].data,
		utask_fields[7].data,
		utask_fields[8].data);
      print(buf);

      /*
      ** The fourth line of task information shows the login name and the command
      ** used to start the process.
      */
      sprintf(buf, "               LoginName %s, command %s",
		(char *) (* (char *) utask_fields[1].data ? utask_fields[1].data : "(none)"),
		(char *) (* (char *) utask_fields[0].data ? utask_fields[0].data : "(none)"));
      print(buf);

      /*
      ** If -threads was included in the options list, then display information about
      ** the various threads belonging to this task.
      */
      if (threads_desired) {
        long thread_pointer;

        for (thread_pointer = (long) task_fields[2].data;
	     task_fields[3].data--;) {
          if (!cast(thread_pointer, "struct thread", &header_address, &error)){
            fprintf(stderr, "Couldn't cast pointer to struct thread:\n");
            fprintf(stderr, "%s\n", error);
            quit(1);
            }

          /*
          ** Read the task structure pointed to by `pointer'.
          */
          if (!read_field_vals(header_address, thread_fields, NUM_THREAD_FIELDS)) {
	    fprintf(stderr, "Unable to read struct thread fields through pointer 0x%08x:\n", pointer);
	    field_errors(thread_fields, NUM_THREAD_FIELDS);
            quit(1);
	    }

	  /*
	  ** Format the thread information.
	  */
	  sprintf(buf, "    Thread %08x: Ref %d, KStack %08x, SwState ",
			thread_pointer,
			thread_fields[1].data,
			thread_fields[2].data);
	  switch ((int) thread_fields[3].data) {
	    case TH_SW_UNSWAPPABLE:	strcat(buf, "Unswappable");
					break;
	    case TH_SW_IN:		strcat(buf, "In");
					break;
	    case TH_SW_GOING_OUT:	strcat(buf, "GoingOut");
					break;
	    case TH_SW_WANT_IN:		strcat(buf, "WantsIn");
					break;
	    case TH_SW_OUT:		strcat(buf, "Out");
					break;
	    case TH_SW_COMING_IN:	strcat(buf, "ComingIn");
					break;
	    default:			strcat(buf, "Unknown");
					break;
	    }

	  /*
	  ** Thread state.
	  */
	  strcat(buf, ", State");
	  if ((int) thread_fields[4].data == 0)
	    strcat(buf, " Unknown");
	  else {
	    if ((int) thread_fields[4].data & TH_WAIT)
	      strcat(buf, " Wait");
	    if ((int) thread_fields[4].data & TH_SUSP)
	      strcat(buf, " Susp");
	    if ((int) thread_fields[4].data & TH_RUN)
	      strcat(buf, " Run");
	    if ((int) thread_fields[4].data & TH_SWAPPED)
	      strcat(buf, " Swapped");
	    if ((int) thread_fields[4].data & TH_IDLE)
	      strcat(buf, " Idle");
	    }
	  print(buf);

	  /*
	  ** Advance to the next thread in the list.
	  */
	  thread_pointer = (long) thread_fields[0].data;
	  } /* for (thread_pointer) */
        } /* if (threads_desired) */

      /*
      ** If -mappings was included in the options list, then display information about
      ** the address space of this task.
      */
      if (mappings_desired) {
        if (!cast((long) task_fields[1].data, "struct vm_map",
		  &header_address, &error)) {
          fprintf(stderr, "Couldn't cast .map to struct vm_map:\n");
          fprintf(stderr, "%s\n", error);
          quit(1);
          }

        /*
        ** Read the vm_map structure pointed to by `pointer->map'.
        */
        if (!read_field_vals(header_address, vm_map_fields, NUM_VM_MAP_FIELDS)) {
	  fprintf(stderr, "Unable to read struct vm_map fields through pointer 0x%08x:\n", task_fields[1].data);
	  field_errors(vm_map_fields, NUM_VM_MAP_FIELDS);
          quit(1);
	  }

        /*
        ** Output some map information.
        */
        sprintf(buf, "    Map    %08x: Nentries %d, Size %d, Ref_count %d, Res_count %d",
		task_fields[1].data,
		vm_map_fields[0].data,
		vm_map_fields[2].data,
		vm_map_fields[4].data,
		vm_map_fields[8].data);
        print(buf);

	/*
	** Output address information.
	*/
	sprintf(buf, "                     Map_ops %08x, Pmap %08x, Hint %08x, FirstFree %08x, Private %08x",
		vm_map_fields[1].data,
		vm_map_fields[3].data,
		vm_map_fields[5].data,
		vm_map_fields[6].data,
		vm_map_fields[7].data);
	print(buf);

	/*
	** Output page/fault information.
	*/
	sprintf(buf, "                     FaultRate %d, PageFaults %d, FaultRateTime %d",
		vm_map_fields[9].data,
		vm_map_fields[10].data,
		vm_map_fields[11].data);
	print(buf);
        } /* if (mappings_desired) */

      /*
      ** Set `pointer' to the next struct in the list.
      */
      pointer = (long) task_fields[5].data;
      } /* for(pointer) */
    } /* for(i) */
  quit(0);
}
