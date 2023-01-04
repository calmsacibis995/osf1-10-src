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
static char *rcsid = "@(#)$RCSfile: scsi.c,v $ $Revision: 1.1.3.4 $ (DEC) $Date: 1992/11/05 17:53:21 $";
#endif
#include <stdio.h>
#include <io/common/devio.h>
#include <sys/time.h>
#include <dec/io/scsi/mips/scsivar.h>
#include <dec/io/scsi/mips/scsireg.h>
#include "krash.h"

static char *help_string =
"scsi - print information about the system's scsi busses                  \\\n\
    Usage : scsi                                                          \\\n\
";

FieldRec fields[] = {
  { ".sc_sysid", NUMBER, NULL, NULL },
  { ".sc_cntlr_alive", NUMBER, NULL, NULL },
  { ".sc_aipfts", NUMBER, NULL, NULL },
  { ".sc_lostarb", NUMBER, NULL, NULL },
  { ".sc_lastid", NUMBER, NULL, NULL },
  { ".sc_active", NUMBER, NULL, NULL },
  { ".sc_prevpha", NUMBER, NULL, NULL },
  { ".sc_fstate", NUMBER, NULL, NULL },
  { ".port_start", NUMBER, NULL, NULL },
  { ".port_reset", NUMBER, NULL, NULL },
  { ".sc_progress.tv_sec", NUMBER, NULL, NULL },
  { ".sc_progress.tv_usec", NUMBER, NULL, NULL }
};

#define NUM_FIELDS (sizeof(fields)/sizeof(fields[0]))

struct scsi_dcode {
	long code;
	char *desc;
};

struct scsi_dcode scsi_device_flags[] = {
	{ SCSI_REQSNS,    "SCSI_REQSNS"                             },
	{ SCSI_STARTUNIT, "SCSI_STARTUNIT"                          },
	{ SCSI_TRYSYNC,   "SCSI_TRYSYNC"                            },
	{ SCSI_TESTUNITREADY,"SCSI_TESTUNITREADY"                   },
	{ SCSI_READCAPACITY,"SCSI_READCAPACITY"                     },
	{ SCSI_NODIAG,    "SCSI_NODIAG"                             },
	{ SCSI_MODSEL_PF, "SCSI_MODSEL_PF"                          },
	{ SCSI_REMOVABLE_DISK,"SCSI_REMOVABLE_DISK"                 },
	{ SCSI_MODSEL_EXABYTE,"SCSI_MODSEL_EXABYTE"                 },
#ifdef vax
	{ SCSI_NODBBR,    "SCSI_NODBBR"                             },
#endif vax
        { NULL,           NULL                                      }
};

struct scsi_dcode scsi_mesg_protocols[] = {
	{ SZ_CMDCPT,     "Command Complete"                         },
        { SZ_EXTMSG,     "Extended message"                         },
        { SZ_SDP,        "Save Data Pointers"                       },
        { SZ_RDP,        "Restore Data Pointers (DISK: new)"        },
        { SZ_DISCON,     "Disconnect"                               },
#ifdef vax
        { SZ_IDE,        "Initiator Detected Error"                 },
#endif vax
        { SZ_ABT,        "Abort"                                    },
        { SZ_MSGREJ,     "Message Reject"                           },
        { SZ_NOP,        "No Operation"                             },
        { SZ_MSGPE,      "Message Parity Error"                     },
        { SZ_LNKCMP,     "Linked Command Complete"                  },
        { SZ_LNKCMPF,    "Linked Command Complete with Flag"        },
        { SZ_DEVRST,     "Bus Device Reset"                         },
        { SZ_ID_NODIS,   "IDENTIFY wo/ disconnect capability"       },
        { SZ_ID_DIS,     "IDENTIFY w/ disconnect capability"        },
        { NULL,          NULL                                       }
};

struct scsi_dcode scsi_phase_states[] = {
        { SZ_SP_ARB,     "Arbitrate for the Bus"                    },
	{ SZ_SP_SEL,     "Select the target device"                 },
        { SZ_CMD_PHA,    "Command Phase"                            },
        { SZ_DATAO_PHA,  "Data Out Phase"                           },
	{ SZ_DATAI_PHA,  "Data In Phase"                            },
	{ SZ_STATUS_PHA, "Status Phase"                             },
	{ SZ_MESSI_PHA,  "Message In Phase"                         },
	{ SZ_MESSO_PHA,  "Message Out Phase"                        },
	{ SZ_RELBUS,     "Release the Bus"                          },
        { NULL,          NULL                                       }
};

struct scsi_dcode scsi_status_byte[] = {
	{ SZ_GOOD,       "Good"                                     },
	{ SZ_CHKCND,     "Check Condition"                          },
        { SZ_BUSY,       "Device cannot accept a command (busy)"    },
	{ SZ_INTRM,      "Intermediate"                             },
	{ SZ_RESCNF,     "Reservation Conflict"                     },
	{ SZ_BAD,        "Fatal error command (couldn't do RQSNS)"  },
        { NULL,          NULL                                       },
};

struct scsi_dcode scsi_statpos_states[] = {
	{ SZ_NEXT,       "Process next request from queue"          },
	{ SZ_SP_START,   "Start a Status or Positioning Operation"  },
	{ SZ_SP_CONT,    "Continue SZ_SP_SETUP:"                    },
	{ SZ_RW_START,   "Setup a Read or Write Operation"          },
	{ SZ_RW_CONT,    "Start/Continue a Read or Write Operation" },
	{ SZ_R_STDMA,    "Start DMA READ"                           },
	{ SZ_W_STDMA,    "Start DMA WRITE"                          },
	{ SZ_R_DMA,      "Read DMA processing"                      },
        { SZ_W_DMA,      "Write DMA processing"                     },
	{ SZ_R_COPY,     "Copy READ data to memory"                 },
	{ SZ_ERR,        "DMA Error"                                },
        { NULL,          NULL                                       }
};
#ifdef notdef
struct scsi_dcode scsi_stat_devio[] = {
	{ DEV_BOM,       "Beginning-of-medium (BOM)"                },
	{ DEV_EOM,       "End-of-medium (EOM)"                      },
	{ DEV_OFFLINE,   "Offline"                                  },
	{ DEV_WRTLCK,    "Write locked"                             },
	{ DEV_BLANK,     "Blank media"                              },
        { DEV_WRITTEN,   "Write on last operation"                  },
        { DEV_CSE,       "Cleared serious exception"                },
	{ DEV_SOFTERR,   "Device soft error"                        },
	{ DEV_HARDERR,   "Device hard error"                        },
	{ DEV_DONE,      "Operation complete"                       },
	{ DEV_RETRY,     "Retry"                                    },
	{ DEV_ERASED,    "Erased"                                   },
        { NULL,          NULL                                       }
};

struct scsi_dcode scsi_category_stat_devio[] = {
	{ DEV_TPMARK,    "Unexpected tape mark"                     },
	{ DEV_SHRTREC,   "Short record"                             },
	{ DEV_RDOPP,     "Read opposite"                            },
	{ DEV_RWDING,    "Rewinding"                                },
	{ DEV_800BPI,    "800 bpi tape density"                     },
	{ DEV_1600BPI,   "1600 bpi tape density"                    },
	{ DEV_6250BPI,   "6250 bpi tape density"                    },
	{ DEV_6666BPI,   "6666 bpi tape density"                    },
        { DEV_10240BPI,  "10240 bpi tape density"                   },
	{ DEV_38000BPI,  "38000 bpi tape density"		    },
	{ DEV_LOADER,    "Media loader present"			    },
	{ DEV_38000_CP,  "38000 bpi compacted density"		    },
	{ DEV_76000BPI,  "76000 bpi tape density"		    },
	{ DEV_76000_CP,  "76000 bpi compacted density"		    },
	{ DEV_8000_BPI,  "QIC-24 9 tracks"			    },
	{ DEV_10000_BPI, "QIC-120 15 tracks & QIC-150 18 tracks"    },
	{ DEV_16000_BPI, "QIC-320/525 26 tracks"		    },
        { NULL,          NULL                                       }
};
#endif
struct scsi_dcode scsi_command_opcodes[] = {
	{ SZ_TUR,        "TEST UNIT READY Command"                  },
	{ SZ_REWIND,     "REWIND Command"                           },
	{ SZ_RQSNS,      "REQUEST SENSE Command"                    },
	{ SZ_RBL,        "READ BLOCK LIMITS Command"                },
        { SZ_READ,       "READ Command"                             },
	{ SZ_WRITE,      "WRITE Command"                            },
	{ SZ_TRKSEL,     "TRACK SELECT Command"                     },
	{ SZ_RESUNIT,    "RESERVE UNIT Command"                     },
	{ SZ_WFM,        "WRITE FILEMARKS Command"                  },
        { SZ_SPACE,      "SPACE Command"                            },
	{ SZ_INQ,        "INQUIRY Command"                          },
	{ SZ_VFY,        "VERIFY Command"                           },
	{ SZ_RBD,        "RECOVER BUFFERED DATA Command"            },
	{ SZ_MODSEL,     "MODE SELECT Command"                      },
	{ SZ_RELUNIT,    "RELEASE UNIT Command"                     },
	{ SZ_ERASE,      "ERASE Command"                            },
	{ SZ_MODSNS,     "MODE SENSE Command"                       },
	{ SZ_SSLU,       "START/STOP or LOAD/UNLOAD Command"        },
	{ SZ_RECDIAG,    "RECEIVE DIAGNOSTIC RESULT Command"        },
	{ SZ_SNDDIAG,    "SEND DIAGNOSTIC Command"                  },
#ifdef mips
	{ SZ_MEDREMOVAL, "PREVENT/ALLOW MEDIUM REMOVAL Command"	    },
#endif /* mips */
	{ SZ_P_FSPACER,  "Psuedo opcode for space record"           },
	{ SZ_P_FSPACEF,  "Psuedo opcode for space file"             },
	{ SZ_RDCAP,      "DISK: READ CAPACITY"                      },
	{ SZ_P_BSPACER,  "Psuedo opcode for backspace record"       },
	{ SZ_P_BSPACEF,  "Pseudo opcode for backspace file"         },
	{ SZ_P_CACHE,    "Pseudo opcode for buffered mode"          },
	{ SZ_P_NOCACHE,  "Pseudo opcode for no buffered mode"       },
	{ SZ_P_LOAD,     "Pseudo opcode for load (not used)"        },
	{ SZ_P_UNLOAD,   "Pseudo opcode for unload"                 },
	{ SZ_P_SSUNIT,   "Pseudo opcode for start/stop unit"        },
	{ SZ_P_RETENSION,"Pseudo opcode for retension"		    },
#ifdef mips
	{ SZ_P_EJECT,	 "Psuedo opcode for eject caddy"	    },
#endif /* mips */
	{ SZ_FORMAT,     "DISK: FORMAT UNIT Command"                },
	{ SZ_REASSIGN,   "DISK: REASSIGN BLOCK Command"             },
	{ SZ_VFY_DATA,   "DISK: VERIFY DATA Command"                },
	{ SZ_RDD,        "DISK: READ DEFECT DATA Command"           },
	{ SZ_READL,      "DISK: READ LONG Command"                  },
	{ SZ_WRITEL,     "DISK: WRITE LONG Command"                 },
	{ SZ_READ_10,    "DISK: READ 10-byte Command"		    },
	{ SZ_WRITE_10,   "DISK: WRITE 10-byte Command"		    },
#ifdef mips
	{ SZ_SEEK_10,	 "DISK: SEEK 10-byte Command"		    },
#endif /* mips */
	{ RZSPECIAL,     "DISK: set for special disk commands"      },
#ifdef mips
	{ SZ_WRITE_BUFFER,      "CDROM: WRITE BUFFER Command"	    },
	{ SZ_READ_BUFFER,       "CDROM: READ BUFFER Command"	    },
	{ SZ_CHANGE_DEFINITION,	"CDROM: CHANGE DEFINITION Command"  },
	{ SZ_READ_SUBCHAN,	"CDROM: READ SUB-CHANNEL Command"   },
	{ SZ_READ_TOC,		"CDROM: READ TOC Command"	    },
	{ SZ_READ_HEADER,	"CDROM: READ HEADER Command"	    },
	{ SZ_PLAY_AUDIO,	"CDROM: PLAY AUDIO Command"	    },
	{ SZ_PLAY_AUDIO_MSF,	"CDROM: PLAY AUDIO MSF Command"	    },
	{ SZ_PLAY_AUDIO_TI,	"CDROM: PLAY AUDIO TRACK/INDEX"	    },
	{ SZ_PLAY_TRACK_REL,	"CDROM: PLAY TRACK RELATIVE Cmd"    },
	{ SZ_PAUSE_RESUME,	"CDROM: PAUSE/RESUME Command"	    },
	{ SZ_PLAY_AUDIO_12,	"CDROM: PLAY AUDIO Cmd (12 byte)"   },
	{ SZ_PLAY_TRACK_REL_12,	"CDROM: PLAY TRACK RELATIVE Cmd"    },
	{ SZ_SET_ADDRESS_FORMAT,"CDROM: SET ADDRESS FORMAT Cmd"	    },
	{ SZ_PLAYBACK_STATUS,	"CDROM: PLAYBACK STATUS Command"    },
	{ SZ_PLAY_TRACK,	"CDROM: PLAY TRACK Command"	    },
	{ SZ_PLAY_MSF,		"CDROM: PLAY MSF Command"	    },
	{ SZ_PLAY_VAUDIO,	"CDROM: PLAY AUDIO Command"	    },
	{ SZ_PLAYBACK_CONTROL,	"CDROM: PLAYBACK CONTROL Command"   },
#endif /* mips */
        { NULL,          NULL                                       }
};

struct scsi_dcode scsi_sc_selstat[] = {
	{ SZ_IDLE,       "The device is not selected (BUS Free)"    },
	{ SZ_SELECT,     "The device is selected"                   },
	{ SZ_DISCONN,    "The device has disconnected"              },
	{ SZ_RESELECT,   "The device is in the reselection process" },
	{ SZ_BBWAIT,     "Bus Busy Wait (wait for bus free)"        },
#ifdef vax
	{ SZ_SELTIMO,    "Waiting for select (250 MS timeout)"      },
#endif vax
        { NULL,          NULL                                       }
};

struct scsi_dcode scsi_sc_szflags[] = {
	{ SZ_NORMAL,     "Normal (no szflags set)"                  },
	{ SZ_NEED_SENSE, "Need to do a Request Sense command"       },
	{ SZ_REPOSITION, "Need to reposition the tape	 (NOT USED)"},
	{ SZ_ENCR_ERR,   "Encountered an error"                     },
	{ SZ_DID_DMA,    "A DMA operation has been done"            },
	{ SZ_WAS_DISCON, "Disconnect occured during this command"   },
	{ SZ_NODEVICE,   "No SCSI device present"                   },
        { SZ_BUSYBUS,    "Bus is busy, don't start next command"    },
	{ SZ_RSTIMEOUT,  "Disconnected command to a disk timed out" },
	{ SZ_TIMERON,    "Disconnect being timed by timeout() call" },
	{ SZ_DID_STATUS, "Status phase occurred"                    },
	{ SZ_DMA_DISCON, "DMA was interrupted by a disconnect"      },
	{ SZ_SELWAIT,    "Waiting for 250 MS select timeout"        },
	{ SZ_ASSERT_ATN, "Need to assert the ATN signal"            },
	{ SZ_REJECT_MSG, "Need to reject a message"                 },
	{ SZ_RESET_DEV,  "Need to reset a scsi device"              },
	{ SZ_ABORT_CMD,  "Need to abort the current command"        },
	{ SZ_RETRY_CMD,  "Need to retry the current command"        },
	{ SZ_BUSYTARG,   "Target is busy, retry command later"      },
	{ SZ_RECOVERED,  "A recovered error occured, used w/BBR"    },
        { NULL,          NULL                                       }
};

struct scsi_dcode scsi_request_sense_key[] = {
	{ SZ_NOSENSE,    "Successful cmd or EOT, ILI, FILEMARK"     },
	{ SZ_RECOVERR,   "Successful cmd with controller recovery"  },
	{ SZ_NOTREADY,   "Device present but not ready"             },
	{ SZ_MEDIUMERR,  "Cmd terminated with media flaw"           },
	{ SZ_HARDWARE,   "Controller or drive hardware error"       },
	{ SZ_ILLEGALREQ, "Illegal command or command parameters"    },
	{ SZ_UNITATTEN,  "Unit Attention"                           },
	{ SZ_DATAPROTECT,"Write attempted to write protected media" },
	{ SZ_BLANKCHK,   "Read zero length record (EOD)"            },
	{ SZ_VNDRUNIQUE, "SZ_VNDRUNIQUE"                            },
	{ SZ_COPYABORTD, "SZ_COPYABORTD"                            },
	{ SZ_ABORTEDCMD, "Cmd aborted that may retry successfully"  },
	{ SZ_EQUAL,      "SZ_EQUAL"                                 },
	{ SZ_VOLUMEOVFL, "Buffer data left over after EOM"          },
	{ SZ_MISCOMPARE, "Miscompare on Verify command"             },
	{ SZ_RESERVED,   "SZ_RESERVED"                              },
#ifdef vax
	{ SZ_ASC_RRETRY, "SZ_ASC_RRETRY"                            },
	{ SZ_ASC_RERROR, "SZ_ASC_RERROR"                            },
#endif vax
        { NULL,          NULL                                       }
};

struct scsi_dcode scsi_device_class[] = {
	{ SZ_TAPE,       "TAPE device"                              },
        { SZ_DISK,       "DISK device"                              },
	{ SZ_CDROM,      "CDROM device"                             },
	{ SZ_UNKNOWN,    "UNKNOWN/UNSUPPORTED device"               },
        { TZ30,          "TZ30 cartridge tape"                      },
	{ TZK50,         "TZK50 cartridge tape"                     },
        { TZxx,          "TZxx non-DEC tape (may[not] work)"        },
	{ TZ05,		 "CSS TZ05 tape"			    },
	{ TZ07,		 "CSS TZ07 tape"			    },
	{ TZ9TRK,	 "Generic non-DEC 9trk tape"		    },
	{ TLZ04,	 "TLZ04 (RDAT) tape"			    },
	{ TZRDAT,	 "Generic RDAT tape"			    },
	{ TZK10,	 "TZK10 (QIC) tape"			    },
	{ TZQIC,	 "Generic non-DEC QIC tape"                 },
	{ TZK08,	 "Exabyte TZK08 8mm tape"		    },
	{ TZ8MM,	 "Generic non-DEC 8mm tape"		    },
	{ RZ22,          "RZ22  40 MB winchester disk"              },
	{ RZ23,          "RZ23 100 MB winchester disk"              },
        { RZ55,          "RZ55 300+ MB winchester disk"             },
#ifdef vax
	{ RZ56,          "RZ56 600+ MB winchester disk"             },
#endif vax
	{ RX23,          "RX23 3.5 1.4MB SCSI floppy disk"          },
	{ RX33,          "RX33 5.25 1.2MB SCSI floppy disk"         },
	{ RZxx,          "RZxx non-DEC disk (may[not] work)"        },
	{ RZ24,          "RZ24 winchester disk"                     },
	{ RZ57,          "RZ57 winchester disk"                     },
	{ RZ23L,         "RZ23L 116Mb winchester disk"              },
	{ RX26,          "RX26 3.5 2.8MB SCSI floppy disk"          },
	{ RZ25,          "RZ25 winchester disk"                     },
	{ RRD40,         "RRD40 CDROM (optical disk)"               },
#ifdef vax
	{ CDxx,          "CDxx non-DEC CDROM (may[not] work)"       },
#endif vax
	{ RRD42,         "RRD42 CDROM (optical disk)"               },
        { NULL,          NULL                                       }
};

#define EQUAL_MATCH 0
#define OR_MATCH 1

void *scsi_decode(code, dc, type, desc)
long code;
struct scsi_dcode *dc;
int type;
char *desc;
{
  int i;

  *desc = '\0';
  for(;dc->desc != NULL;*dc++){
    if(((type == EQUAL_MATCH) && (dc->code == code)) ||
	((type == OR_MATCH) && ((dc->code | code) == code))){
      if(strlen(desc) > 0) strcat(desc, ",");
      strcat(desc, dc->desc);
      if(type == EQUAL_MATCH) break;
      code -= dc->code;
    }
  }
  if(strlen(desc) == 0) sprintf(desc, "0x%p", code);
  return (desc);
}

static Boolean do_scsiprint_cntlr(sz_softc, cntrl)
DataStruct sz_softc;
int cntrl;
{
  DataStruct ele;
  char *error;
  char buf[256], desc[256];
  
  if((ele = array_element(sz_softc, cntrl, &error)) == NULL){
    fprintf(stderr, "Couldn't get %d'th element of sz_softc:\n, cntrl");
    fprintf(stderr, "%s\n", error);
  }
  if(!read_field_vals(ele, fields, NUM_FIELDS)){
    field_errors(fields, NUM_FIELDS);
    quit(1);
  }
  sprintf(buf, "SCSI controller #%d:", cntrl);
  print(buf);  
  sprintf(buf, "\tsc_sysid:\t0x%p", fields[0].data);
  print(buf);
  sprintf(buf, "\tsc_cntlr_alive:\t%d",fields[1].data);
  print(buf);
  sprintf(buf, "\tsc_aipfts:\t%d",fields[2].data);
  print(buf);
  sprintf(buf, "\tsc_lostarg:\t%d",fields[3].data);
  print(buf);
  sprintf(buf, "\tsc_lastid:\t%d",fields[4].data);
  print(buf);
  sprintf(buf, "\tsc_sc_active:\t%d",fields[5].data);
  print(buf);
  sprintf(buf, "\tsc_prevpha:\t%s",
	  scsi_decode((long)fields[6].data, scsi_phase_states,
			    EQUAL_MATCH, desc));
  print(buf);
  sprintf(buf, "\tsc_fstate:\t%s",
	  scsi_decode((long)fields[7].data, scsi_phase_states,
			    EQUAL_MATCH, desc));
  print(buf);
  sprintf(buf, "\tport_start:\t%s", addr_to_proc(fields[8].data));
  print(buf);
  sprintf(buf, "\tport_reset:\t%s", addr_to_proc(fields[9].data));
  print(buf);
  sprintf(buf, "\tsc_progress:\t(0x%p 0x%p)", fields[10].data,
	  fields[11].data);
  print(buf);
  return(True);
}

main(argc, argv)
int argc;
char **argv;
{
  long nscsibus;
  DataStruct sz_softc;
  char *error;
  int i;

  check_args(argc, argv, help_string);
  if(!check_fields("struct sz_softc", fields, NUM_FIELDS, NULL)){
    field_errors(fields, NUM_FIELDS);
    quit(1);
  }
  if(!read_sym_val("nNSCSIBUS", NUMBER, &nscsibus, &error)){
    fprintf(stderr, "Couldn't read nNSCSIBUS:\n");
    fprintf(stderr, "%s:\n", error);
    quit(1);
  }
  if((sz_softc = read_sym("sz_softc")) == NULL){
    fprintf(stderr, "Couldn't read sz_softc\n");
    quit(1);
  }
  print("");
  print("SCSI controller information:");
  print("");
  for(i=0;i<nscsibus;i++){
    if(!do_scsiprint_cntlr(sz_softc, i)) quit(1);
  }
  quit(0);
}
